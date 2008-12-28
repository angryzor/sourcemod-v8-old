/**
* vim: set ts=4 :
* =============================================================================
* SourceMod
* Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
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

#include "GameDataFetcher.h"
#include "bitbuf.h"

#ifdef PLATFORM_WINDOWS
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define INVALID_SOCKET		-1
#define closesocket 		close
#define WSAGetLastError()	errno
#endif

#include "sh_vector.h"
#include "sh_string.h"
#include "sm_version.h"

#ifdef ORANGEBOX_BUILD
#include "convar_sm_ob.h"
#else
#include "convar_sm.h"
#endif

#include "sourcemm_api.h"
#include "time.h"
#include "TimerSys.h"
#include "compat_wrappers.h"
#include "sm_stringutil.h"
#include "md5.h"

#define QUERY_MAX_LENGTH 1024

BuildMD5ableBuffer g_MD5Builder;
FetcherThread g_FetchThread;

FILE *logfile = NULL;

bool g_disableGameDataUpdate = false;
bool g_restartAfterUpdate = false;

int g_serverPort = 6500;
char g_serverAddress[100] = "smupdate.alliedmods.net";

void FetcherThread::RunThread(IThreadHandle *pHandle)
{
	char lock_path[PLATFORM_MAX_PATH];
	g_SourceMod.BuildPath(Path_SM, lock_path, sizeof(lock_path), "data/temp");
	g_LibSys.CreateFolder(lock_path);

	g_SourceMod.BuildPath(Path_SM, lock_path, sizeof(lock_path), "data/temp/gamedata.lock");

	char log_path[PLATFORM_MAX_PATH];
	g_SourceMod.BuildPath(Path_SM, log_path, sizeof(log_path), "logs/gamedata");

	g_LibSys.CreateFolder(log_path);

	time_t t;
	GetAdjustedTime(&t);
	tm *curtime = localtime(&t);

	g_SourceMod.BuildPath(Path_SM, log_path, sizeof(log_path), "logs/gamedata/L%02d%02d.log", curtime->tm_mon + 1, curtime->tm_mday);

	logfile = fopen(log_path, "a");

	if (!logfile)
	{
		/* :( */
		return;
	}

	//Create a blank lock file
	FILE *fp = fopen(lock_path, "w");
	if (fp)
	{
		fclose(fp);
	}

	char query[QUERY_MAX_LENGTH];

	/* Check for updated gamedata files */
	int len = BuildGameDataQuery(query, QUERY_MAX_LENGTH);

	if (len == 0)
	{
		g_Logger.LogToFileOnly(logfile, "Could not build gamedata query!");
		fclose(logfile);
		unlink(lock_path);
		return;
	}

	/* We check this late so we have the MD5 sums available.  This may change in the future. */
	if (g_disableGameDataUpdate)
	{
		g_Logger.LogToFileOnly(logfile, "Skipping gamedata fetcher (DisableAutoUpdate set)");
		fclose(logfile);
		unlink(lock_path);
		return;
	}

	/* Create a new socket for this connection */
	int socketDescriptor = ConnectSocket();

	if (socketDescriptor == INVALID_SOCKET)
	{
		fclose(logfile);
		unlink(lock_path);
		return;
	}

	int sent = SendData(socketDescriptor, query, len);

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "Sent gamedata query");
ENDIF_DEBUG_SPEW

	if (sent == 0)
	{
IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "Failed to send gamedata query data to remote host");
ENDIF_DEBUG_SPEW

		closesocket(socketDescriptor);
		fclose(logfile);
		unlink(lock_path);
		return;
	}

	ProcessGameDataQuery(socketDescriptor);

	/* And we're done! */
	closesocket(socketDescriptor);
	fclose(logfile);
	unlink(lock_path);
}

void FetcherThread::OnTerminate(IThreadHandle *pHandle, bool cancel)
{
	g_blockGameDataLoad = false;

	if (wasSuccess)
	{
		HandleUpdateStatus(updateStatus, build);

		if (needsRestart)
		{
			if (g_restartAfterUpdate)
			{
				g_Logger.LogMessage("Automatically restarting server after a successful gamedata update!");
				engine->ServerCommand("quit\n");
			}
			else
			{
				g_Logger.LogMessage("Your gamedata files have been updated, please restart your server.");
			}
		}
	}
	else if (!g_disableGameDataUpdate)
	{
		g_Logger.LogError("An error occurred in the gamedata fetcher, see your gamedata log files for more information.");
	}
}

int FetcherThread::BuildGameDataQuery(char *buffer, int maxlen)
{
	char gamedata_path[PLATFORM_MAX_PATH];
	g_SourceMod.BuildPath(Path_SM, gamedata_path, sizeof(gamedata_path), "gamedata");

	IDirectory *dir = g_LibSys.OpenDirectory(gamedata_path);

	if (dir == NULL)
	{
		return 0;
	}

	bf_write Writer = bf_write("GameDataQuery", buffer, maxlen);

	Writer.WriteByte('A'); //Generic Header char
	Writer.WriteByte('G'); //G for gamedata query, or green, like my hat.

	short build[4] = { SVN_FILE_VERSION };

	Writer.WriteBytes(&build[0], 8);

	Writer.WriteByte(0); // Initialize the file counter - Index 10

	while (dir->MoreFiles())
	{
		if (dir->IsEntryFile())
		{
			const char *name = dir->GetEntryName();
			size_t len = strlen(name);
			if (len >= 4 && strcmp(&name[len-4], ".txt") == 0)
			{
				MD5 md5;
				SMCError err;
				SMCStates states;
				unsigned char raw[16];
				char file[PLATFORM_MAX_PATH];

				g_LibSys.PathFormat(file, sizeof(file), "%s/%s", gamedata_path, name);

				g_MD5Builder.checksum = &md5;
				if ((err = g_TextParser.ParseFile_SMC(file, &g_MD5Builder, &states)) == SMCError_Okay)
				{
					md5.raw_digest(raw);
					(uint8_t)buffer[10]++; //Increment the file counter
					Writer.WriteBytes(raw, 16);

					FileData *data = new FileData();
					data->filename = new SourceHook::String(file);
					md5.hex_digest(data->checksum);
					filenames.push_back(data);

IF_DEBUG_SPEW
					g_Logger.LogToFileOnly(logfile, "Parsed file: %s as %s", file, data->checksum);
ENDIF_DEBUG_SPEW

				}
				else
				{
IF_DEBUG_SPEW
					const char *error = g_TextParser.GetSMCErrorString(err);
					g_Logger.LogToFileOnly(logfile, "Parsing of file %s failed: %s", file, error);
ENDIF_DEBUG_SPEW
				}
			}
		}
		dir->NextEntry();
	}

	return Writer.GetNumBytesWritten();
}

int FetcherThread::ConnectSocket()
{
#if defined PLATFORM_WINDOWS
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);
#endif

	struct protoent *ptrp;
	
	if ((ptrp = getprotobyname("tcp")) == NULL) 
	{
		g_Logger.LogToFileOnly(logfile, "Error: Failed to find TCP protocol");
		return INVALID_SOCKET;
	}

	int socketDescriptor = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);

	if (socketDescriptor == INVALID_SOCKET)
	{
		char error[255];
		g_LibSys.GetPlatformErrorEx(WSAGetLastError(), error, sizeof(error));
		g_Logger.LogToFileOnly(logfile, "Error: Failed to create socket: %s", error);
		closesocket(socketDescriptor);
		return INVALID_SOCKET;
	}

	struct hostent *he;
	struct sockaddr_in local_addr;

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons((u_short)g_serverPort);

	he = gethostbyname(g_serverAddress);

	if (!he)
	{
		if ((local_addr.sin_addr.s_addr = inet_addr(g_serverAddress)) == INADDR_NONE)
		{
			g_Logger.LogToFileOnly(logfile, "Couldn't locate address: %s", g_serverAddress);
			closesocket(socketDescriptor);
			return INVALID_SOCKET;
		}
	} 
	else 
	{
		memcpy(&local_addr.sin_addr, (struct in_addr *)he->h_addr, he->h_length);
	}

	if (connect(socketDescriptor, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0)
	{
		char error[255];
		g_LibSys.GetPlatformErrorEx(WSAGetLastError(), error, sizeof(error));
		g_Logger.LogToFileOnly(logfile, "Couldn't connect to %s: %s", g_serverAddress, error);
		closesocket(socketDescriptor);
		return INVALID_SOCKET;
	}

	return socketDescriptor;
}

void FetcherThread::ProcessGameDataQuery(int socketDescriptor)
{
	char buffer[50];

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "Waiting for reply!");
ENDIF_DEBUG_SPEW

	//Read in the header bytes
	int returnLen = RecvData(socketDescriptor, buffer, 12);


	if (returnLen == 0)
	{
		char error[255];
		g_LibSys.GetPlatformErrorEx(WSAGetLastError(), error, sizeof(error));
		g_Logger.LogToFileOnly(logfile, "Did not receive reply: %s", error);
		return;
	}

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "Received Header!");
ENDIF_DEBUG_SPEW

	bf_read Reader = bf_read("GameDataQuery", buffer, 12);

	if (Reader.ReadByte() != 'A' || Reader.ReadByte() != 'G')
	{
IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "Unknown Query Response");
ENDIF_DEBUG_SPEW
		return;
	}

	updateStatus = (UpdateStatus)Reader.ReadByte();

	build[0] = Reader.ReadShort();
	build[1] = Reader.ReadShort();
	build[2] = Reader.ReadShort();
	build[3] = Reader.ReadShort();

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile,
						   "Update Status: %i - Latest %i.%i.%i.%i",
						   updateStatus,
						   build[0],
						   build[1],
						   build[2],
						   build[3]);
ENDIF_DEBUG_SPEW

	int changedFiles = Reader.ReadByte();

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "Files to download: %i", changedFiles);
ENDIF_DEBUG_SPEW

	for (int i=0; i<changedFiles; i++)
	{
		//Read in the file index and byte count
		returnLen = RecvData(socketDescriptor, buffer, 5);

		if (returnLen == 0)
		{
			/* Timeout or fail? */
			return;
		}

		Reader.StartReading(buffer, 5);

		int index = Reader.ReadByte();
		int tempLen = Reader.ReadUBitLong(32);

IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "File index %i and length %i", index, tempLen);
ENDIF_DEBUG_SPEW

		void *memPtr;
		memtable->CreateMem(tempLen+1, &memPtr);

		//Read the contents of our file into the memtable
		returnLen = RecvData(socketDescriptor, (char *)memPtr, tempLen);

IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "Received %i bytes", returnLen);
ENDIF_DEBUG_SPEW

		if (returnLen == 0)
		{
			/* Timeout or fail? */
			return;
		}

		((unsigned char *)memPtr)[tempLen] = '\0';

		FileData *data = filenames.at(index);
		const char *filename;
		if (data != NULL)
		{
			filename = data->filename->c_str();

			FILE *fp = fopen(filename, "w");

			if (fp)
			{
				fprintf(fp, "%s", (const char *)memPtr);
				fclose(fp);
			}
			else
			{
				g_Logger.LogToFileOnly(logfile, "Failed to open file \"%s\" for writing", filename);
			}
		}
		else
		{
			filename = "";
		}

		memtable->Reset();

		g_Logger.LogToFileOnly(logfile, "Updated file: %s", filename);
	}

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "File Downloads Completed!");
ENDIF_DEBUG_SPEW

	needsRestart = false;

	if (changedFiles > 0)
	{
		needsRestart = true;
	}

	//Read changed file count
	returnLen = RecvData(socketDescriptor, buffer, 1);

	if (returnLen == 0)
	{
		char error[255];
		g_LibSys.GetPlatformErrorEx(WSAGetLastError(), error, sizeof(error));
		g_Logger.LogToFileOnly(logfile, "Did not receive count reply: %s", error);
		return;
	}

	Reader.StartReading(buffer, 1);

	changedFiles = Reader.ReadByte();

	if (changedFiles < 1)
	{
IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "No unknown files. We're all done");
ENDIF_DEBUG_SPEW
		return;
	}

	char *changedFileIndexes = new char[changedFiles];

IF_DEBUG_SPEW
	g_Logger.LogToFileOnly(logfile, "%i files were unknown", changedFiles);
ENDIF_DEBUG_SPEW

	returnLen = RecvData(socketDescriptor, changedFileIndexes, changedFiles);

	if (returnLen == 0)
	{
		char error[255];
		g_LibSys.GetPlatformErrorEx(WSAGetLastError(), error, sizeof(error));
		g_Logger.LogToFileOnly(logfile, "Did not receive list reply: %s", error);
		return;
	}

	Reader.StartReading(changedFileIndexes, changedFiles);

	for (int i=0; i<changedFiles; i++)
	{
		int index = Reader.ReadByte();
		char fileName[30];

		FileData *data = filenames.at(index);
		const char* pathname;
		if (data != NULL)
		{
			pathname = data->filename->c_str();
		}
		else
		{
			pathname = "";
		}

		g_LibSys.GetFileFromPath(fileName, sizeof(fileName), pathname);
IF_DEBUG_SPEW
		g_Logger.LogToFileOnly(logfile, "Unknown File %i : %s", index, fileName);
ENDIF_DEBUG_SPEW
	}

	delete [] changedFileIndexes;

	wasSuccess = true;
}

int FetcherThread::RecvData(int socketDescriptor, char *buffer, int len)
{
	fd_set fds;
	struct timeval tv;

	/* Create a 10 Second Timeout */
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	int bytesReceivedTotal = 0;

	while (bytesReceivedTotal < len)
	{
		/* Add our socket to a socket set */
		FD_ZERO(&fds);
		FD_SET(socketDescriptor, &fds);

		/* Wait max of 10 seconds for recv to become available */
		select(socketDescriptor+1, &fds, NULL, NULL, &tv);

		int bytesReceived = 0;

		/* Is there a limit on how much we can receive? Some site said 1024 bytes, which will be well short of a file */
		if (FD_ISSET(socketDescriptor, &fds))
		{
			bytesReceived = recv(socketDescriptor, buffer+bytesReceivedTotal, len-bytesReceivedTotal, 0);
		}

		if (bytesReceived == 0 || bytesReceived == -1)
		{
			return 0;
		}

		bytesReceivedTotal += bytesReceived;
	}

	return bytesReceivedTotal;	
}

int FetcherThread::SendData(int socketDescriptor, char *buffer, int len)
{
	fd_set fds;
	struct timeval tv;

	tv.tv_sec = 10;
	tv.tv_usec = 0;

	int sentBytesTotal = 0;

	while (sentBytesTotal < len)
	{
		FD_ZERO(&fds);
		FD_SET(socketDescriptor, &fds);

		select(socketDescriptor+1, NULL, &fds, NULL, &tv);

		int sentBytes = 0;

		if (FD_ISSET(socketDescriptor, &fds))
		{
			sentBytes = send(socketDescriptor, buffer+sentBytesTotal, len-sentBytesTotal, 0);
		}

		if (sentBytes == 0 || sentBytes == -1)
		{
			return 0;
		}		

		sentBytesTotal += sentBytes;
	}

	return sentBytesTotal;
}

void FetcherThread::HandleUpdateStatus(UpdateStatus status, short version[4])
{
	switch (status)
	{
		case Update_Unknown:
		case Update_Current:
		{
			break;
		}

		case Update_NewBuild:
		{
			break;
		}

		case Update_MinorAvailable:
		{
			g_Logger.LogMessage("SourceMod Update: A new release of SourceMod is now available from sourcemod.net");
			g_Logger.LogMessage("Current Version: %i.%i.%i Available: %i.%i.%i", version[0], version[1], version[2], version[0], version[1], version[2]);
			break;
		}

		case Update_MajorAvailable:
		{
			g_Logger.LogMessage("SourceMod Update: An major release of SourceMod is now available from sourcemod.net");
			g_Logger.LogMessage("Current Version: %i.%i.%i Available: %i.%i.%i", version[0], version[1], version[2], version[0], version[1], version[2]);
			break;
		}

		case Update_CriticalAvailable:
		{
			g_Logger.LogError("SourceMod Update: A critical SourceMod release is available from sourcemod.net. It is strongly recommended that you update!");
			g_Logger.LogMessage("Current Version: %i.%i.%i Available: %i.%i.%i", version[0], version[1], version[2], version[0], version[1], version[2]);
			break;
		}
	}
}

bool g_blockGameDataLoad = false;

class InitFetch : public SMGlobalClass
{
public:
	void OnSourceModAllInitialized_Post()
	{
		char lock_path[PLATFORM_MAX_PATH];
		g_SourceMod.BuildPath(Path_SM, lock_path, sizeof(lock_path), "data/temp/gamedata.lock");

		if (g_LibSys.IsPathFile(lock_path) && g_LibSys.PathExists(lock_path))
		{
			g_Logger.LogError("sourcemod/data/temp/gamedata.lock file detected. This is most likely due to a crash during GameData updating - Blocking GameData loading");
			g_Logger.LogError("If this error persists delete the file manually");
			g_blockGameDataLoad = true;
		}

		ThreadParams fetchThreadParams = ThreadParams();
		fetchThreadParams.prio = ThreadPrio_Low;
		g_pThreader->MakeThread(&g_FetchThread, &fetchThreadParams);
	}

	ConfigResult OnSourceModConfigChanged(const char *key, 
		const char *value, 
		ConfigSource source,
		char *error, 
		size_t maxlength)
	{
		if (strcasecmp(key, "DisableAutoUpdate") == 0)
		{
			if (strcasecmp(value, "yes") == 0)
			{
				g_disableGameDataUpdate = true;
				return ConfigResult_Accept;
			}
			else if (strcasecmp(value, "no") == 0)
			{
				g_disableGameDataUpdate = false;
				return ConfigResult_Accept;
			}

			return ConfigResult_Reject;
		}

		if (strcasecmp(key, "ForceRestartAfterUpdate") == 0)
		{
			if (strcasecmp(value, "yes") == 0)
			{
				g_restartAfterUpdate = true;
				return ConfigResult_Accept;
			}
			else if (strcasecmp(value, "no") == 0)
			{
				g_restartAfterUpdate = false;
				return ConfigResult_Accept;
			}

			return ConfigResult_Reject;
		}

		if (strcasecmp(key, "AutoUpdateServer") == 0)
		{
			UTIL_Format(g_serverAddress, sizeof(g_serverAddress), "%s", value);

			return ConfigResult_Accept;
		}

		if (strcasecmp(key, "AutoUpdatePort") == 0)
		{
			int port = atoi(value);

			if (!port)
			{
				return ConfigResult_Reject;
			}

			g_serverPort = port;

			return ConfigResult_Accept;
		}

		return ConfigResult_Ignore;
	}
} g_InitFetch;

BuildMD5ableBuffer::BuildMD5ableBuffer()
{
	stringTable = new BaseStringTable(2048);
}

BuildMD5ableBuffer::~BuildMD5ableBuffer()
{
	delete stringTable;
}

void BuildMD5ableBuffer::ReadSMC_ParseStart()
{
	stringTable->Reset();
}

SMCResult BuildMD5ableBuffer::ReadSMC_KeyValue(const SMCStates *states,
											   const char *key,
											   const char *value)
{
	stringTable->AddString(key);
	stringTable->AddString(value);

	return SMCResult_Continue;
}

SMCResult BuildMD5ableBuffer::ReadSMC_NewSection(const SMCStates *states, const char *name)
{
	stringTable->AddString(name);

	return SMCResult_Continue;
}

void BuildMD5ableBuffer::ReadSMC_ParseEnd(bool halted, bool failed)
{
	if (halted || failed)
	{
		return;
	}

	void *data = stringTable->GetMemTable()->GetAddress(0);

	if (data != NULL)
	{
		checksum->update((unsigned char *)data, stringTable->GetMemTable()->GetActualMemUsed());
	}

	checksum->finalize();
}

CON_COMMAND(sm_gamedata_md5, "Checks the MD5 sum for a given gamedata file")
{
#if !defined ORANGEBOX_BUILD
	CCommand args;
#endif

	if (args.ArgC() < 2)
	{
		g_SMAPI->ConPrint("Usage: sm_gamedata_md5 <file>\n");
		return;
	}

	const char *file = args.Arg(1);
	if (!file || file[0] == '\0')
	{
		g_SMAPI->ConPrint("Usage: sm_gamedata_md5 <file>\n");
		return;
	}

	SourceHook::CVector<FileData *>::iterator iter = g_FetchThread.filenames.begin();

	FileData *curData;

	while (iter != g_FetchThread.filenames.end())
	{
		curData = (*iter);

		char fileName[30];

		g_LibSys.GetFileFromPath(fileName, sizeof(fileName), curData->filename->c_str());

		if (strcmpi(fileName, file) == 0)
		{
			g_SMAPI->ConPrintf("MD5 Sum: %s\n", curData->checksum);
			return;
		}

		iter++;
	}

	g_SMAPI->ConPrint("File not found!\n");
}

FetcherThread::~FetcherThread()
{
	//delete filenames;
	SourceHook::CVector<FileData *>::iterator iter = filenames.begin();

	FileData *curData;

	while (iter != filenames.end())
	{
		curData = (*iter);
		delete curData->filename;
		delete curData;
		iter = filenames.erase(iter);
	}
}

FetcherThread::FetcherThread() 
{
	memtable = new BaseMemTable(4096);
	wasSuccess = false;
}

