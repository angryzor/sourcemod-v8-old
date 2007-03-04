/**
* ===============================================================
* SourceMod (C)2004-2007 AlliedModders LLC.  All rights reserved.
* ===============================================================
*
* This file is not open source and may not be copied without explicit
* written permission of AlliedModders LLC.  This file may not be redistributed 
* in whole or significant part.
* For information, see LICENSE.txt or http://www.sourcemod.net/license.php
*
* Version: $Id$
*/

#include "sm_globals.h"
#include "sourcemm_api.h"
#include "HandleSys.h"
#include "EventManager.h"

static cell_t sm_HookEvent(IPluginContext *pContext, const cell_t *params)
{
	char *name;
	IPluginFunction *pFunction;

	pContext->LocalToString(params[1], &name);
	pFunction = pContext->GetFunctionById(params[2]);

	if (!pFunction)
	{
		return pContext->ThrowNativeError("Invalid function id (%X)", params[2]);
	}

	if (g_EventManager.HookEvent(name, pFunction, static_cast<EventHookMode>(params[3])) == EventHookErr_InvalidEvent)
	{
		return pContext->ThrowNativeError("Game event \"%s\" does not exist", name);
	}

	return 1;
}

static cell_t sm_UnhookEvent(IPluginContext *pContext, const cell_t *params)
{
	char *name;
	IPluginFunction *pFunction;

	pContext->LocalToString(params[1], &name);
	pFunction = pContext->GetFunctionById(params[2]);

	if (!pFunction)
	{
		return pContext->ThrowNativeError("Invalid function id (%X)", params[2]);
	}

	EventHookError err = g_EventManager.UnhookEvent(name, pFunction, static_cast<EventHookMode>(params[3]));

	/* Possible errors that UnhookGameEvent can return */
	if (err == EventHookErr_NotActive)
	{
		return pContext->ThrowNativeError("Game event \"%s\" has no active hook", name);
	} else if (err == EventHookErr_InvalidCallback) {
		return pContext->ThrowNativeError("Invalid hook callback specified for game event \"%s\"", name);
	}

	return 1;
}

static cell_t sm_CreateEvent(IPluginContext *pContext, const cell_t *params)
{
	IGameEvent *pEvent;
	EventInfo *pInfo;
	char *name;

	pContext->LocalToString(params[1], &name);

	pEvent = gameevents->CreateEvent(name, true);

	if (pEvent)
	{
		pInfo = new EventInfo;
		pInfo->pEvent = pEvent;
		pInfo->canDelete = true;

		return g_HandleSys.CreateHandle(g_EventManager.GetHandleType(), pInfo, pContext->GetIdentity(), g_pCoreIdent, NULL);
	}

	return BAD_HANDLE;
}

static cell_t sm_FireEvent(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	if ((params[3] & EVENT_PASSTHRU_ALL) == false)
	{
		/* Set whether or not SourceMod plugins should be notified */
		g_EventManager.SetNotifyState(false);
	}

	/* Fire game event */
	gameevents->FireEvent(pInfo->pEvent, params[2] ? true : false);

	pInfo->canDelete = false;

	/* Free handle on game event */
	HandleSecurity sec = { pContext->GetIdentity(), g_pCoreIdent };
	g_HandleSys.FreeHandle(hndl, &sec);

	/* Free EventInfo memory */
	delete pInfo;

	return 1;
}

static cell_t sm_CancelCreatedEvent(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	gameevents->FreeEvent(pInfo->pEvent);

	// Free GameEventInfo memory
	delete pInfo;

	// Free handle on game event
	HandleSecurity sec = { pContext->GetIdentity(), g_pCoreIdent };
	g_HandleSys.FreeHandle(hndl, &sec);

	return 1;
}

static cell_t sm_GetEventName(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	pContext->StringToLocalUTF8(params[2], params[3], pInfo->pEvent->GetName(), NULL);

	return 1;
}

static cell_t sm_GetEventBool(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	return pInfo->pEvent->GetBool(key);
}

static cell_t sm_GetEventInt(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	return pInfo->pEvent->GetInt(key);
}

static cell_t sm_GetEventFloat(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	float value = pInfo->pEvent->GetFloat(key);

	return sp_ftoc(value);
}

static cell_t sm_GetEventString(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	pContext->StringToLocalUTF8(params[3], params[4], pInfo->pEvent->GetString(key), NULL);

	return 1;
}

static cell_t sm_SetEventBool(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	pInfo->pEvent->SetBool(key, params[3] ? true : false);

	return 1;
}

static cell_t sm_SetEventInt(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	pInfo->pEvent->SetInt(key, params[3]);

	return 1;
}

static cell_t sm_SetEventFloat(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	float value = sp_ctof(params[3]);
	pInfo->pEvent->SetFloat(key, value);

	return 1;
}

static cell_t sm_SetEventString(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast<Handle_t>(params[1]);
	HandleError err;
	EventInfo *pInfo;

	if ((err=g_HandleSys.ReadHandle(hndl, g_EventManager.GetHandleType(), NULL, (void **)&pInfo))
		!= HandleError_None)
	{
		return pContext->ThrowNativeError("Invalid GameEvent Handle %x (error %d)", hndl, err);
	}

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	pInfo->pEvent->SetString(key, value);

	return 1;
}

REGISTER_NATIVES(gameEventNatives)
{
	{"HookEvent",			sm_HookEvent},
	{"UnhookEvent",			sm_UnhookEvent},
	{"CreateEvent",			sm_CreateEvent},
	{"FireEvent",			sm_FireEvent},
	{"CancelCreatedEvent",	sm_CancelCreatedEvent},
	{"GetEventName",		sm_GetEventName},
	{"GetEventBool",		sm_GetEventBool},
	{"GetEventInt",			sm_GetEventInt},
	{"GetEventFloat",		sm_GetEventFloat},
	{"GetEventString",		sm_GetEventString},
	{"SetEventBool",		sm_SetEventBool},
	{"SetEventInt",			sm_SetEventInt},
	{"SetEventFloat",		sm_SetEventFloat},
	{"SetEventString",		sm_SetEventString},
	{NULL,					NULL}
};
