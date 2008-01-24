
#include "messagebuffer.h" 
#include "string"

using namespace std; 
using namespace rbol ;
MessageBuffer::MessageBuffer(string s) { 
	this->str = s;
	this->offset = 0;
	this->err = 0 ; 
}

int 
MessageBuffer::left() { 
	return str.length() - offset ; 
}


MessageBuffer::~MessageBuffer() { 
}

gint32 
MessageBuffer::readInt32() {
	if ( this->left() < 4 ) { 
		throw MessageBufferOverflowException() ;
	}
	int i = *((gint32*) (this->str.c_str()+this->offset));
	this->offset += 4; 
	return i ;
}

void 
MessageBuffer::seek(int len) { 
	if ( this->offset + len >= this->str.length()) 
		throw MessageBufferOverflowException() ;
	this->offset += len ;
}

char 
MessageBuffer::readByte() {
	if ( this->offset >= this->str.length()) { 
		throw MessageBufferOverflowException() ;
	}
	return str[this->offset++] ;
}

string
MessageBuffer::readBytes(int len) { 
	int i ;
	if ( this->left() < len ) { 
		throw MessageBufferOverflowException() ;
	}
	string s ; 
	for(i = 0 ; i < len; i++) 
		s+= readByte() ;
	return s;
}

MessageBuffer
MessageBuffer::readMessageBuffer(int len){ 
	return MessageBuffer(readBytes(len)) ;
}

string
MessageBuffer::readString() { 
	int len = readInt32() ;
	return readBytes(len) ;
}

string
MessageBuffer::readStringn(int len) { 
	if ( len == 0 ) return "" ;
	return readBytes(len) ;
}

gint16 
MessageBuffer::readShort() { 
	gint16 ret = readByte() ;
	ret = (((gint16)readByte()) << 8 ) | ret ;
	return ret ;
}

gboolean 
MessageBuffer::isEnd() { 
	return ( offset >= str.length()) ;
}

gboolean 
MessageBuffer::getLength() { 
	return (str.length()) ;
}

void 
MessageBuffer::reset() {
	offset = 0 ;
	err = 0 ;
}
