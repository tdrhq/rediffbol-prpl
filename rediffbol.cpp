
#include "rediffbol.h"
#include <typeinfo>
#include "GkGetLoginServersResponse.h"
#include "request.h" 
#include "sstream" 
#include <vector>
#include <set>
#include "encode.h"
#include "util.h"

using namespace std;
using namespace rbol ; 


//void RediffBolConn::connectToGK() ;
//void RediffBolConn::connectToCS() ;
void hex_dump (const string a,const string message) { 
	if ( a.size() == 0 ) return ;
	gchar* hex_dump = purple_str_binary_to_ascii
		((const unsigned char*) a.data(), 
		 a.length()) ;
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "Hex Dump[%s,%d]: %s\n",
		     message.c_str(), a.length(),
		     hex_dump) ;
	g_free(hex_dump) ; 
}
void RediffBolConn::startLogin() { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "starting login\n");

	connection = new PurpleAsyncConn(this, 
					 "203.199.83.62",
					 1863,
					 1) ;

}

#define write_int(s, a) { int t = (a) ; s.write((char*) &t, sizeof(t)) ; }
#define write_short(s,a ) { short t = (a) ; s.write((char*) &t, sizeof(t)) ; }
void RediffBolConn::connectToGK() { 
	/* construct a request */
	ostringstream s ;

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "connecting to gatekeeper\n");
	
	int capsize = strlen(CAP_DIR)+strlen(CAP_HTTP_CONNECT)
		+ strlen(CAP_HTTP) ;
	int uasize = userAgent.length() ;
	int size = capsize + uasize + 32 + strlen(GKRequestHeader) + 
		strlen(GKCmdGetLoginServers) + 4*3 ;

	size -= 4 ; 
	gint32 zero = 0 ;
	write_int(s, size) ;
	write_int(s, 0) ;
	write_int(s, 0);

	gint32 len = strlen(GKRequestHeader) ;
	write_int(s, len);
	s.write(GKRequestHeader, len) ;

	write_int(s, strlen(GKCmdGetLoginServers));

	s.write(GKCmdGetLoginServers, strlen(GKCmdGetLoginServers));

	size = capsize + 3*4 + uasize + 8 ;

	write_int(s, size);

	write_int(s, uasize) ;

	s.write(userAgent.c_str(), uasize) ;


	write_int(s, 3) ;

	size = strlen(CAP_DIR) ;
	write_int(s, size) ;
	s.write(CAP_DIR, size) ;

	write_int(s, strlen(CAP_HTTP_CONNECT) );

	s.write(CAP_HTTP_CONNECT, strlen(CAP_HTTP_CONNECT));

	write_int(s, strlen(CAP_HTTP) );

	s.write(CAP_HTTP, strlen(CAP_HTTP));
	printf("%d\n", s.str().length()) ;
	string t = s.str() ;

	hex_dump(t, "Gk message") ;
	printf("here2\n");

	connection->write((const void*)s.str().c_str(), s.str().size()) ;
	printf("here\n");

}
void RediffBolConn::parseTextMessage( MessageBuffer &buffer) { 
	int payloadsize = buffer.readInt() ;
	buffer = buffer.readMessageBuffer(payloadsize) ;

	string senderstr = buffer.readString() ;
	string recipient  = buffer.readString() ;

	int subpayloadsize = buffer.readInt();

	buffer = buffer.readMessageBuffer(subpayloadsize) ;
	
	string final_message ; 


	while ( !buffer.isEnd() ) { 
		int len = buffer.readLEInt() ;

		if ( len == 0 ) { 
			int fontstrlen = buffer.readLEInt() ; 
			string  font = buffer.readStringn(fontstrlen) ;
			string fontinfo = buffer.readStringn(13);
			
			int messagelen = buffer.readLEInt() ;
			string message = buffer.readStringn(messagelen) ;
			hex_dump(message, "message before ASCII encoding\n") ;
			message = encode( message, "UTF-16BE", "US-ASCII") ;
			final_message += message ; 

		} else if ( len == 1 ) { 
			int smileycode = buffer.readLEInt() ;
			int smileylen = buffer.readLEInt() ;
			string smiley = buffer.readStringn(smileylen) ;
		} else { 
			purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
				     "Unknown message type.. \n") ;
			continue ; 
		}
	}

	purple_debug(PURPLE_DEBUG_INFO, "rbol",
		     "got message %s\n", final_message.c_str()) ;
	serv_got_im(purple_account_get_connection(account) ,
		    senderstr.c_str() ,final_message.c_str(), 
		    (PurpleMessageFlags) 0, time(NULL) ) ;

	

}
void RediffBolConn::connectToCS() { 

	ostringstream out ; 
	string name = account->username ;
	name += "@rediffmail.com" ;
	string pass = account->password ; 

	int credsize = name.length() + pass.length() ; 
	int size = 58 + credsize + strlen(CSRequestHeader) 
		+strlen(CSCmdSignIn) ;


	write_int(out, size-4) ;

	write_int(out, 0) ;
	write_int(out, 0) ;
	
	write_int(out, strlen(CSRequestHeader)) ;
	out.write(CSRequestHeader, strlen(CSRequestHeader)) ;

	write_int(out, strlen(CSCmdSignIn)) ;
	out.write(CSCmdSignIn, strlen(CSCmdSignIn)) ;

	size = credsize + 34 ;
	write_int(out, size);


	write_int(out, name.length());
	out.write(name.c_str(), name.length()) ;


	write_int(out, pass.length()) ;
	out.write(pass.c_str(), pass.length()) ;

	/* magic string, even Binu Paul doesn't know :) */
	char magic[] = {00, 00, 00, 00, 00, 00, 03,  00, 00, 00, 33, 31, 35} ;

	out.write(magic, sizeof(magic)) ;

	write_int(out, 9);
	out.write("Winver:48", 9) ;
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "CS packet size: %d\n",
		     out.str().length()) ;

	hex_dump(out.str(), string("Sending CS packet")) ;
	connection->write(out.str().c_str(), out.str().length()) ;
}
		
void RediffBolConn::got_connected_cb() { 
	/* what's my state? */
	if ( connection->getParseMode() ) 
		connectToGK() ;
	else connectToCS() ;

}

void RediffBolConn::parseResponse(MessageBuffer &buffer) try { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "got a response\n") ;


	if ( buffer.peekInt32() + 4 > buffer.left()) return ;
	
	int size = buffer.readInt32() ;
	MessageBuffer buf = buffer.readMessageBuffer(size) ;
	hex_dump(buf.peek(), "response") ;
	buffer = buffer.tail () ;

	/* try and parse to see what we've got */
	if ( connection->getParseMode()) 
		parseGkResponse(buf) ;
	else 
		parseCSResponse(buf);

	
}

catch (MessageBufferOverflowException e ) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "overflow exception %d\n", 
		     buffer.getLength() );
	buffer.reset() ;
	return ;
  }


void RediffBolConn::parseGkResponse(MessageBuffer &buffer) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "got a gk response %d\n", buffer.left());

	int type = buffer.readInt32() ; 
        int subtype = buffer.readInt32() ;
        int payloadsize = buffer.readInt32() ; 
        

        int numentries = buffer.readInt32();

	purple_debug(PURPLE_DEBUG_INFO, "rbol",
		     "initial parameters %d %d %d %d\n", type, subtype, 
		     payloadsize, numentries) ;
	map<string, string> cap ; 

        for(int i = 0 ; i < numentries ; i++) { 
                int subentries = buffer.readInt32() ; 
                for(int j = 0 ; j < subentries; j++ ){ 
                        string cp = buffer.readString() ;
                        string val = buffer.readString() ;
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "cap=%s val=%s\n",
				     cp.c_str(), val.c_str()) ;
                        cap[cp] += val ;
                }
        }

	string list = cap[CAP_DIR] ;
        for(int i = 0 ; i < list.size() ; i++ )
                if ( list[i] == ',' ) list[i] = ' ' ;

        istringstream iss(list) ;
        
        vector<string> res ; 
        string s ; 
        while( iss >> s ) { 
                res.push_back(s) ;
        } 

	
	srand(time(NULL));
	//string ip = res[ rand() % res.size() ] ;
	string ip = res[0] ;

	//connection->unref() ;
	delete connection ;
	connection = NULL ;
	
	int f = ip.find(":") ;
	int port = atoi(ip.substr(f+1).c_str()) ;
	ip = ip.substr(0, f) ;

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "connecting to chatserver[%s,%d]\n", ip.c_str(), port);
	connection = new PurpleAsyncConn(this, 
					 ip, port, 0) ;

}

gboolean keep_alive_timer(gpointer data) { 
	RediffBolConn *conn = (RediffBolConn*) data ;
	conn->sendKeepAlive() ;
	return true ;
}

void RediffBolConn::parseCSLoginResponse(MessageBuffer buffer) { 
	map<string, string> optionsmap ; 
	vector<string> roster ; 
	map<string, vector<string> > groups ;
	int payloadsize = buffer.readInt() ; 
	assert(buffer.left() == payloadsize ) ;
	
	int errorcode = buffer.readInt() ;

	if ( errorcode != 0 ) { 
		char err[100];
		sprintf(err, "CS Login error %d\n", errorcode) ;
		setStateNetworkError(err) ;
		return ;
	}

	string names [3] ;

	for(int i = 0 ; i < 3; i++) 
		names[i] = buffer.readString() ; 

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Got names %s %s %s\n",
		     names[0].c_str() ,
		     names[1].c_str() ,
		     names[2].c_str()) ;

	int code1 = buffer.readInt () ;
	int code2 = buffer.readInt () ;

	int numprops = buffer.readInt() ;
	
	for(int i = 0 ; i < numprops; i++) { 
		string propname = buffer.readString() ;
		string value = buffer.readString(); 
		optionsmap[propname] = value ;
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "%s = %s\n",
			     propname.c_str(), value.c_str()) ;
	}

	//read roster entries (buddy list)
	int rosternum = buffer.readInt() ;

	for(int i = 0 ; i < rosternum; i++) { 
		string name = buffer.readString() ;
		roster.push_back(name) ;
		buffer.readInt() ; /* what integer does this read?? */
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "Got contact %s\n",
			     name.c_str()) ;

	}

	buffer.readInt() ; 

	//read blocked entries 
	set<string> blocked ; 
	int blockednum = buffer.readInt() ; 
	for(int i = 0 ; i <  blockednum ; i++) { 

		string name = buffer.readString() ;
		blocked.insert(name) ;
		buffer.readInt() ;
	}

	int allowednum = buffer.readInt() ;
	for(int i = 0 ; i < allowednum; i++) {
		string name = buffer.readString() ; 
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "allowed --> %s\n", 
			     name.c_str()) ;
		buffer.readInt() ;
	}

	// ?? 
	buffer.seek(6) ;
	
	// session and visible IP
	session = buffer.readString() ;
	string visibleIP = buffer.readString() ;

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Session: %s IP:%s\n", 
		     session.c_str(), visibleIP.c_str()) ;

	int code3 = buffer.readInt() ;
	char code4 = buffer.readByte() ;

	string unknown = buffer.readString() ;

	string code6 = buffer.readString() ;
	string code7 = buffer.readString() ;

	int numgroups = buffer.readInt() ;
        purple_connection_update_progress(account->gc, ("Connected"),
                                          1,   /* which connection step this is */
                                          2);  /* total number of steps */
	

	purple_connection_set_state(account->gc, PURPLE_CONNECTED);
	for(int i = 0 ; i < numgroups ; i++) { 
		string groupname1 = buffer.readString() ;
		string groupname2 = buffer.readString() ;

		int groupentries = buffer.readInt() ;

		purple_debug(PURPLE_DEBUG_INFO , "rbol", "Group %s %s %d\n" ,
			     groupname1.c_str(), groupname2.c_str(),
			     groupentries) ;
		vector<string> members ;
		for(int j = 0 ; j < groupentries ; j ++) {
			string membername = buffer.readString() ;
			members.push_back(membername) ;
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "%s is member of %s\n",
				     membername.c_str(), groupname2.c_str());

			/* add this buddy if required */
			PurpleBuddy *buddy = purple_find_buddy(account, 
						  membername.c_str()) ;
			if ( !buddy ) { 
				buddy = purple_buddy_new(account, 
							 membername.c_str(),
							 membername.c_str());
				PurpleGroup *group = purple_find_group(
					groupname2.c_str()); 
				if ( ! group ) 
					group = purple_group_new(groupname2.c_str()) ; 
				purple_blist_add_buddy(buddy, 
				      purple_buddy_get_contact(buddy),
				      group, NULL);
			}
		}
		groups[groupname2] = members ; 
	}

	string id = buffer.readString() ;

	sendKeepAlive() ;

	purple_timeout_add_seconds(30, keep_alive_timer, this) ;
}
string getStatusFromID(int status) { 
	if ( status == 2 ) { 
		return "away" ;
	} else if ( status == 3 ) 
		return "busy" ; 
	else if ( status == 1 ) 
		return  "online" ; 
	else
		return "offline" ;
	
}
void RediffBolConn::parseCSContacts(MessageBuffer &buffer) { 
	int payloadsize = buffer.readInt() ;
	int numcontacts = buffer.readInt() ; 
	map<string, vector<string> > contacts; 
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "#contacts = %d\n", 
		     numcontacts) ;

	for(int i = 0 ; i < numcontacts ; i++) { 
		string name1 = buffer.readString() ;
		string name2 = buffer.readString() ; 
		string name3 = buffer.readString() ; 
		
		purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
			     "got contact [%s] [%s] [%s]\n", 
			     name1.c_str(), name2.c_str(), name3.c_str()) ;


		int code1 = buffer.readInt() ;
		int status = buffer.readInt() ; 
		string msg = buffer.readString() ;
		int code3 = buffer.readInt() ;
		string version = buffer.readString() ;

		string sstatus = getStatusFromID(status) ; 

		purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
			     "setting status  [%s] [%s] [%s]\n", 
			     name1.c_str(), sstatus.c_str(), msg.c_str()) ;
		
		purple_prpl_got_user_status(account, name1.c_str(), 
					    sstatus.c_str(),
					    NULL,
					    NULL);
					    
					    
		/* do the purple work */
	}
	
	string id = buffer.readString() ;
	
}

void RediffBolConn::parseContactStatusChange(MessageBuffer &buffer) { 
	int payloadsize = buffer.readInt() ;
	buffer = buffer.readMessageBuffer(payloadsize) ;

	string name1 = buffer.readString() ;
	int status = buffer.readInt() ; 
	
	string msg = buffer.readString() ;
	
	int code3 = buffer.readInt() ;
	string version = buffer.readString() ;


	string sstatus = getStatusFromID(status) ;

	purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
		     "setting status  [%s] [%s] [%s]\n", 
		     name1.c_str(), sstatus.c_str(), msg.c_str()) ;
	
	purple_prpl_got_user_status(account, name1.c_str(), 
				    sstatus.c_str(),
				    NULL,
				    NULL);
	
}
void RediffBolConn::parseCSResponse(MessageBuffer &buffer) { 
	/* gah... boring enumeration.. but at the same time
	   if I don't know a packet I should atleast read the 
	   entire packets contents and remove it from the buffer 
	*/

	int type = buffer.readInt() ; 
	
	string header = "" ;
	string cmd = "" ;


	if ( type == 1 ) { 
		if ( buffer.getLength() == 12 ) {
			hex_dump(buffer.peek(), "keep alive string") ;
			return ;
		}

		int subtype = buffer.readInt() ;

		if ( subtype == 0 ) { 
			parseCSLoginResponse(buffer) ;
			return ;
		} else if (subtype == 1 ) { 
			/* offline messages */
		} else if ( subtype == 7 ) { 
			/* chat room response */
		} else if ( subtype == 62 ) { 
			/* join chat room response */
		}

		
		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "Got subtype as %d\n", subtype) ;

		int size = buffer.readInt() ;
		string packet = buffer.readStringn (size) ;
		hex_dump(packet, "Unknown Type 1 response from server") ;
		
		return  ; 
	} else if ( type == 3 ) { 
		header = buffer.readString() ;
		cmd = buffer.readString() ;

		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "cmd is %s\n", cmd.c_str()) ;


		if ( cmd ==  "Contacts") { 
			 parseCSContacts(buffer); 
			 return ; 
		} else if ( cmd ==  "NewMailCount") { 
			/* resp = parseNewMailCount */
		} else if ( cmd == "ContactStatusChange" ) { 
			/* ignore? */
		} else if (cmd == "ContactStatusChange2" ) { 
			parseContactStatusChange(buffer) ;
			return ;
		} else if ( cmd == "TextMessage" ) { 
			parseTextMessage(buffer) ;
			return ; 
		}
	     
		

		int len = buffer.readInt() ;
		string dump = buffer.readStringn(len) ;
		
		hex_dump(dump, "Unknown type 3 response") ;
		return ;
	} else if ( type == 2 ) { 
		/* server message */
		int code = buffer.readInt32() ; 
		int size = buffer.readInt32() ; 
		
		string message = buffer.readString() ; 
		string reason = buffer.readString() ; 
		int code2 = buffer.readInt() ; 

		purple_debug(PURPLE_DEBUG_INFO, "rbol",
			     "SERVER MESSAGE: code=%d code2="
			     "%d MESSAGE=%s | REASON=%s\n",
			     code, code2, message.c_str() , reason.c_str() );

		return  ;
	}
				
		

}

void RediffBolConn::setStateNetworkError(string msg) {
	/* can't do anything now */
	purple_debug(PURPLE_DEBUG_INFO, "rbol" , msg.c_str()) ;
} 

/**
 * Sends _unstylized_ messages 
 */

void RediffBolConn::sendMessage(string to, string message) { 
	string fonttype = "Dialog" ; /* confused :( */

	string sender = account->username ;
	sender += "@rediffmail.com" ;

	fonttype = encode_from_iso(fonttype, "UTF-16BE") ;
	message = encode_from_iso(message, "UTF-16BE") ;

	int msgsize = message.length()  ;
	int sendersize = sender.length() ;
	int recipientsize = to.length() ; 
	int fontstrsize = fonttype.length ()  ; 

	int size = 57 + msgsize + sendersize + recipientsize +
		strlen(CSCmdTextMessage) + strlen(CSPeerRequestHeader) 
		+ fontstrsize ; 

	ostringstream out  ; 
	write_int(out, size-4) ;
	write_int(out, 3) ;
	write_int(out, strlen(CSPeerRequestHeader) );
	out.write(CSPeerRequestHeader, strlen(CSPeerRequestHeader)) ;; 

	write_int(out, strlen(CSCmdTextMessage)) ; 
	out.write(CSCmdTextMessage, strlen(CSCmdTextMessage)) ; 
	
	write_int(out, 37+ sendersize + recipientsize + msgsize + 
		  fontstrsize ) ;

	write_int(out, sender.length()) ;
	out.write(sender.data() , sender.length()) ;

	write_int(out, to.length()) ;
	out.write(to.c_str(), to.length()) ;

	write_int(out, 25+fontstrsize + msgsize) ;
	write_int(out, 0) ;
	write_short(out, 0) ;

	out<<intToSWord(fonttype.size()) ;
	out<<fonttype ;

//	write_shortout, fontstrsize) ;
	//out.write(fonttype.data(), fontstrsize) ;

	char fontBytes[] = {
		0x00, 0x78, 0xff, 0xff, 0x05, 0x01, 0x00, 0x32, 0x00,
		0x49, 0x33, 0x33, 0x33, 0x00, 0x00
		} ;

	out.write(fontBytes, sizeof(fontBytes)) ;

	out<<intToSWord(message.size()) ;
	out<<message ;


	hex_dump(out.str(), "sending message") ;
	connection->write(out.str().data(), out.str().length()) ;
}

void RediffBolConn::sendKeepAlive(){
	int size = 24 + strlen(CSRequestHeader) + strlen(CSCmdKeepAlive) ;

	ostringstream out ; 
	write_int(out, size-4) ;
	write_int(out, 0) ;
	write_int(out, keep_alive_counter) ;
	keep_alive_counter++ ;

	write_int(out, strlen(CSRequestHeader)) ;
	out << CSRequestHeader ;

	write_int(out, strlen(CSCmdKeepAlive)) ;
	out<<CSCmdKeepAlive ;

	write_int(out, 0) ;

	connection->write(out.str().c_str(), out.str().length()) ;
}
