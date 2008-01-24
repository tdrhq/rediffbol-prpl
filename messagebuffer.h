#ifndef _MESSAGEBUFFER_H_
#define _MESSAGEBUFFER_H_

#include <glib.h>
#include <string>

namespace rbol{ 
	class MessageBufferOverflowException { 
	public:
	};
	
	class MessageBuffer { 
		
	private:
		int offset ; 
		int err ; 
		std::string str ; 
		
	public:
		MessageBuffer (std::string s) ;
		~MessageBuffer() ;
		
		gint32 readInt32();
		
		int left();
		void seek(int len) ;
		
		char readByte() ;
		
		std::string readBytes( int len) ;
		
		MessageBuffer
			readMessageBuffer( int len);
		
		std::string readString() ;
		
		std::string readStringn(int len) ;
		
		gint16 readShort() ;
		
		gboolean isEnd() ;
		
		gboolean getLength() ; 
		
		void reset() ;
		
		void push(string s) ;
	} ;
}

#endif
