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
  tUint8 m_pad1;
  tUint8 m_size;
  tUint8 m_pad2;
  char   m_array[256];
  tUint8 m_pad3;
} cTest;


cMarshalType TestVarArrayType = dVarArray( Marshal_Int8Type, 1 );


cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_size , Marshal_Uint8Type ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_array, TestVarArrayType ),
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
    .m_size = 13,
    .m_pad2 = 48,
    .m_array = "123456789012",
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

  if ( value.m_size != result.m_size )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  if ( memcmp( value.m_array, result.m_array, value.m_size ) )
       return 1;

  return 0;
}
