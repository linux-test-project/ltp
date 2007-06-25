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


#define dArraySize 12


typedef struct
{
  tUint8   m_u8;
  tUint16  m_u16;
  tUint32  m_u32;
  tInt8    m_i8;
  tInt16   m_i16;
  tInt32   m_i32;
  tFloat32 m_f32;
  tFloat64 m_f64;
  tInt8    m_array[dArraySize];
} cTest1;

cMarshalType ArrayType = dArray( Marshal_Int8Type, dArraySize );

cMarshalType Test1Elements[] =
{
  dStructElement( cTest1, m_u8 , Marshal_Uint8Type  ),
  dStructElement( cTest1, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest1, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest1, m_u32, Marshal_Uint32Type ),
  dStructElement( cTest1, m_i8,  Marshal_Int8Type ),
  dStructElement( cTest1, m_i16, Marshal_Int16Type ),
  dStructElement( cTest1, m_i32, Marshal_Int32Type ),
  dStructElement( cTest1, m_f32, Marshal_Float32Type ),
  dStructElement( cTest1, m_f64, Marshal_Float64Type ),
  dStructElement( cTest1, m_array, ArrayType ),

  dStructElementEnd()
};

cMarshalType Test1Type = dStruct( cTest1, Test1Elements );


typedef struct
{
  tUint16  m_u16;
  tUint8   m_u8;
  tUint64  m_u64;
  tInt64   m_i64;
} cTest2;

cMarshalType Test2Elements[] =
{
  dStructElement( cTest2, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest2, m_u8 , Marshal_Uint8Type  ),
  dStructElement( cTest2, m_u64, Marshal_Uint64Type ),
  dStructElement( cTest2, m_i64, Marshal_Int64Type  ),
  dStructElementEnd()
};

cMarshalType Test2Type = dStruct( cTest2, Test2Elements );


int
main( int argc, char *argv[] )
{
  cTest1 value1 =
  {
    .m_u8            = 0x42,
    .m_u16           = 0x1234,
    .m_u32           = 0x12345678,
    .m_i8            = -48,
    .m_i16           = -12345,
    .m_i32           = -12345667,
    .m_f32           = 0.123456,
    .m_f64           = -12345.345566,
    .m_array         = "hui jui"
  };

  cTest2 value2 =
  {
    .m_u16 = 0x4434,
    .m_u8  = 0x47,
    .m_u64 = 0x1234567812345678LL,
    .m_i64 = 0x8765432187654321LL,
  };

  unsigned char  buffer[1024];
  cTest1         result1;
  cTest2         result2;

  const cMarshalType *type_array[] =
  {
    &Test1Type,
    &Test2Type,
    0
  };

  const void *value_array[] =
  {
    &value1,
    &value2
  };

  void *result_array[] =
  {
    &result1,
    &result2
  };

  unsigned int s1 = MarshalArray( type_array, value_array, buffer );
  unsigned int s2 = DemarshalArray( MarshalByteOrder(), type_array, result_array, buffer );

  if ( s1 != s2 )
       return 1;

  unsigned int s3 = MarshalSizeArray( type_array );

  if ( s1 != s3 )
       return 1;

  if ( value1.m_u8 != result1.m_u8 )
       return 1;

  if ( value1.m_u16 != result1.m_u16 )
       return 1;

  if ( value1.m_u32 != result1.m_u32 )
       return 1;

  if ( value1.m_i8 != result1.m_i8 )
       return 1;

  if ( value1.m_i16 != result1.m_i16 )
       return 1;

  if ( value1.m_i32 != result1.m_i32 )
       return 1;

  if ( value1.m_f32 != result1.m_f32 )
       return 1;

  if ( value1.m_f64 != result1.m_f64 )
       return 1;

  if ( strcmp( value1.m_array, result1.m_array ) )
       return 1;

  if ( value2.m_u8 != result2.m_u8 )
       return 1;

  if ( value2.m_u16 != result2.m_u16 )
       return 1;

  if ( value2.m_u64 != result2.m_u64 )
       return 1;

  if ( value2.m_i64 != result2.m_i64 )
       return 1;

  return 0;
}
