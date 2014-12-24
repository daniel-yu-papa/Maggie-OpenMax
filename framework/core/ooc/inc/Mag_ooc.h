/*
 * ooc.h
 * 
 * Copyright (c) 2011, old_fashioned. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
 /*
 * 2013-08-19 YJ: ported ooc code to Maggie-OpenMax project using.
 */
#ifndef __MAG_OOC_H__
#define __MAG_OOC_H__

#include <stddef.h>
#include "Mag_hal.h"

/*
* base definitions
*/
#ifndef FALSE
#define FALSE   0
#define TRUE (! FALSE)
#endif

/* Type qualifiers
 */

#ifndef ROM
#define ROM const
#endif

#ifndef ROM_ALLOC
#define ROM_ALLOC ROM
#endif

#ifndef OOC_COMPONENT_INITIALIZED
#define OOC_COMPONENT_INITIALIZED 0x12345678
#endif

typedef ROM struct ClassTable * Class;

typedef ROM struct _ClassCommonsTable * ClassCommons;

typedef		struct BaseObject * Object;

/* Class definitions.
 */
struct BaseVtable_stru
{
	Class			_class;	
	ClassCommons	_class_register_prev;
	ClassCommons	_class_register_next;
	int   			(* _destroy_check )( Object );
	int             _init_flag;
};

typedef		struct BaseVtable_stru * BaseVtable;

typedef            BaseVtable    Vtable;

struct BaseObject											
{
	Vtable	_vtab;
};

extern ROM struct ClassTable BaseClass;

/**	Interface ID.
 * 	Each interface has a unique ID.\n
 * 	In this implementation the ID is a pointer to an interafce descriptor table in the memory,
 *  similarly to the Class definition.
 *  @hideinitializer
 */
typedef ROM struct InterfaceID_struct * InterfaceID;

struct InterfaceOffsets_struct
{
	InterfaceID		id;					/* The interface ID of the implemented interface */
	size_t			vtab_offset;		/* The offset of the interface in the class's virtual table */
};

typedef ROM struct InterfaceOffsets_struct * Itable;

/*@}*/

/* Type identifiers
 */

enum ooc_TypeID { _OOC_TYPE_CLASS, _OOC_TYPE_INTERFACE, _OOC_TYPE_MIXIN };

typedef struct
{
	enum ooc_TypeID		value;
	ROM char *			name;				/* the name of the type (for information only) */
}
oocType;

typedef
struct _ClassCommonsTable
{
	oocType				type;				/* Type identifier */

	Vtable			    vtable;				/* the pointer to the virtual function's table */

}	ClassCommonsTable;

struct ClassTable
{
	ClassCommonsTable	c;

	const size_t		size;				/* size of the object */
	ROM   Class		 	parent;				/* parent of the class */
	const size_t		vtab_size;			/* the size of the vtable */
	Itable				itable;				/* the implemented interfaces' Itable */
	const size_t		itab_size;			/* the number of implemented Interfaces */
	

	void				(* init) ( Class thiz );        				/* class initializer */
	void				(* ctor) (Object self, const void * params );	/* constructor */
	void				(* dtor) (Object self, Vtable vtab);			/* destructor */
};

/** Class declaration macro.
 * This macro should be put int the publicly available header of the class.
 * Use:
 * @code
 * DeclareClass( MyClass, Base );
 * @endcode
 * @param	pClass	The name of the class.
 * @param	pParent	The name of the parent class of the class. Must be @c Base if class does not have other parent.
 * @hideinitializer
 */

#define	DeclareClass( pClass, pParent )  							\
	typedef struct pClass ## Object * pClass;						\
	extern ROM struct ClassTable pClass ## Class

/** Class members and member functions declaration macro.
 * This macro should be put int the implementation header of the class.
 * Use:
 * @code
 * ClassMembers( MyClass, Base, int (*func1)(pClass thiz, int param);  int (*func2)(pClass thiz);)
 *     int   my_data;
 * EndOfClassMembers;
 * @endcode
 * @param	pClass	The name of the class.
 * @param	pParent	The name of the parent class of the class. Must be @c Base if class does not have other parent.
 * @param    (*func1), (*func2)    The member functions 
 * @see		EndOfClassMembers
 * @hideinitializer
 */

#define ClassMembers( pClass, pParent, args...)						\
	struct pClass ## Object {								\
		struct pParent ## Object pParent;			        \
            args 
/** End of class members definition.
 * This macro terminates the @c ClassMembers block.
 * @see		ClassMembers()
 * @hideinitializer
 */
#define EndOfClassMembers	}

/*****************************************************
*  Virtual Function Related Definitions
******************************************************/
#define _vtab_access_prototype( pClass )										\
			pClass ## Vtable pClass ## Virtual( pClass this )
			
#define _vtab_access_fn( pClass )												\
		_vtab_access_prototype( pClass )										\
			{ return ( pClass ## Vtable ) ( ((struct BaseObject *) this )->_vtab ); }

#define _parent_vtab_access_prototype( pClass, pParent )						\
			pParent ## Vtable pClass ## ParentVirtual( pClass this )

#define _parent_vtab_access_fn( pClass, pParent )								\
			_parent_vtab_access_prototype( pClass, pParent )	{				\
				MAG_ASSERT ( ((struct BaseObject *) this )->_vtab->_class->parent != &BaseClass ); \
				return ( pParent ## Vtable ) ( ((struct BaseObject *) this )->_vtab->_class->parent->c.vtable ); \
				}

#define _declare_vtab_access( pClass, pParent )									\
			static __inline__ _vtab_access_fn( pClass )										\
			static __inline__ _parent_vtab_access_fn( pClass, pParent )
			
/** Class virtual functions declaration macro.
 * This macro should be put int the publicly available header of the class.
 * Use:
 * @code
 * Virtuals( MyClass, Base )
 *     int   (* my_method) ( MyClass thiz, int param );
 * EndOfVirtuals;
 * @endcode
 * @param	pClass	The name of the class.
 * @param	pParent	The name of the parent class of the class. Must be @c Base if class does not have other parent.
 * @see		EndOfVirtuals
 * @hideinitializer
 */

#define Virtuals( pClass, pParent )												\
																				\
	typedef struct pClass ## Vtable_stru * pClass ## Vtable;                    \
                                                                                \
	_declare_vtab_access( pClass, pParent )										\
																				\
	struct pClass ## Vtable_stru {												\
		struct pParent ## Vtable_stru	pParent;
		
/** End of virtual functions.
 * This macro terminates the @c Virtuals block.
 * @see		Virtuals()
 * @hideinitializer
 */

#define EndOfVirtuals	}


/** Class allocation macro.
 * This macro should be put int the implementation file of the class.
 * Use:
 * @code
 * AllocateClass( MyClass, Base );
 * @endcode
 * @param	pClass	The name of the class.
 * @param	pParent	The name of the parent class of the class. Must be @c Base if class does not have other parent.
 * @hideinitializer
 */

#define AllocateClass( pClass, pParent )					          \
															          \
	static void   pClass ## _initialize ( Class );	                  \
	static void   pClass ## _constructor( pClass, const void * );     \
	static void   pClass ## _destructor ( pClass, pClass ## Vtable ); \
																      \
	static struct pClass ## Vtable_stru pClass ## VtableInstance;     \
															\
	ROM_ALLOC												\
	struct ClassTable pClass ## Class = {					\
		{													\
			{												\
				_OOC_TYPE_CLASS,							\
				(ROM char *) #pClass						\
			},												\
			(Vtable) & pClass ## VtableInstance		        \
		},													\
		sizeof( struct pClass ## Object ),					\
		& pParent ## Class,	                                \
		sizeof( struct pClass ## Vtable_stru ),				\
		(Itable) NULL, 										\
		(size_t) 0,											\
												pClass ## _initialize,	\
		(void (*)( Object, const void *)) 		pClass ## _constructor,	\
		(void (*)( Object, Vtable ))        	pClass ## _destructor,	\
		}

/********************************** 
 *Object and class manipulation functions
 **********************************/
 
/** Checks if a Class has a parent class.
 * This is a convenient macro. 
 * @param	this		The Class that should be checked.
 * @return	@c TRUE or @c FALSE. Returns @c TRUE if the Class has a parent class.
 * 			Does not throw an Exception.
 */
 
#define ooc_class_has_parent(this) (this->parent != &BaseClass)

/** Initializes a class.
 * Initializes a class using the class name. This is a convenient macro.
 * @param	pClass	Tha name of the class.
 * @see		_ooc_init_class().
 * @warning	This method is not thread safe!
 */
 
#define		ooc_init_class( pClass ) _ooc_init_class( & pClass ## Class )

/** Initializes a class by class table pointer.
 * Initializes the class pointed by the parameter.
 * @param 	class_ptr	Pointer to the class description table.
 * @see		ooc_init_class() for more convenient use.
 * @warning	This method is not thread safe!
 */

void		_ooc_init_class( const Class class_ptr );

/** Creates a new object of a class.
 * Creates a new object of a class with the given construction parameters.
 * @param	pClass	The name of the class.
 * @param	pPar	Pointer to the construction parameters. This pointer is passed to the constructor without any check.
 * @return	The newly created Object.
 * @note	ooc_new may throw an Exception! This is a convenient macro for ooc_new_classptr().
 * @see		ooc_new_classptr()
 */
 
#define		ooc_new( pClass, pPar ) ((pClass) ooc_new_classptr( & pClass ## Class, pPar ))

/** Creates a new object of a class using class table pointer.
 * Creates a new object of a class with the given class table pointer and construction parameters.
 * @param	class_ptr	The pointer to the class description table.
 * @param	par_ptr		Pointer to the construction parameters. This pointer is passed to the constructor without any check.
 * @return	The newly created Object.
 * @note	ooc_new may throw an Exception!
 * @see		ooc_new_classptr() for more convenient use.
 */

Object		ooc_new_classptr( const Class class_ptr, const void * par_ptr );

/** Deletes an Object.
 * Deletes the Object using its destructor, then deallocating the memory.
 * @param	object	The Object to be deleted. Can be NULL.
 */
 
void		ooc_delete( Object object );

/** Deletes an Object via a pointer with nulling.
 * Deletes the Object using its destructor, then deallocating the memory.
 * Thread safely and reentrat safely NULLs the pointer.
 * @param	object_ptr	Pointer to the Object to be deleted. Can be NULL and the object pointer can be NULL as well.
 * @note	Use this version in class destructors! This is important to ensure the reentrancy for destructors.
 */

void		ooc_delete_and_null( Object * object_ptr );

#define OOC_IMPLEMENT_PTR_READ_AND_NULL						\
	void * tmp = * ptr_ptr;									\
	* ptr_ptr = NULL;										\
	return tmp;


/** Run-time safe upcast of an Object.
 * Safely upcasts the Object to the specified type. Throws an Exception if not possible. This is a macro.
 * @param	pObj	The Object to be cast.
 * @param	pClass	The desired type (Class name).
 * @return	The Objcet as a new type.
 * @see		ooc_check_cast()
 */
void
ooc_check_cast( void * _self, const Class target );

#define     ooc_cast( pObj, pClass )  ( ooc_check_cast( pObj, & pClass ## Class ), (pClass) pObj )

/** Chain the parent constructor.
 * This macro calls the constructor of the parent class.
 * Must be used soley in the constructor of the class, at the beginning of the constructor.
 * Use:
 * @code
 * static void MyClass_constructor( MyClass self, void * params )
 * {
 * 		assert( ooc_isInitialized( MyClass ) );
 * 
 * 		chain_constructor( MyClass, self, NULL );
 * 
 * 		.... other construction code here
 * }
 * @endcode
 * @param	pClass	The name of the actual class (not the parent!)
 * @param	pSelf	Pointer to the @c Object being constructed.
 * @param	pParam	Pointer to the construction parameters for the parent class's constructor.
 * 					This pointer is passed to the constructor without any check.
 * @note In some rare cases you may neglect calling the parent constructor, e.g. when there is no parent class,
 * or if you definitly know, that the parent class does not need construction, like in case of @c ListNode.
 * @hideinitializer
 */

#define chain_constructor( pClass, pSelf, pParam ) \
	if( pClass ## Class.parent != &BaseClass ) pClass ## Class.parent->ctor( (Object) pSelf, pParam )


/** Checks if Class is initialized.
 * This is a convenient macro.
 * @param	pClass	The Class that should be checked.
 * @return	@c TRUE or @c FALSE. Does not throw an Exception.
 * @see		_ooc_isInitialized()
 */
 
#define     ooc_isInitialized( pClass ) _ooc_isInitialized( & pClass ## Class )

/** Checks if Class is initialized via a class table pointer.
 * This is a convenient macro.
 * @param	class_ptr	The class table pointer to the Class that should be checked.
 * @return	@c TRUE or @c FALSE.
 * @see		ooc_isInitialized() for more convenient use. 
 */
 
int			_ooc_isInitialized( const Class class_ptr );
#endif