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
cmp_idrfield( SaHpiIdrFieldT *d1, SaHpiIdrFieldT *d2 )
{
  if ( d1->AreaId != d2->AreaId )
       return 0;

  if ( d1->FieldId != d2->FieldId )
       return 0;

  if ( d1->Type != d2->Type )
       return 0;

  if ( d1->ReadOnly != d2->ReadOnly )
       return 0;

  if ( !cmp_text_buffer( &d1->Field, &d2->Field ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiIdrFieldT m_v1;
  tUint8 m_pad2;
  SaHpiIdrFieldT m_v2;
  SaHpiIdrFieldT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiIdrFieldType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiIdrFieldType ),
  dStructElement( cTest, m_v3   , SaHpiIdrFieldType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                = 47,
    .m_v1.AreaId           = 1,
    .m_v1.FieldId          = 1,
    .m_v1.Type             = SAHPI_IDR_FIELDTYPE_CUSTOM,
    .m_v1.ReadOnly         = FALSE,
    .m_v1.Field.DataType   = SAHPI_TL_TYPE_BINARY,
    .m_v1.Field.Language   = SAHPI_LANG_TSONGA,
    .m_v1.Field.DataLength = 3,
    .m_v1.Field.Data       = "AB",
    .m_pad2                = 48,
    .m_v2.AreaId           = 2,
    .m_v2.FieldId          = 2,
    .m_v2.Type             = SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    .m_v2.ReadOnly         = FALSE,
    .m_v2.Field.DataType   = SAHPI_TL_TYPE_BCDPLUS,
    .m_v2.Field.Language   = SAHPI_LANG_SANGRO,
    .m_v2.Field.DataLength = 21,
    .m_v2.Field.Data       = "12345678901234567890",
    .m_v3.AreaId           = 3,
    .m_v3.FieldId          = 3,
    .m_v3.Type             = SAHPI_IDR_FIELDTYPE_CHASSIS_TYPE,
    .m_v3.ReadOnly         = TRUE,
    .m_v3.Field.DataType   = SAHPI_TL_TYPE_ASCII6,
    .m_v3.Field.Language   = SAHPI_LANG_TAJIK,
    .m_v3.Field.DataLength = 0,
    .m_v3.Field.Data       = "",
    .m_pad3                = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_idrfield( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_idrfield( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_idrfield( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
