/*
 * ipmi_entity.cpp
 *
 * Copyright (c) 2003,2004 by FORCE Computers
 * Copyright (c) 2005 by ESO Technologies.
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
#include <stdlib.h>
#include <glib.h>
#include <assert.h>

#include <oh_utils.h>

#include "ipmi_entity.h"
#include "ipmi_log.h"

#include <oh_utils.h>

static const char *entity_id_types[] =
{
  "Unspecified",
  "Other",
  "Unkown",
  "Processor",
  "Disk",
  "Peripheral",
  "SystemManagementModule",
  "SystemBoard",
  "MemoryModule",
  "ProcessorModule",
  "PowerSupply",
  "AddInCard",
  "FrontPanelBoard",
  "BackPanelBoard",
  "PowerSystemBoard",
  "DriveBackplane",
  "SystemInternalExpansionBoard",
  "OtherSystemBoard",
  "ProcessorBoard",
  "PowerUnit",
  "PowerModule",
  "PowerManagementBoard",
  "ChassisBackPanelBoard",
  "SystemChassis",
  "SubChassis",
  "OtherChassisBoard",
  "DiskDriveBay",
  "PeripheralBay",
  "DeviceBay",
  "FanCooling",
  "CoolingUnit",
  "CableInterconnect",
  "MemoryDevice",
  "SystemManagementSoftware",
  "Bios",
  "OperatingSystem",
  "SystemBus",
  "Group",
  "RemoteMgmtCommDevice",
  "ExternalEnvironment",
  "Battery",
  "ProcessingBlade",
  "ConnectivitySwitch",
  "ProcessorMemoryModule",
  "IoModule",
  "ProcessorIoModule",
  "ManagementControllerFirmware",
};

#define dNumEntityIdTypes (sizeof(entity_id_types)/sizeof(char *))


const char *
IpmiEntityIdToString( tIpmiEntityId val )
{
  if ( (unsigned int)val < dNumEntityIdTypes )
       return entity_id_types[val];

  switch( val )
     {
       case eIpmiEntityIdPicMgFrontBoard:
            return "PicmgFrontBoard";

       case eIpmiEntityIdPicMgRearTransitionModule:
            return "PicmgRearTransitionModule";

       case eIpmiEntityIdPicMgAdvancedMcModule:
            return "PicMgAdvancedMcModule";

       case eIpmiEntityIdPicMgMicroTcaCarrierHub:
            return "PicMgMicroTcaCarrierHub";

       case eIpmiEntityIdPicmgShelfManager:
            return "PicmgShelfManager";

       case eIpmiEntityIdPicmgFiltrationUnit:
            return "PicmgFiltrationUnit";

       case eIpmiEntityIdPicmgShelfFruInformation:
            return "PicmgShelfFruInformation";

       case eIpmiEntityIdPicmgAlarmPanel:
            return "PicmgAlarmPanel";

       default:
           break;
     }

  return "Invalid";
}


cIpmiEntityPath::cIpmiEntityPath()
{
  memset( &m_entity_path, 0, sizeof( SaHpiEntityPathT ) );
}


cIpmiEntityPath::cIpmiEntityPath( const SaHpiEntityPathT &entity_path )
{
  m_entity_path = entity_path;
}


void
cIpmiEntityPath::SetEntry( int idx, SaHpiEntityTypeT type,
                           SaHpiEntityLocationT instance )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  m_entity_path.Entry[idx].EntityType     = type;
  m_entity_path.Entry[idx].EntityLocation = instance;
}


SaHpiEntityTypeT
cIpmiEntityPath::GetEntryType( int idx )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  return m_entity_path.Entry[idx].EntityType;
}


void
cIpmiEntityPath::SetEntryType( int idx, SaHpiEntityTypeT type )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  m_entity_path.Entry[idx].EntityType = type;
}


SaHpiEntityLocationT 
cIpmiEntityPath::GetEntryInstance( int idx )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  return m_entity_path.Entry[idx].EntityLocation;
}


void
cIpmiEntityPath::SetEntryInstance( int idx,
                                   SaHpiEntityLocationT instance )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  m_entity_path.Entry[idx].EntityLocation = instance;
}


cIpmiEntityPath &
cIpmiEntityPath::operator+=( const cIpmiEntityPath &epath )
{
  oh_concat_ep( &m_entity_path, &epath.m_entity_path );

  return *this;
}


bool
cIpmiEntityPath::operator==( const cIpmiEntityPath &epath ) const
{
  SaHpiBoolT cmp_result;

  cmp_result = oh_cmp_ep( &m_entity_path, &epath.m_entity_path );

  if ( cmp_result == SAHPI_TRUE )
      return true;
  else
      return false;
}


bool
cIpmiEntityPath::operator!=( const cIpmiEntityPath &epath ) const
{
  SaHpiBoolT cmp_result;

  cmp_result = oh_cmp_ep( &m_entity_path, &epath.m_entity_path );

  if ( cmp_result == SAHPI_TRUE )
      return false;
  else
      return true;
}


void
cIpmiEntityPath::AppendRoot( int idx )
{
  assert( idx >= 0 && idx < SAHPI_MAX_ENTITY_PATH );

  m_entity_path.Entry[idx].EntityType     = SAHPI_ENT_ROOT;
  m_entity_path.Entry[idx].EntityLocation = 0;
}


cIpmiLog &
operator<<( cIpmiLog &log, const cIpmiEntityPath &epath )
{
  oh_big_textbuffer path_text;
  char str[OH_MAX_TEXT_BUFFER_LENGTH+1];

  oh_decode_entitypath( &epath.m_entity_path, &path_text );

  memcpy( str, path_text.Data, path_text.DataLength );
  str[path_text.DataLength] = 0;

  log << str;

  return log;
}


bool
cIpmiEntityPath::FromString( const char *str )
{
  return oh_encode_entitypath( str, &m_entity_path ) ? false : true;
}
