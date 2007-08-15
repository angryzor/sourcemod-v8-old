/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Anti-Flood Plugin
 * Protects against chat flooding.
 *
 * SourceMod (C)2004-2007 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#pragma semicolon 1

#include <sourcemod>

public Plugin:myinfo = 
{
	name = "Anti-Flood",
	author = "AlliedModders LLC",
	description = "Protects against chat flooding",
	version = SOURCEMOD_VERSION,
	url = "http://www.sourcemod.net/"
};

new Float:g_LastTime[MAXPLAYERS + 1] = {0.0, ...};		/* Last time player used say or say_team */
new g_FloodTokens[MAXPLAYERS + 1] = {0, ...};			/* Number of flood tokens player has */

new Handle:sm_flood_time;								/* Handle to sm_flood_time convar */

public OnPluginStart()
{
	LoadTranslations("antiflood.phrases");
	
	RegConsoleCmd("say", CheckChatFlood);
	RegConsoleCmd("say_team", CheckChatFlood);
	sm_flood_time = CreateConVar("sm_flood_time", "0.75", "Amount of time allowed between chat messages");
}

public OnClientPutInServer(client)
{
	g_LastTime[client] = 0.0;
	g_FloodTokens[client] = 0;
}

public Action:CheckChatFlood(client, args)
{
	/* Chat from server console shouldn't be checked for flooding */
	if (client == 0)
	{
		return Plugin_Continue;
	}
	
	new Float:maxChat = GetConVarFloat(sm_flood_time);
	
	if (maxChat > 0.0)
	{
		new Float:curTime = GetGameTime();
		
		if (g_LastTime[client] > curTime)
		{
			/* If player has 3 or more flood tokens, block their message */
			if (g_FloodTokens[client] >= 3)
			{
				PrintToChat(client, "[SM] %t", "Flooding the server");
				g_LastTime[client] = curTime + maxChat + 3.0;
				
				return Plugin_Stop;	
			}
			
			/* Add one flood token when player goes over chat time limit */
			g_FloodTokens[client]++;
		} else if (g_FloodTokens[client]) {
			/* Remove one flood token when player chats within time limit (slow decay) */
			g_FloodTokens[client]--;
		}
		
		/* Store last time of chat usage */
		g_LastTime[client] = curTime + maxChat;
	}
	
	return Plugin_Continue;
}
