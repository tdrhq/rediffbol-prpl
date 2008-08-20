
purpleplugindir = $(HOME)/.purple/plugins

 
allsrc = avatar.cpp  conn.cpp  encode.cpp  FontParser.cpp  messagebuffer.cpp  rediffbol.cpp  rediffbol_init.cpp  util.cpp conn.h  encode.h  FontParser.h  messagebuffer.h  PurpleAsyncConnHandler.h  rediffbol.h  request.h  response.h  util.h RObject.h RObject.cpp rediffbol_chatroom.cpp 


PURPLE_CFLAGS  = $(shell pkg-config purple --cflags)
GLIB_CFLAGS   = $(shell pkg-config glib-2.0 --cflags)
PURPLE_LIBS    = $(shell pkg-config purple --libs)
GLIB_LIBS     = $(shell pkg-config glib-2.0 --libs)

pkgdir  = $(shell pkg-config --variable=libdir purple)/purple-2


EXTRA_DIST = \
	README \
	COPYING

# pkgdir = $(libdir)/purple-$(PURPLE_MAJOR_VERSION)

REDIFFBOLSOURCES = $(allsrc)

AM_CFLAGS = $(st)

librediffbol_la_LDFLAGS = -module -avoid-version $(PURPLE_LIBS) $(GLIB_LIBS)


st =
pkg_LTLIBRARIES     = librediffbol.la
librediffbol_la_SOURCES = $(REDIFFBOLSOURCES)
librediffbol_la_LIBADD  = 


AM_CPPFLAGS = \
	$(PURPLE_CFLAGS) $(GLIB_CFLAGS)


local-install: all-am
	mkdir -p ~/.purple/plugins/
	cp -vf .libs/librediffbol.so ~/.purple/plugins/

local-uninstall:
	rm -f ~/.purple/plugins/librediffbol.*

