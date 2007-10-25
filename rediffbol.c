/**
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * Rediffbol is a mock protocol plugin for Pidgin and libpurple. You can create
 * accounts with it, sign on and off, add buddies, and send and receive IMs,
 * all without connecting to a server!
 * 
 * Beyond that basic functionality, rediffbol supports presence and
 * away/available messages, offline messages, user info, typing notification,
 * privacy allow/block lists, chat rooms, whispering, room lists, and protocol
 * icons and emblems. Notable missing features are file transfer and account
 * registration and authentication.
 * 
 * Rediffbol is intended as an example of how to write a libpurple protocol
 * plugin. It doesn't contain networking code or an event loop, but it does
 * demonstrate how to use the libpurple API to do pretty much everything a prpl
 * might need to do.
 * 
 * Rediffbol is also a useful tool for hacking on Pidgin, Finch, and other
 * libpurple clients. It's a full-featured protocol plugin, but doesn't depend
 * on an external server, so it's a quick and easy way to exercise test new
 * code. It also allows you to work while you're disconnected.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

//#include "internal.h"
//#include "config.h"
#include "account.h"
#include "accountopt.h"
#include "blist.h"
#include "cmds.h"
#include "conversation.h"
#include "connection.h"
#include "debug.h"
#include "notify.h"
#include "privacy.h"
#include "prpl.h"
#include "roomlist.h"
#include "status.h"
#include "util.h"
#include "version.h"
#include <curl/curl.h>
#include <assert.h>
#include "xmlnode.h"

//#include <libxml/xmlreader.h>

#define REDIFFBOLPRPL_ID "prpl-rediffbol"


static PurplePlugin *_null_protocol = NULL;

#define RB_STATUS_ONLINE   "online"
#define RB_STATUS_AWAY     "away"
#define RB_STATUS_OFFLINE  "offline"
#define RB_STATUS_BUSY     "busy" 
#define RB_STATUS_INVISIBLE "invisible" 

typedef int bool ;
#define true 1 
#define false 0

typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
enum { 
	RB_CONN_STATE_OFFLINE=0,
	RB_CONN_STATE_CONNECTING=1,
	RB_CONN_STATE_CONNECTED=2,
	RB_CONN_STATE_BAD=4,
	RB_CONN_STATE_UPDATED=8
};
typedef struct  { 
  enum { 
    COMMAND_UPDATE_CONTACTS,
    COMMAND_UPDATE_MESSAGES,
    COMMAND_SEND_MESSAGE, 
    COMMAND_CHANGE_STATE,
    COMMAND_ADD_CONTACT,

  } code; 
  void * data; 
} RCommand;

typedef struct  { 
  enum { 
    SIGNAL_UPDATE_CONTACTS_COMPLETED, 
    SIGNAL_UPDATE_MESSAGES_COMPLETED,
    SIGNAL_DEL_CONTACT 
  } code ; 
  void *data ;
} RSignal;

typedef struct { 
  GString *to ;
  GString *content ; 
} RMessage ; 

typedef struct { 
  PurpleAccount *acct;
  CURL * easy_handle; 
  GAsyncQueue *commands ; 
  GAsyncQueue *signals ; 
  char* session_id; 
  char* login ;

  int connection_state; 
} RediffBolConn ;

GPrivate *current_connection = NULL  ;
static GList * list = NULL ;

RediffBolConn* r_create_conn(PurpleAccount *acct) { 
  RediffBolConn *conn = g_new(RediffBolConn, 1) ;

  conn->easy_handle = NULL ; 
  conn -> acct = acct ; 
  conn->commands = g_async_queue_new() ;
  conn->signals = g_async_queue_new() ;
  conn->session_id = NULL ;
  conn->login = NULL ; 

  conn->connection_state = RB_CONN_STATE_OFFLINE ; 
  
  list = g_list_append(list, conn) ;
  return conn ; 
}

RediffBolConn* r_find_by_acct(PurpleAccount *acct) { 
  GList *cur = list ; 
  while ( cur ) { 
    if ( ((RediffBolConn*)cur -> data) -> acct == acct )
      return cur->data ; 
    cur = cur->next ;
  }
  return NULL ; 
}

size_t curl_callback_push_on_gstring(void  *buffer,  
				   size_t  size,  size_t  nmemb,  
				   GString  *userp) {
  g_string_append_len( userp, buffer, size*nmemb) ;
  return size*nmemb ; 
}

void send_update_contacts_request(RediffBolConn *conn) { 
  GString* url = g_string_new("")  ; 
  g_string_sprintf(url, 
		   "http://f4webmsngr.rediff.com/webmsngr/Main.php?do=getContacts&login=%s&session_id=%s&random_key=%d", 
		   conn->login, conn->session_id, rand() % 1000000 ) ;

  // Lets make the request.
  printf("url is: %s\n", url->str); 
  curl_easy_setopt(conn->easy_handle, CURLOPT_URL, 
		   url->str);

  GString *data = g_string_new ("") ;
  curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEFUNCTION, 
		   curl_callback_push_on_gstring);
  curl_easy_setopt(conn->easy_handle, CURLOPT_WRITEDATA, 
		   data) ;

  printf("about to easy perform\n");
  if (  curl_easy_perform(conn->easy_handle) != 0  ) { 
    printf("oh crap!\n");
  
    return true ; 
  }
  printf("and done\n");
  //  printf("XML data: %s\n", data->str) ;

  RSignal *sig = g_new(RSignal,1) ;
  sig->data = data ; 
  sig->code = SIGNAL_UPDATE_CONTACTS_COMPLETED ; 
  g_async_queue_push(conn->signals, sig) ;
  
  g_string_free(url, TRUE) ;
}
void parse_url_param(gchar *data, gchar* param, GString *tmp) { 
  char *end=NULL, *beg =data ;
  while ( beg ) { 
    end = strchr(beg, '&') ;
    if ( !end ) end = beg + strlen(beg) ;

    gchar * mid = g_strstr_len( beg, end-beg, "=") ;
    if ( ! mid ) continue ; 
    if ( strncmp ( beg, param, strlen(param) ) == 0 ){
      g_string_overwrite_len(tmp, 0, mid+1, (end-mid-1) );
      return ; 
    }
    if ( ! *end ) return ;
    beg = end + 1 ; 
  }
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
    
    conn->connection_state = RB_CONN_STATE_UPDATED | RB_CONN_STATE_CONNECTED ;
  }
  
  return len; 
}


static gboolean update_messages (RediffBolConn *conn, GString *data) {
  xmlnode *cur = xmlnode_from_str(data->str, data->len) ;
  cur = xmlnode_get_child(cur, "TXTMSG") ;

  do { 
    int len ;
    const gchar *from = xmlnode_get_data( xmlnode_get_child(cur,"from") ) ;
    const gchar *message = xmlnode_get_data(xmlnode_get_child(cur, "message") );
    const gchar *senddate = xmlnode_get_data(xmlnode_get_child(cur, "senddate") );

    /* TODO: parse the time correctly */
    serv_got_im(purple_account_get_connection(conn->acct), 
		from, message, 0,  time(NULL) );

  } while ( cur = xmlnode_get_next_twin(cur) ) ;


  return true ;

  // have to stop sometime... !
  
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
    return true ; 
  }
  //  printf("XML data: %s\n", data->str) ;

  if ( data->len != 0 ) { 
    RSignal *sig = g_new(RSignal,1) ;
    sig->data = data ; 
    sig->code = SIGNAL_UPDATE_MESSAGES_COMPLETED ; 
    g_async_queue_push(conn->signals, sig) ;
  }

  g_string_free(url, TRUE) ;
  return true ;
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
static gboolean  update_contacts (RediffBolConn *conn, GString *data ) { 
  xmlnode *cur = xmlnode_from_str(data->str, data->len) ;
  cur = xmlnode_get_child(cur, "Addressbook") ;
  cur = xmlnode_get_child(cur, "Contact") ;

  do { 
    int len ;
    const gchar *nick = xmlnode_get_data( xmlnode_get_child(cur,"NickName") ) ;
    const gchar *screen = xmlnode_get_data(xmlnode_get_child(cur, "Email") );
    const gchar *status = xmlnode_get_data(xmlnode_get_child(cur, "STATUS") );

    // is this guy already on our friends list?
    PurpleBuddy *buddy = purple_find_buddy( conn->acct, screen ) ;
    if ( ! buddy ) { 
      buddy = purple_buddy_new ( conn->acct, screen, nick) ;
      PurpleGroup *red = purple_group_new ( "Rediff" ) ;
      purple_blist_add_buddy( buddy, purple_buddy_get_contact(buddy) , 
  		      red, NULL ) ;
    }

    /* In any case, update his status */
    char * s = g_ascii_strdown(status, strlen(status)) ;
    purple_prpl_got_user_status(conn->acct, screen, s, NULL, NULL, NULL ) ;
    g_free(s);
    
   
  } while ( cur = xmlnode_get_next_twin(cur) ) ;


  return true ;

  // have to stop sometime... !
}

static gboolean process_signals (RediffBolConn *conn) { 
  printf("Processing signals\n") ;
  RSignal *sig; 
  while (  sig = g_async_queue_try_pop(conn->signals) ){ 
    if ( sig->code == SIGNAL_UPDATE_CONTACTS_COMPLETED ) { 
      update_contacts(conn, sig->data) ;
      g_string_free((GString*) sig->data, false) ;
    }

    if ( sig->code == SIGNAL_UPDATE_MESSAGES_COMPLETED ) {
      update_messages(conn, sig->data) ;
      g_string_free((GString*) sig->data, false) ;
    }

    g_free(sig) ;
  }

  /* finally push commands onto the queue, if possible */
  if ( g_async_queue_length(conn->commands) < 2 && ((rand() & 3) == 0 )) {
    RCommand *comm = g_new(RCommand, 1) ;
    comm->code = COMMAND_UPDATE_CONTACTS; 
    comm->data = NULL ; 
    
    g_async_queue_push(conn->commands, comm) ;
  } else 
    printf("Queue is too big\n") ;
  
  /* push message updater onto the queue */
  if ( g_async_queue_length(conn->commands) < 2 ) {
    RCommand *comm = g_new(RCommand, 1) ;
    comm->code = COMMAND_UPDATE_MESSAGES; 
    comm->data = NULL  ; 

    g_async_queue_push(conn->commands, comm) ;
  }

  /* finally check any updates on the current state */
  if ( conn->connection_state & RB_CONN_STATE_UPDATED ) { 
    conn->connection_state ^= RB_CONN_STATE_UPDATED ;
    int state = 0 ; 
    switch( conn->connection_state ) {
    case RB_CONN_STATE_CONNECTED :
      state=PURPLE_CONNECTED; break ; 
    case RB_CONN_STATE_CONNECTING: 
      state=PURPLE_CONNECTING; break ; 
    }
    purple_connection_set_state(purple_account_get_connection(conn->acct), state);
  }
  return true ;
}

static gpointer
connection_thread(RediffBolConn *ret) {
  // the connection part of it... 
  printf(" start: %x\n", (uint) g_thread_self() );
  g_private_set ( current_connection, ret) ;
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
  
  printf("using postdata %s\n", at->str) ;
  curl_easy_setopt(ret->easy_handle, CURLOPT_POSTFIELDS, at->str);

  curl_easy_setopt(ret->easy_handle, CURLOPT_HEADERFUNCTION, login_header_received) ;
  
  FILE * dump = fopen("/dev/null", "w") ;
  curl_easy_setopt(ret->easy_handle, CURLOPT_WRITEDATA, dump);
  //  curl_easy_setopt(ret->easy_handle, CURLOPT_VERBOSE, 1);
  curl_easy_setopt(ret->easy_handle, CURLOPT_FOLLOWLOCATION, 1) ;
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

  printf("we're now connected\n") ;
  do { 
    printf("back in here\n") ;
    
    GTimeVal v ; 
    g_get_current_time(&v);
    g_time_val_add(&v, 500);
    RCommand *comm = g_async_queue_pop(ret->commands ) ;

    /* ideally we'd like to do some background processing */
    if ( comm == NULL ) continue ; 

    printf("OK sergeant!\n");
    /* do what the command orders */
    if ( comm->code == COMMAND_UPDATE_CONTACTS ) 
      send_update_contacts_request( ret) ;
    
    if ( comm->code == COMMAND_UPDATE_MESSAGES ) 
      send_update_messages_request(ret) ;

    if ( comm->code == COMMAND_SEND_MESSAGE ) 
      send_message (ret, comm->data) ;

  } while ( true) ; 
   
}
typedef struct {
  GcFunc fn;
  PurpleConnection *from;
  gpointer userdata;
} GcFuncData;


static gboolean 
poll_timer(RediffBolConn * conn) {
	//lets try and make a request to get the list friends list!		
}


static  
RediffBolConn * start_connection(PurpleAccount* acct){
	
  if ( ! current_connection ) 
    current_connection = g_private_new(NULL) ;

  
  if ( ! g_thread_supported() ){ 
    printf("gthread not supported!\n");
    exit(0);
  }

  RediffBolConn *ret =  r_create_conn(acct) ;
  g_thread_create(connection_thread, ret, TRUE, NULL) ;

  purple_timeout_add_seconds(1, process_signals, ret) ;
}


static void close_connection(RediffBolConn* conn) { 

	curl_easy_cleanup(conn->easy_handle) ;

	free(conn) ;
}

/*:wq
 *
 * stores offline messages that haven't been delivered yet. maps username
 *
 * (char *) to GList * of GOfflineMessages. initialized in rediffbol_init.
 */
GHashTable* goffline_messages = NULL;

typedef struct {
  char *from;
  char *message;
  time_t mtime;
  PurpleMessageFlags flags;
} GOfflineMessage;

/*
 * helpers
 */
static PurpleConnection *get_rediffbol_gc(const char *username) {
  PurpleAccount *acct = purple_accounts_find(username, REDIFFBOLPRPL_ID);
  if (acct && purple_account_is_connected(acct))
    return acct->gc;
  else
    return NULL;
}

static void call_if_rediffbol(gpointer data, gpointer userdata) {
  PurpleConnection *gc = (PurpleConnection *)(data);
  GcFuncData *gcfdata = (GcFuncData *)userdata;

  if (!strcmp(gc->account->protocol_id, REDIFFBOLPRPL_ID))
    gcfdata->fn(gcfdata->from, gc, gcfdata->userdata);
}

static void foreach_rediffbol_gc(GcFunc fn, PurpleConnection *from,
                                gpointer userdata) {
  GcFuncData gcfdata = { fn, from, userdata };
  g_list_foreach(purple_connections_get_all(), call_if_rediffbol,
                 &gcfdata);
}


typedef void(*ChatFunc)(PurpleConvChat *from, PurpleConvChat *to,
                        int id, const char *room, gpointer userdata);

typedef struct {
  ChatFunc fn;
  PurpleConvChat *from_chat;
  gpointer userdata;
} ChatFuncData;

static void call_chat_func(gpointer data, gpointer userdata) {
  PurpleConnection *to = (PurpleConnection *)data;
  ChatFuncData *cfdata = (ChatFuncData *)userdata;

  int id = cfdata->from_chat->id;
  PurpleConversation *conv = purple_find_chat(to, id);
  if (conv) {
    PurpleConvChat *chat = purple_conversation_get_chat_data(conv);
    cfdata->fn(cfdata->from_chat, chat, id, conv->name, cfdata->userdata);
  }
}

static void foreach_gc_in_chat(ChatFunc fn, PurpleConnection *from,
                               int id, gpointer userdata) {
  PurpleConversation *conv = purple_find_chat(from, id);
  ChatFuncData cfdata = { fn,
                          purple_conversation_get_chat_data(conv),
                          userdata };

  g_list_foreach(purple_connections_get_all(), call_chat_func,
                 &cfdata);
}



/* 
 * UI callbacks
 */
static void rediffbol_input_user_info(PurplePluginAction *action)
{
  PurpleConnection *gc = (PurpleConnection *)action->context;
  PurpleAccount *acct = purple_connection_get_account(gc);
  purple_debug_info("rediffbol", "showing 'Set User Info' dialog for %s\n",
                    acct->username);

  purple_account_request_change_user_info(acct);
}

/* this is set to the actions member of the PurplePluginInfo struct at the
 * bottom.
 */
static GList *rediffbol_actions(PurplePlugin *plugin, gpointer context)
{
  PurplePluginAction *action = purple_plugin_action_new(
    _("Set User Info..."), rediffbol_input_user_info);
  return g_list_append(NULL, action);
}


/*
 * prpl functions
 */
static const char *rediffbol_list_icon(PurpleAccount *acct, PurpleBuddy *buddy)
{
  /* shamelessly steal (er, borrow) the meanwhile protocol icon. it's cute! */
  return "meanwhile";
}

static const char *rediffbol_list_emblem(PurpleBuddy *buddy)
{
  const char* emblem;

  if (get_rediffbol_gc(buddy->name)) {
    PurplePresence *presence = purple_buddy_get_presence(buddy);
    PurpleStatus *status = purple_presence_get_active_status(presence);
    emblem = purple_status_get_name(status);
  } else {
    emblem = "offline";
  }

  purple_debug_info("rediffbol", "using emblem %s for %s's buddy %s\n",
                    emblem, buddy->account->username, buddy->name);
  return emblem;
}

static char *rediffbol_status_text(PurpleBuddy *buddy) {
  purple_debug_info("rediffbol", "getting %s's status text for %s\n",
                    buddy->name, buddy->account->username);

  if (purple_find_buddy(buddy->account, buddy->name)) {
    PurplePresence *presence = purple_buddy_get_presence(buddy);
    PurpleStatus *status = purple_presence_get_active_status(presence);
    const char *name = purple_status_get_name(status);
    const char *message = purple_status_get_attr_string(status, "message");

    char *text;
    if (message && strlen(message) > 0)
      text = g_strdup_printf("%s: %s", name, message);
    else
      text = g_strdup(name);

    purple_debug_info("rediffbol", "%s's status text is %s\n", buddy->name, text);
    return text;

  } else {
    purple_debug_info("rediffbol", "...but %s is not logged in\n", buddy->name);
    return "Not logged in";
  }
}

static void rediffbol_tooltip_text(PurpleBuddy *buddy,
                                  PurpleNotifyUserInfo *info,
                                  gboolean full) {
  PurpleConnection *gc = get_rediffbol_gc(buddy->name);

  if (gc) {
    /* they're logged in */
    PurplePresence *presence = purple_buddy_get_presence(buddy);
    PurpleStatus *status = purple_presence_get_active_status(presence);
    const char *msg = rediffbol_status_text(buddy);
    purple_notify_user_info_add_pair(info, purple_status_get_name(status),
                                     msg);

    if (full) {
      const char *user_info = purple_account_get_user_info(gc->account);
      if (user_info)
        purple_notify_user_info_add_pair(info, _("User info"), user_info);
    }

  } else {
    /* they're not logged in */
    purple_notify_user_info_add_pair(info, _("User info"), _("not logged in"));
  }
    
  purple_debug_info("rediffbol", "showing %s tooltip for %s\n",
                    (full) ? "full" : "short", buddy->name);
}

static GList *rediffbol_status_types(PurpleAccount *acct)
{
  GList *types = NULL;
  PurpleStatusType *type;

  purple_debug_info("rediffbol", "returning status types for %s: %s, %s, %s\n",
                    acct->username,
                    RB_STATUS_ONLINE, RB_STATUS_AWAY, RB_STATUS_OFFLINE);

  type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, RB_STATUS_ONLINE,
                                RB_STATUS_ONLINE, TRUE);
  purple_status_type_add_attr(type, "message", _("Online"),
                              purple_value_new(PURPLE_TYPE_STRING));
  types = g_list_append(types, type);

  type = purple_status_type_new(PURPLE_STATUS_AWAY, RB_STATUS_AWAY,
                                RB_STATUS_AWAY, TRUE);
  purple_status_type_add_attr(type, "message", _("Away"),
                              purple_value_new(PURPLE_TYPE_STRING));
  types = g_list_append(types, type);
  
  type = purple_status_type_new(PURPLE_STATUS_OFFLINE, RB_STATUS_OFFLINE,
                                RB_STATUS_OFFLINE, TRUE);
  purple_status_type_add_attr(type, "message", _("Offline"),
                              purple_value_new(PURPLE_TYPE_STRING));
  types = g_list_append(types, type);

  type = purple_status_type_new(PURPLE_STATUS_AWAY, RB_STATUS_BUSY,
                                RB_STATUS_BUSY, TRUE);
  purple_status_type_add_attr(type, "message", _("Busy"),
                              purple_value_new(PURPLE_TYPE_STRING));
  types = g_list_append(types, type);

  return types;
}

static void blist_example_menu_item(PurpleBlistNode *node, gpointer userdata) {
  purple_debug_info("rediffbol", "example menu item clicked on user\n",
                    ((PurpleBuddy *)node)->name);

  purple_notify_info(NULL,  /* plugin handle or PurpleConnection */
                     _("Primary title"),
                     _("Secondary title"),
                     _("This is the callback for the rediffbol menu item."));
}

static GList *rediffbol_blist_node_menu(PurpleBlistNode *node) {
  purple_debug_info("rediffbol", "providing buddy list context menu item\n");

  if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
    PurpleMenuAction *action = purple_menu_action_new(
      _("Rediffbol example menu item"),
      PURPLE_CALLBACK(blist_example_menu_item),
      NULL,   /* userdata passed to the callback */
      NULL);  /* child menu items */
    return g_list_append(NULL, action);
  } else {
    return NULL;
  }
}

static GList *rediffbol_chat_info(PurpleConnection *gc) {
  struct proto_chat_entry *pce; /* defined in prpl.h */

  purple_debug_info("rediffbol", "returning chat setting 'room'\n");

  pce = g_new0(struct proto_chat_entry, 1);
  pce->label = _(_("Chat _room"));
  pce->identifier = "room";
  pce->required = TRUE;

  return g_list_append(NULL, pce);
}

static GHashTable *rediffbol_chat_info_defaults(PurpleConnection *gc,
                                               const char *room) {
  GHashTable *defaults;

  purple_debug_info("rediffbol", "returning chat default setting "
                    "'room' = 'default'\n");

  defaults = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
  g_hash_table_insert(defaults, "room", g_strdup("default"));
  return defaults;
}  

static void rediffbol_login(PurpleAccount *acct)
{
  PurpleConnection *gc = purple_account_get_connection(acct);
  GList *offline_messages;

  purple_debug_info("rediffbol", "logging in %s\n", acct->username);



  
  purple_connection_update_progress(gc, _("Connecting"),
                                    0,   /* which connection step this is */
                                    2);  /* total number of steps */

	printf("starting connection\n");
	start_connection(acct);


  /* tell purple about everyone on our buddy list who's connected */
  //purple_prpl_got_user_status(acct, "arnold",PURPLE_STATUS_AVAILABLE ,
  //                              "message" , "", NULL);




  /* fetch stored offline messages */
  purple_debug_info("rediffbol", "checking for offline messages for %s\n",
                    acct->username);
  offline_messages = g_hash_table_lookup(goffline_messages, acct->username); 
  while (offline_messages) {
    GOfflineMessage *message = (GOfflineMessage *)offline_messages->data;
    purple_debug_info("rediffbol", "delivering offline message to %s: %s\n",
                      acct->username, message->message);
    serv_got_im(gc, message->from, message->message, message->flags,
                message->mtime);
    offline_messages = g_list_next(offline_messages);

    g_free(message->from);
    g_free(message->message);
    g_free(message);
  }

  g_list_free(offline_messages);
  g_hash_table_remove(goffline_messages, &acct->username);
}

static void rediffbol_close(PurpleConnection *gc)
{
  /* notify other rediffbol accounts */
}

static int rediffbol_send_im(PurpleConnection *gc, const char *who,
                            const char *message, PurpleMessageFlags flags)
{
  const char *from_username = gc->account->username;
  PurpleMessageFlags receive_flags = ((flags & ~PURPLE_MESSAGE_SEND)
                                      | PURPLE_MESSAGE_RECV);
  
  RediffBolConn *conn = r_find_by_acct(gc->account) ;

  assert(conn) ;
  // Push into the queue.
  RCommand *comm = g_new(RCommand, 1) ;
  comm->code = COMMAND_SEND_MESSAGE ; 
  
  RMessage * msg = g_new(RMessage, 1) ;
  msg->to = g_string_new(who) ;
  msg->content = g_string_new(message) ;

  comm->data = msg ; 
  
  g_async_queue_push(conn->commands, comm)  ; 

  purple_debug_info("rediffbol", "sending message from %s to %s: %s\n",
                    from_username, who, message);
  return 1;
}

static void rediffbol_set_info(PurpleConnection *gc, const char *info) {
  purple_debug_info("rediffbol", "setting %s's user info to %s\n",
                    gc->account->username, info);
}

static char *typing_state_to_string(PurpleTypingState typing) {
  switch (typing) {
  case PURPLE_NOT_TYPING:  return "is not typing";
  case PURPLE_TYPING:      return "is typing";
  case PURPLE_TYPED:       return "stopped typing momentarily";
  default:               return "unknown typing state";
  }
}

static void notify_typing(PurpleConnection *from, PurpleConnection *to,
                          gpointer typing) {
  char *from_username = from->account->username;
  char *action = typing_state_to_string((PurpleTypingState)typing);
  purple_debug_info("rediffbol", "notifying %s that %s %s\n",
                    to->account->username, from_username, action);

  serv_got_typing(to,
                  from_username,
                  0, /* if non-zero, a timeout in seconds after which to
                      * reset the typing status to PURPLE_NOT_TYPING */
                  (PurpleTypingState)typing);
}

static unsigned int rediffbol_send_typing(PurpleConnection *gc, const char *name,
                                         PurpleTypingState typing) {
  purple_debug_info("rediffbol", "%s %s\n", gc->account->username,
                    typing_state_to_string(typing));
  foreach_rediffbol_gc(notify_typing, gc, (gpointer)typing);
  return 0;
}

static void rediffbol_get_info(PurpleConnection *gc, const char *username) {
  const char *body;
  PurpleNotifyUserInfo *info = purple_notify_user_info_new();
  PurpleAccount *acct;

  purple_debug_info("rediffbol", "Fetching %s's user info for %s\n", username,
                    gc->account->username);

  if (!get_rediffbol_gc(username)) {
    char *msg = g_strdup_printf(_("%s is not logged in."), username);
    purple_notify_error(gc, _("User Info"), _("User info not available. "), msg);
    g_free(msg);
  }

  acct = purple_accounts_find(username, REDIFFBOLPRPL_ID);
  if (acct)
    body = purple_account_get_user_info(acct);
  else
    body = _("No user info.");
  purple_notify_user_info_add_pair(info, "Info", body);

  /* show a buddy's user info in a nice dialog box */
  purple_notify_userinfo(gc,        /* connection the buddy info came through */
                         username,  /* buddy's username */
                         info,      /* body */
                         NULL,      /* callback called when dialog closed */
                         NULL);     /* userdata for callback */
}

static void rediffbol_set_status(PurpleAccount *acct, PurpleStatus *status) {
  const char *msg = purple_status_get_attr_string(status, "message");
  purple_debug_info("rediffbol", "setting %s's status to %s: %s\n",
                    acct->username, purple_status_get_name(status), msg);


}

static void rediffbol_set_idle(PurpleConnection *gc, int idletime) {
  purple_debug_info("rediffbol",
                    "purple reports that %s has been idle for %d seconds\n",
                    gc->account->username, idletime);
}


static void rediffbol_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
                               PurpleGroup *group)
{
  char *username = gc->account->username;
  PurpleConnection *buddy_gc = get_rediffbol_gc(buddy->name);

  purple_debug_info("rediffbol", "adding %s to %s's buddy list\n", buddy->name,
                    username);

  if (buddy_gc) {
    PurpleAccount *buddy_acct = buddy_gc->account;



    if (purple_find_buddy(buddy_acct, username)) {
      purple_debug_info("rediffbol", "%s is already on %s's buddy list\n",
                        username, buddy->name);
    } else {
      purple_debug_info("rediffbol", "asking %s if they want to add %s\n",
                        buddy->name, username);
      purple_account_request_add(buddy_acct,
                                 username,
                                 NULL,   /* local account id (rarely used) */
                                 NULL,   /* alias */
                                 NULL);  /* message */
    }
  }
}

static void rediffbol_add_buddies(PurpleConnection *gc, GList *buddies,
                                 GList *groups) {
  GList *buddy = buddies;
  GList *group = groups;

  purple_debug_info("rediffbol", "adding multiple buddies\n");

  while (buddy && group) {
    rediffbol_add_buddy(gc, (PurpleBuddy *)buddy->data, (PurpleGroup *)group->data);
    buddy = g_list_next(buddy);
    group = g_list_next(group);
  }
}

static void rediffbol_remove_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
                                  PurpleGroup *group)
{
  
}

static void rediffbol_remove_buddies(PurpleConnection *gc, GList *buddies,
                                    GList *groups) {

}

/*
 * rediffbol uses purple's local whitelist and blacklist, stored in blist.xml, as
 * its authoritative privacy settings, and uses purple's logic (specifically
 * purple_privacy_check(), from privacy.h), to determine whether messages are
 * allowed or blocked.
 */
static void rediffbol_add_permit(PurpleConnection *gc, const char *name) {
  purple_debug_info("rediffbol", "%s adds %s to their allowed list\n",
                    gc->account->username, name);
}

static void rediffbol_add_deny(PurpleConnection *gc, const char *name) {
  purple_debug_info("rediffbol", "%s adds %s to their blocked list\n",
                    gc->account->username, name);
}

static void rediffbol_rem_permit(PurpleConnection *gc, const char *name) {

}

static void rediffbol_rem_deny(PurpleConnection *gc, const char *name) {

}

static void rediffbol_set_permit_deny(PurpleConnection *gc) {
  /* this is for synchronizing the local black/whitelist with the server.
   * for rediffbol, it's a noop.
   */
}






static void rediffbol_get_cb_info(PurpleConnection *gc, int id, const char *who) {
  PurpleConversation *conv = purple_find_chat(gc, id);
  purple_debug_info("rediffbol",
                    "retrieving %s's info for %s in chat room %s\n", who,
                    gc->account->username, conv->name);

  rediffbol_get_info(gc, who);
}

static void rediffbol_alias_buddy(PurpleConnection *gc, const char *who,
                                 const char *alias) {
 purple_debug_info("rediffbol", "%s sets %'s alias to %s\n",
                   gc->account->username, who, alias);
}



/* normalize a username (e.g. remove whitespace, add default domain, etc.)
 * for rediffbol, this is a noop.
 */
static const char *rediffbol_normalize(const PurpleAccount *acct,
                                      const char *input) {
  return NULL;
}

static void rediffbol_set_buddy_icon(PurpleConnection *gc,
                                    PurpleStoredImage *img) {
 purple_debug_info("rediffbol", "setting %s's buddy icon to %s\n",
                   gc->account->username, purple_imgstore_get_filename(img));
}






/*
 * prpl stuff. see prpl.h for more information.
 */

static PurplePluginProtocolInfo prpl_info =
{
  OPT_PROTO_CHAT_TOPIC,  /* options */
  NULL,               /* user_splits, initialized in rediffbol_init() */
  NULL,               /* protocol_options, initialized in rediffbol_init() */
  {   /* icon_spec, a PurpleBuddyIconSpec */
      "png,jpg,gif",                   /* format */
      0,                               /* min_width */
      0,                               /* min_height */
      128,                             /* max_width */
      128,                             /* max_height */
      10000,                           /* max_filesize */
      PURPLE_ICON_SCALE_DISPLAY,       /* scale_rules */
  },
  rediffbol_list_icon,                  /* list_icon */
  rediffbol_list_emblem,                /* list_emblem */
  rediffbol_status_text,                /* status_text */
  rediffbol_tooltip_text,               /* tooltip_text */
  rediffbol_status_types,               /* status_types */
  rediffbol_blist_node_menu,            /* blist_node_menu */
  NULL,                  /* chat_info */
  NULL,         /* chat_info_defaults */
  rediffbol_login,                      /* login */
  rediffbol_close,                      /* close */
  rediffbol_send_im,                    /* send_im */
  NULL,                   /* set_info */
  rediffbol_send_typing,                /* send_typing */
  rediffbol_get_info,                   /* get_info */
  rediffbol_set_status,                 /* set_status */
  rediffbol_set_idle,                   /* set_idle */
  NULL,              /* change_passwd */
  rediffbol_add_buddy,                  /* add_buddy */
  NULL,                /* add_buddies */
  NULL,               /* remove_buddy */
  NULL,             /* remove_buddies */
  rediffbol_add_permit,                 /* add_permit */
  rediffbol_add_deny,                   /* add_deny */
  rediffbol_rem_permit,                 /* rem_permit */
  rediffbol_rem_deny,                   /* rem_deny */
  rediffbol_set_permit_deny,            /* set_permit_deny */
  NULL,                  /* join_chat */
  NULL,                /* reject_chat */
  NULL,              /* get_chat_name */
  NULL,                /* chat_invite */
  NULL,                 /* chat_leave */
  NULL,               /* chat_whisper */
  NULL,                  /* chat_send */
  NULL,                                /* keepalive */
  NULL,              /* register_user */
  rediffbol_get_cb_info,                /* get_cb_info */
  NULL,                                /* get_cb_away */
  rediffbol_alias_buddy,                /* alias_buddy */
  NULL,                /* group_buddy */
  NULL,               /* rename_group */
  NULL,                                /* buddy_free */
  NULL,               /* convo_closed */
  rediffbol_normalize,                  /* normalize */
  rediffbol_set_buddy_icon,             /* set_buddy_icon */
  NULL,               /* remove_group */
  NULL,                                /* get_cb_real_name */
  NULL,             /* set_chat_topic */
  NULL,                                /* find_blist_chat */
  NULL,          /* roomlist_get_list */
  NULL,            /* roomlist_cancel */
  NULL,   /* roomlist_expand_category */
  NULL,           /* can_receive_file */
  NULL,                                /* send_file */
  NULL,                                /* new_xfer */
  NULL,            /* offline_message */
  NULL,                                /* whiteboard_prpl_ops */
  NULL,                                /* send_raw */
  NULL,                                /* roomlist_room_serialize */
  NULL,                                /* padding... */
  NULL,
  NULL,
  NULL,
};

static void rediffbol_init(PurplePlugin *plugin)
{
  purple_debug_info("rediffbol", "starting up\n");
  _null_protocol = plugin;
}

static void rediffbol_destroy(PurplePlugin *plugin) {
  printf("shutting down\n");
  purple_debug_info("rediffbol", "shutting down\n");
}


static PurplePluginInfo info =
{
  PURPLE_PLUGIN_MAGIC,                                     /* magic */
  PURPLE_MAJOR_VERSION,                                    /* major_version */
  PURPLE_MINOR_VERSION,                                    /* minor_version */
  PURPLE_PLUGIN_PROTOCOL,                                  /* type */
  NULL,                                                    /* ui_requirement */
  0,                                                       /* flags */
  NULL,                                                    /* dependencies */
  PURPLE_PRIORITY_DEFAULT,                                 /* priority */
  REDIFFBOLPRPL_ID,                                             /* id */
  "RediffBol",                                              /* name */
  "0.3",                                                   /* version */
  "RediffBol Protocol Plugin",                                  /* summary */
  "RediffBol Protocol Plugin",                                  /* description */
  "Arnold Noronha <arnold+rb@cmi.ac.in>",                     /* author */
  "http://www.cmi.ac.in/rediffbol",  /* homepage */
  NULL,                                                    /* load */
  NULL,                                                    /* unload */
  rediffbol_destroy,                                        /* destroy */
  NULL,                                                    /* ui_info */
  &prpl_info,                                              /* extra_info */
  NULL,                                                    /* prefs_info */
  rediffbol_actions,                                        /* actions */
  NULL,                                                    /* padding... */
  NULL,
  NULL,
  NULL,
};

PURPLE_INIT_PLUGIN(rediffbol, rediffbol_init, info);
