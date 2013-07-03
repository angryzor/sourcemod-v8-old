#ifndef _INCLUDE_PLUGINSYSWRAPPER_H_
#define _INCLUDE_PLUGINSYSWRAPPER_H_

#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <IPluginSys.h>
#include <IHandleSys.h>
#include <IForwardSys.h>
#include <sh_list.h>
#include <sh_stack.h>
#include <sh_vector.h>
#include <sh_string.h>
#include "common_logic.h"
#include <sm_trie_tpl.h>
#include <IRootConsoleMenu.h>
#include "ITranslator.h"
#include "IGameConfigs.h"
#include "NativeOwner.h"
#include "ShareSys.h"

class CPluginManagerProxy
	: public IScriptManager,
	  public SMGlobalClass,
	  public IHandleTypeDispatch,
	  public IRootConsoleCommand
{
	IPlugin *LoadPlugin(const char *path, 
								bool debug,
								PluginType type,
								char error[],
								size_t maxlength,
								bool *wasloaded);
	bool UnloadPlugin(IPlugin *plugin);
	IPlugin *FindPluginByContext(const sp_context_t *ctx);
	unsigned int GetPluginCount();
	IPluginIterator *GetPluginIterator();
	void AddPluginsListener(IPluginsListener *listener);
	void RemovePluginsListener(IPluginsListener *listener);
	void LoadAll(const char *config_path, const char *plugins_path);
	void RefreshAll();
	SMPlugin *FindPluginByOrder(unsigned num) {
		return GetPluginByOrder(num);
	}
	SMPlugin *FindPluginByIdentity(IdentityToken_t *ident) {
		return GetPluginFromIdentity(ident);
	}
	SMPlugin *FindPluginByContext(IPluginContext *ctx) {
		return GetPluginByCtx(ctx->GetContext());
	}
	SMPlugin *FindPluginByContext(sp_context_t *ctx) {
		return GetPluginByCtx(ctx);
	}
	SMPlugin *FindPluginByConsoleArg(const char *text);
	SMPlugin *FindPluginByHandle(Handle_t hndl, HandleError *errp) {
		return static_cast<SMPlugin *>(PluginFromHandle(hndl, errp));
	}
	const CVector<SMPlugin *> *ListPlugins();
	void FreePluginList(const CVector<SMPlugin *> *plugins);
public: //SMGlobalClass
	void OnSourceModAllInitialized();
	void OnSourceModShutdown();
	ConfigResult OnSourceModConfigChanged(const char *key, const char *value, ConfigSource source, char *error, size_t maxlength);
	void OnSourceModMaxPlayersChanged(int newvalue);
public: //IHandleTypeDispatch
	void OnHandleDestroy(HandleType_t type, void *object);
	bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize);
public: //IRootConsoleCommand
	void OnRootConsoleCommand(const char *cmdname, const CCommand &command);
public:
	/**
	 * Loads all plugins not yet loaded
	 */
	void LoadAll_FirstPass(const char *config, const char *basedir);

	/**
	 * Runs the second loading pass for all plugins
	 */
	void LoadAll_SecondPass();

	/**
	 * Tests a plugin file mask against a local folder.
	 * The alias is searched backwards from localdir - i.e., given this input:
	 *   csdm/ban        csdm/ban
	 *   ban             csdm/ban
	 *   csdm/ban        optional/csdm/ban
	 * All of these will return true for an alias match.  
	 * Wildcards are allowed in the filename.
	 */
	bool TestAliasMatch(const char *alias, const char *localdir);

	/** 
	 * Returns whether anything loaded will be a late load.
	 */
	bool IsLateLoadTime() const;

	/**
	 * Converts a Handle to an IPlugin if possible.
	 */
	IPlugin *PluginFromHandle(Handle_t handle, HandleError *err);

	/**
	 * Finds a plugin based on its index. (starts on index 1)
	 */
	CPlugin *GetPluginByOrder(int num);

	int GetOrderOfPlugin(IPlugin *pl);

	/** 
	 * Internal version of FindPluginByContext()
	 */
	CPlugin *GetPluginByCtx(const sp_context_t *ctx);

	/**
	 * Gets status text for a status code 
	 */
	const char *GetStatusText(PluginStatus status);

	/**
	 * Add public functions from all running or paused
	 * plugins to the specified forward if the names match.
	 */
	void AddFunctionsToForward(const char *name, IChangeableForward *pForward);

	/**
	 * Iterates through plugins to call OnAllPluginsLoaded.
	 */
	void AllPluginsLoaded();

	CPlugin *GetPluginFromIdentity(IdentityToken_t *pToken);

	void Shutdown();

	void OnLibraryAction(const char *lib, LibraryAction action);

	bool LibraryExists(const char *lib);

	bool ReloadPlugin(CPlugin *pl);

	void UnloadAll();

	void SyncMaxClients(int max_clients);

	void ListPluginsToClient(CPlayer *player, const CCommand &args);
};

#endif // !defined _INCLUDE_PLUGINSYSWRAPPER_H_