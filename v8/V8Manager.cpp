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
		reqMan = new Require::RequireManager(sm, libsys, depMan);
		this->sm = sm;
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

			string fake_package_id = "__nopak__" + slocation;
			depMan->ResetAliases(fake_package_id);
			depMan->Depend(fake_package_id, "sourcemod", ">= 0");
			return new SPEmulation::PluginRuntime(isolate, reqMan, scriptLoader->LoadScript(slocation));
	}

	SourcePawn::IPluginRuntime *Manager::LoadPakPlugin(const string& package_name)
	{
		try
		{
			string fake_package_id = "__pakplugin__" + package_name;
			depMan->ResetAliases(fake_package_id);
			depMan->Depend(fake_package_id, package_name, ">= 0");
			string script_path = string("v8/packages/") + depMan->ResolvePath(fake_package_id,package_name) + "/main";

			return new SPEmulation::PluginRuntime(isolate, reqMan, scriptLoader->AutoLoadScript(script_path));
		}
		catch(runtime_error& err)
		{
			throw runtime_error("Can't load package-based plugin: " + string(err.what()));
		}
	}

	Manager::~Manager(void)
	{
		delete reqMan;
		delete depMan;
		delete scriptLoader;
	}
}