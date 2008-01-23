
#include <glib.h>

struct MessageBuffer { 
	int offset ; 
	int err ; 
	GString * str ; 
} ;

extern struct MessageBuffer *messagebuffer_init(const gchar *s) ;
extern void messagebuffer_free(struct MessageBuffer* m) ;

extern gint32 messagebuffer_readInt32(struct MessageBuffer *m);

extern void messagebuffer_seek(struct MessageBuffer *m, int len) ;

extern char messagebuffer_readByte(struct MessageBuffer *m) ;

extern void messagebuffer_readBytes(struct MessageBuffer *m, GString *s, int len) ;

extern struct MessageBuffer* 
messagebuffer_readMessageBuffer(struct MessageBuffer *m,  int len);

extern GString* messagebuffer_readString(struct MessageBuffer *m) ;

extern GString* messagebuffer_readStringn(struct MessageBuffer*m, int len) ;

extern gint16 messagebuffer_readShort(struct MessageBuffer *m) ;

extern gboolean messagebuffer_isEnd(struct MessageBuffer *m) ;

extern gboolean messagebuffer_getLength(struct MessageBuffer *m) ; 

extern void messagebuffer_reset(struct MessageBuffer *m) ;
