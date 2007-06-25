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
cmp_ctrlstateoem( SaHpiCtrlStateOemT *d1, SaHpiCtrlStateOemT *d2 )
{
  if ( d1->MId != d2->MId )
       return 0;

  if ( d1->BodyLength != d2->BodyLength )
       return 0;

  if ( memcmp(d1->Body, d2->Body, SAHPI_CTRL_MAX_OEM_BODY_LENGTH) != 0 )
       return 0;

  return 1;
}


static int
cmp_ctrlrecoem( SaHpiCtrlRecOemT *d1, SaHpiCtrlRecOemT *d2 )
{
  if ( d1->MId != d2->MId )
       return 0;

  if ( memcmp(d1->ConfigData, d2->ConfigData, SAHPI_CTRL_OEM_CONFIG_LENGTH) != 0 )
       return 0;

  if ( !cmp_ctrlstateoem( &d1->Default, &d2->Default ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiCtrlRecOemT m_v1;
  tUint8 m_pad2;
  SaHpiCtrlRecOemT m_v2;
  SaHpiCtrlRecOemT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiCtrlRecOemType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiCtrlRecOemType ),
  dStructElement( cTest, m_v3   , SaHpiCtrlRecOemType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                      = 47,
    .m_v1.MId                    = 0,
    .m_v1.ConfigData             = {'A', 'B', 'c', 'd'},
    .m_v1.Default.MId            = 0,
    .m_v1.Default.BodyLength     = 4,
    .m_v1.Default.Body           = {'A', 'B', 'c', 'd'},
    .m_pad2                      = 48,
    .m_v2.MId                    = 0,
    .m_v2.ConfigData             = {'d', 'c', 'B', 'A'},
    .m_v2.Default.MId            = 1,
    .m_v2.Default.BodyLength     = 4,
    .m_v2.Default.Body           = {'d', 'c', 'B', 'A'},
    .m_v3.MId                    = 0,
    .m_v3.ConfigData             = {'A'},
    .m_v3.Default.MId            = 1,
    .m_v3.Default.BodyLength     = 1,
    .m_v3.Default.Body           = {'A'},
    .m_pad3                      = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_ctrlrecoem( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_ctrlrecoem( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_ctrlrecoem( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
