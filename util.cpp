/**
 * @file util.cpp 
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


#include "util.h"
#include <string>
#include <glib.h>

using namespace std;


string rbol::intToSWord(int i) { 
	string a;
	a.push_back((i>>8) & 0xff);
	a.push_back(i& 0xff);
	return a;
}


string rbol::intToDWord(int i) { 
	string a;
	a.push_back (i & 0xff);
	a.push_back ((i>>8) & 0xff);
	a.push_back((i>>16) & 0xff);
	a.push_back((i>>24) & 0xff);
	return a;

		     
}

int rbol::bytesToLEInt(char a, char b, char c, char d) { 
	gint32 ret = 0;
	ret |= (a<< 24);
	ret |= (b <<16);
	ret |= (c<< 8);
	ret |= (d);
	return ret;
}
