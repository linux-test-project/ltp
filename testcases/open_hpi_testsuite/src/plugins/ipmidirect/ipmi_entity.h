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


#ifndef dIpmiEntity_h
#define dIpmiEntity_h


#include <glib.h>


__BEGIN_DECLS
#include "SaHpi.h"
__END_DECLS


#ifndef dIpmiEvent_h
#include "ipmi_event.h"
#endif

#ifndef dIpmiFru_h
#include "ipmi_fru.h"
#endif


class cIpmiSensor;
class cIpmiSel;


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
  eIpmiEntityIdPigMgFrontBoard              = 0xa0,
  eIpmiEntityIdPigMgRearTransitionModule    = 0xc0,
  eIpmiEntityIdAtcaShelfManager             = 0xf0,
  eIpmiEntityIdAtcaFiltrationUnit           = 0xf1,
};

const char *IpmiEntityIdToString( tIpmiEntityId id );


class cIpmiDomain;
class cIpmiEntity;


struct tIpmiDeviceNum
{
  unsigned char channel;
  unsigned char address;
};


class cIpmiEntityInfo
{
protected:
  cIpmiDomain *m_domain;
  GList       *m_entities;

  cIpmiEntity *Add( tIpmiDeviceNum device_num,
                    tIpmiEntityId entity_id, int entity_instance,
                    bool came_from_sdr, const char *id,
                    cIpmiMc *mc, int lun );
  cIpmiEntity *Find( tIpmiDeviceNum device_num,
                     tIpmiEntityId entity_id, int entity_instance );

public:
  cIpmiEntityInfo( cIpmiDomain *domain );
  ~cIpmiEntityInfo();

  cIpmiDomain *Domain() { return m_domain; }

  cIpmiEntity *VerifyEntify( cIpmiEntity *ent );
  cIpmiSensor *VerifySensor( cIpmiSensor *s );
  cIpmiFru    *VerifyFru( cIpmiFru *f );

  cIpmiEntity *Find( cIpmiMc *mc,
                     tIpmiEntityId entity_id, int entity_instance );

  cIpmiEntity *Add( cIpmiMc *mc, int lun, 
                    tIpmiEntityId entity_id, int entity_instance,
                    const char *id );
  void Rem( cIpmiEntity *ent );
};


class cIpmiEntity
{
protected:
  cIpmiDomain *m_domain;

  // Key fields
  unsigned char m_access_address;
  unsigned char m_slave_address;
  unsigned char m_channel;
  unsigned char m_lun;
  unsigned char m_private_bus_id;

  // misc
  bool m_is_fru;
  bool m_is_mc;

  // If this entity is added by the user or comes from the main SDR
  // repository, we don't delete it automatically if it goes
  // away.
  bool m_came_from_sdr;

  // For FRU device only.
  bool m_is_logical_fru;
  unsigned char m_fru_id;

  // hpi fru state
  SaHpiHsStateT m_hs_state;

  /* For MC device only. */
  bool m_acpi_system_power_notify_required;
  bool m_acpi_device_power_notify_required;
  bool m_controller_logs_init_agent_errors;
  bool m_log_init_agent_errors_accessing;
  //  unsigned int global_init : 2;

  bool m_chassis_device;
  bool m_bridge;
  bool m_ipmb_event_generator;
  bool m_ipmb_event_receiver;
  bool m_fru_inventory_device;
  bool m_sel_device;
  bool m_sdr_repository_device;
  bool m_sensor_device;

  // For generic device only.
  unsigned char m_address_span;

  tIpmiDeviceNum m_device_num;
  tIpmiEntityId  m_entity_id;
  unsigned char  m_entity_instance;

  unsigned char m_device_type;
  unsigned char m_device_modifier;
  unsigned char m_oem;

  char m_id[33];

  bool m_presence_sensor_always_there;

  const char   *m_entity_id_string;

  /*
  tIpmiSensor  *presence_sensor;
  int           present;
  */
  int           m_presence_possibly_changed;

  cIpmiEntityInfo *m_ents;

  // list of sensors in this entity
  GList *m_sensors;

  // list of fru inventory
  GList *m_frus;

  // SEL
  cIpmiSel *m_sel;

public:
  cIpmiEntity( cIpmiEntityInfo *ents, tIpmiDeviceNum device_num,
               tIpmiEntityId entity_id, int entity_instance,
               bool came_from_sdr );
  ~cIpmiEntity();

  unsigned char &AccessAddress() { return m_access_address; }
  unsigned char SlaveAddress()   { return m_slave_address; }
  unsigned char &Channel()       { return m_channel; }
  unsigned char &Lun()           { return m_lun; }

  bool &CameFromSdr() { return m_came_from_sdr; }

  unsigned char FruId() { return m_fru_id; }

  tIpmiDeviceNum DeviceNum() { return m_device_num; }
  cIpmiDomain *Domain() { return m_domain; }

  const char   *EntityIdString() const { return m_entity_id_string; }
  tIpmiEntityId EntityId() const { return m_entity_id; }
  unsigned char EntityInstance() const { return m_entity_instance; }
  SaHpiHsStateT &HsState() { return m_hs_state; }

  void SetId( const char *id )
  {
    strncpy( m_id, id, 32 );
    m_id[32] = '\0';
  }

  cIpmiSensor *VerifySensor( cIpmiSensor *s );

  void AddSensor( cIpmiSensor *sensor );
  void RemoveSensor( cIpmiSensor *sensor );

  bool Destroy();

  // HPI resource id
  SaHpiResourceIdT m_resource_id;

  cIpmiFru *VerifyFru( cIpmiFru *f );
  cIpmiFru *FindFru( unsigned int fru_id );
  void AddFru( cIpmiFru *fru );

  cIpmiSel *&Sel() { return m_sel; }
};


#endif
