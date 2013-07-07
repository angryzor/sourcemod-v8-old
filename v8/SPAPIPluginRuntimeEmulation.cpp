#include "SPAPIEmulation.h"
#include <IShareSys.h>
#include <string>

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace SourceMod;
		using namespace v8;

		struct sm_plugininfo_s_t
		{
			std::string name;
			std::string description;
			std::string author;
			std::string version;
			std::string url;
		};

		struct sm_plugininfo_c_t
		{
			cell_t name;
			cell_t description;
			cell_t author;
			cell_t version;
			cell_t url;
		};

		PluginRuntime::PluginRuntime(Isolate* isolate, std::string code)
			: pauseState(false), isolate(isolate)
		{
			HandleScope handle_scope(isolate);

			// Create a new context for this plugin, store it in a persistent handle.
			Handle<Context> ourContext = Context::New(isolate, NULL, GenerateGlobalObject());
			v8Context.Reset(isolate, ourContext);

			Handle<Script> script = Script::Compile(String::New(code.c_str()));
			script->Run();

			ExtractPluginInfo();
		}

		PluginRuntime::~PluginRuntime()
		{
			v8Context.Dispose();
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
			plugin->Set("info",ObjectTemplate::New()->NewInstance());
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
		void PluginRuntime::ExtractPluginInfo()
		{
			HandleScope handle_scope(isolate);
			sm_plugininfo_s_t realInfo;
			sm_plugininfo_c_t emulatedInfo;

			Handle<Context> context(Handle<Context>::New(isolate,v8Context));
			Handle<Object> global(context->Global());
			Handle<Object> plugin(global->Get(String::New("plugin")).As<Object>());
			Handle<Object> info(plugin->Get(String::New("info")).As<Object>());

			realInfo.name = *String::AsciiValue(info->Get(String::New("name")).As<String>());
			realInfo.description = *String::AsciiValue(info->Get(String::New("description")).As<String>());
			realInfo.author = *String::AsciiValue(info->Get(String::New("author")).As<String>());
			realInfo.version = *String::AsciiValue(info->Get(String::New("version")).As<String>());
			realInfo.url = *String::AsciiValue(info->Get(String::New("url")).As<String>());

			LoadEmulatedString(realInfo.name, emulatedInfo.name);
			LoadEmulatedString(realInfo.description, emulatedInfo.description);
			LoadEmulatedString(realInfo.author, emulatedInfo.author);
			LoadEmulatedString(realInfo.version, emulatedInfo.version);
			LoadEmulatedString(realInfo.url, emulatedInfo.url);

			cell_t local_addr;
			cell_t *phys_addr;
			ctx.HeapAlloc(sizeof(sm_plugininfo_c_t) / sizeof(cell_t), &local_addr, &phys_addr);
			memcpy(phys_addr, &emulatedInfo, sizeof(sm_plugininfo_c_t));

			PubvarData pd;
			pd.local_addr = local_addr;
			pd.pubvar.name = "myinfo";
			pd.pubvar.offs = phys_addr;

			pubvars.push_back(pd);
		}

		void PluginRuntime::LoadEmulatedString(const std::string& realstr, cell_t& local_addr_target)
		{
			cell_t local_addr;
			cell_t *phys_addr;
			ctx.HeapAlloc(realstr.size() + 1, &local_addr, &phys_addr);
			ctx.StringToLocal(local_addr, realstr.size(), realstr.c_str());
			local_addr_target = local_addr;
		}

		int PluginRuntime::GetPubvarByIndex(uint32_t index, sp_pubvar_t **pubvar)
		{
			if(index >= pubvars.size())
				return SP_ERROR_INDEX;

			*pubvar = &pubvars[index].pubvar;

			return SP_ERROR_NONE;
		}

		int PluginRuntime::FindPubvarByName(const char *name, uint32_t *index)
		{
			for(uint32_t i = 0; i < pubvars.size(); i++)
			{
				if(!strcmp(pubvars[i].pubvar.name,name))
				{
					*index = i;
				}
			}

			return SP_ERROR_NOT_FOUND;
		}

		int PluginRuntime::GetPubvarAddrs(uint32_t index, cell_t *local_addr, cell_t **phys_addr)
		{
			if(index >= pubvars.size())
				return SP_ERROR_INDEX;

			*local_addr = pubvars[index].local_addr;
			*phys_addr = pubvars[index].pubvar.offs;

			return SP_ERROR_NONE;
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
			return &ctx;
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
			return 0;
		}

		unsigned char *PluginRuntime::GetCodeHash()
		{
			return (unsigned char *)"34234663464";
		}

		unsigned char *PluginRuntime::GetDataHash()
		{
			return (unsigned char *)"453253543";
		}	
	}
}