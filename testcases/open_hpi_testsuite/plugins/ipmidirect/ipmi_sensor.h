/*
 * ipmi_sensor.h
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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


#ifndef dIpmiSensor_h
#define dIpmiSensor_h


extern "C" {
#include "SaHpi.h"
}

#ifndef dIpmiMsg_h
#include "ipmi_msg.h"
#endif

#ifndef dIpmiEvent_h
#include "ipmi_event.h"
#endif

#ifndef dIpmiSdr_h
#include "ipmi_sdr.h"
#endif

#ifndef dIpmiRdr_h
#include "ipmi_rdr.h"
#endif


struct tIpmiEntity;
class  cIpmiDomain;


enum tIpmiSensorType
{
  eIpmiSensorTypeInvalid                    = 0x00,
  eIpmiSensorTypeTemperature                = 0x01,
  eIpmiSensorTypeVoltage                    = 0x02,
  eIpmiSensorTypeCurrent                    = 0x03,
  eIpmiSensorTypeFan                        = 0x04,
  eIpmiSensorTypePhysicalSecurity           = 0x05,
  eIpmiSensorTypePlatformSecurity           = 0x06,
  eIpmiSensorTypeProcessor                  = 0x07,
  eIpmiSensorTypePowerSupply                = 0x08,
  eIpmiSensorTypePowerUnit                  = 0x09,
  eIpmiSensorTypeCoolingDevice              = 0x0a,
  eIpmiSensorTypeOtherUnitsBasedSensor      = 0x0b,
  eIpmiSensorTypeMemory                     = 0x0c,
  eIpmiSensorTypeDriveSlot                  = 0x0d,
  eIpmiSensorTypePowerMemoryResize          = 0x0e,
  eIpmiSensorTypeSystemFirmwareProgress     = 0x0f,
  eIpmiSensorTypeEventLoggingDisabled       = 0x10,
  eIpmiSensorTypeWatchdog1                  = 0x11,
  eIpmiSensorTypeSystemEvent                = 0x12,
  eIpmiSensorTypeCriticalInterrupt          = 0x13,
  eIpmiSensorTypeButton                     = 0x14,
  eIpmiSensorTypeModuleBoard                = 0x15,
  eIpmiSensorTypeMicrocontrollerCoprocessor = 0x16,
  eIpmiSensorTypeAddInCard		            = 0x17,
  eIpmiSensorTypeChassis                    = 0x18,
  eIpmiSensorTypeChipSet                    = 0x19,
  eIpmiSensorTypeOtherFru                   = 0x1a,
  eIpmiSensorTypeCableInterconnect          = 0x1b,
  eIpmiSensorTypeTerminator                 = 0x1c,
  eIpmiSensorTypeSystemBootInitiated        = 0x1d,
  eIpmiSensorTypeBootError                  = 0x1e,
  eIpmiSensorTypeOsBoot                     = 0x1f,
  eIpmiSensorTypeOsCriticalStop             = 0x20,
  eIpmiSensorTypeSlotConnector              = 0x21,
  eIpmiSensorTypeSystemAcpiPowerState       = 0x22,
  eIpmiSensorTypeWatchdog2                  = 0x23,
  eIpmiSensorTypePlatformAlert              = 0x24,
  eIpmiSensorTypeEntityPresence             = 0x25,
  eIpmiSensorTypeMonitorAsicIc              = 0x26,
  eIpmiSensorTypeLan                        = 0x27,
  eIpmiSensorTypeManagementSubsystemHealth  = 0x28,
  eIpmiSensorTypeBattery                    = 0x29,
  eIpmiSensorTypeOemFirst                   = 0xc0,
  eIpmiSensorTypeOemLast                    = 0xef,
  eIpmiSensorTypeAtcaHotSwap                = 0xf0,
  eIpmiSensorTypeAtcaIpmb                   = 0xf1,
  eIpmiSensorTypeAtcaAmcHotSwap             = 0xf2,
  eIpmiSensorTypeAtcaLast                   = 0xff
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
  eIpmiEventReadingTypeSensorSpecific            = 0x6f,
  eIpmiEventReadingTypeOemFirst                  = 0x70,
  eIpmiEventReadingTypeOemLast                   = 0x7f
};

const char *IpmiEventReadingTypeToString( tIpmiEventReadingType type );


enum tIpmiEventSupport
{
  eIpmiEventSupportPerState     = 0,
  eIpmiEventSupportEntireSensor = 1,
  eIpmiEventSupportGlobalEnable = 2,
  eIpmiEventSupportNone         = 3
};

const char *IpmiEventSupportToString( tIpmiEventSupport val );


enum tIpmiValuePresent
{
  eIpmiNoValuesPresent,
  eIpmiRawValuePresent,
  eIpmiBothValuesPresent
};


#define dSensorIdLen 32


class cIpmiSensor : public cIpmiRdr
{
protected:
  cIpmiMc      *m_source_mc; // If the sensor came from the main SDR,
                             // this will be NULL.  Otherwise, it
                             // will be the MC that owned the device
                             // SDR this came from.

  bool          m_destroyed;
  int           m_use_count;

  unsigned char m_owner;
  unsigned char m_channel;
  unsigned char m_num;

  unsigned int  m_virtual_num; // virtual sensor number

  bool          m_sensor_init_scanning;
  bool          m_sensor_init_events;
  bool          m_sensor_init_type;
  bool          m_sensor_init_pu_events;
  bool          m_sensor_init_pu_scanning;
  bool          m_ignore_if_no_entity;
  bool          m_supports_auto_rearm;

  unsigned int  m_assertion_event_mask;
  unsigned int  m_deassertion_event_mask;
  unsigned int  m_reading_mask;

  SaHpiBoolT    m_enabled;
  SaHpiBoolT    m_events_enabled;
  SaHpiEventStateT    m_current_hpi_assert_mask;
  SaHpiEventStateT    m_current_hpi_deassert_mask;
  SaHpiEventStateT    m_hpi_assert_mask;
  SaHpiEventStateT    m_hpi_deassert_mask;
  SaHpiSensorEventCtrlT m_event_control;

  tIpmiEventSupport m_event_support;

  tIpmiSensorType       m_sensor_type;
  tIpmiEventReadingType m_event_reading_type;

  unsigned int m_oem;

  const char *m_sensor_type_string;
  const char *m_event_reading_type_string;
  const char *m_rate_unit_string;
  const char *m_base_unit_string;
  const char *m_modifier_unit_string;

  cIpmiSdr *m_sdr; // full sensor record or 0

public:
  cIpmiSensor( cIpmiMc *mc );
  virtual ~cIpmiSensor();

  cIpmiMc *&SourceMc() { return m_source_mc; }

  virtual unsigned int Num() const { return m_num; }

  cIpmiSdr *GetSdr() { return m_sdr; }
  void SetSdr( cIpmiSdr *sdr ) { m_sdr = sdr; }

  tIpmiSensorType SensorType() const { return m_sensor_type; }
  tIpmiEventReadingType EventReadingType() const { return m_event_reading_type; }

  bool IgnoreIfNoEntity() const { return m_ignore_if_no_entity; }

  tIpmiEventSupport EventSupport() const { return m_event_support; }

  virtual void HandleNew( cIpmiDomain *domain );

  virtual bool Cmp( const cIpmiSensor &s2 ) const;

  unsigned int GetOem() { return m_oem; }

  // create an HPI event from ipmi event
  virtual SaErrorT CreateEvent( cIpmiEvent *event, SaHpiEventT &h );

  // create and send HPI sensor enable change event
  void CreateEnableChangeEvent();

  // handle all incoming sensor events
  virtual void HandleEvent( cIpmiEvent *event );

  virtual void Dump( cIpmiLog &dump ) const;

  // read sensor parameter from Full Sensor Record
  virtual bool GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr );

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  SaHpiEventCategoryT HpiEventCategory(tIpmiEventReadingType reading_type);
  SaHpiSensorTypeT HpiSensorType(tIpmiSensorType sensor_type);

  // read sensor. must be called with a global read lock held.
  SaErrorT GetSensorData( cIpmiMsg &rsp );

  // get sensor data. this function must called with the global read lock held
  virtual SaErrorT GetSensorReading( SaHpiSensorReadingT &data, SaHpiEventStateT &state ) = 0;

  // this function must called with the global read lock held
  SaErrorT GetEnable( SaHpiBoolT &enable );

  // this function must called with the global read lock held
  SaErrorT GetEventEnables( SaHpiBoolT &enables );

  SaErrorT GetEventEnableHw( SaHpiBoolT &enables );

  // this function must called with the global read lock held
  SaErrorT GetEventMasks( SaHpiEventStateT &AssertEventMask,
                          SaHpiEventStateT &DeassertEventMask
                        );

  // this function must called with the global read lock held
  virtual SaErrorT GetEventMasksHw( SaHpiEventStateT &AssertEventMask,
                                    SaHpiEventStateT &DeassertEventMask
                                  ) = 0;

protected:
  // this function must called with the global read lock held
  virtual SaErrorT GetEventMasksHw( cIpmiMsg &rsp );

public:
  // this function must called with the global read lock held
  SaErrorT SetEnable( const SaHpiBoolT &enable );

  // this function must called with the global read lock held
  SaErrorT SetEventEnables( const SaHpiBoolT &enables );

  SaErrorT SetEventEnableHw( const SaHpiBoolT &enables );

  // this function must called with the global read lock held
  SaErrorT SetEventMasks( const SaHpiSensorEventMaskActionT &act,
                          SaHpiEventStateT &AssertEventMask,
                          SaHpiEventStateT &DeassertEventMask
                        );

  // this function must called with the global read lock held
  virtual SaErrorT SetEventMasksHw( const SaHpiEventStateT &AssertEventMask,
                                    const SaHpiEventStateT &DeassertEventMask
                                  ) = 0;

protected:
  // this function must called with the global read lock held
  virtual SaErrorT SetEventMasksHw( cIpmiMsg &msg,
                                    bool evt_enable );
};


#endif
