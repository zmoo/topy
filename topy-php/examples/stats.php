<?php
/*
 *  Copyright (C) 2008 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Topy.
 *
 *   Topy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Topy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Topy; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

if ($argc != 3) {
	die("USAGE: <address> <port>\n");
}

$addr = $argv[1];
$port = $argv[2];


$link = topy_pconnect($addr, $port);

if (!$link) {
	die("Can not connect to Topy server!\n");
}

$t = 3;
$stats = topy_query($link, "stats");

while (true) {
	$before = $cmds;
	sleep($t);
	$stats = topy_query($link, "stats");
	$cmds = (isset($stats["commands"])) ? $stats["commands"] : array();
//	print_r($cmds);

	$freq = array();
	foreach ($cmds as $cmd => $value) {
		if (isset($before[$cmd])) {
				$freq[$cmd] = ($cmds[$cmd] - $before[$cmd]) / $t;
		}
	}

	echo "\n".date("h:m:s");

	foreach($freq as $key => $value) {
		if ($value != 0)
			echo "\t$key: ".sprintf("%01.1f", $value);
	}
}
?>
