#include "SPAPIEmulation.h"

namespace SMV8
{
	namespace SPEmulation
	{
		IPluginDebugInfo *PluginRuntime::GetDebugInfo()
		{
		}

		int PluginRuntime::FindNativeByName(const char *name, uint32_t *index)
		{
		}

		int PluginRuntime::GetNativeByIndex(uint32_t index, sp_native_t **native)
		{
		}

		uint32_t PluginRuntime::GetNativesNum()
		{
			return natives.size();
		}

		int PluginRuntime::FindPublicByName(const char *name, uint32_t *index)
		{
		}

		int PluginRuntime::GetPublicByIndex(uint32_t index, sp_public_t **publicptr)
		{
		}

		uint32_t PluginRuntime::GetPublicsNum()
		{
			return publics.size();
		}

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
		}

		bool PluginRuntime::IsDebugging()
		{
		}

		int PluginRuntime::ApplyCompilationOptions(ICompilation *co)
		{
		}

		void PluginRuntime::SetPauseState(bool paused)
		{
		}

		bool PluginRuntime::IsPaused()
		{
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