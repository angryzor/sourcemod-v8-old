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
			CellType type;
			cell_t addr;
		};

		class V8ToSPMarshaller
		{
		public:
			V8ToSPMarshaller(NativeData& native);
			virtual ~V8ToSPMarshaller();
			Handle<Value> HandleNativeCall(const FunctionCallbackInfo<Value>& info);
		private:
			void PushParam(Handle<Value> val, cell_t* param_dst);
			void PushInt(Handle<Int32> val, cell_t* param_dst);
			void PushFloat(Handle<Number> val, cell_t* param_dst);
			void PushByRef(Handle<Object> val, cell_t* param_dst);
			void PushArray(Handle<Array> val, cell_t* param_dst);
			void PushString(Handle<String> val, cell_t* param_dst);
			void SetBaseAddr(cell_t addr);
		private:
			PluginRuntime& runtime;
			IPluginContext& ctx;
			NativeData& native;
			std::stack<ReferenceInfo> refs;
		};
	}
}

#endif // !_INCLUDE_V8_MARSHALLER_H_
