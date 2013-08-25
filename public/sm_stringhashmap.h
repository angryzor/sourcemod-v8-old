/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * =============================================================================
 * SourceMod
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _include_sourcemod_hashtable_h_
#define _include_sourcemod_hashtable_h_

#include <am-allocator-policies.h>
#include <am-hashmap.h>
#include <am-string.h>
#include <string.h>

namespace SourceMod
{

using namespace ke;

namespace detail
{
	class CharsAndLength
	{
	 public:
	  CharsAndLength(const char *str)
		: str_(str),
		  length_(0)
	  {
		  int c;
		  uint32_t hash = 0;
		  while ((c = *str++))
			  hash = c + (hash << 6) + (hash << 16) - hash;
		  hash_ = hash;
		  length_ = str - str_ - 1;
	  }

	  uint32_t hash() const {
		  return hash_;
	  }
	  const char *chars() const {
		  return str_;
	  }
	  size_t length() const {
		  return length_;
	  }

	 private:
	  const char *str_;
	  size_t length_;
	  uint32_t hash_;
	};

	struct StringHashMapPolicy
	{
		static inline bool matches(const CharsAndLength &lookup, const AString &key) {
			return lookup.length() == key.length() &&
				   memcmp(lookup.chars(), key.chars(), key.length()) == 0;
		}
		static inline uint32_t hash(const CharsAndLength &key) {
			return key.hash();
		}
	};
}

template <typename T>
class StringHashMap
{
	typedef detail::CharsAndLength CharsAndLength;
	typedef HashMap<AString, T, detail::StringHashMapPolicy> Internal;

public:
	StringHashMap()
		: internal_(SystemAllocatorPolicy()),
		  memory_used_(0)
	{
		if (!internal_.init())
			internal_.reportOutOfMemory();
	}

	// Some KTrie-like helper functions.
	bool retrieve(const char *aKey, T *aResult = NULL)
	{
		CharsAndLength key(aKey);
		typename Internal::Result r = internal_.find(key);
		if (!r.found())
			return false;
		if (aResult)
			*aResult = r->value;
		return true;
	}

	bool contains(const char *aKey)
	{
		CharsAndLength key(aKey);
		typename Internal::Result r = internal_.find(key);
		return r.found();
	}

	bool replace(const char *aKey, const T &value)
	{
		CharsAndLength key(aKey);
		typename Internal::Insert i = internal_.findForAdd(key);
		if (!i.found())
		{
			memory_used_ += key.length() + 1;
			return internal_.add(i, value);
		}
		i->value = value;
		return true;
	}

	bool insert(const char *aKey, const T &value)
	{
		CharsAndLength key(aKey);
		typename Internal::Insert i = internal_.findForAdd(key);
		if (i.found())
			return false;
		if (!internal_.add(i))
			return false;
		memory_used_ += key.length() + 1;
		i->key = aKey;
		i->value = value;
		return true;
	}

	bool remove(const char *aKey)
	{
		CharsAndLength key(aKey);
		typename Internal::Result r = internal_.find(key);
		if (!r.found())
			return false;
		memory_used_ -= key.length() + 1;
		internal_.remove(r);
		return true;
	}

	void clear()
	{
		internal_.clear();
	}

	size_t mem_usage() const {
		return internal_.estimateMemoryUse() + memory_used_;
	}

private:
	Internal internal_;
	size_t memory_used_;
};

}

#endif // _include_sourcemod_hashtable_h_
