#include "rediffbol.h"

/**
 * This file contains all the functions that run in the polling thread 
 */
void send_update_contacts_request(RediffBolConn *conn) ;
void send_update_messages_request(RediffBolConn *conn) ;
bool send_message(RediffBolConn*conn, RMessage* msg) ;
void send_add_buddy(RediffBolConn *conn, const char *email) ;
size_t login_header_received( void *ptr,  size_t  size,  size_t  nmemb,  void
			      *stream) ;
size_t curl_callback_push_on_gstring(void  *buffer,  
				     size_t  size,  size_t  nmemb,  
				     GString  *userp) ;



GPrivate *current_connection  = NULL ;

gpointer
connection_thread(RediffBolConn *ret) {
	
	/* anywhere in the thread I should be able to retreive which 
	   connection we are pointing to. */
	g_private_set (current_connection, ret) ;
	assert(g_private_get( current_connection)) ;
	int number;
	
	
	/* start the connection */
	ret->easy_handle = curl_easy_init() ;
	
	/* specify the important options */
	curl_easy_setopt(ret->easy_handle, CURLOPT_COOKIEFILE, "/sucks") ;
	curl_easy_setopt(ret->easy_handle, CURLOPT_URL, 
			 "http://mail.rediff.com/cgi-bin/login.cgi");	
	
	GString *at  = g_string_new("") ;
	gchar *username = strdup(ret->acct->username) ;
	
	char* pp = strchr(username,'@'); 
	if ( pp ) *pp = 0 ;
	
	g_string_printf(at, "login=%s&passwd=%s&FormName=existing", 
			username, ret->acct->password) ;
	
	curl_easy_setopt(ret->easy_handle, CURLOPT_POSTFIELDS, at->str);
	curl_easy_setopt(ret->easy_handle, CURLOPT_HEADERFUNCTION, 
			 login_header_received) ;
	
	FILE * dump = fopen("/dev/null", "w") ;
	curl_easy_setopt(ret->easy_handle, CURLOPT_WRITEDATA, dump);
	curl_easy_perform(ret->easy_handle) ;
	
	if ( false && !(ret->connection_state & RB_CONN_STATE_CONNECTED) ) { 
		// connection has failed! Just exit the thread, we'll need to do more
		// on this later.
		ret->connection_state = RB_CONN_STATE_BAD ; 
		printf("Connection failed\n");
		return ;
	}
	curl_easy_setopt(ret->easy_handle, CURLOPT_HTTPGET, 1); 
	curl_easy_setopt(ret->easy_handle, CURLOPT_HEADERFUNCTION, NULL) ;
	
	//finally make a webmessenger connection, we don't need to get much data
	// from it.
	GString *tmp = g_string_new("") ; 
	g_string_sprintf(tmp, "http://f4webmsngr.rediff.com/webmsngr/Main.php?do=instamsngr&login=%s&session_id=%s", 
			 ret->login, ret->session_id) ;
	curl_easy_setopt(ret->easy_handle, CURLOPT_URL, tmp->str) ;
	curl_easy_perform(ret->easy_handle) ;
	
	curl_easy_setopt(ret->easy_handle, CURLOPT_WRITEDATA, NULL) ;
	
	g_string_free(at, TRUE) ;
	g_string_free(tmp, TRUE) ;
	
	g_async_queue_ref(ret->commands);
	g_async_queue_ref(ret->signals) ;
	
	do { 
		GTimeVal v ; 
		g_get_current_time(&v);
		g_time_val_add(&v, 500);
		RCommand *comm = g_async_queue_pop(ret->commands ) ;
		
		if ( comm == NULL ) continue ; 
		
		/* do what the command orders */
		if ( comm->code == COMMAND_UPDATE_CONTACTS ) 
			send_update_contacts_request( ret) ;
		
		if ( comm->code == COMMAND_UPDATE_MESSAGES ) 
			send_update_messages_request(ret) ;
		
		if ( comm->code == COMMAND_SEND_MESSAGE ) 
			send_message (ret, comm->data) ;
		
		if ( comm->code == COMMAND_ADD_BUDDY ) { 
			send_add_buddy(ret, (char*)comm->data) ;
			g_free(comm->data) ;
		}
		if ( comm->code == COMMAND_SHUTDOWN ) { 

			/* cleanup what belongs to me */
			curl_easy_cleanup(ret->easy_handle) ;
			ret->easy_handle = NULL ; 
			g_free(ret->login) ;
			g_free(ret->session_id) ;


			RSignal *sig = g_new(RSignal,1) ;
			sig->data = NULL; 
			sig->code = SIGNAL_SHUTDOWN_COMPLETED ; 
			g_async_queue_push(ret->signals, sig) ;
			g_async_queue_unref(ret->commands) ;
			g_async_queue_unref(ret->signals) ;
			return NULL;
		} 
	} while ( true) ; 
	
}
/* while logging in, a header has been received! */
size_t login_header_received( void *ptr,  size_t  size,  size_t  nmemb,  void
			      *stream) {
	
	//  printf("%x\n", (uint)  g_thread_self() );
	size_t len = size*nmemb;
	((char*)ptr)[len] = '\0'; 
	
	char *header = (char*) ptr;
	char* pos = strchr(header, ':');
	if ( !pos ) return len ;
	*pos = '\0';
	pos ++ ;
	while ( isspace(*pos) ) pos ++ ; 
	
	if ( strcmp(header, "Location") == 0 )   { 
		RediffBolConn *conn = g_private_get(current_connection) ;
		
		char *session_string = strdup(strchr(pos, '?')+1 );
		
		g_strchomp(session_string) ;
		
		GString *tmp = g_string_new("") ;
		parse_url_param(session_string, "session_id", tmp) ;
		conn->session_id = g_string_free(tmp, false) ;
		
		tmp = g_string_new("") ;
		parse_url_param(session_string, "login", tmp) ;
		conn->login = g_string_free(tmp, false) ;
		
		conn->connection_state = RB_CONN_STATE_UPDATED 
			| RB_CONN_STATE_CONNECTED ;
	}
	
	return len; 
}

void send_update_contacts_request(RediffBolConn *conn) { 
	GString* url = g_string_new("")  ; 
	g_string_sprintf(url, 
			 "http://f4webmsngr.rediff.com/webmsngr/Main.php?do=getContacts&login=%s&session_id=%s&random_key=%d", 
			 conn->login, conn->session_id, rand() % 1000000 ) ;
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_URL, 
			 url->str);
	
	GString *data = g_string_new ("") ;
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEFUNCTION, 
			 curl_callback_push_on_gstring);
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEDATA, 
			 data) ;
	
	if (  curl_easy_perform(conn->easy_handle) != 0  ) { 
		return  ; /* bad */ 
	}
	printf("and done\n");
	//  printf("XML data: %s\n", data->str) ;
	
	RSignal *sig = g_new(RSignal,1) ;
	sig->data = data ; 
	sig->code = SIGNAL_UPDATE_CONTACTS_COMPLETED ; 
	g_async_queue_push(conn->signals, sig) ;
	
	g_string_free(url, TRUE) ;
}

void send_update_messages_request(RediffBolConn *conn ) { 
	GString *url = g_string_new("") ;
	g_string_printf(url, "http://f4webmsngr.rediff.com/webmsngr/Main.php?do=getChatData&login=%s&session_id=%s&random_key=%d", conn->login, conn->session_id, 
			rand() % 1000000 ) ;
	
	// Lets make the request.
	printf("url is: %s\n", url->str); 
	curl_easy_setopt(conn->easy_handle, CURLOPT_URL, 
			 url->str);
	
	GString *data = g_string_new ("") ;
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEFUNCTION, 
			 curl_callback_push_on_gstring);
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEDATA, 
			 data) ;
	
	if (  curl_easy_perform(conn->easy_handle) != 0  ) { 
		printf("oh crap, messages failed!\n");
		return ; 
	}
	//  printf("XML data: %s\n", data->str) ;
	
	if ( data->len != 0 ) { 
		RSignal *sig = g_new(RSignal,1) ;
		sig->data = data ; 
		sig->code = SIGNAL_UPDATE_MESSAGES_COMPLETED ; 
		g_async_queue_push(conn->signals, sig) ;
	}
	
	g_string_free(url, TRUE) ;
	return  ;
}

bool send_message(RediffBolConn *conn, RMessage *msg) { 
	GString *post = g_string_new("") ;
	GString *data = g_string_new("") ;
	
	char *escaped_msg = curl_easy_escape( conn->easy_handle,
					      msg->content->str, msg->content->len) ;
	
	g_string_printf(post, "do=setChatData&login=%s&session_id=%s&to=%s&msg=%s",
			conn->login, conn->session_id,
			msg->to->str, escaped_msg) ;
	
	curl_free(escaped_msg) ;
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_URL, "http://f4webmsngr.rediff.com/webmsngr/Main.php" ) ;
	
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_POSTFIELDS, post->str) ; 
	
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEFUNCTION, 
			 curl_callback_push_on_gstring) ;
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEDATA,
			 data); 
	
	bool ret = true ; 
	if ( curl_easy_perform(conn->easy_handle) != 0 ) 
		ret = false ;
	
	/* Note that we don't need to send a signal now! */
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_HTTPGET, 1);  
	
	/* cleanup */
	g_string_free(post, TRUE);
	g_string_free(data, TRUE) ;
	g_string_free(msg->to, TRUE);
	g_string_free(msg->content, TRUE);
	
	g_free(msg); 
}

size_t curl_callback_push_on_gstring(void  *buffer,  
				     size_t  size,  size_t  nmemb,  
				     GString  *userp) {
	g_string_append_len( userp, buffer, size*nmemb) ;
	return size*nmemb ; 
}

void send_add_buddy(RediffBolConn *conn, const char *email) { 
	GString *post = g_string_new("") ;
	GString *data = g_string_new("") ;
	
	char *escaped_email = curl_easy_escape( conn->easy_handle,
						email, strlen(email)) ;
	
	g_string_printf(post, "do=addfriends&login=%s&session_id=%s&txtaddfrnds=%s",
			conn->login, conn->session_id,
			escaped_email) ;
	
	curl_free(escaped_email) ;
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_URL, "http://f1webmsngr.rediff.com/webmsngr/Main.php" ) ;
	
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_POSTFIELDS, post->str) ; 
	
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEFUNCTION, 
			 curl_callback_push_on_gstring) ;
	curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEDATA,
			 data); 
	
	bool ret = true ; 
	if ( curl_easy_perform(conn->easy_handle) != 0 ) 
		ret = false ;
	
	/* Note that we don't need to send a signal now! */
	
	curl_easy_setopt(conn->easy_handle, CURLOPT_HTTPGET, 1);  
	
	/* cleanup */
	g_string_free(post, TRUE);
	g_string_free(data, TRUE) ;


}
