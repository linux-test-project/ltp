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
cmp_watchdogevent( SaHpiWatchdogEventT *d1, SaHpiWatchdogEventT *d2 )
{
  if ( d1->WatchdogNum != d2->WatchdogNum )
       return 0;

  if ( d1->WatchdogAction != d2->WatchdogAction )
       return 0;

  if ( d1->WatchdogPreTimerAction != d2->WatchdogPreTimerAction )
       return 0;

  if ( d1->WatchdogUse != d2->WatchdogUse )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiWatchdogEventT m_v1;
  tUint8 m_pad2;
  SaHpiWatchdogEventT m_v2;
  SaHpiWatchdogEventT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiWatchdogEventType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiWatchdogEventType ),
  dStructElement( cTest, m_v3   , SaHpiWatchdogEventType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                            = 47,
    .m_v1.WatchdogNum                  = 1,
    .m_v1.WatchdogAction               = SAHPI_WA_RESET,
    .m_v1.WatchdogPreTimerAction       = SAHPI_WPI_SMI,
    .m_v1.WatchdogUse                  = SAHPI_WTU_NONE,
    .m_pad2                            = 48,
    .m_v2.WatchdogNum                  = 2,
    .m_v2.WatchdogAction               = SAHPI_WA_POWER_DOWN,
    .m_v2.WatchdogPreTimerAction       = SAHPI_WPI_NMI,
    .m_v2.WatchdogUse                  = SAHPI_WTU_BIOS_POST,
    .m_v3.WatchdogNum                  = 3,
    .m_v3.WatchdogAction               = SAHPI_WA_POWER_CYCLE,
    .m_v3.WatchdogPreTimerAction       = SAHPI_WPI_OEM,
    .m_v3.WatchdogUse                  = SAHPI_WTU_OS_LOAD,
    .m_pad3                            = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_watchdogevent( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_watchdogevent( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_watchdogevent( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
