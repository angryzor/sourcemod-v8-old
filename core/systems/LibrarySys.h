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

#ifndef _INCLUDE_SOURCEMOD_SYSTEM_LIBRARY_H_
#define _INCLUDE_SOURCEMOD_SYSTEM_LIBRARY_H_

#include <ILibrarySys.h>
#include "sm_platform.h"

using namespace SourceMod;

#if defined PLATFORM_WINDOWS
typedef HMODULE		LibraryHandle;
#elif defined PLATFORM_POSIX
typedef void *		LibraryHandle;
#endif

class CDirectory : public IDirectory
{
public:
	CDirectory(const char *path);
	~CDirectory();
public:
	virtual bool MoreFiles();
	virtual void NextEntry();
	virtual const char *GetEntryName();
	virtual bool IsEntryDirectory();
	virtual bool IsEntryFile();
	virtual bool IsEntryValid();
public:
	bool IsValid();
private:
#if defined PLATFORM_WINDOWS
	HANDLE m_dir;
	WIN32_FIND_DATAA m_fd;
#elif defined PLATFORM_POSIX
	DIR *m_dir;
	struct dirent *ep;
	char m_origpath[PLATFORM_MAX_PATH];
#endif
};

class CLibrary : public ILibrary
{
public:
	CLibrary(LibraryHandle me);
	~CLibrary();
public:
	virtual void CloseLibrary();
	virtual void *GetSymbolAddress(const char *symname);
private:
	LibraryHandle m_lib;
};

class LibrarySystem : public ILibrarySys
{
public:
	virtual ILibrary *OpenLibrary(const char *path, char *error, size_t err_max);
	virtual IDirectory *OpenDirectory(const char *path);
	virtual void CloseDirectory(IDirectory *dir);
	virtual bool PathExists(const char *path);
	virtual bool IsPathFile(const char *path);
	virtual bool IsPathDirectory(const char *path);
	virtual void GetPlatformError(char *error, size_t err_max);
	virtual size_t PathFormat(char *buffer, size_t len, const char *fmt, ...);
};

extern LibrarySystem g_LibSys;

#endif //_INCLUDE_SOURCEMOD_SYSTEM_LIBRARY_H_
