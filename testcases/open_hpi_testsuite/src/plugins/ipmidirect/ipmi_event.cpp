/*
 * ipmi_event.c
 *
 * Copyright (c) 2003 by FORCE Computers
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

#include <string.h>
#include <assert.h>

#include "ipmi_event.h"
#include "ipmi_utils.h"
#include "ipmi_log.h"
#include "ipmi_sensor.h"
#include "ipmi_mc.h"


cIpmiEventState::cIpmiEventState()
  : m_status( 0 ), m_assertion_events( 0 ), m_deassertion_events( 0 )
{
}


void
cIpmiEventState::SetEventsEnabled( bool val )
{
  if ( val )
       m_status |= dIpmiSensorEventsEnabled;
  else
       m_status &= ~dIpmiSensorEventsEnabled;
}


bool
cIpmiEventState::GetEventsEnabled() const
{
  return (m_status >> 7) & 1;
}


void
cIpmiEventState::SetScanningEnabled( bool val )
{
  if ( val )
       m_status |= dIpmiSensorScanningEnabled;
  else
       m_status &= ~dIpmiSensorScanningEnabled;
}


bool
cIpmiEventState::GetScanningEnabled() const
{
  return ( m_status >> 6) & 1;
}


void
cIpmiEventState::SetBusy( bool val )
{
  if ( val )
       m_status |= dIpmiSensorBusy;
  else
       m_status &= ~dIpmiSensorBusy;
}


bool
cIpmiEventState::GetBusy() const
{
  return ( m_status >> 5) & 1;
}


void
cIpmiEventState::ThresholdEventClear( tIpmiThresh type,
                                      tIpmiEventValueDir value_dir,
                                      tIpmiEventDir      dir )
{
  if ( dir == eIpmiAssertion )
       m_assertion_events   &= ~(1 << (type*2+value_dir)); 
  else
       m_deassertion_events &= ~(1 << (type*2+value_dir));
}


void
cIpmiEventState::ThresholdEventSet( tIpmiThresh type,
                                    tIpmiEventValueDir value_dir,
                                    tIpmiEventDir      dir )
{
  if ( dir == eIpmiAssertion )
       m_assertion_events   |= 1 << (type*2+value_dir); 
  else 
       m_deassertion_events |= 1 << (type*2+value_dir);
}


bool
cIpmiEventState::IsThresholdEventSet( tIpmiThresh        type,
                                      tIpmiEventValueDir value_dir,
                                      tIpmiEventDir      dir ) const
{
  if ( dir == eIpmiAssertion )
       return (m_assertion_events & (1 << (type*2+value_dir))) != 0;

  return (m_deassertion_events & (1 << (type*2+value_dir))) != 0;
}


void
cIpmiEventState::DiscreteEventClear( int           event_offset,
                                     tIpmiEventDir dir )
{
  if ( dir == eIpmiAssertion )
       m_assertion_events   &= ~(1 << event_offset);
  else 
       m_deassertion_events &= ~(1 << event_offset);
}


void
cIpmiEventState::DiscreteEventSet( int           event_offset,
                                   tIpmiEventDir dir )
{
  if ( dir == eIpmiAssertion )
       m_assertion_events   |= 1 << event_offset;
  else
       m_deassertion_events |= 1 << event_offset;
}


bool
cIpmiEventState::IsDiscreteEventSet( int           event_offset,
                                     tIpmiEventDir dir ) const
{
  if ( dir == eIpmiAssertion )
       return (m_assertion_events & (1 << event_offset)) != 0;

  return (m_deassertion_events & (1 << event_offset)) != 0;
}


cIpmiEvent::cIpmiEvent()
  : m_mc( 0 ), m_record_id( 0 ), m_type( 0 )
{
  memset( m_data, 0, dIpmiMaxSelData );
}


int
cIpmiEvent::Cmp( const cIpmiEvent &event2 ) const
{
  //  if ( event1->mc != event2->mc )
  //       return 1;

  if ( m_record_id > event2.m_record_id )
       return 1;

  if ( m_record_id < event2.m_record_id )
       return -1;

  if ( m_type > event2.m_type )
	return 1;

  if ( m_type < event2.m_type )
       return -1;

  return memcmp( m_data, event2.m_data, 13 );
}


void
cIpmiEvent::Log() const
{
  assert( m_mc );

  unsigned int time = IpmiGetUint32( m_data );
  char ts[dDateTimeStringSize];

  IpmiDateTimeToString( time, ts );

  IpmiLog( "mc 0x%02x, id %d, type %s, time %s, slave addr 0x%02x, rev 0x%02x, sensor type %s, sensor id 0x%02x, dir %s, event type %s, 0x%02x, 0x%02x, 0x%02x\n",
           m_mc->GetAddress(), m_record_id,
           m_type == 0x02 ? "system event" : "OEM", ts, m_data[4], m_data[6],
           IpmiSensorTypeToString( (tIpmiSensorType)( m_data[7]) ),
           m_data[8], (m_data[9] >> 7) ? "deassertion" : "assertion",
           IpmiEventReadingTypeToString( (tIpmiEventReadingType)( m_data[9] & 0x7f) ),
           m_data[10], m_data[11], m_data[12] );
}
