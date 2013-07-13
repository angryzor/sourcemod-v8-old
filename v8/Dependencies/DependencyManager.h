#pragma once

#include <ISourceMod.h>
#include <v8.h>
#include <string>
#include <map>

namespace SMV8
{
	namespace Dependencies
	{
		using namespace std;
		using namespace v8;
		using namespace SourceMod;

		class DependencyManager
		{
		public:
			DependencyManager(Isolate *isolate, ISourceMod *sm);
			virtual ~DependencyManager();
			virtual void LoadDependencies(const string& package);
			virtual void GetFileSystem(const string& script);
			void ParseManifest(const string& code);
			void BuildGlobalObjectTemplate();
			static void ScriptFun_pkg(const FunctionCallbackInfo<Value>& info);
		private:
			map<std::string,PackageFS *> filesystems;
			Isolate *isolate;
			ISourceMod *sm;
			Persistent<ObjectTemplate> global_template;
		};
	}
}