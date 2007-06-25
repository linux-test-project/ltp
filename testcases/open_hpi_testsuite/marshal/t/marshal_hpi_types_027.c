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
cmp_name( SaHpiNameT *d1, SaHpiNameT *d2 )
{
  if ( d1->Length != d2->Length )
       return 0;

  if ( memcmp(&d1->Value, &d2->Value, SA_HPI_MAX_NAME_LENGTH) != 0 )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiNameT m_v1;
  tUint8 m_pad2;
  SaHpiNameT m_v2;
  SaHpiNameT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiNameType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiNameType ),
  dStructElement( cTest, m_v3   , SaHpiNameType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                  = 47,
    .m_v1.Length             = 7,
    .m_v1.Value              = "My text",
    .m_pad2                  = 48,
    .m_v2.Length             = 9,
    .m_v2.Value              = "Next text",
    .m_v3.Length             = 15,
    .m_v3.Value              = "My text my text",
    .m_pad3                  = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_name( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_name( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_name( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
