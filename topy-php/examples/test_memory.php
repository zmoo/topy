<?php

if ($argc != 3) die("USAGE: cmd <addr> <port>\n");

$addr = $argv[1];
$port = $argv[2];

$link = topy_pconnect($addr, $port);

if (!$link) die("Could not connect to server\n");

echo "Creating users...\n";

for ($i = 0; $i < 500000; $i++) {
	echo "\r$i  ";
	$result = topy_query($link, "user *i get");
	if ($result == false) echo ".\n";
//	var_dump($result);
}

echo "Waiting...\n";
for (;;) sleep(1);
