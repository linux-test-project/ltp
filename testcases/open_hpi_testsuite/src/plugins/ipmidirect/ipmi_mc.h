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

#ifndef dIpmiMc_h
#define dIpmiMc_h


#include <glib.h>
#include <sys/time.h>


#ifndef dIpmiSdr_h
#include "ipmi_sdr.h"
#endif

#ifndef dIpmiAddr_h
#include "ipmi_addr.h"
#endif

#ifndef dIpmiMsg_h
#include "ipmi_msg.h"
#endif

#ifndef dIpmiSensor_h
#include "ipmi_sensor.h"
#endif

#ifndef dIpmiSel_h
#include "ipmi_sel.h"
#endif


class cIpmiDomain;


class cIpmiMc
{
protected:
  cIpmiAddr      m_addr;

  // If the MC is known to be good in the system, then active is
  // true.  If active is false, that means that there are sensors
  // that refer to this MC, but the MC is not currently in the
  // system.
  bool           m_active;

  // current fru state
  tIpmiFruState m_fru_state;

  cIpmiDomain   *m_domain;

  cIpmiSdrs     *m_sdrs;

  // The sensors that came from the device SDR on this MC.
  cIpmiSensor  **m_sensors_in_my_sdr;
  unsigned int   m_sensors_in_my_sdr_count;

  // Sensors that this MC owns (you message this MC to talk to this
  // sensor, and events report the MC as the owner.
  cIpmiSensorInfo *m_sensors;

  // The system event log, for querying and storing events.
  bool           m_sel_rescan; // true => sel will be periodical scanned
  cIpmiSel      *m_sel;

  // The rest is the actual data from the get device id and SDRs.
  // There's the real version and the normal version, the real
  // version is the one from the get device id response, the normal
  // version may have been adjusted by the OEM code. */
  unsigned char  m_device_id;

  unsigned char  m_device_revision;

  bool           m_provides_device_sdrs;
  bool           m_device_available;

  bool           m_chassis_support;
  bool           m_bridge_support;
  bool           m_ipmb_event_generator_support;
  bool           m_ipmb_event_receiver_support;
  bool           m_fru_inventory_support;
  bool           m_sel_device_support;
  bool           m_sdr_repository_support;
  bool           m_sensor_device_support;

  unsigned char  m_major_fw_revision;
  unsigned char  m_minor_fw_revision;

  unsigned char  m_major_version;
  unsigned char  m_minor_version;

  unsigned int   m_manufacturer_id;
  unsigned short m_product_id;

  unsigned char  m_aux_fw_revision[4];

  unsigned char  m_real_device_id;

  unsigned char  m_real_device_revision;

  bool           m_real_provides_device_sdrs;
  bool           m_real_device_available;

  bool           m_real_chassis_support;
  bool           m_real_bridge_support;
  bool           m_real_ipmb_event_generator_support;
  bool           m_real_ipmb_event_receiver_support;
  bool           m_real_fru_inventory_support;
  bool           m_real_sel_device_support;
  bool           m_real_sdr_repository_support;
  bool           m_real_sensor_device_support;

  unsigned char  m_real_major_fw_revision;
  unsigned char  m_real_minor_fw_revision;

  unsigned char  m_real_major_version;
  unsigned char  m_real_minor_version;

  unsigned int   m_real_manufacturer_id;
  unsigned short m_real_product_id;

  unsigned char  m_real_aux_fw_revision[4];

  void SendSetEventRcvr( unsigned int addr );

public:
  cIpmiMc( cIpmiDomain *ipmi, const cIpmiAddr &addr );
  ~cIpmiMc();

  cIpmiDomain *Domain() const { return m_domain; }
  cIpmiSensorInfo *Sensors() const { return m_sensors; }

  unsigned char MajorFwRevision() const { return m_major_fw_revision; }
  unsigned char MinorFwRevision() const { return m_minor_fw_revision; }

  unsigned char MajorVersion()    const { return m_major_version; }
  unsigned char MinorVersion()    const { return m_minor_version; }

  unsigned int  ManufacturerId()  const { return m_manufacturer_id; }
  unsigned short ProductId()      const { return m_product_id; }

  unsigned char AuxFwRevision( int v ) const
  {
    assert( v >= 0 && v < 4 );
    return m_aux_fw_revision[v];
  }

  tIpmiFruState &FruState() { return m_fru_state; }

  const cIpmiAddr &Addr() { return m_addr; }

  bool IsActive() const { return m_active; }
  void SetActive( bool active ) { m_active = active; }

  cIpmiSensor **GetSdrSensors( unsigned int &count ) const
  {
    count = m_sensors_in_my_sdr_count;
    return m_sensors_in_my_sdr;
  }

  void SetSdrSensors( cIpmiSensor **sensors,
                      unsigned int  count )
  {
    m_sensors_in_my_sdr_count = count;
    m_sensors_in_my_sdr       = sensors;
  }

  bool ProvidesDeviceSdrs() { return m_provides_device_sdrs; }

  bool &SdrRepositorySupport() { return m_sdr_repository_support; }
  bool SelDeviceSupport() const { return m_sel_device_support; }

  cIpmiSel *Sel() { return m_sel; }

  bool Cleanup(); // true => it is safe to destroy mc

  void     HandleNew();
  bool     DeviceDataCompares( const cIpmiMsg &msg ) const;
  int      GetDeviceIdDataFromRsp( const cIpmiMsg &msg );
  void     CheckEventRcvr();
  int      SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp_msg, unsigned int lun = 0 );

  unsigned int GetChannel();
  unsigned int GetAddress();
  void         SetActive();
  cIpmiSensor *FindSensor( unsigned int lun, unsigned int sensor_id );
};


#endif
