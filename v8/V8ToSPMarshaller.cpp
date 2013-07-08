#include "Marshaller.h"

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace std;

		V8ToSPMarshaller::V8ToSPMarshaller(Isolate& isolate, NativeData& native)
			: isolate(isolate), native(native), runtime(*native.runtime), ctx(*runtime.GetDefaultContext())
		{
		}

		V8ToSPMarshaller::~V8ToSPMarshaller()
		{
		}

		Handle<Value> V8ToSPMarshaller::HandleNativeCall(const FunctionCallbackInfo<Value>& info)
		{
			if((size_t)info.Length() > native.params.size())
				throw runtime_error("Not enough parameters for native call");

			cell_t params[SP_MAX_EXEC_PARAMS];

			for(int i = 0; i < info.Length() && i < SP_MAX_EXEC_PARAMS; i++)
			{
				PushParam(info[i], &params[i]);
			}

			cell_t result = native.state.pfn(&ctx, params);

			return BuildResultObject(result);
		}

		void V8ToSPMarshaller::PushParam(Handle<Value> val, cell_t* param_dst)
		{
			if(val->IsObject())
				PushByRef(val.As<Object>(), param_dst);
			else if(val->IsInt32())
				PushInt(val.As<Integer>(), param_dst);
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

			Handle<Value> realVal = val->Get(String::New("value"));

			// I feel sad that i have to add this part. The code flow was so neat... :<
			if(realVal->IsInt32())
				refs.push(ReferenceInfo(INT,dst_local));
			else if(realVal->IsNumber())
				refs.push(ReferenceInfo(FLOAT,dst_local));
			else
				throw runtime_error("Invalid ref type");

			PushParam(realVal,dst_phys);
			*param_dst = dst_local;
		}

		void V8ToSPMarshaller::PushInt(Handle<Integer> val, cell_t* param_dst)
		{
			*param_dst = static_cast<cell_t>(val->Value());
		}

		void V8ToSPMarshaller::PushFloat(Handle<Number> val, cell_t* param_dst)
		{
			float fNumb = static_cast<float>(val->Value());
			*param_dst = sp_ftoc(fNumb);
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

			refs.push(ReferenceInfo(STRING,dst_local));

			ctx.StringToLocalUTF8(dst_local, str.size(), str.c_str(), NULL);
			*param_dst = dst_local;
		}

		Handle<Object> V8ToSPMarshaller::BuildResultObject(cell_t result)
		{
			Handle<ObjectTemplate> resObj = ObjectTemplate::New();
			resObj->Set("result", native.resultType == FLOAT ?
			                      Number::New(sp_ctof(result)) :
			                      Integer::New(result));

			Handle<Array> refArr = Array::New();

			for(int i = refs.size() - 1; i <= 0; i--)
			{
				switch(refs.top().type)
				{
				case INT:
					refArr->Set(i, PopIntRef());
					break;
				case FLOAT:
					refArr->Set(i, PopFloatRef());
					break;
				case STRING:
					refArr->Set(i, PopString());
					break;
				}
				refs.pop();
			}

			resObj->Set("refs", refArr);

			return resObj->NewInstance();
		}

		Handle<Integer> V8ToSPMarshaller::PopIntRef()
		{
			cell_t addr = refs.top().addr;
			cell_t* phys_addr;

			ctx.LocalToPhysAddr(addr, &phys_addr);

			cell_t val = *phys_addr;

			ctx.HeapPop(addr);

			return Integer::New(val).As<Integer>();
		}

		Handle<Number> V8ToSPMarshaller::PopFloatRef()
		{
			cell_t addr = refs.top().addr;
			cell_t* phys_addr;

			ctx.LocalToPhysAddr(addr, &phys_addr);

			float val = sp_ctof(*phys_addr);

			ctx.HeapPop(addr);

			return Number::New(val);
		}

		Handle<String> V8ToSPMarshaller::PopString()
		{
			cell_t addr = refs.top().addr;
			char* str;

			ctx.LocalToString(addr, &str);

			ctx.HeapPop(addr);

			return String::New(str);
		}
	}
}