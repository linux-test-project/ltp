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
} cTest2;

cMarshalType Test2Elements[] =
{
  dStructElement( cTest2, m_u8 , Marshal_Uint8Type  ),
  dStructElement( cTest2, m_u16, Marshal_Uint16Type ),
  dStructElementEnd()
};

cMarshalType Test2Type = dStruct( cTest2, Test2Elements );


typedef struct
{
  tUint16  m_u16;
  tUint8   m_u8;
} cTest3;

cMarshalType Test3Elements[] =
{
  dStructElement( cTest3, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest3, m_u8 , Marshal_Uint8Type  ),
  dStructElementEnd()
};

cMarshalType Test3Type = dStruct( cTest3, Test3Elements );


typedef struct
{
  tUint8   m_u81;
  tUint16  m_u16;
  cTest2   m_struct2;
  tUint8   m_u82;
  cTest3   m_struct3;
  tUint8   m_u83;
} cTest1;


cMarshalType Test1Elements[] =
{
  dStructElement( cTest1, m_u81, Marshal_Uint8Type  ),
  dStructElement( cTest1, m_u16, Marshal_Uint16Type ),
  dStructElement( cTest1, m_struct2, Test2Type ),
  dStructElement( cTest1, m_u82, Marshal_Uint8Type  ),
  dStructElement( cTest1, m_struct3, Test3Type ),
  dStructElement( cTest1, m_u83, Marshal_Uint8Type  ),
  dStructElementEnd()
};


cMarshalType Test1Type = dStruct( cTest1, Test1Elements );


int
main( int argc, char *argv[] )
{
  cTest1 value =
  {
    .m_u81           = 0x42,
    .m_u16           = 0x1234,
    .m_struct2.m_u8  = 0x17,
    .m_struct2.m_u16 = 0x6789,
    .m_u82           = 0x43,
    .m_struct3.m_u16 = 0x3456,
    .m_struct3.m_u8  = 0x18,
    .m_u83           = 0x44
  };

  unsigned char  buffer[256];
  cTest1         result;

  unsigned int s1 = Marshal( &Test1Type, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &Test1Type, &result, buffer );

  if ( s1 != s2 )
       return 1;

  unsigned int s3 = MarshalSize( &Test1Type );

  if ( s1 != s3 )
       return 1;

  if ( value.m_u81 != result.m_u81 )
       return 1;

  if ( value.m_u16 != result.m_u16 )
       return 1;

  if ( value.m_struct2.m_u8 != result.m_struct2.m_u8 )
       return 1;

  if ( value.m_struct2.m_u16 != result.m_struct2.m_u16 )
       return 1;

  if ( value.m_u82 != result.m_u82 )
       return 1;

  if ( value.m_struct3.m_u16 != result.m_struct3.m_u16 )
       return 1;

  if ( value.m_struct3.m_u8 != result.m_struct3.m_u8 )
       return 1;

  if ( value.m_u83 != result.m_u83 )
       return 1;

  return 0;
}
