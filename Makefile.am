EXTRA_DIST = README Makefile.mingw

pkgdir = $(libdir)/purple-$(PURPLE_MAJOR_VERSION)

REDIFFBOLSOURCES = rediffbol.c  

AM_CFLAGS = $(st) -I/usr/include/libxml2  

librediffbol_la_LDFLAGS = -module -avoid-version

# nullprpl isn't built by default. when it is built, it's dynamically linked.
st =
pkg_LTLIBRARIES   = librediffbol.la
librediffbol_la_SOURCES = $(REDIFFBOLSOURCES)
librediffbol_la_LIBADD  = $(GLIB_LIBS) -lcurl -lgssapi_krb5 -lxml2

AM_CPPFLAGS = \
	-I$(top_srcdir)/libpurple \
	-I$(top_builddir)/libpurple \
	$(GLIB_CFLAGS) \
	$(DEBUG_CFLAGS) -I/usr/include/libxml2 

