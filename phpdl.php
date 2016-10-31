<?php
header('Content-type: text/plain');
readfile( preg_replace('/[^-a-z0-9]/', '', $_GET['f']) . '.php' );

