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
	 * This object tries to mimic a *validity tester*. Any child
	 * of this should be able to easily verify if it is itself valid 
	 * or not.
	 * 
	 * A object can be set to "Invalid" even if the object still exists
	 * in memory. Thus we 3 possible states: perfectly valid, Invalid
	 * but in object exists, and absolutely invalid.
	 *
	 * This is *NOT* guaranteed to work always, there's a small but
	 * non-zero probability of failure.
	 */

	class RObject { 

		static std::set<RObject*> _valid;
#define ROBJECT_RAND_STRING_LENGTH 5
		char rand_string[ROBJECT_RAND_STRING_LENGTH];
		char rand_string_verify[ROBJECT_RAND_STRING_LENGTH];
		int ref_counter;
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

		void addRef();
		int  getRef() const;
		void  delRef();

		void dump()  const;
	};
}

#endif

