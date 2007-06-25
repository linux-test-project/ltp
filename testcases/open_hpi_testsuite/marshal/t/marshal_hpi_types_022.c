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
cmp_idrareaheader( SaHpiIdrAreaHeaderT *d1, SaHpiIdrAreaHeaderT *d2 )
{
  if ( d1->AreaId != d2->AreaId )
       return 0;

  if ( d1->Type != d2->Type )
       return 0;

  if ( d1->ReadOnly != d2->ReadOnly )
       return 0;

  if ( d1->NumFields != d2->NumFields )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiIdrAreaHeaderT m_v1;
  tUint8 m_pad2;
  SaHpiIdrAreaHeaderT m_v2;
  SaHpiIdrAreaHeaderT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiIdrAreaHeaderType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiIdrAreaHeaderType ),
  dStructElement( cTest, m_v3   , SaHpiIdrAreaHeaderType ),
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
    .m_v1.Type             = SAHPI_IDR_FIELDTYPE_CUSTOM,
    .m_v1.ReadOnly         = FALSE,
    .m_v1.NumFields        = 1,
    .m_pad2                = 48,
    .m_v2.AreaId           = 2,
    .m_v2.Type             = SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    .m_v2.ReadOnly         = FALSE,
    .m_v2.NumFields        = 3,
    .m_v3.AreaId           = 3,
    .m_v3.Type             = SAHPI_IDR_FIELDTYPE_CHASSIS_TYPE,
    .m_v3.ReadOnly         = TRUE,
    .m_v3.NumFields        = 2,
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

  if ( !cmp_idrareaheader( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_idrareaheader( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_idrareaheader( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
