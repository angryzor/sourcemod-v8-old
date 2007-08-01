/**
 * vim: set ts=4 :
 * ================================================================
 * SourceMod
 * Copyright (C) 2004-2007 AlliedModders LLC.  All rights reserved.
 * ================================================================
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License, 
 * version 3.0, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to 
 * link the code of this program (as well as its derivative works) to 
 * "Half-Life 2," the "Source Engine," the "SourcePawn JIT," and any 
 * Game MODs that run on software by the Valve Corporation.  You must 
 * obey the GNU General Public License in all respects for all other 
 * code used. Additionally, AlliedModders LLC grants this exception 
 * to all derivative works. AlliedModders LLC defines further 
 * exceptions, found in LICENSE.txt (as of this writing, version 
 * JULY-31-2007), or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_MENUSTYLE_RADIO_H
#define _INCLUDE_MENUSTYLE_RADIO_H

#include "sm_globals.h"
#include "MenuManager.h"
#include "MenuStyle_Base.h"
#include "sourcemm_api.h"
#include <IPlayerHelpers.h>
#include <IUserMessages.h>
#include "sm_fastlink.h"
#include <sh_stack.h>

using namespace SourceMod;

class CRadioDisplay;
class CRadioMenu;

class CRadioStyle : 
	public BaseMenuStyle,
	public SMGlobalClass,
	public IUserMessageListener
{
public:
	CRadioStyle();
public: //SMGlobalClass
	void OnSourceModAllInitialized();
	void OnSourceModLevelChange(const char *mapName);
	void OnSourceModShutdown();
public: //BaseMenuStyle
	CBaseMenuPlayer *GetMenuPlayer(int client);
	void SendDisplay(int client, IMenuPanel *display);
public: //IMenuStyle
	const char *GetStyleName();
	IMenuPanel *CreatePanel();
	IBaseMenu *CreateMenu(IMenuHandler *pHandler, IdentityToken_t *pOwner);
	unsigned int GetMaxPageItems();
public: //IUserMessageListener
	void OnUserMessage(int msg_id, bf_write *bf, IRecipientFilter *pFilter);
	void OnUserMessageSent(int msg_id);
public:
	bool IsSupported();
	bool OnClientCommand(int client);
public:
	CRadioDisplay *MakeRadioDisplay(CRadioMenu *menu=NULL);
	void FreeRadioDisplay(CRadioDisplay *display);
private:
	CBaseMenuPlayer *m_players;
	CStack<CRadioDisplay *> m_FreeDisplays;
};

class CRadioDisplay : public IMenuPanel
{
	friend class CRadioStyle;
public:
	CRadioDisplay();
	CRadioDisplay(CRadioMenu *menu);
public: //IMenuPanel
	IMenuStyle *GetParentStyle();
	void Reset();
	void DrawTitle(const char *text, bool onlyIfEmpty=false);
	unsigned int DrawItem(const ItemDrawInfo &item);
	bool DrawRawLine(const char *rawline);
	bool SetExtOption(MenuOption option, const void *valuePtr);
	bool CanDrawItem(unsigned int drawFlags);
	bool SendDisplay(int client, IMenuHandler *handler, unsigned int time);
	void DeleteThis();
	void SendRawDisplay(int client, unsigned int time);
	bool SetSelectableKeys(unsigned int keymap);
	unsigned int GetCurrentKey();
	bool SetCurrentKey(unsigned int key);
private:
	String m_BufferText;
	String m_Title;
	unsigned int m_NextPos;
	int keys;
};

class CRadioMenu : public CBaseMenu
{
public:
	CRadioMenu(IMenuHandler *pHandler, IdentityToken_t *pOwner);
public:
	bool SetExtOption(MenuOption option, const void *valuePtr);
	IMenuPanel *CreatePanel();
	bool Display(int client, unsigned int time);
	void Cancel_Finally();
	void VoteDisplay(int client, unsigned int maxTime);
};

extern CRadioStyle g_RadioMenuStyle;

#endif //_INCLUDE_MENUSTYLE_RADIO_H
