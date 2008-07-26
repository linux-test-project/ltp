/*
 * ipmi_con.c
 *
 * Interface code for handling IPMI connections
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
 * Copyright (c) 2005-2007 by ESO Technologies.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include "ipmi_con.h"


cIpmiCon::cIpmiCon( unsigned int timeout, int log_level )
  : m_is_open( false ), m_fd( -1 ), m_slave_addr( dIpmiBmcSlaveAddr ),
    m_max_outstanding( 1 ), m_queue( 0 ),
    m_num_outstanding( 0 ), m_current_seq( 0 ),
    m_exit( false ), m_log_level( log_level ),
    m_timeout( timeout ), m_check_connection( false )
{
  // m_log_level = dIpmiConLogEvent;

  for( int i = 0; i < dMaxSeq; i++ )
       m_outstanding[i] = 0;

  m_last_receive_timestamp = cTime::Now();
}


cIpmiCon::~cIpmiCon()
{
  assert( !IsRunning() );
  RequeueOutstanding();

  while( m_queue )
     {
       cIpmiRequest *r = (cIpmiRequest *)m_queue->data;
       delete r;

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
       m_current_seq %= m_max_seq;
     }

  r->m_seq = m_current_seq;

  m_outstanding[m_current_seq] = r;
  m_num_outstanding++;

  m_current_seq++;
  m_current_seq %= m_max_seq;

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
cIpmiCon::HandleMsgError( cIpmiRequest *r, SaErrorT err )
{
  // try again
  if ( r->m_retries_left > 0 )
     {
       m_log_lock.Lock();
       stdlog << "timeout: resending message.\n";
       m_log_lock.Unlock();

       m_queue = g_list_append( m_queue, r );

       // if a check connection is not in progress
       // initiate a check connection
       cTime timeout = m_last_receive_timestamp;
       timeout += m_timeout;

       if (    !m_check_connection
               && timeout < cTime::Now() )
          {
            m_check_connection = true;

            bool check = IfCheckConnection( timeout );

            if ( !check )
                 m_check_connection = false;
            else
                 m_check_connection_timeout = timeout;
          }

       return;
     }

  // error while sending command
  m_log_lock.Lock();

  if ( err == SA_ERR_HPI_TIMEOUT )
       stdlog << ">tim " << (unsigned char)r->m_seq << "\n";
  else
       stdlog << ">err " << (unsigned char)r->m_seq << " " << err << "\n";

  m_log_lock.Unlock();

  r->m_error = err;
  r->m_signal->Lock();
  r->m_signal->Signal();
  r->m_signal->Unlock();
}


SaErrorT
cIpmiCon::SendCmd( cIpmiRequest *request )
{
  assert( m_num_outstanding < m_max_outstanding );

  request->m_retries_left--;
  assert( request->m_retries_left >= 0 );

  int seq = AddOutstanding( request );

  if ( m_log_level & dIpmiConLogCmd )
     {
       m_log_lock.Lock();

       stdlog <<  ">cmd " << (unsigned char)seq << "  ";
       IpmiLogDataMsg( request->m_addr, request->m_msg );
       stdlog << "\n";

       m_log_lock.Unlock();
     }

  // message timeout
  request->m_timeout = cTime::Now();

  // add timeout
  request->m_timeout += m_timeout;

  // addr translation
  IfAddrToSendAddr( request->m_addr, request->m_send_addr );

  // send message
  SaErrorT rv = IfSendCmd( request );

  if ( rv != SA_OK )
     {
       RemOutstanding( seq );
       return rv;
     }

  return SA_OK;
}


void
cIpmiCon::SendCmds()
{
  while( m_queue && m_num_outstanding < m_max_outstanding )
     {
       cIpmiRequest *r = (cIpmiRequest *)m_queue->data;
       m_queue = g_list_remove( m_queue, r );

       SaErrorT rv = SendCmd( r );

       if ( rv != SA_OK )
            HandleMsgError( r, rv );
     }
}


void *
cIpmiCon::Run()
{
  stdlog << "starting reader thread.\n";

  // create pollfd
  struct pollfd pfd;

  pfd.events = POLLIN;

  // reader loop
  while( !m_exit )
     {
       // check for check connction timeout
       if ( m_check_connection )
          {
            cTime now = cTime::Now();

            if ( now >= m_check_connection_timeout )
               {
                 IfCheckConnectionTimeout();

                 // if a new connection is established
                 // resend messages 
                 m_queue_lock.Lock();
                 SendCmds();
                 m_queue_lock.Unlock();

                 m_check_connection = false;
               }
          }

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
                 stdlog << "poll returns " << rv << ", " << errno << ", " 
                        << strerror( errno ) << " !\n";
                 assert( 0 );
               }
          }

       // check for expiered ipmi commands
       cTime now = cTime::Now();

       m_queue_lock.Lock();

       for( int i = 0; i < m_max_seq; i++ )
          {
            if ( m_outstanding[i] == 0 )
                 continue;

            cIpmiRequest *r = m_outstanding[i];

            if ( r->m_timeout > now )
                 continue;

            stdlog << "IPMI msg timeout: addr " << r->m_addr.m_slave_addr << " "
                   << IpmiCmdToString( r->m_msg.m_netfn, r->m_msg.m_cmd )
                   << ", seq " << (unsigned char)r->m_seq
                   << ", timeout " << (int)r->m_timeout.m_time.tv_sec << " " << (int)r->m_timeout.m_time.tv_usec 
                   << ", now " << (int)now.m_time.tv_sec << " " << (int)now.m_time.tv_usec << "!\n";

            // timeout expired
            RemOutstanding( r->m_seq );

            HandleMsgError( r, SA_ERR_HPI_TIMEOUT );
          }

       // send new comands
       SendCmds();

       m_queue_lock.Unlock();
     }

  stdlog << "stop reader thread.\n";

  return 0;
}


void
cIpmiCon::IfClose()
{
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
cIpmiCon::IfCheckConnection( cTime & /*timeout*/ )
{
  // no connection check
  return false;
}


void
cIpmiCon::IfCheckConnectionTimeout()
{
}


void
cIpmiCon::HandleCheckConnection( bool state )
{
  if ( state )
       m_last_receive_timestamp = cTime::Now();

  m_check_connection = false;
}


bool
cIpmiCon::Open()
{
  if ( IsOpen() )
       return true;

  m_max_seq = IfGetMaxSeq();
  
  assert( m_max_seq > 0 && m_max_seq <= dMaxSeq );

  m_fd = IfOpen();

  if ( m_fd == -1 )
       return false;

  m_last_receive_timestamp = cTime::Now();

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
SaErrorT
cIpmiCon::Cmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
               cIpmiAddr &rsp_addr, cIpmiMsg &rsp, int retries )
{
  assert( retries > 0 );

  SaErrorT rv;

  assert( msg.m_data_len <= dIpmiMaxMsgLength );
  assert( IsRunning() );

  cThreadCond cond;

  // create request
  cIpmiRequest *r = new cIpmiRequest( addr, msg );
  r->m_rsp_addr     = &rsp_addr;
  r->m_rsp          = &rsp;
  r->m_signal       = &cond;
  r->m_error        = SA_ERR_HPI_INVALID_CMD;
  r->m_retries_left = retries;

  // lock queue
  cond.Lock();
  m_queue_lock.Lock();

  if ( m_num_outstanding < m_max_outstanding )
     {
       // send the command within this thread context.
       rv = SendCmd( r );

       if ( rv != SA_OK )
	  {
	    // error
	    delete r;

	    m_queue_lock.Unlock();
	    cond.Unlock();
	    return rv;
	  }
     }
  else
     {
       stdlog << "send queue full.\n";
       m_queue = g_list_append( m_queue, r );       
     }

  m_queue_lock.Unlock();

  // wait for response
  cond.Wait();
  cond.Unlock();

  rv = r->m_error;

  delete r;

  if ( rv == SA_OK )
     {
       if ( ((tIpmiNetfn)(msg.m_netfn | 1) != rsp.m_netfn)
           || (msg.m_cmd != rsp.m_cmd) )
       {
            stdlog << "Mismatch send netfn " << msg.m_netfn << " cmd " << msg.m_cmd << ", recv netfn " << rsp.m_netfn << " cmd " << rsp.m_cmd << "\n";
            rv = SA_ERR_HPI_INTERNAL_ERROR;
       }
     }

  return rv;
}


SaErrorT
cIpmiCon::ExecuteCmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
                      cIpmiMsg &rsp_msg, int retries )
{
  cIpmiAddr rsp_addr;

  return Cmd( addr, msg, rsp_addr, rsp_msg, retries );
}


void
cIpmiCon::HandleResponse( int seq, const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  m_last_receive_timestamp = cTime::Now();

  m_queue_lock.Lock();

  if ( m_outstanding[seq] == 0 )
     {
       m_log_lock.Lock();

       stdlog << "reading response without request:\n";
       stdlog << "# " << (unsigned char)seq << "  ";
       IpmiLogDataMsg( addr, msg );
       stdlog << "\n";

       m_log_lock.Unlock();

       m_queue_lock.Unlock();

       return;
     }

  cIpmiRequest *r = m_outstanding[seq];
  assert( r->m_seq == seq );

  if ( m_log_level & dIpmiConLogCmd )
     {
       m_log_lock.Lock();

       stdlog << "<rsp " << (unsigned char)r->m_seq << "  ";
       IpmiLogDataMsg( addr, msg );
       stdlog << "\n";

       m_log_lock.Unlock();
     }

  RemOutstanding( seq );

  // addr translation
  *r->m_rsp_addr = addr;

  // convert braodcast to ipmb
  if ( r->m_rsp_addr->m_type == eIpmiAddrTypeIpmbBroadcast )
       r->m_rsp_addr->m_type = eIpmiAddrTypeIpmb;

  r->m_error     = SA_OK;
  *r->m_rsp      = msg;

  r->m_signal->Lock();
  r->m_signal->Signal();
  r->m_signal->Unlock();

  m_queue_lock.Unlock();
}


void
cIpmiCon::HandleEvent( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  m_last_receive_timestamp = cTime::Now();

  if ( m_log_level & dIpmiConLogEvent )
     {
       m_log_lock.Lock();

       stdlog << ">evt ";
       IpmiLogDataMsg( addr, msg );
       stdlog << "\n";

       m_log_lock.Unlock();
     }

  HandleAsyncEvent( addr, msg );
}


void
IpmiLogDataMsg( const cIpmiAddr &addr, const cIpmiMsg &msg )
{
  char str[1024];
  char *s = str;
  int remaining;

  // addr
  switch( addr.m_type )
     {
       case eIpmiAddrTypeIpmb:
            s += snprintf( s, sizeof(str), "%02x %02x %02x %02x",
                           eIpmiAddrTypeIpmb, addr.m_channel, addr.m_lun, addr.m_slave_addr );
            break;

       case eIpmiAddrTypeSystemInterface:
            s += snprintf( s, sizeof(str), "%02x %02x %02x   ",
                           eIpmiAddrTypeSystemInterface, addr.m_channel, addr.m_lun );
            break;

       case eIpmiAddrTypeIpmbBroadcast:
            s += snprintf( s, sizeof(str), "%02x %02x %02x %02x",
                           eIpmiAddrTypeIpmbBroadcast, addr.m_channel, addr.m_lun, addr.m_slave_addr );
     }

  remaining = sizeof(str) - (s - str);

  if (remaining > 0)
     s += snprintf( s, remaining, "  %s (%02d) ",
                    IpmiCmdToString( (tIpmiNetfn)(msg.m_netfn & 0xfe), msg.m_cmd ), msg.m_data_len );

  const unsigned char *p = msg.m_data;

  for( int i = 0; i < msg.m_data_len; i++ )
  {
       remaining = sizeof(str) - (s - str);
       if (remaining > 0)
            s += snprintf( s, remaining, " %02x", *p++ );
       else
            break;
  }

  stdlog << str;
}
