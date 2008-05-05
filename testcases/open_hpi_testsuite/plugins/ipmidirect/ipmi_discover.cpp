/*
 * ipmi_discover.cpp
 *
 * discover MCs
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
 *
 * This file contains:
 * 1. discover of MCs
 * 2. periodic polling of MCs
 * 3. periodic reading SEL
 * 4. handling of events
 */


#include "ipmi_domain.h"
#include <assert.h>
#include <errno.h>


class cIpmiMcTask
{
public:
  cIpmiMcTask *m_next;
  tIpmiMcTask  m_task;
  cTime        m_timeout;
  void        *m_userdata;

  cIpmiMcTask( tIpmiMcTask task, const cTime &timeout, 
               void *userdata )
    : m_next( 0 ), m_task( task ),
      m_timeout( timeout ), m_userdata( userdata )
  {}
};


cIpmiMcThread::cIpmiMcThread( cIpmiDomain *domain,
                              unsigned char addr,
                              unsigned int properties )
  : m_domain( domain ), m_addr( addr ),
    m_mc( 0 ),
    m_properties( properties ),
    m_exit( false ), m_tasks( 0 ),
    m_sel( 0 ), m_events( 0 )
{
}


cIpmiMcThread::~cIpmiMcThread()
{
  ClearMcTaskList();
}


void
cIpmiMcThread::WriteLock()
{
  m_domain->WriteLock();
}


void
cIpmiMcThread::WriteUnlock()
{
  m_domain->WriteUnlock();
}


void
cIpmiMcThread::AddMcTask( tIpmiMcTask task, const cTime &timeout, 
                          void *userdata )
{
  cIpmiMcTask *dt = new cIpmiMcTask( task, timeout, userdata );

  cIpmiMcTask *prev = 0;
  cIpmiMcTask *current = m_tasks;

  // loop to right position
  while( current && current->m_timeout <= dt->m_timeout )
     {
       prev = current;
       current = current->m_next;
     }

  if ( prev == 0 )
     {
       // insert dt at first position
       m_tasks = dt;
       dt->m_next = current;
     }
  else
     {
       dt->m_next = current;
       prev->m_next = dt;
     }
}


void
cIpmiMcThread::AddMcTask( tIpmiMcTask task, unsigned int ms,
                          void *userdata )
{
  cTime timeout = cTime::Now();
  timeout += ms;

  AddMcTask( task, timeout, userdata );
}


bool
cIpmiMcThread::RemMcTask( void *userdata )
{
  bool rv = false;

  cIpmiMcTask *prev = 0;
  cIpmiMcTask *current = m_tasks;

  // loop to right position
  while( current && current->m_userdata != userdata )
     {
       prev = current;
       current = current->m_next;
     }

  if ( current && current->m_userdata )
     {
       if ( prev == 0 )
	    m_tasks = current->m_next;
       else
	    prev->m_next = current->m_next;

       delete current;

       rv = true;
     }
  else
       assert( 0 );

  return rv;
}


void
cIpmiMcThread::ClearMcTaskList()
{
  while( m_tasks )
     {
       cIpmiMcTask *dt = m_tasks;
       m_tasks = m_tasks->m_next;
       delete dt;
     }
}


void *
cIpmiMcThread::Run()
{
  stdlog << "starting MC thread " << m_addr << ".\n";

  m_domain->m_mc_thread_lock.Lock();
  m_domain->m_num_mc_threads++;
  m_domain->m_mc_thread_lock.Unlock();

  if ( m_properties & dIpmiMcThreadInitialDiscover )
     {
       if ( m_addr != dIpmiBmcSlaveAddr )
       {
           stdlog << "Waiting for BMC discovery (" << m_addr << ").\n";
           while ( m_domain->m_bmc_discovered == false )
           {
               usleep( 100000 );
           }
           stdlog << "BMC Discovery done, let's go (" << m_addr << ").\n";
       }
       else
       {
           stdlog << "BMC Discovery Start\n";
       }
                
       Discover();
       m_domain->m_initial_discover_lock.Lock();
       m_domain->m_initial_discover--;
       m_domain->m_initial_discover_lock.Unlock();

       // clear initial discover flag
       m_properties &= ~dIpmiMcThreadInitialDiscover;

       if ( m_addr == dIpmiBmcSlaveAddr )
       {
           stdlog << "BMC Discovery done\n";
           m_domain->m_bmc_discovered = true;
       }
     }

  if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
       || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
       PollAddr( m_mc );

  // this is a hack, because
  // of the usleep calls and polling
  // of event queue and task list.
  while( !m_exit )
     {
       // handling all events in the event 
       // in the event queue
       HandleEvents();
       usleep( 100000 );

       // check for tasks to do
       while( m_tasks )
          {
            cTime now = cTime::Now();

	    if ( now < m_tasks->m_timeout )
		 break;

	    // timeout
            cIpmiMcTask *dt = m_tasks;
            m_tasks = m_tasks->m_next;

            (this->*dt->m_task)( dt->m_userdata );
            delete dt;
          }
     }

  stdlog << "stop MC thread " << m_addr << ".\n";

  m_domain->m_mc_thread_lock.Lock();
  assert( m_domain->m_num_mc_threads > 0 );
  m_domain->m_num_mc_threads--;
  m_domain->m_mc_thread_lock.Unlock();  

  return 0;
}


void
cIpmiMcThread::Discover( cIpmiMsg *get_device_id_rsp )
{
  cIpmiAddr addr( eIpmiAddrTypeIpmb, 0, 0, m_addr );
  cIpmiMsg gdi_rsp;

  if ( !get_device_id_rsp )
     {
       // send a get device id
       cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdGetDeviceId );

       // try sending the command only one time
       SaErrorT rv = m_domain->SendCommand( addr, msg, gdi_rsp, 1 );

       if ( rv != SA_OK || gdi_rsp.m_data[0] != 0 )
            return;

       get_device_id_rsp = &gdi_rsp;
     }

  stdlog << "MC at " << m_addr << " found:\n";
  stdlog << "\tdevice id             : " <<  get_device_id_rsp->m_data[1] << "\n";
  stdlog << "\tdevice SDR            : " << ((get_device_id_rsp->m_data[2] & 0x80) ? "yes" : "no") << "\n";
  stdlog << "\tdevice revision       : " << (get_device_id_rsp->m_data[2] & 0x0f ) << "\n";
  stdlog << "\tdevice available      : " << ((get_device_id_rsp->m_data[3] & 0x80) ? "update" : "normal operation" ) << "\n";
  stdlog << "\tmajor FW revision     : " << (get_device_id_rsp->m_data[3] & 0x7f) << "\n";
  stdlog << "\tminor FW revision     : " << (int)((get_device_id_rsp->m_data[4] >>4) & 0xf)
         << (int)(get_device_id_rsp->m_data[4] & 0xf) << "\n";
  stdlog << "\tIPMI version          : " << (int)(get_device_id_rsp->m_data[5] & 0xf) << "."
         << ((get_device_id_rsp->m_data[5] >> 4) & 0xf) << "\n";
  stdlog << "\tchassis device        : " << ((get_device_id_rsp->m_data[6] & 0x80) ? "yes" : "no") << "\n";
  stdlog << "\tbridge                : " << ((get_device_id_rsp->m_data[6] & 0x40) ? "yes" : "no") << "\n";
  stdlog << "\tIPMB event generator  : " << ((get_device_id_rsp->m_data[6] & 0x20) ? "yes" : "no") << "\n";
  stdlog << "\tIPMB event receiver   : " << ((get_device_id_rsp->m_data[6] & 0x10) ? "yes" : "no") << "\n";
  stdlog << "\tFRU inventory data    : " << ((get_device_id_rsp->m_data[6] & 0x08) ? "yes" : "no") << "\n";
  stdlog << "\tSEL device            : " << ((get_device_id_rsp->m_data[6] & 0x04) ? "yes" : "no") << "\n";
  stdlog << "\tSDR repository device : " << ((get_device_id_rsp->m_data[6] & 0x02) ? "yes" : "no") << "\n";
  stdlog << "\tsensor device         : " << ((get_device_id_rsp->m_data[6] & 0x01) ? "yes" : "no") << "\n";

  unsigned int mid =    get_device_id_rsp->m_data[7]
                     | (get_device_id_rsp->m_data[8] << 8)
                     | (get_device_id_rsp->m_data[9] << 16);
  stdlog.Hex();
  stdlog << "\tmanufacturer id       : " << mid << "\n";

  unsigned int pid = IpmiGetUint16( get_device_id_rsp->m_data + 10 );
  stdlog << "\tproduct id            : " << pid << "\n";

  if ( m_mc )
     {
       // m_mc should be NULL here
       // Let's clean up this mess
       stdlog << "m_mc not NULL !\n";

       m_mc->Cleanup();
       delete m_mc;
       m_mc = 0;

       return;
     }

  m_mc = new cIpmiMc( m_domain, addr );

  int rrv = m_mc->GetDeviceIdDataFromRsp( *get_device_id_rsp );

  if ( rrv )
     {
       // If we couldn't handle the device data, just clean
       // it up
       stdlog << "couldn't handle the device data !\n";

       m_mc->Cleanup();
       delete m_mc;
       m_mc = 0;

       return;
     }

  m_mc->CheckTca();

  if ( m_domain->IsTca() )
  {
      // If board is not ATCA, just give up
      if (!m_mc->IsTcaMc())
      {
          m_mc->Cleanup();
          delete m_mc;
          m_mc = 0;

          return;
      }
  }

  if (( m_domain->m_enable_sel_on_all == false ) && ( addr.SlaveAddr() != dIpmiBmcSlaveAddr ))
  {
    stdlog << "Disabling SEL for MC " << addr.SlaveAddr() << "\n";
    m_mc->SetSel(false);
  }

  cIpmiMcVendor *mv = cIpmiMcVendorFactory::GetFactory()->Get( mid, pid );
  m_mc->SetVendor( mv );

  if ( mv->InitMc( m_mc, *get_device_id_rsp ) == false )
     {
       stdlog << "cannot initialize MC: " <<  (unsigned char)m_mc->GetAddress() << " !\n";

       m_mc->Cleanup();
       delete m_mc;
       m_mc = 0;

       return;
     }

  SaErrorT rv = m_mc->HandleNew();

  if ( rv != SA_OK )
     {
       stdlog << "ERROR while discover MC " << m_addr << ", giving up !\n";

       m_mc->Cleanup();
       delete m_mc;
       m_mc = 0;

       return;
     }

  WriteLock();
  m_domain->AddMc( m_mc );
  m_mc->Populate();
  WriteUnlock();

  if ( m_mc->SelDeviceSupport() )
     {
       GList *new_events = m_mc->Sel()->GetEvents();

       // We only care for events from the BMC SEL
       if (( m_addr == dIpmiBmcSlaveAddr )
           && ( new_events ))
           	    m_domain->HandleEvents( new_events );
     }

  if ( m_mc->SelDeviceSupport() )
     {
       assert( m_sel == 0 );

       stdlog << "addr " << m_addr << ": add read sel. cIpmiMcThread::Discover\n";

       m_sel = m_mc->Sel();

       AddMcTask( &cIpmiMcThread::ReadSel,
                  m_domain->m_sel_rescan_interval,
                  m_sel );
     }
}


void
cIpmiMcThread::HandleEvents()
{
  bool loop = true;

  while( loop )
     {
       cIpmiEvent *event = 0;

       m_events_lock.Lock();

       if ( m_events )
          {
            event = (cIpmiEvent *)m_events->data;
            m_events = g_list_remove( m_events, event );
          }

       loop = m_events ? true : false;

       m_events_lock.Unlock(); 

       if ( event )
          {
            HandleEvent( event );
            delete event;
          }
     }
}


void
cIpmiMcThread::HandleEvent( cIpmiEvent *event )
{
  // can handle only events for that mc thread
  assert( event->m_data[4] == m_addr );

  stdlog << "event: ";
  event->Dump( stdlog, "event" );

  if ( event->m_type != 0x02 )
     {
       stdlog << "remove event: unknown event type " << (unsigned char)event->m_type << " !\n";
       return;
     }

  if ( (event->m_data[4] & 0x01) != 0 )
     {
       stdlog << "remove event: system software event.\n";
       return;
     }

  // reveive an event from an unknown MC
  // => discover
  if ( !m_mc )
     {
       assert( m_sel == 0 );

       // remove old task
       if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
            || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
          {
            stdlog << "addr " << m_addr << ": rem poll. cIpmiMcThread::HandleEvent\n";
            RemMcTask( m_mc );
          }

       Discover();

       // add new poll task
       if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
            || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
          {
            stdlog << "addr " << m_addr << ": add poll. cIpmiMcThread::HandleEvent\n";
            AddMcTask( &cIpmiMcThread::PollAddr, m_domain->m_mc_poll_interval,
                       m_mc );
          }
     }

  if ( m_mc == 0 )
     {
       stdlog << "hotswap event without a MC !\n";
       return;
     }

  cIpmiSensor *sensor = m_mc->FindSensor( (event->m_data[5] & 0x3), event->m_data[8] );

  if ( sensor == 0 )
     {
       stdlog << "sensor of event not found !\n";
       return;
     }

  // hotswap event
  if ( event->m_data[7] == eIpmiSensorTypeAtcaHotSwap )
     {
       cIpmiSensorHotswap *hs = dynamic_cast<cIpmiSensorHotswap *>( sensor );

       if ( !hs )
          {
            stdlog << "Not a hotswap sensor !\n";
            return;
          }

       HandleHotswapEvent( hs, event );
       return;
     }

  sensor->HandleEvent( event );
}


void
cIpmiMcThread::HandleHotswapEvent( cIpmiSensorHotswap *sensor,
                                   cIpmiEvent *event )
{
  tIpmiFruState current_state = (tIpmiFruState)(event->m_data[10] & 0x0f);
  tIpmiFruState prev_state    = (tIpmiFruState)(event->m_data[11] & 0x0f);
  unsigned int fru_id         = event->m_data[12] & 0xff;

  stdlog << "hot swap event at MC " << m_addr << ", sensor " << sensor->Num() << ",FRU " << fru_id << ",M" << (int)prev_state << " -> M"
         << (int)current_state << ".\n";

  if (sensor->Resource()->GetHotswapSensor() != sensor)
  {
        stdlog << "WARNING: sensor NOT resource hot swap sensor, discard event\n";
        return;
  }

  if (sensor->Resource()->FruId() != fru_id)
  {
        stdlog << "WARNING: FRU id NOT resource FRU id, discard event\n";
        return;
  }

  // remove old task
  if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
       || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
     {
       stdlog << "addr " << m_addr << ": rem poll. cIpmiMcThread::HandleHotswapEvent\n";
       RemMcTask( m_mc );
     }

  sensor->Resource()->PicmgFruState() = current_state;
  sensor->HandleEvent( event );

  switch (current_state)
  {
  case eIpmiFruStateNotInstalled:
    // We care only if it's the MC itself
    if ( sensor->Resource()->FruId() == 0 )
    {
       // remove mc
       WriteLock();

       if ( m_mc )
            m_domain->CleanupMc( m_mc );

       WriteUnlock();

       m_mc = 0;
    }
    break;

  case eIpmiFruStateActivationRequest:
    if (sensor->Resource()->Domain()->InsertTimeout() == SAHPI_TIMEOUT_IMMEDIATE)
    {
        sensor->Resource()->Activate();
    }
    else
    {
        sensor->Resource()->PolicyCanceled() = false;
    }
    break;

  case eIpmiFruStateDeactivationRequest:
    if (sensor->Resource()->ExtractTimeout() == SAHPI_TIMEOUT_IMMEDIATE)
    {
        sensor->Resource()->Deactivate();
    }
    else
    {
        sensor->Resource()->PolicyCanceled() = false;
    }
    break;

  default:
    break;
  }

  if ( m_mc == 0 && m_sel )
     {
       RemMcTask( m_sel );
       m_sel = 0;
     }

  // add new poll task
  if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
       || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
     {
       stdlog << "addr " << m_addr << ": add poll. cIpmiMcThread::HandleHotswapEvent\n";
       AddMcTask( &cIpmiMcThread::PollAddr, m_domain->m_mc_poll_interval, m_mc );
     }
}


void
cIpmiMcThread::AddEvent( cIpmiEvent *event )
{
  m_events_lock.Lock();
  m_events = g_list_append( m_events, event );
  m_events_lock.Unlock();
}


void
cIpmiMcThread::PollAddr( void *userdata )
{
  cIpmiMc *mc = (cIpmiMc *)userdata;

  if ( m_domain->ConLogLevel( dIpmiConLogCmd ) )
       stdlog << "poll MC at " << m_addr << ".\n";

  // send a get device id
  cIpmiAddr addr( eIpmiAddrTypeIpmb, 0, 0, m_addr );
  cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdGetDeviceId );
  cIpmiMsg rsp;

  SaErrorT rv = m_domain->SendCommand( addr, msg, rsp, 3 );

  if ( rv != SA_OK )
     {
       if ( m_mc )
          {
            stdlog << "communication lost: " << m_addr << " !\n";

            if ( m_properties & dIpmiMcThreadCreateM0 )
               {
                 cIpmiSensorHotswap *hs = m_mc->FindHotswapSensor();

                 if ( hs )
                    {
                      // generate an event hotswap event M0
                      cIpmiEvent *event = new cIpmiEvent;
                      event->m_mc = m_mc;

                      event->m_data[0]  = 0; // timestamp
                      event->m_data[1]  = 0;
                      event->m_data[2]  = 0;
                      event->m_data[3]  = 0;
                      event->m_data[4]  = m_mc->GetAddress();
                      event->m_data[5]  = 0;
                      event->m_data[6]  = 0x04; // message format
                      event->m_data[7]  = hs->SensorType();
                      event->m_data[8]  = hs->Num();
                      event->m_data[9]  = 0; // assertion
                      event->m_data[10] = 0; // M0
                      event->m_data[11] = hs->Resource()->PicmgFruState() | (7 << 4); // communication lost
                      event->m_data[12] = 0;

                      // this is because HandleHotswapEvent first removes the PollAddr task
                      if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
                              || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
                         {
                           stdlog << "addr " << m_addr << ": add poll. cIpmiMcThread::PollAddr\n";
                           AddMcTask( &cIpmiMcThread::PollAddr, m_domain->m_mc_poll_interval, m_mc );
                         }

                      HandleHotswapEvent( hs, event );
                      delete event;
                      
                      return;
                    }
               }

            m_domain->CleanupMc( mc );
            m_mc = 0;
          }
     }
  else
     {
       if ( !mc )
            // MC found.
            Discover( &rsp );
       //       else if ( m_mc )
            // Periodically check the event receiver for existing MCs.
       //     m_mc->CheckEventRcvr();
     }

  if ( m_mc == 0 && m_sel )
     {
       RemMcTask( m_sel );
       m_sel = 0;
     }

  if (    ( m_mc  && (m_properties & dIpmiMcThreadPollAliveMc ) )
       || ( !m_mc && (m_properties & dIpmiMcThreadPollDeadMc ) ) )
     {
       if ( m_domain->ConLogLevel( dIpmiConLogCmd ) )
	    stdlog << "addr " << m_addr << ": add poll. cIpmiMcThread::PollAddr\n";

       AddMcTask( &cIpmiMcThread::PollAddr, m_domain->m_mc_poll_interval, m_mc );
     }
}


void 
cIpmiMcThread::ReadSel( void *userdata )
{
  cIpmiSel *sel = (cIpmiSel *)userdata;
  GList *new_events = sel->GetEvents();

  if ( m_domain->ConLogLevel( dIpmiConLogCmd ) )
       stdlog << "addr " << m_addr << ": add sel reading. cIpmiMcThread::ReadSel\n";

  // add myself to task list
  AddMcTask( &cIpmiMcThread::ReadSel, m_domain->m_sel_rescan_interval,
                   userdata );

  // We only care for events from the BMC SEL
  if (( m_addr == dIpmiBmcSlaveAddr )
      && ( new_events ))
        m_domain->HandleEvents( new_events );
}
