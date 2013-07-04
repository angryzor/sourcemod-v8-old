#include "V8Manager.h"

namespace SMV8
{
	using namespace v8;

	Manager::Manager(void)
	{
	}


	Manager::~Manager(void)
	{
	}

	Handle<Object> Manager::BuildGlobalObject(PluginInfo& pi) const
	{
		HandleScope handle_scope(isolate);
		Handle<ObjectTemplate> global = ObjectTemplate::New();
		global->Set("plugin", pi.GenerateV8Interface());
		global->Set("natives", NativeInterop::GenerateV8Interface());

	}
}