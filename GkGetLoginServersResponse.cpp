
#include "GkGetLoginServersResponse.h" 

using namespace std; 
usine namespace rbol ;

#define CAP_DIR "RBOL/1.2.5"

bool 
GkGetLoginServersResponse::parsePacket(rbol::MessageBuffer &m) { 
	type = m.readInt32() ; 
	subtype = m.readInt32() ;
	payloadsize = m.readInt32() ; 
	
	int numentries = m.readInt32();

	for(int i = 0 ; i < numentries ; i++) { 
		int subentries = m.readInt32() ; 
		for(int j = 0 ; j < subentries; j++ ){ 
			string cp = m.readString() ;
			string val = m.readString() ;
			
			cap[cp] += val ;
		}
	}
}

list<string> 
getDirectConnectionIP() { 
	string list = cap[CAP_DIR] ;
	for(int i = 0 ; i < list.size() ; i++ )
		if ( list[i] == ',' ) list[i] = ' ' ;

	istringstream iss(list) ;
	
	list<string> res ; 
	string s ; 
	while( iss >> s ) { 
		res.push_back(s) ;
	} 
	return res ;
}

void libpurpleProcess(RediffBolConn *rb) { 
	
}
