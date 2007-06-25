/*
 * ipmi_sensor_threshold.h
 *
 * Copyright (c) 2004 by FORCE Computers
 * Copyright (c) 2005 by ESO Technologies.
 *
 * Note that this file is based on parts of OpenIPMI
 * written by Corey Minyard <minyard@mvista.com>
 * of MontaVista Software. Corey's code was helpful
 * and many thanks go to him. He gave the permission
 * to use this code in OpenHPI under BSD license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#ifndef dIpmiSensorThreshold_h
#define dIpmiSensorThreshold_h


#ifndef dIpmiSensor_h
#include "ipmi_sensor.h"
#endif

#ifndef dIpmiSensorFactors_h
#include "ipmi_sensor_factors.h"
#endif


enum tIpmiHysteresisSupport
{
  eIpmiHysteresisSupportNone     = 0,
  eIpmiHysteresisSupportReadable = 1,
  eIpmiHysteresisSupportSettable = 2,
  eIpmiHysteresisSupportFixed    = 3
};

const char *IpmiHysteresisSupportToString( tIpmiHysteresisSupport val );


enum tIpmiThresholdAccessSuport
{
  eIpmiThresholdAccessSupportNone     = 0,
  eIpmiThresholdAccessSupportReadable = 1,
  eIpmiThresholdAccessSupportSettable = 2,
  eIpmiThresholdAccessSupportFixed    = 3
};

const char *IpmiThresholdAccessSupportToString( tIpmiThresholdAccessSuport val );

// event mask bits for threshold sensors
#define dIpmiEventLowerNonCriticalLow     0x0001
#define dIpmiEventLowerNonCriticalHigh    0x0002
#define dIpmiEventLowerCriticalLow        0x0004
#define dIpmiEventLowerCriticalHigh       0x0008
#define dIpmiEventLowerNonRecoverableLow  0x0010
#define dIpmiEventLowerNonRecoverableHigh 0x0020
#define dIpmiEventUpperNonCriticalLow     0x0040
#define dIpmiEventUpperNonCriticalHigh    0x0080
#define dIpmiEventUpperCriticalLow        0x0100
#define dIpmiEventUpperCriticalHigh       0x0200
#define dIpmiEventUpperNonRecoverableLow  0x0400
#define dIpmiEventUpperNonRecoverableHigh 0x0800
#define dIpmiEventMask 0x0fff


void IpmiThresholdEventMaskToString( unsigned short em, char *str );


enum tIpmiUnitType
{
  eIpmiUnitTypeUnspecified      = 0,
  eIpmiUnitTypeDegreesC         = 1,
  eIpmiUnitTypeDegreesF         = 2,
  eIpmiUnitTypeDegreesK         = 3,
  eIpmiUnitTypeVolts            = 4,
  eIpmiUnitTypeAmps             = 5,
  eIpmiUnitTypeWatts            = 6,
  eIpmiUnitTypeJoules           = 7,
  eIpmiUnitTypeCoulombs         = 8,
  eIpmiUnitTypeVa               = 9,
  eIpmiUnitTypeNits             = 10,
  eIpmiUnitTypeLumens           = 11,
  eIpmiUnitTypeLux              = 12,
  eIpmiUnitTypeCandela          = 13,
  eIpmiUnitTypeKpa              = 14,
  eIpmiUnitTypePsi              = 15,
  eIpmiUnitTypeNewtons          = 16,
  eIpmiUnitTypeCfm              = 17,
  eIpmiUnitTypeRpm              = 18,
  eIpmiUnitTypeHz               = 19,
  eIpmiUnitTypeUseconds         = 20,
  eIpmiUnitTypeMseconds         = 21,
  eIpmiUnitTypeSeconds          = 22,
  eIpmiUnitTypeMinute           = 23,
  eIpmiUnitTypeHour             = 24,
  eIpmiUnitTypeDay              = 25,
  eIpmiUnitTypeWeek             = 26,
  eIpmiUnitTypeMil              = 27,
  eIpmiUnitTypeInches           = 28,
  eIpmiUnitTypeFeet             = 29,
  eIpmiUnitTypeCubicInchs       = 30,
  eIpmiUnitTypeCubicFeet        = 31,
  eIpmiUnitTypeMillimeters      = 32,
  eIpmiUnitTypeCentimeters      = 33,
  eIpmiUnitTypeMeters           = 34,
  eIpmiUnitTypeCubicCentimeters = 35,
  eIpmiUnitTypeCubicMeters      = 36,
  eIpmiUnitTypeLiters           = 37,
  eIpmiUnitTypeFlOz             = 38,
  eIpmiUnitTypeRadians          = 39,
  eIpmiUnitTypeSeradians        = 40,
  eIpmiUnitTypeRevolutions      = 41,
  eIpmiUnitTypeCycles           = 42,
  eIpmiUnitTypeGravities        = 43,
  eIpmiUnitTypeOunces           = 44,
  eIpmiUnitTypePounds           = 45,
  eIpmiUnitTypeFootPounds       = 46,
  eIpmiUnitTypeOunceInches      = 47,
  eIpmiUnitTypeGauss            = 48,
  eIpmiUnitTypeGilberts         = 49,
  eIpmiUnitTypeHenries          = 50,
  eIpmiUnitTypeMhenries         = 51,
  eIpmiUnitTypeFarads           = 52,
  eIpmiUnitTypeUfarads          = 53,
  eIpmiUnitTypeOhms             = 54,
  eIpmiUnitTypeSiemens          = 55,
  eIpmiUnitTypeMoles            = 56,
  eIpmiUnitTypeBecquerels       = 57,
  eIpmiUnitTypePpm              = 58,
  eIpmiUnitTypeReserved1        = 59,
  eIpmiUnitTypeDecibels         = 60,
  eIpmiUnitTypeDbA              = 61,
  eIpmiUnitTypeDbC              = 62,
  eIpmiUnitTypeGrays            = 63,
  eIpmiUnitTypeSieverts         = 64,
  eIpmiUnitTypeColorTempDegK    = 65,
  eIpmiUnitTypeBits             = 66,
  eIpmiUnitTypeKBits            = 67,
  eIpmiUnitTypeMBits            = 68,
  eIpmiUnitTypeGBits            = 69,
  eIpmiUnitTypeBytes            = 70,
  eIpmiUnitTypeKBytes           = 71,
  eIpmiUnitTypeMBytes           = 72,
  eIpmiUnitTypeGBytes           = 73,
  eIpmiUnitTypeWords            = 74,
  eIpmiUnitTypeDWords           = 75,
  eIpmiUnitTypeQWords           = 76,
  eIpmiUnitTypeLines            = 77,
  eIpmiUnitTypeHits             = 78,
  eIpmiUnitTypeMisses           = 79,
  eIpmiUnitTypeRetries          = 80,
  eIpmiUnitTypeResets           = 81,
  eIpmiUnitTypeOverruns         = 82,
  eIpmiUnitTypeUnderruns       	= 83,
  eIpmiUnitTypeCollisions       = 84,
  eIpmiUnitTypePackets          = 85,
  eIpmiUnitTypeMessages         = 86,
  eIpmiUnitTypeCharacters       = 87,
  eIpmiUnitTypeErrors           = 88,
  eIpmiUnitTypeCorrectableErrors = 89,
  eIpmiUnitTypeUncorrectableErrors = 90
};

const char *IpmiUnitTypeToString( tIpmiUnitType val );


enum tIpmiRateUnit
{
  eIpmRateUnitNone      = 0,
  eIpmRateUnitPerUS     = 1,
  eIpmRateUnitPerMs     = 2,
  eIpmRateUnitPerS      = 3,
  eIpmRateUnitPerMinute = 4,
  eIpmRateUnitPerHour   = 5,
  eIpmRateUnitDay       = 6
};

const char *IpmiRateUnitToString( tIpmiRateUnit unit );


enum tIpmiModifierUnit
{
  eIpmiModifierUnitNone             = 0,
  eIpmiModifierUnitBasicDivModifier = 1,
  eIpmiModifierUnitBasicMulModifier = 2
};

const char *IpmiModifierUnitToString( tIpmiModifierUnit unit );


struct tIpmiThresholds
{
  /* Pay no attention to the implementation here. */
  struct
  {
    unsigned int m_status; /* Is this threshold enabled? */
    double       m_val;
  } m_vals[6];
};


class cIpmiSensorThreshold : public cIpmiSensor
{
protected:
  bool                   m_sensor_init_thresholds;
  bool                   m_sensor_init_hysteresis;

  tIpmiHysteresisSupport m_hysteresis_support;
  tIpmiThresholdAccessSuport m_threshold_access;

  unsigned int           m_threshold_readable;
  unsigned int           m_threshold_settable;

  tIpmiRateUnit          m_rate_unit;
  tIpmiModifierUnit      m_modifier_unit_use;

  bool                   m_percentage;

  tIpmiUnitType          m_base_unit;
  tIpmiUnitType          m_modifier_unit;

  bool                   m_normal_min_specified;
  bool                   m_normal_max_specified;
  bool                   m_nominal_reading_specified;
  bool                   m_swap_thresholds;

  unsigned char          m_nominal_reading;
  unsigned char          m_normal_max;
  unsigned char          m_normal_min;
  unsigned char          m_sensor_max;
  unsigned char          m_sensor_min;
  unsigned char          m_upper_non_recoverable_threshold;
  unsigned char          m_upper_critical_threshold;
  unsigned char          m_upper_non_critical_threshold;
  unsigned char          m_lower_non_recoverable_threshold;
  unsigned char          m_lower_critical_threshold;
  unsigned char          m_lower_non_critical_threshold;
  unsigned char          m_positive_going_threshold_hysteresis;
  unsigned char          m_negative_going_threshold_hysteresis;

  unsigned char          m_current_positive_hysteresis;
  unsigned char          m_current_negative_hysteresis;

public:
  cIpmiSensorThreshold( cIpmiMc *mc );
  virtual ~cIpmiSensorThreshold();

  virtual void HandleNew( cIpmiDomain *domain );

  tIpmiHysteresisSupport     HysteresisSupport() const { return m_hysteresis_support; }
  tIpmiThresholdAccessSuport ThresholdAccess()   const { return m_threshold_access; }

  tIpmiUnitType          BaseUnit() const { return m_base_unit; }
  tIpmiUnitType ModifierUnit() const { return m_modifier_unit; }
  tIpmiModifierUnit      ModifierUnitUse() const { return m_modifier_unit_use; }

  bool Percentage() const { return m_percentage; }
  unsigned char SensorMax() const { return m_sensor_max; }
  unsigned char SensorMin() const { return m_sensor_min; }
  bool          NominalReadingSpecified() const { return m_nominal_reading_specified; }
  unsigned char NominalReading() const { return m_nominal_reading; }
  bool          NormalMaxSpecified() const { return m_normal_max_specified; }
  unsigned char NormalMax() const { return m_normal_max; }
  bool          NormalMinSpecified() const { return m_normal_min_specified; }
  unsigned char NormalMin() const { return m_normal_min; }
  bool          SwapThresholds() const { return m_swap_thresholds; }

  bool IsThresholdReadable( tIpmiThresh event );
  bool IsThresholdSettable( tIpmiThresh event );

  // create an hpi event from ipmi event
  virtual SaErrorT CreateEvent( cIpmiEvent *event, SaHpiEventT &h );

  virtual void Dump( cIpmiLog &dump ) const;
  bool Cmp( const cIpmiSensor &s2 ) const;

protected:
  cIpmiSensorFactors *m_sensor_factors;

  virtual cIpmiSensorFactors *CreateSensorFactors( cIpmiMc *mc, cIpmiSdr *sdr );

  // create HPI event mask
  unsigned short GetEventMask(unsigned int ipmi_event_mask);

  // convert to HPI interpreted values
  void ConvertToInterpreted( unsigned int v, SaHpiSensorReadingT &r );
  void ConvertToInterpreted( unsigned int v, SaHpiSensorReadingT &r, bool is_hysteresis );
  SaErrorT ConvertFromInterpreted( const SaHpiSensorReadingT r,
                                   unsigned char &v );
  SaErrorT ConvertFromInterpreted( const SaHpiSensorReadingT r,
                                   unsigned char &v,
                                   bool is_hysteresis);

  SaErrorT ConvertThreshold( const SaHpiSensorReadingT &r, 
                             tIpmiThresh event,
                             unsigned char &data,
                             unsigned char &mask );

public:
  cIpmiSensorFactors *GetFactors() { return m_sensor_factors; }

  // read sensor parameter from Full Sensor Record
  virtual bool GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr );

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  // get sensor data
  SaErrorT GetSensorReading( SaHpiSensorReadingT &data, SaHpiEventStateT &state );

  SaErrorT GetThresholdsAndHysteresis( SaHpiSensorThresholdsT &thres );

protected:
  // helper functions for GetThresholdsAndHysteresis
  SaErrorT GetThresholds( SaHpiSensorThresholdsT &thres );
  SaErrorT GetDefaultThresholds( SaHpiSensorThresholdsT &thres );
  SaErrorT GetHysteresis( SaHpiSensorThresholdsT &thres );

public:
  SaErrorT SetThresholdsAndHysteresis( const SaHpiSensorThresholdsT &thres );

protected:
  // helper functions for SetThresholdsAndHysteresis
  SaErrorT SetThresholds( const SaHpiSensorThresholdsT &thres );
  SaErrorT SetHysteresis( const SaHpiSensorThresholdsT &thres );

public:
  virtual SaErrorT GetEventMasksHw( SaHpiEventStateT &AssertEventMask,
                                    SaHpiEventStateT &DeassertEventMask
                                  );
  virtual SaErrorT SetEventMasksHw( const SaHpiEventStateT &AssertEventMask,
                                    const SaHpiEventStateT &DeassertEventMask
                                  );
};


#endif
