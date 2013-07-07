#include "SPAPIEmulation.h"
#include <IShareSys.h>
#include <string>

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace SourceMod;
		using namespace v8;

		PluginRuntime::PluginRuntime()
			: pauseState(false)
		{
			isolate = Isolate::GetCurrent();
			HandleScope handle_scope(isolate);

			// Create a new context for this plugin, store it in a persistent handle.
			Handle<Context> ourContext = Context::New(isolate, NULL, GenerateGlobalObject());
			v8Context = new Persistent<Context>(isolate, ourContext);
		}

		PluginRuntime::~PluginRuntime()
		{
			delete v8Context;
		}

		/**
		 * Generate the global object
		 */
		Handle<ObjectTemplate> PluginRuntime::GenerateGlobalObject()
		{
			HandleScope handle_scope(isolate);
			Handle<ObjectTemplate> global(ObjectTemplate::New());
			global->Set("natives",GenerateNativesObject()->NewInstance());
			global->Set("plugin",GeneratePluginObject()->NewInstance());
			//global->Set("forwards",GenerateForwardsObject());
			return handle_scope.Close(global);
		}

		/**
		 * Generate the natives object
		 */
		Handle<ObjectTemplate> PluginRuntime::GenerateNativesObject()
		{
			HandleScope handle_scope(isolate);
			Handle<ObjectTemplate> natives(ObjectTemplate::New());
			Handle<External> self = External::New(this);
			natives->Set("declare", FunctionTemplate::New(DeclareNative,self));
			return handle_scope.Close(natives);
		}

		static cell_t InvalidV8Native(IPluginContext *pCtx, const cell_t *params)
		{
			return pCtx->ThrowNativeErrorEx(SP_ERROR_INVALID_NATIVE, "Invalid native");
		}

		void PluginRuntime::DeclareNative(const FunctionCallbackInfo<Value>& info)
		{
			PluginRuntime* self = (PluginRuntime*)info.Data().As<External>()->Value();
			HandleScope(self->isolate);

			if(info.Length() != 2)
				ThrowException(String::New("Invalid argument count"));

			Handle<String> nativeName = info[0].As<String>();
			Handle<Array> signature = info[1].As<Array>();
			String::AsciiValue nativeNameAscii(nativeName);

			NativeData nd;
			nd.name = *nativeNameAscii;
			self->InsertNativeParams(nd, signature);
			nd.state.flags = 0;
			nd.state.pfn = InvalidV8Native;
			nd.state.status = SP_NATIVE_UNBOUND;
			nd.state.name = nd.name.c_str();
			nd.state.user = reinterpret_cast<void *>(self->natives.size());
			self->natives.push_back(nd);
		}

		void PluginRuntime::InsertNativeParams(NativeData& nd, Handle<Array> signature)
		{
			HandleScope(isolate);

			for(int i = 0; i < signature->Length(); i++)
			{
				Handle<Object> paramInfo = signature->Get(i).As<Object>();
				nd.params.push_back(CreateNativeParamInfo(paramInfo));
			}
		}

		NativeParamInfo PluginRuntime::CreateNativeParamInfo(Handle<Object> paramInfo)
		{
			Handle<String> name = paramInfo->Get(String::New("name")).As<String>();
			Handle<Integer> type = paramInfo->Get(String::New("type")).As<Integer>();

			String::AsciiValue nameAscii(name);

			return NativeParamInfo(*nameAscii, (NativeParamType)type->Value());
		}

		Handle<ObjectTemplate> PluginRuntime::GeneratePluginObject()
		{
			HandleScope handle_scope(isolate);
			Handle<ObjectTemplate> plugin = ObjectTemplate::New();
			return handle_scope.Close(plugin);
		}

		IPluginDebugInfo *PluginRuntime::GetDebugInfo()
		{

		}

		int PluginRuntime::FindNativeByName(const char *name, uint32_t *index)
		{
			for(uint32_t i = 0; i < natives.size(); i++)
			{
				if(natives[i].name == name)
				{
					*index = i;
				}
			}

			return SP_ERROR_NOT_FOUND;
		}

		int PluginRuntime::GetNativeByIndex(uint32_t index, sp_native_t **native)
		{
			if(index >= natives.size())
				return SP_ERROR_INDEX;

			*native = &natives[index].state;

			return SP_ERROR_NONE;
		}

		uint32_t PluginRuntime::GetNativesNum()
		{
			return natives.size();
		}

		// TODO: Implement forwards
		int PluginRuntime::FindPublicByName(const char *name, uint32_t *index)
		{
			return SP_ERROR_NOT_FOUND;
		}

		int PluginRuntime::GetPublicByIndex(uint32_t index, sp_public_t **publicptr)
		{
			return SP_ERROR_INDEX;
		}

		uint32_t PluginRuntime::GetPublicsNum()
		{
			return publics.size();
		}

		/*
		 * Fake pubvars to publish dependencies
		 */
		int PluginRuntime::GetPubvarByIndex(uint32_t index, sp_pubvar_t **pubvar)
		{
		}

		int PluginRuntime::FindPubvarByName(const char *name, uint32_t *index)
		{
		}

		int PluginRuntime::GetPubvarAddrs(uint32_t index, cell_t *local_addr, cell_t **phys_addr)
		{
		}

		uint32_t PluginRuntime::GetPubVarsNum()
		{
			return pubvars.size();
		}

		IPluginFunction *PluginRuntime::GetFunctionByName(const char *public_name)
		{
		}

		IPluginFunction *PluginRuntime::GetFunctionById(funcid_t func_id)
		{
		}

		IPluginContext *PluginRuntime::GetDefaultContext()
		{
			return &defaultContext;
		}

		bool PluginRuntime::IsDebugging()
		{
			return true;
		}

		int PluginRuntime::ApplyCompilationOptions(ICompilation *co)
		{
			return false;
		}

		void PluginRuntime::SetPauseState(bool paused)
		{
			pauseState = paused;
		}

		bool PluginRuntime::IsPaused()
		{
			return pauseState;
		}

		size_t PluginRuntime::GetMemUsage()
		{
		}

		unsigned char *PluginRuntime::GetCodeHash()
		{
		}

		unsigned char *PluginRuntime::GetDataHash()
		{
		}	
	}
}