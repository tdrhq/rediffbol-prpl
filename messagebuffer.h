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
		
		MessageBuffer() { 
			offset = err = 0 ;
		}
		gint32 readInt32();
		gint32 readInt() {
			return readInt32 () ;
		}
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
		
		int getLength() ; 
		
		void reset() ;
		
		void push(std::string s) ;

		MessageBuffer tail() { 
			return readMessageBuffer(left()) ;
		}
	} ;
}

#endif
