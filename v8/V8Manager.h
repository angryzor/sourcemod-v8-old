#ifndef _INCLUDE_V8_V8MANAGER_H_
#define _INCLUDE_V8_V8MANAGER_H_

#include <v8.h>

#include "PluginInfo.h"
#include <ISourceMod.h>
#include <sp_vm_api.h>
#include <string>

namespace SMV8
{
	using namespace v8;
	using namespace SourceMod;

	class Manager
	{
	public:
		Manager(ISourceMod *sm);
		virtual void LoadCoffeeCompiler();
		virtual SourcePawn::IPluginRuntime *LoadPlugin(std::string location);
		virtual std::string CompileCoffee(const std::string& coffee);
		virtual ~Manager(void);
	private:
		Handle<Object> BuildGlobalObject(PluginInfo& pi) const;
		Persistent<Context> coffeeCompilerContext;
		Isolate *isolate;
		ISourceMod *sm;
	};
}

#endif // !defined _INCLUDE_V8_V8MANAGER_H_
