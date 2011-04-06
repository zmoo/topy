<?php

while (true) {
	$link = topy_pconnect("127.0.0.1", 8888);
	if ($link) {
		var_dump(topy_query($link, "stats"));
	}
	else {
		echo "Could not connect\n";
	}
	sleep(1);
}
