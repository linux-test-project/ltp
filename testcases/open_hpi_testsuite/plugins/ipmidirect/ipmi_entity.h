/*
 * ipmi_entity.h
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

#ifndef dIpmiEntity_h
#define dIpmiEntity_h


#include <glib.h>
#include <string.h>


extern "C" {
#include "SaHpi.h"
}


#ifndef dIpmiEvent_h
#include "ipmi_event.h"
#endif


enum tIpmiEntityId
{
  eIpmiEntityInvalid                        =  0,
  eIpmiEntityIdOther                        =  1,
  eIpmiEntityIdUnkown                       =  2,
  eIpmiEntityIdProcessor                    =  3,
  eIpmiEntityIdDisk                         =  4,
  eIpmiEntityIdPeripheral                   =  5,
  eIpmiEntityIdSystemManagementModule       =  6,
  eIpmiEntityIdSystemBoard                  =  7,
  eIpmiEntityIdMemoryModule                 =  8,
  eIpmiEntityIdProcessorModule              =  9,
  eIpmiEntityIdPowerSupply                  = 10,
  eIpmiEntityIdAddInCard                    = 11,
  eIpmiEntityIdFrontPanelBoard              = 12,
  eIpmiEntityIdBackPanelBoard               = 13,
  eIpmiEntityIdPowerSystemBoard             = 14,
  eIpmiEntityIdDriveBackplane               = 15,
  eIpmiEntityIdSystemInternalExpansionBoard = 16,
  eIpmiEntityIdOtherSystemBoard             = 17,
  eIpmiEntityIdProcessorBoard               = 18,
  eIpmiEntityIdPowerUnit                    = 19,
  eIpmiEntityIdPowerModule                  = 20,
  eIpmiEntityIdPowerManagementBoard         = 21,
  eIpmiEntityIdChassisBackPanelBoard        = 22,
  eIpmiEntityIdSystemChassis                = 23,
  eIpmiEntityIdSubChassis                   = 24,
  eIpmiEntityIdOtherChassisBoard            = 25,
  eIpmiEntityIdDiskDriveBay                 = 26,
  eIpmiEntityIdPeripheralBay                = 27,
  eIpmiEntityIdDeviceBay                    = 28,
  eIpmiEntityIdFanCooling                   = 29,
  eIpmiEntityIdCoolingUnit                  = 30,
  eIpmiEntityIdCableInterconnect            = 31,
  eIpmiEntityIdMemoryDevice                 = 32,
  eIpmiEntityIdSystemManagementSoftware     = 33,
  eIpmiEntityIdBios                         = 34,
  eIpmiEntityIdOperatingSystem              = 35,
  eIpmiEntityIdSystemBus                    = 36,
  eIpmiEntityIdGroup                        = 37,
  eIpmiEntityIdRemoteMgmtCommDevice         = 38,
  eIpmiEntityIdExternalEnvironment          = 39,
  eIpmiEntityIdBattery                      = 40,
  eIpmiEntityIdProcessingBlade              = 41,
  eIpmiEntityIdConnectivitySwitch           = 42,
  eIpmiEntityIdProcessorMemoryModule        = 43,
  eIpmiEntityIdIoModule                     = 44,
  eIpmiEntityIdProcessorIoModule            = 45,
  eIpmiEntityIdMgmtControllerFirmware       = 46,

  // PIGMIG entity ids
  eIpmiEntityIdPicMgFrontBoard              = 0xa0,
  eIpmiEntityIdPicMgRearTransitionModule    = 0xc0,
  eIpmiEntityIdPicMgAdvancedMcModule        = 0xc1,
  eIpmiEntityIdPicMgMicroTcaCarrierHub      = 0xc2,
  eIpmiEntityIdPicmgShelfManager            = 0xf0,
  eIpmiEntityIdPicmgFiltrationUnit          = 0xf1,
  eIpmiEntityIdPicmgShelfFruInformation     = 0xf2,
  eIpmiEntityIdPicmgAlarmPanel              = 0xf3,
};

const char *IpmiEntityIdToString( tIpmiEntityId id );


// wrapper class for entity path
class cIpmiEntityPath
{
public:
  SaHpiEntityPathT m_entity_path;

  cIpmiEntityPath();
  cIpmiEntityPath( const SaHpiEntityPathT &entity_path );

  operator SaHpiEntityPathT()
  {
    return m_entity_path;
  }

  void SetEntry( int idx, SaHpiEntityTypeT type, SaHpiEntityLocationT instance );
  SaHpiEntityTypeT GetEntryType( int idx );
  void SetEntryType( int idx, SaHpiEntityTypeT type );

  SaHpiEntityLocationT GetEntryInstance( int idx );
  void SetEntryInstance( int idx, SaHpiEntityLocationT instance );

  cIpmiEntityPath &operator+=( const cIpmiEntityPath &epath );
  bool operator==( const cIpmiEntityPath &epath ) const;
  bool operator!=( const cIpmiEntityPath &epath ) const;
  
  void AppendRoot( int idx );

  bool FromString( const char *str );
};


cIpmiLog &operator<<( cIpmiLog &log, const cIpmiEntityPath &epath );


#endif
