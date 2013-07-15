#ifndef _INCLUDE_V8_MARSHALLER_H_
#define _INCLUDE_V8_MARSHALLER_H_

#include "SPAPIEmulation.h"
#include <v8.h>
#include <stack>
#include <string>

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace v8;

		struct ReferenceInfo
		{
			Persistent<Object> refObj;
			cell_t addr;
		};

		class V8ToSPMarshaller
		{
		public:
			V8ToSPMarshaller(Isolate& isolate, NativeData& native);
			virtual ~V8ToSPMarshaller();
			Handle<Value> HandleNativeCall(const FunctionCallbackInfo<Value>& info);
		private:
			void PushParam(Handle<Value> val, cell_t* param_dst, bool forcefloat);
			void PushObject(Handle<Object> val, cell_t* param_dst, bool forcefloat);
			void PushInt(Handle<Integer> val, cell_t* param_dst);
			void PushFloat(Handle<Number> val, cell_t* param_dst);
			void PushBool(Handle<Boolean> val, cell_t* param_dst);
			void PushByRef(Handle<Object> val, cell_t* param_dst, bool forcefloat);
			void PushArray(Handle<Array> val, Handle<Object> refObj, cell_t* param_dst, bool forcefloat);
			void PushString(Handle<String> val, Handle<Object> refObj, cell_t* param_dst);
			void PushFunction(Handle<Function> val, cell_t* param_dst);
			Handle<Object> WrapInDummyObject(Handle<Value> val);
			Handle<Value> GetResult(cell_t result);
			Handle<Integer> PopIntRef();
			Handle<Number> PopFloatRef();
			Handle<String> PopString();
			void CopyBackRefs();
		private:
			PluginRuntime& runtime;
			IPluginContext& ctx;
			NativeData& native;
			std::stack<ReferenceInfo*> refs;
			Isolate& isolate;
		};
	}
}

#endif // !_INCLUDE_V8_MARSHALLER_H_
