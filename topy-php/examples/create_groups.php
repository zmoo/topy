<?php

function create_groups($prefix, $mask_bin, $unit_bin, $flags) {
	$mask = bindec($mask_bin);
	$unit = bindec($unit_bin);

	foreach($flags as $key => $flag) {
		$id = $key * $unit;
		echo "groups add $prefix$flag $id $mask\n";
	}
}

$countries = array("fr", "ru", "it", "nl", "ua");
$locales = array("fr_FR", "en_GB", "it_IT", "nl_NL");
$sex = array("m", "f", "n");
$exclude = array("true", "false");

create_groups("c:", "000001111", "000000001", $countries);
create_groups("l:", "000110000", "000010000", $locales);
create_groups("s:", "011000000", "001000000", $sex);
create_groups("x:", "100000000", "100000000", $exclude);


