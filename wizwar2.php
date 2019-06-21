<?php
$rom    = file_get_contents('wizwar2.nes');
$table0 = unpack('C*', substr($rom, 0x1FB8E+16, 5));
$table1 = unpack('C*', substr($rom, 0x1FB6A+16, 128));

for($n=0; $n<64; ++$n) printf("%02X ", $table1[$n*2+1]); print "\n";
for($n=0; $n<64; ++$n) printf("%02X ", $table1[$n*2+2]); print "\n";

/* Set password to decode */
$password = 'GNHMNBKZBGTG';
$charset  = 'BDGHJKLMNPQRTWXZ';

/* Convert to 4-bit numbers */
$nibbles = Array();
for($n=0; $n<12; ++$n) $nibbles[$n] = strpos($charset, $password[$n]);
/* Merge into 8-bit bytes */
$bytes = Array();
for($n=0; $n<6; ++$n) $bytes[$n] = $nibbles[$n*2+0] * 16 + $nibbles[$n*2+1];

/* Decrypt */
$bytes[0] ^= $bytes[5];
$bytes[3] ^= $bytes[5];
for($n=0; $n<5; ++$n) $bytes[$n] ^= $table0[1 + $n];

$levelnumber = $bytes[4] ^ $bytes[0] ^ $bytes[1];
print "Level is $levelnumber . Reject if >= 62\n";

$bytes[2] ^= $table1[1 + $levelnumber*2 + 0];
$bytes[3] ^= $table1[1 + $levelnumber*2 + 1];
$crazy = ($bytes[0] ^ $bytes[1] ^ $bytes[2] ^ (($bytes[0] ^ $bytes[1]) >> 4)) & 15;
printf("Crazy = %d Byte3low = %d, these should match\n", $crazy, $bytes[3] & 15);
#for($n=0; $n<6; ++$n) printf("%02X ", $bytes[$n]); print "\n";

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
  'level' => $levelnumber
);

/* For testing, make a little change */
$values['level'] = 59;

/* Encode the password */

$bytes[5] = 0xFF; // Arbitrarily chosen byte (?)

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
$bytes[3] = $values['number_of_item3'] * 16
          + $values['unknown'] * 64;

$crazy = ($bytes[0] ^ $bytes[1] ^ $bytes[2] ^ (($bytes[0] ^ $bytes[1]) >> 4)) & 15;
$bytes[3] += $crazy;
$bytes[4] = $values['level'];

$levelnumber = $bytes[0] ^ $bytes[1] ^ $bytes[4];
$bytes[4] ^= $bytes[0] ^ $bytes[1];

print "We got A = $levelnumber\n";
$bytes[2] ^= $table1[1 + $levelnumber*2 + 0];
$bytes[3] ^= $table1[1 + $levelnumber*2 + 1];
for($n=0; $n<5; ++$n) $bytes[$n] ^= $table0[1 + $n];
$bytes[0] ^= $bytes[5];
$bytes[3] ^= $bytes[5];

/* Convert bytes into symbols */
$p = '';
for($n=0; $n<6; ++$n)
{
  $p .= $charset[$bytes[$n] >> 4];
  $p .= $charset[$bytes[$n] & 0xF];
}

/* Print output */
print "Pass: $p\n";
