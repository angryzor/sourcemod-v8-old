#include "Plugin.h"

namespace SMV8
{
	using namespace v8;

	Plugin::Plugin(Isolate* isolate, Handle<Context> context)
		: isolate(isolate), context(Isolate::GetCurrent(), context)
	{
	}

	Plugin::~Plugin(void)
	{
		context.Dispose();
	}
}