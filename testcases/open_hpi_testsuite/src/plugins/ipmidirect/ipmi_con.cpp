/*
 * ipmi_con.c
 *
 * Interface code for handling IPMI connections
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>


__BEGIN_DECLS
#include <openhpi.h>
__END_DECLS


#include "ipmi_con.h"
#include "ipmi_domain.h"


cIpmiCon::cIpmiCon( unsigned int ipmi_timeout, unsigned int atca_timeout,
                    unsigned int max_outstanding )
  : m_domain( 0 ), m_is_open( false ), m_fd( -1 ), m_slave_addr( 0x20 ),
    m_max_outstanding( max_outstanding ), m_queue( 0 ),
    m_num_outstanding( 0 ), m_current_seq( 0 ),
    m_exit( false ),
    m_default_ipmi_timeout( ipmi_timeout ),
    m_default_atca_timeout( atca_timeout ),
    m_timeout( ipmi_timeout )
{
  assert( m_max_outstanding > 0 );
  assert( m_max_outstanding <= dMaxSeq );

  for( int i = 0; i < dMaxSeq; i++ )
       m_outstanding[i] = 0;
}


cIpmiCon::~cIpmiCon()
{
  assert( !IsRunning() );
  RequeueOutstanding();

  while( m_queue )
     {
       cIpmiRequest *r = (cIpmiRequest *)m_queue->data;

       // there is no way to call others
       // than cIpmiCon::IfDestroyRequest !!!
       IfDestroyRequest( r );

       m_queue = g_list_remove( m_queue, r );
     }
}


void
cIpmiCon::RequeueOutstanding()
{
  for( int i = 0; i < dMaxSeq; i++ )
     {
       if ( m_outstanding[i] == 0 )
            continue;

       if ( m_outstanding[i]->m_retries_left == 0 )
            m_outstanding[i]->m_retries_left = 1;

       m_queue = g_list_append( m_queue, m_outstanding[i] );
       RemOutstanding( i );
     }
}


int
cIpmiCon::AddOutstanding( cIpmiRequest *r )
{
  assert( m_num_outstanding < m_max_outstanding );

  // find next free seq
  while( true )
     {
       if ( m_outstanding[m_current_seq] == 0 )
            break;

       m_current_seq++;
       m_current_seq %= m_max_outstanding;
     }

  r->m_seq = m_current_seq;
  m_outstanding[m_current_seq] = r;
  m_num_outstanding++;

  m_current_seq++;
  m_current_seq %= m_max_outstanding;

  return r->m_seq;
}


void
cIpmiCon::RemOutstanding( int seq )
{
  assert( seq >= 0 && seq < dMaxSeq );

  if ( m_outstanding[seq] == 0 )
     {
       assert( 0 );
       return;
     }

  m_outstanding[seq] = 0;
  m_num_outstanding--;

  assert( m_num_outstanding >= 0 );
}


void
cIpmiCon::HandleMsgError( cIpmiRequest *r, int err )
{
  // try again
  if ( r->m_retries_left > 0 )
     {
       m_queue = g_list_append( m_queue, r );
       return;
     }

  // error while sending command
  r->m_error = err;
  r->m_signal->Lock();
  r->m_signal->Signal();
  r->m_signal->Unlock();
}


int
cIpmiCon::SendCmd( cIpmiRequest *request )
{
  assert( m_num_outstanding < m_max_outstanding );

  request->m_retries_left--;
  assert( request->m_retries_left >= 0 );

  int seq = AddOutstanding( request );

  // message timeout
  gettimeofday( &request->m_timeout, 0 );

  // add timeout
  request->m_timeout.tv_sec  += m_timeout / 1000;
  request->m_timeout.tv_usec += (m_timeout % 1000) * 1000;

  while( request->m_timeout.tv_usec > 10000000 )
     {
       request->m_timeout.tv_sec++;
       request->m_timeout.tv_usec -= 10000000;
     }

/*
  IpmiLog( "SendMsg: cmd = %s, seq = %d, timeout %d %d\n",
           IpmiCmdToString( request->m_msg.m_netfn, request->m_msg.m_cmd ),
           seq,
           request->m_timeout.tv_sec, request->m_timeout.tv_usec );
*/

  // addr translation
  IfAddrToSendAddr( request->m_addr, request->m_send_addr );

  // send message
  int rv = IfSendCmd( request );

  if ( rv )
     {
       RemOutstanding( seq );
       return rv;
     }

  return 0;
}


void
cIpmiCon::SendCmds()
{
  while( m_queue && m_num_outstanding < m_max_outstanding )
     {
       cIpmiRequest *r = (cIpmiRequest *)m_queue->data;
       m_queue = g_list_remove( m_queue, r );

       int rv = SendCmd( r );

       if ( rv )
            HandleMsgError( r, rv );
     }
}


void *
cIpmiCon::Run()
{
  IpmiLog( "starting reader thread.\n" );

  // create pollfd
  struct pollfd pfd;

  pfd.events = POLLIN;

  // reader loop
  while( !m_exit )
     {
       assert( m_fd >= 0 );

       // do this before every poll(),
       // because m_fd can change
       pfd.fd = m_fd;

       int rv = poll( &pfd, 1, 100 );

       if ( rv == 1 )
            // read response
            IfReadResponse();
       else if ( rv != 0 )
          {
            if ( errno != EINTR )
               {
                 // error
                 fprintf( stderr, "poll returns %d, %d, %s !\n", rv, errno, strerror( errno ) );
                 assert( 0 );
                 abort();
               }
          }

       // check for expiered ipmi commands
       struct timeval t0;
       gettimeofday( &t0, 0 );

       m_queue_lock.Lock();

       for( int i = 0; i < m_max_outstanding; i++ )
          {
            if ( m_outstanding[i] == 0 )
                 continue;

            cIpmiRequest *r = m_outstanding[i];

            if ( timercmp( &t0, &r->m_timeout, < ) >= 0 )
                 continue;

            IpmiLog( "IPMI msg timeout: cmd = %s, seq = %d, timeout %d %d, now %d %d\n",
                     IpmiCmdToString( r->m_msg.m_netfn, r->m_msg.m_cmd ),
                     r->m_seq, r->m_timeout.tv_sec, r->m_timeout.tv_usec,
                     t0.tv_sec, t0.tv_usec );

            // timeout expired
            RemOutstanding( r->m_seq );

            HandleMsgError( r, ETIMEDOUT );
          }

       // send new comands
       SendCmds();

       m_queue_lock.Unlock();
     }

  IpmiLog( "stop reader thread.\n" );

  return 0;
}


void
cIpmiCon::IfClose()
{
}


cIpmiRequest *
cIpmiCon::IfAllocRequest( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  return new cIpmiRequest( addr, msg );
}


void 
cIpmiCon::IfDestroyRequest( cIpmiRequest *r )
{
  delete r;
}


void
cIpmiCon::IfAddrToSendAddr( const cIpmiAddr &addr, cIpmiAddr &send_addr )
{
  // address translation
  send_addr = addr;

  if (    addr.m_type == eIpmiAddrTypeIpmb
       || addr.m_type == eIpmiAddrTypeIpmbBroadcast )
     {
       if ( addr.m_slave_addr == m_slave_addr )
	  {
	    // Most systems don't handle sending to your own slave
	    // address, so we have to translate here.
            send_addr.Si();
            send_addr.m_lun = addr.m_lun;
	  }
     }
}


bool
cIpmiCon::Open()
{
  if ( IsOpen() )
       return true;

  m_fd = IfOpen();

  if ( m_fd == -1 )
       return false;

  m_exit = false;

  // start reader thread
  if ( !Start() )
       return false;

  m_is_open = true;

  return true;
}


void
cIpmiCon::Close()
{
  if ( !IsOpen() )
       return;

  assert( IsRunning() );

  // signal reader thread to terminate
  m_exit = true;

  // wait for reader thread
  void *rv;

  Wait( rv );

  IfClose();

  m_is_open = false;
}


// send an ipmi command and wait for response.
int
cIpmiCon::Cmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
               cIpmiAddr &rsp_addr, cIpmiMsg &rsp )
{
  int rv;

  assert( msg.m_data_len <= dIpmiMaxMsgLength );

  m_log_lock.Lock();

  IpmiLogData( "cmd " );
  IpmiLogDataMsg( addr, msg );
  IpmiLogData( "\n" );

  m_log_lock.Unlock();

  assert( IsRunning() );

  int idx = addr.m_slave_addr;

  // use 0 for system interface
  if ( addr.m_type == eIpmiAddrTypeSystemInterface )
       idx = 0;

  cThreadCond cond;

  // create request
  cIpmiRequest *r = IfAllocRequest( addr, msg );
  r->m_rsp_addr = &rsp_addr;
  r->m_rsp      = &rsp;
  r->m_signal   = &cond;
  r->m_error    = EINVAL;

  // lock queue
  cond.Lock();
  m_queue_lock.Lock();

  if ( m_num_outstanding < m_max_outstanding )
     {
       // send the command within this thread context.
       rv = SendCmd( r );

       if ( rv )
	  {
	    // error
	    IfDestroyRequest( r );

	    m_queue_lock.Unlock();
	    cond.Unlock();
	    return rv;
	  }
     }
  else
       m_queue = g_list_append( m_queue, r );       

  m_queue_lock.Unlock();

  // wait for response
  cond.Wait();
  cond.Unlock();

  rv = r->m_error;
  IfDestroyRequest( r );

  m_log_lock.Lock();

  if ( !rv )
     {
       assert( (tIpmiNetfn)(msg.m_netfn | 1) == rsp.m_netfn );
       assert( msg.m_cmd == rsp.m_cmd );  

       IpmiLogData( "rsp " );
       IpmiLogDataMsg( rsp_addr, rsp );
       IpmiLogData( "\n" );
     }
  else if ( rv == ETIMEDOUT )
       IpmiLogData( "tim\n" );
  else
       IpmiLogData( "err\n" );

  m_log_lock.Unlock();

  return rv;
}


int
cIpmiCon::ExecuteCmd( const cIpmiAddr &addr, const cIpmiMsg &msg, cIpmiMsg &rsp_msg )
{
  cIpmiAddr rsp_addr;

  return Cmd( addr, msg, rsp_addr, rsp_msg );
}


void
cIpmiCon::HandleResponse( int seq, const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  m_queue_lock.Lock();

  if ( m_outstanding[seq] == 0 )
     {
       m_log_lock.Lock();

       IpmiLog( "reading response without request:\n" );
       IpmiLogData( "# " );
       IpmiLogDataMsg( addr, msg );
       IpmiLogData( "\n" );

       m_log_lock.Unlock();

       m_queue_lock.Unlock();

       return;
     }

  cIpmiRequest *r = m_outstanding[seq];
  assert( r->m_seq == seq );
  RemOutstanding( seq );

  // addr translation
  *r->m_rsp_addr = addr;

  // convert braodcast to ipmb
  if ( r->m_rsp_addr->m_type == eIpmiAddrTypeIpmbBroadcast )
       r->m_rsp_addr->m_type = eIpmiAddrTypeIpmb;

  r->m_error     = 0;
  *r->m_rsp      = msg;

  r->m_signal->Lock();
  r->m_signal->Signal();
  r->m_signal->Unlock();

  m_queue_lock.Unlock();
}


void
cIpmiCon::HandleEvent( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  m_log_lock.Lock();

  IpmiLogData( "evt " );
  IpmiLogDataMsg( addr, msg );
  IpmiLogData( "\n" );

  m_log_lock.Unlock();

  assert( m_domain );
  m_domain->AddAsyncEvent( addr, msg );
}


void
IpmiLogDataMsg( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  char str[1024];
  char *s = str;

  // addr
  switch( addr.m_type )
     {
       case eIpmiAddrTypeIpmb:
            s += sprintf( s, "%02x %02x %02x",
                          eIpmiAddrTypeIpmb, addr.m_channel, addr.m_lun );
            break;

       case eIpmiAddrTypeSystemInterface:
            s += sprintf( s, "%02x %02x %02x %02x",
                          eIpmiAddrTypeSystemInterface, addr.m_channel, addr.m_lun, addr.m_slave_addr );
            break;

       case eIpmiAddrTypeIpmbBroadcast:
            s += sprintf( s, "%02x %02x %02x %02x",
                          eIpmiAddrTypeIpmbBroadcast, addr.m_channel, addr.m_lun, addr.m_slave_addr );
     }

  // msg
  s += sprintf( s, "  %02x %02x %02x ",
		msg.m_netfn, msg.m_cmd, msg.m_data_len );

  const unsigned char *p = msg.m_data;

  for( int i = 0; i < msg.m_data_len; i++ )
       s += sprintf( s, " %02x", *p++ );

  IpmiLogData( str );
}
