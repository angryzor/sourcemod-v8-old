#ifndef _INCLUDE_SOURCEMOD_ROOT_CONSOLE_MENU_H_
#define _INCLUDE_SOURCEMOD_ROOT_CONSOLE_MENU_H_

/**
 * @brief Note: This interface is not exposed.
 * The reason should be obvious: we do not want users touching the "root" console menu.
 */

namespace SourceMod
{
	/**
	 * @brief Handles a root console menu action.
	 */
	class IRootConsoleCommand
	{
	public:
		virtual void OnRootConsoleCommand(const char *command, unsigned int argcount) =0;
	};

	/**
	 * @brief Manages the root console menu.
	 */
	class IRootConsole
	{
	public:
		/**
		 * @brief Adds a root console command handler.  The command must be unique.
		 *
		 * @param cmd			String containing the console command.
		 * @param text			Description text.
		 * @param pHandler		An IRootConsoleCommand pointer to handle the command.
		 * @return				True on success, false on too many commands or duplicate command.
		 */
		virtual bool AddRootConsoleCommand(const char *cmd, const char *text, IRootConsoleCommand *pHandler) =0;

		/**
		 * @brief Removes a root console command handler.
		 *
		 * @param cmd			String containing the console command.
		 * @param pHandler		An IRootConsoleCommand pointer for verification.
		 * @return				True on success, false otherwise.
		 */
		virtual bool RemoveRootConsoleCommand(const char *cmd, IRootConsoleCommand *pHandler) =0;

		/**
		 * @brief Prints text back to the console.
		 *
		 * @param fmt			Format of string.
		 * @param ...			Format arguments.
		 */
		virtual void ConsolePrint(const char *fmt, ...) =0;

		/**
		 * @brief Returns the string of an argument.
		 *
		 * @param argno			The index of the argument.
		 * @return				A string containing the argument, or nothing if invalid.
		 */
		virtual const char *GetArgument(unsigned int argno) =0;

		/**
		 * @brief Returns the number of arguments.
		 *
		 * @return				Number of arguments.
		 */
		virtual unsigned int GetArgumentCount() =0;

		/** 
		 * @brief Returns the entire argument string.
		 *
		 * @return				String containing all arguments.
		 */
		virtual const char *GetArguments() =0;
	};
};

#endif //_INCLUDE_SOURCEMOD_ROOT_CONSOLE_MENU_H_
