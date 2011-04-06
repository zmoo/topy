PHP_ARG_ENABLE(topy, whether to enable Topy support,
[ --enable-topy   Enable Topy support])

if test "$PHP_TOPY" = "yes"; then
	PHP_NEW_EXTENSION(topy, php_topy.c, $ext_shared,,)
fi 
