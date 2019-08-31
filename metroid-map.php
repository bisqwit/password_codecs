<?php
function ReadAt($bank, $address, $length, $globoffset=0)
{
  static $s = null;
  if(!isset($s)) $s = file_get_contents('metroid.nes');
  $offset = 0x10 + ($bank * 0x4000) + ($address & 0x3FFF);
  $data = substr($s, $offset, $length);
  $result = Array();
  for($n=0; $n<$length; ++$n) $result[$globoffset+$n] = ord($data[$n]);
  return $result;
}
function ReadWords($bank, $address, $length)
{
  $bytes = ReadAt($bank, $address, $length);
  $result = Array();
  for($n=0; $n<$length; $n+=2) $result[$n>>1] = $bytes[$n] | ($bytes[$n+1] << 8);
  return $result;
}

function LoadWorldMap()
{
  return ReadAt(0, 0xA53E, 1024);
}

function LoadGraphicsFor($area_bank)
{
  switch($area_bank)
  {
    case 1: $gfx_indices = Array(0x00,0x1B,0x14,0x17,0x18,0x19,0x16, 0x03,0x04,0x05,0x06,0x19,0x16); break;
    case 2: $gfx_indices = Array(0x00,0x1B,0x14,0x17,0x18,0x19,0x16, 0x04,0x05,0x07,0x08,0x09,0x19,0x16); break;
    case 4: $gfx_indices = Array(0x00,0x1B,0x14,0x17,0x18,0x19,0x16, 0x04,0x05,0x0A,0x0F,0x10,0x11,0x19,0x16); break;
    case 3: $gfx_indices = Array(0x00,0x1B,0x14,0x17,0x18,0x19,0x16, 0x05,0x0A,0x0B,0x0C,0x0D,0x0E,0x1A,0x1C,0x19,0x16); break;
    case 5: $gfx_indices = Array(0x00,0x1B,0x14,0x17,0x18,0x19,0x16, 0x04,0x05,0x0A,0x12,0x13,0x19,0x16); break;
  }
  $chr_tables = Array();
  for($n=0; $n<0x2000; ++$n) $chr_tables[] = 0;
  foreach($gfx_indices as $index)
  {
    $specs = ReadAt(7, 0xC6E0 + $index*7,   1);
    $words = ReadWords(7, 0xC6E0 + $index*7+1, 6);
    #print "--\n";
    $gfx_bank = $specs[0];
    $gfx_offs = $words[0];
    $ppu_offs = $words[1];
    $gfx_len  = $words[2];
    $gfx_data = ReadAt($gfx_bank, $gfx_offs, $gfx_len);
    foreach($gfx_data as $byte)
      $chr_tables[$ppu_offs++] = $byte;
  }
  ksort($chr_tables);
  return $chr_tables;
}
function LoadRoomsFrom($area_bank)
{
  $paletteptrtable = ReadWords($area_bank, 0x9560, 56); // Also in title screen; on title there's also $9F81 for ending
  $areapointers    = ReadWords($area_bank, 0x9598, 16);
  $startpos = ReadAt($area_bank, 0x95D7, 4);
  $start_xcoord = $startpos[0]; //OUTCOME; coords on global map
  $start_ycoord = $startpos[1]; //OUTCOME
  $ypos_screen  = $startpos[2]; //OUTCOME; start y-coord on screen
  $default_pal  = $startpos[3];
  
  $chr_tables = LoadGraphicsFor($area_bank);
  
  $palettes = Array(); //OUTCOME
  foreach($paletteptrtable as $pointer)
  {
    $bytes = ReadAt($area_bank, $pointer, 3);
    $ppu_offset = $bytes[1] | ($bytes[0] << 8);
    $length     = $bytes[2];
    $data  = ReadAt($area_bank, $pointer+3, $length);
    $pal = Array();
    for($n=0; $n<32; ++$n)   $pal[$n] = -1;
    foreach($data as $k=>$v) $pal[($ppu_offset&0x1F)+$k] = $v;
    $palettes[] = $pal;//Array('offset'=>$ppu_offset&0x1F, 'data'=>$data);
  }

  $special_pointer = $areapointers[0];
  $rooms_pointers  = ReadWords($area_bank, $areapointers[1], $areapointers[2] - $areapointers[1]);
  $struct_pointers = ReadWords($area_bank, $areapointers[2], 256);
  $macro_pointer   = $areapointers[3];
  
  $bytes = ReadAt($area_bank, $special_pointer, 0x800, $special_pointer);
  $arr_start = $special_pointer;
  
  $special_items = Array(); //OUTCOME
  while($special_pointer != 0xFFFF)
  {
    $ptr = $special_pointer;
    $ycoord           = $bytes[$ptr++];
    $special_pointer  = $bytes[$ptr++];
    $special_pointer |= $bytes[$ptr++] << 8;
    do {
      $xcoord           = $bytes[$ptr++];
      $boffs            = $bytes[$ptr++];
      $sizes = Array(0,2,2,0,1, 1,0,0,0,1,0, 0,0,0,0,0);
      // Types:
      //   0 = ExitSub
      //   1 = SqueeptHandler:   (type>>4 = slot number, params: enemy type, initial position (yyyyxxxx))
      //   2 = PowerUpHandler:   { Unique items. Params: itemtype, position (yyyyxxxx) }
      //   3 = SpecEnemyHandler  { no params }
      //   4 = ElevatorHandler   { param: Target map ID? }
      //   5 = CannonHandler     { one param byte }
      //   6 = MotherBrainHandler { no params }
      //   7 = ZeebetiteHandler  { no params }
      //   8 = RinkaHandler      { no params }
      //   9 = DoorHandler       { param: door info byte }
      //  10 = PaletteHandler    { no params }
      $endptr = $ptr+$boffs-2;
      while($ptr < $endptr)
      {
        $type             = $bytes[$ptr];
        if(($type & 0xF) == 0) { $ptr = $endptr; break; }
        $special_items[] = Array('x'=>$xcoord, 'y'=>$ycoord, 'type'=>$type&0xF, 'data'=>array_slice($bytes, $ptr-$arr_start, 1+$sizes[$type&0xF]));
        $ptr += $sizes[$type&0xF]+1;
      }
    } while($boffs != 0xFF);
  }

  $room_data = Array(); // OUTCOME
  foreach($rooms_pointers as $pointer)
  {
    $bytes = ReadAt($area_bank, $pointer, 256);
    $default_attribute = $bytes[0];
    $room = Array('default_attribute_ignored'=>$default_attribute, 'screen'=>[], 'actors'=>[]);
    $index = 1;
    $screen = Array();
    while($bytes[$index] != 0xFD && $bytes[$index] != 0xFF)
    {
      $ypos      = $bytes[$index] >> 4;
      $xpos      = $bytes[$index] & 0xF;
      $structno  = $bytes[$index+1];
      $attribute = $bytes[$index+2];
      $index += 3;
      //$room['objects'][] = Array($xpos,$ypos,$structno,$attribute);
      $screen = &$room['screen'];
      
      $ydelta = 0;
      $struct = ReadAt($area_bank, $struct_pointers[$structno], 256);
      #print "struct $structno: ".join('/',$struct)."\n";
      for($pos=0; ($len = $struct[$pos++]) != 0xFF; $pos += $len, ++$ydelta)
      {
        $slice = array_slice($struct, $pos, $len);
        foreach($slice as $xdelta => $macrobyte)
        {
          // Put macro $macros[$macrobyte] at $xpos+×delta, $ypos+$ydelta
          $macro = ReadAt($area_bank, $macro_pointer + $macrobyte*4, 4);
          for($m=0; $m<4; ++$m)
            $screen[ ($ypos+$ydelta)*2 + ($m>>1) ]
                   [ ($xpos+$xdelta)*2 + ($m&1) ] = Array($attribute, $macro[$m]);
        }
      }
      
      ksort($screen);
      foreach($screen as $y=>$rows) ksort($screen[$y]);
      unset($screen);
    }
    if($bytes[$index] == 0xFD) ++$index;
    while($bytes[$index] != 0xFF)
    {
      $type = $bytes[$index] & 0xF;
      switch($type)
      {
        case 7:
        case 1:  $room['actors'][] = Array('type'=>$type, 'data'=>array_slice($bytes,$index,3));
                 $index += 3;
                 break;
        case 4:
        case 2:  $room['actors'][] = Array('type'=>$type, 'data'=>array_slice($bytes,$index,2));
                 $index += 2;
                 break;
        case 6:  $room['actors'][] = Array('type'=>$type, 'data'=>array_slice($bytes,$index,1));
                 $index += 1;
                 break;
        default: $room['actors'][] = 'ERROR'; break;
      }
      // First byte & 0x0F:
      //    0,3,5 = exit sub
      //    1 = load enemy    (first byte>>4 = slot number, params: enemy type, initial position (yyyyxxxx))
      //    2 = load door     (param: door info byte)
      //    4 = load elevator (param: 032F)
      //    6 = load statues  (no params)
      //    7 = zeb hole (first byte & 0xF0 = $729, two params: $728,$72A+$72B)
    }
    $room_data[] = $room;
  }
  
  return Array('rooms'       => $room_data,
               'items'       => $special_items,
               'palettes'    => $palettes,
               'dfl_palette' => $default_pal,
               'xcoord'      => $start_xcoord,
               'ycoord'      => $start_ycoord,
               'ypos_screen' => $ypos_screen,
               'chr_tables'  => $chr_tables);
}

$areas    = explode('/', 'Brinstar/Norfair/Kraid/Tourian/Ridley');
$worldmap = LoadWorldMap();

function wave($p,$color) { return ($color+$p+8) % 12 < 6; }
function MakeRGBcolor($pixel,
	$saturation = 1.0, $hue = 0.0,
	$contrast = 1.0, $brightness = 1.0,
	$gamma = 1.8)
{
    static $sin=false, $cos=false;
    if($sin===false)
    {
        $cos = Array();
        $sin = Array();
        for($p=0; $p<12; ++$p)
        {
          $cos[$p] = cos( (3.141592653/6.) * ($p+$hue) );
          $sin[$p] = sin( (3.141592653/6.) * ($p+$hue) );
        }
    }
    
    // The input value is a NES color index (with de-emphasis bits). 
    // We need RGB values. Convert the index into RGB. 
    // For most part, this process is described at: 
    //    http://wiki.nesdev.com/w/index.php/NTSC_video 

    // Decode the color index 
    $color = ($pixel & 0x0F);
    $level = $color<0xE ? ($pixel>>4) & 3 : 1; 

    // Voltage levels, relative to synch voltage 
    $black = 0.518;
    $white = 1.962;
    $attenuation = 0.746;
    $levels = Array(0.350,0.518,0.962,1.550,  // Signal low 
                    1.094,1.506,1.962,1.962); // Signal high 

    $lo_and_hi = Array( $levels[$level + 4 * ($color == 0x0)], 
                        $levels[$level + 4 * ($color <  0xD)] );

    // Calculate the luma and chroma by emulating the relevant circuits: 
    $y=0;
    $i=0;
    $q=0;
    for($p=0; $p<12; ++$p) // 12 clock cycles per pixel. 
    { 
        // NES NTSC modulator (square wave between two voltage levels): 
        $spot = $lo_and_hi[wave($p,$color)]; 

        // De-emphasis bits attenuate a part of the signal: 
        if((($pixel & 0x40) && wave($p,12)) 
        || (($pixel & 0x80) && wave($p, 4)) 
        || (($pixel &0x100) && wave($p, 8))) $spot *= $attenuation; 

        // Normalize: 
        $v = ($spot - $black) / ($white-$black); 

        // Ideal TV NTSC demodulator: 
        // Apply contrast/brightness 
        $v = ($v - .5) * $contrast + .5; 
        $v *= $brightness / 12.; 

        $y += $v; 
        $i += $v * $cos[$p];
        $q += $v * $sin[$p];
    }    

    $i *= $saturation; 
    $q *= $saturation; 

    // Convert YIQ into RGB according to FCC-sanctioned conversion matrix. 
    $r = $y +  0.946882*$i +  0.623557*$q; if($r<0) $r=0; else { $r=pow($r, 2.2/$gamma)*255; if($r>255) $r=255; }
    $g = $y + -0.274788*$i + -0.635691*$q; if($g<0) $g=0; else { $g=pow($g, 2.2/$gamma)*255; if($g>255) $g=255; }
    $b = $y + -1.108545*$i +  1.709007*$q; if($b<0) $b=0; else { $b=pow($b, 2.2/$gamma)*255; if($b>255) $b=255; }

    return (((int)($r)) << 16) | (((int)($g)) << 8) | (((int)($b)));
}
$rgbpalette = Array();
for($n=0; $n<64; ++$n)
  $rgbpalette[] = MakeRGBcolor($n, 1.0/*sat*/, -0.081/*hue*/, 1.0/*con*/, 1.0/*bri*/, 1.8/*gam*/);

function RenderTile($im, $x,$y, $tileno,$attr, $chr_tables,$palette)
{
  global $rgbpalette;
  for($yp=0; $yp<8; ++$yp)
  {
    $byte1 = $chr_tables[ 0x1000 + ($tileno<<4) + ($yp) + 0 ];
    $byte2 = $chr_tables[ 0x1000 + ($tileno<<4) + ($yp) + 8 ];
    for($xp=0; $xp<8; ++$xp)
    {
      $pix = ((($byte1 >> (7-$xp)) & 1)     )
           | ((($byte2 >> (7-$xp)) & 1) << 1);
      #$color = $pix+$attr*4;
      $color = $palette[$pix + $attr*4];
      ImageSetPixel($im, $x+$xp, $y+$yp, @$rgbpalette[$color]);
    }
  }
}
function RenderRoom(&$im, $beginx,$beginy, $rooms, $room_no, $palette, $mapx,$mapy)
{
  $result = 0;
  $room = $rooms['rooms'][$room_no];
  $tiles = Array();
  for($y=0; $y<960; ++$y) $tiles[$y] = 0xFF;
  foreach($room['screen'] as $y => $columns)
    foreach($columns as $x => $data)
    {
      if($y >= 30 || $x >= 32) continue;
      $attr   = $data[0];
      $tileno = $data[1];
      $tiles[($y << 5) | $x] = $tileno + ($attr << 8);
    }
  #print "pal=$palette\n";
  #print_r($rooms['palettes']);
  for($y=0; $y<30; ++$y)
    for($x=0; $x<32; ++$x)
    {
      $tileno = $tiles[($y << 5) | $x];
      $attr   = $tileno >> 8;
      $tileno &= 0xFF;

      if($tileno >= 0xA0) // door
      {
        if($y==0)    $result |= 1;
        if($y >= 29) $result |= 2;
        if($x==0)    $result |= 4;
        if($x==31)   $result |= 8;
      }
      if($tileno != 0xFF)
        RenderTile($im, $beginx + ($x<<3), $beginy + ($y << 3),
                   $tileno, $attr,
                   $rooms['chr_tables'],
                   $rooms['palettes'][$palette]);
    }

  // TODO: result flag 16
  global $items;
  foreach($items as $item)
    if($item['x'] == $mapx && $item['y'] == $mapy && $item['type'] == 10)
      $result |= 16; // palette swap

  for($n=0; $n<32; ++$n)
    if(($tiles[$n] & 0xFF) == 0xA4
    || ($tiles[$n] & 0xFF) == 0xA5)
    {
      // Disable all directions if in elevator shaft
      $result = 0;
      break;
    }
  for($n=0; $n<32; ++$n)
    if(($tiles[$n+29*32] & 0xFF) == 0xA4
    || ($tiles[$n+29*32] & 0xFF) == 0xA5)
    {
      // Disable sideways if at top of elevator shaft
      $result &= 3;
      break;
    }

  foreach($room['actors'] as $actor)
  {
    $font = 4;
    $position = 0x88;
    if($actor['type'] == 1)
    {
      $position = $actor['data'][2];
    }
    if($actor['type'] == 2) // door
    {
      $position -= 0x10;
      if($actor['data'][1] & 0x10) $position -= 5; else $position += 4;
    }
    if($actor['type'] == 4) // elevator
    {
      $position -= 0x20;
    }
    switch($actor['type'])
    {
      case 1: $font = 2; $str = sprintf("Obj%02X.%d", $actor['data'][1], $actor['data'][0] >> 4); break;
      case 2: $font = 3; $str = sprintf("Door%02X", $actor['data'][1]); break; // load door
      case 4: $font = 3; $str = sprintf("Elev%02X", $actor['data'][1]); break; // load elevator
      case 6: $str = 'Statues'; break; // load statues
      case 7: $font = 1; $str = sprintf("Zeb%02X.%02X.%d", $actor['data'][1], $actor['data'][2], $actor['data'][0] >> 4); break; // zeb hole
      default: $str = sprintf("#{$actor['type']}");
    }
    // DoorXY:
    //     Ax = approach from left side (right side of screen)
    //     Bx = approach from right side (left side of screen)
    //     x1 = Blue door
    //     x0 = Red missile door
    //     x2 = Purple door
    //     x3 = Yellow door?
    // Door80 = missile door on left side (??WHERE?)
    // 
    // ElevXY:
    //     8x = bottom side (go up)
    //     0x = top side (go down)
    //     xF = end
    //  (INCORRECT):
    //     x3 = Brinstar-Tourian
    //     x1 = Brinstar-Norfair
    //     x2 = Brinstar-Kraid
    //     x4 = Norfair-Ridley
    // 
    
    $x = $beginx + ($position & 0xF)*16 + 8;
    $y = $beginy + ($position >> 4 )*16 + 8;
    $w = ImageFontWidth($font);
    $h = ImageFontHeight($font);
    ImageString($im, $font, $x-$w*(strlen($str)/2),$y-$h*(1/2), $str, 0xFF0000);
  }
  return $result;
}

$items = Array();
# 1=Brinstar, 2=Norfair, 3=Tourian, 4=Kraid, 5=Ridley
foreach(Array(1, 2, 4, 3, 5) as $area_bank)
{
  $bank = LoadRoomsFrom($area_bank);
  #print_r($bank);
  foreach($bank['items'] as $item) $items[] = $item;
  unset($bank['items']);
  $banks[$area_bank] = $bank;
}

$rw = 256; $rh = 240;
$im = ImageCreateTrueColor(32*$rw, 32*$rh);

/* PASS 1 */
$done  = Array();
$ok    = Array();
$blank = Array();
foreach(Array(1, 2, 4, 3, 5) as $area_bank)
{
  $rooms   = $banks[$area_bank];
  $start   = ($rooms['ycoord'] << 5) | $rooms['xcoord'];
  $palette = 0;//$rooms['dfl_palette']-1;
  print "dfl pal: $palette\n";
  $todo = Array();
  array_push($todo, $start | ($palette << 10));
  while(!empty($todo))
  {
    $current = array_pop($todo);
    $curpal = $current >> 10;
    $current &= 0x3FF;
    if(isset($done[$current])) continue;
    $done[$current] = $area_bank;

    $x = $current & 0x1F;
    $y = $current >> 5;
    $cell = $worldmap[$current];
    if($cell < count($rooms['rooms']))
    {
      $flags = RenderRoom($im, $x*$rw, $y*$rh, $rooms, $cell, $curpal, $x,$y);
      switch($current)
      {
        case (0x0B << 5) | 0x02: 
          $flags |= 4; // Force enable exit for mother brain
          break;
      }
      $nextpal = $curpal;
      if($flags & 16) $nextpal = (($nextpal+1) ^ 7) - 1;

      if(($flags & 1) && $y>0)  array_push($todo, ($current-32) | ($nextpal << 10));
      if(($flags & 2) && $y<31) array_push($todo, ($current+32) | ($nextpal << 10));
      if(($flags & 4) && $x>0)  array_push($todo, ($current-1) | ($nextpal << 10));
      if(($flags & 8) && $x<31) array_push($todo, ($current+1) | ($nextpal << 10));
      $ok[$current] = $nextpal;
    }
    else
    {
      $blank[$current] = true;
    }
  }
}
/* PASS 2 */
for($y=0; $y<32; ++$y)
  for($x=0; $x<32; ++$x)
  {
    $current = ($y << 5) | $x;
    #$cell = $worldmap[$current];
    if(isset($blank[$current]) && !isset($ok[$current]))
    {
      ImageFilledRectangle($im, $x*$rw, $y*$rh, $x*$rw+$rw-1, $y*$rh+$rh-1, 0xFF00FF);
    }
    elseif(!isset($done[$current]) && $worldmap[$current] < 0xFF)
    {
      $find_bank = 3;
      $find_pal  = 0;
      for($mul=1; $mul<24; ++$mul)
        foreach(Array(32,-32,1,-1) as $offset)
        {
          $next = $current + $offset * $mul;
          if(isset($done[$next]))
          {
            $cand_bank = $done[$next];
            if($worldmap[$current] < count($banks[$cand_bank]['rooms']))
            {
              $find_bank = $cand_bank;
              if(isset($ok[$next]))
              {
                $find_pal = $ok[$next];
              }
              else
              {
                $find_pal = $banks[$cand_bank]['dfl_palette']-1;
              }
              break 2;
            }
          }
        }
      
      $flags = RenderRoom($im, $x*$rw, $y*$rh, $banks[$find_bank], $worldmap[$current], $find_pal, $x,$y);
      ImageAlphaBlending($im, 1);
      ImageFilledRectangle($im, $x*$rw, $y*$rh, $x*$rw+$rw-1, $y*$rh+$rh-1, 0x400000FF);
    }
    elseif(!isset($done[$current]))
    {
      ImageFilledRectangle($im, $x*$rw, $y*$rh, $x*$rw+$rw-1, $y*$rh+$rh-1, 0x303030);
    }
  }
/* PASS 3: ITEMS */
foreach($items as $item)
{
  $x = $item['x'];
  $y = $item['y'];
  $position = 0x88;
  if($item['type'] == 1 || $item['type'] == 2) // objs and powerups
  {
    $position = $item['data'][2];
  }
  $font = 4;
  $color = 0x00FF00;

  $x = ($x*$rw) + ($position & 0xF)*16 + 8;
  $y = ($y*$rh) + ($position >> 4 )*16 + 8;

  switch($item['type'])
  {
    case 1:  $font = 2; $str = sprintf("Obj%02X.%d", $item['data'][1], $item['data'][0] >> 4); break;
    case 2:  $str = sprintf("Pwr%02X", $item['data'][1]); break;
    case 3:  $font = 2; $y -= 0x20; $str = sprintf("Spec"); break;
    case 4:  $font = 3; $y -= 0x10; $str = sprintf("Elev%02X", $item['data'][1]); break;
    case 5:  $font = 1; $y -= 0x20; $str = sprintf("Cann%02X", $item['data'][1]); break;
    case 6:  $font = 2; $y -= 0x14; $str = sprintf("Brain"); break;
    case 7:  $font = 1; $y -= 0x06; $str = sprintf("Zebe"); break;
    case 8:  $font = 1; $y -= 0x30; $str = sprintf("Rinka"); break;
    case 9:  $font = 3; $y -= 0x10; $str = sprintf("Door%02X", $item['data'][1]); break;
    case 10: $color = 0xFFFF55; $y -= 0x20; $str = "PALETTE"; break;
    default: $str = sprintf("#{$item['type']}");
  }

  $w = ImageFontWidth($font);
  $h = ImageFontHeight($font);
  ImageString($im, $font, $x-$w*(int)(strlen($str)/2),$y-(int)($h*(1/2)), $str, $color);
}

ImagePng($im, "test.png");
print "done $area_bank\n";

