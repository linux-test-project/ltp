/*
 * ipmi_discover.cpp
 *
 * discover MCs
 *
 * Copyright (c) 2004 by FORCE Computers.
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
 *
 * This file contains the discover thread which handles
 * the following things:
 *   1. periodic send an GetDeviceId to the IPMI hardware.
 *   2. periodic reading SEL and handling new entries.
 *
 * Async events are handled in this file too. They
 * came from the reader thread.
 */


#include "ipmi_domain.h"


class cIpmiDiscoverTask
{
public:
  cIpmiDiscoverTask *m_next;
  tIpmiDiscoverTask  m_task;
  timeval            m_timeout;
  void              *m_userdata;

  cIpmiDiscoverTask( tIpmiDiscoverTask task, const timeval &timeout, 
		     void *userdata )
    : m_next( 0 ), m_task( task ), 
      m_timeout( timeout ), m_userdata( userdata )
  {}

  // <  0 => m_timeout < timeout
  // >  0 => m_timeout > timeout
  // == 0 => m_timeout = timeout
  int Cmp( const timeval &timeout )
  {
    if ( m_timeout.tv_sec < timeout.tv_sec )
	 return -1;

    if ( m_timeout.tv_sec > timeout.tv_sec )
	 return 1;

    if ( m_timeout.tv_usec < timeout.tv_usec )
	 return -1;

    if ( m_timeout.tv_usec > timeout.tv_usec )
	 return 1;

    return 0;
  }
};


void
cIpmiDomain::AddDiscoverTask( tIpmiDiscoverTask task, timeval timeout, 
			      void *userdata )
{
  cIpmiDiscoverTask *dt = new cIpmiDiscoverTask( task, timeout, userdata );

  m_discover_tasks_lock.Lock();

  cIpmiDiscoverTask *prev = 0;
  cIpmiDiscoverTask *current = m_discover_tasks;

  // loop to right position
  while( current && current->Cmp( dt->m_timeout ) <= 0 )
     {
       prev = current;
       current = current->m_next;
     }

  if ( prev == 0 )
     {
       // insert dt at first position
       m_discover_tasks = dt;
       dt->m_next = current;
     }
  else
     {
       dt->m_next = prev->m_next;
       prev->m_next = dt;
     }

  m_discover_tasks_lock.Unlock();  
}


void
cIpmiDomain::AddDiscoverTask( tIpmiDiscoverTask task, unsigned int ms,
			      void *userdata )
{
  timeval timeout;
  gettimeofday( &timeout, 0 );

  timeout.tv_sec  += ms / 1000;
  timeout.tv_usec += (ms % 1000) * 1000;

  while( timeout.tv_usec > 10000000 )
     {
       timeout.tv_sec++;
       timeout.tv_usec -= 10000000;
     }

  AddDiscoverTask( task, timeout, userdata );
}


bool
cIpmiDomain::RemDicoveryTask( void *userdata )
{
  bool rv = false;

  m_discover_tasks_lock.Lock();

  cIpmiDiscoverTask *prev = 0;
  cIpmiDiscoverTask *current = m_discover_tasks;

  // loop to right position
  while( current && current->m_userdata != userdata )
     {
       prev = current;
       current = current->m_next;
     }

  if ( current && current->m_userdata )
     {
       if ( prev == 0 )
	    m_discover_tasks = current->m_next;
       else
	    prev->m_next = current->m_next;

       delete current;

       rv = true;
     }
  else
       assert( 0 );

  m_discover_tasks_lock.Unlock();

  return rv;
}


void
cIpmiDomain::ClearDiscoverTaskList()
{
  m_discover_tasks_lock.Lock();

  while( m_discover_tasks )
     {
       cIpmiDiscoverTask *dt = m_discover_tasks;
       m_discover_tasks = m_discover_tasks->m_next;
       delete dt;
     }

  m_discover_tasks_lock.Unlock();
}


void *
cIpmiDomain::Run()
{
  IpmiLog( "starting discover thread.\n" );

  m_initial_discover = true;

  Discover();

  m_initial_discover = false;

  ConnectionCheck( m_con );

  // periodic SEL read loop
  while( !m_exit )
     {
       usleep( 250000 );

       m_discover_tasks_lock.Lock();

       while( m_discover_tasks )
          {
	    timeval now;
	    gettimeofday( &now, 0 );

	    if ( m_discover_tasks->Cmp( now ) > 0 )
		 break;

	    // timeout
            cIpmiDiscoverTask *dt = m_discover_tasks;
            m_discover_tasks = m_discover_tasks->m_next;

            m_discover_tasks_lock.Unlock();

            (this->*dt->m_task)( dt->m_userdata );
            delete dt;

            m_discover_tasks_lock.Lock();
          }

       m_discover_tasks_lock.Unlock();  
     }

  IpmiLog( "stop discover thread.\n" );

  return 0;
}


void
cIpmiDomain::Discover()
{
}


void
cIpmiDomain::ConnectionCheck( void *userdata )
{
  assert( userdata == m_con );

  // check the connection if
  // there is no ipmi trafic
  if ( !m_con->CheckPending() )
     {
       IpmiLog( "check ipmi connection.\n" );

       cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdGetDeviceId );
       cIpmiMsg rsp;

       m_si_mc->SendCommand( msg, rsp );

       // do not check the return code.
       // connection check is to trigger
       // a reconnect in cIpmiCon it nessesary.
     }
  else
       IpmiLog( "check ipmi connection not nessesary.\n" );

  // add myself to task list
  AddDiscoverTask( &cIpmiDomain::ConnectionCheck, 1000, userdata );
}


void 
cIpmiDomain::ReadSel( void *userdata )
{
  cIpmiMc *mc = (cIpmiMc *)userdata;

  GList *new_events = mc->Sel()->GetEvents();

  if ( new_events )
       HandleEvents( new_events );

  // add myself to task list
  AddDiscoverTask( &cIpmiDomain::ReadSel, m_sel_rescan_interval, userdata );
}
