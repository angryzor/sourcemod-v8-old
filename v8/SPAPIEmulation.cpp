#include "SPAPIEmulation.h"

namespace SMV8
{
	namespace SPEmulation
	{
		IVirtualMachine *PluginContext::GetVirtualMachine()
		{
			return NULL;
		}

		sp_context_t *PluginContext::GetContext()
		{
			// TODO: Don't know what to do with this. It's deprecated, evil and doesn't cooperate with my emulation
			return NULL;
		}

		bool PluginContext::IsDebugging()
		{
			return true;
		}

		int PluginContext::SetDebugBreak(void *newpfn, void *oldpfn)
		{
			return SP_ERROR_ABORTED;
		}

		IPluginDebugInfo *PluginContext::GetDebugInfo()
		{
			return NULL;
		}

		/* "heap" management -_-' */
		int PluginContext::HeapAlloc(unsigned int cells, cell_t *local_addr, cell_t **phys_addr)
		{
			cell_t* mem = new cell_t[cells];
			*local_addr = allocations.size();
			allocations.push_back(mem);
			*phys_addr = mem;

			return SP_ERROR_NONE;
		}

		int PluginContext::HeapPop(cell_t local_addr)
		{
			if(allocations.empty() || local_addr != allocations.size() - 1)
			{
				return SP_ERROR_INVALID_ADDRESS;
			}

			allocations.pop_back();

			return SP_ERROR_NONE;
		}

		int PluginContext::HeapRelease(cell_t local_addr)
		{
			if(allocations.empty() || local_addr < 0 || local_addr >= allocations.size())
			{
				return SP_ERROR_INVALID_ADDRESS;
			}

			allocations.erase(allocations.begin() + local_addr, allocations.end());

			return SP_ERROR_NONE;
		}
		
		// Why the hell are these not only in the IPluginRuntime if you can just get the runtime with GetRuntime...
		IPluginFunction *PluginContext::GetFunctionById(funcid_t func_id)
		{
			return parentRuntime->GetFunctionById(func_id);
		}

		IPluginFunction *PluginContext::GetFunctionByName(const char *public_name)
		{
			return parentRuntime->GetFunctionByName(public_name);
		}

		int PluginContext::FindNativeByName(const char *name, uint32_t *index)
		{
			return parentRuntime->FindNativeByName(name, index);
		}

		int PluginContext::GetNativeByIndex(uint32_t index, sp_native_t **native)
		{
			return parentRuntime->GetNativeByIndex(index, native);
		}

		uint32_t PluginContext::GetNativesNum()
		{
			return parentRuntime->GetNativesNum();
		}

		int PluginContext::FindPublicByName(const char *name, uint32_t *index)
		{
			return parentRuntime->FindPublicByName(name, index);
		}

		int PluginContext::GetPublicByIndex(uint32_t index, sp_public_t **pblic)
		{
			return parentRuntime->GetPublicByIndex(index, pblic);
		}

		uint32_t PluginContext::GetPublicsNum()
		{
			return parentRuntime->GetPublicsNum();
		}

		int PluginContext::GetPubvarByIndex(uint32_t index, sp_pubvar_t **pubvar)
		{
			return parentRuntime->GetPubvarByIndex(index, pubvar);
		}

		int PluginContext::FindPubvarByName(const char *name, uint32_t *index)
		{
			return parentRuntime->FindPubvarByName(name, index);
		}

		int PluginContext::GetPubvarAddrs(uint32_t index, cell_t *local_addr, cell_t **phys_addr)
		{
			return parentRuntime->GetPubvarAddrs(index, local_addr, phys_addr);
		}

		uint32_t PluginContext::GetPubVarsNum()
		{
			return parentRuntime->GetPubVarsNum();
		}

		int PluginContext::LocalToPhysAddr(cell_t local_addr, cell_t **phys_addr)
		{
			if(allocations.empty() || local_addr < 0 || local_addr >= allocations.size())
			{
				return SP_ERROR_INVALID_ADDRESS;
			}

			if(phys_addr)
			{
				*phys_addr = allocations[local_addr];
			}

			return SP_ERROR_NONE;
		}

		int PluginContext::LocalToString(cell_t local_addr, char **addr)
		{
			if(allocations.empty() || local_addr < 0 || local_addr >= allocations.size())
			{
				return SP_ERROR_INVALID_ADDRESS;
			}

			*addr = (char *)allocations[local_addr];

			return SP_ERROR_NONE;
		}

		int PluginContext::StringToLocal(cell_t local_addr, size_t bytes, const char *source)
		{
			char *dest;
			size_t len;

			LocalToString(local_addr, &dest);

			len = strlen(source);
			if(len >= bytes)
			{
				len = bytes - 1;
			}

			memmove(dest, source, len);
			dest[len] = '\0';

			return SP_ERROR_NONE;
		}

		int PluginContext::StringToLocalUTF8(cell_t local_addr, 
										size_t maxbytes, 
										const char *source, 
										size_t *wrtnbytes)
		{
		}

		int PluginContext::PushCell(cell_t value)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::PushCellArray(cell_t *local_addr, cell_t **phys_addr, cell_t array[], unsigned int numcells)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::PushString(cell_t *local_addr, char **phys_addr, const char *string)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::PushCellsFromArray(cell_t array[], unsigned int numcells)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::BindNatives(const sp_nativeinfo_t *natives, unsigned int num, int overwrite)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::BindNative(const sp_nativeinfo_t *native)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::BindNativeToAny(SPVM_NATIVE_FUNC native)
		{
			return SP_ERROR_ABORTED;
		}

		int PluginContext::Execute(uint32_t code_addr, cell_t *result)
		{
			return SP_ERROR_ABORTED;
		}

		cell_t PluginContext::ThrowNativeErrorEx(int error, const char *msg, ...){}
		cell_t PluginContext::ThrowNativeError(const char *msg, ...){}
		IPluginFunction *PluginContext::GetFunctionByName(const char *public_name){}
		IPluginFunction *PluginContext::GetFunctionById(funcid_t func_id){}
		SourceMod::IdentityToken_t *PluginContext::GetIdentity(){}
		cell_t *PluginContext::GetNullRef(SP_NULL_TYPE type){}
		int PluginContext::LocalToStringNULL(cell_t local_addr, char **addr){}

		int PluginContext::BindNativeToIndex(uint32_t index, SPVM_NATIVE_FUNC native)
		{
			return SP_ERROR_ABORTED;
		}

		bool PluginContext::IsInExec(){}

		IPluginRuntime *PluginContext::GetRuntime()
		{
			return parentRuntime;
		}

		int PluginContext::Execute2(IPluginFunction *function, 
			const cell_t *params, 
			unsigned int num_params,
			cell_t *result)
		{
		}

		int PluginContext::GetLastNativeError()
		{
			return nativeError;
		}

		cell_t *PluginContext::GetLocalParams(){}
		void PluginContext::SetKey(int k, void *value)
		{
			if (k < 1 || k > 4)
			{
				return;
			}

			keys[k - 1] = value;
			keys_set[k - 1] = true;
		}

		bool PluginContext::GetKey(int k, void **value)
		{
			if (k < 1 || k > 4 || keys_set[k - 1] == false)
			{
				return false;
			}

			*value = keys[k - 1];

			return true;
		}

		void PluginContext::ClearLastNativeError()
		{
			nativeError = SP_ERROR_NONE;
		}
	}
}
