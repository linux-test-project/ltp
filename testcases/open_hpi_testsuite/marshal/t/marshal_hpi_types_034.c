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
cmp_sensorreading( SaHpiSensorReadingT *d1, SaHpiSensorReadingT *d2 )
{
  if ( d1->IsSupported != d2->IsSupported )
       return 0;

  if ( d1->Type != d2->Type )
       return 0;

  if ( d1->Value.SensorInt64 != d2->Value.SensorInt64 )
       return 0;

  return 1;
}


static int
cmp_sensorevent( SaHpiSensorEventT *d1, SaHpiSensorEventT *d2 )
{
  if ( d1->SensorNum != d2->SensorNum )
       return 0;

  if ( d1->SensorType != d2->SensorType )
       return 0;

  if ( d1->EventCategory != d2->EventCategory )
       return 0;

  if ( d1->Assertion != d2->Assertion )
       return 0;

  if ( d1->EventState != d2->EventState )
       return 0;

  if ( d1->OptionalDataPresent != d2->OptionalDataPresent )
       return 0;

  if ( !cmp_sensorreading( &d1->TriggerReading, &d2->TriggerReading ) )
       return 0;

  if ( !cmp_sensorreading( &d1->TriggerThreshold, &d2->TriggerThreshold ) )
       return 0;

  if ( d1->PreviousState != d2->PreviousState )
       return 0;

  if ( d1->CurrentState != d2->CurrentState )
       return 0;

  if ( d1->Oem != d2->Oem )
       return 0;

  if ( d1->SensorSpecific != d2->SensorSpecific )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiSensorEventT m_v1;
  tUint8 m_pad2;
  SaHpiSensorEventT m_v2;
  SaHpiSensorEventT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorEventType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorEventType ),
  dStructElement( cTest, m_v3   , SaHpiSensorEventType ),
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
    .m_v1.Assertion                    = TRUE,
    .m_v1.EventState                   = SAHPI_ES_UPPER_MAJOR,
    .m_v1.OptionalDataPresent          = 1,
    .m_v1.TriggerReading.IsSupported   = TRUE,
    .m_v1.TriggerReading.Type          = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.TriggerReading.Value         = {21},
    .m_v1.TriggerThreshold.IsSupported = TRUE,
    .m_v1.TriggerThreshold.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.TriggerThreshold.Value       = {20},
    .m_v1.PreviousState                = SAHPI_ES_ACTIVE,
    .m_v1.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
    .m_v1.Oem                          = 0,
    .m_v1.SensorSpecific               = 0,
    .m_pad2                            = 48,
    .m_v2.SensorNum                    = 2,
    .m_v2.SensorType                   = SAHPI_BATTERY,
    .m_v2.EventCategory                = SAHPI_EC_THRESHOLD,
    .m_v2.Assertion                    = TRUE,
    .m_v2.EventState                   = SAHPI_ES_UPPER_MAJOR,
    .m_v2.OptionalDataPresent          = 1,
    .m_v2.TriggerReading.IsSupported   = TRUE,
    .m_v2.TriggerReading.Type          = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.TriggerReading.Value         = {11},
    .m_v2.TriggerThreshold.IsSupported = TRUE,
    .m_v2.TriggerThreshold.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.TriggerThreshold.Value       = {10},
    .m_v2.PreviousState                = SAHPI_ES_ACTIVE,
    .m_v2.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
    .m_v2.Oem                          = 1,
    .m_v2.SensorSpecific               = 1,
    .m_v3.SensorNum                    = 3,
    .m_v3.SensorType                   = SAHPI_VOLTAGE,
    .m_v3.EventCategory                = SAHPI_EC_THRESHOLD,
    .m_v3.Assertion                    = TRUE,
    .m_v3.EventState                   = SAHPI_ES_UPPER_MAJOR,
    .m_v3.OptionalDataPresent          = 1,
    .m_v3.TriggerReading.IsSupported   = TRUE,
    .m_v3.TriggerReading.Type          = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.TriggerReading.Value         = {6},
    .m_v3.TriggerThreshold.IsSupported = TRUE,
    .m_v3.TriggerThreshold.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.TriggerThreshold.Value       = {5},
    .m_v3.PreviousState                = SAHPI_ES_ACTIVE,
    .m_v3.CurrentState                 = SAHPI_ES_UPPER_MAJOR,
    .m_v3.Oem                          = 10,
    .m_v3.SensorSpecific               = 10,
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

  if ( !cmp_sensorevent( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_sensorevent( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_sensorevent( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
