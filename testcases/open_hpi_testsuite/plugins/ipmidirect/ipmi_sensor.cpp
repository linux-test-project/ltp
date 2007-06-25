/*
 * ipmi_sensor.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <glib.h>
#include <math.h>

#include "ipmi_domain.h"
#include "ipmi_mc.h"
#include "ipmi_sensor.h"
#include "ipmi_entity.h"
#include "ipmi_utils.h"
#include "ipmi_text_buffer.h"


static const char *sensor_types[] =
{
  "Unspecified",
  "Temperature",
  "Voltage",
  "Current",
  "Fan",
  "PhysicalSecurity",
  "PlatformSecurity",
  "Processor",
  "PowerSupply",
  "PowerUnit",
  "CoolingDevice",
  "OtherUnitsBasedSensor",
  "Memory",
  "DriveSlot",
  "PowerMemoryResize",
  "SystemFirmwareProgress",
  "EventLoggingDisabled",
  "Watchdog1",
  "SystemEvent",
  "CriticalInterrupt",
  "Button",
  "ModuleBoard",
  "MicrocontrollerCoprocessor",
  "AddInCard",
  "Chassis",
  "ChipSet",
  "OtherFru",
  "CableInterconnect",
  "Terminator",
  "SystemBootInitiated",
  "BootError",
  "OsBoot",
  "OsCriticalStop",
  "SlotConnector",
  "SystemAcpiPowerState",
  "Watchdog2",
  "PlatformAlert",
  "EntityPresense",
  "MonitorAsicIc",
  "Lan",
  "ManagementSubsystemHealth",
  "Battery"
};


const char *
IpmiSensorTypeToString( tIpmiSensorType val )
{
  if ( val > eIpmiSensorTypeBattery )
     {
       if ( val == eIpmiSensorTypeAtcaHotSwap )
            return "AtcaHotswap";

       if ( val == eIpmiSensorTypeAtcaIpmb )
           return "AtcaIpmb";

       return "Invalid";
     }

  return sensor_types[val];
}


static const char *event_support_types[] =
{
    "PerState",
    "EntireSensor",
    "GlobalDisable",
    "None",
};


const char *
IpmiEventSupportToString( tIpmiEventSupport val )
{
  if ( val > eIpmiEventSupportNone )
       return "Invalid";

  return event_support_types[val];
}


static const char *event_reading_types[] =
{
  "Unspecified",
  "Threshold",
  "DiscreteUsage",
  "DiscreteState",
  "DiscretePredictiveFailure",
  "DiscreteLimitExceeded",
  "DiscretePerformanceMet",
  "DiscreteSeverity",
  "DiscreteDevicePresense",
  "DiscreteDeviceEnable",
  "DiscreteAvailability",
  "DiscreteRedundancy",
  "DiscreteAcpiPower",
};


const char *
IpmiEventReadingTypeToString( tIpmiEventReadingType val )
{
  if ( val == eIpmiEventReadingTypeSensorSpecific )
       return "SensorSpecific";

  if (( val >= eIpmiEventReadingTypeOemFirst )
      && ( val <= eIpmiEventReadingTypeOemLast ))
      return "Oem";

  if ( val > eIpmiEventReadingTypeDiscreteAcpiPower )
       return "Invalid";

  return event_reading_types[val];
}


cIpmiSensor::cIpmiSensor( cIpmiMc *mc )
  : cIpmiRdr( mc, SAHPI_SENSOR_RDR ), m_source_mc( 0 ),
    m_destroyed( false ),
    m_use_count( 0 ),
    m_owner( 0 ), m_channel( 0 ),
    m_num( 0 ),
    m_sensor_init_scanning( false ),
    m_sensor_init_events( false ),
    m_sensor_init_type( false ),
    m_sensor_init_pu_events( false ),
    m_sensor_init_pu_scanning( false ),
    m_ignore_if_no_entity( false ),
    m_supports_auto_rearm( false ),
    m_assertion_event_mask( 0 ),
    m_deassertion_event_mask( 0 ),
    m_reading_mask( 0 ),
    m_enabled( SAHPI_TRUE ),
    m_event_support( eIpmiEventSupportPerState ),
    m_sensor_type( eIpmiSensorTypeInvalid ),
    m_event_reading_type( eIpmiEventReadingTypeInvalid ),
    m_oem( 0 ),
    m_sensor_type_string( 0 ),
    m_event_reading_type_string( 0 ),
    m_rate_unit_string( 0 ),
    m_base_unit_string( 0 ),
    m_modifier_unit_string( 0 ),
    m_sdr( 0 )
{
}


cIpmiSensor::~cIpmiSensor()
{
}


bool
cIpmiSensor::GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr )
{
  m_use_count = 1;
  m_destroyed = false;

  m_mc = mc;

  m_source_mc = mc;
  m_owner                   = sdr->m_data[5];
  m_channel                 = sdr->m_data[6] >> 4;
  m_lun                     = sdr->m_data[6] & 0x03;
  m_num                     = sdr->m_data[7];
  m_sensor_init_scanning    = (sdr->m_data[10] >> 6) & 1;
  m_sensor_init_events      = (sdr->m_data[10] >> 5) & 1;
  if ( m_sensor_init_events )
      m_events_enabled = SAHPI_TRUE;
  else
      m_events_enabled = SAHPI_FALSE;
  m_sensor_init_type        = (sdr->m_data[10] >> 2) & 1;
  m_sensor_init_pu_events   = (sdr->m_data[10] >> 1) & 1;
  m_sensor_init_pu_scanning = (sdr->m_data[10] >> 0) & 1;
  m_ignore_if_no_entity     = (sdr->m_data[11] >> 7) & 1;
  m_supports_auto_rearm     = (sdr->m_data[11] >> 6) & 1;
  m_event_support           = (tIpmiEventSupport)(sdr->m_data[11] & 3);
  m_sensor_type             = (tIpmiSensorType)sdr->m_data[12];
  m_event_reading_type      = (tIpmiEventReadingType)(sdr->m_data[13] & 0x7f);

  m_oem                     = sdr->m_data[46];

  IdString().SetIpmi( sdr->m_data+47 );

  if ( m_owner != mc->GetAddress() )
      stdlog << "WARNING : SDR " << sdr->m_record_id << " sensor " << m_num << " slave address " << m_owner << " NOT equal to MC slave address " << (unsigned char)mc->GetAddress() << "\n";

  if ( m_channel != mc->GetChannel() )
      stdlog << "WARNING : SDR " << sdr->m_record_id << " sensor " << m_num << " channel " << m_channel << " NOT equal to MC channel " << (unsigned short)mc->GetChannel() << "\n";

  return true;
}


void
cIpmiSensor::HandleNew( cIpmiDomain *domain )
{
  m_sensor_type_string        = IpmiSensorTypeToString( m_sensor_type );
  m_event_reading_type_string = IpmiEventReadingTypeToString( m_event_reading_type );
}


bool
cIpmiSensor::Cmp( const cIpmiSensor &s2 ) const
{
  if ( m_entity_path != s2.m_entity_path )
       return false;

  if ( m_sensor_init_scanning    != s2.m_sensor_init_scanning )
       return false;

  if ( m_sensor_init_events      != s2.m_sensor_init_events )
       return false;

  if ( m_sensor_init_type        != s2.m_sensor_init_type )
       return false;

  if ( m_sensor_init_pu_events   != s2.m_sensor_init_pu_events )
       return false;

  if ( m_sensor_init_pu_scanning != s2.m_sensor_init_pu_scanning )
       return false;

  if ( m_ignore_if_no_entity     != s2.m_ignore_if_no_entity )
       return false;

  if ( m_supports_auto_rearm     != s2.m_supports_auto_rearm )
       return false;

  if ( m_event_support           != s2.m_event_support )
       return false;

  if ( m_sensor_type             != s2.m_sensor_type )
       return false;

  if ( m_event_reading_type      != s2.m_event_reading_type )
       return false;

  if ( m_oem != s2.m_oem )
       return false;

  if ( IdString() != s2.IdString() )
       return false;

  return true;
}


void
cIpmiSensor::Dump( cIpmiLog &dump ) const
{
  char str[256];
  IdString().GetAscii( str, 256 );

  dump << "Sensor: " << m_num << " " << str << "\n";
}


bool
cIpmiSensor::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  if ( cIpmiRdr::CreateRdr( resource, rdr ) == false )
       return false;

  // update resource
  resource.ResourceCapabilities |= SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_SENSOR;

  // sensor record
  SaHpiSensorRecT &rec = rdr.RdrTypeUnion.SensorRec;

  int v = Resource()->CreateSensorNum( Num() );

  if ( v == -1 )
     {
       stdlog << "too many sensors (> 255) for a resource !\n";

       assert( v != -1 );
       return false;
     }

  m_virtual_num = v;

  rec.Num       = v;
  rec.Type      = HpiSensorType(SensorType());

  rec.Category  = HpiEventCategory(EventReadingType());
  rec.Oem       = GetOem();

  switch( EventSupport() )
  {
  case eIpmiEventSupportPerState:
      m_event_control = SAHPI_SEC_PER_EVENT;
      break;
  case eIpmiEventSupportEntireSensor:
  case eIpmiEventSupportGlobalEnable:
      m_event_control  = SAHPI_SEC_READ_ONLY_MASKS;
      break;
  case eIpmiEventSupportNone:
      m_event_control  = SAHPI_SEC_READ_ONLY;
      break;
  }

  rec.Events     = m_reading_mask;
  rec.EventCtrl  = m_event_control;
  rec.EnableCtrl = SAHPI_TRUE;

  return true;
}


SaErrorT
cIpmiSensor::GetSensorData( cIpmiMsg &rsp )
{
  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorReading );
  msg.m_data_len = 1;
  msg.m_data[0]  = m_num;

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "IPMI error getting states: " << rv << " \n";

       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       stdlog << "IPMI error getting reading: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  if ( rsp.m_data_len < 4 )
     {
       stdlog << "IPMI error getting reading: data to small "
              << rsp.m_data_len << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  return SA_OK;
}

SaErrorT
cIpmiSensor::GetEnable( SaHpiBoolT &enable )
{
    enable = m_enabled;

    return SA_OK;
}

SaErrorT
cIpmiSensor::SetEnable( const SaHpiBoolT &enable )
{
    if (m_enabled == enable)
        return SA_OK;

    m_enabled = enable;

    CreateEnableChangeEvent();

    return SA_OK;
}

SaErrorT
cIpmiSensor::GetEventEnables( SaHpiBoolT &enables )
{
    SaErrorT rv = GetEventEnableHw( m_events_enabled );

    enables = m_events_enabled;

    return rv;
}

SaErrorT
cIpmiSensor::SetEventEnables( const SaHpiBoolT &enables )
{
    if ( m_event_control == SAHPI_SEC_READ_ONLY )
        return SA_ERR_HPI_READ_ONLY;

    if ( m_events_enabled == enables )
        return SA_OK;

    m_events_enabled = enables;

    SaErrorT rv = SetEventEnableHw( m_events_enabled );

    if ( rv == SA_OK )
        CreateEnableChangeEvent();

    return rv;
}

SaErrorT
cIpmiSensor::GetEventMasks( SaHpiEventStateT &AssertEventMask,
                            SaHpiEventStateT &DeassertEventMask
                          )
{
    SaErrorT rv = GetEventMasksHw( m_current_hpi_assert_mask,
                                   m_current_hpi_deassert_mask
                                 );

    stdlog << "GetEventMasks sensor " << m_num << " assert " << m_current_hpi_assert_mask << " deassert " << m_current_hpi_deassert_mask << "\n";

    if (&AssertEventMask) AssertEventMask = m_current_hpi_assert_mask;
    if (&DeassertEventMask) DeassertEventMask = m_current_hpi_deassert_mask;

    return rv;
}

SaErrorT
cIpmiSensor::SetEventMasks( const SaHpiSensorEventMaskActionT &act,
                            SaHpiEventStateT            &AssertEventMask,
                            SaHpiEventStateT            &DeassertEventMask
                          )
{
    if ( m_event_control != SAHPI_SEC_PER_EVENT )
        return SA_ERR_HPI_READ_ONLY;

    if ( AssertEventMask == SAHPI_ALL_EVENT_STATES )
        AssertEventMask = m_hpi_assert_mask;

    if ( DeassertEventMask == SAHPI_ALL_EVENT_STATES )
        DeassertEventMask = m_hpi_deassert_mask;

    if ( act == SAHPI_SENS_ADD_EVENTS_TO_MASKS )
    {
        if ((( AssertEventMask & ~m_hpi_assert_mask ) != 0 )
            || (( DeassertEventMask & ~m_hpi_deassert_mask ) != 0 ))
            return SA_ERR_HPI_INVALID_DATA;
    }

    SaHpiEventStateT save_assert_mask = m_current_hpi_assert_mask;
    SaHpiEventStateT save_deassert_mask = m_current_hpi_deassert_mask;

    if ( act == SAHPI_SENS_ADD_EVENTS_TO_MASKS )
        {
            m_current_hpi_assert_mask   |= AssertEventMask;
            m_current_hpi_deassert_mask |= DeassertEventMask;
        }
    else if ( act == SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS )
        {
            m_current_hpi_assert_mask   &= (AssertEventMask ^ SAHPI_ALL_EVENT_STATES);
            m_current_hpi_deassert_mask &= (DeassertEventMask ^ SAHPI_ALL_EVENT_STATES);
        }
    else 
        return SA_ERR_HPI_INVALID_PARAMS;
    
    stdlog << "SetEventMasks sensor " << m_num << " assert " << m_current_hpi_assert_mask << " deassert " << m_current_hpi_deassert_mask << "\n";

    if (( save_assert_mask == m_current_hpi_assert_mask )
        && ( save_deassert_mask == m_current_hpi_deassert_mask ))
        return SA_OK;

    SaErrorT rv = SetEventMasksHw( m_current_hpi_assert_mask,
                                   m_current_hpi_deassert_mask
                                 );

    if ( rv == SA_OK )
        CreateEnableChangeEvent();

    return rv;
}

SaErrorT
cIpmiSensor::GetEventEnableHw( SaHpiBoolT &enables )
{
  cIpmiMsg  msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorEventEnable );
  msg.m_data_len = 1;
  msg.m_data[0]  = m_num;

  cIpmiMsg rsp;

  stdlog << "get event enables command for sensor : " << m_num << " !\n";

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending get event enables command: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error getting sensor enables: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
    }

  enables = (rsp.m_data[1] & 0x80) ? SAHPI_TRUE : SAHPI_FALSE;

  return SA_OK;
}

SaErrorT
cIpmiSensor::SetEventEnableHw( const SaHpiBoolT &enables )
{
  cIpmiMsg msg;
  msg.m_netfn = eIpmiNetfnSensorEvent;
  msg.m_cmd   = eIpmiCmdSetSensorEventEnable;

  msg.m_data[0] = m_num;
  msg.m_data[1] = ((m_events_enabled == SAHPI_TRUE) ? 0x80 : 0) | (1<<6);

  msg.m_data_len = 2;

  cIpmiMsg rsp;

  stdlog << "set event enables command for sensor : " << m_num << " !\n";

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending set event enables command: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error setting sensor enables: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}


SaErrorT
cIpmiSensor::GetEventMasksHw( cIpmiMsg &rsp )
{
  cIpmiMsg  msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorEventEnable );
  msg.m_data_len = 1;
  msg.m_data[0]  = m_num;

  stdlog << "get event enables command for sensor : " << m_num << " !\n";

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending get event enables command: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error getting sensor enables: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
    }

  return SA_OK;
}


SaErrorT
cIpmiSensor::SetEventMasksHw( cIpmiMsg &msg,
                              bool evt_enable)
{
  msg.m_netfn = eIpmiNetfnSensorEvent;
  msg.m_cmd   = eIpmiCmdSetSensorEventEnable;

  msg.m_data[0] = m_num;
  msg.m_data[1] = ((m_events_enabled == SAHPI_TRUE) ? 0x80 : 0) | (1<<6);

  if ( m_event_support == eIpmiEventSupportEntireSensor )
     {
       // We can only turn on/off the entire sensor, just pass the
       // status to the sensor.
       msg.m_data_len = 2;
     }
  else
     {
       if ( evt_enable == true ) 
            msg.m_data[1] |= (1<<4); // enable selected event messages
       else
            msg.m_data[1] |= (1<<5); // disable selected event messages
       msg.m_data_len = 6;
     }

  cIpmiMsg rsp;

  stdlog << "set event enables command for sensor : " << m_num << " !\n";

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending set event enables command: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error setting sensor enables: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}

SaHpiSensorTypeT
cIpmiSensor::HpiSensorType(tIpmiSensorType sensor_type)
{
  if ( sensor_type >= eIpmiSensorTypeOemFirst)
      return SAHPI_OEM_SENSOR;

  return (SaHpiSensorTypeT)sensor_type;
}

SaHpiEventCategoryT
cIpmiSensor::HpiEventCategory(tIpmiEventReadingType reading_type)
{
  if ( reading_type == eIpmiEventReadingTypeSensorSpecific )
      return SAHPI_EC_SENSOR_SPECIFIC;
  
  if (( reading_type >= eIpmiEventReadingTypeOemFirst )
      && ( reading_type <= eIpmiEventReadingTypeOemLast ))
      return SAHPI_EC_GENERIC;

  return (SaHpiEventCategoryT)reading_type;
}

void
cIpmiSensor::CreateEnableChangeEvent()
{
  cIpmiResource *res = Resource();
  if( !res )
     {
       stdlog << "CreateEnableChangeEvent: No resource !\n";
       return;
     }

  oh_event *e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );
  
  e->event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;

  SaHpiRptEntryT *rptentry = oh_get_resource_by_id( res->Domain()->GetHandler()->rptcache, res->m_resource_id );
  SaHpiRdrT *rdrentry = oh_get_rdr_by_id( res->Domain()->GetHandler()->rptcache, res->m_resource_id, m_record_id );

  if ( rptentry )
      e->resource = *rptentry;
  else
      e->resource.ResourceCapabilities = 0;

  if ( rdrentry )
      e->rdrs = g_slist_append(e->rdrs, g_memdup(rdrentry, sizeof(SaHpiRdrT)));
  else
      e->rdrs = NULL;

  // hpi event
  e->event.Source    = res->m_resource_id;
  e->event.EventType = SAHPI_ET_SENSOR_ENABLE_CHANGE;
  e->event.Severity  = SAHPI_INFORMATIONAL;
  
  oh_gettimeofday(&e->event.Timestamp);
  
  // sensor enable event
  SaHpiSensorEnableChangeEventT *se = &e->event.EventDataUnion.SensorEnableChangeEvent;
  se->SensorNum     = m_num;
  se->SensorType    = HpiSensorType(SensorType());
  se->EventCategory = HpiEventCategory(EventReadingType());
  se->SensorEnable  = m_enabled;
  se->SensorEventEnable = m_events_enabled;
  se->AssertEventMask   = m_current_hpi_assert_mask;
  se->DeassertEventMask = m_current_hpi_deassert_mask;
  
  stdlog << "cIpmiSensor::CreateEnableChangeEvent OH_ET_HPI Event enable change resource " << res->m_resource_id << "\n";
  m_mc->Domain()->AddHpiEvent( e );

  return;
}


SaErrorT
cIpmiSensor::CreateEvent( cIpmiEvent *event, SaHpiEventT &h )
{
  memset( &h, 0, sizeof( SaHpiEventT ) );

  cIpmiResource *res = Resource();
  if( !res )
     {
       stdlog << "CreateEvent: No resource !\n";
       return SA_ERR_HPI_NOT_PRESENT;
     }

  h.Source    = res->m_resource_id;
  h.EventType = SAHPI_ET_SENSOR;
  h.Timestamp = (SaHpiTimeT)IpmiGetUint32( event->m_data );

  if ( h.Timestamp == 0 )
       h.Timestamp = SAHPI_TIME_UNSPECIFIED;
  else
       h.Timestamp *= 1000000000;

  // sensor event
  SaHpiSensorEventT &s = h.EventDataUnion.SensorEvent;
  s.SensorNum     = m_num;
  s.SensorType    = HpiSensorType((tIpmiSensorType)event->m_data[7]);

  tIpmiEventReadingType reading_type = (tIpmiEventReadingType)(event->m_data[9] & 0x7f);

  s.EventCategory = HpiEventCategory(reading_type);
  
  return SA_OK;
}


void
cIpmiSensor::HandleEvent( cIpmiEvent *event )
{
  cIpmiResource *res = Resource();

  if( !res )
     {
       stdlog << "HandleEvent: No resource !\n";
       return;
     }

  SaHpiRptEntryT *rptentry;
  SaHpiRdrT *rdrentry;

  // Sensor disabled -> ignore event
  if (m_enabled == SAHPI_FALSE)
    {
      stdlog << "reading event : Ignore (Sensor disabled).\n";
      return;
    }

  stdlog << "reading event.\n";

  oh_event *e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  rptentry = oh_get_resource_by_id( res->Domain()->GetHandler()->rptcache, res->m_resource_id );
  rdrentry = oh_get_rdr_by_id( res->Domain()->GetHandler()->rptcache, res->m_resource_id, m_record_id );

  if ( rptentry )
      e->resource = *rptentry;
  else
      e->resource.ResourceCapabilities = 0;

  if ( rdrentry )
      e->rdrs = g_slist_append(e->rdrs, g_memdup(rdrentry, sizeof(SaHpiRdrT)));
  else
      e->rdrs = NULL;

  // hpi event

  SaHpiEventT &he = e->event;
  int rv = CreateEvent( event, he );

  if ( rv != SA_OK )
       return;

  stdlog << "cIpmiSensor::HandleEvent OH_ET_HPI Event resource " << res->m_resource_id << "\n";
  m_mc->Domain()->AddHpiEvent( e );
}
