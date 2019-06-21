<?php
/* Load ciphers */
$rom    = file_get_contents('wizwar2.nes');
$table0 = unpack('C*', substr($rom, 0x1FB8E+16, 5));
$table1 = unpack('C*', substr($rom, 0x1FB6A+16, 128));
$charset = 'BDGHJKLMNPQRTWXZ';

/* Define password contents (name, bytes#, bit position, bitmask, max value) */
$script = [ ['spellcount7',0, 0, 3, 3], // $45D.
            ['armor',      0, 1, 3, 3], // $68.
            ['shield',     0, 3, 3, 3], // $69. Note: Interpret 3 as 255
            ['weapon',     0, 5, 7, 6], // $67.
            ['spellcount5',1, 0, 3, 3], // $458.
            ['spellcount4',1, 2, 3, 3], // $457.
            ['spellcount2',1, 4, 3, 3], // $455.
            ['spellcount1',1, 6, 3, 3], // $454.
            ['keys',       2, 0,15,15],
            ['lives',      2, 4, 3, 3], // $44. Note: Interpret 0 as 3
            ['spellcount6',2, 6, 3, 3], // $459.
            ['spellcount3',3, 4, 3, 3], // $456.
            ['unknown',    3, 6, 3, 3], // $EF.
            ['level',      4, 0,255, 61] ];

/*************************************/

/* Set password to decode */
$password = 'NRTRZBDTRQDL';

/* Convert to 4-bit numbers */
$nibbles = Array();
for($n=0; $n<12; ++$n) $nibbles[$n] = strpos($charset, $password[$n]);
/* Merge into 8-bit bytes */
$bytes = Array();
for($n=0; $n<6; ++$n) $bytes[$n] = $nibbles[$n*2+0] * 16 + $nibbles[$n*2+1];

/* Decrypt */
print "byte5 = {$bytes[5]}\n";
$bytes[0] ^= $table0[1] ^ $bytes[5];
$bytes[1] ^= $table0[2];
$a = $bytes[0] ^ $bytes[1];
$bytes[4] ^= $table0[5] ^ $a;
$bytes[2] ^= $table0[3] ^ $table1[1 + $bytes[4]*2 + 0];
$bytes[3] ^= $table0[4] ^ $table1[1 + $bytes[4]*2 + 1] ^ $bytes[5];

/* Verify checksum */
if((($bytes[3] ^ $a ^ $bytes[2] ^ ($a >> 4)) & 15) != 0) print "Data error\n";

/* Extract data */
$values = Array();
foreach($script as $d)
{
  $values[$d[0]] = ($bytes[$d[1]] >> $d[2]) & $d[3];
  if($values[$d[0]] > $d[4]) { print "{$d[0]} is out of bounds\n"; }
}

print_r($values);

/*************************************/
// // For testing, make a little change
// $values['level'] = 59;
// foreach($values as &$v) $v=0; unset($v);

/* Encode the password */
$bytes = Array(0,0,0,0,0, $bytes[5]);
foreach($script as $d) { $bytes[$d[1]] += ($values[$d[0]] & $d[3]) << $d[2]; }

/* Add checksum (the xor of the first five nibbles) */
$a = $bytes[0] ^ $bytes[1];
$bytes[3] += (($a ^ $bytes[2] ^ ($a >> 4)) & 15);

/* Byte 5: Any arbitrary byte used for encryption. */
/*
    byte0 ^ table01
  ^ byte1 ^ table02
  ^ byte2 ^ table03 ^ table10
  ^ byte3 ^ table04 ^ table11
  ^ byte4 ^ table05 ^ byte0 ^ byte1
*/
$bytes[5] = $bytes[2] ^ $bytes[3] ^ $bytes[4] ^ 0x43
          ^ $table1[1 + $bytes[4]*2 + 0]
          ^ $table1[1 + $bytes[4]*2 + 1];
print "using byte5 = {$bytes[5]}\n";
/* Encrypt */
$bytes[0] ^= $table0[1] ^ $bytes[5];
$bytes[1] ^= $table0[2];
$bytes[2] ^= $table0[3] ^ $table1[1 + $bytes[4]*2 + 0];
$bytes[3] ^= $table0[4] ^ $table1[1 + $bytes[4]*2 + 1] ^ $bytes[5];
$bytes[4] ^= $table0[5] ^ $a;

/*
 *   X = table1[level*2+0]
 *   Y = table1[level*2+1]
 *
 * GAME:
 *
 *   Nibble11: 0x3 ^ byte4lo ^ byte0hi ^ byte0lo ^ byte1lo ^ Xlo ^ Ylo
 *   Nibble10: 0x4 ^ byte4hi ^ byte2hi ^ byte3hi           ^ Xhi ^ Yhi
 *   Nibble9:  0x9 ^ byte4lo ^ byte0lo ^ byte1lo
 *   Nibble8:        byte4hi ^ byte0hi ^ byte1hi
 *   Nibble7:  0x5 ^ byte4lo ^ byte2lo ^ Xlo
 *   Nibble6:        byte4hi ^ byte2hi ^ Xhi
 *   Nibble5:  0xA ^ byte2lo ^ Xlo
 *   Nibble4:  0xF ^ byte2hi ^ Xhi
 *   Nibble3:  0xB ^ byte1lo
 *   Nibble2:  0xC ^ byte1hi
 *   Nibble1:  0xE ^ byte4lo ^ byte0hi ^ byte1lo           ^ Xlo ^ Ylo
 *   Nibble0:  0x7 ^ byte4hi ^ byte0hi ^ byte2hi ^ byte3hi ^ Xhi ^ Yhi
 *
 * REALITY:
 *
 *   Nibble11: anything
 *   Nibble10: anything
 *   Nibble9:  0x9 ^ byte4lo ^ byte0lo ^ byte1lo
 *   Nibble8:        byte4hi ^ byte0hi ^ byte1hi
 *   Nibble7:  0x6 ^ byte2lo ^ Nibble11 ^ Ylo ^ byte0hi ^ byte0lo ^ byte1lo
 *   Nibble6:  0x4 ^ byte3hi ^ Nibble10 ^ Yhi
 *   Nibble5:  0xA ^ byte2lo ^ Xlo
 *   Nibble4:  0xF ^ byte2hi ^ Xhi
 *   Nibble3:  0xB ^ byte1lo
 *   Nibble2:  0xC ^ byte1hi
 *   Nibble1:  0xD ^ byte0lo ^ Nibble11
 *   Nibble0:  0x3 ^ byte0hi ^ Nibble10
 */
for($n=0; $n<6; ++$n) printf("%d: %02X\n", $n, $bytes[$n]);
/* Convert bytes into symbols */
$p = '';
for($n=0; $n<6; ++$n)
{
  $p .= $charset[$bytes[$n] >> 4];
  $p .= $charset[$bytes[$n] & 0xF];
}

/* Print output */
print "Orig: $password\n";
print "Pass: $p\n";
