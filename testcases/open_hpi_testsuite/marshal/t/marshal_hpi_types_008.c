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
cmp_thddefn( SaHpiSensorThdDefnT *d1, SaHpiSensorThdDefnT *d2 )
{
  if ( d1->IsAccessible != d2->IsAccessible )
       return 0;

  if ( d1->ReadThold != d2->ReadThold )
       return 0;

  if ( d1->WriteThold != d2->WriteThold )
       return 0;

  if ( d1->Nonlinear != d2->Nonlinear )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiSensorRecT m_v1;
  tUint8 m_pad2;
  SaHpiSensorRecT m_v2;
  SaHpiSensorRecT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiSensorRecType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiSensorRecType ),
  dStructElement( cTest, m_v3   , SaHpiSensorRecType ),
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
    .m_v1.Num                              = 1,
    .m_v1.Type                             = SAHPI_TEMPERATURE,
    .m_v1.Category                         = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v1.EnableCtrl                       = TRUE,
    .m_v1.EventCtrl                        = SAHPI_SEC_READ_ONLY,
    .m_v1.Events                           = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v1.DataFormat.IsSupported           = TRUE,
    .m_v1.DataFormat.ReadingType           = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.BaseUnits             = SAHPI_SU_DEGREES_C,
    .m_v1.DataFormat.ModifierUnits         = SAHPI_SU_DEGREES_C,
    .m_v1.DataFormat.ModifierUse           = SAHPI_SMUU_NONE,
    .m_v1.DataFormat.Percentage            = TRUE,
    .m_v1.DataFormat.Range.Flags           = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v1.DataFormat.Range.Max.IsSupported = TRUE,
    .m_v1.DataFormat.Range.Max.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.Range.Max.Value       = {10},
    .m_v1.DataFormat.Range.Min.IsSupported = TRUE,
    .m_v1.DataFormat.Range.Min.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.Range.Min.Value       = {-10},
    .m_v1.DataFormat.Range.Nominal.IsSupported = TRUE,
    .m_v1.DataFormat.Range.Nominal.Type    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.Range.Nominal.Value   = {0},
    .m_v1.DataFormat.Range.NormalMax.IsSupported = TRUE,
    .m_v1.DataFormat.Range.NormalMax.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.Range.NormalMax.Value = {5},
    .m_v1.DataFormat.Range.NormalMin.IsSupported = TRUE,
    .m_v1.DataFormat.Range.NormalMin.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v1.DataFormat.Range.NormalMin.Value = {-5},
    .m_v1.DataFormat.AccuracyFactor        = 0,
    .m_v1.ThresholdDefn.IsAccessible       = TRUE,
    .m_v1.ThresholdDefn.ReadThold          = SAHPI_STM_LOW_MINOR,
    .m_v1.ThresholdDefn.WriteThold         = SAHPI_STM_UP_CRIT,
    .m_v1.ThresholdDefn.Nonlinear          = FALSE,
    .m_v1.Oem                              = 0,
    .m_pad2                                = 48,
    .m_v2.Num                              = 2,
    .m_v2.Type                             = SAHPI_TEMPERATURE,
    .m_v2.Category                         = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v2.EnableCtrl                       = FALSE,
    .m_v2.EventCtrl                        = SAHPI_SEC_READ_ONLY,
    .m_v2.Events                           = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v2.DataFormat.IsSupported           = TRUE,
    .m_v2.DataFormat.ReadingType           = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.BaseUnits             = SAHPI_SU_DEGREES_C,
    .m_v2.DataFormat.ModifierUnits         = SAHPI_SU_DEGREES_C,
    .m_v2.DataFormat.ModifierUse           = SAHPI_SMUU_NONE,
    .m_v2.DataFormat.Percentage            = FALSE,
    .m_v2.DataFormat.Range.Flags           = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v2.DataFormat.Range.Max.IsSupported = TRUE,
    .m_v2.DataFormat.Range.Max.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.Range.Max.Value       = {10},
    .m_v2.DataFormat.Range.Min.IsSupported = FALSE,
    .m_v2.DataFormat.Range.Min.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.Range.Min.Value       = {-10},
    .m_v2.DataFormat.Range.Nominal.IsSupported = TRUE,
    .m_v2.DataFormat.Range.Nominal.Type    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.Range.Nominal.Value   = {0},
    .m_v2.DataFormat.Range.NormalMax.IsSupported = TRUE,
    .m_v2.DataFormat.Range.NormalMax.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.Range.NormalMax.Value = {5},
    .m_v2.DataFormat.Range.NormalMin.IsSupported = TRUE,
    .m_v2.DataFormat.Range.NormalMin.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v2.DataFormat.Range.NormalMin.Value = {-5},
    .m_v2.DataFormat.AccuracyFactor        = 0,
    .m_v2.ThresholdDefn.IsAccessible       = FALSE,
    .m_v2.ThresholdDefn.ReadThold          = SAHPI_STM_LOW_MINOR,
    .m_v2.ThresholdDefn.WriteThold         = SAHPI_STM_UP_CRIT,
    .m_v2.ThresholdDefn.Nonlinear          = FALSE,
    .m_v2.Oem                              = 0,
    .m_v3.Num                              = 3,
    .m_v3.Type                             = SAHPI_TEMPERATURE,
    .m_v3.Category                         = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v3.EnableCtrl                       = TRUE,
    .m_v3.EventCtrl                        = SAHPI_SEC_READ_ONLY,
    .m_v3.Events                           = SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC,
    .m_v3.DataFormat.IsSupported           = TRUE,
    .m_v3.DataFormat.ReadingType           = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.BaseUnits             = SAHPI_SU_DEGREES_C,
    .m_v3.DataFormat.ModifierUnits         = SAHPI_SU_DEGREES_C,
    .m_v3.DataFormat.ModifierUse           = SAHPI_SMUU_NONE,
    .m_v3.DataFormat.Percentage            = TRUE,
    .m_v3.DataFormat.Range.Flags           = SAHPI_SRF_MIN | SAHPI_SRF_MAX |
                                             SAHPI_SRF_NORMAL_MIN | SAHPI_SRF_NORMAL_MAX |
                                             SAHPI_SRF_NOMINAL,
    .m_v3.DataFormat.Range.Max.IsSupported = FALSE,
    .m_v3.DataFormat.Range.Max.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.Range.Max.Value       = {10},
    .m_v3.DataFormat.Range.Min.IsSupported = FALSE,
    .m_v3.DataFormat.Range.Min.Type        = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.Range.Min.Value       = {-10},
    .m_v3.DataFormat.Range.Nominal.IsSupported = TRUE,
    .m_v3.DataFormat.Range.Nominal.Type    = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.Range.Nominal.Value   = {0},
    .m_v3.DataFormat.Range.NormalMax.IsSupported = FALSE,
    .m_v3.DataFormat.Range.NormalMax.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.Range.NormalMax.Value = {5},
    .m_v3.DataFormat.Range.NormalMin.IsSupported = TRUE,
    .m_v3.DataFormat.Range.NormalMin.Type  = SAHPI_SENSOR_READING_TYPE_INT64,
    .m_v3.DataFormat.Range.NormalMin.Value = {-5},
    .m_v3.DataFormat.AccuracyFactor        = 0,
    .m_v3.ThresholdDefn.IsAccessible       = TRUE,
    .m_v3.ThresholdDefn.ReadThold          = SAHPI_STM_LOW_MINOR,
    .m_v3.ThresholdDefn.WriteThold         = SAHPI_STM_UP_CRIT,
    .m_v3.ThresholdDefn.Nonlinear          = TRUE,
    .m_v2.Oem                              = 0,
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

  if ( value.m_v1.Num != result.m_v1.Num )
       return 1;
  if ( value.m_v1.Type != result.m_v1.Type )
       return 1;
  if ( value.m_v1.Category != result.m_v1.Category )
       return 1;
  if ( value.m_v1.EnableCtrl != result.m_v1.EnableCtrl )
       return 1;
  if ( value.m_v1.EventCtrl != result.m_v1.EventCtrl )
       return 1;
  if ( value.m_v1.Events != result.m_v1.Events )
       return 1;
  if ( value.m_v1.DataFormat.IsSupported != result.m_v1.DataFormat.IsSupported )
       return 1;
  if ( value.m_v1.DataFormat.ReadingType != result.m_v1.DataFormat.ReadingType )
       return 1;
  if ( value.m_v1.DataFormat.BaseUnits != result.m_v1.DataFormat.BaseUnits )
       return 1;
  if ( value.m_v1.DataFormat.ModifierUnits != result.m_v1.DataFormat.ModifierUnits )
       return 1;
  if ( value.m_v1.DataFormat.ModifierUse != result.m_v1.DataFormat.ModifierUse )
       return 1;
  if ( value.m_v1.DataFormat.Percentage != result.m_v1.DataFormat.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.DataFormat.Range.Max, &result.m_v1.DataFormat.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.DataFormat.Range.Min, &result.m_v1.DataFormat.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.DataFormat.Range.Nominal, &result.m_v1.DataFormat.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.DataFormat.Range.NormalMax, &result.m_v1.DataFormat.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v1.DataFormat.Range.NormalMin, &result.m_v1.DataFormat.Range.NormalMin ) )
       return 1;
  if ( value.m_v1.DataFormat.AccuracyFactor != result.m_v1.DataFormat.AccuracyFactor )
       return 1;
  if ( !cmp_thddefn( &value.m_v1.ThresholdDefn, &result.m_v1.ThresholdDefn ) )
       return 1;
  if ( value.m_v1.Oem != result.m_v1.Oem )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( value.m_v2.Num != result.m_v2.Num )
       return 1;
  if ( value.m_v2.Type != result.m_v2.Type )
       return 1;
  if ( value.m_v2.Category != result.m_v2.Category )
       return 1;
  if ( value.m_v2.EnableCtrl != result.m_v2.EnableCtrl )
       return 1;
  if ( value.m_v2.EventCtrl != result.m_v2.EventCtrl )
       return 1;
  if ( value.m_v2.Events != result.m_v2.Events )
       return 1;
  if ( value.m_v2.DataFormat.IsSupported != result.m_v2.DataFormat.IsSupported )
       return 1;
  if ( value.m_v2.DataFormat.ReadingType != result.m_v2.DataFormat.ReadingType )
       return 1;
  if ( value.m_v2.DataFormat.BaseUnits != result.m_v2.DataFormat.BaseUnits )
       return 1;
  if ( value.m_v2.DataFormat.ModifierUnits != result.m_v2.DataFormat.ModifierUnits )
       return 1;
  if ( value.m_v2.DataFormat.ModifierUse != result.m_v2.DataFormat.ModifierUse )
       return 1;
  if ( value.m_v2.DataFormat.Percentage != result.m_v2.DataFormat.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.DataFormat.Range.Max, &result.m_v2.DataFormat.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.DataFormat.Range.Min, &result.m_v2.DataFormat.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.DataFormat.Range.Nominal, &result.m_v2.DataFormat.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.DataFormat.Range.NormalMax, &result.m_v2.DataFormat.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v2.DataFormat.Range.NormalMin, &result.m_v2.DataFormat.Range.NormalMin ) )
       return 1;
  if ( value.m_v2.DataFormat.AccuracyFactor != result.m_v2.DataFormat.AccuracyFactor )
       return 1;
  if ( !cmp_thddefn( &value.m_v2.ThresholdDefn, &result.m_v2.ThresholdDefn ) )
       return 1;
  if ( value.m_v2.Oem != result.m_v2.Oem )
       return 1;

  if ( value.m_v3.Num != result.m_v3.Num )
       return 1;
  if ( value.m_v3.Type != result.m_v3.Type )
       return 1;
  if ( value.m_v3.Category != result.m_v3.Category )
       return 1;
  if ( value.m_v3.EnableCtrl != result.m_v3.EnableCtrl )
       return 1;
  if ( value.m_v3.EventCtrl != result.m_v3.EventCtrl )
       return 1;
  if ( value.m_v3.Events != result.m_v3.Events )
       return 1;
  if ( value.m_v3.DataFormat.IsSupported != result.m_v3.DataFormat.IsSupported )
       return 1;
  if ( value.m_v3.DataFormat.ReadingType != result.m_v3.DataFormat.ReadingType )
       return 1;
  if ( value.m_v3.DataFormat.BaseUnits != result.m_v3.DataFormat.BaseUnits )
       return 1;
  if ( value.m_v3.DataFormat.ModifierUnits != result.m_v3.DataFormat.ModifierUnits )
       return 1;
  if ( value.m_v3.DataFormat.ModifierUse != result.m_v3.DataFormat.ModifierUse )
       return 1;
  if ( value.m_v3.DataFormat.Percentage != result.m_v3.DataFormat.Percentage )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.DataFormat.Range.Max, &result.m_v3.DataFormat.Range.Max ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.DataFormat.Range.Min, &result.m_v3.DataFormat.Range.Min ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.DataFormat.Range.Nominal, &result.m_v3.DataFormat.Range.Nominal ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.DataFormat.Range.NormalMax, &result.m_v3.DataFormat.Range.NormalMax ) )
       return 1;
  if ( !cmp_sensorreading( &value.m_v3.DataFormat.Range.NormalMin, &result.m_v3.DataFormat.Range.NormalMin ) )
       return 1;
  if ( value.m_v3.DataFormat.AccuracyFactor != result.m_v3.DataFormat.AccuracyFactor )
       return 1;
  if ( !cmp_thddefn( &value.m_v3.ThresholdDefn, &result.m_v3.ThresholdDefn ) )
       return 1;
  if ( value.m_v3.Oem != result.m_v3.Oem )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
