/*
 * ipmi_sensor_discrete.cpp
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

#include "ipmi_sensor_discrete.h"
#include "ipmi_domain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <math.h>


cIpmiSensorDiscrete::cIpmiSensorDiscrete( cIpmiMc *mc )
  : cIpmiSensor( mc )
{
}


cIpmiSensorDiscrete::~cIpmiSensorDiscrete()
{
}

  
bool
cIpmiSensorDiscrete::GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr )
{
  if ( !cIpmiSensor::GetDataFromSdr( mc, sdr ) )
       return false;

  m_assertion_event_mask = IpmiGetUint16( sdr->m_data + 14 );
  m_assertion_event_mask &= 0x7fff;

  m_current_hpi_assert_mask = m_assertion_event_mask;
  m_hpi_assert_mask = m_assertion_event_mask;

  m_deassertion_event_mask = IpmiGetUint16( sdr->m_data + 16 );
  m_deassertion_event_mask &= 0x7fff;

  m_current_hpi_deassert_mask = m_deassertion_event_mask;
  m_hpi_deassert_mask = m_deassertion_event_mask;

  m_reading_mask = IpmiGetUint16( sdr->m_data + 18 );
  m_reading_mask &= 0x7fff;

  return true;
}


bool
cIpmiSensorDiscrete::CreateRdr( SaHpiRptEntryT &resource,
                                SaHpiRdrT &rdr )
{
  if ( cIpmiSensor::CreateRdr( resource, rdr ) == false )
       return false;

  SaHpiSensorRecT &rec = rdr.RdrTypeUnion.SensorRec;

  rec.DataFormat.IsSupported     = SAHPI_FALSE;
  rec.ThresholdDefn.IsAccessible = SAHPI_FALSE;

  switch (SensorType())
  {
  // Don't want anybody to mess with these sensors
  case eIpmiSensorTypeAtcaHotSwap:
  case eIpmiSensorTypeAtcaIpmb:
  case eIpmiSensorTypeAtcaAmcHotSwap:
      rec.EventCtrl  = SAHPI_SEC_READ_ONLY;
      rec.EnableCtrl = SAHPI_FALSE;
      break;
  default:
      break;
  }

  return true;
}


SaErrorT
cIpmiSensorDiscrete::GetSensorReading( SaHpiSensorReadingT &data,
                                       SaHpiEventStateT &state )
{
  if ( m_enabled == SAHPI_FALSE )
      return SA_ERR_HPI_INVALID_REQUEST;

  cIpmiMsg rsp;
  SaErrorT rv = GetSensorData( rsp );

  if ( rv != SA_OK )
       return rv;

  if ( &data != NULL )
  {
      memset( &data, 0, sizeof( SaHpiSensorReadingT ) );
      data.IsSupported = SAHPI_FALSE;
  }

  if ( &state != NULL )
  {
      // only 15 states
      rsp.m_data[4] &= 0x7f;

      state = IpmiGetUint16( rsp.m_data + 3 );
  }

  return SA_OK;
}


SaErrorT
cIpmiSensorDiscrete::GetEventMasksHw( SaHpiEventStateT &AssertEventMask,
                                      SaHpiEventStateT &DeassertEventMask
                                    )
{
  cIpmiMsg rsp;
  SaErrorT rv = cIpmiSensor::GetEventMasksHw( rsp );

  if ( rv != SA_OK )
       return rv;

  AssertEventMask   = IpmiGetUint16( rsp.m_data + 2 );
  DeassertEventMask = IpmiGetUint16( rsp.m_data + 4 );

  return SA_OK;
}


SaErrorT
cIpmiSensorDiscrete::SetEventMasksHw( const SaHpiEventStateT &AssertEventMask,
                                      const SaHpiEventStateT &DeassertEventMask
                                    )
{
  // create de/assertion event mask
  unsigned int amask;
  unsigned int dmask;

  amask = AssertEventMask;
  
  dmask = DeassertEventMask;

  cIpmiMsg msg;
  SaErrorT rv = SA_OK;

  if (( amask != 0 )
      || ( dmask != 0 ))
  {
    IpmiSetUint16( msg.m_data + 2, amask );
    IpmiSetUint16( msg.m_data + 4, dmask );

    rv = cIpmiSensor::SetEventMasksHw( msg, true );
  }

  if ( rv != SA_OK )
      return rv;

  amask = ( amask ^ m_assertion_event_mask ) & m_assertion_event_mask;
  dmask = ( dmask ^ m_deassertion_event_mask ) & m_deassertion_event_mask;

  if (( amask != 0 )
      || ( dmask != 0 ))
  {
    IpmiSetUint16( msg.m_data + 2, amask );
    IpmiSetUint16( msg.m_data + 4, dmask );

    rv = cIpmiSensor::SetEventMasksHw( msg, false );
  }

  return rv;
}


SaErrorT
cIpmiSensorDiscrete::CreateEvent( cIpmiEvent *event, SaHpiEventT &h )
{
  SaErrorT rv = cIpmiSensor::CreateEvent( event, h );

  if ( rv != SA_OK )
       return rv;

  // sensor event
  SaHpiSensorEventT &se = h.EventDataUnion.SensorEvent;

  se.Assertion = (SaHpiBoolT)!(event->m_data[9] & 0x80);

  se.EventState = (1 << (event->m_data[10] & 0x0f));

  // default value
  h.Severity = SAHPI_INFORMATIONAL;

  SaHpiSensorOptionalDataT optional_data = 0;

  // byte 2
  tIpmiEventType type = (tIpmiEventType)(event->m_data[10] >> 6);

  if ( type == eIpmiEventData1 )
  {
      if ((event->m_data[11] & 0x0f) != 0x0f)
      {
          se.PreviousState = (1 << (event->m_data[11] & 0x0f));
          optional_data |= SAHPI_SOD_PREVIOUS_STATE;
      }
      if ((event->m_data[11] & 0xf0) != 0xf0)
      {
          SaHpiEventStateT evt_sec_state = (1 << ((event->m_data[11]>> 4) & 0x0f));
          switch (evt_sec_state)
          {
          case SAHPI_ES_OK:
              h.Severity = SAHPI_OK;
              break;
          case SAHPI_ES_MINOR_FROM_OK:
              h.Severity = SAHPI_MINOR;
              break;
          case SAHPI_ES_MAJOR_FROM_LESS:
              h.Severity = SAHPI_MAJOR;
              break;
          case SAHPI_ES_CRITICAL_FROM_LESS:
              h.Severity = SAHPI_CRITICAL;
              break;
          case SAHPI_ES_MINOR_FROM_MORE:
              h.Severity = SAHPI_MINOR;
              break;
          case SAHPI_ES_MAJOR_FROM_CRITICAL:
              h.Severity = SAHPI_MAJOR;
              break;
          case SAHPI_ES_CRITICAL:
              h.Severity = SAHPI_CRITICAL;
              break;
          case SAHPI_ES_MONITOR:
              h.Severity = SAHPI_INFORMATIONAL;
              break;
          case SAHPI_ES_INFORMATIONAL:
              h.Severity = SAHPI_INFORMATIONAL;
              break;
          }
      }
  }
  else if ( type == eIpmiEventData2 )
  {
       se.Oem = (SaHpiUint32T)event->m_data[11]; 
       optional_data |= SAHPI_SOD_OEM;
  }
  else if ( type == eIpmiEventData3 )
  {
       se.SensorSpecific = (SaHpiUint32T)event->m_data[11]; 
       optional_data |= SAHPI_SOD_SENSOR_SPECIFIC;
  }

  // byte 3
  type = (tIpmiEventType)((event->m_data[10] & 0x30) >> 4);

  if ( type == eIpmiEventData2 )
  {
       se.Oem |= (SaHpiUint32T)((event->m_data[12] << 8) & 0xff00);
       optional_data |= SAHPI_SOD_OEM;
  }
  else if ( type == eIpmiEventData3 )
  {
       se.SensorSpecific |= (SaHpiUint32T)((event->m_data[12] << 8) & 0xff00);
       optional_data |= SAHPI_SOD_SENSOR_SPECIFIC;
  }

  se.OptionalDataPresent = optional_data;


  return SA_OK;
}
