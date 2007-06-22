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

#ifndef _INCLUDE_SOURCEMOD_MENU_SYSTEM_H_
#define _INCLUDE_SOURCEMOD_MENU_SYSTEM_H_

#include <IShareSys.h>
#include <IHandleSys.h>

#define SMINTERFACE_MENUMANAGER_NAME		"IMenuManager"
#define SMINTERFACE_MENUMANAGER_VERSION		6

/**
 * @file IMenuManager.h
 * @brief Abstracts on-screen menus for clients.
 */

namespace SourceMod
{
	/**
	 * @brief Used to determine how an item selection is interpreted.
	 */
	enum ItemSelection
	{
		ItemSel_None,					/**< Invalid selection */
		ItemSel_Back,					/**< Go back one page (really "Previous") */
		ItemSel_Next,					/**< Go forward one page */
		ItemSel_Exit,					/**< Menu was exited */
		ItemSel_Item,					/**< Valid item selection */
		ItemSel_ExitBack,				/**< Sends MenuEnd_ExitBack */
	};

	/**
	 * @brief Used to determine which order to search for items in.
	 */
	enum ItemOrder
	{
		ItemOrder_Ascending,			/**< Items should be drawn ascendingly */
		ItemOrder_Descending,			/**< Items should be drawn descendingly */
	};

	/**
	 * @brief Pairs an item type with an item menu position.
	 */
	struct menu_slots_t
	{
		ItemSelection type;				/**< Item selection type */
		unsigned int item;				/**< Item position, if applicable */
	};

	class IBaseMenu;
	class IMenuPanel;
	class IMenuHandler;

	/**
	 * @brief Describes menu display information.
	 */
	struct menu_states_t
	{
		unsigned int apiVers;			/**< Must be filled with the API version */
		IBaseMenu *menu;				/**< Menu pointer, or NULL if there is only a display */
		IMenuHandler *mh;				/**< Menu callbacks handler */
		unsigned int firstItem;			/**< MENU ONLY: First item displayed on the last page */
		unsigned int lastItem;			/**< MENU ONLY: Last item displayed on the last page */
		menu_slots_t slots[11];			/**< MENU ONLY: Item selection table (first index is 1) */
	};

	#define ITEMDRAW_DEFAULT		(0)		/**< Item should be drawn normally */
	#define ITEMDRAW_DISABLED		(1<<0)	/**< Item is drawn but not selectable */
	#define ITEMDRAW_RAWLINE		(1<<1)	/**< Item should be a raw line, without a slot */
	#define ITEMDRAW_NOTEXT			(1<<2)	/**< No text should be drawn */
	#define ITEMDRAW_SPACER			(1<<3)	/**< Item should be drawn as a spacer, if possible */
	#define ITEMDRAW_IGNORE	((1<<1)|(1<<2))	/**< Item should be completely ignored (rawline + notext) */
	#define ITEMDRAW_CONTROL		(1<<4)	/**< Item is control text (back/next/exit) */

	/**
	 * @brief Information about item drawing.
	 */
	struct ItemDrawInfo
	{
		ItemDrawInfo()
		{
			style = 0;
			display = NULL;
		}
		ItemDrawInfo(const char *DISPLAY, unsigned int STYLE=ITEMDRAW_DEFAULT) 
			: display(DISPLAY), style(STYLE)
		{
		}
		const char *display;			/**< Display text (NULL for none) */
		unsigned int style;				/**< ITEMDRAW style flags */
	};

	/**
	 * @brief Reasons for a menu dying.
	 */
	enum MenuCancelReason
	{
		MenuCancel_Disconnected = -1,	/**< Client dropped from the server */
		MenuCancel_Interrupted = -2,	/**< Client was interrupted with another menu */
		MenuCancel_Exit = -3,			/**< Client selected "exit" on a paginated menu */
		MenuCancel_NoDisplay = -4,		/**< Menu could not be displayed to the client */
		MenuCancel_Timeout = -5,		/**< Menu timed out */
		MenuCancel_ExitBack = -6,		/**< Client selected "exit back" on a paginated menu */
	};

	/**
	 * @brief Reasons a menu ended.
	 */
	enum MenuEndReason
	{
		MenuEnd_Selected = 0,				/**< Menu item was selected */
		MenuEnd_VotingDone = -1,			/**< Voting finished */
		MenuEnd_VotingCancelled = -2,		/**< Voting was cancelled */
		MenuEnd_Cancelled = -3,				/**< Menu was uncleanly cancelled */
		MenuEnd_Exit = -4,					/**< Menu was cleanly exited via "exit" */
		MenuEnd_ExitBack = -5,				/**< Menu was cleanly exited via "back" */
	};


	#define MENU_NO_PAGINATION			0		/**< Menu should not be paginated (10 items max) */
	#define MENU_TIME_FOREVER			0		/**< Menu should be displayed as long as possible */

	/**
	 * @brief Extended menu options.
	 */
	enum MenuOption
	{
		MenuOption_IntroMessage,				/**< CONST CHAR *: Valve menus only; defaults to:
													 "You have a menu, hit ESC"
													 */
		MenuOption_IntroColor,					/**< INT[4]: Valve menus only; specifies the intro message colour
													 using R,G,B,A (defaults to 255,0,0,255)
													 */
		MenuOption_Priority,					/**< INT *: Valve menus only; priority (less is higher) */
	};

	/**
	 * @brief Describes the menu a player is viewing.
	 */
	enum MenuSource
	{
		MenuSource_None = 0,					/**< No menu is being displayed */
		MenuSource_External = 1,				/**< External menu, no pointer */
		MenuSource_BaseMenu = 2,				/**< An IBaseMenu pointer. */
		MenuSource_Display = 3,					/**< IMenuPanel source, no pointer */
	};

	class IMenuStyle;

	/**
	 * @brief Sets how a raw menu should be drawn.
	 */
	class IMenuPanel
	{
	public:
		/**
		 * @brief Returns the parent IMenuStyle pointer.
		 *
		 * @return				IMenuStyle pointer which created
		 *						this object.
		 */
		virtual IMenuStyle *GetParentStyle() =0;

		/**
		 * @brief Resets/clears the cached display text.
		 */
		virtual void Reset() =0;

		/**
		 * @brief Sets how the title should be drawn.
		 *
		 * @param text			Text string to display for the title.
		 * @param onlyIfEmpty	Only sets the title if one does not already 
		 *						exist.
		 */
		virtual void DrawTitle(const char *text, bool onlyIfEmpty=false) =0;

		/**
		 * @brief Adds an item to the menu and returns the position (1-10).
		 *
		 * Note: Item will fail to draw if there are too many items,
		 * or the item is not drawable (for example, invisible).	
		 * 
		 * @return				Item draw position, or 0 on failure.
		 */
		virtual unsigned int DrawItem(const ItemDrawInfo &item) =0;

		/**
		 * @brief Draws a raw line of text, if supported.  The line does not 
		 * need to be newline terminated.
		 *
		 * @return				True on success, false if not supported.
		 */
		virtual bool DrawRawLine(const char *rawline) =0;

		/**
		 * @brief Sets an extended menu option.
		 *
		 * @param option		Option type.
		 * @param valuePtr		Pointer of the type expected by the option.
		 * @return				True on success, false if option or value is not supported.
		 */
		virtual bool SetExtOption(MenuOption option, const void *valuePtr) =0;

		/**
		 * @brief Returns whether the display is capable of rendering an item
		 * with the given flags.
		 *
		 * @param flags			ITEMDRAW flags.
		 * @return				True if renderable, false otherwise.
		 */
		virtual bool CanDrawItem(unsigned int drawFlags) =0;

		/**
		 * @brief Sends the menu display to a client.
		 *
		 * @param client		Client index to display to.
		 * @param handler		Menu handler to use.
		 * @param time			Time to hold menu for.
		 * @return				True on success, false otherwise.
		 */
		virtual bool SendDisplay(int client, IMenuHandler *handler, unsigned int time) =0;

		/**
		 * @brief Destroys the display object.
		 */
		virtual void DeleteThis() =0;

		/**
		 * @brief Sets the selectable key map.  Returns false if the function 
		 * is not supported.
		 * 
		 * @param keys			A bit string where each bit N-1 specifies 
		 *						that key N is selectable (key 0 is bit 9).  
		 *						If the selectable key map is 0, it will be
		 *						automatically set to allow 0.
		 * @return				True on success, false if not supported.
		 */
		virtual bool SetSelectableKeys(unsigned int keymap) =0;

		/**
		 * @brief Returns the current key position.
		 *
		 * @return				Current key position starting at 1.
		 */
		virtual unsigned int GetCurrentKey() =0;

		/**
		 * @brief Sets the next key position.  This cannot be used
		 * to traverse backwards.
		 *
		 * @param key			Key that is greater or equal to
		 *						GetCurrentKey().
		 * @return				True on success, false otherwise.
		 */
		virtual bool SetCurrentKey(unsigned int key) =0;
	};

	/**
	 * @brief Describes a "MenuStyle" system which manages
	 * menu drawing and construction.
	 */
	class IMenuStyle
	{
	public:
		/**
		 * @brief Returns the style API version.
		 *
		 * @return				API version.
		 */
		virtual unsigned int GetStyleAPIVersion()
		{
			return SMINTERFACE_MENUMANAGER_VERSION;
		}

		/**
		 * @brief Returns the name of the menu style.
		 *
		 * @return				String containing the style name.
		 */
		virtual const char *GetStyleName() =0;

		/**
		 * @brief Creates an IMenuPanel object.
		 *
		 * Note: the object should be freed using ::DeleteThis.
		 *
		 * @return				IMenuPanel object.
		 */
		virtual IMenuPanel *CreatePanel() =0;

		/**
		 * @brief Creates an IBaseMenu object of this style.
		 *
		 * Note: the object should be freed using IBaseMenu::Destroy.
		 *
		 * @param handler		IMenuHandler pointer.
		 * @param pOwner		Optional IdentityToken_t owner for handle 
		 *						creation.
		 * @return				An IBaseMenu pointer.
		 */
		virtual IBaseMenu *CreateMenu(IMenuHandler *handler, IdentityToken_t *pOwner=NULL) =0;

		/**
		 * @brief Returns the maximum number of items per page.
		 *
		 * @return				Number of items per page.
		 */
		virtual unsigned int GetMaxPageItems() =0;

		/**
		 * @brief Returns whether or not a client is viewing a menu.
		 *
		 * @param client		Client index.
		 * @param object		Optional pointer to retrieve menu object,
		 *						if any.
		 * @return				MenuSource value.
		 */
		virtual MenuSource GetClientMenu(int client, void **object) =0;

		/**
		 * @brief Cancels a client's menu.
		 *
		 * @param client		Client index.
		 * @param autoIgnore	If true, no menus can be created during 
		 *						the cancellation process.
		 * @return				True if a menu was cancelled, false otherwise.
		 */
		virtual bool CancelClientMenu(int client, bool autoIgnore=false) =0;

		/**
		 * @brief Returns a Handle the IMenuStyle object.
		 *
		 * @return				Handle_t pointing to this object.
		 */
		virtual Handle_t GetHandle() =0;
	};

	/**
	 * @brief High-level interface for building menus.
	 */
	class IBaseMenu
	{
	public:
		/**
		 * @brief Appends an item to the end of a menu.
		 *
		 * @param info			Item information string.
		 * @param draw			Default drawing information.
		 * @return				True on success, false on item limit reached.
		 */
		virtual bool AppendItem(const char *info, const ItemDrawInfo &draw) =0;

		/**
		 * @brief Inserts an item into the menu before a certain position; 
		 * the new item will be at the given position and all next items 
		 * pushed forward.
		 *
		 * @param position		Position, starting from 0.
		 * @param info			Item information string.
		 * @param draw			Default item draw info.
		 * @return				True on success, false on invalid menu position
		 */
		virtual bool InsertItem(unsigned int position, const char *info, const ItemDrawInfo &draw) =0;

		/**
		 * @brief Removes an item from the menu.
		 *
		 * @param position		Position, starting from 0.
		 * @return				True on success, false on invalid menu position.
		 */
		virtual bool RemoveItem(unsigned int position) =0;

		/**
		 * @brief Removes all items from the menu.
		 */
		virtual void RemoveAllItems() =0;

		/**
		 * @brief Returns an item's info.
		 *
		 * @param position		Position, starting from 0.
		 * @param draw			Optional pointer to store a draw information.
		 * @return				Info string pointer, or NULL if position was invalid.
		 */
		virtual const char *GetItemInfo(unsigned int position, ItemDrawInfo *draw) =0;

		/**
		 * @brief Returns the number of items.
		 *
		 * @return				Number of items in the menu.
		 */
		virtual unsigned int GetItemCount() =0;

		/** 
		 * @brief Sets the menu's pagination,.
		 *
		 * @param itemsPerPage	Number of items per page, or MENU_NO_PAGINATION.
		 * @return				True on success, false if itemsPerPage is too large.
		 */
		virtual bool SetPagination(unsigned int itemsPerPage) =0;

		/**
		 * @brief Returns an item's pagination.
		 *
		 * @return				Pagination setting.
		 */
		virtual unsigned int GetPagination() =0;

		/**
		 * @brief Returns the menu style.
		 * 
		 * @return				Menu style.
		 */
		virtual IMenuStyle *GetDrawStyle() =0;

		/**
		 * @brief Sets the menu's display title/message.
		 *
		 * @param message		Message (format options allowed).
		 */
		virtual void SetDefaultTitle(const char *message) =0;

		/**
		 * @brief Returns the menu's display/title message.
		 *
		 * @return				Message string.
		 */
		virtual const char *GetDefaultTitle() =0;

		/**
		 * @brief Sets an extended menu option.
		 *
		 * @param option		Option type.
		 * @param valuePtr		Pointer of the type expected by the option.
		 * @return				True on success, false if option or value is not supported.
		 */
		virtual bool SetExtOption(MenuOption option, const void *valuePtr) =0;

		/**
		 * @brief Creates a new IMenuPanel object using extended options specific
		 * to the IMenuStyle parent.  Titles, items, etc, are not copied.
		 *
		 * Note: The object should be freed with IMenuPanel::DeleteThis.
		 *
		 * @return				IMenuPanel pointer.
		 */
		virtual IMenuPanel *CreatePanel() =0;

		/**
		 * @brief Returns whether or not the menu should have an "Exit" button for
		 * paginated menus.
		 *
		 * @return				True to have an exit button, false otherwise.
		 */
		virtual bool GetExitButton() =0;
		
		/**
		 * @brief Sets whether or not the menu should have an "Exit" button for
		 * paginated menus.
		 *
		 * @param set			True to enable, false to disable the exit button.
		 * @return				True on success, false if the exit button is 
		 *						non-optional.
		 */
		virtual bool SetExitButton(bool set) =0;

		/**
		 * @brief Sends the menu to a client.
		 *
		 * @param client		Client index to display to.
		 * @param time			Time to hold menu for.
		 * @return				True on success, false otherwise.
		 */
		virtual bool Display(int client, unsigned int time) =0;

		/**
		 * @brief Destroys the menu and frees all associated resources.
		 *
		 * @param releaseHandle	If true, the Handle will be released
		 *						in the destructor.  This should be set
		 *						to true except for IHandleTypeDispatch
		 *						destructors.
		 */
		virtual void Destroy(bool releaseHandle=true) =0;

		/**
		 * @brief Cancels the menu on all client's displays.  While the menu is
		 * being cancelled, the menu may not be re-displayed to any clients.
		 * If a vote menu is currently active, it will be cancelled as well.
		 *
		 * @return				Number of menus cancelled.
		 */
		virtual void Cancel() =0;

		/**
		 * @brief Returns the menu's Handle.  The Handle is automatically
		 * removed when the menu is destroyed.
		 *
		 * @return				Handle_t handle value.
		 */
		virtual Handle_t GetHandle() =0;

		/**
		 * @brief Sends a menu to multiple clients as a vote menu.  All callbacks
		 * will be sent as normal, except two extras, OnMenuVoteStart and 
		 * OnMenuVoteEnd, will be called.
		 *
		 * @param clients		Array of client indexes.
		 * @param numClients	Number of client indexes in the array.
		 * @param maxTime		Maximum amount of time to hold the vote.
		 * @param flags			Optional voting flags (currently unused).
		 * @return				True on success, false if a vote is already in
		 *						progress (the menu must be cancelled first).
		 */
		virtual bool BroadcastVote(int clients[], 
			unsigned int numClients, 
			unsigned int maxTime,
			unsigned int flags=0) =0;

		/**
		 * @brief Returns whether a vote menu is active.
		 *
		 * @return				True if a vote menu is active, false otherwise.
		 */
		virtual bool IsVoteInProgress() =0;

		/**
		 * @brief Returns whether to draw a "Back" button on the first page.
		 * ExitBack buttons are disabled by default.
		 *
		 * @return				True if enabled, false otherwise.
		 */
		virtual bool GetExitBackButton() =0;

		/**
		 * @brief Sets whether to draw a "Back" button on the first page.
		 * ExitBack buttons are disabled by default.
		 *
		 * @param set			True to enable, false to disable.
		 */
		virtual void SetExitBackButton(bool set) =0;
	};

	/** 
	 * @brief Contains callbacks for menu actions.
	 */
	class IMenuHandler
	{
	public:
		/**
		 * @brief Returns the menu api verison.
		 *
		 * @return				Menu API version.
		 */
		virtual unsigned int GetMenuAPIVersion2()
		{
			return SMINTERFACE_MENUMANAGER_VERSION;
		}

		/** 
		 * @brief A display/selection cycle has started.
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void OnMenuStart(IBaseMenu *menu)
		{
		}

		/**
		 * @brief Called before a menu is being displayed.  This is where
		 * you can set an alternate title on the menu.
		 *
		 * @param menu			Menu pointer.
		 * @param client		Client index.
		 * @param display		IMenuPanel pointer.
		 */
		virtual void OnMenuDisplay(IBaseMenu *menu, int client, IMenuPanel *display)
		{
		}

		/**
		 * @brief Called when an item is selected.
		 *
		 * @param menu			Menu pointer.
		 * @param client		Client that selected the item.
		 * @param item			Item number.
		 */
		virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item)
		{
		}

		/**
		 * @brief An active menu display was dropped from a client.
		 *
		 * @param menu			Menu pointer.
		 * @param client		Client that had the menu.
		 * @param reason		Menu cancellation reason.
		 */
		virtual void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
		{
		}

		/**
		 * @brief A display/selection cycle has ended.
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
		}

		/**
		 * @brief Called when the menu object is destroyed.
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void OnMenuDestroy(IBaseMenu *menu)
		{
		}

		/**
		 * @brief Called when requesting how to render an item.
		 *
		 * @param menu			Menu pointer.
		 * @param client		Client index receiving the menu.
		 * @param item			Item number in the menu.
		 * @param style			ITEMSTYLE flags, by reference for modification.
		 */
		virtual void OnMenuDrawItem(IBaseMenu *menu, int client, unsigned int item, unsigned int &style)
		{
		}

		/**
		 * @brief Called when requesting how to draw an item's text.
		 *
		 * @param menu			Menu pointer.
		 * @param client		Client index receiving the menu.
		 * @param item			Item number in the menu.
		 * @param display		Pointer to the display text string (changeable).
		 */
		virtual void OnMenuDisplayItem(IBaseMenu *menu, int client, unsigned int item, const char **display)
		{
		}

		/**
		 * @brief Called when a vote has been started and displayed to 
		 * clients.  This is called after OnMenuStart() and OnMenuDisplay(),
		 * but before OnMenuSelect().
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void OnMenuVoteStart(IBaseMenu *menu)
		{
		}

		/**
		 * @brief Called when a vote ends.  This is automatically called by the 
		 * wrapper, and never needs to called from a style implementation.  
		 *
		 * This function does not replace OnMenuEnd(), nor does it have the 
		 * same meaning as OnMenuEnd(), meaning you should not destroy a menu
		 * while it is in this function.
		 *
		 * @param menu			Menu pointer.
		 * @param item			Item position that was chosen by a majority.
		 * @param winningVotes	Number of votes from the winning item.
		 * @param totalVotes	Number of votes total.
		 */
		virtual void OnMenuVoteEnd(IBaseMenu *menu,
			unsigned int item, 
			unsigned int winningVotes,
			unsigned int totalVotes)
		{
		}

		/**
		 * @brief Called when a vote is cancelled.  If this is called, then 
		 * OnMenuVoteEnd() will not be called.  In both cases, OnMenuEnd will 
		 * always be called.
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void OnMenuVoteCancel(IBaseMenu *menu)
		{
		}
	};

	/**
	 * @brief Contains functions for managing a vote handler.
	 */
	class IVoteMenuHandler : public IMenuHandler
	{
	public:
		/**
		 * @brief Returns whether or not a vote is in progress.
		 *
		 * @return				True if a vote is in progress, false otherwise.
		 */
		virtual bool IsVoteInProgress() =0;

		/**
		 * @brief Use this to mark the vote as in progress (start).
		 *
		 * @param menu			Menu pointer.
		 */
		virtual void InitializeVoting(IBaseMenu *menu) =0;

		/**
		 * @brief Use this to notify that all clients' displays have been
		 * processed (i.e., there are no more clients to display to).
		 */
		virtual void StartVoting() =0;

		/**
		 * @brief Notifies the vote handler that the voting should be 
		 * cancelled.  
		 *
		 * Cancellation is not immediate and will only occur once every menu 
		 * has been cancelled from clients.  Thus this should only be called
		 * from the beginning of IBaseMenu::Cancel.
		 */
		virtual void CancelVoting() =0;
	};

	/**
	 * @brief Manages menu creation and displaying.
	 */
	class IMenuManager : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName()
		{
			return SMINTERFACE_MENUMANAGER_NAME;
		}
		virtual unsigned int GetInterfaceVersion()
		{
			return SMINTERFACE_MENUMANAGER_VERSION;
		}
		virtual bool IsVersionCompatible(unsigned int version)
		{
			if (version < 5 || version > GetInterfaceVersion())
			{
				return false;
			}
			return true;
		}
	public:
		/**
		 * @brief Finds a style by name.
		 *
		 * @param name			Name of the style (case insensitive).
		 * @return				IMenuStyle pointer, or NULL if not found.
		 */
		virtual IMenuStyle *FindStyleByName(const char *name) =0;

		/**
		 * @brief Returns the default draw style Core is using.
		 *
		 * @return				Menu style pointer.
		 */
		virtual IMenuStyle *GetDefaultStyle() =0;

		/**
		 * @brief Given a set of menu states, converts it to an IDisplay object.
		 *
		 * The state parameter is both INPUT and OUTPUT.
		 * INPUT: menu, mh, firstItem, lastItem
		 * OUTPUT: display, firstItem, lastItem, slots
		 *
		 * @param client		Client index.
		 * @param states		Menu states.
		 * @return				IDisplay pointer, or NULL if no items could be 
		 *						found in the IBaseMenu pointer, or NULL if any
		 *						other error occurred.  Any valid pointer must
		 *						be freed using IMenuPanel::DeleteThis.
		 */
		virtual IMenuPanel *RenderMenu(int client, menu_states_t &states, ItemOrder order) =0;

		/**
		 * @brief Creates a standard voting wrapper.  The wrapper is not 
		 * re-entrant; a second menu cannot be displayed on the same handler
		 * at the same time.
		 *
		 * @param mh			Menu handler to wrap around.
		 * @return				An IMenuHandler pointer that is a wrapper
		 *						around IMenuHandler callbacks to invoke
		 *						voting related callbacks.
		 */
		virtual IVoteMenuHandler *CreateVoteWrapper(IMenuHandler *mh) =0;

		/**
		 * @brief Frees a standard voting wrapper.
		 *
		 * @param mh			Menu handler pointer created by 
		 *						CreateVoteWrapper().  NULL values will be 
		 *						safely ignored.
		 */
		virtual void ReleaseVoteWrapper(IVoteMenuHandler *mh) =0;
	};
}

#endif //_INCLUDE_SOURCEMOD_MENU_SYSTEM_H_
