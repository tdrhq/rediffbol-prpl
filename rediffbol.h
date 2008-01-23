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

struct RediffBolConn { 
	int fd ; 
	GString buffer ;
};
