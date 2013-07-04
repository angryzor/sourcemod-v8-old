#ifndef _INCLUDE_V8_PLUGIN_H_
#define _INCLUDE_V8_PLUGIN_H_

#include <v8.h>

namespace SMV8
{
	using namespace v8;

	class Plugin
	{
	public:
		Plugin(Isolate* isolate, Handle<Context> context);
		virtual ~Plugin(void);
	private:
		Persistent<Context> context;
		Isolate* isolate;
	};
}

#endif // !defined _INCLUDE_V8_PLUGIN_H_
