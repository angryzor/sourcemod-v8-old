#ifndef _INCLUDE_V8_V8MANAGER_H_
#define _INCLUDE_V8_V8MANAGER_H_

#include <v8.h>

#include <ISourceMod.h>
#include <sp_vm_api.h>
#include <string>

namespace SMV8
{
	using namespace v8;
	using namespace SourceMod;

	class IManager
	{
	public:
		virtual void Initialize(ISourceMod *sm) = 0;
		virtual SourcePawn::IPluginRuntime *LoadPlugin(std::string location) = 0;
	};

	class Manager : public IManager
	{
	public:
		Manager();
		virtual void Initialize(ISourceMod *sm);
		virtual SourcePawn::IPluginRuntime *LoadPlugin(std::string location);
		virtual ~Manager(void);
	private:
		void LoadCoffeeCompiler(ISourceMod *sm);
		std::string CompileCoffee(const std::string& coffee) const;
		Persistent<Context> coffeeCompilerContext;
		Isolate *isolate;
	};

}

typedef SMV8::IManager* (*GET_V8)();

#endif // !defined _INCLUDE_V8_V8MANAGER_H_
