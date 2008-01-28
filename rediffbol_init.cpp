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

using namespace rbol ;
#define REDIFFBOLPRPL_ID "prpl-rediffbol"

static PurplePlugin *_null_protocol = NULL;



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



typedef struct {
	GcFunc fn;
	PurpleConnection *from;
	gpointer userdata;
} GcFuncData;



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
}

static void rediffbol_tooltip_text(PurpleBuddy *buddy,
				   PurpleNotifyUserInfo *info,
				   gboolean full) {
	
	/*TODO*/
}

static GList *rediffbol_status_types(PurpleAccount *acct)
{        GList *types = NULL;
        PurpleStatusType *type;
//#define PURPLE_STATUS_AWAY "Away" 
#define RB_STATUS_AWAY "away"
//#define PURPLE_STATUS_ONLINE "Online" 
#define RB_STATUS_ONLINE "online" 
//#define PURPLE_STATUS_OFFLINE "Offline" 
#define RB_STATUS_OFFLINE       "offline"
 
        
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
        
        
        return types;

}

static void rediffbol_login(PurpleAccount *acct)
{

	purple_connection_update_progress(acct->gc, ("Connecting"),
					  0,   /* which connection step this is */
					  2);  /* total number of steps */
	

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "here here here\n");

	if ( acct->gc->proto_data ) { 
		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "Closing an exisiting connection before "
			     "reconnecting.\n") ;
	}

	RediffBolConn* conn = new RediffBolConn (acct) ;
	conn->startLogin() ;

}

static void rediffbol_close(PurpleConnection *gc)
{
	RediffBolConn *conn = (RediffBolConn*) gc->proto_data ; 
	if ( conn) delete conn ;
	else purple_debug(PURPLE_DEBUG_WARNING, "rbol", 
			  "oh! how did this get destroyed already?") ;
}

static int rediffbol_send_im(PurpleConnection *gc, const char *who,
			     const char *message, PurpleMessageFlags flags)
{
	RediffBolConn *rb = (RediffBolConn*) gc->proto_data ; 
	rb->sendMessage(who, message) ;
	return 1;
}


static void rediffbol_set_status(PurpleAccount *acct, PurpleStatus *status) {
	const char *msg = purple_status_get_attr_string(status, "message");
	purple_debug_info("rediffbol", "setting %s's status to %s: %s\n",
			  acct->username, purple_status_get_name(status), msg);
	

	PurpleStatusType *stype = purple_status_get_type (status) ;
	assert(stype) ;
	PurpleStatusPrimitive primitive = purple_status_type_get_primitive(
		stype) ;
	
	RediffBolConn* rb = (RediffBolConn*) acct->gc->proto_data ;
	if ( primitive == PURPLE_STATUS_AVAILABLE or
	     primitive == PURPLE_STATUS_MOBILE ) { 
		rb->setStatus("Online", "") ;
		return ;
	}

	if ( primitive == PURPLE_STATUS_AWAY  ) { 
		rb->setStatus("Away", "") ;
		return ;
	}

	if ( primitive == PURPLE_STATUS_UNAVAILABLE
		or primitive == PURPLE_STATUS_EXTENDED_AWAY) {
		rb->setStatus("Busy", "" ) ;
		return ;
	}

	if ( primitive == PURPLE_STATUS_INVISIBLE ) { 
		rb->setStatus("Invisible", "") ;
		return ;
	}
}


static void rediffbol_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,
				PurpleGroup *group)
{

	
}

static void rediffbol_add_buddies(PurpleConnection *gc, GList *buddies,
				  GList *groups) {

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
const char *rediffbol_normalize(const PurpleAccount *acct,
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
  purple_debug_info("rbol", "starting up\n");
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

extern "C" { 
PURPLE_INIT_PLUGIN(rediffbol, rediffbol_init, info);
}
