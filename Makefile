#
# pidgin-latex Makefile
#
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

CC := gcc
LIBTOOL := libtool

ifeq ($(PREFIX),)
  LIB_INSTALL_DIR = $(HOME)/.purple/plugins
else
  LIB_INSTALL_DIR = $(PREFIX)/lib/pidgin
endif

REDIFFBOL = rediffbol

PIDGIN_CFLAGS  = $(shell pkg-config pidgin --cflags)
GTK_CFLAGS   = $(shell pkg-config gtk+-2.0 --cflags)
PIDGIN_LIBS    = $(shell pkg-config pidgin --libs)
GTK_LIBS     = $(shell pkg-config gtk+-2.0 --libs)
PIDGIN_LIBDIR  = $(shell pkg-config --variable=libdir pidgin)/pidgin
CURL_LIBS = $(shell curl-config --libs) 
CURL_CFLAGS = $(shell curl-config --cflags)
CFLAGS = $(PIDGIN_CFLAGS) $(GTK_CFLAGS) $(CURL_CFLAGS)
LDFLAGS = $(PIDGIN_LIBS) $(CURL_LIBS) $(GTK_LIBS)

all: $(REDIFFBOL).so $(REDIFFBOL).la

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	cp $(REDIFFBOL).so $(LIB_INSTALL_DIR)

$(REDIFFBOL).so: $(REDIFFBOL).o
	$(CC) -shared $(CFLAGS) $< -o $@ $(PIDGIN_LIBS) $(GTK_LIBS) $(CURL_LIBS) -Wl,--export-dynamic -Wl,-soname

$(REDIFFBOL).o:$(REDIFFBOL).c 
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) $(CURL_CFLAGS) -DHAVE_CONFIG_H

$(REDIFFBOL).la:$(REDIFFBOL).o $(REDIFFBOL)
	$(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS)   $(LDFLAGS) $< -o $@

clean:
	rm -rf *.o *.c~ *.h~ *.so *.la .libs
