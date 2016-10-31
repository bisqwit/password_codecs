<?php

$s = file_get_contents('php://stdin');
$text = $argv[1];

for($step=1; $step<8; ++$step)
  for($offset=0; $offset<256; ++$offset)
  {
    $txt = Array();
    for($a=0; $a<strlen($text); ++$a)
      $txt[] = chr( ord($text[$a]) + $offset );
    
    $b = strlen($s) - $step * count($txt);
    for($a=0; $a<$b; ++$a)
    {
      foreach($txt as $d => $c)
        if($s[$a + $d*$step] != $c)
          continue 2;
      printf('Found <%1$s> at position %2$d ($%2$X), '.
             'with offset %3$d ($%3$X), step %4$d ($%4$X)'. "\n",
             $text, $a, $offset, $step);
    }
  }
