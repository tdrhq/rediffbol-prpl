#ifndef __REDIFFBOL_H__
#define __REDIFFBOL_H__ 

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


typedef int bool ;
#define true 1 
#define false 0

typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);

struct RediffBolConn { 
	int fd ; 
	PacketRecvCallback rx_cb ;
	GotConnectedCallback got_connected_cb ;
	g_circ_buffer *txbuf ;
	guint tx_handler; 
	guint rx_handler; 
};


#endif
