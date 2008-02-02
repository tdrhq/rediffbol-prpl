
#include "encode.h"
#include <iconv.h> 
#include <string>
#include <cassert>
#include <cerrno>

using namespace std; 
using namespace rbol ;

string rbol::encode(const string a, const string from, const string to) { 
	iconv_t ic = iconv_open( to.c_str(), from.c_str()) ;

	if ( ic == (iconv_t) -1) { 
		perror("iconv_open failed") ;
		assert(false ) ;
	}
	char* buf = new char [5*a.size()] ; 
	char * obuf = buf ;
	char *inbuf = new char[a.length()] ;
	copy(a.begin(), a.end(), inbuf ) ;

	char* oinbuf = inbuf ; 
	
	size_t inbytesleft = a.size() ; 
	size_t outbytesleft = 5*a.size() -1 ;
	size_t oo = outbytesleft ;

	size_t size = iconv(ic, &inbuf, &inbytesleft, &buf, &outbytesleft) ; 

	if ( size == (size_t) -1 ) {
		
		perror(a.c_str());
		assert(false ) ;
	}
	
	string ret ( obuf, obuf + (oo-outbytesleft) ) ;

	delete[] obuf ; 
	delete[] oinbuf ; 
	
	if ( iconv_close(ic) != 0 ) { 
		perror("encode") ;
		assert(false);
	}
	return ret ;
}


string rbol::encode_from_iso(const string a, const string to) { 
	return encode(a, string("UTF-8"), to) ;
}
