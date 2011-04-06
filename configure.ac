AC_INIT

AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(topy,0.43)

AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES([GLIB], [glib-2.0])

AC_CHECK_LIB(event, event_init)
AC_CHECK_FUNC(event_loopbreak, AC_DEFINE(HAVE_LIBEVENT_LOOPBREAK, 1, [Wheter You have a recent libevent version]))

AC_ARG_ENABLE(strid, [  --enable-strid          enabled string ids], [
	AC_DEFINE(HAVE_STRID, 1, [Enabled string ids])
])

AC_ARG_ENABLE(int64id, [  --enable-int64id        enabled 64 bits ids], [
	AC_DEFINE(HAVE_INT64ID, 1, [Enabled 64 bits ids])
])

AC_OUTPUT(Makefile src/Makefile doc/Makefile)
