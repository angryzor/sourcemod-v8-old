#include "Marshaller.h"
#include <math.h>

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
//			if((size_t)info.Length() > native.params.size())
//				throw runtime_error("Not enough parameters for native call");

			HandleScope handle_scope(&isolate);

			cell_t params[SP_MAX_EXEC_PARAMS];

			params[0] = info.Length();

			for(int i = 0; i < std::min(info.Length(), SP_MAX_EXEC_PARAMS); i++)
			{
				PushParam(info[i], &params[i+1]);
			}

			cell_t result = native.state.pfn(&ctx, params);

			CopyBackRefs();

			return handle_scope.Close(GetResult(result));
		}

		void V8ToSPMarshaller::PushParam(Handle<Value> val, cell_t* param_dst)
		{
			if(val->IsObject())
				PushByRef(val.As<Object>(), param_dst);
			else if(val->IsInt32())
				PushInt(val.As<Integer>(), param_dst);
			else if(val->IsNumber())
				PushFloat(val.As<Number>(), param_dst);
			else if(val->IsArray() || val->IsString())
				PushByRef(WrapInDummyObject(val), param_dst);
			else 
				throw runtime_error("Unacceptable argument type");
		}

		Handle<Object> V8ToSPMarshaller::WrapInDummyObject(Handle<Value> val)
		{
			HandleScope handle_scope(&isolate);
			Handle<Object> dummy = Object::New();
			dummy->Set(String::New("value"), val);
			return handle_scope.Close(dummy);
		}

		void V8ToSPMarshaller::PushByRef(Handle<Object> val, cell_t* param_dst)
		{
			HandleScope handle_scope(&isolate);
			Handle<Value> realVal = val->Get(String::New("value"));

			if(realVal->IsString()) {
				PushString(realVal.As<String>(), val, param_dst);
				return;
			}

			cell_t dst_local;
			cell_t* dst_phys;
			ctx.HeapAlloc(1, &dst_local, &dst_phys);

			ReferenceInfo *ri = new ReferenceInfo;
			ri->addr = dst_local;
			ri->refObj.Reset(&isolate, val);
			refs.push(ri);
			
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

		void V8ToSPMarshaller::PushString(Handle<String> val, Handle<Object> refObj, cell_t* param_dst)
		{
			string str = *String::Utf8Value(val);
			size_t bytes_required = str.size() + 1;

			// If this ref is carrying a size parameter for us, we set that size instead.
			if(refObj->Has(String::New("size")))
				bytes_required = refObj->Get(String::New("size")).As<Integer>()->Value();

			/* Calculate cells required for the string */
			size_t cells_required = (bytes_required + sizeof(cell_t) - 1) / sizeof(cell_t);
			
			cell_t dst_local;
			cell_t* dst_phys;
			ctx.HeapAlloc(cells_required, &dst_local, &dst_phys);

			ReferenceInfo *ri = new ReferenceInfo;
			ri->addr = dst_local;
			ri->refObj.Reset(&isolate, refObj);
			refs.push(ri);

			ctx.StringToLocalUTF8(dst_local, bytes_required, str.c_str(), NULL);
			*param_dst = dst_local;
		}

		Handle<Value> V8ToSPMarshaller::GetResult(cell_t result)
		{
			return native.resultType == FLOAT ?
			       Number::New(sp_ctof(result)) :
			       Integer::New(result);
		}

		void V8ToSPMarshaller::CopyBackRefs()
		{
			HandleScope handle_scope(&isolate);
			Handle<Value> valueStr = String::New("value");
			for(int i = refs.size() - 1; i >= 0; i--)
			{
				ReferenceInfo *ri = refs.top();
				Handle<Object> refObj = Handle<Object>::New(&isolate, ri->refObj);

				if(refObj->Get(valueStr)->IsInt32())
					refObj->Set(valueStr, PopIntRef());
				else if(refObj->Get(valueStr)->IsNumber())
					refObj->Set(valueStr, PopFloatRef());
				else if(refObj->Get(valueStr)->IsString())
					refObj->Set(valueStr, PopString());

				ri->refObj.Dispose();
				delete ri;
				refs.pop();
			}
		}

		Handle<Integer> V8ToSPMarshaller::PopIntRef()
		{
			cell_t addr = refs.top()->addr;
			cell_t* phys_addr;

			ctx.LocalToPhysAddr(addr, &phys_addr);

			cell_t val = *phys_addr;

			ctx.HeapPop(addr);

			return Integer::New(val).As<Integer>();
		}

		Handle<Number> V8ToSPMarshaller::PopFloatRef()
		{
			cell_t addr = refs.top()->addr;
			cell_t* phys_addr;

			ctx.LocalToPhysAddr(addr, &phys_addr);

			float val = sp_ctof(*phys_addr);

			ctx.HeapPop(addr);

			return Number::New(val);
		}

		Handle<String> V8ToSPMarshaller::PopString()
		{
			HandleScope handle_scope(&isolate);
			cell_t addr = refs.top()->addr;
			char* str;

			ctx.LocalToString(addr, &str);

			Handle<String> result = String::New(str);

			ctx.HeapPop(addr);

			return handle_scope.Close(result);
		}
	}
}