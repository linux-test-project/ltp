/*
 *
 * Copyright (c) 2003,3004 by FORCE Computers
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

#include <errno.h>
#include <assert.h>

#include "ipmi.h"
#include "ipmi_utils.h"
#include "force.h"


cIpmiDomain::cIpmiDomain()
  : m_con( 0 ), m_is_atca( false ), m_main_sdrs( 0 ),
    m_sensors_in_main_sdr( 0 ), m_sensors_in_main_sdr_count( 0 ), 
    m_major_version( 0 ), m_minor_version( 0 ), m_sdr_repository_support( false ),
    m_si_mc( 0 ), m_mcs( 0 ),
    m_initial_discover( true ), m_exit( false ), m_discover_tasks( 0 ),
    m_connection_check_interval( dIpmiConnectionCheckInterval ),
    m_sel_rescan_interval( dIpmiSelQueryInterval ), m_entities( 0 ),
    m_async_event_list( 0 )
{
  for( int i = 0; i < 256; i++ )
       m_mc_to_check[i] = 0;

  // scan at least at 0x20
  AddMcToScan( 0, 0x20 );
}


cIpmiDomain::~cIpmiDomain()
{
  while( m_async_event_list )
     {
       cIpmiEventAsync *ea = (cIpmiEventAsync *)m_async_event_list->data;
       m_async_event_list = g_list_remove( m_async_event_list, ea );
       delete ea;
     }
}


bool
cIpmiDomain::Init( cIpmiCon *con )
{
  assert( m_con == 0 );
  m_con = con;
  con->SetDomain( this ); // this is for async events

  m_entities = new cIpmiEntityInfo( this );

  if ( m_entities == 0 )
     {
       IpmiLog( "cannot create entity into !\n" );
       return false;
     }

  cIpmiAddr si( eIpmiAddrTypeSystemInterface);

  m_si_mc = new cIpmiMc( this, si );

  if ( m_si_mc == 0 )
     {
       IpmiLog( "cannot create system interface !\n" );
       return false;
     }

  m_main_sdrs = new cIpmiSdrs( m_si_mc, 0, 0 );
  assert( m_main_sdrs );

  cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdGetDeviceId );
  cIpmiMsg rsp;

  // send get device id
  int rv = m_si_mc->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send IPMI get device id %d, %s !\n", rv, strerror( rv ) );
       return false;
     }

  if (    rsp.m_data[0] != 0
       || rsp.m_data_len < 12 )
     {
       // At least the get device id has to work.
       IpmiLog( "get device id fails 0x%02x !\n", rsp.m_data[0] );

       return false;
     }

  m_major_version          = rsp.m_data[5] & 0xf;
  m_minor_version          = (rsp.m_data[5] >> 4) & 0xf;
  m_sdr_repository_support = (rsp.m_data[6] & 0x02) == 0x02;

  m_si_mc->SdrRepositorySupport() = m_sdr_repository_support;

  if ( m_major_version < 1 )
     {
       // We only support 1.0 and greater.
       IpmiLog( "ipmi version %d.%d not supported !\n",
                m_major_version, m_minor_version );

       return false;
     }

  // do vendor specific things
  if ( StartSystem( rsp ) )
       return false;

  CheckAtca();

  if ( m_sdr_repository_support )
     {
       IpmiLog( "reading repository SDR.\n" );

       rv = m_main_sdrs->Fetch();

       if ( rv )
            // Just report an error, it shouldn't be a big deal if this
            // fails.
            IpmiLog( "Could not get main SDRs, error 0x%02x.\n", rv );
     }

  rv = GetChannels();

  if ( rv )
       return false;

  m_initial_discover = true;

  // start discover thread
  if ( !Start() )
       return false;

  // scan mc
  McRescan();

  return true;
}


void
cIpmiDomain::Cleanup()
{
  unsigned int i;

  // stop discover thread
  if ( IsRunning() )
     {
       m_exit = true;
       void *rv;

       Wait( rv );
     }

  // stop reader thread
  if ( m_con && m_con->IsOpen() )
       m_con->Close();

  // Delete the sensors from the main SDR repository.
  if ( m_sensors_in_main_sdr )
     {
       for( i = 0; i < m_sensors_in_main_sdr_count; i++ )
	    if ( m_sensors_in_main_sdr[i] )
                 m_sensors_in_main_sdr[i]->Destroy();

       delete [] m_sensors_in_main_sdr;
    }

  // cleanup all MCs
  GList *l = g_list_first( m_mcs );

  while( l )
     {
       GList *n = g_list_next( l );

       cIpmiMc *mc = (cIpmiMc *)l->data;
       CleanupMc( mc );

       l = n;
     }

  // now all mc's are ready to detroy
  l = g_list_first( m_mcs );

  while( l )
     {
       GList *n = g_list_next( l );

       cIpmiMc *mc = (cIpmiMc *)l->data;
       if ( CleanupMc( mc ) == false )
            assert( 0 );

       l = n;
     }

  // clear the discover task list
  ClearDiscoverTaskList();

  // destroy si
  if ( m_si_mc )
     {
       bool rr = m_si_mc->Cleanup();
       assert( rr );
       delete m_si_mc;
       m_si_mc = 0;
     }

  /* Destroy the main SDR repository, if it exists. */
  if ( m_main_sdrs )
     {
       delete m_main_sdrs;
       m_main_sdrs = 0;
     }
  
  if ( m_entities )
     {
       delete m_entities;
       m_entities = 0;
     }
}


cIpmiMc *
cIpmiDomain::NewMc( const cIpmiAddr &addr )
{
  cIpmiMc *mc = new cIpmiMc( this, addr );
  m_mcs = g_list_append( m_mcs, mc );

  return mc;
}


bool
cIpmiDomain::CleanupMc( cIpmiMc *mc )
{
  if ( !mc->Cleanup() )
       return false;

  m_mcs = g_list_remove( m_mcs, mc );
  delete mc;

  return true;
}


int 
cIpmiDomain::GetChannels()
{
  int rv = 0;

  return rv;
}


int
cIpmiDomain::CheckAtca()
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPicMgProperties );
  cIpmiMsg rsp;
  int      rv;

  m_is_atca = false;

  assert( m_si_mc );

  msg.m_data_len = 1;
  msg.m_data[0]  = dIpmiPigMgId;

  IpmiLog( "checking for ATCA system.\n" );

  rv = m_si_mc->SendCommand( msg, rsp );

  if ( rv || rsp.m_data[0] || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "not an ATCA system.\n" );

       return rv;
     }

  unsigned char minor = (rsp.m_data[2] >> 4) & 0x0f;
  unsigned char major = rsp.m_data[2] & 0x0f;

  IpmiLog( "found a PigMg system version %d.%d.\n",
           major, minor );

  if ( major == 2 && minor == 0 )
     {
       IpmiLog( "found an ATCA system.\n" );

       // use atca timeout
       m_con->m_timeout = m_con->m_default_atca_timeout;

       m_is_atca = true;
     }

  return 0;
}


cIpmiMc *
cIpmiDomain::FindMcByAddr( const cIpmiAddr &addr )
{
  GList *l;

  if (    ( addr.m_type == eIpmiAddrTypeSystemInterface )
       && ( addr.m_channel == dIpmiBmcChannel ) )
       return m_si_mc;

  l = 0;

  for( l = g_list_first( m_mcs ); l; l = g_list_next( l ) )
     {
       cIpmiMc *mc = (cIpmiMc *)l->data;

       if ( addr == mc->Addr() )
            return mc;
     }

  return 0;
}


cIpmiMc *
cIpmiDomain::Scan( const cIpmiAddr &addr,
                   int &missed_responses, tScanState &state )
{
  cIpmiMc     *mc;
  cIpmiMsg     msg( eIpmiNetfnApp, eIpmiCmdGetDeviceId );
  cIpmiMsg     rsp_msg;
  cIpmiAddr    rsp_addr;
  int rv;

  IpmiLog( "Scan for MC at " );
  addr.Log();
  IpmiLog( "\n" );

  rv = m_con->Cmd( addr, msg, rsp_addr, rsp_msg );

  if ( rv )
     {
       if ( rv == ETIMEDOUT )
	  {
            if (    addr.m_type == eIpmiAddrTypeSystemInterface
                 || ++missed_responses >= dMaxMcMissedResponses )
               {
                 IpmiLog( "timeout, give it up.\n" );

                 state = eScanStateError;
                 return 0;
               }

            IpmiLog( "timeout %d.\n", missed_responses );

            state = eScanStateRescan;
	    return 0;
	  }

       IpmiLog( "error %d, %s !\n", rv, strerror( rv ) );

       state = eScanStateError;
       return 0;
     }

  assert( rsp_addr.m_type == eIpmiAddrTypeIpmb );

  mc = FindMcByAddr( rsp_addr );

  if ( rsp_msg.m_data[0] == 0 )
     {
       if (    mc && mc->IsActive()
            && !mc->DeviceDataCompares( rsp_msg ) )
          {
            // The MC was replaced with a new one, so clear the old
            // one and add a new one.
            CleanupMc( mc );

            mc = 0;
          }

       if ( !mc || !mc->IsActive() )
          {
            // It doesn't already exist, or it's inactive, so add
            // it.
            if ( !mc )
               {
                 // If it's not there, then add it.  If it's just not
                 // active, reuse the same data.
                 mc = NewMc( rsp_addr );
               }

            rv = mc->GetDeviceIdDataFromRsp( rsp_msg );

            if ( rv )
               {
                 // If we couldn't handle the device data, just clean
                 // it up
                 IpmiLog( "couldn't handle the device data !\n" );

                 CleanupMc( mc );

                 state = eScanStateError;

                 return 0;
               }

            mc->HandleNew();
          }
       else
          {
            // Periodically check the event receiver for existing MCs.
            mc->CheckEventRcvr();
          }
     }
  else if ( mc && mc->IsActive() )
     {
       if (    addr.m_type == eIpmiAddrTypeSystemInterface
            || ++missed_responses >= dMaxMcMissedResponses )
          {
            IpmiLog( "error give it up !\n" );

            CleanupMc( mc );

            state = eScanStateError;

            return 0;
          }

       IpmiLog( "error try again.\n" );

       // rerun cmd
       state = eScanStateRescan;

       return 0;
     }

  IpmiLog( "found.\n" );

  state = eScanOk;

  return mc;
}


cIpmiMc *
cIpmiDomain::ScanSi( unsigned int channel )
{
  int                      missed_responses = 0;
  tScanState               state;
  cIpmiAddr si( eIpmiAddrTypeSystemInterface );
  si.m_channel = channel;

  cIpmiMc *mc = Scan( si, missed_responses, state );

  return mc;
}


cIpmiMc *
cIpmiDomain::ScanMc( unsigned int channel, unsigned int addr )
{
  cIpmiAddr ipmb( eIpmiAddrTypeIpmbBroadcast, channel, 0, addr );
  int           missed_responses = 0;
  cIpmiMc      *mc;
  tScanState    state;

  do
     {
       mc = Scan( ipmb, missed_responses, state );
     }
  while( state == eScanStateRescan );

  return mc;
}


int
cIpmiDomain::SendCommand( const cIpmiAddr &addr, const cIpmiMsg &msg,
                          cIpmiMsg &rsp_msg )
{
  if ( m_con == 0 )
       return ENOSYS;

  return m_con->ExecuteCmd( addr, msg, rsp_msg );
}


cIpmiMc *
cIpmiDomain::FindOrCreateMcBySlaveAddr( unsigned int slave_addr )
{
  cIpmiMc   *mc;
  cIpmiAddr addr( eIpmiAddrTypeIpmb, 0, 0, slave_addr );

  mc = FindMcByAddr( addr );

  if ( mc )
       return mc;

  mc = NewMc( addr );
  mc->SetActive( false );

  return mc;
}


cIpmiSensor **
cIpmiDomain::GetSdrSensors( cIpmiMc  *mc,
                            unsigned int &count )
{
  if ( mc )
       return mc->GetSdrSensors( count );

  count = m_sensors_in_main_sdr_count;
  return m_sensors_in_main_sdr;
}


void
cIpmiDomain::SetSdrSensors( cIpmiMc      *mc,
                            cIpmiSensor **sensors,
                            unsigned int  count )
{
  if ( mc )
       mc->SetSdrSensors( sensors, count );
  else
     {
       m_sensors_in_main_sdr       = sensors;
       m_sensors_in_main_sdr_count = count;
     }
}


int
cIpmiDomain::GetEventRcvr()
{
  GList *l = m_mcs;

  while( l )
     {
       cIpmiMc *mc = (cIpmiMc *)l->data;

       if ( mc->SelDeviceSupport() )
            return mc->GetAddress();

       l = g_list_next( l );
     }

  return 0;
}


void
cIpmiDomain::AddAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  cIpmiEvent devent;

  // It came from an MC, so find the MC.
  cIpmiMc *mc = FindMcByAddr( addr );

  if ( !mc )
     {
       IpmiLog( "cannot find mc for event !\n" );
       return;
     }

  devent.m_mc        = mc;
  devent.m_record_id = IpmiGetUint16( msg.m_data );
  devent.m_type      = msg.m_data[2];
  memcpy( devent.m_data, msg.m_data + 3, dIpmiMaxSelData );

  // Add it to the mc's event log.
  mc->Sel()->AddAsyncEvent( &devent );

  cIpmiEventAsync *ea = new cIpmiEventAsync( devent );

  m_async_event_list_lock.Lock();
  m_async_event_list = g_list_append( m_async_event_list, ea );
  m_async_event_list_lock.Unlock();
}


void
cIpmiDomain::HandleAsyncEvents()
{
  while( true )
     {
       m_async_event_list_lock.Lock();

       if ( m_async_event_list == 0 )
          {
            m_async_event_list_lock.Unlock();
            break;
          }

       cIpmiEventAsync *ea = (cIpmiEventAsync *)m_async_event_list->data;
       m_async_event_list = g_list_remove( m_async_event_list, ea );
       m_async_event_list_lock.Unlock();

       cIpmiEvent event = ea->m_event;

       // check presence of mc
       cIpmiMc *mc = VerifyMc( ea->m_event.m_mc );

       delete ea;

       if ( !mc )
            continue;

       // Call the handler on it.
       SystemHandleEvent( &event );
     }
}


void
cIpmiDomain::SystemHandleEvent( cIpmiEvent *event )
{
  IpmiLog( "domain event: " );
  event->Log();

  if ( event->m_type != 0x02 )
     {
       IpmiLog( "remove event: unknown event type %02x !\n", event->m_type );
       return;
     }

  if ( (event->m_data[4] & 0x01) != 0 )
     {
       IpmiLog( "remove event: system software event.\n" );
       return;
     }

  cIpmiMc     *mc = 0;
  cIpmiSensor *sensor = 0;
  cIpmiAddr    addr;

  addr.m_type = eIpmiAddrTypeIpmb;
  
  if ( event->m_data[6] == 0x03 )
       addr.m_channel = 0;
  else
       addr.m_channel = event->m_data[5] >> 4;

  addr.m_slave_addr = event->m_data[4];
  addr.m_lun = 0;

  mc = FindMcByAddr( addr );

  if ( mc )
       sensor = mc->FindSensor( (event->m_data[5] & 0x3), event->m_data[8] );

  HandleEvent( mc, sensor, event );
}


void
cIpmiDomain::HandleEvents( GList *list )
{
  while( list )
     {
       cIpmiEvent *event = (cIpmiEvent *)list->data;

       SystemHandleEvent( event );

       delete event;
       list = g_list_remove( list, event );
     }
}


void
cIpmiDomain::HandleHotswapEvent( cIpmiMc *mc,
                                 cIpmiSensor *sensor,
                                 cIpmiEvent *event )
{
  tIpmiFruState current_state = (tIpmiFruState)(event->m_data[10] & 0x0f);
  tIpmiFruState prev_state    = (tIpmiFruState)(event->m_data[11] & 0x0f);

  IpmiLog( "hot swap event M%d -> M%d.\n", prev_state, current_state );

  // "not installed" -> "inactive" => scan for MC
  if (    current_state == eIpmiFruStateInactive 
       && prev_state    == eIpmiFruStateNotInstalled )
     {
       IpmiLog( "scan for MC 0x%02x 0x%02x.\n",
                event->m_data[5] >> 4, event->m_data[4] );

       if (    mc && mc->IsActive()
            && mc->GetChannel() == (unsigned int)(event->m_data[5] >> 4)
            && mc->GetAddress() == (unsigned int)event->m_data[4] )
            IpmiLog( "MC exists and is active !\n" );
       else
          {
            mc = ScanMc( event->m_data[5] >> 4, event->m_data[4] );

            // find sensor
            if ( mc )
                 sensor = mc->FindSensor( (event->m_data[5] & 0x3), event->m_data[8] );
          }
     }
  else if ( current_state == eIpmiFruStateActivationInProgress )
     {
       assert( mc );

       int rv = PowerFru( mc );

       if ( rv )
          { 
            // deactivate fru, because M3 has no representation in 
            // HPI we try to restart the FRU
            cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruActivation );
            
            msg.m_data[0] = dIpmiPigMgId;
            msg.m_data[1] = 0; // FRU id
            msg.m_data[2] = 0; // deactivate fru
            msg.m_data_len = 3;

            cIpmiMsg rsp;

            rv = mc->SendCommand( msg, rsp );

            if ( rv )
                 IpmiLog( "cannot send set fru activation: %d\n", rv );
            else if (    rsp.m_data_len != 2
                      || rsp.m_data[0] != eIpmiCcOk 
                      || rsp.m_data[1] != dIpmiPigMgId )
                 IpmiLog( "cannot set fru activation: 0x%02x !\n", rsp.m_data[0] );
          }
       else
            IpmiLog( "power fru.\n" );
     }
  
  IfHotswapEvent( mc, sensor, event );

  if ( mc )
       mc->FruState() = current_state;

  if ( current_state == eIpmiFruStateNotInstalled )
       // remove mc
       mc->Cleanup();
}


int
cIpmiDomain::PowerFru( cIpmiMc *mc )
{
  // get power level
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPowerLevel );
  cIpmiMsg rsp;

  msg.m_data[0] = dIpmiPigMgId;
  msg.m_data[1] = 0; // FRU id
  msg.m_data[2] = 0x01; // desired steady power
  msg.m_data_len = 3;

  int rv = mc->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send get power level: %d\n", rv );
       return EINVAL;
     }
  
  if (    rsp.m_data_len < 3
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "cannot get power level: 0x%02x !\n", rsp.m_data[0] );
       return EINVAL;
     }
  
  unsigned char power_level = rsp.m_data[2] & 0x1f;
  
  // set power level
  msg.m_netfn = eIpmiNetfnPicmg;
  msg.m_cmd   = eIpmiCmdSetPowerLevel;
  msg.m_data[0] = dIpmiPigMgId;
  msg.m_data[1] = 0; // FRU id
  msg.m_data[2] = power_level;
  msg.m_data[3] = 0x01; // copy desierd level to present level
  msg.m_data_len = 4;

  rv = mc->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send set power level: %d\n", rv );
       return EINVAL;
     }

  if (    rsp.m_data_len != 2
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[1] != dIpmiPigMgId )
       IpmiLog( "cannot set power level: 0x%02x !\n", rsp.m_data[0] );

  return 0;
}


void
cIpmiDomain::HandleEvent( cIpmiMc *mc,
                          cIpmiSensor *sensor, cIpmiEvent *event )
{
  if ( event->m_data[7] == eIpmiSensorTypeAtcaHotSwap )
     {
       HandleHotswapEvent( mc, sensor, event );
       return;
     }

  if ( sensor )
     {
       sensor->Event( event );
       return;
     }

  IpmiLog( "unknown event.\n" );
}


cIpmiEntity *
cIpmiDomain::VerifyEntity( cIpmiEntity *ent )
{
  return m_entities->VerifyEntify( ent );
}


cIpmiMc *
cIpmiDomain::VerifyMc( cIpmiMc *mc )
{
  if ( m_si_mc == mc )
       return mc;
  
  if ( g_list_find( m_mcs, mc ) )
       return mc;

  return 0;
}


cIpmiSensor *
cIpmiDomain::VerifySensor( cIpmiSensor *s )
{
  return m_entities->VerifySensor( s );
}


cIpmiFru *
cIpmiDomain::VerifyFru( cIpmiFru *f )
{
  return m_entities->VerifyFru( f );
}


// rescan is done in 2 steps:
// 1. build a list of all mcs to scan
//    a. list of predefined mcs to scan (m_mc_to_check)
//    b. mcs found in the main repository sdr
//    c. mcs in m_msc
// 2. scan for mcs in list
//    a. add new mcs
//    b. remove old mcs

void
cIpmiDomain::McRescan()
{
  // list of mcs to check:
  // list[0x82] = 0x5 => scan for mc at (channel,addr)
  // (0,0x82 and (2,0x82)
  unsigned char list[256];

  memcpy( list, m_mc_to_check, 256 );

  // add mcs found in main repository sdr to list
  for( unsigned int i = 0; i < m_main_sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = m_main_sdrs->Sdr( i );

       if ( sdr->m_type != eSdrTypeMcDeviceLocatorRecord )
            continue;

       // scan for mc at sdr->m_data[5] channel sdr->m_data[6]
       list[sdr->m_data[5]] |= 1 << sdr->m_data[6];
     }

  // add mcs already found to list
  cIpmiMc *mc;
  GList *item = g_list_first( m_mcs );

  while( item )
     {
       mc = (cIpmiMc *)item->data;

       list[mc->GetAddress()] |= 1 << mc->GetChannel();
       item = g_list_next( item );
     }

  // scan for mcs in list
  for( unsigned int addr = 0; addr < 256; addr++ )
     {
       if ( list[addr] == 0 )
            continue;

       for( unsigned int channel = 0; channel < dIpmiMaxChannel; channel++ )
          {
            if ( (list[addr] & (1 << channel)) == 0 )
                 continue;

            mc = ScanMc( channel, addr );

            if ( mc )
                 continue;

            // is mc in m_mcs ?
           item = g_list_first( m_mcs );

            while( item )
               {
                 mc = (cIpmiMc *)item->data;

                 if (    mc->GetChannel() == channel
                      && mc->GetAddress() == addr )
                    {
                      // kill mc
                      mc->Cleanup();
                      break;
                    }

                 item = g_list_next( item );
               }
          }
     }
}


int
cIpmiDomain::StartSystem( const cIpmiMsg &devid )
{
  // manufacturer id
  unsigned int mid =    devid.m_data[7]
                     | (devid.m_data[8] << 8)
                     | (devid.m_data[9] << 16);

  // product id
  unsigned int pid = IpmiGetUint16( devid.m_data + 10 );

  IpmiLog( "Manufacturer: %06x, Product Id: %04x.\n", 
           mid, pid );

  switch( mid )
     {
       case 0x000e48: // FORCE Computers
            if ( pid == 0x1011 )
                 return ForceShMcSetup( this, devid, mid, pid );

            break;
     }

  return 0;
}
