
#include "packet_handler.h"
#include <sstream>
using namespace rbol ;
using namespace std; 

namespace rbol { 
	map<string, PacketHandlerFn> packet_handler;
}
int rbol::setPacketHandler(string cmd, PacketHandlerFn fn) { 
	packet_handler[cmd] = fn ;
}
int rbol::setPacketHandler(int subtype, PacketHandlerFn fn) {
	ostringstream s ; 
	s<<subtype; 
	packet_handler[s.str()] = fn ;
}

PacketHandlerFn rbol::getPacketHandler(string s ) {
	assert( packet_handler.count(s) ) ;
	return packet_handler[s] ;
}

PacketHandlerFn rbol::getPacketHandler(int i) { 
	ostringstream s ; 
	s << i ;
	return getPacketHandler(s.str()) ;
}

void MyTest(MessageBuffer &buffer, RediffBolConn* conn) { 

}
