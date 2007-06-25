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

#include "marshal_hpi_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static int
cmp_text_buffer( SaHpiTextBufferT *d1, SaHpiTextBufferT *d2 )
{
  if ( d1->DataType != d2->DataType )
       return 0;

  if ( d1->Language != d2->Language )
       return 0;

  if ( d1->DataLength != d2->DataLength )
       return 0;

  return memcmp( d1->Data, d2->Data, d1->DataLength ) ? 0 : 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiTextBufferT m_tb1;
  tUint8 m_pad2;
  SaHpiTextBufferT m_tb2;
  SaHpiTextBufferT m_tb3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_tb1  , SaHpiTextBufferType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_tb2  , SaHpiTextBufferType ),
  dStructElement( cTest, m_tb3  , SaHpiTextBufferType ),
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
    .m_tb1.DataType   = SAHPI_TL_TYPE_BINARY,
    .m_tb1.Language   = SAHPI_LANG_TSONGA,
    .m_tb1.DataLength = 3,
    .m_tb1.Data       = "AB",
    .m_pad2 = 48,
    .m_tb2.DataType   = SAHPI_TL_TYPE_BCDPLUS,
    .m_tb2.Language   = SAHPI_LANG_SANGRO,
    .m_tb2.DataLength = 21,
    .m_tb2.Data       = "12345678901234567890",
    .m_tb3.DataType   = SAHPI_TL_TYPE_ASCII6,
    .m_tb3.Language   = SAHPI_LANG_TAJIK,
    .m_tb3.DataLength = 0,
    .m_tb3.Data = "",
    .m_pad3 = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_text_buffer( &value.m_tb1, &result.m_tb1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_text_buffer( &value.m_tb2, &result.m_tb2 ) )
       return 1;

  if ( !cmp_text_buffer( &value.m_tb3, &result.m_tb3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
