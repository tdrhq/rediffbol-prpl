/**
 * @file messagebuffer.h 
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

#ifndef _MESSAGEBUFFER_H_
#define _MESSAGEBUFFER_H_

#include <glib.h>
#include <string>
#include "config.h"
namespace rbol{ 
	class MessageBufferOverflowException { 
	public:
	};
	
	class MessageBuffer { 
		
	private:
		int offset;
		int err;
		std::string str;
		
	public:
		MessageBuffer (std::string s);
		~MessageBuffer();
		
		MessageBuffer() { 
			offset = err = 0;
		}
		std::string getRawBuffer() {
			return str;
		}
		gint32 readInt32();
		gint32 peekInt32();
		gint32 readLEInt();

		gint32 readInt() {
			return readInt32 ();
		}
		int left();
		void seek(int len);
		
		char readByte();
		
		std::string readBytes(int len);
		
		MessageBuffer
			readMessageBuffer(int len);
		
		std::string readString();
		
		std::string readStringn(int len);
		
		gint16 readShort();
		
		gboolean isEnd();
		
		int getLength();
		
		void reset();
		
		void push(std::string s);

		MessageBuffer tail() { 
			return MessageBuffer(str.substr(offset));
		}
		std::string peek() { 
			return str.substr(offset);
		}
	};
}

#endif
