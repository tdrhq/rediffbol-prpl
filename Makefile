#
# rediffbol-prpl Makefile
#
# Copyright 2007 Arnold Noronha <arnstein87 AT sourceforge DOT net>
#
# From existing code for pidgin-LaTeX by:
# Copyright 2004 Edouard Geuten <thegrima AT altern DOT org>
#
# Heavily inspired and copied from :
# Gaim Extended Preferences Plugin Main Makefile
# Copyright 2004 Kevin Stange <extprefs@simguy.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CC := g++
LIBTOOL := libtool
SRC_FILES := rediffbol.cpp messagebuffer.cpp rediffbol_init.cpp conn.cpp encode.cpp util.cpp FontParser.cpp 
HEADERS := rediffbol.h messagebuffer.h conn.h encode.h util.h FontParser.h

ifeq ($(PREFIX),)
  LIB_INSTALL_DIR = $(HOME)/.purple/plugins
else
  LIB_INSTALL_DIR = $(PREFIX)/lib/pidgin
endif


PIDGIN_CFLAGS  = $(shell pkg-config pidgin --cflags)
GTK_CFLAGS   = $(shell pkg-config gtk+-2.0 --cflags)
PIDGIN_LIBS    = $(shell pkg-config pidgin --libs)
GTK_LIBS     = $(shell pkg-config gtk+-2.0 --libs)
PIDGIN_LIBDIR  = $(shell pkg-config --variable=libdir pidgin)/pidgin

all: librediffbol.so

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	cp librediffbol.so $(LIB_INSTALL_DIR)

librediffbol.so: $(SRC_FILES) $(HEADERS) 
	$(CC) --debug -shared -o librediffbol.so $(PIDGIN_CFLAGS) $(PIDGIN_LIBS) $(CFLAGS)  $(SRC_FILES) -fPIC 

clean:
	rm -rf *.o *.c~ *.h~ *.so *.la .libs *.cpp~
