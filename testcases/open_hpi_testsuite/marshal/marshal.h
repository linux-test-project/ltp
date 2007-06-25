/*
 * marshaling/demarshaling
 *
 * Copyright (c) 2004 by FORCE Computers.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 *     W. David Ashley <dashley@us.ibm.com.com>
 */

#ifndef dMarshal_h
#define dMarshal_h


#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char      tUint8;
typedef unsigned short     tUint16;
typedef unsigned int       tUint32;
typedef unsigned long long tUint64;
typedef char               tInt8;
typedef short              tInt16;
typedef int                tInt32;
typedef long long          tInt64;
typedef float              tFloat32;
typedef double             tFloat64;


typedef enum 
{
  eMtUnknown,
  eMtVoid,  // do nothing
  eMtUint8,
  eMtUint16,
  eMtUint32,
  eMtUint64,
  eMtInt8,
  eMtInt16,
  eMtInt32,
  eMtInt64,
  eMtFloat32,
  eMtFloat64,
  eMtArray,
  eMtVarArray, // only possible in structs
  eMtStruct,
  eMtStructElement,
  eMtUnion,
  eMtUnionElement,
  eMtUserDefined // user defined marshalling
} tMarshalType;


int IsSimpleType( tMarshalType type );


#define dStructOffset(type,field) ((unsigned int)(((char *) (&(((type *)0)->field)))-((char*)0)))


// helper macro for arrays
#define dArray( type, size )  \
{                             \
  .m_type = eMtArray,         \
  .m_u.m_array =              \
  {                           \
    .m_size = size,           \
    .m_type = &type           \
  }                           \
}

// helper macro for var arrays
#define dVarArray( type, size_pos )  \
{                             \
  .m_type = eMtVarArray,      \
  .m_u.m_var_array =          \
  {                           \
    .m_size = size_pos,       \
    .m_type = &type           \
  }                           \
}

// helper marco for struct
#define dStruct( type, elems ) \
{                             \
  .m_type = eMtStruct,        \
  .m_u.m_struct =             \
  {                           \
    .m_size = sizeof( type ), \
    .m_elements = elems       \
  }                           \
}

// helper marco for struct elements
#define dStructElement( struct_type, field, type ) \
{                                                  \
  .m_type = eMtStructElement,                      \
  .m_u.m_struct_element.m_offset = dStructOffset( struct_type, field ), \
  .m_u.m_struct_element.m_type   = &type           \
}

// helper marco for struct end
#define dStructElementEnd() \
{                            \
  .m_type = eMtUnknown       \
}

// helper marco for unions
#define dUnion( offset, type, elems ) \
{                              \
  .m_type = eMtUnion,          \
  .m_u.m_union =               \
  {                            \
    .m_offset = offset,        \
    .m_size    = sizeof( type ), \
    .m_elements = elems        \
  }                            \
}

// helper marco for union elements
#define dUnionElement( mod, type )    \
{                                     \
  .m_type = eMtUnionElement,          \
  .m_u.m_union_element.m_mod  = mod,  \
  .m_u.m_union_element.m_type = &type \
}

// helper marco for union end
#define dUnionElementEnd() \
{                          \
  .m_type = eMtUnknown     \
}

// helper macro for user define mashaller
#define dUserDefined( marshaller, demarshaller, user_data )    \
{                                     \
  .m_type = eMtUserDefined,           \
  .m_u.m_user_defined.m_marshal   = marshaller, \
  .m_u.m_user_defined.m_demarshal = demarshaller, \
  .m_u.m_user_defined.m_user_data = user_data \
}


struct sMarshalType;
typedef struct sMarshalType cMarshalType;

typedef int (*tMarshalFunction)( const cMarshalType *type, const void *data,
				 void *buffer, void *user_data );
typedef int (*tDemarshalFunction)( int byte_order, const cMarshalType *type, void *data,
				  const void *buffer, void *user_data );

struct sMarshalType
{
  tMarshalType m_type;

  union
  {
    struct
    {
      int            m_size;  // array size
      cMarshalType  *m_type;  // array element 
    } m_array;

    struct
    {
      unsigned int   m_size; // struct pos where size is
      cMarshalType  *m_type; // array element
    } m_var_array;

    struct
    {
      unsigned int   m_size; // size of structure
      cMarshalType  *m_elements; // struct elements
    } m_struct;

    struct
    {
      unsigned int   m_offset;
      cMarshalType  *m_type;
    } m_struct_element;

    struct
    {
      int            m_offset;    // mod offset in struct
      unsigned int   m_size;      // size of structure
      cMarshalType  *m_elements;  // union elements
    } m_union;

    struct
    {
      unsigned int   m_mod;
      cMarshalType  *m_type; // union element 
    } m_union_element;

    struct
    {
      tMarshalFunction   m_marshal;
      tDemarshalFunction m_demarshal;
      void              *m_user_data;
    } m_user_defined;
  } m_u;
};


// some simple types
extern cMarshalType Marshal_VoidType;
extern cMarshalType Marshal_Uint8Type;
extern cMarshalType Marshal_Uint16Type;
extern cMarshalType Marshal_Uint32Type;
extern cMarshalType Marshal_Uint64Type;
extern cMarshalType Marshal_Int8Type;
extern cMarshalType Marshal_Int16Type;
extern cMarshalType Marshal_Int32Type;
extern cMarshalType Marshal_Int64Type;
extern cMarshalType Marshal_Float32Type;
extern cMarshalType Marshal_Float64Type;


// marshal order used
int MarshalByteOrder( void );

// size in bytes
int MarshalSize( const cMarshalType *type );
int MarshalSizeArray( const cMarshalType **types );

// marshal data into buffer
int Marshal( const cMarshalType *type, const void *data,
		      void  *buffer );
int MarshalArray( const cMarshalType **types, const void **data,
		  void *buffer );
int MarshalSimpleTypes( tMarshalType type, const void *data,
			void  *buffer );

// demarshal buffer into data
int Demarshal( int byte_order, const cMarshalType *type,
	       void *data, const void *buffer );
int DemarshalArray( int byte_order, const cMarshalType **types,
		    void **data, const void *buffer );
int DemarshalSimpleTypes( int byte_order, tMarshalType type, 
			  void *data, const void *buffer );

#ifdef __cplusplus
}
#endif

#endif
