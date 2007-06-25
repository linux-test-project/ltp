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
cmp_drtentry( SaHpiDrtEntryT *d1, SaHpiDrtEntryT *d2 )
{
  if ( d1->EntryId != d2->EntryId )
       return 0;

  if ( d1->DomainId != d2->DomainId )
       return 0;

  if ( d1->IsPeer != d2->IsPeer )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiDrtEntryT m_v1;
  tUint8 m_pad2;
  SaHpiDrtEntryT m_v2;
  SaHpiDrtEntryT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiDrtEntryType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiDrtEntryType ),
  dStructElement( cTest, m_v3   , SaHpiDrtEntryType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                      = 47,
    .m_v1.EntryId                                = 1,
    .m_v1.DomainId                               = 1,
    .m_v1.IsPeer                                 = TRUE,
    .m_pad2                                      = 48,
    .m_v2.EntryId                                = 2,
    .m_v2.DomainId                               = 2,
    .m_v2.IsPeer                                 = TRUE,
    .m_v3.EntryId                                = 3,
    .m_v3.DomainId                               = 3,
    .m_v3.IsPeer                                 = FALSE,
    .m_pad3                                      = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_drtentry( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_drtentry( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_drtentry( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
