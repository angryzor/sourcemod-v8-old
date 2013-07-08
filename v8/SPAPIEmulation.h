#ifndef _INCLUDE_V8_SPAPIEMULATION_H_
#define _INCLUDE_V8_SPAPIEMULATION_H_

#include <sp_vm_api.h>
#include <vector>
#include <cstdarg>
#include <v8.h>

namespace SMV8
{
	namespace SPEmulation
	{
		using namespace SourcePawn;
		using namespace v8;

		enum CellType
		{
			INT,
			FLOAT
		};

		enum NativeParamType
		{
			INT,
			FLOAT,
			INTBYREF,
			FLOATBYREF,
			ARRAY,
			STRING,
			VARARG
		};

		class PluginRuntime;
		class PluginContext;

		struct NativeParamInfo
		{
			NativeParamInfo(std::string name, NativeParamType type) : name(name), type(type)
			{}
			std::string name;
			NativeParamType type;
		};

		struct NativeData
		{
			std::string name;
			std::vector<NativeParamInfo> params;
			sp_native_t state;
			PluginRuntime* runtime;
		};

		struct PubvarData
		{
			cell_t local_addr;
			sp_pubvar_t pubvar;
		};

		class PluginFunction : public IPluginFunction
		{
		public:
			PluginFunction(PluginRuntime& ctx, funcid_t id);
			virtual int Execute(cell_t *result);
			virtual int CallFunction(const cell_t *params, unsigned int num_params, cell_t *result);
			virtual IPluginContext *GetParentContext();
			virtual bool IsRunnable();
			virtual funcid_t GetFunctionID();
			virtual int Execute2(IPluginContext *ctx, cell_t *result);
			virtual int CallFunction2(IPluginContext *ctx, 
				const cell_t *params, 
				unsigned int num_params, 
				cell_t *result);
			virtual IPluginRuntime *GetParentRuntime();
		private:
			PluginRuntime& runtime;
			funcid_t id;
		};

		/* Bridges between the SPAPI and V8 implementation by holding an internal stack only used during 
		   native or cross-plugin calls */
		class PluginContext : public IPluginContext
		{
		public:
			virtual ~PluginContext();
		public:
			virtual IVirtualMachine *GetVirtualMachine();
			virtual sp_context_t *GetContext();
			virtual bool IsDebugging();
			virtual int SetDebugBreak(void *newpfn, void *oldpfn);
			virtual IPluginDebugInfo *GetDebugInfo();
			virtual int HeapAlloc(unsigned int cells, cell_t *local_addr, cell_t **phys_addr);
			virtual int HeapPop(cell_t local_addr);
			virtual int HeapRelease(cell_t local_addr);
			virtual int FindNativeByName(const char *name, uint32_t *index);
			virtual int GetNativeByIndex(uint32_t index, sp_native_t **native);
			virtual uint32_t GetNativesNum();
			virtual int FindPublicByName(const char *name, uint32_t *index);
			virtual int GetPublicByIndex(uint32_t index, sp_public_t **publicptr);
			virtual uint32_t GetPublicsNum();
			virtual int GetPubvarByIndex(uint32_t index, sp_pubvar_t **pubvar);
			virtual int FindPubvarByName(const char *name, uint32_t *index);
			virtual int GetPubvarAddrs(uint32_t index, cell_t *local_addr, cell_t **phys_addr);
			virtual uint32_t GetPubVarsNum();
			virtual int LocalToPhysAddr(cell_t local_addr, cell_t **phys_addr);
			virtual int LocalToString(cell_t local_addr, char **addr);
			virtual int StringToLocal(cell_t local_addr, size_t bytes, const char *source);
			virtual int StringToLocalUTF8(cell_t local_addr, 
										  size_t maxbytes, 
										  const char *source, 
										  size_t *wrtnbytes);

			virtual int PushCell(cell_t value);
			virtual int PushCellArray(cell_t *local_addr, cell_t **phys_addr, cell_t array[], unsigned int numcells);
			virtual int PushString(cell_t *local_addr, char **phys_addr, const char *string);
			virtual int PushCellsFromArray(cell_t array[], unsigned int numcells);
			virtual int BindNatives(const sp_nativeinfo_t *natives, unsigned int num, int overwrite);
			virtual int BindNative(const sp_nativeinfo_t *native);
			virtual int BindNativeToAny(SPVM_NATIVE_FUNC native);
			virtual int Execute(uint32_t code_addr, cell_t *result);
			virtual cell_t ThrowNativeErrorEx(int error, const char *msg, ...);
			virtual cell_t ThrowNativeError(const char *msg, ...);
			virtual cell_t ThrowNativeErrorEx(int error, const char *msg, va_list args);
			virtual cell_t ThrowNativeError(const char *msg, va_list args);
			virtual IPluginFunction *GetFunctionByName(const char *public_name);
			virtual IPluginFunction *GetFunctionById(funcid_t func_id);
			virtual SourceMod::IdentityToken_t *GetIdentity();
			virtual cell_t *GetNullRef(SP_NULL_TYPE type);
			virtual int LocalToStringNULL(cell_t local_addr, char **addr);
			virtual int BindNativeToIndex(uint32_t index, SPVM_NATIVE_FUNC native);
			virtual bool IsInExec();
			virtual IPluginRuntime *GetRuntime();
			virtual int Execute2(IPluginFunction *function, 
				const cell_t *params, 
				unsigned int num_params, 
				cell_t *result);
			virtual int GetLastNativeError();
			virtual cell_t *GetLocalParams();
			virtual void SetKey(int k, void *value);
			virtual bool GetKey(int k, void **value);
			virtual void ClearLastNativeError();
			virtual void CheckHeapSize(unsigned int cells);
		private:
			PluginRuntime *parentRuntime;
			int32_t nativeError;
			void *keys[4]; // <-- What... the... crap...
			bool keys_set[4]; // <-- What... the... crap...
			char* heap;
			char* hp;
			unsigned int heapSize;
			bool inExec;
			std::string errMessage;
		};

		class PluginDebugInfo : public IPluginDebugInfo
		{
		public:
			virtual int LookupFile(ucell_t addr, const char **filename);
			virtual int LookupFunction(ucell_t addr, const char **name);
			virtual int LookupLine(ucell_t addr, uint32_t *line);
		};

		class Compilation : public ICompilation
		{
		public:
			virtual bool SetOption(const char *key, const char *val);
			virtual void Abort();
		};

		class PluginRuntime : public IPluginRuntime
		{
		public:
			PluginRuntime(Isolate* isolate, std::string code);
			virtual ~PluginRuntime();
			virtual IPluginDebugInfo *GetDebugInfo();
			virtual int FindNativeByName(const char *name, uint32_t *index);
			virtual int GetNativeByIndex(uint32_t index, sp_native_t **native);
			virtual uint32_t GetNativesNum();
			virtual int FindPublicByName(const char *name, uint32_t *index);
			virtual int GetPublicByIndex(uint32_t index, sp_public_t **publicptr);
			virtual uint32_t GetPublicsNum();
			virtual int GetPubvarByIndex(uint32_t index, sp_pubvar_t **pubvar);
			virtual int FindPubvarByName(const char *name, uint32_t *index);
			virtual int GetPubvarAddrs(uint32_t index, cell_t *local_addr, cell_t **phys_addr);
			virtual uint32_t GetPubVarsNum();
			virtual IPluginFunction *GetFunctionByName(const char *public_name);
			virtual IPluginFunction *GetFunctionById(funcid_t func_id);
			virtual IPluginContext *GetDefaultContext();
			virtual bool IsDebugging();
			virtual int ApplyCompilationOptions(ICompilation *co);
			virtual void SetPauseState(bool paused);
			virtual bool IsPaused();
			virtual size_t GetMemUsage();
			virtual unsigned char *GetCodeHash();
			virtual unsigned char *GetDataHash();
		protected:
			virtual Handle<ObjectTemplate> GenerateGlobalObject();
			virtual Handle<ObjectTemplate> GenerateNativesObject();
			virtual Handle<ObjectTemplate> GeneratePluginObject();
			static void DeclareNative(const FunctionCallbackInfo<Value>& info);
			virtual void InsertNativeParams(NativeData& nd, Handle<Array> signature);
			virtual NativeParamInfo CreateNativeParamInfo(Handle<Object> paramInfo);
			virtual void ExtractPluginInfo();
			virtual void LoadEmulatedString(const std::string& realstr, cell_t& local_addr_target);
			virtual void RegisterNativeInNativesObject(NativeData& native);
			static void NativeRouter(const FunctionCallbackInfo<Value>& info);
		private:
			std::vector<NativeData> natives;
			std::vector<sp_public_t> publics;
			std::vector<PubvarData> pubvars;
			bool pauseState;
			Isolate* isolate;
			Persistent<Context> v8Context;
			PluginContext ctx;
			Persistent<Object> nativesObj;
		};


		class ContextTrace : public IContextTrace
		{
		public:
			virtual int GetErrorCode();
			virtual const char *GetErrorString();
			virtual bool DebugInfoAvailable();
			virtual const char *GetCustomErrorString();
			virtual bool GetTraceInfo(CallStackInfo *trace);
			virtual void ResetTrace();
			virtual const char *GetLastNative(uint32_t *index);
		};

		class DebugListener : public IDebugListener
		{
		public:
			virtual void OnContextExecuteError(IPluginContext *ctx, IContextTrace *error);
			virtual void OnDebugSpew(const char *msg, ...);
		};

		class Profiler : public IProfiler
		{
		public:
			virtual void OnNativeBegin(IPluginContext *pContext, sp_native_t *native);
			virtual void OnNativeEnd();
			virtual void OnFunctionBegin(IPluginContext *pContext, const char *name);
			virtual void OnFunctionEnd();
			virtual int OnCallbackBegin(IPluginContext *pContext, sp_public_t *pubfunc);
			virtual void OnCallbackEnd(int serial);
		};

		class SourcePawnEngine : public ISourcePawnEngine
		{
		public:
			virtual sp_plugin_t *LoadFromFilePointer(FILE *fp, int *err);
			virtual sp_plugin_t *LoadFromMemory(void *base, sp_plugin_t *plugin, int *err);
			virtual int FreeFromMemory(sp_plugin_t *plugin);
			virtual void *BaseAlloc(size_t size);
			virtual void BaseFree(void *memory);
			virtual void *ExecAlloc(size_t size);
			virtual void ExecFree(void *address);
			virtual IDebugListener *SetDebugListener(IDebugListener *listener);
			virtual unsigned int GetContextCallCount();
			virtual unsigned int GetEngineAPIVersion();
			virtual void *AllocatePageMemory(size_t size);
			virtual void SetReadWrite(void *ptr);
			virtual void SetReadExecute(void *ptr);
			virtual void FreePageMemory(void *ptr);
		};

		class SourcePawnEngine2 : public ISourcePawnEngine2
		{
		public:
			virtual unsigned int GetAPIVersion();
			virtual const char *GetEngineName();
			virtual const char *GetVersionString();
			virtual ICompilation *StartCompilation();
			virtual IPluginRuntime *LoadPlugin(ICompilation *co, const char *file, int *err);
			virtual SPVM_NATIVE_FUNC CreateFakeNative(SPVM_FAKENATIVE_FUNC callback, void *pData);
			virtual void DestroyFakeNative(SPVM_NATIVE_FUNC func);
			virtual IDebugListener *SetDebugListener(IDebugListener *listener);
			virtual void SetProfiler(IProfiler *profiler);
			virtual const char *GetErrorString(int err);
			virtual bool Initialize();
			virtual void Shutdown();
			virtual IPluginRuntime *CreateEmptyRuntime(const char *name, uint32_t memory);
		};
	}
}

#endif // !defined _INCLUDE_V8_SPAPIEMULATION_H_
