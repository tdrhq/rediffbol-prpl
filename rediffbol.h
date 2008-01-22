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
    COMMAND_SHUTDOWN
  } code; 
  void * data; 
} RCommand;

typedef struct  { 
  enum { 
    SIGNAL_UPDATE_CONTACTS_COMPLETED, 
    SIGNAL_UPDATE_MESSAGES_COMPLETED,
    SIGNAL_DEL_CONTACT ,
    SIGNAL_SHUTDOWN_COMPLETED
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


extern GList * list  ;
extern GPrivate *current_connection   ;

/**
 * Function listing that runs in the thread
 */

extern gpointer connection_thread(RediffBolConn *ret) ;
