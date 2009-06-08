/**
 * @file PurpleAsyncConnHandler.h 
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

#ifndef __purple_async_conn_handler__h__
#define __purple_async_conn_handler__h__

#include "conn.h"
#include <account.h>
#include "RObject.h"

namespace rbol { 

	class PurpleAsyncConn;
	class PurpleAsyncConnHandler : public RObject { 
	private:
		virtual void gotConnected()  = 0;
		virtual void readCallback(MessageBuffer &buffer, 
			PurpleAsyncConn*) = 0;
		

		/* this is called before the actual buffer */
		virtual void writeCallback(PurpleAsyncConn*) { 
		}
		
		/* failed to connect */
		virtual void connectionError(std::string message, 
			PurpleAsyncConn*) = 0;

		/* there was a network error in reading data*/
		virtual void readError(PurpleAsyncConn*) = 0;
		
		/* the server closed the connection */
		virtual void closeCallback(PurpleAsyncConn*) = 0;
		
		virtual PurpleAccount* getProxyAccount() = 0;

		friend class PurpleAsyncConn;

	public:
		virtual ~PurpleAsyncConnHandler() { };
		
	};
}

#endif
