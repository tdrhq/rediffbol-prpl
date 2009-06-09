/**
 * @file RObject.h 
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

#ifndef __robject__h__
#define __robject__h__

#include <string>
#include <set>
#include "config.h"
namespace rbol { 
	/**
	 * This class just tries to do the following: it creates 
	 * a "handler" for objects, so that the handler can be passed
	 * safely in callback functions, instead of the pointer itself.
	 * It also creates a notion of the object being "invalid", whose
	 * actual purpose is defined by the child class. The basic idea is
	 * that the object is still existing, but it should not be trusted
	 * as long as it remains invalid.
	 */
	class RObject { 

		bool invalid;
		int id;

	public:
		int getId () {
			return id;
		}

		static RObject* getObjectById (int id);
		virtual bool isInvalid();
		virtual void setInvalid();

		virtual ~RObject();
		RObject();

	};
}

#endif

