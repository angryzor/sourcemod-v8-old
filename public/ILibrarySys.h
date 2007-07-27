/**
 * vim: set ts=4 :
 * ===============================================================
 * SourceMod, Copyright (C) 2004-2007 AlliedModders LLC. 
 * All rights reserved.
 * ===============================================================
 *
 *  This file is part of the SourceMod/SourcePawn SDK.  This file may only be 
 * used or modified under the Terms and Conditions of its License Agreement, 
 * which is found in public/licenses/LICENSE.txt.  As of this notice, derivative 
 * works must be licensed under the GNU General Public License (version 2 or 
 * greater).  A copy of the GPL is included under public/licenses/GPL.txt.
 * 
 * To view the latest information, see: http://www.sourcemod.net/license.php
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_LIBRARY_INTERFACE_SYS_H_
#define _INCLUDE_SOURCEMOD_LIBRARY_INTERFACE_SYS_H_

/**
 * @file ILibrarySys.h
 * @brief Defines platform-dependent operations, such as opening libraries and files.
 */

#include <IShareSys.h>

namespace SourceMod
{
	#define SMINTERFACE_LIBRARYSYS_NAME		"ILibrarySys"
	#define SMINTERFACE_LIBRARYSYS_VERSION	3

	class ILibrary
	{
	public:
		/** Virtual destructor (calls CloseLibrary) */
		virtual ~ILibrary()
		{
		};
	public:
		/**
		 * @brief Closes dynamic library and invalidates pointer.
		 */
		virtual void CloseLibrary() =0;

		/**
		 * @brief Retrieves a symbol pointer from the dynamic library.
		 *
		 * @param symname	Symbol name.
		 * @return			Symbol pointer, NULL if not found.
		 */
		virtual void *GetSymbolAddress(const char *symname) =0;
	};

	/**
	 * @brief Directory browsing abstraction.
	 */
	class IDirectory
	{
	public:
		/** Virtual destructor */
		virtual ~IDirectory()
		{
		}
	public:
		/**
		 * @brief Returns true if there are more files to read, false otherwise.
		 */
		virtual bool MoreFiles() =0;

		/**
		 * @brief Advances to the next entry in the stream.
		 */
		virtual void NextEntry() =0;

		/**
		 * @brief Returns the name of the current entry.
		 */
		virtual const char *GetEntryName() =0;

		/**
		 * @brief Returns whether the current entry is a directory.
		 */
		virtual bool IsEntryDirectory() =0;

		/**
		 * @brief Returns whether the current entry is a file.
		 */
		virtual bool IsEntryFile() =0;

		/**
		 * @brief Returns true if the current entry is valid
		 *        (Used similarly to MoreFiles).
		 */
		virtual bool IsEntryValid() =0;
	};

	/**
	 * @brief Contains various operating system specific code.
	 */
	class ILibrarySys : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName()
		{
			return SMINTERFACE_LIBRARYSYS_NAME;
		}
		virtual unsigned int GetInterfaceVersion()
		{
			return SMINTERFACE_LIBRARYSYS_VERSION;
		}
	public:
		/**
		 * @brief Opens a dynamic library file.
		 * 
		 * @param path		Path to library file (.dll/.so).
		 * @param error		Buffer for any error message (may be NULL).
		 * @param maxlength	Maximum length of error buffer.
		 * @return			Pointer to an ILibrary, NULL if failed.
		 */
		virtual ILibrary *OpenLibrary(const char *path, char *error, size_t maxlength) =0;

		/**
		 * @brief Opens a directory for reading.
		 *
		 * @param path		Path to directory.
		 * @return			Pointer to an IDirectory, NULL if failed.
		 */
		virtual IDirectory *OpenDirectory(const char *path) =0;

		/**
		 * @brief Closes a directory and frees its handle.
		 * 
		 * @param dir		Pointer to IDirectory.
		 */
		virtual void CloseDirectory(IDirectory *dir) =0;

		/**
		 * @brief Returns true if a path exists.
		 */
		virtual bool PathExists(const char *path) =0;

		/**
		 * @brief Returns true if the path is a normal file.
		 */
		virtual bool IsPathFile(const char *path) =0;

		/**
		 * @brief Returns true if the path is a normal directory.
		 */
		virtual bool IsPathDirectory(const char *path) =0;

		/**
		 * @brief Gets a platform-specific error message.
		 * This should only be called when an ILibrary function fails.
		 * Win32 equivalent: GetLastError() + FormatMessage()
		 * POSIX equivalent: errno + strerror()
		 *
		 * @param error		Error message buffer.
		 * @param maxlength	Maximum length of error buffer.
		 */
		virtual void GetPlatformError(char *error, size_t maxlength) =0;

		/**
		 * @brief Formats a string similar to snprintf(), except
		 * corrects all non-platform compatible path separators to be
		 * the correct platform character.
		 *
		 * @param buffer	Output buffer pointer.
		 * @param maxlength	Output buffer size.
		 * @param pathfmt	Format string of path.
		 * @param ...		Format string arguments.
		 */
		virtual size_t PathFormat(char *buffer, size_t maxlength, const char *pathfmt, ...) =0;

		/**
		 * @brief Returns a pointer to the extension in a filename.
		 *
		 * @param filename	Name of file from which the extension should be extracted.
		 * @return			Pointer to file extension.
		 */
		virtual const char *GetFileExtension(const char *filename) =0;

		/**
		 * @brief Creates a directory.
		 *
		 * @param path		Full, absolute path of the directory to create.
		 * @return			True on success, false otherwise.
		 */
		virtual bool CreateFolder(const char *path) =0;
	};
}

#endif //_INCLUDE_SOURCEMOD_LIBRARY_INTERFACE_SYS_H_
