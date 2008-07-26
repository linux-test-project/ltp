/*
 * ipmi_event.c
 *
 * Copyright (c) 2003 by FORCE Computers
 * Copyright (c) 2007 by ESO Technologies.
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

#include <string.h>

#include "ipmi_event.h"
#include "ipmi_utils.h"
#include "ipmi_log.h"
#include "ipmi_sensor.h"
#include "ipmi_mc.h"


static const char *thres_map[] =
{
  "LowerNonCritical",
  "LowerCritical",
  "LowerNonRecoverable",
  "UpperNonCritical",
  "UpperCritical",
  "UpperNonRecoverable"
};

static int thres_map_num = sizeof( thres_map ) / sizeof( char * );


const char *
IpmiThresToString( tIpmiThresh val )
{
  if ( val >= thres_map_num )
       return "invalid";

  return thres_map[val];
}


void 
IpmiThresholdMaskToString( unsigned int mask, char *str )
{
  *str = 0;

  for( int i = 0; i < 6; i++ )
       if ( mask & ( 1 << i ) )
          {
            if ( *str )
                 strcat( str, " | " );

            strcat( str, thres_map[i] );
          }
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
cIpmiEvent::Dump( cIpmiLog &dump, const char *name ) const
{
  dump.Begin( "Event", name );
  dump.Entry( "RecordId" ) << m_record_id << ";\n";

  char str[80];

  if ( m_type == 0x02 )
       strcpy( str, "SystemEvent" );
  else
       snprintf( str, sizeof(str), "0x%02x", m_type );

  dump.Entry( "RecordType" ) << str << ";\n";

  unsigned int t = IpmiGetUint32( m_data );
  dump.Hex( true );
  dump.Entry( "Timestamp" ) << t << ";\n";
  dump.Hex( false );

  dump.Entry( "SlaveAddr" ) << m_data[4] << ";\n";
  dump.Entry( "Channel" ) << (m_data[5] >> 4) << ";\n";
  dump.Entry( "Lun" ) << (m_data[5] & 3 ) << ";\n";
  dump.Entry( "Revision" ) << (unsigned int)m_data[6] << ";\n";

  tIpmiSensorType sensor_type = (tIpmiSensorType)m_data[7];

  if ( !strcmp( IpmiSensorTypeToString( sensor_type ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", sensor_type );
  else
       snprintf( str, sizeof(str), "%s", IpmiSensorTypeToString( sensor_type ) );

  dump.Entry( "SensorType" ) << str << ";\n";

  snprintf( str, sizeof(str), "0x%02x", m_data[8] );
  dump.Entry( "SensorNum" ) << str << ";\n";

  dump.Entry( "EventDirection" ) << ((m_data[9] & 0x80) ?
				     "Deassertion" 
				     : "Assertion" )
				 << ";\n";

  tIpmiEventReadingType reading_type = (tIpmiEventReadingType)(m_data[9] & 0x7f);

  if ( !strcmp( IpmiEventReadingTypeToString( reading_type ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", reading_type );
  else
       snprintf( str, sizeof(str), "%s", IpmiEventReadingTypeToString( reading_type ) );

  dump.Entry( "EventReadingType" ) << str << ";\n";

  snprintf( str, sizeof(str), "0x%02x", m_data[10] );
  dump.Entry( "EventData1" ) << str << ";\n";

  snprintf( str, sizeof(str), "0x%02x", m_data[11] );
  dump.Entry( "EventData2" ) << str << ";\n";

  snprintf( str, sizeof(str), "0x%02x", m_data[12] );
  dump.Entry( "EventData3" ) << str << ";\n";

  dump.End();
}
