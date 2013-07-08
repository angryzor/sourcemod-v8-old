#include "Marshaller.h"

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace std;

		V8ToSPMarshaller::V8ToSPMarshaller(NativeData& native)
			: runtime(*native.runtime), native(native), ctx(*runtime.GetDefaultContext())
		{
		}

		V8ToSPMarshaller::~V8ToSPMarshaller()
		{
		}

		Handle<Value> V8ToSPMarshaller::HandleNativeCall(const FunctionCallbackInfo<Value>& info)
		{
			if(info.Length > native.params.size())
				throw runtime_error("Not enough parameters for native call");

			cell_t params[SP_MAX_EXEC_PARAMS];

			for(unsigned int i = 0; i < info.Length() && i < SP_MAX_EXEC_PARAMS; i++)
			{
				PushParam(info[i], &params[i]);
			}

			cell_t result = native.state.pfn(ctx, params);

			return BuildResultObject(result);
		}

		void V8ToSPMarshaller::PushParam(Handle<Value> val, cell_t* param_dst)
		{
			if(val->IsObject())
				PushByRef(val.As<Object>(), param_dst);
			else if(val->IsInt32())
				PushInt(val.As<Int32>(), param_dst);
			else if(val->IsNumber())
				PushFloat(val.As<Number>(), param_dst);
			else if(val->IsArray())
				PushArray(val.As<Array>(), param_dst);
			else if(val->IsString())
				PushString(val.As<String>(), param_dst);
			else 
				throw runtime_error("Unacceptable argument type");
		}

		void V8ToSPMarshaller::PushByRef(Handle<Object> val, cell_t* param_dst)
		{
			cell_t dst_local;
			cell_t* dst_phys;
			ctx.HeapAlloc(1, &dst_local, &dst_phys);

			SetBaseAddr(dst_local);

			PushParam(val->Get(String::New("value")),dst_phys);
			*param_dst = dst_local;
		}

		void V8ToSPMarshaller::PushInt(Handle<Int32> val, cell_t* param_dst)
		{
			*param_dst = val->Value();
		}

		void V8ToSPMarshaller::PushFloat(Handle<Number> val, cell_t* param_dst)
		{
			float fNumb = val->Value();
			*param_dst = *((cell_t *)(&fNumb));
		}

		void V8ToSPMarshaller::PushArray(Handle<Array> val, cell_t* param_dst)
		{
			// TODO: Add arrays
		}

		void V8ToSPMarshaller::PushString(Handle<String> val, cell_t* param_dst)
		{
			string str = *String::Utf8Value(val);
			
			cell_t dst_local;
			cell_t* dst_phys;
			ctx.HeapAlloc(str.size(), &dst_local, &dst_phys);

			SetBaseAddr(dst_local);

			ctx.StringToLocalUTF8(dst_local, str.size(), str.c_str(), NULL);
			*param_dst = dst_local;
		}
	}
}