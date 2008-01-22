/**
 * rediffbol.c
 * (c) Copyright 2007, Arnold Noronha
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

#define PURPLE_PLUGINS
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

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
#include <ctype.h>

//#include <libxml/xmlreader.h>
#include "rediffbol.h"

#define REDIFFBOLPRPL_ID "prpl-rediffbol"

GList * list = NULL ;

static PurplePlugin *_null_protocol = NULL;

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
		
		if ( sig -> code == SIGNAL_SHUTDOWN_COMPLETED ) { 
			/* free up everything related to this connection*/
			g_free (sig) ;
			g_async_queue_unref(conn->commands);
			g_async_queue_unref(conn->signals);
			return false ;
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
	g_thread_create(connection_thread, (void*)ret, TRUE, NULL) ;
	
	purple_timeout_add_seconds(3, process_signals, ret) ;
}


/*
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
		"Set User Info...", rediffbol_input_user_info);
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
	
	
	PurplePresence *presence = purple_buddy_get_presence(buddy);
	PurpleStatus *status = purple_presence_get_active_status(presence);
	emblem = purple_status_get_name(status);
	
	purple_debug_info("rediffbol", "using emblem %s for %s's buddy %s\n",
			  emblem, buddy->account->username, buddy->name);
	return emblem;
}

static void rediffbol_tooltip_text(PurpleBuddy *buddy,
				   PurpleNotifyUserInfo *info,
				   gboolean full) {
	
	/*TODO*/
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
	purple_status_type_add_attr(type, "message", ("Online"),
				    purple_value_new(PURPLE_TYPE_STRING));
	types = g_list_append(types, type);
	
	type = purple_status_type_new(PURPLE_STATUS_AWAY, RB_STATUS_AWAY,
				      RB_STATUS_AWAY, TRUE);
	purple_status_type_add_attr(type, "message", ("Away"),
				    purple_value_new(PURPLE_TYPE_STRING));
	types = g_list_append(types, type);
	
	type = purple_status_type_new(PURPLE_STATUS_OFFLINE, RB_STATUS_OFFLINE,
				      RB_STATUS_OFFLINE, TRUE);
	purple_status_type_add_attr(type, "message", ("Offline"),
				    purple_value_new(PURPLE_TYPE_STRING));
	types = g_list_append(types, type);
	
	type = purple_status_type_new(PURPLE_STATUS_AWAY, RB_STATUS_BUSY,
				      RB_STATUS_BUSY, TRUE);
	purple_status_type_add_attr(type, "message", ("Busy"),
				    purple_value_new(PURPLE_TYPE_STRING));
	types = g_list_append(types, type);
	
	return types;
}

static void rediffbol_login(PurpleAccount *acct)
{
	PurpleConnection *gc = purple_account_get_connection(acct);
	GList *offline_messages;
	
	purple_debug_info("rediffbol", "logging in %s\n", acct->username);
	purple_connection_update_progress(gc, ("Connecting"),
					  0,   /* which connection step this is */
					  2);  /* total number of steps */
	
	printf("starting connection\n");
	start_connection(acct);
}

static void rediffbol_close(PurpleConnection *gc)
{
	/* notify other rediffbol accounts */
	RCommand *comm = g_new(RCommand, 1 ) ;
	comm->code = COMMAND_SHUTDOWN ; 
	comm->data = NULL ;

	RediffBolConn *conn = r_find_by_acct(gc->account) ;
	g_async_queue_push(conn->commands, comm) ;
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


static void rediffbol_set_status(PurpleAccount *acct, PurpleStatus *status) {
	const char *msg = purple_status_get_attr_string(status, "message");
	purple_debug_info("rediffbol", "setting %s's status to %s: %s\n",
			  acct->username, purple_status_get_name(status), msg);
	
	
}


static void rediffbol_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
				PurpleGroup *group)
{

	RediffBolConn *conn = r_find_by_acct(gc->account) ;
	
	assert(conn) ;
	RCommand *comm = g_new(RCommand, 1) ;
	comm->code = COMMAND_ADD_BUDDY ; 
	comm->data = g_strdup(buddy->name) ;
	
	g_async_queue_push(conn->commands, comm)  ; 
	
	purple_debug_info("rediffbol", "Adding %s", buddy->name);
	return ;
	
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


static void rediffbol_set_permit_deny(PurpleConnection *gc) {
  /* this is for synchronizing the local black/whitelist with the server.
   * for rediffbol, it's a noop.
   */
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
	NULL,                /* status_text */
	NULL,               /* tooltip_text */
	rediffbol_status_types,               /* status_types */
	NULL,            /* blist_node_menu */
	NULL,                  /* chat_info */
	NULL,         /* chat_info_defaults */
	rediffbol_login,                      /* login */
	rediffbol_close,                      /* close */
	rediffbol_send_im,                    /* send_im */
	NULL,                   /* set_info */
	NULL,                /* send_typing */
	NULL,                   /* get_info */
	rediffbol_set_status,                 /* set_status */
	NULL,                   /* set_idle */
	NULL,              /* change_passwd */
	rediffbol_add_buddy,                  /* add_buddy */
	NULL,                /* add_buddies */
	NULL,               /* remove_buddy */
	NULL,             /* remove_buddies */
	NULL,                 /* add_permit */
	NULL,                   /* add_deny */
	NULL,                 /* rem_permit */
	NULL,                   /* rem_deny */
	NULL,            /* set_permit_deny */
	NULL,                  /* join_chat */
	NULL,                /* reject_chat */
	NULL,              /* get_chat_name */
	NULL,                /* chat_invite */
	NULL,                 /* chat_leave */
	NULL,               /* chat_whisper */
	NULL,                  /* chat_send */
	NULL,                                /* keepalive */
	NULL,              /* register_user */
	NULL,                /* get_cb_info */
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
	printf("rediff starting up\n"); 
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
