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
  SaHpiSensorRangeT m_v1;
  tUint8 m_pad2;
  SaHpiSensorRangeT m_v2;
  SaHpiSensorRangeT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorRangeType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorRangeType ),
  dStructElement( cTest, m_v3   , SaHpiSensorRangeType ),
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
    .m_v1.Flags                            = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v1.Max.IsSupported                  = TRUE,
    .m_v1.Max.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Max.Value                        = {21},
    .m_v1.Min.IsSupported                  = TRUE,
    .m_v1.Min.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Min.Value                        = {-21},
    .m_v1.Nominal.IsSupported              = TRUE,
    .m_v1.Nominal.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Nominal.Value                    = {0},
    .m_v1.NormalMax.IsSupported            = TRUE,
    .m_v1.NormalMax.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.NormalMax.Value                  = {5},
    .m_v1.NormalMin.IsSupported            = TRUE,
    .m_v1.NormalMin.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.NormalMin.Value                  = {-5},
    .m_pad2                                = 48,
    .m_v2.Flags                            = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v2.Max.IsSupported                  = TRUE,
    .m_v2.Max.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Max.Value                        = {10},
    .m_v2.Min.IsSupported                  = TRUE,
    .m_v2.Min.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Min.Value                        = {-10},
    .m_v2.Nominal.IsSupported              = TRUE,
    .m_v2.Nominal.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Nominal.Value                    = {0},
    .m_v2.NormalMax.IsSupported            = TRUE,
    .m_v2.NormalMax.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.NormalMax.Value                  = {5},
    .m_v2.NormalMin.IsSupported            = TRUE,
    .m_v2.NormalMin.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.NormalMin.Value                  = {-5},
    .m_v3.Flags                            = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v3.Max.IsSupported                  = TRUE,
    .m_v3.Max.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Max.Value                        = {10},
    .m_v3.Min.IsSupported                  = TRUE,
    .m_v3.Min.Type                         = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Min.Value                        = {-10},
    .m_v3.Nominal.IsSupported              = TRUE,
    .m_v3.Nominal.Type                     = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Nominal.Value                    = {3},
    .m_v3.NormalMax.IsSupported            = TRUE,
    .m_v3.NormalMax.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.NormalMax.Value                  = {7},
    .m_v3.NormalMin.IsSupported            = TRUE,
    .m_v3.NormalMin.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.NormalMin.Value                  = {-7},
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

  if ( value.m_v1.Flags != result.m_v1.Flags )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Max, &result.m_v1.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Min, &result.m_v1.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Nominal, &result.m_v1.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.NormalMax, &result.m_v1.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.NormalMin, &result.m_v1.NormalMin ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( value.m_v2.Flags != result.m_v2.Flags )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Max, &result.m_v2.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Min, &result.m_v2.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Nominal, &result.m_v2.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.NormalMax, &result.m_v2.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.NormalMin, &result.m_v2.NormalMin ) )
       return 1;

  if ( value.m_v3.Flags != result.m_v3.Flags )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Max, &result.m_v3.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Min, &result.m_v3.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Nominal, &result.m_v3.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.NormalMax, &result.m_v3.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.NormalMin, &result.m_v3.NormalMin ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
