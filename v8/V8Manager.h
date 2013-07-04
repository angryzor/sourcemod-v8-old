#ifndef _INCLUDE_V8_V8MANAGER_H_
#define _INCLUDE_V8_V8MANAGER_H_

#include <v8.h>

#include "PluginInfo.h"

namespace SMV8
{
	using namespace v8;

	class Manager
	{
	public:
		Manager(void);
		virtual ~Manager(void);
	private:
		Handle<Object> BuildGlobalObject(PluginInfo& pi) const;
		Isolate* isolate;
	};
}

#endif // !defined _INCLUDE_V8_V8MANAGER_H_
