#include "V8Manager.h"

#include "SPAPIEmulation.h"


namespace SMV8
{
	using namespace std;
	using namespace v8;
	using namespace SourceMod;

	Manager::Manager()
	{
	}

	void Manager::Initialize(ISourceMod *sm, ILibrarySys *libsys)
	{
		isolate = Isolate::GetCurrent();
		HandleScope handle_scope(isolate);
		scriptLoader = new ScriptLoader(isolate, sm);
		depMan = new DependencyManager(isolate, sm, libsys, scriptLoader);
	}


	SourcePawn::IPluginRuntime *Manager::LoadPlugin(char* location)
	{
		string slocation(location);

		auto pakPluginLoc = slocation.find(".pakplugin", slocation.size() - 10);

		if(pakPluginLoc != string::npos)
		{
			auto afterSlash = slocation.find_last_of("/") + 1;
			return LoadPakPlugin(slocation.substr(afterSlash, slocation.find(".coffee",pakPluginLoc - afterSlash)));	
		}

		return new SPEmulation::PluginRuntime(isolate, scriptLoader->LoadScript(slocation));
	}

	SourcePawn::IPluginRuntime *Manager::LoadPakPlugin(const string& package_name)
	{
		try
		{
			depMan->Depend(package_name, ">= 0");
			string script_path = string("v8/packages/") + depMan->ResolvePath("__depend__",package_name) + "/main";

			return new SPEmulation::PluginRuntime(isolate, scriptLoader->AutoLoadScript(script_path));
		}
		catch(runtime_error& err)
		{
			throw runtime_error("Can't load package-based plugin: " + string(err.what()));
		}
	}

	Manager::~Manager(void)
	{
		delete depMan;
		delete scriptLoader;
	}
}