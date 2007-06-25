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
cmp_watchdog( SaHpiWatchdogT *d1, SaHpiWatchdogT *d2 )
{
  if ( d1->Log != d2->Log )
       return 0;

  if ( d1->Running != d2->Running )
       return 0;

  if ( d1->TimerUse != d2->TimerUse )
       return 0;

  if ( d1->TimerAction != d2->TimerAction )
       return 0;

  if ( d1->PretimerInterrupt != d2->PretimerInterrupt )
       return 0;

  if ( d1->PreTimeoutInterval != d2->PreTimeoutInterval )
       return 0;

  if ( d1->TimerUseExpFlags != d2->TimerUseExpFlags )
       return 0;

  if ( d1->InitialCount != d2->InitialCount )
       return 0;

  if ( d1->PresentCount != d2->PresentCount )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiWatchdogT m_v1;
  tUint8 m_pad2;
  SaHpiWatchdogT m_v2;
  SaHpiWatchdogT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiWatchdogType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiWatchdogType ),
  dStructElement( cTest, m_v3   , SaHpiWatchdogType ),
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
    .m_v1.Log                = TRUE,
    .m_v1.Running            = TRUE,
    .m_v1.TimerUse           = SAHPI_WTU_NONE,
    .m_v1.TimerAction        = SAHPI_WA_NO_ACTION,
    .m_v1.PretimerInterrupt  = SAHPI_WPI_NONE,
    .m_v1.PreTimeoutInterval = 100,
    .m_v1.TimerUseExpFlags   = SAHPI_WATCHDOG_EXP_BIOS_FRB2,
    .m_v1.InitialCount       = 0,
    .m_v1.PresentCount       = 0,
    .m_pad2                  = 48,
    .m_v2.Log                = FALSE,
    .m_v2.Running            = FALSE,
    .m_v2.TimerUse           = SAHPI_WTU_BIOS_FRB2,
    .m_v2.TimerAction        = SAHPI_WA_RESET,
    .m_v2.PretimerInterrupt  = SAHPI_WPI_SMI,
    .m_v2.PreTimeoutInterval = 1100,
    .m_v2.TimerUseExpFlags   = SAHPI_WATCHDOG_EXP_BIOS_FRB2,
    .m_v2.InitialCount       = 1,
    .m_v2.PresentCount       = 1,
    .m_v3.Log                = TRUE,
    .m_v3.Running            = FALSE,
    .m_v3.TimerUse           = SAHPI_WTU_BIOS_POST,
    .m_v3.TimerAction        = SAHPI_WA_POWER_CYCLE,
    .m_v3.PretimerInterrupt  = SAHPI_WPI_OEM,
    .m_v3.PreTimeoutInterval = 100,
    .m_v3.TimerUseExpFlags   = SAHPI_WATCHDOG_EXP_OEM,
    .m_v3.InitialCount       = 1,
    .m_v3.PresentCount       = 0,
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

  if ( !cmp_watchdog( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_watchdog( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_watchdog( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
