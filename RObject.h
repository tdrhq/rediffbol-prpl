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

	public:
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

