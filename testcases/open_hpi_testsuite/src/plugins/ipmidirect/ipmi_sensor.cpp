/*
 * ipmi_sensor.cpp
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
#include "ipmi_type_code.h"


cIpmiSensorInfo::cIpmiSensorInfo( cIpmiMc *mc )
  : m_mc( mc ), m_destroyed( false ), 
    m_sensor_count( 0 )
{
  for( int i = 0; i < 5; i++ )
     {
       m_sensors_by_idx[i] = NULL;
       m_idx_size[i] = 0;
     }
}


cIpmiSensorInfo::~cIpmiSensorInfo()
{
  for( int i = 0; i <= 4; i++ )
     {
       for( int j = 0; j < m_idx_size[i]; j++ )
	    if ( m_sensors_by_idx[i][j] )
                 m_sensors_by_idx[i][j]->Destroy();

       if ( m_sensors_by_idx[i] )
	    delete [] m_sensors_by_idx[i];
     }
}


cIpmiSensor::cIpmiSensor( cIpmiMc *mc )
  : m_mc( mc ), m_source_mc( 0 ),
    m_source_idx( 0 ), m_source_array( 0 ),
    m_event_state( 0 ), m_destroyed( false ),
    m_use_count( 0 ),  
    m_owner( 0 ), m_channel( 0 ),
    m_lun( 0 ), m_num( 0 ),
    m_entity_id( eIpmiEntityInvalid ), m_entity_instance( 0 ),
    m_entity_instance_logical( false ),
    m_sensor_init_scanning( false ),
    m_sensor_init_events( false ),
    m_sensor_init_thresholds( false ),
    m_sensor_init_hysteresis( false ),
    m_sensor_init_type( false ),
    m_sensor_init_pu_events( false ),
    m_sensor_init_pu_scanning( false ),
    m_ignore_if_no_entity( false ),
    m_supports_auto_rearm( false ),
    m_hysteresis_support( eIpmiHysteresisSupportNone ),
    m_threshold_access( eIpmiThresholdAccessSupportNone ),
    m_event_support( eIpmiEventSupportPerState ),
    m_hot_swap_requester( 0 ), m_hot_swap_requester_val( 0 ),
    m_sensor_type( eIpmiSensorTypeInvalid ),
    m_event_reading_type( eIpmiEventReadingTypeInvalid ),
    m_analog_data_format( eIpmiAnalogDataFormatUnsigned ),
    m_rate_unit( eIpmRateUnitNone ),
    m_modifier_unit_use( eIpmiModifierUnitNone ),
    m_percentage( false ),
    m_base_unit( eIpmiUnitTypeUnspecified ),
    m_modifier_unit( eIpmiUnitTypeUnspecified ),
    m_linearization( eIpmiLinearizationLinear ),
    m_normal_min_specified( false ),
    m_normal_max_specified( false ),
    m_nominal_reading_specified( false ),
    m_nominal_reading( 0 ),
    m_normal_max( 0 ),
    m_normal_min( 0 ),
    m_sensor_max( 0 ),
    m_sensor_min( 0 ),
    m_upper_non_recoverable_threshold( 0 ),
    m_upper_critical_threshold( 0 ),
    m_upper_non_critical_threshold( 0 ),
    m_lower_non_recoverable_threshold( 0 ),
    m_lower_critical_threshold( 0 ),
    m_lower_non_critical_threshold( 0 ),
    m_positive_going_threshold_hysteresis( 0 ),
    m_negative_going_threshold_hysteresis( 0 ),
    m_oem1( 0 ),
    m_sensor_type_string( 0 ),
    m_event_reading_type_string( 0 ),
    m_rate_unit_string( 0 ),
    m_base_unit_string( 0 ),
    m_modifier_unit_string( 0 )
{
  for( int i = 0; i < 16; i++ )
     {
       m_mask1[i] = 0;
       m_mask2[i] = 0;
       m_mask3[i] = 0;
     }

  m_m = 0;
  m_tolerance = 0;
  m_b = 0;
  m_r_exp = 0;
  m_accuracy_exp = 0;
  m_accuracy = 0;
  m_b_exp = 0;

  m_id[0] = 0;
}


cIpmiSensor::~cIpmiSensor()
{
}


void
cIpmiSensor::HandleNew( cIpmiDomain *domain )
{
  m_sensor_type_string        = IpmiSensorTypeToString( m_sensor_type );
  m_event_reading_type_string = IpmiEventReadingTypeToString( m_event_reading_type );
  m_rate_unit_string          = IpmiRateUnitToString( m_rate_unit );
  m_base_unit_string          = IpmiUnitTypeToString( m_base_unit );
  m_modifier_unit_string      = IpmiUnitTypeToString( m_modifier_unit );

  cIpmiEntityInfo &ents = domain->Entities();
  cIpmiEntity *ent = ents.Find( m_mc, m_entity_id, m_entity_instance );

  ent->AddSensor( this );
}


bool
cIpmiSensor::Cmp( const cIpmiSensor &s2 ) const
{
  int i;

  if ( m_entity_instance_logical != s2.m_entity_instance_logical ) return false;
  if ( m_sensor_init_scanning    != s2.m_sensor_init_scanning    ) return false;
  if ( m_sensor_init_events      != s2.m_sensor_init_events      ) return false;
  if ( m_sensor_init_thresholds  != s2.m_sensor_init_thresholds  ) return false;
  if ( m_sensor_init_hysteresis  != s2.m_sensor_init_hysteresis  ) return false;
  if ( m_sensor_init_type        != s2.m_sensor_init_type        ) return false;
  if ( m_sensor_init_pu_events   != s2.m_sensor_init_pu_events   ) return false;
  if ( m_sensor_init_pu_scanning != s2.m_sensor_init_pu_scanning ) return false;
  if ( m_ignore_if_no_entity     != s2.m_ignore_if_no_entity     ) return false;
  if ( m_supports_auto_rearm     != s2.m_supports_auto_rearm     ) return false;
  if ( m_hysteresis_support      != s2.m_hysteresis_support      ) return false;
  if ( m_threshold_access        != s2.m_threshold_access        ) return false;
  if ( m_event_support           != s2.m_event_support           ) return false;
  if ( m_sensor_type             != s2.m_sensor_type             ) return false;
  if ( m_event_reading_type      != s2.m_event_reading_type      ) return false;
  
  for( i = 0; i < 16; i++ )
     {
       if ( m_mask1[i] != s2.m_mask1[i] ) return false;
       if ( m_mask2[i] != s2.m_mask2[i] ) return false;
       if ( m_mask3[i] != s2.m_mask3[i] ) return false;
     }
    
  if ( m_analog_data_format != s2.m_analog_data_format ) return false;
  if ( m_rate_unit          != s2.m_rate_unit          ) return false;
  if ( m_modifier_unit_use  != s2.m_modifier_unit_use  ) return false;
  if ( m_percentage         != s2.m_percentage         ) return false;
  if ( m_base_unit          != s2.m_base_unit          ) return false;
  if ( m_modifier_unit      != s2.m_modifier_unit      ) return false;
  if ( m_linearization      != s2.m_linearization      ) return false;

  if ( m_linearization <= 11 )
     {
       if ( m_m            != s2.m_m            ) return false;
       if ( m_tolerance    != s2.m_tolerance    ) return false;
       if ( m_b            != s2.m_b            ) return false;
       if ( m_accuracy     != s2.m_accuracy     ) return false;
       if ( m_accuracy_exp != s2.m_accuracy_exp ) return false;
       if ( m_r_exp        != s2.m_r_exp        ) return false;
       if ( m_b_exp        != s2.m_b_exp        ) return false;
     }

  if ( m_normal_min_specified != s2.m_normal_min_specified ) return false;
  if ( m_normal_max_specified != s2.m_normal_max_specified ) return false;
  if ( m_nominal_reading_specified != s2.m_nominal_reading_specified ) return false;
  if ( m_nominal_reading != s2.m_nominal_reading ) return false;
  if ( m_normal_max != s2.m_normal_max ) return false;
  if ( m_normal_min != s2.m_normal_min ) return false;
  if ( m_sensor_max != s2.m_sensor_max ) return false;
  if ( m_sensor_min != s2.m_sensor_min ) return false;
  if (    m_upper_non_recoverable_threshold
       != s2.m_upper_non_recoverable_threshold )
       return false;
  if ( m_upper_critical_threshold != s2.m_upper_critical_threshold ) return false;
  if (    m_upper_non_critical_threshold
       != s2.m_upper_non_critical_threshold )
       return false;
  if (    m_lower_non_recoverable_threshold
       != s2.m_lower_non_recoverable_threshold )
       return false;
  if ( m_lower_critical_threshold != s2.m_lower_critical_threshold ) return false;
  if ( m_lower_non_critical_threshold
      != s2.m_lower_non_critical_threshold )
       return false;
  if (    m_positive_going_threshold_hysteresis
       != s2.m_positive_going_threshold_hysteresis )
       return false;
  if (    m_negative_going_threshold_hysteresis
       != s2.m_negative_going_threshold_hysteresis )
       return false;
  if ( m_oem1 != s2.m_oem1 ) return false;

  if ( strcmp( m_id, s2.m_id ) != 0 ) return false;

  return true;
}


void
cIpmiSensor::FinalDestroy()
{
  cIpmiDomain     *domain = m_mc->Domain();
  cIpmiEntityInfo &ents   = domain->Entities();
  cIpmiSensorInfo *sensors = m_mc->Sensors();

  assert( sensors->m_sensors_by_idx[m_lun][m_num] == this );

  if ( m_source_array )
       m_source_array[m_source_idx] = 0;

  sensors->m_sensor_count--;
  sensors->m_sensors_by_idx[m_lun][m_num] = 0;

  /* XXXX remove HPI resource */
  /*
  if (sensor->destroy_handler)
      sensor->destroy_handler(sensor, sensor->destroy_handler_cb_data);
  */

  // This is were we remove the sensor from the entity, possibly
  // destroying it.  The opq destruction can call a bunch of
  // callbacks with the sensor, so we want the entity to exist until
  // this point in time.
  cIpmiEntity *ent = ents.Find( m_mc, m_entity_id, m_entity_instance );

  if ( ent )
       ent->RemoveSensor( this );

  delete this;
}


bool
cIpmiSensor::Destroy()
{
  cIpmiSensorInfo *sensors = m_mc->Sensors();

  if ( sensors->m_sensors_by_idx[m_lun][m_num] != this )
     {
       assert( 0 );
       return false;
     }

  FinalDestroy();

  return true;
}


cIpmiSensor **
IpmiGetSensorsFromSdrs( cIpmiDomain  *domain,
                        cIpmiMc      *source_mc,
                        cIpmiSdrs    *sdrs,
                        unsigned int &sensor_count )
{
  cIpmiSdr      *sdr;
  cIpmiSensor  **s = 0;
  unsigned int  p, s_size = 0;
  int           val;
  unsigned int  i;
  int           j;

  // Get a real count on the number of sensors, since a single SDR can
  // contain multiple sensors.
  p = 0;

  for( i = 0; i < sdrs->NumSdrs(); i++)
     {
       int incr;
       int lun;

       sdr = sdrs->Sdr( i );

       lun = sdr->m_data[6] & 0x03;

       if ( sdr->m_type == eSdrTypeFullSensorRecord )
	    incr = 1;
       else if ( sdr->m_type == eSdrTypeCompactSensorRecord )
          {
	    if ( sdr->m_data[23] & 0x0f )
                 incr = sdr->m_data[23] & 0x0f;
	    else
                 incr = 1;
          }
       else
	    continue;

       p += incr;
     }

  // Setup memory to hold the sensors.
  s = new cIpmiSensor *[p];

  if ( !s )
       return 0;

  s_size = p;
  memset( s, 0, sizeof( cIpmiSensor *) * p);

  p = 0;

  for( i = 0; i < sdrs->NumSdrs(); i++ )
     {
       sdr = sdrs->Sdr( i );

       if (    sdr->m_type != eSdrTypeFullSensorRecord
            && sdr->m_type != eSdrTypeCompactSensorRecord )
	    continue;

       s[p] = new cIpmiSensor( source_mc );

       if ( !s[p] )
	    goto out_err;

       s[p]->m_use_count = 1;
       s[p]->m_hot_swap_requester = -1;
       
       s[p]->m_destroyed = false;

       s[p]->m_mc = domain->FindOrCreateMcBySlaveAddr( sdr->m_data[5] );

       if ( s[p]->m_mc == 0 )
            goto out_err;

       s[p]->m_source_mc = source_mc;
       s[p]->m_source_idx = p;
       s[p]->m_source_array = s;
       s[p]->m_owner                   = sdr->m_data[5];
       s[p]->m_channel                 = sdr->m_data[6] >> 4;
       s[p]->m_lun                     = sdr->m_data[6] & 0x03;
       s[p]->m_num                     = sdr->m_data[7];
       s[p]->m_entity_id               = (tIpmiEntityId)sdr->m_data[8];
       s[p]->m_entity_instance_logical = sdr->m_data[9] >> 7;
       s[p]->m_entity_instance         = sdr->m_data[9] & 0x7f;
       s[p]->m_sensor_init_scanning    = (sdr->m_data[10] >> 6) & 1;
       s[p]->m_sensor_init_events      = (sdr->m_data[10] >> 5) & 1;
       s[p]->m_sensor_init_thresholds  = (sdr->m_data[10] >> 4) & 1;
       s[p]->m_sensor_init_hysteresis  = (sdr->m_data[10] >> 3) & 1;
       s[p]->m_sensor_init_type        = (sdr->m_data[10] >> 2) & 1;
       s[p]->m_sensor_init_pu_events   = (sdr->m_data[10] >> 1) & 1;
       s[p]->m_sensor_init_pu_scanning = (sdr->m_data[10] >> 0) & 1;
       s[p]->m_ignore_if_no_entity     = (sdr->m_data[11] >> 7) & 1;
       s[p]->m_supports_auto_rearm     = (sdr->m_data[11] >> 6) & 1;
       s[p]->m_hysteresis_support      = (tIpmiHysteresisSupport)((sdr->m_data[11] >> 4) & 3);
       s[p]->m_threshold_access        = (tIpmiThresholdAccessSuport)((sdr->m_data[11] >> 2) & 3);
       s[p]->m_event_support           = (tIpmiEventSupport)(sdr->m_data[11] & 3);
       s[p]->m_sensor_type             = (tIpmiSensorType)sdr->m_data[12];
       s[p]->m_event_reading_type      = (tIpmiEventReadingType)sdr->m_data[13];

       val = IpmiGetUint16( sdr->m_data + 14 );
       for( j = 0; j < 16; j++ )
          {
            s[p]->m_mask1[j] = val & 1;
            val >>= 1;
          }

       val = IpmiGetUint16( sdr->m_data+16) ;
       for( j = 0; j < 16; j++ )
          {
            s[p]->m_mask2[j] = val & 1;
            val >>= 1;
          }

       val = IpmiGetUint16( sdr->m_data+18 );
       for( j = 0; j < 16; j++ )
          {
            s[p]->m_mask3[j] = val & 1;
            val >>= 1;
          }

       s[p]->m_analog_data_format = (tIpmiAnalogeDataFormat)((sdr->m_data[20] >> 6) & 3);
       s[p]->m_rate_unit          = (tIpmiRateUnit)((sdr->m_data[20] >> 3) & 7);
       s[p]->m_modifier_unit_use  = (tIpmiModifierUnit)((sdr->m_data[20] >> 1) & 3);
       s[p]->m_percentage         = sdr->m_data[20] & 1;
       s[p]->m_base_unit          = (tIpmiUnitType)sdr->m_data[21];
       s[p]->m_modifier_unit      = (tIpmiUnitType)sdr->m_data[22];

       if ( sdr->m_type == eSdrTypeFullSensorRecord )
          {
            // A full sensor record.
            s[p]->m_linearization = (tIpmiLinearization)(sdr->m_data[23] & 0x7f);

            if ( s[p]->m_linearization <= 11 )
               {
                 s[p]->m_m            = sdr->m_data[24] | ((sdr->m_data[25] & 0xc0) << 2);
                 s[p]->m_tolerance    = sdr->m_data[25] & 0x3f;
                 s[p]->m_b            = sdr->m_data[26] | ((sdr->m_data[27] & 0xc0) << 2);
                 s[p]->m_accuracy     = ((sdr->m_data[27] & 0x3f)
                                       | ((sdr->m_data[28] & 0xf0) << 2));
                 s[p]->m_accuracy_exp = (sdr->m_data[28] >> 2) & 0x3;
                 s[p]->m_r_exp        = (sdr->m_data[29] >> 4) & 0xf;
                 s[p]->m_b_exp        = sdr->m_data[29] & 0xf;
               }

            s[p]->m_normal_min_specified = (sdr->m_data[30] >> 2) & 1;
            s[p]->m_normal_max_specified = (sdr->m_data[30] >> 1) & 1;
            s[p]->m_nominal_reading_specified = sdr->m_data[30] & 1;
            s[p]->m_nominal_reading = sdr->m_data[31];
            s[p]->m_normal_max = sdr->m_data[32];
            s[p]->m_normal_min = sdr->m_data[33];
            s[p]->m_sensor_max = sdr->m_data[34];
            s[p]->m_sensor_min = sdr->m_data[35];
            s[p]->m_upper_non_recoverable_threshold = sdr->m_data[36];
            s[p]->m_upper_critical_threshold = sdr->m_data[37];
            s[p]->m_upper_non_critical_threshold = sdr->m_data[38];
            s[p]->m_lower_non_recoverable_threshold = sdr->m_data[39];
            s[p]->m_lower_critical_threshold = sdr->m_data[40];
            s[p]->m_lower_non_critical_threshold = sdr->m_data[41];
            s[p]->m_positive_going_threshold_hysteresis = sdr->m_data[42];
            s[p]->m_negative_going_threshold_hysteresis = sdr->m_data[43];
            s[p]->m_oem1 = sdr->m_data[44];

            IpmiGetDeviceString( sdr->m_data+47, sdr->m_length-47, s[p]->m_id,
                                 dSensorIdLen );

            p++;
          }
       else 
          {
            // FIXME - make sure this is not a threshold sensor.  The
            // question is, what do I do if it is?
            // A short sensor record.
            s[p]->m_positive_going_threshold_hysteresis = sdr->m_data[25];
            s[p]->m_negative_going_threshold_hysteresis = sdr->m_data[26];
            s[p]->m_oem1 = sdr->m_data[30];

            IpmiGetDeviceString( sdr->m_data+31, sdr->m_length-31, s[p]->m_id,
                                 dSensorIdLen );

            // Duplicate the sensor records for each instance.  Go
            // backwards to avoid destroying the first one until we
            // finish the others.
            for( j = (sdr->m_data[23] & 0x0f) - 1; j >= 0; j-- )
               {
                 int len;

                 if ( j != 0 )
                    {
                      // The first one is already allocated, we are
                      // using it to copy to the other ones, so this is
                      // not necessary.
                      s[p+j] = new cIpmiSensor( *s[p] );

                      if ( !s[p+j] )
                           goto out_err;

                      s[p+j]->m_num += j;

                      if ( sdr->m_data[24] & 0x80 )
                           s[p+j]->m_entity_instance += j;

                      //                       s[p+j]->source_idx += j;
                    }

                 val = (sdr->m_data[24] & 0x3f) + j;
                 len = strlen( s[p+j]->m_id );
                 switch( (sdr->m_data[23] >> 4) & 0x03 )
                    {
                      case 0: // Numeric
                           if ((val / 10) > 0)
                              {
                                if ( len < dSensorIdLen )
                                   {
                                     s[p+j]->m_id[len] = (val/10) + '0';
                                     len++;
                                   }
                              }

                           if ( len < dSensorIdLen )
                              {
                                s[p+j]->m_id[len] = (val%10) + '0';
                                len++;
                              }
                           break;

                      case 1: // Alpha
                           if ( (val / 26) > 0 )
                              {
                                if ( len < dSensorIdLen )
                                   {
                                     s[p+j]->m_id[len] = (val/26) + 'A';
                                     len++;
                                   }
                              }

                           if ( len < dSensorIdLen )
                              {
                                s[p+j]->m_id[len] = (val%26) + 'A';
                                len++;
                              }
                           break;
                    }

                 s[p+j]->m_id[len] = '\0';
               }

            if ( sdr->m_data[23] & 0x0f )
                 p += sdr->m_data[23] & 0x0f;
            else
                 p++;
          }
     }

  sensor_count = s_size;
  return s;

out_err:
  if ( s )
     {
       for( i = 0; i < s_size; i++ )
	    if ( s[i] )
                 delete s[i];

       delete [] s;
     }

  return 0;
}


int
IpmiSensorHandleSdr( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  unsigned int   i;
  unsigned int   j;
  cIpmiSensor  **sdr_sensors;
  cIpmiSensor  **old_sdr_sensors;
  unsigned int   old_count;
  unsigned int   count;

  sdr_sensors = IpmiGetSensorsFromSdrs( domain, source_mc, sdrs, count );

  if ( !sdr_sensors )
       goto out_err;

  for( i = 0; i < count; i++ )
     {
       cIpmiSensor *nsensor = sdr_sensors[i];

       if ( nsensor == 0 )
            continue;

       if ( domain->Entities().Add( nsensor->Mc(),
                                    i, // is this right (lun) ?
                                    nsensor->EntityId(),
                                    (unsigned int)nsensor->EntityInstance(),
                                    "" ) == 0 )
            goto out_err;

       cIpmiSensorInfo *sensors = nsensor->Mc()->Sensors();

       // There's not enough room in the sensor repository for the new
       // item, so expand the array.
       if ( nsensor->Num() >= sensors->m_idx_size[nsensor->Lun()] )
          {
            unsigned int  new_size = nsensor->Num() + 10;
            cIpmiSensor **new_by_idx = new cIpmiSensor *[new_size];
            if ( !new_by_idx )
                 goto out_err;

            if ( sensors->m_sensors_by_idx[nsensor->Lun()] )
               {
                 memcpy( new_by_idx,
                        sensors->m_sensors_by_idx[nsensor->Lun()],
                        (sensors->m_idx_size[nsensor->Lun()]
                         * sizeof( cIpmiSensor *) ) );

                 delete [] sensors->m_sensors_by_idx[nsensor->Lun()];
               }

            for( j = sensors->m_idx_size[nsensor->Lun()]; j < new_size; j++)
                 new_by_idx[j] = 0;

            sensors->m_sensors_by_idx[nsensor->Lun()] = new_by_idx;
            sensors->m_idx_size[nsensor->Lun()] = new_size;
          }
     }

  // After this point, the operation cannot fail.
  old_sdr_sensors = domain->GetSdrSensors( source_mc, old_count );

  // For each new sensor, put it into the MC it belongs with.
  for( i = 0; i < count; i++ )
     {
       cIpmiSensor *nsensor = sdr_sensors[i];

       if ( !nsensor )
            continue;

       cIpmiSensorInfo *sensors = nsensor->Mc()->Sensors();

       if (    sensors->m_sensors_by_idx[nsensor->Lun()]
            && (nsensor->Num() < sensors->m_idx_size[nsensor->Lun()])
            && sensors->m_sensors_by_idx[nsensor->Lun()][nsensor->Num()])
          {
            // It's already there.
            cIpmiSensor *osensor
              = sensors->m_sensors_by_idx[nsensor->Lun()][nsensor->Num()];

            if ( osensor->SourceArray() == sdr_sensors )
               {
                 // It's from the same SDR repository, log an error
                 // and continue to delete the first one.
                 IpmiLog( "Sensor 0x%x is the same as sensor 0x%x in the"
                          " repository",
                          osensor->SourceIdx(),
                          nsensor->SourceIdx());
               }

            // Delete the sensor from the source array it came
            // from.
            if ( osensor->SourceArray() )
               {
                 osensor->SetSourceArray( osensor->SourceIdx(), 0 );
                 osensor->SetSourceArray( 0 );
               }

            if ( nsensor->Cmp( *osensor ) )
               {
                 // They compare, prefer to keep the old data.
                 delete nsensor;
                 sdr_sensors[i] = osensor;
                 osensor->SourceIdx() = i;
                 osensor->SetSourceArray( sdr_sensors );
               }
            else
               {
                 osensor->Destroy();

                 sensors->m_sensors_by_idx[nsensor->Lun()][nsensor->Num()]
                   = nsensor;
                 nsensor->HandleNew( domain );
               }
          }
       else
          {
            // It's a new sensor.
            sensors->m_sensors_by_idx[nsensor->Lun()][nsensor->Num()] = nsensor;
            sensors->m_sensor_count++;
            nsensor->HandleNew( domain );
          }
     }

  domain->SetSdrSensors( source_mc, sdr_sensors, count );

  if ( old_sdr_sensors )
     {
       for( i = 0; i < old_count; i++ )
          {
            cIpmiSensor *osensor = old_sdr_sensors[i];
            if ( osensor != 0 )
               {
                 // This sensor was not in the new repository, so it must
                 // have been deleted.
                 osensor->Destroy();
               }
          }

       delete [] old_sdr_sensors;
     }

out_err:
  return 0;
}


int
cIpmiSensor::ThresholdAssertionEventSupported( tIpmiThresh  event,
                                               tIpmiEventValueDir dir,
                                               int &val )
{
  int idx;
  
  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  idx = (event * 2) + dir;
  if (idx > 11)
       return EINVAL;

  val = m_mask1[idx];

  return 0;
}


int
cIpmiSensor::ThresholdDeassertionEventSupported( tIpmiThresh  event,
                                                 tIpmiEventValueDir dir,
                                                 int &val )
{
  int idx;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  idx = (event * 2) + dir;
  if ( idx > 11 )
       return 0;

  val = m_mask2[idx];
  return 0;
}


int
cIpmiSensor::DiscreteEventReadable( int event, int &val )
{
  if ( m_event_reading_type == eIpmiEventReadingTypeThreshold )
       // A threshold sensor, it doesn't have events.
       return ENOSYS;

  if ( event > 14 )
       return EINVAL;

  val = m_mask3[event];
  return 0;
}


int
cIpmiSensor::DiscreteAssertionEventSupported( int event, int &val )
{
  if ( m_event_reading_type == eIpmiEventReadingTypeThreshold )
       // A threshold sensor, it doesn't have events.
       return ENOSYS;

  if ( event > 14 )
       return EINVAL;

  val = m_mask1[event];
  return 0;
}


int
cIpmiSensor::DiscreteDeassertionEventSupported( int event, int &val )
{
  if ( m_event_reading_type == eIpmiEventReadingTypeThreshold )
       // A threshold sensor, it doesn't have events.
       return ENOSYS;

  if ( event > 14 )
       return EINVAL;

  val = m_mask2[event];
  return 0;
}


void
cIpmiSensor::GetId( char *id, int length )
{
  strncpy( id, m_id, length );
  id[length] = '\0';
}


int
cIpmiSensor::ThresholdReadable( tIpmiThresh event )
{
  assert( m_event_reading_type == eIpmiEventReadingTypeThreshold );
  assert( event <= eIpmiUpperNonRecoverable );

  return m_mask3[event];
}


int
cIpmiSensor::ThresholdSettable( tIpmiThresh event )
{
  assert( m_event_reading_type == eIpmiEventReadingTypeThreshold );
  assert( event <= eIpmiUpperNonRecoverable );
  
  return m_mask3[event + 8];
}


cIpmiEntity *
cIpmiSensor::GetEntity()
{
  cIpmiDomain *domain = m_mc->Domain();

  return domain->Entities().Find( m_mc, m_entity_id,
                                  m_entity_instance );
}


static double
c_linear( double val )
{
  return val;
}


static double
c_exp10( double val )
{
  return pow( 10.0, val );
}


static double
c_exp2( double val )
{
  return pow( 2.0, val );
}


static double
c_1_over_x( double val )
{
  return 1.0 / val;
}


static double
c_sqr( double val )
{
  return pow( val, 2.0 );
}


static double
c_cube( double val )
{
  return pow( val, 3.0 );
}


static double
c_1_over_cube( double val )
{
  return 1.0 / pow( val, 3.0 );
}


typedef double (*linearizer)(double val);
static linearizer linearize[11] =
{
  c_linear,
  log,
  log10,
  exp,
  c_exp10,
  c_exp2,
  c_1_over_x,
  c_sqr,
  c_cube,
  sqrt,
  c_1_over_cube
};


static int
sign_extend(int m, int bits)
{
    if ( m & (1 << (bits-1)) )
	return m | (-1 << bits);
    else
	return m & (~(-1 << bits));
}


int
cIpmiSensor::ConvertFromRaw( unsigned int val,
                             double      &result )
{
  double m, b, b_exp, r_exp, fval;
  linearizer c_func;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  if ( m_linearization == eIpmiLinearizationNonlinear )
       c_func = c_linear;
  else if ( m_linearization <= 11 )
       c_func = linearize[m_linearization];
  else
       return EINVAL;

  val &= 0xff;

  m     = m_m;
  b     = m_b;
  r_exp = m_r_exp;
  b_exp = m_b_exp;

  switch( m_analog_data_format )
     {
       case eIpmiAnalogDataFormatUnsigned:
	    fval = val;
	    break;

       case eIpmiAnalogDataFormat1Compl:
	    val = sign_extend( val, 8 );
	    if ( val == 0xffffffff )
                 val += 1;

	    fval = val;
	    break;

       case eIpmiAnalogDataFormat2Compl:
	    fval = sign_extend( val, 8 );
	    break;

       default:
	    return EINVAL;
     }

  result = c_func( ((m * fval) + (b * pow(10, b_exp))) * pow(10, r_exp) );

  return 0;
}


int
cIpmiSensor::ConvertToRaw( tIpmiRound    rounding,
                           double        val,
                           unsigned int &result )
{
  double cval;
  int    lowraw, highraw, raw, maxraw, minraw, next_raw;
  int    rv;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  switch( m_analog_data_format )
     {
       case eIpmiAnalogDataFormatUnsigned:
	    lowraw = 0;
	    highraw = 255;
	    minraw = 0;
	    maxraw = 255;
	    next_raw = 128;
	    break;

       case eIpmiAnalogDataFormat1Compl:
	    lowraw = -127;
	    highraw = 127;
	    minraw = -127;
	    maxraw = 127;
	    next_raw = 0;
	    break;

       case eIpmiAnalogDataFormat2Compl:
	    lowraw = -128;
	    highraw = 127;
	    minraw = -128;
	    maxraw = 127;
	    next_raw = 0;
	    break;

       default:
	    return EINVAL;
    }

  // We do a binary search for the right value.  Yuck, but I don't
  // have a better plan that will work with non-linear sensors.
  do
     {
       raw = next_raw;
       rv = ConvertFromRaw( raw, cval );
       if ( rv )
	    return rv;

       if ( cval < val )
          {
	    next_raw = ((highraw - raw) / 2) + raw;
	    lowraw = raw;
          } 
       else
          {
	    next_raw = ((raw - lowraw) / 2) + lowraw;
	    highraw = raw;
          }
     } 
  while( raw != next_raw );

  /* The above loop gets us to within 1 of what it should be, we
     have to look at rounding to make the final decision. */
  switch( rounding )
     {
       case eRoundNormal:
	    if ( val > cval )
               {
                 if ( raw < maxraw )
                    {
                      double nval;
                      rv = ConvertFromRaw( raw + 1, nval );
                      if ( rv )
                           return rv;
                      nval = cval + ((nval - cval) / 2.0);
                      if ( val >= nval )
                           raw++;
                    }
               } 
            else
               {
                 if ( raw > minraw )
                    {
                      double pval;
                      rv = ConvertFromRaw( raw-1, pval );
                      if ( rv )
                           return rv;
                      pval = pval + ((cval - pval) / 2.0);
                      if ( val < pval )
                           raw--;
                    }
               }
	    break;
            
       case eRoundUp:
	    if ((val > cval) && (raw < maxraw))
                 raw++;

	    break;

       case eRoundDown:
	    if ( ( val < cval) && (raw > minraw ) )
                 raw--;

	    break;
     }

  if ( m_analog_data_format == eIpmiAnalogDataFormat1Compl )
       if ( raw < 0 )
	    raw -= 1;

  result = raw & 0xff;
  return 0;
}


static void
IpmiInitStates( tIpmiStates &states )
{
  states.m_event_messages_enabled = 0;
  states.m_sensor_scanning_enabled = 0;
  states.m_initial_update_in_progress = 0;
  states.m_states = 0;
}


int
cIpmiSensor::ReadingGet( tIpmiValuePresent &val_present,
                         unsigned int &raw_val, double &val,
                         tIpmiStates &states )
{
  cIpmiEntity *ent = GetEntity();
  IpmiLog( "%d.%d sensor %d (%s) read.\n",
           ent->EntityId(), ent->EntityInstance(),
           m_num, m_id );

  cIpmiMsg cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorReading );
  cIpmiMsg rsp;
  int      rv;

  cmd_msg.m_data_len = 1;
  cmd_msg.m_data[0]  = m_num;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error sending reading get command: %d, %s!\n",
                rv, strerror( rv ) );

       return rv;
     }

  if ( rsp.m_data[0] )
     {
       IpmiLog( "IPMI error getting reading: %x !\n", rsp.m_data[0] );

       return EINVAL;
     }

  IpmiInitStates( states );

  if ( m_analog_data_format != eIpmiAnalogDataFormatNotAnalog )
     {
       rv = ConvertFromRaw( rsp.m_data[1], val );

       if ( rv )
	    val_present = eIpmiRawValuePresent;
       else
	    val_present = eIpmiBothValuesPresent;
     }
  else
       val_present = eIpmiNoValuesPresent;

  states.m_event_messages_enabled     = (rsp.m_data[2] >> 7) & 1;
  states.m_sensor_scanning_enabled    = (rsp.m_data[2] >> 6) & 1;
  states.m_initial_update_in_progress = (rsp.m_data[2] >> 5) & 1;
  states.m_states                     = rsp.m_data[3];

  raw_val = rsp.m_data[1];

  return 0;
}


int
cIpmiSensor::StatesGet( tIpmiStates &states )
{
  cIpmiMsg cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorReading );
  cIpmiMsg rsp;
  int      rv;

  cmd_msg.m_data_len = 1;
  cmd_msg.m_data[0]  = m_num;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "IPMI error getting states: %d, %s!\n",
                rv, strerror( rv ) );

       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       IpmiLog( "IPMI error getting reading: %x !\n", rsp.m_data[0] );

       return EINVAL;       
     }
  
  IpmiInitStates( states );

  states.m_event_messages_enabled     = (rsp.m_data[2] >> 7) & 1;
  states.m_sensor_scanning_enabled    = (rsp.m_data[2] >> 6) & 1;
  states.m_initial_update_in_progress = (rsp.m_data[2] >> 5) & 1;
  states.m_states                     = (rsp.m_data[4] << 8) | rsp.m_data[3];

  return 0;
}


int
cIpmiSensor::GetDefaultSensorThresholds( int              raw,
                                         tIpmiThresholds &th )
{
  int val;
  //tIpmiThresh thnum;
  int rv = 0;
  int thnum;

  for( thnum = eIpmiLowerNonCritical; thnum <= eIpmiUpperNonRecoverable; thnum++ )
     {
       val = ThresholdReadable( (tIpmiThresh)thnum );

       if ( val )
          {
            th.m_vals[thnum].m_status = 1;
            rv = ConvertFromRaw( raw, th.m_vals[thnum].m_val );
            if ( rv )
                 return rv;
          }
       else
	    th.m_vals[thnum].m_status = 0;
     }

  return 0;
}


int
cIpmiSensor::ThresholdsGet( tIpmiThresholds &th )
{
  int rv;
  //tIpmiThresh t;
  int t;
  
  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       /* Not a threshold sensor, it doesn't have readings. */
       return ENOSYS;

  if ( m_threshold_access == eIpmiThresholdAccessSupportNone )
       return ENOTSUP;

  if ( m_threshold_access == eIpmiThresholdAccessSupportFixed )
       /* Thresholds are fixed, pull them from the SDR. */
       return GetDefaultSensorThresholds( 0, th );

  cIpmiEntity *ent = GetEntity();
  
  IpmiLog( "%d.%d sensor %d (%s) get thresholds.\n",
           ent->EntityId(), ent->EntityInstance(),
           m_num, m_id );

  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorThreshold );
  cIpmiMsg  rsp;

  cmd_msg.m_data_len = 1;
  cmd_msg.m_data[0]  = m_num;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error getting thresholds: %x !\n", rv );

       return rv;
     }

  if ( rsp.m_data[0] )
     {
       IpmiLog( "IPMI error getting thresholds: %x !\n", rsp.m_data[0] );

       return EINVAL;
    }

  for( t = eIpmiLowerNonCritical; t <= eIpmiUpperNonRecoverable; t++)
     {
       if ( rsp.m_data[1] & (1 << t) )
          {
	    th.m_vals[t].m_status = 1;

	    rv = ConvertFromRaw( rsp.m_data[t+2],
                                 th.m_vals[t].m_val );
	    if ( rv )
               {
                 IpmiLog( "Could not convert raw threshold value: %x !\n", rv );

                 return rv;
               }
          }
       else
	    th.m_vals[t].m_status = 0;
     }

  return 0;
}


int
cIpmiSensor::ThresholdsSet( tIpmiThresholds &th )
{
  int rv;
  // tIpmiThresh t;
  int t;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  if ( m_threshold_access != eIpmiThresholdAccessSupportSettable )
       return ENOTSUP;

  cIpmiEntity *ent = GetEntity();

  IpmiLog( "%d.%d sensor %d (%s) set thresholds.\n",
           ent->EntityId(), ent->EntityInstance(),
           m_num, m_id );

  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdSetSensorThreshold );
  cIpmiMsg  rsp;

  cmd_msg.m_data_len = 8;
  cmd_msg.m_data[0]  = m_num;
  cmd_msg.m_data[1]  = 0;

  for( t = eIpmiLowerNonCritical; t <= eIpmiUpperNonRecoverable; t++ )
     {
       unsigned int val = 0;

       if ( th.m_vals[t].m_status )
          {
	    cmd_msg.m_data[1] |= (1 << t);
	    rv = ConvertToRaw( eRoundNormal,
                               th.m_vals[t].m_val,
                               val );
	    if ( rv )
               {
                 IpmiLog( "Error converting threshold to raw: %x !\n", rv );

                 return EINVAL;
               }
          }

       cmd_msg.m_data[t+2] = val;
     }

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error sending thresholds set command: %x !\n", rv );
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       IpmiLog( "IPMI error setting thresholds: %x !\n", rsp.m_data[0] );
       return EINVAL;
    }

  return 0;
}


int
cIpmiSensor::GetHysteresis( unsigned int &positive_hysteresis,
                            unsigned int &negative_hysteresis )
{
  int rv;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  if (    m_hysteresis_support != eIpmiHysteresisSupportReadable
       && m_hysteresis_support != eIpmiHysteresisSupportSettable)
       return ENOTSUP;

  cIpmiEntity *ent = GetEntity();

  IpmiLog( "%d.%d sensor %d (%s) get hysteresis.\n",
           ent->EntityId(), ent->EntityInstance(),
           m_num, m_id );

  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorHysteresis );
  cIpmiMsg  rsp;

  cmd_msg.m_data_len = 2;
  cmd_msg.m_data[0]  = m_num;
  cmd_msg.m_data[1]  = 0xff;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error sending hysteresis get command: %x !\n", rv );

       return rv;
     }

  if ( rsp.m_data[0] || rsp.m_data_len < 3 )
     {
       IpmiLog( "IPMI error getting hysteresis: %x !\n", rsp.m_data[0] );

       return EINVAL;
    }

  positive_hysteresis = rsp.m_data[1];
  negative_hysteresis = rsp.m_data[2];

  return 0;
}


int
cIpmiSensor::SetHysteresis( unsigned int positive_hysteresis,
                            unsigned int negative_hysteresis )
{
  int rv;

  if ( m_event_reading_type != eIpmiEventReadingTypeThreshold )
       // Not a threshold sensor, it doesn't have readings.
       return ENOSYS;

  if ( m_hysteresis_support != eIpmiHysteresisSupportSettable )
       return ENOTSUP;

  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdSetSensorHysteresis );
  cIpmiMsg  rsp;

  cmd_msg.m_data_len = 4;
  cmd_msg.m_data[0]  = m_num;
  cmd_msg.m_data[1]  = 0xff;
  cmd_msg.m_data[2]  = positive_hysteresis;
  cmd_msg.m_data[3]  = negative_hysteresis;

  cIpmiEntity *ent = GetEntity();

  IpmiLog( "%d.%d sensor %d (%s) set hysteresis %d, %d.\n",
           ent->EntityId(), ent->EntityInstance(),
           m_num, m_id,
           (int)positive_hysteresis, (int)negative_hysteresis );

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error sending hysteresis set command: %x !\n", rv );

       return rv;
     }

  if ( rsp.m_data[0] )
     {
       IpmiLog( "IPMI error setting hysteresis: %x !\n", rsp.m_data[0] );
       return EINVAL;
     }

  return 0;
}


int
cIpmiSensor::EventsEnableGet( cIpmiEventState &state )
{
  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorEventEnable );
  cIpmiMsg  rsp;
  int       rv;

  cmd_msg.m_data_len = 1;
  cmd_msg.m_data[0]  = m_num;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Error sending get event enables command: %x !\n", rv );
       return EINVAL;
     }

  if ( rsp.m_data[0] )
     {
       IpmiLog( "IPMI error getting sensor enables: %x !\n", rsp.m_data[0] );

       return EINVAL;
    }

  state.m_status = rsp.m_data[1] & 0xc0;
  state.m_assertion_events   =   (rsp.m_data[2]
                               | (rsp.m_data[3] << 8));
  state.m_deassertion_events =   (rsp.m_data[4]
                               | (rsp.m_data[5] << 8));

  return 0;
}


int
cIpmiSensor::CheckEventsCapability( const cIpmiEventState &states ) const
{
  int event_support;

  event_support = m_event_support;

  if (    (event_support == eIpmiEventSupportNone )
       || (event_support == eIpmiEventSupportGlobalEnable ) )
       // We don't support setting events for this sensor.
       return EINVAL;

  if (    ( event_support == eIpmiEventSupportEntireSensor )
       && (    (states.m_assertion_events   != 0 )
	    || (states.m_deassertion_events != 0 ) ) )
       // This sensor does not support individual event states, but
       // the user is trying to set them.
	return EINVAL;

  if ( event_support == eIpmiEventSupportPerState )
     {
       int i;

       for( i = 0; i < 16; i++ )
          {
	    unsigned int bit = 1 << i;

	    if (    ((!m_mask1[i]) && (bit & states.m_assertion_events))
                 || ((!m_mask2[i]) && (bit & states.m_deassertion_events)))
                 // The user is attempting to set a state that the
                 // sensor does not support.
                 return EINVAL;
          }
     }

  return 0;
}


int
cIpmiSensor::EventsEnableSet( const cIpmiEventState &states )
{
  int       event_support;
  int rv;

  rv = CheckEventsCapability( states );

  if ( rv )
       return rv;

  event_support = m_event_support;

  cIpmiMsg  cmd_msg( eIpmiNetfnSensorEvent, eIpmiCmdSetSensorEventEnable );
  cIpmiMsg  rsp;

  cmd_msg.m_data[0] = m_num;

  if ( event_support == eIpmiEventSupportEntireSensor )
     {
       // We can only turn on/off the entire sensor, just pass the
       // status to the sensor.
       cmd_msg.m_data[1] = states.m_status & 0xc0;
       cmd_msg.m_data_len = 2;

       rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

       if ( rv )
          {
            IpmiLog( "Disable sensor events fail: %x !\n", rv );
            return rv;
          }

       if ( rsp.m_data[0] )
          {
            IpmiLog( "IPMI Disable sensor events fail: %x !\n", rv );
            return rv;
          }
     }
  else
     {
       // Start by first setting the enables, then set the disables
       // in a second operation.  We do this because enables and
       // disables cannot both be set at the same time, and it's
       // safer to first enable the new events then to disable the
       // events we want disabled.  It would be *really* nice if IPMI
       // had a way to do this in one operation, such as using 11b in
       // the request byte 2 bits 5:4 to say "set the events to
       // exactly this state".
       cmd_msg.m_data[1] = 0xc0;  // (states->status & 0xc0) | (0x01 << 4);
       cmd_msg.m_data[2] = states.m_assertion_events & 0xff;
       cmd_msg.m_data[3] = states.m_assertion_events >> 8;
       cmd_msg.m_data[4] = states.m_deassertion_events & 0xff;
       cmd_msg.m_data[5] = states.m_deassertion_events >> 8;
       cmd_msg.m_data_len = 6;

       rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

       if ( rv )
          {
            IpmiLog( "Enable sensor events fail: %x !\n", rv );
            return rv;
          }

       if ( rsp.m_data[0] )
          {
            IpmiLog( "IPMI enable sensor events fail: %x !\n", rv );
            return rv;
          }
    }

  return 0;
}


static const char *sensor_types[] =
{
  "unspecified",
  "temperature",
  "voltage",
  "current",
  "fan",
  "physical_security",
  "platform_security",
  "processor",
  "power_supply",
  "power_unit",
  "cooling_device",
  "other_units_based_sensor",
  "memory",
  "drive_slot",
  "power_memory_resize",
  "system_firmware_progress",
  "event_logging_disabled",
  "watchdog_1",
  "system_event",
  "critical_interrupt",
  "button",
  "module_board",
  "microcontroller_coprocessor",
  "add_in_card",
  "chassis",
  "chip_set",
  "other_fru",
  "cable_interconnect",
  "terminator",
  "system_boot_initiated",
  "boot_error",
  "os_boot",
  "os_critical_stop",
  "slot_connector",
  "system_acpi_power_state",
  "watchdog_2",
  "platform_alert",
  "entity_presense",
  "monitor_asic_ic",
  "lan",
  "management_subsystem_health",
  "battery",
};


const char *
IpmiSensorTypeToString( tIpmiSensorType val )
{
  if ( val > eIpmiSensorTypeBattery )
     {
       if ( val == eIpmiSensorTypeAtcaHotSwap )
            return "atca_hotswap";

       return "invalid";
     }

  return sensor_types[val];
}


static const char *hysteresis_support_types[] =
{
    "none",
    "readable",
    "settable",
    "fixed",
};

const char *
IpmiHysteresisSupportToString( tIpmiHysteresisSupport val )
{
  if ( val > eIpmiHysteresisSupportFixed )
       return "invalid";
 
  return hysteresis_support_types[val];
}


static const char *threshold_access_support_types[] =
{
    "none",
    "readable",
    "settable",
    "fixed",
};

const char *
IpmiThresholdAccessSupportToString( tIpmiThresholdAccessSuport val )
{
  if ( val > eIpmiThresholdAccessSupportFixed )
       return "invalid";

  return threshold_access_support_types[val];
}


static const char *event_support_types[] =
{
    "per_state",
    "entire_sensor",
    "global_disable",
    "none",
};


const char *
IpmiEventSupportToString( tIpmiEventSupport val )
{
  if ( val > eIpmiEventSupportNone )
       return "invalid";

  return event_support_types[val];
}


static const char *event_reading_types[] =
{
  "unspecified",
  "threshold",
  "discrete_usage",
  "discrete_state",
  "discrete_predictive_failure",
  "discrete_limit_exceeded",
  "discrete_performance_met",
  "discrete_severity",
  "discrete_device_presense",
  "discrete_device_enable",
  "discrete_availability",
  "discrete_redundancy",
  "discrete_acpi_power",
};

const char *
IpmiEventReadingTypeToString( tIpmiEventReadingType val )
{
  if ( val == eIpmiEventReadingTypeSensorSpecific )
       return "sensor specific";
  
  if ( val > eIpmiEventReadingTypeDiscreteAcpiPower )
       return "invalid";

  return event_reading_types[val];
}


static const char *rate_unit[] =
{
  "None",
  "Us"
  "ms"
  "s",
  "minute",
  "hour",
  "day"
};

#define dNumRateUnit (sizeof(rate_unit)/sizeof(char *))

const char *
IpmiRateUnitToString( tIpmiRateUnit val )
{
  if ( val > (int)dNumRateUnit )
       return "Invalid";

  return rate_unit[val];  
}


static const char *unit_types[] =
{
  "unspecified",
  "C",
  "F",
  "K",
  "volts",
  "amps",
  "watts",
  "joules",
  "coulombs",
  "VA",
  "nits",
  "lumens",
  "lux",
  "candela",
  "kpa",
  "PSI",
  "newtons",
  "CFM",
  "RPM",
  "HZ",
  "useconds",
  "mseconds",
  "seconds",
  "minute",
  "hour",
  "day",
  "week",
  "mil",
  "inches",
  "feet",
  "cubic inchs",
  "cubic feet",
  "millimeters",
  "centimeters",
  "meters",
  "cubic centimeters"
  "cubic meters",
  "liters",
  "fluid ounces",
  "radians",
  "seradians",
  "revolutions",
  "cycles",
  "gravities",
  "ounces",
  "pounds",
  "foot pounds",
  "ounce inches",
  "gauss",
  "gilberts",
  "henries",
  "mhenries",
  "farads",
  "ufarads",
  "ohms",
  "siemens",
  "moles",
  "becquerels",
  "PPM",
  "unspecified",
  "decibels",
  "DbA",
  "DbC",
  "grays",
  "sieverts",
  "color temp deg K",
  "bits",
  "kbits",
  "mbits",
  "gbits",
  "bytes",
  "kbytes",
  "mbytes",
  "gbytes",
  "words",
  "dwords",
  "qwords",
  "lines",
  "hits",
  "misses",
  "retries",
  "resets",
  "overruns",
  "underruns",
  "collisions",
  "packets",
  "messages",
  "characters",
  "errors",
  "correctable_errors",
  "uncorrectable_errors"
};

#define dNumUnitTypes (sizeof(unit_types)/sizeof(char *))


const char *
IpmiUnitTypeToString( tIpmiUnitType val )
{
  if ( val > (int)dNumUnitTypes )
       return "invalid";

  return unit_types[val];
}


static const char *fru_state[] =
{
  "not installed",
  "inactive",
  "activation request",
  "activation in progress",
  "active",
  "deactivation request",
  "deactivation in progress",
  "communication lost"
};

const char *
IpmiFruStateToString( tIpmiFruState val )
{
  if ( val > eIpmiFruStateCommunicationLost )
       return "invalid";

  return fru_state[val];
}


cIpmiSensor *
IpmiMcFindSensor( cIpmiMc *mc, unsigned int lun, unsigned int sensor_num )
{
  cIpmiSensorInfo *sensors = mc->Sensors();

  if ( lun > 4 )
     {
       assert( 0 );
       return 0;
     }

  if ( sensor_num > (unsigned int)sensors->m_idx_size[lun] )
       return 0;

  return sensors->m_sensors_by_idx[lun][sensor_num];
}


void
cIpmiSensor::Event( cIpmiEvent *event )
{
  tIpmiEventDir dir;

  if ( m_event_reading_type == eIpmiEventReadingTypeThreshold )
     {
       tIpmiThresh        threshold;
       tIpmiEventValueDir high_low;

       dir       = (tIpmiEventDir)(event->m_data[9] >> 7);
       threshold = (tIpmiThresh)((event->m_data[10] >> 1) & 0x07);
       high_low  = (tIpmiEventValueDir)(event->m_data[10] & 1);

       m_mc->Domain()->IfSensorThresholdEvent( this, dir, threshold, high_low, event );

       return;
     }

  // discrete sensor
  int offset;
  int severity = -1;
  int prev_severity = -1;

  dir    = (tIpmiEventDir)(event->m_data[9] >> 7);
  offset = event->m_data[10] & 0x0f;

  if ( (event->m_data[10] >> 6 ) == 2 )
     {
       severity = event->m_data[11] >> 4;
       prev_severity = event->m_data[11] & 0xf;

       if ( severity == 0xf )
            severity = -1;

       if ( prev_severity == 0xf )
            prev_severity = -11;
     }

  m_mc->Domain()->IfSensorDiscreteEvent( this, dir, offset,
                                         severity,
                                         prev_severity,
                                         event );
}


bool
cIpmiSensor::Ignore()
{
  // not ipmlemented 
  return false;
}


void 
cIpmiSensor::Log()
{
  cIpmiEntity *ent = GetEntity();

  IpmiLog( "mc = 0x%02x, num 0x%02x, %d.%d (%s), %s\n",
           m_mc->GetAddress(), m_num,
           ent->EntityId(), ent->EntityInstance(),
           ent->EntityIdString(), m_id );

  IpmiLog( "\tthreshold_access %s, hysteresis_support %s, event_support %s\n",
           IpmiThresholdAccessSupportToString( m_threshold_access ),
           IpmiHysteresisSupportToString( m_hysteresis_support ),
           IpmiEventSupportToString( m_event_support ) );
}
