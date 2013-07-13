#include "DependencyManager.h"
#include <sm_platform.h>
#include <fstream>
#include <sstream>

namespace SMV8
{
	namespace Dependencies
	{
		using namespace std;
		using namespace SourceMod;
		using namespace v8;

		DependencyManager::DependencyManager(Isolate *isolate, ISourceMod *sm)
			: isolate(isolate), sm(sm)
		{
		}

		DependencyManager::~DependencyManager()
		{
		}

		void DependencyManager::LoadDependencies(const string& package_path)
		{
			char fullpath[PLATFORM_MAX_PATH];
			sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), (package_path + "Pkgfile").c_str());

			ifstream ifs(fullpath);
			ostringstream oss;

			oss << ifs.rdbuf();

			ParseManifest(oss.str());
		}

		Handle<ObjectTemplate> DependencyManager::BuildGlobalObjectTemplate()
		{
			HandleScope handle_scope(isolate);
			Handle<ObjectTemplate> gt = ObjectTemplate::New();
			gt->Set("pkg",FunctionTemplate::New(&ScriptFun_pkg,External::New(this)));
			handle_scope.Close(gt);
		}

		void DependencyManager::ParseManifest(const string& code)
		{
			HandleScope handle_scope(isolate);
			Handle<ObjectTemplate> global = Handle<ObjectTemplate>::New(isolate, global_template);
			Handle<Context> context = Context::New(isolate, NULL, global);
			Context::Scope context_scope(context);

			Handle<Script> script = Script::Compile(String::New(code.c_str()));
			script->Run();
		}

		void DependencyManager::ScriptFun_pkg(const FunctionCallbackInfo<Value>& info)
		{
			
		}
	}
}