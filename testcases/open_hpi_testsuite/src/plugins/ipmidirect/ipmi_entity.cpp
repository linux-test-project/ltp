/*
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
#include <stdlib.h>
#include <glib.h>
#include <assert.h>

#include "ipmi_mc.h"
#include "ipmi_entity.h"
#include "ipmi_event.h"
#include "ipmi_log.h"
#include "ipmi_domain.h"


cIpmiEntityInfo::cIpmiEntityInfo( cIpmiDomain *domain )
  : m_domain( domain ), m_entities( 0 )
{
}


cIpmiEntityInfo::~cIpmiEntityInfo()
{
  while( m_entities )
     {
       cIpmiEntity *ent = (cIpmiEntity *)m_entities->data;
     
       m_entities = g_list_remove( m_entities, ent );
       delete ent;
     }  
}


cIpmiEntity *
cIpmiEntityInfo::VerifyEntify( cIpmiEntity *ent )
{
  if ( g_list_find( m_entities, ent ) )
       return ent;

  return 0;
}


cIpmiSensor *
cIpmiEntityInfo::VerifySensor( cIpmiSensor *s )
{
  for( GList *list = m_entities; list; list = g_list_next( list ) )
     {
       cIpmiEntity *ent = (cIpmiEntity *)list->data;
       
       if ( ent->VerifySensor( s ) )
            return s;
     }

  return 0;
}


cIpmiFru *
cIpmiEntityInfo::VerifyFru( cIpmiFru *f )
{
  for( GList *list = m_entities; list; list = g_list_next( list ) )
     {
       cIpmiEntity *ent = (cIpmiEntity *)list->data;
       
       if ( ent->VerifyFru( f ) )
            return f;
     }

  return 0;
}


cIpmiEntity *
cIpmiEntityInfo::Find( tIpmiDeviceNum device_num,
                       tIpmiEntityId entity_id, int entity_instance )
{
  GList *list = m_entities;

  while( list )
     {
       cIpmiEntity *ent = (cIpmiEntity *)list->data;

       if (    ( ent->DeviceNum().channel == device_num.channel )
	    && ( ent->DeviceNum().address == device_num.address )
	    && ( ent->EntityId()          == entity_id )
            && ( ent->EntityInstance()    == entity_instance ) )
            return ent;

       list  = g_list_next( list );
     }
  
  return 0;
}


cIpmiEntity *
cIpmiEntityInfo::Find( cIpmiMc *mc,
                       tIpmiEntityId entity_id, int entity_instance )
{
  tIpmiDeviceNum device_num;

  if ( mc && entity_instance >= 0x60 )
     {
       device_num.channel = mc->GetChannel();
       device_num.address = mc->GetAddress();
       entity_instance   -= 0x60;
     }
  else
     {
       device_num.channel = 0;
       device_num.address = 0;
     }

  return Find( device_num, entity_id, entity_instance );
}


cIpmiEntity *
cIpmiEntityInfo::Add( tIpmiDeviceNum device_num,
                      tIpmiEntityId  entity_id, int entity_instance,
                      bool came_from_sdr, const char *id,
                      cIpmiMc *mc, int lun )
{
  cIpmiEntity *ent = Find( device_num, entity_id, entity_instance );

  if ( ent )
     {
       // If it came from an SDR, it always will have come from an SDR.
       if ( !ent->CameFromSdr() )
	    ent->CameFromSdr() = came_from_sdr;

       ent->SetId( id );
       ent->AccessAddress() = mc->GetAddress();
       ent->Channel()       = mc->GetChannel();
       ent->Lun()           = lun;

       return ent;
    }

  ent = new cIpmiEntity( this, device_num, entity_id, entity_instance, came_from_sdr );
  assert( ent );

  m_entities = g_list_append( m_entities, ent );

  ent->SetId( id );
  ent->AccessAddress() = mc->GetAddress();
  ent->Channel()       = mc->GetChannel();
  ent->Lun()           = lun;

  // XXXX add entity
  m_domain->IfEntityAdd( ent );

  return ent;
}


cIpmiEntity *
cIpmiEntityInfo::Add( cIpmiMc *mc, int lun, 
                      tIpmiEntityId entity_id, int entity_instance,
                      const char *id )
{
  tIpmiDeviceNum device_num;
  cIpmiEntity *ent;

  if ( mc && entity_instance >= 0x60 )
     {
       device_num.channel = mc->GetChannel();
       device_num.address = mc->GetAddress();
       entity_instance -= 0x60;
     }
  else
     {
       device_num.channel = 0;
       device_num.address = 0;
     }

  ent = Add( device_num, entity_id, entity_instance, false, id, mc, lun );

  if ( !ent )
       return 0;

  return ent;
}


void
cIpmiEntityInfo::Rem( cIpmiEntity *ent )
{
  m_entities = g_list_remove( m_entities, ent );
}


cIpmiEntity::cIpmiEntity( cIpmiEntityInfo *ents, tIpmiDeviceNum device_num,
                          tIpmiEntityId entity_id, int entity_instance,
                          bool came_from_sdr )
  : m_domain( ents->Domain() ),
    m_access_address( 0 ),
    m_slave_address( 0 ), 
    m_channel( 0 ),
    m_lun( 0 ),
    m_private_bus_id( 0 ),
    m_is_fru( false ), m_is_mc( false ),
    m_came_from_sdr( came_from_sdr ),
    m_is_logical_fru( false ), m_fru_id( 0 ),
    m_hs_state( SAHPI_HS_STATE_NOT_PRESENT ),
    m_acpi_system_power_notify_required( false ),
    m_acpi_device_power_notify_required( false ),
    m_controller_logs_init_agent_errors( false ),
    m_log_init_agent_errors_accessing( false ),
  //  unsigned int global_init : 2,
    m_chassis_device( false ),
    m_bridge( false ),
    m_ipmb_event_generator( false ),
    m_ipmb_event_receiver( false ),
    m_fru_inventory_device( false ),
    m_sel_device( false ),
    m_sdr_repository_device( false ),
    m_sensor_device( false ),
    m_address_span( 0 ), m_device_num( device_num ),
    m_entity_id( entity_id ),
    m_entity_instance( entity_instance ),
    m_device_type( 0 ), m_device_modifier( 0 ),
    m_oem( 0 ), m_presence_sensor_always_there( false ),
    m_entity_id_string( 0 ),
    m_presence_possibly_changed( true ),
    m_ents( ents ), m_sensors( 0 ), m_frus( 0 ), m_sel( 0 )
{
  m_id[0] = 0;

  m_entity_id_string = IpmiEntityIdToString( entity_id );
}


cIpmiEntity::~cIpmiEntity()
{
  while( m_sensors )
       m_sensors = g_list_remove( m_sensors, m_sensors->data );

  while( m_frus )
     {
       delete (cIpmiFru *)m_frus->data;
       m_frus = g_list_remove( m_sensors, m_frus->data );
     }

  if ( m_sel )
     {
       // remove connection
       m_sel->Entity() = 0;
       m_sel = 0;
     }
}


cIpmiSensor *
cIpmiEntity::VerifySensor( cIpmiSensor *s )
{
  for( GList *list = m_sensors; list; list = g_list_next( list ) )
     {
       cIpmiSensor *sensor = (cIpmiSensor *)list->data;

       if ( s == sensor )
            return s;
     }

  return 0;
}


cIpmiFru *
cIpmiEntity::VerifyFru( cIpmiFru *f )
{
  for( GList *list = m_frus; list; list = g_list_next( list ) )
     {
       cIpmiFru *fru = (cIpmiFru *)list->data;

       if ( f == fru )
            return f;
     }

  return 0;
}


bool
cIpmiEntity::Destroy()
{
  // XXXX remove ent
  m_domain->IfEntityRem( this );

  // Remove it from the entities list.
  m_ents->Rem( this );

  // The sensor and control lists should be empty now, we can just
  // destroy it.
  delete this;

  return true;
}


static const char *entity_id_types[] =
{
  "unspecified",
  "other",
  "unkown",
  "processor",
  "disk",
  "peripheral",
  "system_management_module",
  "system_board",
  "memory_module",
  "processor_module",
  "power_supply",
  "add_in_card",
  "front_panel_board",
  "back_panel_board",
  "power_system_board",
  "drive_backplane",
  "system_internal_expansion_board",
  "other_system_board",
  "processor_board",
  "power_unit",
  "power_module",
  "power_management_board",
  "chassis_back_panel_board",
  "system_chassis",
  "sub_chassis",
  "other_chassis_board",
  "disk_drive_bay",
  "peripheral_bay",
  "device_bay",
  "fan_cooling",
  "cooling_unit",
  "cable_interconnect",
  "memory_device",
  "system_management_software",
  "bios",
  "operating_system",
  "system_bus",
  "group",
  "remote_mgmt_comm_device",
  "external_environment",
  "battery",
  "processing blade",
  "connectivity switch",
  "processor/memory module",
  "I/O module",
  "processor I/O module",
  "management controller firmware",
};

#define dNumEntityIdTypes (sizeof(entity_id_types)/sizeof(char *))


const char *
IpmiEntityIdToString( tIpmiEntityId val )
{
  if ( (unsigned int)val < dNumEntityIdTypes )
       return entity_id_types[val];

  switch( val )
     {
       case eIpmiEntityIdPigMgFrontBoard:
            return "PIGMG front board";

       case eIpmiEntityIdPigMgRearTransitionModule:
            return "PIGMG rear transition module";

       case eIpmiEntityIdAtcaShelfManager:
            return "ATCA shelf manager";
            
       case eIpmiEntityIdAtcaFiltrationUnit:
            return "ATCA filtration unit";

       default:
            return "invalid";
     }

  // not reached
  return "invalid";
}


void
cIpmiEntity::AddSensor( cIpmiSensor *sensor )
{
/*
  if ( sensor->SensorType() == eIpmiSensorTypeAtcaHotSwap )
     {
       // read fru state
       tIpmiStates states;

       int rv = sensor->StatesGet( states );

       if ( !rv )
	  {
	    int state = -1;
	    int i;

	    for( i = 0; i < 8; i++ )
		 if ( states.m_states & ( 1 << i ) )
		    {
		      assert( state == -1 );
		      state = i;
		    }

	    switch( state )
	       {
		 case eIpmiFruStateNotInstalled:
		 case eIpmiFruStateCommunicationLost:
		 case eIpmiFruStateDeactivationInProgress:
		      m_hs_state = SAHPI_HS_STATE_NOT_PRESENT;
		      break;

		 case eIpmiFruStateInactive:
                      // if the initial state is inactive
                      // => the lock bit is set or ejector open
                      //    or insertion criteria not met
		      m_hs_state = SAHPI_HS_STATE_INACTIVE;
		      break;

		 case eIpmiFruStateActivationRequest:
		      m_hs_state = SAHPI_HS_STATE_INSERTION_PENDING;
		      break;

		 case eIpmiFruStateActivationInProgress:
		 case eIpmiFruStateActive:
		      m_hs_state = SAHPI_HS_STATE_ACTIVE_HEALTHY;
		      break;

		 case eIpmiFruStateDeactivationRequest:
		      m_hs_state = SAHPI_HS_STATE_EXTRACTION_PENDING;
                      break;
	       }

	    IpmiLog( "adding hotswap sensor to entity: M%d\n", state );
	  }
     }
*/

  m_sensors = g_list_append( m_sensors, sensor );

  // XXXX add a rdr
  m_domain->IfSensorAdd( this, sensor );
}


void
cIpmiEntity::RemoveSensor( cIpmiSensor *sensor )
{
  GList *item = g_list_find( m_sensors, sensor );

  if ( item == 0 )
     {
       IpmiLog( "User requested removal of a sensor"
                " from an entity, but the sensor was not there");
       return;
     }

  m_sensors = g_list_remove( m_sensors, sensor );

  // XXXX remove rdr
  m_domain->IfSensorRem( this, sensor );

  if (    (!m_came_from_sdr )
       && m_sensors == 0 )
       Destroy();
}


cIpmiFru *
cIpmiEntity::FindFru( unsigned int fru_id )
{
  GList *item = g_list_first( m_frus );

  while( item )
     {
       cIpmiFru *fru = (cIpmiFru *)item->data;

       if ( fru->FruId() == fru_id )
            return fru;

       item = g_list_next( item );
     }

  return 0;
}


void
cIpmiEntity::AddFru( cIpmiFru *fru )
{
  m_frus = g_list_append( m_frus, fru );
}
