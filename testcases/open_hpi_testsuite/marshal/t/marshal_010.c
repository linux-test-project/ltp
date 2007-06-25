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


typedef struct
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
} cTest;


cMarshalType Elements[] =
{
  dStructElement( cTest, m_u8 , Marshal_Uint8Type  ),
  dStructElement( cTest, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest, m_u32, Marshal_Uint32Type ),
  dStructElement( cTest, m_u64, Marshal_Uint64Type ),
  dStructElement( cTest, m_i8 , Marshal_Int8Type ),
  dStructElement( cTest, m_i16, Marshal_Int16Type ),
  dStructElement( cTest, m_i32, Marshal_Int32Type ),
  dStructElement( cTest, m_i64, Marshal_Int64Type ),
  dStructElement( cTest, m_f32, Marshal_Float32Type ),
  dStructElement( cTest, m_f64, Marshal_Float64Type ),
  dStructElementEnd()
};


cMarshalType TestType = dStruct( cTest, Elements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_u8  = 42,
    .m_u16 = 0x1234,
    .m_u32 = 0x23456789,
    .m_u64 = 0x1234234534564567LL,
    .m_i8  = 43,
    .m_i16 = 0x7654,
    .m_i32 = 0x98765432,
    .m_i64 = 0x9876543210987654LL,
    .m_f32 = -12.5,
    .m_f64 = 34.5
  };

  unsigned char  buffer[256];
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_u8 != result.m_u8 )
       return 1;

  if ( value.m_u16 != result.m_u16 )
       return 1;

  if ( value.m_u32 != result.m_u32 )
       return 1;

  if ( value.m_u64 != result.m_u64 )
       return 1;

  if ( value.m_i8 != result.m_i8 )
       return 1;

  if ( value.m_i16 != result.m_i16 )
       return 1;

  if ( value.m_i32 != result.m_i32 )
       return 1;

  if ( value.m_i64 != result.m_i64 )
       return 1;

  if ( value.m_f32 != result.m_f32 )
       return 1;

  if ( value.m_f64 != result.m_f64 )
       return 1;

  return 0;
}
