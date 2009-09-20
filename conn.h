/**
 * @file conn.h 
 * 
 * Copyright (C) 2008-2009 Arnold Noronha <arnstein87 AT gmail DOT com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __CONN_H__
#define __CONN_H__

#include <list>
#include <glib.h>
#include <libpurple/circbuffer.h>
#include "messagebuffer.h"
#include <string>
#include "PurpleAsyncConnHandler.h" 
#include <set>
#include "RObject.h"
#include "config.h"

namespace rbol {
	class PurpleAsyncConn : public RObject { 
	private:
		/*
		 * After starting a connection with purple_proxy_connect,
		 * there is a time lag before we get connected, and hence
		 * before the got_connected callback is called. In that
		 * time I might have "close"d this connection, from which
		 * point it becomes invalid. After that I cannot even 
		 * trust that this object *exists* in memory. 
		 */

		PurpleProxyConnectData *connection_attempt_data;

		/* buffers */
		PurpleCircBuffer *txbuf;
		MessageBuffer awaiting;

		/* handlers */
		guint tx_handler;
		guint rx_handler;
		
		/* the purple connection socket */
		int fd;

		/* store some information for the parent */
		int parse_mode;

		/* super handler */
		int handler_id;
	public:
		int getParseMode() { 
			return parse_mode;
		}
		void setParseMode(int a) {
			parse_mode = a;
		}
		PurpleAsyncConn(int conn_id, int pm);
		bool establish_connection(std::string ip, gint32 port);

		virtual ~PurpleAsyncConn();
		
		void write(const void* data, int len);
		void write(std::string s);
		/* internal callbacks */
		void write_cb();
		void read_cb(int source);

		void close();
		void got_connected_cb(int source, const gchar*);
	};
}

#endif
