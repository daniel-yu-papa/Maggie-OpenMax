#include "ooc.h"
#include <string.h>
#include <stdlib.h>
#include "Mag_base.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-OOC"


/**	Base Class.
 * Used for root Class in ooc. It must be a superclass for all classes.
 */

ROM_ALLOC struct ClassTable BaseClass;


/* Class initialization
 */

static
void
invalid_virtual_function( void )
{
    MAG_ASSERT(1);
}


/*	Calculating the offset of the first virtual function in the Vtable.
	Must be done this tricky, because the following assumption is not necessarily true:
	sizeof(struct BaseVtable) == offsetof( _dummyVtable, first_virtual_function)
	This assumption may fail, depending on the compiler and the system ! */

struct _dummyVtable {
	struct	BaseVtable_stru	Base;
	void	(* first_virtual_function)( void );
};
#define virtual_function_alignment (offsetof( struct _dummyVtable, first_virtual_function))

static
void
invalidate_vtable( const Class type )
{
	size_t i;
	void (**fnp)(void);

	if( type->vtab_size > virtual_function_alignment ) {
    	i = ( type->vtab_size - virtual_function_alignment ) / sizeof( void(*)() );
    	fnp = (void (**)(void)) ( ((char*) type->c.vtable) + virtual_function_alignment);

    	for( ; i ; i--,  fnp++ )
    		*fnp = invalid_virtual_function;
    }
}

static
void
inherit_vtable_from_parent( const Class self )
{
	if( ooc_class_has_parent(self) ) {

		MAG_ASSERT( self->vtab_size >= self->parent->vtab_size );
		
		/* Inherit the overridden operators */
		self->c.vtable->_destroy_check = self->parent->c.vtable->_destroy_check;

		/* Inherit the virtual functions */
		if( self->parent->vtab_size > virtual_function_alignment )
		    memcpy( ((char *) self->c.vtable)+ virtual_function_alignment,			/* destination */
			        ((char *) self->parent->c.vtable) + virtual_function_alignment,	/* source */
			        self->parent->vtab_size - virtual_function_alignment );		    /* bytes to copy */
		}
}


void
_ooc_init_class( const Class self )
{
	if( self->c.vtable->_class == NULL ) {

		if( ooc_class_has_parent( self ) )
			_ooc_init_class( self->parent );

		self->c.vtable->_class = self;
		self->c.vtable->_destroy_check = NULL;

		invalidate_vtable( self );

		inherit_vtable_from_parent( self );

		self->init( self );
	}
}

int
_ooc_isInitialized( const Class type )
{
	return ( type == & BaseClass || type->c.vtable->_class == type ) ? TRUE : FALSE;
}

void *
ooc_calloc( size_t num, size_t size )
{
	void * allocated;

	allocated = calloc( num, size );

	return allocated;
}

static __inline__
void
ooc_build_object( Object object, const Class type, const void * params )
{
	/* Building the Object header */
	object->_vtab = type->c.vtable;		/* Vtable pointer */
	
	MAG_ASSERT( sizeof( struct BaseObject ) == sizeof( struct BaseVtable * ) );
	/* If struct BaseObject has been changed, additional initialization might be missing here! */

	/* Constructs the object instance, that may fail */
	type->ctor( object, params );	
}


Object
ooc_new_classptr( const Class type, const void * params )
{
	Object object;

	/* The class type must be initialized already! */
	MAG_ASSERT( _ooc_isInitialized( type ) );

	/* Allocates a memory block for the object instance, and initializes it with zeros */
	object = ooc_calloc(  1, type->size );

    if (NULL != object)
	{
    	ooc_build_object( object, type, params );

    	return object;
	}else{
        return NULL;
    }
}

/************************************
* Delete the Object
************************************/
/*  Helper: pointer read-out while nulling
 */

void *
ooc_ptr_read_and_null( void ** ptr_ptr )
{
    OOC_IMPLEMENT_PTR_READ_AND_NULL
}

void
ooc_free( void * mem )
{
	if( mem )
		free( mem );
}

static
void
ooc_destroy_object( Object self )
{
	Class type, next;
	Vtable vtab;

	MAG_ASSERT( self != NULL );
	
	/* Makes the Object invalid */
	vtab = ooc_ptr_read_and_null( (void**) &self->_vtab );
	
	if( vtab )
	{
		next = vtab->_class;
		
		do {
			type = next;

			type->dtor( self, vtab );
	
			next = type->parent;
		} while( ooc_class_has_parent( type ) );
	}
}

void
ooc_delete( Object self )
{
	if( self && self->_vtab )
		if( self->_vtab->_destroy_check == NULL || (self->_vtab->_destroy_check)(self) == TRUE )
		{
			ooc_destroy_object( self );
			ooc_free( self );
		}
}

void
ooc_delete_and_null( Object * obj_ptr )
{
	MAG_ASSERT( obj_ptr );

	/* Thread safe release of the object */
	ooc_delete( (Object) ooc_ptr_read_and_null( (void**) obj_ptr ) );
}

/************************************
* Safty Check
************************************/
static
int
ooc_isClassChildOf( const Class checkable, const Class base )
{
    Class actual = checkable;
    
    do {
        if( actual->parent == base )
            return TRUE;
        actual = actual->parent;
        } while( actual );

    return FALSE;
}


int
_ooc_isInstanceOf( const Object self, const Class base )
{
	MAG_ASSERT( _ooc_isInitialized( base ) );

	if( ! self )
		return FALSE;

	if( ! self->_vtab )
		return FALSE;

	if( ! self->_vtab->_class )
		return FALSE;

	if( self->_vtab->_class == base )
		return TRUE;

	if( self->_vtab->_class->c.vtable != self->_vtab )
		return FALSE;

	return ooc_isClassChildOf( self->_vtab->_class, base );
}

void
ooc_check_cast( void * _self, const Class target )
{
	if( ! _ooc_isInstanceOf( _self, target ) )
		MAG_ASSERT(1);
		
	return;
}


