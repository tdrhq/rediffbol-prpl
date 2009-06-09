/**
 * @file conn.cpp 
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



#include "request.h" 
#include "conn.h"
#include <glib.h>
#include <errno.h>
#include <debug.h>
#include <cassert>
using namespace rbol;
using namespace std;

#include <connection.h>

namespace rbol { 
	void hex_dump(const string, const string);
}


static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond);

PurpleAsyncConn::PurpleAsyncConn(PurpleAsyncConnHandler *_handler,
				 int pm) :awaiting("") 
{ 
	handler = _handler;
	handlerId = _handler->getId ();
	assert (gpointer(handler) == RObject::getObjectById (handlerId));

	txbuf = purple_circ_buffer_new(0);
	parse_mode = pm;
	rx_handler = 0;
	tx_handler = 0;
	connection_attempt_data = NULL;

	purple_debug_info("rbol", "Connection object created: %p\n", this);

}

bool PurpleAsyncConn::isHandlerExisting ()
{
	handler = (PurpleAsyncConnHandler*) RObject::getObjectById (handlerId);
	if (!handler) return false;
}

PurpleAsyncConn::~PurpleAsyncConn() { 
	this->close();
}

void 
PurpleAsyncConn::got_connected_cb(gint source, const gchar* error) { 
	isHandlerExisting ();

	connection_attempt_data = NULL;
	purple_debug_info("rbol", "got an fd of %d\n", source);
	if (isInvalid()) { 
		purple_debug_info("rbol", "oops, connection is no longer valid!\n");
		return;
	}


	if (source < 0) { 
		if (handler) handler->connectionError(error, this);
		close();

		setInvalid();
		purple_debug_error("rbol", "connection error '%s'\n", 
			     error);
		return;
	}

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "got connected\n");
	fd = source;
	
	/* so now this callback need not take care of connection issues */
	assert(rx_handler == 0);
	rx_handler = purple_input_add(fd, PURPLE_INPUT_READ, 
				      conn_read_cb, (gpointer)this->getId());
	if (handler) handler->gotConnected();
}



static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message);

bool 
PurpleAsyncConn::establish_connection(
	const string ip, 
	const int port) {
 
	/* try to ensure this is called only once */
	assert(rx_handler == 0 and tx_handler == 0
		and connection_attempt_data == NULL);

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "establishing connection\n");
	fd = -1;
	
	connection_attempt_data = purple_proxy_connect(NULL, handler->getProxyAccount(), 
						       ip.c_str(), 
						       port, conn_got_connected, (gpointer) this->getId());
	if (connection_attempt_data == NULL) {
		purple_debug(PURPLE_DEBUG_ERROR, "rbol", 
			     "connection failed\n");
		return false;
	}
	return true;
}


static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) { 

	PurpleAsyncConn *conn = (PurpleAsyncConn*)RObject::getObjectById (GPOINTER_TO_UINT(data));
	if (!conn) return;

	conn->got_connected_cb(source, error_message);
}



/* shutdowns the connection, waits till all packets are sent. 
 * TODO: fix  but returns immediately */
void 
PurpleAsyncConn::close() { 

	purple_debug_info("rbol", "Closing connection object %p\n", this);
	setInvalid();

	if (connection_attempt_data) {
		purple_proxy_connect_cancel(connection_attempt_data);
		connection_attempt_data = NULL;
	}

	int _tmp;

	if (fd > 0 and purple_input_get_error(fd, &_tmp) == 0) {
		if (::close(fd) < 0) { 
			purple_debug_info("rbol", "Failed to close fd with error %d\n", errno);
		}
		fd = 0;
	}

	if (txbuf) {
		purple_circ_buffer_destroy(txbuf);
		txbuf = NULL;
	}
	if (tx_handler) { 
		purple_input_remove(tx_handler);
		tx_handler = 0;
	}
	if (rx_handler) { 
		purple_input_remove(rx_handler);
		rx_handler = 0;
	}

	handler = NULL;
	
	
	
}

static void conn_write_cb(gpointer data, gint source, 
			   PurpleInputCondition cond);



void  
PurpleAsyncConn::write(const void* data, 
	int datalen) { 
	int written;
	isHandlerExisting ();
	if (isInvalid()) { 
		purple_debug_info("rbol", "Writing to a invalid conn?\n");
		return;
	}
	if (!tx_handler) 
		written = ::write(fd, data, datalen);
	else { 
		written = -1;
		errno = EAGAIN;
	}

	if (written < 0 && errno == EAGAIN) 
		written = 0;
	else if (written <= 0) { 
		written = 0;
		if (handler) handler->readError(this);
		close();
		return;
	}

	if (written < datalen) { 
		purple_circ_buffer_append(txbuf, ((char*)data)+written,
					  datalen-written);

		if (! tx_handler) 
			tx_handler = purple_input_add(fd, 
				  PURPLE_INPUT_WRITE, 
				  conn_write_cb, 
						      (gpointer) this->getId());


	}

	if (handler) handler->writeCallback(this);
}


void 
PurpleAsyncConn::write_cb() { 
	isHandlerExisting ();
	int writelen , ret;
	
	writelen = purple_circ_buffer_get_max_read(txbuf);
	if (writelen == 0) { 
		purple_input_remove(tx_handler);
		tx_handler = 0;
		return;
	}

	ret = ::write(fd, txbuf->outptr, writelen);

	if (ret < 0 && errno == EAGAIN) {
		return;
	}
	else if (ret <= 0) {
		
		purple_debug_error("rbol", "writing failed, going into bad state\n");
		if (handler) handler->readError(this);
		close();
		return;
	}

	purple_circ_buffer_mark_read(txbuf, ret);


}


static void conn_write_cb(gpointer data, gint source, 
			   PurpleInputCondition cond) { 
	PurpleAsyncConn* conn = (PurpleAsyncConn*) RObject::getObjectById (GPOINTER_TO_UINT (data));
	if (!conn) return;

	conn->write_cb();
}

#include <iostream>
void
PurpleAsyncConn::read_cb(int source) { 
	dump();

	purple_debug_info("rbol", "got a read on %p with fd=%d\n", this, source);
	char buf[1024];
	int len = read(source, buf, sizeof(buf));
	if (isInvalid()) { 
		purple_debug_info("rbol", "Orphaned connection callback\n");
		return;
	}

	if(len < 0) {
		if (errno == EAGAIN)
			return;/* safe */
		
		/* todo: register a connection error */
		if (handler) handler->readError(this);
		close();
		return;
	} else if (len == 0) { 
		/* todo: server closed conenction */ 
		
		if (handler) handler->closeCallback(this);
		close();

		purple_debug(PURPLE_DEBUG_ERROR, "rbol",
			     "pathetic, the server has closed the connection\n");
		return;
	}
	
	awaiting.push(string(buf, buf+len));

	if (handler) handler->readCallback(awaiting, this);
}

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond) {
	PurpleAsyncConn *conn = (PurpleAsyncConn*) RObject::getObjectById (GPOINTER_TO_UINT(data));

	if (!conn) return;

	int _tmp;
	if (source < 0 or purple_input_get_error(source, &_tmp) != 0) { 
		purple_debug_info("rbol", "Callback called on invalid handle\n");
		return;

	}
	conn->read_cb(source);
}

void PurpleAsyncConn::write(string s) { 
	write(s.data(), s.length());
}
