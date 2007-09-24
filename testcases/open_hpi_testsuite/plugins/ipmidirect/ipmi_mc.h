/*
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

#ifndef dIpmiSensorHotswap_h
#include "ipmi_sensor_hotswap.h"
#endif

#ifndef dIpmiSel_h
#include "ipmi_sel.h"
#endif

#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif

#ifndef dIpmiMcVendor_h
#include "ipmi_mc_vendor.h"
#endif

#ifndef dIpmiInventory_h
#include "ipmi_inventory.h"
#endif

class cIpmiDomain;
class cIpmiMcThread;


class cIpmiMc : cArray<cIpmiResource>
{
protected:
  cIpmiMcVendor *m_vendor;

  cIpmiAddr      m_addr;

  // If the MC is known to be good in the system, then active is
  // true.  If active is false, that means that there are sensors
  // that refer to this MC, but the MC is not currently in the
  // system.
  bool           m_active;

  cIpmiDomain   *m_domain;

  cIpmiSdrs     *m_sdrs;

  // The sensors that came from the device SDR on this MC.
  GList         *m_sensors_in_my_sdr;

  // The system event log, for querying and storing events.
  cIpmiSel      *m_sel;

  // PICMG version for ATCA boards
  unsigned char m_picmg_major;
  unsigned char m_picmg_minor;

  // The rest is the actual data from the get device id and SDRs.
  unsigned char  m_device_id;

  unsigned char  m_device_revision;

  bool           m_provides_device_sdrs;
  bool           m_device_available;

  unsigned char  m_device_support;
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

  bool           m_is_tca_mc;
  bool           m_is_rms_board;

  SaErrorT SendSetEventRcvr( unsigned int addr );

public:
  void AddResource( cIpmiResource *res );
  void RemResource( cIpmiResource *res );
  cIpmiResource *FindResource( const cIpmiEntityPath &ep );
  cIpmiResource *FindResource( cIpmiResource *res );
  cIpmiResource *GetResource( int i );
  int           NumResources() const { return Num(); }
public:
  cIpmiMc( cIpmiDomain *domain, const cIpmiAddr &addr );
  virtual ~cIpmiMc();

  cIpmiDomain *Domain() const { return m_domain; }

  unsigned char DeviceRevision() const { return m_device_revision; }
  unsigned char DeviceSupport() const { return m_device_support; }

  unsigned char MajorFwRevision() const { return m_major_fw_revision; }
  unsigned char MinorFwRevision() const { return m_minor_fw_revision; }

  unsigned char MajorVersion()    const { return m_major_version; }
  unsigned char MinorVersion()    const { return m_minor_version; }

  unsigned int  ManufacturerId()  const { return m_manufacturer_id; }
  unsigned short ProductId()      const { return m_product_id; }

  unsigned char AuxFwRevision( int v ) const
  {
    if ( v >= 0 && v < 4 )
        return m_aux_fw_revision[v];
    else
        return 0;
  }

  const cIpmiAddr &Addr() { return m_addr; }

  void CheckTca();

  bool IsTcaMc() { return m_is_tca_mc; }

  bool &IsRmsBoard() { return m_is_rms_board; }

  void SetSel( bool sel ) { m_sel_device_support = sel; }

  cIpmiSensorHotswap *FindHotswapSensor();

  GList *GetSdrSensors() const
  {
    return m_sensors_in_my_sdr;
  }

  void SetSdrSensors( GList *sensors )
  {
    m_sensors_in_my_sdr = sensors;
  }

  bool ProvidesDeviceSdrs() const { return m_provides_device_sdrs; }
  void SetProvidesDeviceSdrs(bool val) { m_provides_device_sdrs = val; }

  bool &SdrRepositorySupport() { return m_sdr_repository_support; }
  bool SelDeviceSupport() const { return m_sel_device_support; }

  cIpmiSel *Sel() const { return m_sel; }

  bool Cleanup(); // true => it is safe to destroy mc

  SaErrorT HandleNew();
  bool     DeviceDataCompares( const cIpmiMsg &msg ) const;
  int      GetDeviceIdDataFromRsp( const cIpmiMsg &msg );
  void     CheckEventRcvr();
  SaErrorT SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp_msg,
                        unsigned int lun = 0, int retries = dIpmiDefaultRetries );

  unsigned int GetChannel() const;
  unsigned int GetAddress() const;
  void         SetActive();
  cIpmiSensor *FindSensor( unsigned int lun, unsigned int sensor_id );
  cIpmiRdr *FindRdr( cIpmiRdr *r );

  void SetVendor( cIpmiMcVendor *mv )
  {
    if ( mv )
        m_vendor = mv;
  }

  cIpmiMcVendor *GetVendor()  { return m_vendor; }
protected:
  bool DumpFrus( cIpmiLog &dump, const char *name ) const;
  bool DumpControls( cIpmiLog &dump, const char *name ) const;

public:
  void Dump( cIpmiLog &dump, const char *name ) const;

  // create and populate resources and rdrs
  virtual bool Populate();
};


#endif
