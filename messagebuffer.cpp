/**
 * @file messagebuffer.cpp 
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


#include "messagebuffer.h" 
#include "string"
#include "util.h"

using namespace std;
using namespace rbol;
MessageBuffer::MessageBuffer(string s) { 
	this->str = s;
	this->offset = 0;
	this->err = 0;
}

int 
MessageBuffer::left() { 
	return str.length() - offset;
}


MessageBuffer::~MessageBuffer() { 
}

gint32 
MessageBuffer::readInt32() {
	if (this->left() < 4) { 
		throw MessageBufferOverflowException();
	}
	int i = *((gint32*) (this->str.c_str()+this->offset));
	this->offset += 4;
	return i;
}

gint32 
MessageBuffer::peekInt32() {
	if (this->left() < 4) { 
		throw MessageBufferOverflowException();
	}
	int i = *((gint32*) (this->str.c_str()+this->offset));
	return i;
}


void 
MessageBuffer::seek(int len) { 
	if (size_t(this->offset) + len >= this->str.length()) 
		throw MessageBufferOverflowException();
	this->offset += len;
}

char 
MessageBuffer::readByte() {
	if (size_t(this->offset) >= this->str.length()) { 
		throw MessageBufferOverflowException();
	}
	return str[this->offset++];
}

string
MessageBuffer::readBytes(int len) { 
	if (this->left() < len) { 
		throw MessageBufferOverflowException();
	}
	string s = str.substr(this->offset, len);
	this->offset += len;
	return s;
}

MessageBuffer
MessageBuffer::readMessageBuffer(int len){ 
	return MessageBuffer(readBytes(len));
}

string
MessageBuffer::readString() { 
	int len = readInt32();
	return readBytes(len);
}

string
MessageBuffer::readStringn(int len) { 
	if (len == 0) return "";
	return readBytes(len);
}

gint16 
MessageBuffer::readShort() { 
	gint16 ret = readByte();
	ret = (((gint16)readByte()) << 8) | ret;
	return ret;
}

gboolean 
MessageBuffer::isEnd() { 
	return (size_t(offset) >= str.length());
}

gboolean 
MessageBuffer::getLength() { 
	return (str.length());
}

void 
MessageBuffer::reset() {
	offset = 0;
	err = 0;
}

void MessageBuffer::push(string s) { 
	str += s;
}


int MessageBuffer::readLEInt() { 
	char a = readByte ();
	char b = readByte();
	char c = readByte();
	char d = readByte();
	return bytesToLEInt(a, b, c, d);
}
