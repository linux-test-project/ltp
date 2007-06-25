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
  SaHpiSensorDataFormatT m_v1;
  tUint8 m_pad2;
  SaHpiSensorDataFormatT m_v2;
  SaHpiSensorDataFormatT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorDataFormatType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorDataFormatType ),
  dStructElement( cTest, m_v3   , SaHpiSensorDataFormatType ),
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
    .m_v1.IsSupported                      = TRUE,
    .m_v1.ReadingType                      = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.BaseUnits                        = SAHPI_SU_DEGREES_C,
    .m_v1.ModifierUnits                    = SAHPI_SU_DEGREES_C,
    .m_v1.ModifierUse                      = SAHPI_SMUU_NONE,
    .m_v1.Percentage                       = TRUE,
    .m_v1.Range.Flags                      = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v1.Range.Max.IsSupported            = TRUE,
    .m_v1.Range.Max.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Range.Max.Value                  = {10},
    .m_v1.Range.Min.IsSupported            = TRUE,
    .m_v1.Range.Min.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Range.Min.Value                  = {-10},
    .m_v1.Range.Nominal.IsSupported        = TRUE,
    .m_v1.Range.Nominal.Type               = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Range.Nominal.Value              = {0},
    .m_v1.Range.NormalMax.IsSupported      = TRUE,
    .m_v1.Range.NormalMax.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Range.NormalMax.Value            = {5},
    .m_v1.Range.NormalMin.IsSupported      = TRUE,
    .m_v1.Range.NormalMin.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.Range.NormalMin.Value            = {-5},
    .m_v1.AccuracyFactor                   = 0,
    .m_pad2                                = 48,
    .m_v2.IsSupported                      = TRUE,
    .m_v2.ReadingType                      = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.BaseUnits                        = SAHPI_SU_DEGREES_F,
    .m_v2.ModifierUnits                    = SAHPI_SU_DEGREES_F,
    .m_v2.ModifierUse                      = SAHPI_SMUU_NONE,
    .m_v2.Percentage                       = FALSE,
    .m_v2.Range.Flags                      = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v2.Range.Max.IsSupported            = TRUE,
    .m_v2.Range.Max.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Range.Max.Value                  = {10},
    .m_v2.Range.Min.IsSupported            = TRUE,
    .m_v2.Range.Min.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Range.Min.Value                  = {-10},
    .m_v2.Range.Nominal.IsSupported        = TRUE,
    .m_v2.Range.Nominal.Type               = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Range.Nominal.Value              = {0},
    .m_v2.Range.NormalMax.IsSupported      = TRUE,
    .m_v2.Range.NormalMax.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Range.NormalMax.Value            = {5},
    .m_v2.Range.NormalMin.IsSupported      = TRUE,
    .m_v2.Range.NormalMin.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.Range.NormalMin.Value            = {-5},
    .m_v2.AccuracyFactor                   = 0,
    .m_v3.IsSupported                      = TRUE,
    .m_v3.ReadingType                      = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.BaseUnits                        = SAHPI_SU_DEGREES_F,
    .m_v3.ModifierUnits                    = SAHPI_SU_DEGREES_F,
    .m_v3.ModifierUse                      = SAHPI_SMUU_NONE,
    .m_v3.Percentage                       = FALSE,
    .m_v3.Range.Flags                      = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v3.Range.Max.IsSupported            = TRUE,
    .m_v3.Range.Max.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Range.Max.Value                  = {10},
    .m_v3.Range.Min.IsSupported            = TRUE,
    .m_v3.Range.Min.Type                   = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Range.Min.Value                  = {-10},
    .m_v3.Range.Nominal.IsSupported        = TRUE,
    .m_v3.Range.Nominal.Type               = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Range.Nominal.Value              = {0},
    .m_v3.Range.NormalMax.IsSupported      = FALSE,
    .m_v3.Range.NormalMax.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Range.NormalMax.Value            = {5},
    .m_v3.Range.NormalMin.IsSupported      = FALSE,
    .m_v3.Range.NormalMin.Type             = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.Range.NormalMin.Value            = {-5},
    .m_v3.AccuracyFactor                   = 0,
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

  if ( value.m_v1.IsSupported != result.m_v1.IsSupported )
       return 1;
  if ( value.m_v1.ReadingType != result.m_v1.ReadingType )
       return 1;
  if ( value.m_v1.BaseUnits != result.m_v1.BaseUnits )
       return 1;
  if ( value.m_v1.ModifierUnits != result.m_v1.ModifierUnits )
       return 1;
  if ( value.m_v1.ModifierUse != result.m_v1.ModifierUse )
       return 1;
  if ( value.m_v1.Percentage != result.m_v1.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Range.Max, &result.m_v1.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Range.Min, &result.m_v1.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Range.Nominal, &result.m_v1.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Range.NormalMax, &result.m_v1.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.Range.NormalMin, &result.m_v1.Range.NormalMin ) )
       return 1;
  if ( value.m_v1.AccuracyFactor != result.m_v1.AccuracyFactor )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( value.m_v2.IsSupported != result.m_v2.IsSupported )
       return 1;
  if ( value.m_v2.ReadingType != result.m_v2.ReadingType )
       return 1;
  if ( value.m_v2.BaseUnits != result.m_v2.BaseUnits )
       return 1;
  if ( value.m_v2.ModifierUnits != result.m_v2.ModifierUnits )
       return 1;
  if ( value.m_v2.ModifierUse != result.m_v2.ModifierUse )
       return 1;
  if ( value.m_v2.Percentage != result.m_v2.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Range.Max, &result.m_v2.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Range.Min, &result.m_v2.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Range.Nominal, &result.m_v2.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Range.NormalMax, &result.m_v2.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.Range.NormalMin, &result.m_v2.Range.NormalMin ) )
       return 1;
  if ( value.m_v2.AccuracyFactor != result.m_v2.AccuracyFactor )
       return 1;

  if ( value.m_v3.IsSupported != result.m_v3.IsSupported )
       return 1;
  if ( value.m_v3.ReadingType != result.m_v3.ReadingType )
       return 1;
  if ( value.m_v3.BaseUnits != result.m_v3.BaseUnits )
       return 1;
  if ( value.m_v3.ModifierUnits != result.m_v3.ModifierUnits )
       return 1;
  if ( value.m_v3.ModifierUse != result.m_v3.ModifierUse )
       return 1;
  if ( value.m_v3.Percentage != result.m_v3.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Range.Max, &result.m_v3.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Range.Min, &result.m_v3.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Range.Nominal, &result.m_v3.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Range.NormalMax, &result.m_v3.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.Range.NormalMin, &result.m_v3.Range.NormalMin ) )
       return 1;
  if ( value.m_v3.AccuracyFactor != result.m_v3.AccuracyFactor )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
