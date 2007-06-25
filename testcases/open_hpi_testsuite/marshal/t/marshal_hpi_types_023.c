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
cmp_idrinfo( SaHpiIdrInfoT *d1, SaHpiIdrInfoT *d2 )
{
  if ( d1->IdrId != d2->IdrId )
       return 0;

  if ( d1->UpdateCount != d2->UpdateCount )
       return 0;

  if ( d1->ReadOnly != d2->ReadOnly )
       return 0;

  if ( d1->NumAreas != d2->NumAreas )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiIdrInfoT m_v1;
  tUint8 m_pad2;
  SaHpiIdrInfoT m_v2;
  SaHpiIdrInfoT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiIdrInfoType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiIdrInfoType ),
  dStructElement( cTest, m_v3   , SaHpiIdrInfoType ),
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
    .m_v1.IdrId            = 1,
    .m_v1.UpdateCount      = 1,
    .m_v1.ReadOnly         = FALSE,
    .m_v1.NumAreas         = 1,
    .m_pad2                = 48,
    .m_v2.IdrId            = 2,
    .m_v2.UpdateCount      = 23,
    .m_v2.ReadOnly         = FALSE,
    .m_v2.NumAreas         = 3,
    .m_v3.IdrId            = 3,
    .m_v3.UpdateCount      = 10,
    .m_v3.ReadOnly         = TRUE,
    .m_v3.NumAreas         = 2,
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

  if ( !cmp_idrinfo( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_idrinfo( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_idrinfo( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
