<?php

$ciph      = 'BAIFJCGEDH';
$num_codes = 0;
$codes     = Array();
for($aa=0; $aa<10; ++$aa)
for($bb=0; $bb<10; ++$bb)
for($cc=0; $cc<10; ++$cc)
for($dd=0; $dd<10; ++$dd)
for($ee=0; $ee<10; ++$ee)
{
  $e = $ee ^ $dd;
  $d = $dd ^ $cc;
  $c = $cc ^ $bb;
  $b = $bb ^ $aa;
  $a = $aa;

  if( ($e | (($c & 6)*4)) == ($a + $b + ($c & 1) + $d) )
  {
    $code = sprintf('level %3d super=%d unknown=%d', ($c & 1) | ($a * 16) | ($b * 2), $d>>2, $d&3);
    $codes[$code][] = $ciph[$aa] . $ciph[$bb] . $ciph[$cc] . $ciph[$dd] . $ciph[$ee];
    ++$num_codes;
  }
}
ksort($codes);
foreach($codes as $code => $opts)
  print "$code : " . join(' ', $opts) . "\n";

printf("%d valid codes (out of 100000)\n", $num_codes);

#$reverse = join('', array_map(function($n)use($ciph)
#           { return chr(65 + strpos($ciph, $n)); }, range('A','J')));

