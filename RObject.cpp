/**
 * @file RObject.cpp 
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



#include <set>
#include <string>
#include "RObject.h"
#include <glib.h>
#include <cassert>
#include <eventloop.h>
#include <sstream>
#include <debug.h>
#include <cstdlib>
#include <cstring>

using namespace rbol;
using namespace std;

std::set<RObject*> RObject::_valid;

RObject::RObject() :ref_counter(1) 
{ 
	for(int i = 0; i < ROBJECT_RAND_STRING_LENGTH; i++) 
		rand_string[i] = rand() & 127;

	memcpy(rand_string_verify, rand_string, ROBJECT_RAND_STRING_LENGTH);
	_valid.insert(this);
}

RObject::~RObject() { 
	/* blank out one of the strings */
	setInvalid();
}

void RObject::setInvalid() { 
	for(int i = 0; i < ROBJECT_RAND_STRING_LENGTH; i++) 
		rand_string[i] = 0;
	_valid.erase(this);
}

bool RObject::isInvalid() { 

	try { 	
		if (ref_counter == 0) return true;
		if (_valid.count(this) == 0) return true;
		
		/* verify the hashes match */
		if (memcmp(rand_string, rand_string_verify, ROBJECT_RAND_STRING_LENGTH) == 0)
			return false;
		else return true;
	} catch (...) {
		return true;
	}
}


void RObject::addRef() { 
	ref_counter ++;
}

static gboolean 
del_robject(gpointer data) {
	purple_debug_info("rbol", "Deleting object %p\n", data);
	RObject* d = (RObject*) data;
	delete d;
	return FALSE;/* should not run again */
}

void RObject::delRef() { 
	assert(ref_counter > 0);
	ref_counter --;
	if (ref_counter == 0) { 
		/* make sure all its deleted only after existing callbacks
		   are called */
		purple_timeout_add_seconds(2, del_robject, this);
	}
}

int RObject::getRef() const {
	return ref_counter;
}

void RObject::dump() const {
	ostringstream s;
	for(typeof(_valid.begin()) it = _valid.begin();
	    it != _valid.end(); it++) { 
		s << *it;
		s << " ";
	}

	purple_debug_info("rbol", "[%p] Valid objects: %s\n", this, 
			  s.str().c_str());
}
