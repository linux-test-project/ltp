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
cmp_ctrlstatestream( SaHpiCtrlStateStreamT *d1, SaHpiCtrlStateStreamT *d2 )
{
  if ( d1->Repeat != d2->Repeat )
       return 0;

  if ( d1->StreamLength != d2->StreamLength )
       return 0;

  if ( memcmp(d1->Stream, d2->Stream, SAHPI_CTRL_MAX_STREAM_LENGTH) != 0 )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiCtrlStateStreamT m_v1;
  tUint8 m_pad2;
  SaHpiCtrlStateStreamT m_v2;
  SaHpiCtrlStateStreamT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiCtrlStateStreamType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiCtrlStateStreamType ),
  dStructElement( cTest, m_v3   , SaHpiCtrlStateStreamType ),
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
    .m_v1.Repeat         = 0,
    .m_v1.StreamLength   = 4,
    .m_v1.Stream         = {'A', 'B', 'c', 'd'},
    .m_pad2              = 48,
    .m_v2.Repeat         = 1,
    .m_v2.StreamLength   = 4,
    .m_v2.Stream         = {'d', 'c', 'B', 'A'},
    .m_v3.Repeat         = 1,
    .m_v3.StreamLength   = 1,
    .m_v3.Stream         = {'A'},
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

  if ( !cmp_ctrlstatestream( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_ctrlstatestream( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_ctrlstatestream( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
