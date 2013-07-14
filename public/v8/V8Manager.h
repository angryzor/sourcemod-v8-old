#ifndef _INCLUDE_V8_V8MANAGER_H_
#define _INCLUDE_V8_V8MANAGER_H_

#include <v8.h>

#include <ISourceMod.h>
#include <sp_vm_api.h>
#include <string>
#include "ScriptLoader.h"
#include "DependencyManager.h"
#include <ILibrarySys.h>

namespace SMV8
{
	using namespace v8;
	using namespace SourceMod;

	class IManager
	{
	public:
		virtual void Initialize(ISourceMod *sm, ILibrarySys *libsys) = 0;
		virtual SourcePawn::IPluginRuntime *LoadPlugin(char* location) = 0;
	};

	class Manager : public IManager
	{
	public:
		Manager();
		virtual void Initialize(ISourceMod *sm, ILibrarySys *libsys);
		virtual SourcePawn::IPluginRuntime *LoadPlugin(char* location);
		SourcePawn::IPluginRuntime *LoadPakPlugin(const string& package_name);
		virtual ~Manager(void);
	private:
		Isolate *isolate;
		ScriptLoader *scriptLoader;
		DependencyManager *depMan;
	};
}

typedef SMV8::IManager* (*GET_V8)();

#endif // !defined _INCLUDE_V8_V8MANAGER_H_
