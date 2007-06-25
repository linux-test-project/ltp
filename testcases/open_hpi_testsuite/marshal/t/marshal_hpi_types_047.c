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
cmp_eventloginfo( SaHpiEventLogInfoT *d1, SaHpiEventLogInfoT *d2 )
{
  if ( d1->Entries != d2->Entries )
       return 0;

  if ( d1->Size != d2->Size )
       return 0;

  if ( d1->UserEventMaxSize != d2->UserEventMaxSize )
       return 0;

  if ( d1->UpdateTimestamp != d2->UpdateTimestamp )
       return 0;

  if ( d1->CurrentTime != d2->CurrentTime )
       return 0;

  if ( d1->Enabled != d2->Enabled )
       return 0;

  if ( d1->OverflowFlag != d2->OverflowFlag )
       return 0;

  if ( d1->OverflowResetable != d2->OverflowResetable )
       return 0;

  if ( d1->OverflowAction != d2->OverflowAction )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiEventLogInfoT m_v1;
  tUint8 m_pad2;
  SaHpiEventLogInfoT m_v2;
  SaHpiEventLogInfoT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiEventLogInfoType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiEventLogInfoType ),
  dStructElement( cTest, m_v3   , SaHpiEventLogInfoType ),
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
    .m_v1.Entries                                = 1,
    .m_v1.Size                                   = 100,
    .m_v1.UserEventMaxSize                       = 100,
    .m_v1.UpdateTimestamp                        = 2000,
    .m_v1.CurrentTime                            = 2010,
    .m_v1.Enabled                                = TRUE,
    .m_v1.OverflowFlag                           = FALSE,
    .m_v1.OverflowResetable                      = TRUE,
    .m_v1.OverflowAction                         = SAHPI_EL_OVERFLOW_DROP,
    .m_pad2                                      = 48,
    .m_v2.Entries                                = 100,
    .m_v2.Size                                   = 110,
    .m_v2.UserEventMaxSize                       = 100,
    .m_v2.UpdateTimestamp                        = 4000,
    .m_v2.CurrentTime                            = 4010,
    .m_v2.Enabled                                = FALSE,
    .m_v2.OverflowFlag                           = TRUE,
    .m_v2.OverflowResetable                      = FALSE,
    .m_v2.OverflowAction                         = SAHPI_EL_OVERFLOW_OVERWRITE,
    .m_v3.Entries                                = 1000,
    .m_v3.Size                                   = 10,
    .m_v3.UserEventMaxSize                       = 20,
    .m_v3.UpdateTimestamp                        = 3000,
    .m_v3.CurrentTime                            = 3010,
    .m_v3.Enabled                                = TRUE,
    .m_v3.OverflowFlag                           = TRUE,
    .m_v3.OverflowResetable                      = TRUE,
    .m_v3.OverflowAction                         = SAHPI_EL_OVERFLOW_DROP,
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

  if ( !cmp_eventloginfo( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_eventloginfo( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_eventloginfo( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
