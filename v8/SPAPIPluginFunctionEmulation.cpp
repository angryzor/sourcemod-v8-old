#include "SPAPIEmulation.h"

namespace SMV8
{
	namespace SPEmulation
	{
		PluginFunction::PluginFunction(PluginRuntime& runtime, funcid_t id) : runtime(runtime), id(id)
		{
		}

		int PluginFunction::Execute(cell_t *result)
		{
			return Execute2(runtime.GetDefaultContext(), result);
		}

		int PluginFunction::CallFunction(const cell_t *params, unsigned int num_params, cell_t *result)
		{
			return CallFunction2(runtime.GetDefaultContext(), params, num_params, result);
		}

		int PluginFunction::CallFunction2(IPluginContext *ctx, 
			const cell_t *params, 
			unsigned int num_params, 
			cell_t *result)
		{
			return runtime.GetDefaultContext()->Execute2(this, params, num_params, result);
		}

		IPluginContext *PluginFunction::GetParentContext()
		{
			return GetParentRuntime()->GetDefaultContext();
		}

		bool PluginFunction::IsRunnable()
		{
			return true;
		}

		funcid_t PluginFunction::GetFunctionID()
		{
			return id;
		}

		int PluginFunction::Execute2(IPluginContext *ctx, cell_t *result)
		{
			return 0;
		}

		IPluginRuntime *PluginFunction::GetParentRuntime()
		{
			return &runtime;
		}
	}
}
