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
#include <map>

using namespace rbol;
using namespace std;

static int id_counter = 0;
static map<int, RObject*> id_map;

RObject::RObject()
{ 
	invalid = false;
	id = id_counter ++;
	id_map[id] = this;
}

RObject::~RObject() { 
	/* blank out one of the strings */
	purple_debug_info ("rbol", "Deleteing object %p with id %d\n", this, id);
	id_map.erase (id);
	assert(id_map.count (id) == 0);
	setInvalid();
}

void RObject::setInvalid() { 
	invalid = true;
}

RObject* RObject::getObjectById (int id)
{
	if (id_map.count (id) == 0) return NULL;
	else return id_map[id];
}

bool RObject::isInvalid() { 
	return invalid;
}


