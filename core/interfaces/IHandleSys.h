#ifndef _INCLUDE_SOURCEMOD_HANDLESYSTEM_INTERFACE_H_
#define _INCLUDE_SOURCEMOD_HANDLESYSTEM_INTERFACE_H_

#include <IShareSys.h>
#include <sp_vm_types.h>

#define SMINTERFACE_HANDLESYSTEM_NAME			"IHandleSys"
#define SMINTERFACE_HANDLESYSTEM_VERSION		1

#define DEFAULT_IDENTITY			NULL

namespace SourceMod
{
	/**
	 * Both of these types have invalid values of '0' for error checking.
	 */
	typedef unsigned int HandleType_t;
	typedef unsigned int Handle_t;

	class SourcePawn::IPluginContext;

	/**
	 * About type checking:
	 * Types can be inherited - a Parent type ("Supertype") can have child types.
	 * When accessing handles, type checking is done.  This table shows how this is resolved:
	 *
	 * HANDLE		CHECK		->  RESULT
	 * ------		-----			------
	 * Parent		Parent			Success
	 * Parent		Child			Fail
	 * Child		Parent			Success
	 * Child		Child			Success
	 */

	enum HandleError
	{
		HandleError_None = 0,		/* No error */
		HandleError_Changed,		/* The handle has been freed and reassigned */
		HandleError_Type,			/* The handle has a different type registered */
		HandleError_Freed,			/* The handle has been freed */
		HandleError_Index,			/* generic internal indexing error */
		HandleError_Access,			/* No access permitted to free this handle */
		HandleError_Limit,			/* The limited number of handles has been reached */
		HandleError_Identity,		/* The identity token was not usable */
	};

	enum HandleAccessRight
	{
		HandleAccess_Create,		/* TYPE: Instances can be created by other objects (this makes it searchable) */
		HandleAccess_Read,			/* HANDLES: Can be read by other objects */
		HandleAccess_Delete,		/* HANDLES: Can be deleted by other objects */
		HandleAccess_Inherit,		/* TYPE: Can be inherited by new types */
		HandleAccess_Clone,			/* HANDLES: Can be cloned */
		/* ------------- */
		HandleAccess_TOTAL,			/* Total number of access rights */
	};

	struct HandleSecurity
	{
		HandleSecurity()
		{
			owner = NULL;
			access[HandleAccess_Create] = true;
			access[HandleAccess_Read] = true;
			access[HandleAccess_Delete] = true;
			access[HandleAccess_Inherit] = true;
			access[HandleAccess_Clone] = true;
		}
		IdentityToken_t *owner;				/* Owner of the handle */
		bool access[HandleAccess_TOTAL];	/* World access rights */
	};

	class IHandleTypeDispatch
	{
	public:
		virtual unsigned int GetDispatchVersion()
		{
			return SMINTERFACE_HANDLESYSTEM_VERSION;
		}
	public:
		/**
		 * @brief Called when destroying a handle.  Must be implemented.
		 */
		virtual void OnHandleDestroy(HandleType_t type, void *object) =0;
	};

	class IHandleSys : public SMInterface
	{
	public:
		virtual unsigned int GetInterfaceVersion()
		{
			return SMINTERFACE_HANDLESYSTEM_VERSION;
		}
		virtual const char *GetInterfaceName()
		{
			return SMINTERFACE_HANDLESYSTEM_NAME;
		}
	public:
		/**
		 * @brief Creates a new Handle type.  
		 * NOTE: Handle names must be unique if not private.
		 *
		 * @param name		Name of handle type (NULL or "" to be anonymous)
		 * @param dispatch	Pointer to a valid IHandleTypeDispatch object.
		 * @return			A new HandleType_t unique ID, or 0 on failure.
		 */
		virtual HandleType_t CreateType(const char *name, 
										IHandleTypeDispatch *dispatch) =0;

		/**
		 * @brief Creates a new Handle type.
		 * NOTE: Currently, a child type may not have its own children.
		 * NOTE: Handle names must be unique if not private.
		 *
		 * @param name		Name of handle type (NULL or "" to be anonymous)
		 * @param dispatch	Pointer to a valid IHandleTypeDispatch object.
		 * @param parent	Parent handle to inherit from, 0 for none.
		 * @param security	Pointer to a temporary HandleSecurity object, NULL to use default 
		 *					or inherited permissions.
		 * @param ident		Security token for any permissions.
		 * @return			A new HandleType_t unique ID, or 0 on failure.
		 */
		virtual HandleType_t CreateTypeEx(const char *name,
										  IHandleTypeDispatch *dispatch,
										  HandleType_t parent,
										  const HandleSecurity *security,
										  IdentityToken_t *ident) =0;


		/**
		 * @brief Creates a sub-type for a Handle.
		 * NOTE: Currently, a child type may not have its own children.
		 * NOTE: Handle names must be unique if not private.
		 * NOTE: This is a wrapper around the above.
		 *
		 * @param name		Name of a handle.
		 * @param parent	Parent handle type.
		 * @param dispatch	Pointer to a valid IHandleTypeDispatch object.
		 * @return			A new HandleType_t unique ID.
		 */
		virtual HandleType_t CreateChildType(const char *name, 
										     HandleType_t parent, 
										     IHandleTypeDispatch *dispatch) =0;

		/**
		 * @brief Removes a handle type.
		 * NOTE: This removes all child types.
		 *
		 * @param token		Identity token.  Removal fails if the token does not match.
		 * @param type		Type chain to remove.
		 * @return			True on success, false on failure.
		 */
		virtual bool RemoveType(HandleType_t type, IdentityToken_t *ident) =0;

		/**
		 * @brief Finds a handle type by name.
		 *
		 * @param name		Name of handle type to find (anonymous not allowed).
		 * @param type		Address to store found handle in (if not found, undefined).
		 * @return			True if found, false otherwise.
		 */
		virtual bool FindHandleType(const char *name, HandleType_t *type) =0;

		/**
		 * @brief Creates a new handle.
		 * 
		 * @param type		Type to use on the handle.
		 * @param object	Object to bind to the handle.
		 * @param owner		Owner for the handle.
		 * @param ident		Identity token if any security rights are needed.
		 * @return			A new Handle_t, or 0 on failure.
		 */
		virtual Handle_t CreateHandle(HandleType_t type, 
										void *object, 
										IdentityToken_t *owner, 
										IdentityToken_t *ident) =0;

		/**
		 * @brief Creates a new handle.
		 * NOTE: This is a wrapper around the above function.
		 * 
		 * @param type		Type to use on the handle.
		 * @param object	Object to bind to the handle.
		 * @param pOwner	Plugin context that will own this handle.  NULL for none.
		 * @param ident		Identity token if any security rights are needed.
		 * @return			A new Handle_t.
		 */
		virtual Handle_t CreateScriptHandle(HandleType_t type, 
											void *object, 
											SourcePawn::IPluginContext *pOwner,
											IdentityToken_t *ident) =0;

		/**
		 * @brief Frees the memory associated with a handle and calls any destructors.
		 * NOTE: This function will decrement the internal reference counter.  It will
		 * only perform any further action if the counter hits 0.
		 *
		 * @param type		Handle_t identifier to destroy.
		 * @param ident		Identity token, for destroying secure handles (NULL for none).
		 * @return			A HandleError error code.
		 */
		virtual HandleError FreeHandle(Handle_t handle, IdentityToken_t *ident) =0;

		/**
		 * @brief Clones a handle by adding to its internal reference count.  Its data,
		 * type, and security permissions remain the same.
		 *
		 * @param handle	Handle to duplicate.  Any non-free handle target is valid.
		 * @param newhandle	If non-NULL, stores the duplicated handle in the pointer.
		 * @param owner		New owner of cloned handle.
		 * @param ident		Security token, if needed.
		 * @return			A HandleError error code.
		 */
		virtual HandleError CloneHandle(Handle_t handle, Handle_t *newhandle, IdentityToken_t *owner, IdentityToken_t *ident) =0;

		/**
		 * @brief Retrieves the contents of a handle.
		 *
		 * @param handle	Handle_t from which to retrieve contents.
		 * @param type		Expected type to read as.  0 ignores typing rules.
		 * @param ident		Identity token to validate as.
		 * @param object	Optional address to store object in.
		 * @return			HandleError error code.
		 */
		virtual HandleError ReadHandle(Handle_t handle, HandleType_t type, IdentityToken_t *ident, void **object) =0;
	};
};

#endif //_INCLUDE_SOURCEMOD_HANDLESYSTEM_INTERFACE_H_
