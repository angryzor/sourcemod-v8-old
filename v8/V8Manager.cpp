#include "V8Manager.h"
#include <sm_platform.h>
#include <fstream>
#include <sstream>
#include "SPAPIEmulation.h"


namespace SMV8
{
	using namespace std;
	using namespace v8;
	using namespace SourceMod;

	Manager::Manager()
	{
	}

	void Manager::Initialize(ISourceMod *sm)
	{
		isolate = Isolate::GetCurrent();
		HandleScope handle_scope(isolate);
		coffeeCompilerContext.Reset(isolate, Context::New(isolate));
		LoadCoffeeCompiler(sm);
	}

	void Manager::LoadCoffeeCompiler(ISourceMod *sm)
	{
		HandleScope handle_scope(isolate);

		char fullpath[PLATFORM_MAX_PATH];
		sm->BuildPath(Path_SM, fullpath, sizeof(fullpath), "v8/coffee_compiler.js");

		ifstream ifs(fullpath);
		if(!ifs.is_open())
			throw runtime_error("CoffeeScript compiler not found.");

		ostringstream oss;
		oss << ifs.rdbuf();

		Handle<Context> context = Handle<Context>::New(isolate, coffeeCompilerContext);
		Context::Scope context_scope(context);

		Handle<Script> coffeeCompiler = Script::Compile(String::New(oss.str().c_str()));
		coffeeCompiler->Run();
	}

	std::string Manager::CompileCoffee(const std::string& coffee) const
	{
		HandleScope handle_scope(isolate);
		Handle<Context> context = Handle<Context>::New(isolate,coffeeCompilerContext);
		Handle<Object> coffeescript = context->Global()->Get(String::New("CoffeeScript")).As<Object>();

		const int argc = 1;
		Handle<Value> argv[argc] = { String::New(coffee.c_str()) };

		Handle<Value> result = coffeescript->Get(String::New("compile")).As<Function>()->Call(coffeescript, argc, argv);
		String::Utf8Value jsutf8(result.As<String>());
		return *jsutf8;
	}

	SourcePawn::IPluginRuntime *Manager::LoadPlugin(std::string location)
	{
		ifstream ifs(location);
		if(!ifs.is_open())
			throw runtime_error("Plugin can't be loaded");

		ostringstream oss;
		oss << ifs.rdbuf();

		std::string code(oss.str());

		if(location.find(".coffee", location.size() - 7) != string::npos)
			code = CompileCoffee(code);

		return new SPEmulation::PluginRuntime(isolate, code);
	}

	Manager::~Manager(void)
	{
		coffeeCompilerContext.Dispose();
	}
}