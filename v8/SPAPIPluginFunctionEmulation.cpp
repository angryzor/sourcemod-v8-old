#include "SPAPIEmulation.h"
#include <cmath>

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace SourcePawn;

		PluginFunction::PluginFunction(PluginRuntime& runtime, funcid_t id) : runtime(runtime), id(id), curParam(0)
		{
		}

		int PluginFunction::PushValue(Handle<Value> val)
		{
			params[curParam++].Reset(runtime.isolate, val);
			return SP_ERROR_NONE;
		}

		int PluginFunction::PushCell(cell_t cell)
		{
			return PushValue(Integer::New(cell));
		}

		int PluginFunction::PushCellByRef(cell_t *cell, int flags)
		{
			return PushCell(*cell);
			if(flags & SM_PARAM_COPYBACK)
				refs.push_back(RefData(cell,1));
		}

		int PluginFunction::PushFloat(float number)
		{
			return PushValue(Number::New(number));
		}

		int PluginFunction::PushFloatByRef(float *number, int flags)
		{
			return PushFloat(*number);
			if(flags & SM_PARAM_COPYBACK)
				refs.push_back(RefData((cell_t*)number,1));
		}

		// Problem: No float arrays
		int PluginFunction::PushArray(cell_t *inarray, unsigned int cells, int flags)
		{
			HandleScope handle_scope(runtime.isolate);
			Handle<Array> array = Array::New(cells);

			for(unsigned int i = 0; i < cells; i++)
			{
				array->Set(i, Integer::New(inarray[i]));
			}
			if(flags & SM_PARAM_COPYBACK)
				refs.push_back(RefData(inarray,cells));

			return PushValue(array);
		}

		int PluginFunction::PushString(const char *string)
		{
			return PushValue(String::New(string));
			refs.push_back(RefData((cell_t*)string,strlen(string) + 1));
		}

		int PluginFunction::PushStringEx(char *buffer, size_t length, int sz_flags, int cp_flags)
		{
			return SP_ERROR_ABORTED;
			//PushValue(String::New(buffer));
		}

		void PluginFunction::Cancel()
		{
			for(int i = 0; i < curParam; i++)
			{
				params[i].Dispose();
			}
			curParam = 0;
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
			PluginContext* pcontext = static_cast<PluginContext*>(ctx);
			HandleScope handle_scope(runtime.isolate);

			Handle<Context> context = Handle<Context>::New(runtime.isolate, runtime.v8Context);
			Context::Scope context_scope(context);

			Handle<Value> args[SP_MAX_EXEC_PARAMS];

			for(int i = 0; i < curParam; i++)
			{
				args[i] = Handle<Value>::New(runtime.isolate, params[i]);
			}

			Handle<Value> res = pcontext->ExecuteV8(this, curParam, args);

			if(res->IsObject())
				ExtractResultValues(res.As<Object>(), result);
			else
				SetSingleCellValue(res, result);

			Cancel();

			return SP_ERROR_NONE;
		}

		void PluginFunction::SetSingleCellValue(Handle<Value> val, cell_t *result)
		{
			if(val->IsInt32())
				*result = (cell_t)val.As<Integer>()->Value();
			else if(val->IsNumber())
				*result = sp_ftoc((float)val.As<Number>()->Value());
		}

		void PluginFunction::CopyBackString(Handle<String> val, RefData& ref)
		{
			// TODO: UTF-8

			String::AsciiValue ascii(val);
			size_t len = std::min((size_t)ascii.length() + 1,ref.size);
			memcpy(ref.addr, *ascii, len);
			ref.addr[len - 1] = '\0';
		}

		void PluginFunction::CopyBackArray(Handle<Array> val, RefData& ref)
		{
			for(unsigned int i = 0; i < std::min(val->Length(), ref.size); i++)
			{
				SetSingleCellValue(val->Get(i), &ref.addr[i]);
			}
		}

		void PluginFunction::ExtractResultValues(Handle<Object> resObj, cell_t *result)
		{
			HandleScope handle_scope(runtime.isolate);

			SetSingleCellValue(resObj->Get(String::New("result")), result);

			Handle<Array> refArr = resObj->Get(String::New("refs")).As<Array>();
			for(unsigned int i = 0; i < std::min(refArr->Length(), refs.size()); i++)
			{
				Handle<Value> val = refArr->Get(i);
				if(val->IsString())
					CopyBackString(val.As<String>(), refs[i]);
				else if(val->IsArray())
					CopyBackArray(val.As<Array>(), refs[i]);
				else 
					SetSingleCellValue(val, refs[i].addr);
			}
		}

		IPluginRuntime *PluginFunction::GetParentRuntime()
		{
			return &runtime;
		}
	}
}
