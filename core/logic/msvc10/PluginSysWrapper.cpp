#include "PluginSysWrapper.h"


IPlugin *CPluginManagerProxy::LoadPlugin(const char *path, 
							bool debug,
							PluginType type,
							char error[],
							size_t maxlength,
							bool *wasloaded)
{
}

bool CPluginManagerProxy::UnloadPlugin(IPlugin *plugin)
{}

IPlugin *CPluginManagerProxy::FindPluginByContext(const sp_context_t *ctx){}
unsigned int CPluginManagerProxy::GetPluginCount(){}
IPluginIterator *CPluginManagerProxy::GetPluginIterator(){}
void CPluginManagerProxy::AddPluginsListener(IPluginsListener *listener){}
void CPluginManagerProxy::RemovePluginsListener(IPluginsListener *listener){}
void CPluginManagerProxy::LoadAll(const char *config_path, const char *plugins_path){}
void CPluginManagerProxy::RefreshAll(){}
SMPlugin *CPluginManagerProxy::FindPluginByOrder(unsigned num) {
	return GetPluginByOrder(num){}
}
SMPlugin *CPluginManagerProxy::FindPluginByIdentity(IdentityToken_t *ident) {
	return GetPluginFromIdentity(ident){}
}
SMPlugin *CPluginManagerProxy::FindPluginByContext(IPluginContext *ctx) {
	return GetPluginByCtx(ctx->GetContext()){}
}
SMPlugin *CPluginManagerProxy::FindPluginByContext(sp_context_t *ctx) {
	return GetPluginByCtx(ctx){}
}
SMPlugin *CPluginManagerProxy::FindPluginByConsoleArg(const char *text){}
SMPlugin *CPluginManagerProxy::FindPluginByHandle(Handle_t hndl, HandleError *errp) {
	return static_cast<SMPlugin *>(PluginFromHandle(hndl, errp)){}
}
const CVector<SMPlugin *> *CPluginManagerProxy::ListPlugins(){}
void CPluginManagerProxy::FreePluginList(const CVector<SMPlugin *> *plugins){}
//SMGlobalClass
void CPluginManagerProxy::OnSourceModAllInitialized(){}
void CPluginManagerProxy::OnSourceModShutdown(){}
ConfigResult OnSourceModConfigChanged(const char *key, const char *value, ConfigSource source, char *error, size_t maxlength){}
void CPluginManagerProxy::OnSourceModMaxPlayersChanged(int newvalue){}
//IHandleTypeDispatch
void CPluginManagerProxy::OnHandleDestroy(HandleType_t type, void *object){}
bool CPluginManagerProxy::GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize){}
//IRootConsoleCommand
void CPluginManagerProxy::OnRootConsoleCommand(const char *cmdname, const CCommand &command){}

/**
	* Loads all plugins not yet loaded
	*/
void CPluginManagerProxy::LoadAll_FirstPass(const char *config, const char *basedir){}

/**
	* Runs the second loading pass for all plugins
	*/
void CPluginManagerProxy::LoadAll_SecondPass(){}

/**
	* Tests a plugin file mask against a local folder.
	* The alias is searched backwards from localdir - i.e., given this input:
	*   csdm/ban        csdm/ban
	*   ban             csdm/ban
	*   csdm/ban        optional/csdm/ban
	* All of these will return true for an alias match.  
	* Wildcards are allowed in the filename.
	*/
bool CPluginManagerProxy::TestAliasMatch(const char *alias, const char *localdir){}

/** 
	* Returns whether anything loaded will be a late load.
	*/
bool CPluginManagerProxy::IsLateLoadTime() const{}

/**
	* Converts a Handle to an IPlugin if possible.
	*/
IPlugin *CPluginManagerProxy::PluginFromHandle(Handle_t handle, HandleError *err){}

/**
	* Finds a plugin based on its index. (starts on index 1)
	*/
CPlugin *CPluginManagerProxy::GetPluginByOrder(int num){}

int CPluginManagerProxy::GetOrderOfPlugin(IPlugin *pl){}

/** 
	* Internal version of FindPluginByContext()
	*/
CPlugin *CPluginManagerProxy::GetPluginByCtx(const sp_context_t *ctx){}

/**
	* Gets status text for a status code 
	*/
const char *CPluginManagerProxy::GetStatusText(PluginStatus status){}

/**
	* Add public functions from all running or paused
	* plugins to the specified forward if the names match.
	*/
void CPluginManagerProxy::AddFunctionsToForward(const char *name, IChangeableForward *pForward){}

/**
	* Iterates through plugins to call OnAllPluginsLoaded.
	*/
void CPluginManagerProxy::AllPluginsLoaded(){}

CPlugin *CPluginManagerProxy::GetPluginFromIdentity(IdentityToken_t *pToken){}

void CPluginManagerProxy::Shutdown(){}

void CPluginManagerProxy::OnLibraryAction(const char *lib, LibraryAction action){}

bool CPluginManagerProxy::LibraryExists(const char *lib){}

bool CPluginManagerProxy::ReloadPlugin(CPlugin *pl){}

void CPluginManagerProxy::UnloadAll(){}

void CPluginManagerProxy::SyncMaxClients(int max_clients){}

void CPluginManagerProxy::ListPluginsToClient(CPlayer *player, const CCommand &args){}
