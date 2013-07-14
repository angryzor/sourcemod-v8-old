#pragma once
#include <IV8Manager.h>
#include "ScriptLoader.h"
#include "DependencyManager.h"
#include "Require/RequireManager.h"

namespace SMV8
{
	using namespace v8;
	using namespace SourceMod;

	class Manager : public IManager
	{
	public:
		Manager();
		virtual void Initialize(ISourceMod *sm, ILibrarySys *libsys);
		virtual SourcePawn::IPluginRuntime *LoadPlugin(char* filename);
		SourcePawn::IPluginRuntime *LoadPakPlugin(const string& package_name);
		virtual ~Manager(void);
	private:
		Isolate *isolate;
		ScriptLoader *scriptLoader;
		ISourceMod *sm;
		DependencyManager *depMan;
		Require::RequireManager *reqMan;
	};
}