
#include "rediffbol.h"
#include <typeinfo>
#include "request.h" 
#include "sstream" 
#include <vector>
#include <set>
#include "encode.h"
#include "util.h"
#include <connection.h>
#include "FontParser.h"



using namespace std;
using namespace rbol;


//void RediffBolConn::connectToGK();
//void RediffBolConn::connectToCS();

#define SAFE(a) (a?a:"")


RediffBolConn::RediffBolConn(PurpleAccount *acct) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "creating a connection\n");
	
	account  = acct;
	account -> gc -> proto_data = this;
	connection = NULL;
	userAgent = DEFAULT_USERAGENT;
	keep_alive_counter = 0;
	connection_state = 0;
	keep_alive_timer_handle = 0;
}



static string escape_html_entities(string text) {
	char *ret = g_markup_escape_text(text.data(), text.length());
	string _ret = SAFE(ret);
	g_free(ret);
	return _ret;
}

void rbol::hex_dump (const string a,const string message) { 
	if ( a.size() == 0 ) return;
	gchar* hex_dump = purple_str_binary_to_ascii
		((const unsigned char*) a.data(), 
		 a.length());
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "Hex Dump[%s,%d]: %s\n",
		     message.c_str(), (int) a.length(),
		     hex_dump);
	g_free(hex_dump);
}

void RediffBolConn::softDestroy() { 
	if ( isInvalid() ) return;

	if ( keep_alive_timer_handle ) {
		purple_timeout_remove(keep_alive_timer_handle);
		keep_alive_timer_handle = 0;
	}
	setInvalid();
	connection->delRef();
	connection = NULL;
	
}
void RediffBolConn::startLogin() { 

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "starting login\n");

	connection = new PurpleAsyncConn(this, 
					 1);
	if ( !connection->establish_connection(	"gatekeeper.rediff.com",
						1863) ) {
		setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
				     "Unable to initiate connection to GK\n");
		connection = NULL;
	}
	

}

void RediffBolConn::startLoginOver80() 
{
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "trying login over port 80\n");
	connection = new PurpleAsyncConn (this, 2);
	if (!connection->establish_connection ("gatekeeper.rediff.com", 80)) {
		setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
				     "Unable to initiate port 80 connection to GK\n");
		connection = NULL;
	}

}

#define write_int(s, a) { int t = (a);s.write((char*) &t, sizeof(t)) ; }
#define write_short(s,a ) { short t = (a);s.write((char*) &t, sizeof(t)) ; }
void RediffBolConn::connectToGK() { 
	/* construct a request */
	ostringstream s;

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "connecting to gatekeeper\n");
	
	int capsize = strlen(CAP_DIR)+strlen(CAP_HTTP_CONNECT)
		+ strlen(CAP_HTTP);
	int uasize = userAgent.length();
	int size = capsize + uasize + 32 + strlen(GKRequestHeader) + 
		strlen(GKCmdGetLoginServers) + 4*3;

	size -= 4;
	write_int(s, size);
	write_int(s, 0);
	write_int(s, 0);

	gint32 len = strlen(GKRequestHeader);
	write_int(s, len);
	s.write(GKRequestHeader, len);

	write_int(s, strlen(GKCmdGetLoginServers));

	s.write(GKCmdGetLoginServers, strlen(GKCmdGetLoginServers));

	size = capsize + 3*4 + uasize + 8;

	write_int(s, size);

	write_int(s, uasize);

	s.write(userAgent.c_str(), uasize);


	write_int(s, 3);

	size = strlen(CAP_DIR);
	write_int(s, size);
	s.write(CAP_DIR, size);

	write_int(s, strlen(CAP_HTTP_CONNECT) );

	s.write(CAP_HTTP_CONNECT, strlen(CAP_HTTP_CONNECT));

	write_int(s, strlen(CAP_HTTP) );

	s.write(CAP_HTTP, strlen(CAP_HTTP));
	string t = s.str();

	hex_dump(t, "Gk message");

	connection->write((const void*)s.str().data(), s.str().size());
}
void RediffBolConn::connectToCS() { 

	ostringstream out;
	string name = account->username;
	name = fixEmail(name);
	string pass = account->password;

	int credsize = name.length() + pass.length();
	int size = 58 + credsize + strlen(CSRequestHeader) 
		+strlen(CSCmdSignIn);


	write_int(out, size-4);

	write_int(out, 0);
	write_int(out, 0);
	
	write_int(out, strlen(CSRequestHeader));
	out.write(CSRequestHeader, strlen(CSRequestHeader));

	write_int(out, strlen(CSCmdSignIn));
	out.write(CSCmdSignIn, strlen(CSCmdSignIn));

	size = credsize + 34;
	write_int(out, size);


	write_int(out, name.length());
	out.write(name.c_str(), name.length());


	write_int(out, pass.length());
	out.write(pass.c_str(), pass.length());

	/* magic string, even Binu Paul doesn't know :) */
	char magic[] = {00, 00, 00, 00, 00, 00, 03,  00, 00, 00, 33, 31, 35};

	out.write(magic, sizeof(magic));

	write_int(out, 9);
	out.write("Winver:48", 9);
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "CS packet size: %d\n",
		     (int) out.str().length());

	hex_dump(out.str(), string("Sending CS acket"));
	connection->write(out.str().data(), out.str().length());
}

RediffBolConn::~RediffBolConn() { 
	softDestroy();

	if ( connection ) delete connection;
}		
void RediffBolConn::gotConnected() { 
	/* what's my state? */
	if (connection->getParseMode () == 1) {
		connectToGK();
	}
	else if (connection->getParseMode () == 2) { 
		/* HTTP GK */
		string request = "GET http://gatekeeper.rediff.com:80/direct.cgi HTTP/1.1\r\n\r\n";
		connection->write(request);
	}
	else connectToCS();

}

void RediffBolConn::readCallback(MessageBuffer &buffer, PurpleAsyncConn *conn) try { 
	if ( conn != connection ) { 
		purple_debug_info("rbol", "unintended read callback\n");
		return;
	}

	if ( isInvalid () ) { 
		purple_debug_info("rbol", "read callback on bad guy\n");
	}

	if ( conn->getParseMode() == 2 ) {
		string resp = buffer.getRawBuffer();
		buffer = MessageBuffer("");
		const char* goodreply = "HTTP/1.1 200 OK";
		if (resp.substr(0, strlen(goodreply)) == goodreply ) {
			connectToGK();
		} else { 
			purple_debug_info("rbol", "%s\n", resp.c_str());
			setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
					     "Problem with Gatekeeper HTTP\n");
		}
		conn->setParseMode(1);/* from now onwards looks like normal GK */
	}
	while ( buffer.left() >= 4 ) { 
		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "got a response\n");
		
		
		if ( buffer.peekInt32() + 4 > buffer.left()) return;
		
		int size = buffer.readInt32();
		MessageBuffer buf = buffer.readMessageBuffer(size);
		hex_dump(buf.peek(), "response");
		buffer = buffer.tail ();
		
		/* try and parse to see what we've got */
		if ( connection->getParseMode()) 
			parseGkResponse(buf);
		else 
			parseCSResponse(buf);
	}

	
}

catch (MessageBufferOverflowException e ) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "overflow exception %d\n", 
		     buffer.getLength() );
	buffer.reset();
	return;
  }


void RediffBolConn::parseGkResponse(MessageBuffer &buffer) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "got a gk response %d\n", buffer.left());

	int type = buffer.readInt32();
        int subtype = buffer.readInt32();
        int payloadsize = buffer.readInt32();
	
        int numentries = buffer.readInt32();

	purple_debug(PURPLE_DEBUG_INFO, "rbol",
		     "initial parameters %d %d %d %d\n", type, subtype, 
		     payloadsize, numentries);
	map<string, string> cap;

        for(int i = 0;i < numentries ; i++) { 
                int subentries = buffer.readInt32();
                for(int j = 0;j < subentries; j++ ){ 
                        string cp = buffer.readString();
                        string val = buffer.readString();
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "cap=%s val=%s\n",
				     cp.c_str(), val.c_str());
                        cap[cp] += val;
                }
        }

	string list = cap[CAP_DIR];
        for(size_t i = 0;i < list.size() ; i++ )
                if ( list[i] == ',' ) list[i] = ' ';

        istringstream iss(list);
        
        vector<string> res;
        string s;
        while( iss >> s ) { 
                res.push_back(s);
        } 

	
	string ip = res[ rand() % res.size() ];
	//string ip = res[0];

	//connection->unref();


	
	int f = ip.find(":");
	int port = atoi(ip.substr(f+1).c_str());
	ip = ip.substr(0, f);


	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "connecting to chatserver[%s,%d]\n", ip.c_str(), port);

	PurpleAsyncConn* newconn =  new PurpleAsyncConn(this, 
					 0);
	if ( ! newconn->establish_connection(ip, port)) 
		setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
				     "Failed to initiate connection to CS\n");

	connection->close();
	connection->delRef();

	connection = newconn;
}

gboolean keep_alive_timer(gpointer data) { 
	RediffBolConn *conn = (RediffBolConn*) data;
	conn->sendKeepAlive();
	return true;
}

void RediffBolConn::parseCSLoginResponse(MessageBuffer buffer) { 
	if ( connection_state >= 2 ) { 
		purple_debug(PURPLE_DEBUG_ERROR, "rbol", 
			     "Uh? Why am I getting a connection packet" 
			     " when I'm connected!?\n");
		return;
	}
	map<string, string> optionsmap;
	vector<string> roster;
	map<string, vector<string> > groups;
	int payloadsize = buffer.readInt();
	assert(buffer.left() == payloadsize );
	
	int errorcode = buffer.readInt();

	if ( errorcode != 0 ) { 
		char err[100];

		if ( errorcode == 1 ) { 
			setStateNetworkError(PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED, "Authentication failed");
			return;
		}
		sprintf(err, "CS Login error %d\n", errorcode);
		setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR , err);
		return;
	}

	connection_state = 2;
	string names [3];

	for(int i = 0;i < 3; i++) 
		names[i] = buffer.readString();

	this->server_userId = names[0];
	this->server_displayname = names[1];
	this->server_nickname = names[2];

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Got names %s %s %s\n",
		     names[0].c_str() ,
		     names[1].c_str() ,
		     names[2].c_str());

	buffer.readInt ();/* code1 */
	buffer.readInt ();/* code2 */

	int numprops = buffer.readInt();
	
	for(int i = 0;i < numprops; i++) { 
		string propname = buffer.readString();
		string value = buffer.readString();
		optionsmap[propname] = value;
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "%s = %s\n",
			     propname.c_str(), value.c_str());
	}

	//read roster entries (buddy list)
	int rosternum = buffer.readInt();

	for(int i = 0;i < rosternum; i++) { 
		string name = buffer.readString();
		roster.push_back(name);
		buffer.readInt();/* what integer does this read?? */
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "Got contact %s\n",
			     name.c_str());

	}

	int telnums = buffer.readInt();
	for(int i = 0;i < telnums; i++ ){ 
		buffer.readString();/* telephone number */
		buffer.readString();/* contact name */ 
		/* discard! */
	}

	//read blocked entries 
	set<string> blocked;
	int blockednum = buffer.readInt();
	for(int i = 0;i <  blockednum ; i++) { 

		string name = buffer.readString();
		blocked.insert(name);
		buffer.readInt();
	}

	int allowednum = buffer.readInt();
	for(int i = 0;i < allowednum; i++) {
		string name = buffer.readString();
		purple_debug(PURPLE_DEBUG_INFO, "rbol", "allowed --> %s\n", 
			     name.c_str());
		buffer.readInt();
	}

	// ?? 
	buffer.seek(6);
	
	// session and visible IP
	session = buffer.readString();
	visibleIP = buffer.readString();

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Session: %s IP:%s\n", 
		     session.c_str(), visibleIP.c_str());

	buffer.readInt();/* code3 */
	buffer.readByte();/* code4 */

	string unknown = buffer.readString();

	string code6 = buffer.readString();
	string code7 = buffer.readString();

	int numgroups = buffer.readInt();
        purple_connection_update_progress(account->gc, ("Connected"),
                                          1,   /* which connection step this is */
                                          2);/* total number of steps */
	

	purple_connection_set_state(account->gc, PURPLE_CONNECTED);
	for(int i = 0;i < numgroups ; i++) { 
		string groupname1 = buffer.readString();
		string groupname2 = buffer.readString();

		int groupentries = buffer.readInt();

		this->groups.insert(groupname1);

		purple_debug(PURPLE_DEBUG_INFO , "rbol", "Group %s %s %d\n" ,
			     groupname1.c_str(), groupname2.c_str(),
			     groupentries);
		vector<string> members;
		for(int j = 0;j < groupentries ; j ++) {
			string membername = buffer.readString();
			members.push_back(membername);
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "%s is member of %s\n",
				     membername.c_str(), groupname2.c_str());

			/* add this buddy if required */
			PurpleBuddy *buddy = purple_find_buddy(account, 
						  membername.c_str());
			if ( !buddy ) { 
				buddy = purple_buddy_new(account, 
							 membername.c_str(),
							 membername.c_str());
				PurpleGroup *group = purple_find_group(
					groupname2.c_str());
				if ( ! group ) 
					group = purple_group_new(groupname2.c_str());
				purple_blist_add_buddy(buddy, 
				      purple_buddy_get_contact(buddy),
				      group, NULL);
			}
		}
		groups[groupname2] = members;
	}

	string id = buffer.readString();

	PurpleStatus * status = purple_account_get_active_status(account);
	string msg = SAFE(purple_status_get_attr_string(status, "message"));
	string status_id = SAFE(purple_status_get_id(status));
	setStatus(status_id, msg);

	sendKeepAlive();

	/* first fix my current status */
	
	sendOfflineMessagesRequest();
	sendGetAddRequest();

	for(size_t i = 0;i < roster.size(); i ++) 
		loadAvatar(roster[i]);

	keep_alive_timer_handle = purple_timeout_add_seconds(30, keep_alive_timer, this);
}
string getStatusFromID(int status) { 
	if ( status == 2 ) { 
		return "away";
	} else if ( status == 3 ) 
		return "busy";
	else if ( status == 1 ) 
		return  "online";
	else
		return "offline";
	
}
void RediffBolConn::parseCSContacts(MessageBuffer &buffer) { 
	assert(!isInvalid());
	buffer.readInt();/* int payloadsize */
	int numcontacts = buffer.readInt();
	map<string, vector<string> > contacts;
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "#contacts = %d\n", 
		     numcontacts);

	for(int i = 0;i < numcontacts ; i++) { 
		string name1 = buffer.readString();
		string name2 = buffer.readString();
		string name3 = buffer.readString();
		
		purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
			     "got contact [%s] [%s] [%s]\n", 
			     name1.c_str(), name2.c_str(), name3.c_str());


		buffer.readInt();/* code1 */
		int status = buffer.readInt();
		string msg = buffer.readString();
		buffer.readInt();/* code3 */
		string version = buffer.readString();

		string sstatus = getStatusFromID(status);

		purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
			     "setting status  [%s] [%s] [%s]\n", 
			     name1.c_str(), sstatus.c_str(), msg.c_str());
		
		purple_prpl_got_user_status(account, name1.c_str(), 
					    sstatus.c_str(),
					    NULL,
					    NULL);
					    
					    
		/* do the purple work */
	}
	
	string id = buffer.readString();
	
}

void RediffBolConn::parseContactStatusChange(MessageBuffer &buffer) { 
	assert(!isInvalid());
	int payloadsize = buffer.readInt();
	buffer = buffer.readMessageBuffer(payloadsize);

	string name1 = buffer.readString();
	int status = buffer.readInt();
	
	string msg = buffer.readString();
	
	buffer.readInt();/* code3 */
	string version = buffer.readString();


	string sstatus = getStatusFromID(status);

	purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
		     "setting status  [%s] [%s] [%s]\n", 
		     name1.c_str(), sstatus.c_str(), msg.c_str());
	
	purple_prpl_got_user_status(account, name1.c_str(), 
				    sstatus.c_str(),
				    "message",
				    msg.c_str(),
				    NULL);

	status_text[name1] = msg;

	/* attempt to ask libpurple to update status message too */

	PurpleBuddy *b = purple_find_buddy(account, name1.c_str());
	if ( b == NULL ) return;
	PurplePresence *pres = purple_buddy_get_presence(b);
	PurpleStatus * pstatus = purple_presence_get_active_status(pres);

	purple_blist_update_buddy_status(b, pstatus);
	
}
void RediffBolConn::parseCSResponse(MessageBuffer &buffer) { 
	/* gah... boring enumeration.. but at the same time
	   if I don't know a packet I should atleast read the 
	   entire packets contents and remove it from the buffer 
	*/

	assert(!isInvalid());
	int type = buffer.readInt();
	
	string header = "";
	string cmd = "";


	if ( type == 1 ) { 
		if ( buffer.getLength() == 12 ) {
			hex_dump(buffer.peek(), "keep alive string");
			return;
		}

		int subtype = buffer.readInt();

		if ( subtype == 0 and connection_state < 2 ) { 
			parseCSLoginResponse(buffer);
			return;
		} else if (subtype == 1 or subtype == 0) { 
			parseOfflineMessages(buffer);
			return;
		} else if ( subtype == 2 ) {
			parseOfflineAddContactResponse(buffer);
			return;
		} else if ( subtype == 7 ) { 
			/* chat room response */
			parseChatRoomsResponse(buffer);
			return;
		} else if ( subtype == 62 ) { 
			/* join chat room response */
		} else if ( subtype == 14 ) { 
			parseGetContactIdResponse(buffer);
			return;
		}


		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "Got subtype as %d\n", subtype);

		int size = buffer.readInt();
		string packet = buffer.readStringn (size);
		hex_dump(packet, "Unknown Type 1 response from server");
		
		return;
	} else if ( type == 3 ) { 
		header = buffer.readString();
		cmd = buffer.readString();

		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "cmd is %s\n", cmd.c_str());


		if ( cmd ==  "Contacts") { 
			parseCSContacts(buffer);
			return;
		} else if ( cmd ==  "NewMailCount") { 
			parseNewMailsResponse(buffer);
			return;
		} else if ( cmd == "ContactStatusChange" ) { 
			/* ignore? */
		} else if (cmd == "ContactStatusChange2" ) { 
			parseContactStatusChange(buffer);
			return;
		} else if ( cmd == "ContactAddRequest" ){ 
			parseContactAddRequest(buffer);
			return;
		}else if ( cmd == "TextMessage" ) { 
			parseTextMessage(buffer);
			return;
		} else if ( cmd == "MessageFromMobileUser" ) {
			parseMessageFromMobileUser(buffer);
			return;
		} else if ( cmd == "TypingNotify" ) { 
			parseTypingNoficiationResponse(buffer);
			return;
		} else if ( cmd == "Disconnect" ) { 
			setStateNetworkError(PURPLE_CONNECTION_ERROR_NAME_IN_USE,
					     "It seems like you've logged in "
					     "elsewhere :(" );
			return;
		}
	     
		int len = buffer.readInt();
		string dump = buffer.readStringn(len);
		
		hex_dump(dump, "Unknown type 3 response");
		return;
	} else if ( type == 2 ) { 
		/* server message */
		int code = buffer.readInt32();
		buffer.readInt32();/* size */
		
		string message = buffer.readString();
		string reason = buffer.readString();
		int code2 = buffer.readInt();

		purple_debug(PURPLE_DEBUG_INFO, "rbol",
			     "SERVER MESSAGE: code=%d code2="
			     "%d MESSAGE=%s | REASON=%s\n",
			     code, code2, message.c_str() , reason.c_str() );

		return;
	}
				
		

}

void RediffBolConn::setStateNetworkError(int  reason,
					 string msg) {
	/* can't do anything now */
	assert(!isInvalid());
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "%s\n", msg.c_str());
	softDestroy();
	purple_connection_error_reason(account->gc, 
	      (PurpleConnectionError)reason, (msg+"\n").c_str());



} 

/**
 * Sends _unstylized_ messages 
 */

void RediffBolConn::sendMessage(string to, string message) { 
	assert(!isInvalid());
	string fonttype = "Dialog";/* confused :( */

	char *tmp = purple_unescape_html(message.c_str());
	message = tmp;
	g_free(tmp);

	string sender = account->username;
	sender = fixEmail(sender);

	fonttype = encode_from_iso(fonttype, "UTF-16BE");
	message = encode_from_iso(message, "UTF-16BE");

	int msgsize = message.length();
	int sendersize = sender.length();
	int recipientsize = to.length();
	int fontstrsize = fonttype.length ();

	int size = 57 + msgsize + sendersize + recipientsize +
		strlen(CSCmdTextMessage) + strlen(CSPeerRequestHeader) 
		+ fontstrsize;

	ostringstream out;
	write_int(out, size-4);
	write_int(out, 3);
	write_int(out, strlen(CSPeerRequestHeader) );
	out.write(CSPeerRequestHeader, strlen(CSPeerRequestHeader));; 

	write_int(out, strlen(CSCmdTextMessage));
	out.write(CSCmdTextMessage, strlen(CSCmdTextMessage));
	
	write_int(out, 37+ sendersize + recipientsize + msgsize + 
		  fontstrsize );

	write_int(out, sender.length());
	out.write(sender.data() , sender.length());

	write_int(out, to.length());
	out.write(to.c_str(), to.length());

	write_int(out, 25+fontstrsize + msgsize);
	write_int(out, 0);
	write_short(out, 0);

	out<<intToSWord(fonttype.size());
	out<<fonttype;

//	write_shortout, fontstrsize);
	//out.write(fonttype.data(), fontstrsize);

	char fontBytes[] = {
		0x00, 
		0x78, /* font size, scaled up by 10 */ 
		0xff, 0xff, 0x05, 0x01, 0x00, 0x32, 0x00,
		0x49, 0x33, 0x33, 0x33, 0x00, 0x00
		};

	out.write(fontBytes, sizeof(fontBytes));

	out<<intToSWord(message.size());
	out<<message;


	hex_dump(out.str(), "sending message");
	connection->write(out.str().data(), out.str().length());
}

void RediffBolConn::sendKeepAlive(){
	assert(!isInvalid());

	int size = 24 + strlen(CSRequestHeader) + strlen(CSCmdKeepAlive);

	ostringstream out;
	write_int(out, size-4);
	write_int(out, 0);
	write_int(out, keep_alive_counter);
	keep_alive_counter++;

	write_int(out, strlen(CSRequestHeader));
	out << CSRequestHeader;

	write_int(out, strlen(CSCmdKeepAlive));
	out<<CSCmdKeepAlive;

	write_int(out, 0);

	connection->write(out.str().data(), out.str().length());
}

void RediffBolConn::parseOfflineMessages(MessageBuffer &buffer) { 
	purple_debug(PURPLE_DEBUG_INFO, "rbol" , 
		     "parsing offline messages\n" );
	int payloadsize = buffer.readInt();
	buffer = buffer.readMessageBuffer(payloadsize);
	int numentries = buffer.readInt();

	for(int i = 0;i < numentries; i ++) { 
		string msgid = buffer.readString();
		string sender = buffer.readString();
		
		int msgsize = buffer.readInt();
		MessageBuffer msgbuf = buffer.readMessageBuffer(msgsize);
		
		int msgtype = buffer.readInt();
		int timestamp = buffer.readInt();

		purple_debug_info("rbol", "Got offline message time as %d",
				  timestamp);
		
		if ( msgtype == 0 ) { 
			string msg = msgbuf.readStringn(msgbuf.getLength());

			serv_got_im(purple_account_get_connection(account),
				    sender.c_str(), msg.c_str(), 
				    PurpleMessageFlags(0), time(NULL));
		} else {
			string final_message = _parseChatMessage(msgbuf);
			serv_got_im(purple_account_get_connection(account),
				    sender.c_str(), final_message.c_str() ,
				    PurpleMessageFlags(0), timestamp );
				  
		} /* else */ 


		deleteOfflineMessage(msgid);
	}
	
}


void RediffBolConn::sendOfflineMessagesRequest() { 
	assert(!isInvalid());
	ostringstream out;
	int totalsize = 24 + strlen(CSRequestHeader) + strlen(CSCmdGetOfflineMsgs);

	out<<intToDWord(totalsize-4);
	out<<intToDWord(0);
	out<<intToDWord(1);
	out<<intToDWord(strlen(CSRequestHeader) );
	out<<CSRequestHeader;
	out<<intToDWord(strlen(CSCmdGetOfflineMsgs));
	out<<CSCmdGetOfflineMsgs;
	

	out<<intToDWord(0);
	
	connection->write(out.str().data(), out.str().length());
}

PurpleAccount* RediffBolConn::getProxyAccount() { 
	return account;
}

string RediffBolConn::_parseChatMessage(MessageBuffer &buffer) { 
	string final_message;


	while ( !buffer.isEnd() ) { 
		int len = buffer.readLEInt();

		if ( len == 0 ) { 
			int fontstrlen = buffer.readLEInt();
			string  font = buffer.readStringn(fontstrlen);
			string fontinfo = buffer.readStringn(13);
			
			int messagelen = buffer.readLEInt();
			//string message = escape_html_entities(
			//	buffer.readStringn(messagelen) );
			
			string message = buffer.readStringn(messagelen);



			
			hex_dump(message, "message before ASCII encoding\n");
			message = encode( message, "UTF-16BE", "UTF-8");
			font = encode(font, "UTF-16BE", "UTF-8");
			message = escape_html_entities (message);


			FontParser fp (font, fontinfo);

#ifdef REDIFFBOL_ENABLE_INCOMING_FONT_SUPPORT
			final_message += "<font face='";
			final_message += font;
			final_message += "' size='";

			ostringstream fs;
			fs << double(fp.getSize ())/8;

			final_message += fs.str();
			final_message += "' color='";
			final_message += fp.getColor();
			final_message += "' >";
#endif

			if ( fp.isItalic()) final_message += "<em>";
			if ( fp.isBold()) final_message += "<b>";

			final_message += message;

			if ( fp.isBold()) final_message += "</b>";
			if ( fp.isItalic()) final_message += "</em>";

#ifdef REDIFFBOL_ENABLE_INCOMING_FONT_SUPPORT 
			final_message += "</font>";
#endif

		} else if ( len == 1 ) { 
			int smileycode = buffer.readLEInt();
			int smileylen = buffer.readLEInt();
			string smiley = buffer.readStringn(smileylen);

			purple_debug_info("rbol", "Got smiley %s %d\n",
					  smiley.c_str(), smileycode);
			const char* codes[] = { 
				"0:-)", ">:-(", "(B)", ":s", ":`(",
				">:|" , "(D)" , ":-$", "8-|","#:)",
				":^)" , ":D"  , ":-(" , "^o)", ":((",
				":-z", ":-)", ":-O", "\\:)" , ":p",
				";-)"} ;
			
			if ( smileycode < int(sizeof(codes)/sizeof(codes[0]))) 
				final_message += codes[smileycode];
		} else { 
			purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
				     "Unknown message type.. \n");
			continue;
		}
	}
	
	return final_message;
	
}
void RediffBolConn::parseTextMessage( MessageBuffer &buffer) { 
	int payloadsize = buffer.readInt();
	buffer = buffer.readMessageBuffer(payloadsize);
	
	string senderstr = buffer.readString();
	string recipient  = buffer.readString();
	
	int subpayloadsize = buffer.readInt();

	buffer = buffer.readMessageBuffer(subpayloadsize);
	
	string final_message = _parseChatMessage(buffer);

	purple_debug(PURPLE_DEBUG_INFO, "rbol",
		     "got message %s\n", final_message.c_str());
	serv_got_im(purple_account_get_connection(account) ,
		    senderstr.c_str() ,final_message.c_str(), 
		    (PurpleMessageFlags) 0, time(NULL) );

	

}


void RediffBolConn::deleteOfflineMessage(string id) {
	assert(!isInvalid());
	int totalsize = 24+strlen(CSRequestHeader) + 
		strlen(CSCmdDelOfflineMsg) + id.length();

	ostringstream out;
	out<<intToDWord(totalsize-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));

	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdDelOfflineMsg));
	out<<CSCmdDelOfflineMsg;

	out<<intToDWord(4+id.length());
	out<<intToDWord(id.length());
	out<<id;

	connection->write(out.str().data(), out.str().length());
}
#include <ctype.h>
#include <algorithm>
void RediffBolConn::setStatus(string status, string message) { 
	
	assert(!isInvalid());


	string t = status;
	transform(t.begin(), t.end(), t.begin(), ::tolower );

	purple_debug(PURPLE_DEBUG_INFO, "rbol", 
		     "Setting status [%s] [%s] [%s]\n", status.c_str(), 
		     message.c_str(), t.c_str());

	if ( message == "" ) message = status;

	int code;
	if ( t == "online" or t == "available" ) 
		code = 1;
	else if ( t == "away" ) 
		code = 2;
	else if ( t == "busy" ) 
		code = 3;
	else if ( t == "invisible" )
		code = 4;
	else return;

	int size = 28 + strlen(CSRequestHeader) + strlen(CSCmdSetOnlineStatus) 
		+ message.length();
	
	ostringstream out;
	out << intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;
	out<<intToDWord(strlen(CSCmdSetOnlineStatus));
	out<<CSCmdSetOnlineStatus;

	size = 8 + message.length();
	
	out<<intToDWord(size);
	out<<intToDWord(code);
	out<<intToDWord(message.length());
	out<<message;

	hex_dump(out.str(), "status change request");
	connection->write(out.str());
	
}

void RediffBolConn::closeCallback(PurpleAsyncConn* conn) { 
	if ( conn != connection ) return;
	/* aargh.. server has closed the connection */

	if (conn->getParseMode () == 1) {
		/* hmm, let's go into HTTP GK mode */
		startLoginOver80 ();
	}
	
	assert(!isInvalid());
	setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
			     "Pathetic, the server has closed the connection");
}


void RediffBolConn::sendGetAddRequest() { 
	assert(!isInvalid());
	int size = 24+strlen(CSRequestHeader) + strlen(CSCmdGetAddRequests);

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(0);
	out<<intToDWord(2);

	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;
	out<<intToDWord(strlen(CSCmdGetAddRequests));
	out<<CSCmdGetAddRequests;

	out<<intToDWord(0);
	
	connection->write(out.str());
}

string RediffBolConn::getBuddyNickname(std::string buddyname) { 
	return nickname[buddyname];
}
std::string RediffBolConn::getBuddyStatusMessage(std::string buddyname) { 
	return status_text[buddyname];
}

void RediffBolConn::parseNewMailsResponse(MessageBuffer &buffer) { 
	int payloadsize = buffer.readInt();
	buffer = buffer.readMessageBuffer(payloadsize);
	
	string str = buffer.readString();
	int num = atoi(str.c_str());

	const char* to = purple_account_get_username(account);
	static const char *url = "http://www.rediffmail.com";
	purple_notify_emails(account->gc, num, FALSE, NULL, NULL, &to, 
			     &url, /* replace with rediffmail URL sometime */
			     NULL, NULL);
}

string RediffBolConn::fixEmail(string original) { 
	if ( find(original.begin(), original.end(), '@') == original.end() ) 
		return original + "@rediffmail.com";
	else return original;
}

struct AddRequest { 
	RediffBolConn* conn;
	string localid;
	string from;
	string from2;
	string reqId;
};

void RediffBolConn::sendDenyAddRequest(string localid, 
				       string reqId, 
				       string from, string from2, 
				       string group) { 
	int size = 24 + strlen(CSRequestHeader) 
		+ strlen(CSCmdDelAddRequest) 
		+ reqId.length();

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdDelAddRequest));
	out<<CSCmdDelAddRequest;

	out<<intToDWord(reqId.length()+4);
	out<<intToDWord(reqId.length());
	out<<reqId;

	connection->write(out.str());
}
void RediffBolConn::sendAcceptAddRequest(string localid, 
					 string reqId, 
					 string from, string from2,
					 string group) { 

	assert(!isInvalid());
	int size = 38 + strlen(CSRequestHeader) + strlen(CSCmdAcceptAdd2) +
		localid.length() + reqId.length() + group.length()
		+from.length();
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Accepting [%s] [%s] [%s] [%s] [%s]\n", 
		     localid.c_str(), reqId.c_str(), from.c_str(),
		     from2.c_str(), group.c_str());
	
	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdAcceptAdd2));
	out<<CSCmdAcceptAdd2;

	size = 18 + localid.length() + reqId.length()  + group.length() 
		+ from.length();

	out<<intToDWord(size);
	
	out<<intToDWord(localid.length());
	out<<localid;

	out<<intToDWord(reqId.length());
	out<<reqId;

	out<<intToDWord(group.length());
	out<<group;

	out<<intToDWord(from.length());
	out<<from;

	out<<intToDWord(1);
	out<<intToDWord(1);

	hex_dump(out.str(), "Accepting Addrequest");
	connection->write(out.str());
}


static void contact_add_request_authorize_cb(void *data) {
	AddRequest *ar = (AddRequest*) data;

	ar->conn->sendAcceptAddRequest(ar->localid, ar->reqId, ar->from, 
				       ar->from2, "Friends");
	delete ar;
}

static void contact_add_request_deny_cb(void *data) { 
	AddRequest* ar = ((AddRequest*) data);;

	ar->conn->sendDenyAddRequest(string(ar->localid), 
				     string(ar->reqId), 
				     string(ar->from),
				     string(ar->from2), string("Friends"));
	delete ar;
}

void RediffBolConn::parseContactAddRequest(MessageBuffer &buffer) {
	int size = buffer.readInt();
	buffer = buffer.readMessageBuffer(size);
	AddRequest *ar = new AddRequest;
	ar->localid = buffer.readString();
	ar->reqId = buffer.readString();
	ar->from = buffer.readString();
	ar->from2 = buffer.readString();
	ar->conn = this;
	
	purple_debug(PURPLE_DEBUG_INFO, "rbol", "Received an AddRequest %s %s %s\n", 
		     ar->localid.c_str(), ar->from.c_str(), ar->from2.c_str());



	purple_account_request_authorization(account, 
					     ar->from.c_str(), 
					     ar->localid.c_str(), 
					     ar->from2.c_str(),
					     NULL,
					     false, 

					     contact_add_request_authorize_cb,
					     contact_add_request_deny_cb,
					     ar);

	
}

void RediffBolConn::parseOfflineAddContactResponse(MessageBuffer &buffer) { 
	buffer = buffer.readMessageBuffer(buffer.readInt());

	int numentries = buffer.readInt();
	if ( numentries == 0 ) return;

	for(int i = 0;i < numentries; i ++) { 
		string reqId = buffer.readString();
		string from = buffer.readString();
		buffer.readInt();/* unknown code */
		string addreqto = buffer.readString();

		AddRequest* ar = new AddRequest;
		ar->conn = this;
		ar->reqId = reqId;
		ar->from = from;
		ar->localid = addreqto;

		purple_account_request_authorization(account,
						     ar->from.c_str(), 
						     ar->localid.c_str(),
						     ar->from2.c_str(),
						     NULL,
						     false,
						     contact_add_request_authorize_cb,
						     contact_add_request_deny_cb,
						     ar);
	}
}

void RediffBolConn::sendDelContactRequest(string idToDel, string group) { 
	idToDel = fixEmail(idToDel);
	int size = 28 + strlen(CSRequestHeader) + strlen(CSCmdDelContact) 
		+ group.length() + idToDel.length();

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdDelContact));
	out<<CSCmdDelContact;
	
	size = 8 + group.length() + idToDel.length();
	out<<intToDWord(size);

	out<<intToDWord(group.length());
	out<<group;

	out<<intToDWord(idToDel.length());
	out<<idToDel;

	hex_dump(out.str(), "delete request");
	connection->write(out.str());
}

void RediffBolConn::sendAddContactRequest( 
					  std::string remoteid, 
					  std::string group) { 

	if ( this->groups.count(group) == 0 ) 
		sendAddRemoveGroupRequest(group);

	string localid = fixEmail(SAFE(account->username) );
	remoteid = fixEmail(remoteid);

	int size = 36 + strlen(CSRequestHeader) + strlen(CSCmdAddContact)
		+ localid.length() + remoteid.length() + group.length();

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);

	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdAddContact));
	out<<CSCmdAddContact;

	size = 16  +localid.length() + remoteid.length() + group.length();
	out<<intToDWord(size);
	out<<intToDWord(localid.length());
	out<<localid;
	
	out<<intToDWord(group.length());
	out<<group;

	out<<intToDWord(remoteid.length());
	out<<remoteid;

	out<<intToDWord(0);
	
	hex_dump(out.str(), "AddRequest");
	connection->write(out.str());
	
}

void RediffBolConn::readError(PurpleAsyncConn *conn) { 
	if ( conn != connection ) return;
	assert(!isInvalid());
	setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
			     "Failed reading through socket");
}

void RediffBolConn::parseGetContactIdResponse(MessageBuffer &buffer){
	/* I don't know what to do with this, since BP has not implemented it*/
	int size = buffer.readInt();
	buffer = buffer.readMessageBuffer(size);
	
	string contactid = buffer.readString();
	purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
		     "Contact = %s", contactid.c_str());
}

void RediffBolConn::parseMessageFromMobileUser(MessageBuffer &buffer) {
	int size = buffer.readInt();
	buffer = buffer.readMessageBuffer(size);
	string to = buffer.readString();
	string from = buffer.readString();
	string message = buffer.readString();

	serv_got_im(purple_account_get_connection(account) ,
		    from.c_str() ,message.c_str(), 
		    (PurpleMessageFlags) 0, time(NULL) );
	
}

void RediffBolConn::sendTypingNotification(string to) { 
	string name = fixEmail(account->username);
	int size = 28 + strlen(CSPeerRequestHeader) + 
		strlen(CSCmdTypingNotify) + name.length() 
		+ to.length();
	
	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);

	out<<intToDWord(strlen(CSPeerRequestHeader));
	out<<CSPeerRequestHeader;

	out<<intToDWord(strlen(CSCmdTypingNotify));
	out<<CSCmdTypingNotify;

	out<<intToDWord(8+name.length()+to.length());
	out<<intToDWord(name.length());
	out<<name;
	out<<intToDWord(to.length());
	out<<to;

	assert(out.str().length() == size_t(size) );
	hex_dump(out.str(), "typing notification request");

	connection->write(out.str());
		
}
void RediffBolConn::parseTypingNoficiationResponse(MessageBuffer &buffer) {
	int size = buffer.readInt();
	buffer = buffer.readMessageBuffer(size);
	string from = buffer.readString();
	string to = buffer.readString();

	serv_got_typing(account->gc, 
			from.c_str(), 
			60, 
			PURPLE_TYPING);
}

void RediffBolConn::sendChangeBuddyGroupRequest(std::string buddy, 
				 std::string from_group, 
						std::string to_group) {

	if ( groups.count(to_group) == 0 ) { 
		sendAddRemoveGroupRequest(to_group);
	}

	string username = fixEmail(SAFE(account->username));
	int size = 36 + strlen(CSRequestHeader) + 
		strlen(CSCmdMoveContact) + 
		username.length() + 
		buddy.length() + 
		from_group.length() + 
		to_group.length();

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(strlen(CSCmdMoveContact));
	out<<CSCmdMoveContact;

	size = 16 + username.length() + buddy.length() + 
		from_group.length() + to_group.length();

	out<<intToDWord(size);
	out<<intToDWord(username.length());
	out<<username;

	out<<intToDWord(buddy.length());
	out<<buddy;

	out<<intToDWord(from_group.length());
	out<<from_group;

	out<<intToDWord(to_group.length());
	out<<to_group;

		
	hex_dump(out.str(), "move buddy request");
	connection->write(out.str());
}

void RediffBolConn::sendAddRemoveGroupRequest(string groupname, bool Remove) { 
	string cmd = CSCmdAddGroup;
	if ( Remove ) cmd = CSCmdRemoveGroup;

	string username = fixEmail(SAFE(account->username));

	int size = 28 + strlen(CSRequestHeader) 
		+ cmd.length() + groupname.length() + username.length();

	ostringstream out;
	out<<intToDWord(size-4);
	out<<intToDWord(3);
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;

	out<<intToDWord(cmd.length());
	out<<cmd;


	size = 8 + username.length() + groupname.length();
	out<<intToDWord(size);

	out<<intToDWord(username.length());
	out<<username;
	
	out<<intToDWord(groupname.length());
	out<<groupname;
	
	hex_dump(out.str(), "Group Add/Remove request");
	connection->write(out.str());
}

string RediffBolConn::getServerUserId()  const { 
	return server_userId;
}

string RediffBolConn::getServerDisplayName() const  { 
	return server_displayname;
}

string RediffBolConn::getServerNickname() const { 
	return server_nickname;
}

string RediffBolConn::getSessionString() const { 
	return session;
}

string RediffBolConn::getVisibleIP() const { 
	return visibleIP;
}

void RediffBolConn::connectionError(string error, PurpleAsyncConn* conn) {
	if ( conn != connection ) return;
	assert(!isInvalid());
	setStateNetworkError(PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
			     "Failed to connect to server");

}


