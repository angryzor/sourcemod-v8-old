#include "extension.h"
#include "GeoIP.h"

GeoIP_Extension g_GeoIP;
GeoIP *gi = NULL;

SMEXT_LINK(&g_GeoIP);

bool GeoIP_Extension::SDK_OnLoad(char *error, size_t err_max, bool late)
{
	char path[PLATFORM_MAX_PATH+1];

	g_pSM->BuildPath(Path_SM, path, sizeof(path), "configs/geoip/GeoIP.dat");
	gi = GeoIP_open(path, GEOIP_MEMORY_CACHE);

	if (!gi)
	{
		snprintf(error, err_max, "Failed to instantiate GeoIP!");
		return false;
	}

	g_pShareSys->AddNatives(myself, geoip_natives);
	g_pSM->LogMessage(myself, "GeoIP database info: %s", GeoIP_database_info(gi));

	return true;
}

void GeoIP_Extension::SDK_OnUnload()
{
	GeoIP_delete(gi);
	gi = NULL;
}

/*******************************
*                              *
* GEOIP NATIVE IMPLEMENTATIONS *
*                              *
*******************************/

inline void StripPort(char *ip)
{
	char *tmp = strchr(ip, ':');
	if (!tmp)
		return;
	*tmp = '\0';
}

static cell_t sm_Geoip_Code2(IPluginContext *pCtx, const cell_t *params)
{
	char *ip;
	const char *ccode;

	pCtx->LocalToString(params[1], &ip);
	StripPort(ip);

	ccode = GeoIP_country_code_by_addr(gi, ip);
	pCtx->StringToLocal(params[2], 3, (ccode) ? ccode : "er");

	return 1;
}

static cell_t sm_Geoip_Code3(IPluginContext *pCtx, const cell_t *params)
{
	char *ip;
	const char *ccode;

	pCtx->LocalToString(params[1], &ip);
	StripPort(ip);

	ccode = GeoIP_country_code3_by_addr(gi, ip);
	pCtx->StringToLocal(params[2], 4, (ccode) ? ccode : "err");

	return 1;
}

static cell_t sm_Geoip_Country(IPluginContext *pCtx, const cell_t *params)
{
	char *ip;
	const char *ccode;

	pCtx->LocalToString(params[1], &ip);
	StripPort(ip);

	ccode = GeoIP_country_name_by_addr(gi, ip);
	pCtx->StringToLocal(params[2], params[3], (ccode) ? ccode : "error");

	return 1;
}

const sp_nativeinfo_t geoip_natives[] = 
{
	{"GeoipCode2",			sm_Geoip_Code2},
	{"GeoipCode3",			sm_Geoip_Code3},
	{"GeoipCountry",		sm_Geoip_Country},
	{NULL,					NULL},
};

