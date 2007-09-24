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

#ifndef dIpmiDomain_h
#define dIpmiDomain_h


#include <stdlib.h>
#include <string.h>


extern "C" {
#include "SaHpi.h"
}

#include <openhpi.h>
#include <oh_utils.h>

#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif

#ifndef dIpmiMc_h
#include "ipmi_mc.h"
#endif

#ifndef dIpmiResource_h
#include "ipmi_resource.h"
#endif

#ifndef dIpmiSensorHotswap_h
#include "ipmi_sensor_hotswap.h"
#endif

#ifndef dIpmiSensorThreshold_h
#include "ipmi_sensor_threshold.h"
#endif

#ifndef dIpmiControl_h
#include "ipmi_control.h"
#endif

#ifndef dIpmiWatchdog_h
#include "ipmi_watchdog.h"
#endif

#ifndef dIpmiDiscover_h
#include "ipmi_discover.h"
#endif

#ifndef dIpmiFruInfo_h
#include "ipmi_fru_info.h"
#endif


// property for site types
// found by get address info
class cIpmiAtcaSiteProperty
{
public:
  unsigned int m_property;
  int          m_max_side_id;
  unsigned int m_mc_type; // dIpmiMcTypeBitXXX ipmi_discover.cpp
};


// Re-query the SEL every 5 seconds by default.
#define dIpmiSelQueryInterval 5000

// Default poll interval for MCs
#define dIpmiMcPollInterval 1000


class cIpmiDomain : public cIpmiFruInfoContainer
{
public:
  unsigned int m_con_ipmi_timeout;
  unsigned int m_con_atca_timeout;
  bool m_enable_sel_on_all;

  unsigned int m_max_outstanding; // 0 => use default
  bool         m_atca_poll_alive_mcs;
protected:
  // ipmi connection
  cIpmiCon     *m_con;

  SaHpiDomainIdT m_did;
  cIpmiTextBuffer m_domain_tag;
  SaHpiTimeoutT   m_insert_timeout;
  SaHpiTimeoutT   m_extract_timeout;
  bool m_own_domain;
  int m_handler_id;

public:
  SaHpiDomainIdT DomainId() { return m_did; }
  SaHpiTimeoutT &InsertTimeout() { return m_insert_timeout; }
  SaHpiTimeoutT &ExtractTimeout() { return m_extract_timeout; }
  int HandlerId() { return m_handler_id; }

  bool ConLogLevel( int v )
  {
    return m_con->LogLevel( v );
  }

  // true => TCA
  bool           m_is_tca;

public:
  bool IsTca() { return m_is_tca; }

protected:
  // properties for site types
  // found by get address info
  cIpmiAtcaSiteProperty m_atca_site_property[256];

  void SetAtcaSiteProperty( tIpmiAtcaSiteType type,
                            unsigned int property, int max_id )
  {
    cIpmiAtcaSiteProperty *p = &m_atca_site_property[type];
    p->m_property    = property;
    p->m_max_side_id = max_id;
  }

  // The main set of SDRs on a BMC.
  cIpmiSdrs    *m_main_sdrs;

  // The sensors that came from the main SDR.
  GList *m_sensors_in_main_sdr;

  // Major and minor versions of the connection.
  unsigned int  m_major_version;
  unsigned int  m_minor_version;
  bool          m_sdr_repository_support;

  // A special MC used to represent the system interface.
  cIpmiMc      *m_si_mc;

public:
  cIpmiMc *SiMc()   { return m_si_mc; }

protected:
  // global lock for reading/writing:
  //   mcs, entities, sensors, frus, sels
  cThreadLockRw m_lock;

  cArray<cIpmiMc> m_mcs; // list of all MCs

public:
  void ReadLock()    { m_lock.ReadLock(); }
  void ReadUnlock()  { m_lock.ReadUnlock(); }
  void WriteLock()   { m_lock.WriteLock(); }
  void WriteUnlock() { m_lock.WriteUnlock(); }
  bool CheckLock()   { return m_lock.CheckLock(); }

  // lock m_initial_discover
  cThreadLock m_initial_discover_lock;

  // > 0 => initial discover in progress
  int m_initial_discover;

protected:
  // array of mc threads
  cIpmiMcThread *m_mc_thread[256];

public:
  int           m_num_mc_threads;
  cThreadLock   m_mc_thread_lock;

public:
  // time between mc poll in ms
  unsigned int m_mc_poll_interval;

  // time between sel rescan in ms
  unsigned int m_sel_rescan_interval;
  bool m_bmc_discovered;

  SaErrorT CheckTca();

public:
  void AddMc( cIpmiMc *mc );
  bool CleanupMc( cIpmiMc *mc );

public:
  cIpmiDomain();
  virtual ~cIpmiDomain();

  bool Init( cIpmiCon *c );
  void Cleanup();

  cIpmiMc *FindMcByAddr( const cIpmiAddr &addr );
  //cIpmiMc *FindOrCreateMcBySlaveAddr( unsigned int slave_addr );
  SaErrorT SendCommand( const cIpmiAddr &addr, const cIpmiMsg &msg, cIpmiMsg &rsp_msg,
                        int retries = dIpmiDefaultRetries );
  GList *GetSdrSensors( cIpmiMc *mc );
  void   SetSdrSensors( cIpmiMc *mc, GList *sensors );
  cIpmiMc *GetEventRcvr();

  // called from cIpmiCon to handle async events
  void HandleAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg );

  // called with a list of events to handle from cIpmiMcThread
  void HandleEvents( GList *list );

  // handle a single event
  void HandleEvent( cIpmiEvent *event );

  cIpmiResource  *VerifyResource( cIpmiResource *res );
  cIpmiMc        *VerifyMc( cIpmiMc *mc );
  cIpmiRdr       *VerifyRdr( cIpmiRdr *rdr );
  cIpmiSensor    *VerifySensor( cIpmiSensor *s );
  cIpmiControl   *VerifyControl( cIpmiControl *c );
  cIpmiWatchdog  *VerifyWatchdog( cIpmiWatchdog *c );
  cIpmiInventory *VerifyInventory( cIpmiInventory *i );

  virtual void AddHpiEvent( oh_event *event ) = 0;
  virtual oh_evt_queue *GetHpiEventList() = 0;

  virtual const cIpmiEntityPath &EntityRoot() = 0;
  virtual oh_handler_state *GetHandler() = 0;

  virtual SaHpiRptEntryT *FindResource( SaHpiResourceIdT id ) = 0;

  void Dump( cIpmiLog &dump ) const;
};


#endif
