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
cmp_entities( SaHpiEntityT *d1, SaHpiEntityT *d2 )
{
  if ( d1->EntityType != d2->EntityType )
       return 0;

  if ( d1->EntityLocation != d2->EntityLocation )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiEntityPathT m_v1;
  tUint8 m_pad2;
  SaHpiEntityPathT m_v2;
  SaHpiEntityPathT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiEntityPathType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiEntityPathType ),
  dStructElement( cTest, m_v3   , SaHpiEntityPathType ),
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
    .m_v1.Entry[0].EntityType     = SAHPI_ENT_SYSTEM_BOARD,
    .m_v1.Entry[0].EntityLocation = 1,
    .m_v1.Entry[1].EntityType     = SAHPI_ENT_BATTERY,
    .m_v1.Entry[1].EntityLocation = 2,
    .m_pad2                       = 48,
    .m_v2.Entry[0].EntityType     = SAHPI_ENT_POWER_MODULE,
    .m_v2.Entry[0].EntityLocation = 2,
    .m_v2.Entry[1].EntityType     = SAHPI_ENT_SYSTEM_BUS,
    .m_v2.Entry[1].EntityLocation = 3,
    .m_v3.Entry[0].EntityType     = SAHPI_ENT_SWITCH,
    .m_v3.Entry[0].EntityLocation = 3,
    .m_v3.Entry[1].EntityType     = SAHPI_ENT_SYSTEM_BLADE,
    .m_v3.Entry[1].EntityLocation = 4,
    .m_v3.Entry[2].EntityType     = SAHPI_ENT_RACK_MOUNTED_SERVER,
    .m_v3.Entry[2].EntityLocation = 5,
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

  if ( !cmp_entities( &value.m_v1.Entry[0], &result.m_v1.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v1.Entry[1], &result.m_v1.Entry[1] ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_entities( &value.m_v2.Entry[0], &result.m_v2.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v2.Entry[1], &result.m_v2.Entry[1] ) )
       return 1;

  if ( !cmp_entities( &value.m_v3.Entry[0], &result.m_v3.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v3.Entry[1], &result.m_v3.Entry[1] ) )
       return 1;
  if ( !cmp_entities( &value.m_v3.Entry[2], &result.m_v3.Entry[2] ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
