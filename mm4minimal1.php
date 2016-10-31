<?php
// Binary pattern: Coordinates of dot in each group for none, first, second, both
$groups= [['12','21','32','11', 'TO','BR'],
          ['13','24','33','14', 'PH','DR'],
          ['15','26','35','25', 'RI','DU'],
          ['41','62','51','52', 'SK','DI'],
          ['53','44','43','63', 'WI','BA']];

// Population count indicator locations
$popcnt = ['55','16','22','23','31','34','36','42','54','56','61'];

// Generate a random set of items
$bitmask = rand(0,1023);

// Load the skeleton of the password entry picture
$im = ImageCreateFromPng('http://iki.fi/bisqwit/jkp2/megaman4-model2.png');

// An inefficient but short function for calculating hamming weight of a 10-bit integer
function popcnt($c) { $r=0; for($n=1;$n<1024;$n<<=1)if($c&$n)++$r; return $r; };

// Shorthand functions for placing dots and drawing filled rectangles
$place = function($l)       use(&$im) { ImageCopy($im,$im, 40+16*$l[1], 24+16*$l[0], 136,120, 8,8); };
$rect  = function($x,$y, $w)use(&$im) { ImageFilledRectangle($im, $x,$y, $x+$w-1, $y+7, ImageColorClosest($im,9,9,9)); };

// Place the population count indicator dot
$place($popcnt[popcnt($bitmask)]);

// Process all the five groups
for($nr=0; $nr<5; ++$nr, $bitmask>>=2)
{
  // Place dot
  $place($groups[$nr][$bitmask & 3]);

  // Blot out the names of items we don't have
  if(!($bitmask & 1)) $rect(64+$nr*24, 176, 16);
  if(!($bitmask & 2)) $rect(64+$nr*24, 194, 16);
}

// Remove the model dot from cell F6
$rect(136,120, 8);

// Save output
ImagePng($im, 'megaman4password.png');
