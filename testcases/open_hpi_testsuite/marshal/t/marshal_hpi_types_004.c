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


typedef struct
{
  tUint8 m_pad1;
  SaHpiSensorThresholdsT m_v1;
  tUint8 m_pad2;
  SaHpiSensorThresholdsT m_v2;
  SaHpiSensorThresholdsT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorThresholdsType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorThresholdsType ),
  dStructElement( cTest, m_v3   , SaHpiSensorThresholdsType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                = 47,
    .m_v1.LowCritical.IsSupported          = TRUE,
    .m_v1.LowCritical.Type                 = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.LowCritical.Value                = {-21},
    .m_v1.LowMajor.IsSupported             = TRUE,
    .m_v1.LowMajor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.LowMajor.Value                   = {-21},
    .m_v1.LowMinor.IsSupported             = TRUE,
    .m_v1.LowMinor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.LowMinor.Value                   = {-10},
    .m_v1.UpCritical.IsSupported           = TRUE,
    .m_v1.UpCritical.Type                  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.UpCritical.Value                 = {21},
    .m_v1.UpMajor.IsSupported              = TRUE,
    .m_v1.UpMajor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.UpMajor.Value                    = {21},
    .m_v1.UpMinor.IsSupported              = TRUE,
    .m_v1.UpMinor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.UpMinor.Value                    = {10},
    .m_v1.PosThdHysteresis.IsSupported     = TRUE,
    .m_v1.PosThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.PosThdHysteresis.Value           = {5},
    .m_v1.NegThdHysteresis.IsSupported     = TRUE,
    .m_v1.NegThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.NegThdHysteresis.Value           = {-5},
    .m_pad2                                = 48,
    .m_v2.LowCritical.IsSupported          = TRUE,
    .m_v2.LowCritical.Type                 = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.LowCritical.Value                = {-21},
    .m_v2.LowMajor.IsSupported             = FALSE,
    .m_v2.LowMajor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.LowMajor.Value                   = {-21},
    .m_v2.LowMinor.IsSupported             = TRUE,
    .m_v2.LowMinor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.LowMinor.Value                   = {-10},
    .m_v2.UpCritical.IsSupported           = TRUE,
    .m_v2.UpCritical.Type                  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.UpCritical.Value                 = {21},
    .m_v2.UpMajor.IsSupported              = FALSE,
    .m_v2.UpMajor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.UpMajor.Value                    = {21},
    .m_v2.UpMinor.IsSupported              = TRUE,
    .m_v2.UpMinor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.UpMinor.Value                    = {10},
    .m_v2.PosThdHysteresis.IsSupported     = TRUE,
    .m_v2.PosThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.PosThdHysteresis.Value           = {5},
    .m_v2.NegThdHysteresis.IsSupported     = TRUE,
    .m_v2.NegThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.NegThdHysteresis.Value           = {-5},
    .m_v3.LowCritical.IsSupported          = TRUE,
    .m_v3.LowCritical.Type                 = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.LowCritical.Value                = {-21},
    .m_v3.LowMajor.IsSupported             = TRUE,
    .m_v3.LowMajor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.LowMajor.Value                   = {-21},
    .m_v3.LowMinor.IsSupported             = TRUE,
    .m_v3.LowMinor.Type                    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.LowMinor.Value                   = {-10},
    .m_v3.UpCritical.IsSupported           = TRUE,
    .m_v3.UpCritical.Type                  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.UpCritical.Value                 = {21},
    .m_v3.UpMajor.IsSupported              = TRUE,
    .m_v3.UpMajor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.UpMajor.Value                    = {21},
    .m_v3.UpMinor.IsSupported              = TRUE,
    .m_v3.UpMinor.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.UpMinor.Value                    = {10},
    .m_v3.PosThdHysteresis.IsSupported     = FALSE,
    .m_v3.PosThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.PosThdHysteresis.Value           = {5},
    .m_v3.NegThdHysteresis.IsSupported     = FALSE,
    .m_v3.NegThdHysteresis.Type            = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.NegThdHysteresis.Value           = {-5},
    .m_pad3                                = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_sensorreading( &value.m_v1.LowCritical, &result.m_v1.LowCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.LowMajor, &result.m_v1.LowMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.LowMinor, &result.m_v1.LowMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.UpCritical, &result.m_v1.UpCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.UpMajor, &result.m_v1.UpMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.UpMinor, &result.m_v1.UpMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.PosThdHysteresis, &result.m_v1.PosThdHysteresis ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.NegThdHysteresis, &result.m_v1.NegThdHysteresis ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_sensorreading( &value.m_v2.LowCritical, &result.m_v2.LowCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.LowMajor, &result.m_v2.LowMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.LowMinor, &result.m_v2.LowMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.UpCritical, &result.m_v2.UpCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.UpMajor, &result.m_v2.UpMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.UpMinor, &result.m_v2.UpMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.PosThdHysteresis, &result.m_v2.PosThdHysteresis ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.NegThdHysteresis, &result.m_v2.NegThdHysteresis ) )
       return 1;

  if ( !cmp_sensorreading( &value.m_v3.LowCritical, &result.m_v3.LowCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.LowMajor, &result.m_v3.LowMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.LowMinor, &result.m_v3.LowMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.UpCritical, &result.m_v3.UpCritical ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.UpMajor, &result.m_v3.UpMajor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.UpMinor, &result.m_v3.UpMinor ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.PosThdHysteresis, &result.m_v3.PosThdHysteresis ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.NegThdHysteresis, &result.m_v3.NegThdHysteresis ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
