
#include "messagebuffer.h" 

struct MessageBuffer *messagebuffer_init(const gchar *s) { 
	struct MessageBuffer *m = g_new(struct MessageBuffer, 1) ;
	m->str = g_string_new(s) ;
	m->offset = 0;
}

void messagebuffer_free(struct MessageBuffer* m) { 
	g_string_free(m->str, TRUE); 
	g_free(m) ;
}

gint32 messagebuffer_readInt32(struct MessageBuffer *m) {
	int i = *((gint32*) (m->str->str+m->offset));
	m->offset += 4; 
	return i ;
}

void messagebuffer_seek(struct MessageBuffer *m, int len) { 
	m->offset += len ;
}

char messagebuffer_readByte(struct MessageBuffer *m) {
	return m->str->str[m->offset++] ;
}

void messagebuffer_readBytes(struct MessageBuffer *m, GString *s, int len) { 
	int i ;
	for(i = 0 ; i < len; i++) 
		g_string_append_c(s, messagebuffer_readByte(m) );
}

struct MessageBuffer* 
messagebuffer_readMessageBuffer(struct MessageBuffer *m,  int len){ 
	GString *s = g_string_new("") ;
	messagebuffer_readBytes(m, s, len) ;
	struct MessageBuffer*ret =  messagebuffer_init(s->str);
	g_string_free(s, TRUE) ;
	return ret; 
}

GString* messagebuffer_readString(struct MessageBuffer *m) { 
	int len = messagebuffer_readInt(m) ;
	if ( len == 0 ) return g_string_new("") ;
	GString *ret = g_string_new("") ;
	messagebuffer_readBytes(m, ret, len) ;
	return ret; 
}

GString* messagebuffer_readStringn(struct MessageBuffer*m, int len) { 
	if ( len == 0 ) return g_string_new("") ;
	GString *ret = g_string_new("") ;
	messagebuffer_readBytes(m, ret, len) ;
	return ret; 
}

gint16 messagebuffer_readShort(struct MessageBuffer *m) { 
	gint16 ret = messagebuffer_readByte(m) ;
	ret = (((gint16)messagebuffer_readByte(m)) << 8 ) | ret ;
	return ret ;
}

gboolean messagebuffer_isEnd(struct MessageBuffer *m) { 
	return ( m->offset >= m->str->len) ;
}

gboolean messagebuffer_getLength(struct MessageBuffer *m) { 
	return (m->str->len) ;
}
