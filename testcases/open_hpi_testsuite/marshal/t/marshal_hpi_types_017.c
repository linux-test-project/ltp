/*
 * Copyright (c) 2005 by IBM Corporation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     W. David Ashley <dashley@us.ibm.com>
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


static int
cmp_ctrlstatetext( SaHpiCtrlStateTextT *d1, SaHpiCtrlStateTextT *d2 )
{
  if ( d1->Line != d2->Line )
       return 0;

  if ( !cmp_text_buffer( &d1->Text, &d2->Text ) )
       return 0;

  return 1;
}


static int
cmp_ctrlrectext( SaHpiCtrlRecTextT *d1, SaHpiCtrlRecTextT *d2 )
{
  if ( d1->MaxChars != d2->MaxChars )
       return 0;

  if ( d1->MaxLines != d2->MaxLines )
       return 0;

  if ( d1->Language != d2->Language )
       return 0;

  if ( d1->DataType != d2->DataType )
       return 0;

  if ( !cmp_ctrlstatetext( &d1->Default, &d2->Default ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiCtrlRecTextT m_v1;
  tUint8 m_pad2;
  SaHpiCtrlRecTextT m_v2;
  SaHpiCtrlRecTextT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiCtrlRecTextType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiCtrlRecTextType ),
  dStructElement( cTest, m_v3   , SaHpiCtrlRecTextType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                       = 47,
    .m_v1.MaxChars                = 80,
    .m_v1.MaxLines                = 10,
    .m_v1.Language                = SAHPI_LANG_ENGLISH,
    .m_v1.DataType                = SAHPI_TL_TYPE_TEXT,
    .m_v1.Default.Line            = 1,
    .m_v1.Default.Text.DataType   = SAHPI_TL_TYPE_TEXT,
    .m_v1.Default.Text.Language   = SAHPI_LANG_ENGLISH,
    .m_v1.Default.Text.DataLength = 7,
    .m_v1.Default.Text.Data       = "My text",
    .m_pad2                       = 48,
    .m_v2.MaxChars                = 80,
    .m_v2.MaxLines                = 10,
    .m_v2.Language                = SAHPI_LANG_ENGLISH,
    .m_v2.DataType                = SAHPI_TL_TYPE_TEXT,
    .m_v2.Default.Line            = 1,
    .m_v2.Default.Text.DataType   = SAHPI_TL_TYPE_TEXT,
    .m_v2.Default.Text.Language   = SAHPI_LANG_ENGLISH,
    .m_v2.Default.Text.DataLength = 8,
    .m_v2.Default.Text.Data       = "My text1",
    .m_v3.MaxChars                = 80,
    .m_v3.MaxLines                = 10,
    .m_v3.Language                = SAHPI_LANG_ENGLISH,
    .m_v3.DataType                = SAHPI_TL_TYPE_TEXT,
    .m_v3.Default.Line            = 1,
    .m_v3.Default.Text.DataType   = SAHPI_TL_TYPE_TEXT,
    .m_v3.Default.Text.Language   = SAHPI_LANG_ENGLISH,
    .m_v3.Default.Text.DataLength = 15,
    .m_v2.Default.Text.Data       = "My text My text",
    .m_pad3                       = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_ctrlrectext( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_ctrlrectext( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_ctrlrectext( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
