/*
 * ipmi_sensor.h
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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
 */


#ifndef dIpmiSensor_h
#define dIpmiSensor_h


__BEGIN_DECLS
#include "SaHpi.h"
__END_DECLS


#ifndef dIpmiEvent_h
#include "ipmi_event.h"
#endif

#ifndef dIpmiSdr_h
#include "ipmi_sdr.h"
#endif

#ifndef dIpmiEntityr_h
#include "ipmi_entity.h"
#endif


struct tIpmiEntity;
class  cIpmiDomain;


enum tIpmiSensorType
{
  eIpmiSensorTypeInvalid                      = 0x00,
  eIpmiSensorTypeTemperature                  = 0x01,
  eIpmiSensorTypeVoltage		      = 0x02,
  eIpmiSensorTypeCurrent		      = 0x03,
  eIpmiSensorTypeFan			      = 0x04,
  eIpmiSensorTypePhysicalSecurity	      = 0x05,
  eIpmiSensorTypePlatformSecurity	      = 0x06,
  eIpmiSensorTypeProcessor		      =	0x07,
  eIpmiSensorTypePowerSupply		      =	0x08,
  eIpmiSensorTypePowerUnit		      =	0x09,
  eIpmiSensorTypeCoolingDevice		      =	0x0a,
  eIpmiSensorTypeOtherUnitsBasedSensor	      = 0x0b,
  eIpmiSensorTypeMemory			      =	0x0c,
  eIpmiSensorTypeDriveSlot		      =	0x0d,
  eIpmiSensorTypePowerMemoryResize	      =	0x0e,
  eIpmiSensorTypeSystemFirmwareProgress	      = 0x0f,
  eIpmiSensorTypeEventLoggingDisabled	      =	0x10,
  eIpmiSensorTypeWatchdog1		      =	0x11,
  eIpmiSensorTypeSystemEvent		      =	0x12,
  eIpmiSensorTypeCriticalInterrupt	      =	0x13,
  eIpmiSensorTypeButton			      =	0x14,
  eIpmiSensorTypeModuleBoard		      =	0x15,
  eIpmiSensorTypeMicrocontrollerCoprocessor   =	0x16,
  eIpmiSensorTypeAddInCard		      =	0x17,
  eIpmiSensorTypeChassis		      =	0x18,
  eIpmiSensorTypeChipSet		      =	0x19,
  eIpmiSensorTypeOtherFru		      =	0x1a,
  eIpmiSensorTypeCableInterconnect	      =	0x1b,
  eIpmiSensorTypeTerminator		      =	0x1c,
  eIpmiSensorTypeSystemBootInitiated	      =	0x1d,
  eIpmiSensorTypeBootError		      =	0x1e,
  eIpmiSensorTypeOsBoot			      = 0x1f,
  eIpmiSensorTypeOsCriticalStop		      = 0x20,
  eIpmiSensorTypeSlotConnector		      = 0x21,
  eIpmiSensorTypeSystemAcpiPowerState	      = 0x22,
  eIpmiSensorTypeWatchdog2		      = 0x23,
  eIpmiSensorTypePlatformAlert		      = 0x24,
  eIpmiSensorTypeEntityPresence		      = 0x25,
  eIpmiSensorTypeMonitorAsicIc		      = 0x26,
  eIpmiSensorTypeLan			      = 0x27,
  eIpmiSensorTypeManagementSubsystemHealth    = 0x28,
  eIpmiSensorTypeBattery		      = 0x29,
  eIpmiSensorTypeAtcaHotSwap                  = 0xf0
};

const char *IpmiSensorTypeToString( tIpmiSensorType type );


enum tIpmiEventReadingType
{
  eIpmiEventReadingTypeInvalid                   = 0x00,
  eIpmiEventReadingTypeThreshold                 = 0x01,
  eIpmiEventReadingTypeDiscreteUsage             = 0x02,
  eIpmiEventReadingTypeDiscreteState             = 0x03,
  eIpmiEventReadingTypeDiscretePredictiveFailure = 0x04,
  eIpmiEventReadingTypeDiscreteLimitExceeded     = 0x05,
  eIpmiEventReadingTypeDiscretePerformanceMet    = 0x06,
  eIpmiEventReadingTypeDiscreteSeverity          = 0x07,
  eIpmiEventReadingTypeDiscreteDevicePresence    = 0x08,
  eIpmiEventReadingTypeDiscreteDeviceEnable      = 0x09,
  eIpmiEventReadingTypeDiscreteAvailability      = 0x0a,
  eIpmiEventReadingTypeDiscreteRedundancy        = 0x0b,
  eIpmiEventReadingTypeDiscreteAcpiPower         = 0x0c,
  eIpmiEventReadingTypeSensorSpecific            = 0x6f
};

const char *IpmiEventReadingTypeToString( tIpmiEventReadingType type );


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


enum tIpmiEventSupport
{
  eIpmiEventSupportPerState     = 0,
  eIpmiEventSupportEntireSensor = 1,
  eIpmiEventSupportGlobalEnable = 2,
  eIpmiEventSupportNone         = 3
};

const char *IpmiEventSupportToString( tIpmiEventSupport val );

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


// analog data format
enum tIpmiAnalogeDataFormat
{
  eIpmiAnalogDataFormatUnsigned  = 0,
  eIpmiAnalogDataFormat1Compl    = 1,
  eIpmiAnalogDataFormat2Compl    = 2,
  eIpmiAnalogDataFormatNotAnalog = 3
};

const char *IpmiAnalogeDataFormatToString( tIpmiAnalogeDataFormat fmt );


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


// raw value linearization
enum tIpmiLinearization
{
  eIpmiLinearizationLinear    =  0,
  eIpmiLinearizationLn        =  1,
  eIpmiLinearizationLog10     =  2,
  eIpmiLinearizationLog2      =  3,
  eIpmiLinearizationE         =  4,
  eIpmiLinearizationExp10     =  5,
  eIpmiLinearizationExp2      =  6,
  eIpmiLinearization1OverX    =  7,
  eIpmiLinearizationSqr       =  8,
  eIpmiLinearizationCube      =  9,
  eIpmiLinearizationSqrt      = 10,
  eIpmiLinearization1OverCube = 11,
  eIpmiLinearizationNonlinear = 0x70
};

const char *IpmiLinearizationToString( tIpmiLinearization val );


enum tIpmiFruState
{
  eIpmiFruStateNotInstalled           = 0,
  eIpmiFruStateInactive               = 1,
  eIpmiFruStateActivationRequest      = 2,
  eIpmiFruStateActivationInProgress   = 3,
  eIpmiFruStateActive                 = 4,
  eIpmiFruStateDeactivationRequest    = 5,
  eIpmiFruStateDeactivationInProgress = 6,
  eIpmiFruStateCommunicationLost      = 7
};

const char *IpmiFruStateToString( tIpmiFruState state );


struct tIpmiThresholds
{
  /* Pay no attention to the implementation here. */
  struct
  {
    unsigned int m_status; /* Is this threshold enabled? */
    double       m_val;
  } m_vals[6];
};


struct tIpmiStates
{
  int          m_event_messages_enabled;
  int          m_sensor_scanning_enabled;
  int          m_initial_update_in_progress;
  unsigned int m_states;
};

enum tIpmiValuePresent
{
  eIpmiNoValuesPresent,
  eIpmiRawValuePresent,
  eIpmiBothValuesPresent
};


#define dSensorIdLen 32

cIpmiSensor **IpmiGetSensorsFromSdrs( cIpmiDomain  *domain,
                                      cIpmiMc      *source_mc,
                                      cIpmiSdrs    *sdrs,
                                      unsigned int &sensor_count );


class cIpmiSensor
{
protected:
  cIpmiMc     *m_mc; // My owner, NOT the SMI mc (unless that
                     // happens to be my direct owner).

  cIpmiMc     *m_source_mc; // If the sensor came from the main SDR,
                            // this will be NULL.  Otherwise, it
                            // will be the MC that owned the device
                            // SDR this came from.

  int          m_source_idx; // The index into the source array where
                             // this is stored.  This will be -1 if
                             // it does not have a source index (ie
                             // it's a non-standard sensor)
  cIpmiSensor **m_source_array; // This is the source array where
                                // the sensor is stored.

  unsigned int m_event_state; // HPI current event state

  bool          m_destroyed;
  int           m_use_count;

  unsigned char m_owner;
  unsigned char m_channel;
  unsigned char m_lun;
  unsigned char m_num;

  tIpmiEntityId m_entity_id;
  unsigned char m_entity_instance;

  bool          m_entity_instance_logical;
  bool          m_sensor_init_scanning;
  bool          m_sensor_init_events;
  bool          m_sensor_init_thresholds;
  bool          m_sensor_init_hysteresis;
  bool          m_sensor_init_type;
  bool          m_sensor_init_pu_events;
  bool          m_sensor_init_pu_scanning;
  bool          m_ignore_if_no_entity;
  bool          m_supports_auto_rearm;
  tIpmiHysteresisSupport m_hysteresis_support;
  tIpmiThresholdAccessSuport m_threshold_access;
  tIpmiEventSupport m_event_support;

  bool         m_hot_swap_requester;
  unsigned int m_hot_swap_requester_val;

  tIpmiSensorType m_sensor_type;
  tIpmiEventReadingType m_event_reading_type;

  unsigned char m_mask1[16];
  unsigned char m_mask2[16];
  unsigned char m_mask3[16];

  tIpmiAnalogeDataFormat m_analog_data_format;
  tIpmiRateUnit          m_rate_unit;
  tIpmiModifierUnit      m_modifier_unit_use;

  bool m_percentage;

  tIpmiUnitType m_base_unit;
  tIpmiUnitType m_modifier_unit;

  tIpmiLinearization m_linearization;

  int           m_m : 10;
  unsigned int  m_tolerance : 6;
  int           m_b : 10;
  int           m_r_exp : 4;
  unsigned int  m_accuracy_exp : 2;
  int           m_accuracy : 10;
  int           m_b_exp : 4;

  bool          m_normal_min_specified;
  bool          m_normal_max_specified;
  bool          m_nominal_reading_specified;

  unsigned char m_nominal_reading;
  unsigned char m_normal_max;
  unsigned char m_normal_min;
  unsigned char m_sensor_max;
  unsigned char m_sensor_min;
  unsigned char m_upper_non_recoverable_threshold;
  unsigned char m_upper_critical_threshold;
  unsigned char m_upper_non_critical_threshold;
  unsigned char m_lower_non_recoverable_threshold;
  unsigned char m_lower_critical_threshold;
  unsigned char m_lower_non_critical_threshold;
  unsigned char m_positive_going_threshold_hysteresis;
  unsigned char m_negative_going_threshold_hysteresis;

  unsigned char m_oem1;

  char m_id[dSensorIdLen+1]; // The ID from the device SDR.

  const char *m_sensor_type_string;
  const char *m_event_reading_type_string;
  const char *m_rate_unit_string;
  const char *m_base_unit_string;
  const char *m_modifier_unit_string;

  void FinalDestroy();
  int GetDefaultSensorThresholds( int raw,
                                  tIpmiThresholds &th );
  int CheckEventsCapability( const cIpmiEventState &states ) const;

  //  tIpmi_event_state_t event_state;
public:
  cIpmiSensor( cIpmiMc *mc );
  ~cIpmiSensor();

  cIpmiMc *Mc() const { return m_mc; }

  unsigned char Lun() const { return m_lun; }

  unsigned char Num() const { return m_num; }
  const char *Id() const { return m_id; }

  int &SourceIdx() { return m_source_idx; }
  cIpmiSensor **SourceArray() const { return m_source_array; }
  void SetSourceArray( cIpmiSensor **array ) { m_source_array = array; }
  void SetSourceArray( int idx, cIpmiSensor *sensor )
  {
    assert( idx >= 0 );
    m_source_array[idx] = sensor;
  }

  unsigned int &EventState() { return m_event_state; }

  tIpmiEntityId EntityId() const { return m_entity_id; }
  unsigned char EntityInstance() const { return m_entity_instance; }

  tIpmiSensorType SensorType() const { return m_sensor_type; }
  tIpmiEventReadingType EventReadingType() const { return m_event_reading_type; }
  bool IsThreshold() { return m_event_reading_type == eIpmiEventReadingTypeThreshold; }

  bool IgnoreIfNoEntity() const { return m_ignore_if_no_entity; }

  tIpmiHysteresisSupport HysteresisSupport() const { return m_hysteresis_support; }
  tIpmiThresholdAccessSuport ThresholdAccess() const { return m_threshold_access; }
  tIpmiEventSupport EventSupport() const { return m_event_support; }

  tIpmiAnalogeDataFormat AnalogDataFormat() const { return  m_analog_data_format; }
  tIpmiUnitType          BaseUnit() const { return m_base_unit; }
  tIpmiUnitType ModifierUnit() const { return m_modifier_unit; }
  tIpmiModifierUnit      ModifierUnitUse() const { return m_modifier_unit_use; }

  int           M()        const { return m_m; }
  unsigned int  Tolerance() const { return m_tolerance; }
  int           B()        const { return m_b; }
  int           RExp()     const { return m_r_exp; }
  unsigned int  AccuracyExp() const { return m_accuracy_exp; }
  int           Accuracy() const { return m_accuracy; }
  int           BExp()     const { return m_b_exp; }
  tIpmiLinearization Linearization() const { return m_linearization; }
  bool Percentage() const { return m_percentage; }
  unsigned char SensorMax() const { return m_sensor_max; }
  unsigned char SensorMin() const { return m_sensor_min; }
  bool          NominalReadingSpecified() const { return m_nominal_reading_specified; }
  unsigned char NominalReading() const { return m_nominal_reading; }
  bool          NormalMaxSpecified() const { return m_normal_max_specified; }
  unsigned char NormalMax() const { return m_normal_max; }
  bool          NormalMinSpecified() const { return m_normal_min_specified; }
  unsigned char NormalMin() const { return m_normal_min; }

  void HandleNew( cIpmiDomain *domain );
  bool Destroy();

  bool Cmp( const cIpmiSensor &s2 ) const ;

  int  ThresholdAssertionEventSupported( tIpmiThresh  event,
                                         tIpmiEventValueDir dir,
                                         int &val );
  int  ThresholdDeassertionEventSupported( tIpmiThresh  event,
                                           tIpmiEventValueDir dir,
                                           int &val );

  int  DiscreteEventReadable( int event, int &val );
  int  DiscreteAssertionEventSupported( int event, int &val );
  int  DiscreteDeassertionEventSupported( int event, int &val );
  void GetId( char *id, int length );
  int  ThresholdReadable( tIpmiThresh  event );
  int  ThresholdSettable( tIpmiThresh  event );
  cIpmiEntity *GetEntity();

  enum tIpmiRound
  {
      eRoundNormal,
      eRoundDown,
      eRoundUp
  };

  int ConvertFromRaw( unsigned int val, double &result );
  int ConvertToRaw( tIpmiRound rounding, double val, unsigned int &result );

  int ReadingGet( tIpmiValuePresent &val_present,
                  unsigned int &raw_val, double &val,
                  tIpmiStates &states );

  int StatesGet( tIpmiStates &states );

  int ThresholdsGet( tIpmiThresholds &th );
  int ThresholdsSet( tIpmiThresholds &th );

  int GetHysteresis( unsigned int &positive_hysteresis,
                     unsigned int &negative_hysteresis );
  int SetHysteresis( unsigned int pos, unsigned int neg );

  int EventsEnableGet( cIpmiEventState &state );
  int EventsEnableSet( const cIpmiEventState &state );

  void Event( cIpmiEvent *event );

  bool Ignore();
  
  void Log();

  friend cIpmiSensor **IpmiGetSensorsFromSdrs( cIpmiDomain  *domain,
                                               cIpmiMc      *source_mc,
                                               cIpmiSdrs    *sdrs,
                                               unsigned int &sensor_count );

  // HPI record id to find the rdr
  SaHpiEntryIdT m_record_id;
};


class cIpmiSensorInfo
{
public:
  cIpmiMc *m_mc;
  bool m_destroyed;

  // Indexed by LUN and sensor # 
  cIpmiSensor **(m_sensors_by_idx[5]);
  // Size of above sensor array, per LUN.  This will be 0 if the
  // LUN has no sensors.
  int            m_idx_size[5];
  // In the above two, the 5th index is for non-standard sensors.
  
  // Total number of sensors we have in this.
  unsigned int   m_sensor_count;

public:
  cIpmiSensorInfo( cIpmiMc *mc );
  ~cIpmiSensorInfo();
};


int IpmiSensorHandleSdr( cIpmiDomain *ipmi, cIpmiMc *mc, cIpmiSdrs *sdrs );

cIpmiSensor *IpmiMcFindSensor( cIpmiMc *mc, unsigned int lun, unsigned int sensor_num );


#endif
