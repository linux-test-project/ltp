/*
 * ipmi_resource.h
 *
 * Copyright (c) 2004 by FORCE Computers.
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

#ifndef dIpmiResource_h
#define dIpmiResource_h


#ifndef dIpmiSensorHotswap_h
#include "ipmi_sensor_hotswap.h"
#endif

#ifndef dIpmiControl_h
#include "ipmi_control.h"
#endif

#ifndef dArray_h
#include "array.h"
#endif

#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif


class cIpmiResource : cArray<cIpmiRdr>
{
public:
  bool m_sel; // true if this is a resource,
              // which provides access to SEL

  // find a specific rdr
  cIpmiRdr *FindRdr( cIpmiMc *mc, SaHpiRdrTypeT type, unsigned int num, unsigned int lun = 0 );
  cIpmiRdr *FindRdr( cIpmiMc *mc, SaHpiRdrTypeT type, cIpmiRdr *rdr );

  bool AddRdr( cIpmiRdr *rdr );
  bool RemRdr( cIpmiRdr *rdr );
  int FindRdr( cIpmiRdr *rdr ) { return Find( rdr ); }
  int NumRdr() { return Num(); }
  cIpmiRdr *GetRdr( int idx ) { return operator[]( idx ); }

protected:
  cIpmiMc            *m_mc;
  unsigned int        m_fru_id;
  cIpmiEntityPath     m_entity_path;
  bool                m_is_fru;

  cIpmiSensorHotswap *m_hotswap_sensor;

  // state only to create state change Mx -> M0
  // where Mx is m_picmg_fru_state
  tIpmiFruState       m_picmg_fru_state;
  bool                m_policy_canceled;
  SaHpiTimeoutT       m_extract_timeout;
  tIpmiFruState       m_prev_prev_fru_state;

  unsigned int        m_oem;

  // mapping of sensor numbers
  int m_sensor_num[256];

public:
  int CreateSensorNum( SaHpiSensorNumT num );

public:
  cIpmiMc *Mc() const { return m_mc; }
  unsigned int FruId() const { return m_fru_id; }
  tIpmiFruState &PicmgFruState() { return m_picmg_fru_state; }
  bool &PolicyCanceled() { return m_policy_canceled; }
  SaHpiTimeoutT &ExtractTimeout() { return m_extract_timeout; }
  tIpmiFruState &PreviousPrevFruState() { return m_prev_prev_fru_state; }
  cIpmiDomain *Domain() const;
  unsigned int &Oem() { return m_oem; }
  cIpmiEntityPath &EntityPath() { return m_entity_path; }
  bool &IsFru() { return m_is_fru; }

protected:
  cIpmiTextBuffer m_resource_tag;

public:
  cIpmiTextBuffer &ResourceTag() { return m_resource_tag; }

public:
  cIpmiResource( cIpmiMc *mc, unsigned int fru_id );
  virtual ~cIpmiResource();

public:
  // return hotswap sensor if there is one
  cIpmiSensorHotswap *GetHotswapSensor() { return m_hotswap_sensor; }

protected:
  unsigned int m_current_control_id;

public:
  // get a unique control num for this resource
  unsigned int GetControlNum()
  {
    return ++m_current_control_id;
  }

  // HPI resource id
  SaHpiResourceIdT m_resource_id;

  virtual bool Create( SaHpiRptEntryT &entry );
  virtual void Destroy();

  SaErrorT SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp,
                        unsigned int lun = 0, int retries = dIpmiDefaultRetries );

  SaErrorT SendCommandReadLock( cIpmiRdr *rdr, const cIpmiMsg &msg, cIpmiMsg &rsp,
                                unsigned int lun = 0, int retries = dIpmiDefaultRetries );
  SaErrorT SendCommandReadLock( const cIpmiMsg &msg, cIpmiMsg &rsp,
                                unsigned int lun = 0, int retries = dIpmiDefaultRetries );

  void Activate();
  void Deactivate();
  SaHpiHsStateT GetHpiState();

private:
  bool m_populate;

public:
  // create and populate hpi resource
  virtual bool Populate();
};


#endif
