<?php
$rom    = file_get_contents('wizwar2.nes');
$table0 = unpack('C*', substr($rom, 0x1FB8E+16, 5));
$table1 = unpack('C*', substr($rom, 0x1FB6A+16, 128));

/* Set password to decode */
$password = 'QKKKKKKKKKKK';
$charset  = 'BDGHJKLMNPQRTWXZ';

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

if((($bytes[3] ^ $a ^ $bytes[2] ^ ($a >> 4)) & 15) != 0) print "Data error\n";

$values = Array(
  'lives'  => ($bytes[2] >> 4) & 3, //     but store 3 if value is 0
  'weapon' => ($bytes[0] >> 5) & 7, // 67 (die if value is 7)
  'armor'  => ($bytes[0] >> 1) & 3, // 68
  'shield' => ($bytes[0] >> 3) & 3, // 69, but store 255 if value is 3
  'unknown' => ($bytes[3] >> 6) & 3, // EF
  'number_of_item1' => ($bytes[1] >> 6) & 3, // 454
  'number_of_item2' => ($bytes[1] >> 4) & 3, // 455
  'number_of_item3' => ($bytes[3] >> 4) & 3, // 456
  'number_of_item4' => ($bytes[1] >> 2) & 3, // 457
  'number_of_item5' => ($bytes[1] >> 0) & 3, // 458
  'number_of_item6' => ($bytes[2] >> 6) & 3, // 459
  'number_of_item7' => ($bytes[0] >> 0) & 1, // 45D
  'keys' => $bytes[2] & 15, // keys
  'level' => $bytes[4]
);

print "Note: Level is {$values['level']}. Reject if >= 62\n";
print "Note: Weapon is {$values['weapon']}. Reject if >= 7\n";


/* For testing, make a little change */
#$values['level'] = 59;

/* Encode the password */

$bytes[0] = $values['number_of_item7']
          + $values['armor'] * 2
          + $values['shield'] * 8
          + $values['weapon'] * 32;
$bytes[1] = $values['number_of_item5']
          + $values['number_of_item4'] * 4
          + $values['number_of_item2'] * 16
          + $values['number_of_item1'] * 64;
$bytes[2] = $values['keys']
          + $values['lives'] * 16
          + $values['number_of_item6'] * 64;
$a = $bytes[0] ^ $bytes[1];
$bytes[3] = $values['number_of_item3'] * 16
          + $values['unknown'] * 64
          + (($a ^ $bytes[2] ^ ($a >> 4)) & 15);
$bytes[4] = $values['level'];

// Byte 5: Any arbitrary byte used for encryption.
#$bytes[5] = $a ^ $bytes[2] ^ $bytes[3] ^ $bytes[4];
print "using byte5 = {$bytes[5]}\n";
$bytes[0] ^= $table0[1] ^ $bytes[5];
$bytes[1] ^= $table0[2];
$bytes[2] ^= $table0[3] ^ $table1[1 + $bytes[4]*2 + 0];
$bytes[3] ^= $table0[4] ^ $table1[1 + $bytes[4]*2 + 1] ^ $bytes[5];
$bytes[4] ^= $table0[5] ^ $a;

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
