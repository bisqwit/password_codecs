<?php
class DynaBlasterPasswords
{
  var $strings = Array('MUROYAGCQXSKIWHB', 'WRECKFVBYHOZNAGL', 'NEWSLOGIMPHJTVBQ', 'NUCLEARHPSTVZYGK', 'MONNVWYN');
  // This array specifies each component of the password, where the bits of that component are, and the value ranges.
  var $specs = Array(
    'levelhi' => ['src'=>[022,024,032,013],     'range'=>[1,8],  'pedantic'=>[1,8]],
    'levello' => ['src'=>[033,010,026,036],     'range'=>[1,8],  'pedantic'=>[1,8]],
    'bombs'   => ['src'=>[035,020,014,027],     'range'=>[0,15], 'pedantic'=>[1,8]],
    'length'  => ['src'=>[011,031,023,034],     'range'=>[0,15], 'pedantic'=>[1,15]],
    'running' => ['src'=>[015],                 'range'=>[0,1],  'pedantic'=>[0,1]],
     // Game validates only that nbits < 16.
    'nbits'   => ['src'=>[025,037,012,030,021], 'range'=>[0,15], 'pedantic'=>[0,31]],
     // Two unused bits, one "must be 1" bit and one "must be 0" bit
    'dummy'   => ['src'=>[016,017, 4,3],        'range'=>[4,7],  'pedantic'=>[4,4]],
     // Encryption settings
    'shift'   => ['src'=>[0,1,2],               'range'=>[0,7],  'pedantic'=>[0,1]],
    'invert'  => ['src'=>[5,6,7],               'range'=>[0,7],  'pedantic'=>[0,7]]
  );

  private function PopCount($w) { for($n=0; $w; $w &= ($w-1)) { ++$n; } return $n; }
  private function Mask($i) { return (($i&1) + (($i&2)<<7) + (($i&4)<<14))*0xFF; }
  
  function Decode($input)
  {
    // Convert letters into a numeric value. Set the error flag if an unrecognized character is found.
    for($word=$a=0; $a<8; ++$a) { $p = strpos(' '.$this->strings[$a>>1], $input[$a]);
                                  $word |= ($p-1) << ($a*4); }
    // Decrypt
    $word = (((($word >> 8) ^ $this->Mask($word >> 5)) >> ($word & 7)) << 8) | ($word & 0xFF);
    
    // Extract data
    $result =  Array();
    foreach($this->specs as $name=>$v)
    {
      // Read value from password
      for($value = $n = 0; $n < count($v['src']); ++$n)
        $value |= (($word >> $v['src'][$n]) & 1) << $n;

      // Verify that it’s within permitted range
      if($value < $v['range'][0] || $value > $v['range'][1])
        $result['error'] = true; // Set error flag

      // Verify that it’s a genuine password
      if($name == 'nbits' ? ($value != $this->PopCount($word & 0x7EDDFB00))
                          : ($value < $v['pedantic'][0] || $value > $v['pedantic'][1]))
        $result['fake'] = true; // Set fake flag (not an error though!)

      // Put it in result
      $result[$name] = $value;
    }
    return $result;
  }
  function Encode($data)
  {
    // Fill in all the missing values
    if(!isset($v['shift']))  $v['shift'] = rand(0,1);
    if(!isset($v['invert'])) $v['invert'] = rand(0,7);
    if(!isset($v['nbits']))  $v['nbits'] = 0; // TODO: PopCount(levelhi,levello,running,bombs,length)
    
    // Encode all the values into the password (including the encryption parameters).
    $word = 0x10;
    foreach($this->specs as $name=>$v)
      foreach($v['src'] as $n=>$position)
        $word |= ((@$data[$name] >> $n) & 1) << $position;

    // Encrypt the password.
    $word = ((($word & 0xFFFFFF00) << ($word&7)) ^ ($this->Mask($word>>5) << 8)) | ($word & 0xFF);

    // Convert the numeric value into letters.
    $result = '';
    for($a=0; $a<8; ++$a) $result .= $this->strings[$a>>1][ ($word >> ($a*4)) & 0xF ];

    // Append an error marker if some of the parameters were illegal
    if(isset($this->Decode($result)['error'])) $result .= '!';

    return $result;
  }
};

$p = new DynaBlasterPasswords;
$val = $p->Decode('UKZRJOTP');
$enc = $p->Encode($val);
print_r($val);
printf("%s\n", $enc);
