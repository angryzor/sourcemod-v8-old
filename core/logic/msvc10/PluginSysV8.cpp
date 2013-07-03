#include "PluginSysV8.h"

using namespace v8;

CPluginV8::CPluginV8(Handle<Context> context)
	: context(Isolate::GetCurrent(),context)
{
}

CPluginV8::~CPluginV8(void)
{
	context.Dispose();
}
