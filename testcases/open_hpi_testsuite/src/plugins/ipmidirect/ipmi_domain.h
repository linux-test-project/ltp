/*
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

#ifndef dIpmiDomain_h
#define dIpmiDomain_h


#include <stdlib.h>
#include <string.h>


__BEGIN_DECLS
#include "SaHpi.h"
#include <openhpi.h>
#include <epath_utils.h>
#include <uid_utils.h>
__END_DECLS


#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif

#ifndef dIpmiMc_h
#include "ipmi_mc.h"
#endif

#ifndef dIpmiEntity_h
#include "ipmi_entity.h"
#endif

#ifndef dIpmiSensor_h
#include "ipmi_sensor.h"
#endif


#define dMaxMcMissedResponses 1

// Re-query the SEL every 10 seconds by default.
#define dIpmiSelQueryInterval 10000

// Check connection every second by default.
#define dIpmiConnectionCheckInterval 1000


class cIpmiEventAsync
{
public:
  cIpmiEvent m_event;

  cIpmiEventAsync( const cIpmiEvent &event )
    : m_event( event )  {}
};


class cIpmiDiscoverTask;
class cIpmiDomain;
typedef void (cIpmiDomain::*tIpmiDiscoverTask)( void *userdata );


// cThread is the discover thread.
class cIpmiDomain : public cThread 
{
public:
  unsigned int m_con_timeout;
  unsigned int m_con_atca_timeout;

protected:
  // ipmi connection
  cIpmiCon     *m_con;

  // true => ATCA
  int           m_is_atca;

  // The main set of SDRs on a BMC.
  cIpmiSdrs    *m_main_sdrs;

  // The sensors that came from the main SDR.
  cIpmiSensor **m_sensors_in_main_sdr;
  unsigned int  m_sensors_in_main_sdr_count;

  // Major and minor versions of the connection.
  unsigned int  m_major_version;
  unsigned int  m_minor_version;
  bool          m_sdr_repository_support;

  // A special MC used to represent the system interface.
  cIpmiMc      *m_si_mc;

  // global lock for reading/writing:
  //   mcs, entities, sensors, frus, sels
  cThreadLockRw m_lock;
  GList        *m_mcs; // list of all msc

  // true => initial discover in progress
  // false => done
  bool m_initial_discover;

  void Discover();

  // the discover thread
  virtual void *Run();

  // signal to discover thread to exit
  bool m_exit;

  // list of tasks to be done in the discover thread
  cThreadLock        m_discover_tasks_lock;
  cIpmiDiscoverTask *m_discover_tasks;

public:
  // add a task to discover thread
  void AddDiscoverTask( tIpmiDiscoverTask task, timeval timeout,
			void *userdata );

  // add a task to discover thread
  void AddDiscoverTask( tIpmiDiscoverTask task, unsigned int diff_ms,
			void *userdata );

  // remove discover task from list
  bool RemDicoveryTask( void *userdata );

  // clear the discover task list
  void ClearDiscoverTaskList();

  // time between connection check in ms
  unsigned int m_connection_check_interval;

  void ConnectionCheck( void *userdata );
  void ReadSel( void *userdata );

  cIpmiMc *SiMc()   { return m_si_mc; }

protected:
  unsigned char m_mc_to_check[256]; // m_mc_to_check[addr] & (1 << channel) =>
                                   // Rescan will check for mc at channel,addr

public:
  void AddMcToScan( unsigned int channel, unsigned int addr )
  {
    assert( channel < 4 );
    assert( addr < 256 );

    m_mc_to_check[addr] |= 1 << channel;
  }

  // time between sel rescan in ms
  unsigned int m_sel_rescan_interval;

protected:
  cIpmiEntityInfo *m_entities;

  int CheckAtca();
  int GetChannels();

  enum tScanState
  {
    eScanOk,
    eScanStateRescan,
    eScanStateError
  };

  cIpmiMc *Scan( const cIpmiAddr &addr, int &missed_responses, tScanState &state );
  void SelTimerAddToList( cIpmiMc *mc );

  cIpmiMc *NewMc( const cIpmiAddr &addr );
  bool     CleanupMc( cIpmiMc *mc );

  int StartSystem( const cIpmiMsg &devid );

public:
  cIpmiDomain();
  virtual ~cIpmiDomain();

  bool Init( cIpmiCon *c );
  void Cleanup();

  cIpmiEntityInfo &Entities() { return *m_entities; }
  
  cIpmiMc *FindMcByAddr( const cIpmiAddr &addr );
  cIpmiMc *FindOrCreateMcBySlaveAddr( unsigned int slave_addr );
  cIpmiMc *ScanMc( unsigned int channel, unsigned int addr );
  cIpmiMc *ScanSi( unsigned int channel );
  int      SendCommand( const cIpmiAddr &addr, const cIpmiMsg &msg, cIpmiMsg &rsp_msg );
  cIpmiSensor **GetSdrSensors( cIpmiMc *mc, unsigned int &count );
  void          SetSdrSensors( cIpmiMc *mc, cIpmiSensor **sensors,
                               unsigned int count );
  int GetEventRcvr();
  int PowerFru( cIpmiMc *mc );

  void HandleHotswapEvent( cIpmiMc *mc, cIpmiSensor *sensor, cIpmiEvent *event );
  void HandleEvent( cIpmiMc *mc, cIpmiSensor *sensor, cIpmiEvent *event );

  void SystemHandleEvent( cIpmiEvent *event );

protected:
  cThreadLock m_async_event_list_lock;
  GList      *m_async_event_list;

public:
  void AddAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg );
  void HandleAsyncEvents();

  // called with a list of events to handle
  void HandleEvents( GList *list );

  cIpmiEntity *VerifyEntity( cIpmiEntity *ent );
  cIpmiMc     *VerifyMc( cIpmiMc *mc );
  cIpmiSensor *VerifySensor( cIpmiSensor *s );
  cIpmiFru    *VerifyFru( cIpmiFru *f );

  void McRescan();

  virtual void AddHpiEvent( oh_event *event ) = 0;
  virtual GSList *GetHpiEventList() = 0;

  // called from IPMI framework to create HPI hotswap events
  virtual void IfHotswapEvent( cIpmiMc *mc, cIpmiSensor *sensor,
                               cIpmiEvent *event ) = 0;

  // called from IPMI framework to create/destroy HPI resources
  virtual void IfEntityAdd( cIpmiEntity *ent ) = 0;
  virtual void IfEntityRem( cIpmiEntity *ent ) = 0;

  // called from IPMI framework to create/destroy HPI rdr
  virtual void IfSensorAdd( cIpmiEntity *ent, cIpmiSensor *sensor ) = 0;
  virtual void IfSensorRem( cIpmiEntity *ent, cIpmiSensor *sensor ) = 0;
  virtual void IfSensorDiscreteEvent( cIpmiSensor  *sensor,
                                      tIpmiEventDir dir,
                                      int           offset,
                                      int           severity,
                                      int           prev_severity,
                                      cIpmiEvent   *event ) = 0;
  virtual void IfSensorThresholdEvent( cIpmiSensor        *sensor,
                                       tIpmiEventDir      dir,
                                       tIpmiThresh        threshold,
                                       tIpmiEventValueDir high_low,
                                       cIpmiEvent        *event ) = 0;

  // called from IPMI framework to create FRU inventory
  virtual void IfFruAdd( cIpmiEntity *ent, cIpmiFru *fru ) = 0;

  // called from IPMI framwork to create SEL
  virtual void IfSelAdd( cIpmiEntity *ent, cIpmiSel *sel ) = 0;

  virtual const char *EntityRoot() = 0;
  virtual oh_handler_state *GetHandler() = 0;

  virtual SaHpiRptEntryT *FindResource( SaHpiResourceIdT id ) = 0;
};


// helper functions found in sensor.cpp for decoding IPMI events
void SetThresholedSensorEventState( tIpmiThresh threshold, tIpmiEventDir dir,
                                    tIpmiEventValueDir high_low,
                                    SaHpiSensorEventT *event,
                                    SaHpiSeverityT *severity );
void SetThresholdsSensorMiscEvent( cIpmiSensor *sensor,
                                   cIpmiEvent *event,
                                   SaHpiSensorEventT *e );
void SetDiscreteSensorMiscEvent( cIpmiEvent *event,
                                 SaHpiSensorEventT *e );
void SetDiscreteSensorEventState( cIpmiEvent *event,
                                  SaHpiEventStateT *state );


#endif
