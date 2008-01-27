#ifndef __packet_handler_h__
#define __packet_handler_h__

#include <map>
#include<string>
#include "messagebuffer.h"
#include "rediffbol.h"

namespace rbol {
	
	typedef void (*PacketHandlerFn) (MessageBuffer &, RediffBolConn*) ;
	extern std::map <std::string, PacketHandlerFn> packet_handler ;

#define SET_PACKET_HANDLER(a,fn) rbol::packet_handler[a] = fn ;
	
	extern int setPacketHandler(std::string cmd, PacketHandlerFn fn) ;
	extern int setPacketHandler(int subtype, PacketHandlerFn fn) ;
	PacketHandlerFn  getPacketHandler(int subtype) ;
	PacketHandlerFn  getPacketHandler(std::string cmd) ;

#define INIT_PACKET_HANDLER(a, name) { void name(rbol::MessageBuffer&, RediffBolConn*); \
		rbol::setPacketHandler(a, name) ; }
}

#endif
