/*
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
 */

#include "marshal.h"
#include <string.h>


#define dModU8  0xabcdf001
#define dModU16 0xabcdf002
#define dModU32 0xabcdf003
#define dModU64 0xabcdf004
#define dModI8  0xabcdf005
#define dModI16 0xabcdf006
#define dModI32 0xabcdf007
#define dModI64 0xabcdf008
#define dModF32 0xabcdf009
#define dModF64 0xabcdf010


typedef union
{
  tUint8   m_u8;
  tUint16  m_u16;
  tUint32  m_u32;
  tUint64  m_u64;
  tInt8    m_i8;
  tInt16   m_i16;
  tInt32   m_i32;
  tInt64   m_i64;
  tFloat32 m_f32;
  tFloat64 m_f64;
} cUnion;


typedef struct
{
  tUint8 m_pad1;
  tUint32 m_mod;
  tUint8 m_pad2;
  cUnion m_union;
  tUint8 m_pad3;
} cTest;


cMarshalType UnionElements[] =
{
  dUnionElement( dModU8 , Marshal_Uint8Type ),
  dUnionElement( dModU16, Marshal_Uint16Type ),
  dUnionElement( dModU32, Marshal_Uint32Type ),
  dUnionElement( dModU64, Marshal_Uint64Type ),
  dUnionElement( dModI8 , Marshal_Int8Type ),
  dUnionElement( dModI16, Marshal_Int16Type ),
  dUnionElement( dModI32, Marshal_Int32Type ),
  dUnionElement( dModI64, Marshal_Int64Type ),
  dUnionElement( dModF32, Marshal_Float32Type ),
  dUnionElement( dModF64, Marshal_Float64Type ),
  dUnionElementEnd()
};


cMarshalType TestUnionType = dUnion( 1, cUnion, UnionElements );


cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_mod  , Marshal_Uint32Type ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_union, TestUnionType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};


cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1 = 47,
    .m_mod  = dModU64,
    .m_pad2 = 48,
    .m_union.m_u64 = 0xabcd123456780123LL,
    .m_pad3 = 49
  };

  unsigned char  buffer[256];
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( value.m_mod != result.m_mod )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  if ( value.m_union.m_u64 != result.m_union.m_u64 )
       return 1;

  return 0;
}
