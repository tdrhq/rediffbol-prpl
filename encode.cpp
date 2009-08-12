/**
 * @file encode.cpp 
 * 
 * Copyright (C) 2008-2009 Arnold Noronha <arnstein87 AT gmail DOT com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "encode.h"
#include <iconv.h> 
#include <string>
#include <cassert>
#include <cerrno>
#include <cstdio>

using namespace std;
using namespace rbol;

string rbol::encode(const string a, const string from, const string to) { 
	iconv_t ic = iconv_open(to.c_str(), from.c_str());

	if (ic == (iconv_t) -1) { 
		perror("iconv_open failed");
		assert(false);
	}
	char* buf = new char [5*a.size()];
	char * obuf = buf;
	char *inbuf = new char[a.length()];
	copy(a.begin(), a.end(), inbuf);

	char* oinbuf = inbuf;
	
	size_t inbytesleft = a.size();
	size_t outbytesleft = 5*a.size() -1;
	size_t oo = outbytesleft;

	size_t size = iconv(ic, &inbuf, &inbytesleft, &buf, &outbytesleft);

	if (size == (size_t) -1) {
		
		perror(a.c_str());
		assert(false);
	}
	
	string ret (obuf, obuf + (oo-outbytesleft));

	delete[] obuf;
	delete[] oinbuf;
	
	if (iconv_close(ic) != 0) { 
		perror("encode");
		assert(false);
	}
	return ret;
}


string rbol::encode_from_iso(const string a, const string to) { 
	return encode(a, string("UTF-8"), to);
}
