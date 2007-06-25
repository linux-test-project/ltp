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
cmp_sensorenablechangeevent( SaHpiSensorEnableChangeEventT *d1, SaHpiSensorEnableChangeEventT *d2 )
{
  if ( d1->SensorNum != d2->SensorNum )
       return 0;

  if ( d1->SensorType != d2->SensorType )
       return 0;

  if ( d1->EventCategory != d2->EventCategory )
       return 0;

  if ( d1->SensorEnable != d2->SensorEnable )
       return 0;

  if ( d1->SensorEventEnable != d2->SensorEventEnable )
       return 0;

  if ( d1->AssertEventMask != d2->AssertEventMask )
       return 0;

  if ( d1->DeassertEventMask != d2->DeassertEventMask )
       return 0;

  if ( d1->OptionalDataPresent != d2->OptionalDataPresent )
       return 0;

  if ( d1->CurrentState != d2->CurrentState )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiSensorEnableChangeEventT m_v1;
  tUint8 m_pad2;
  SaHpiSensorEnableChangeEventT m_v2;
  SaHpiSensorEnableChangeEventT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorEnableChangeEventType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorEnableChangeEventType ),
  dStructElement( cTest, m_v3   , SaHpiSensorEnableChangeEventType ),
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
    .m_v1.SensorNum                    = 1,
    .m_v1.SensorType                   = SAHPI_TEMPERATURE,
    .m_v1.EventCategory                = SAHPI_EC_THRESHOLD,
    .m_v1.SensorEnable                 = TRUE,
    .m_v1.SensorEventEnable            = TRUE,
    .m_v1.AssertEventMask              = 0,
    .m_v1.DeassertEventMask            = 0,
    .m_v1.OptionalDataPresent          = 1,
    .m_v1.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
    .m_pad2                            = 48,
    .m_v2.SensorNum                    = 2,
    .m_v2.SensorType                   = SAHPI_BATTERY,
    .m_v2.EventCategory                = SAHPI_EC_THRESHOLD,
    .m_v2.SensorEnable                 = TRUE,
    .m_v2.SensorEventEnable            = FALSE,
    .m_v2.AssertEventMask              = 0,
    .m_v2.DeassertEventMask            = 0,
    .m_v2.OptionalDataPresent          = 1,
    .m_v2.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
    .m_v3.SensorNum                    = 3,
    .m_v3.SensorType                   = SAHPI_VOLTAGE,
    .m_v3.EventCategory                = SAHPI_EC_THRESHOLD,
    .m_v3.SensorEnable                 = FALSE,
    .m_v3.SensorEventEnable            = TRUE,
    .m_v3.AssertEventMask              = 1,
    .m_v3.DeassertEventMask            = 1,
    .m_v3.OptionalDataPresent          = 1,
    .m_v3.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
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

  if ( !cmp_sensorenablechangeevent( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_sensorenablechangeevent( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_sensorenablechangeevent( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
