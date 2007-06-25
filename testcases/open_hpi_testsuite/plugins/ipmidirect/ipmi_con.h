/*
 * ipmi_con.h
 *
 * abstract interface for handling IPMI connections
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

#ifndef dIpmiCon_h
#define dIpmiCon_h


#ifndef dIpmiLog_h
#include "ipmi_log.h"
#endif

#ifndef dIpmiAddr_h
#include "ipmi_addr.h"
#endif

#ifndef dIpmiMsg_h
#include "ipmi_msg.h"
#endif

#ifndef dIpmiCmd_h
#include "ipmi_cmd.h"
#endif

#ifndef dIpmiUtils_h
#include "ipmi_utils.h"
#endif

#ifndef dThread_h
#include "thread.h"
#endif

extern "C" {
#include "SaHpi.h"
}


#include <glib.h>

#define dIpmiConLogCmd   1
#define dIpmiConLogEvent 2
#define dIpmiConLogAll   0xffff


// default retries for an IPMI command before timeoue
#define dIpmiDefaultRetries 3


// ipmi command
class cIpmiRequest
{
public:
  cIpmiAddr      m_addr;
  cIpmiAddr      m_send_addr;
  cIpmiMsg       m_msg;
  int            m_seq;  // seq of msg
  cIpmiAddr     *m_rsp_addr;
  cIpmiMsg      *m_rsp;
  SaErrorT       m_error;  // if != 0 => error
  cThreadCond   *m_signal; // the calling thread is waiting for this
  cTime          m_timeout;
  int            m_retries_left;

  cIpmiRequest( const cIpmiAddr &addr, const cIpmiMsg &msg )
    : m_addr( addr ), m_send_addr( addr ), m_msg( msg ), m_rsp_addr( 0 ), m_rsp( 0 ),
    m_error( SA_OK ), m_signal( 0 ), m_retries_left( -1 )  {}

  virtual ~cIpmiRequest() {}
};


#define dMaxSeq 256


class cIpmiCon : public cThread
{
protected:
  bool          m_is_open;

  // file handle returned by IfOpen
  int           m_fd;

  // address of SMI
  unsigned char m_slave_addr;

  cThreadLock   m_log_lock;

  // maximum outstanding requests
  int           m_max_outstanding; // must be <= dMaxSeq

  // max seq number
  int           m_max_seq; // must be <= dMaxSeq

  // lock for m_queue and m_outstanding
  cThreadLock   m_queue_lock;
  GList        *m_queue;

  cIpmiRequest *m_outstanding[dMaxSeq];
  int           m_num_outstanding;
  int           m_current_seq;

  void RequeueOutstanding();
  int  AddOutstanding( cIpmiRequest *r );
  void RemOutstanding( int seq );
  void HandleMsgError( cIpmiRequest *r, int err );

  // send a command
  SaErrorT SendCmd( cIpmiRequest *request );

  // send the first command of the given queue
  void SendCmds();

  // thread entry function
  virtual void *Run();

  // signal the exit of the thread
  bool m_exit;

  // log output
  int m_log_level;

public:
  bool LogLevel( int v )
  {
    return m_log_level & v;
  }
  
  // current timeout in ms
  unsigned int m_timeout;

public:
  cIpmiCon( unsigned int timeout, int log_level );
  virtual ~cIpmiCon();

  bool IsOpen() { return m_is_open; }

protected:
  // get number of seq ids
  virtual int IfGetMaxSeq() = 0;

  // connection interface functions

  // open connection return file handle
  virtual int  IfOpen() = 0;

  // close connection
  virtual void IfClose();

  // convertion from addr to send addr
  virtual void IfAddrToSendAddr( const cIpmiAddr &addr, cIpmiAddr &send_addr );

  // send an ipmi command
  virtual SaErrorT IfSendCmd( cIpmiRequest *r ) = 0;

  // read ipmi response
  virtual void IfReadResponse() = 0;

  // true => connection check mode
  bool        m_check_connection;
  cTime       m_check_connection_timeout;

  // called to check the connection
  // true => going to check connection mode
  virtual bool IfCheckConnection( cTime &timeout );

  // called in case of check connection timeout
  virtual void IfCheckConnectionTimeout();

protected:
  cTime m_last_receive_timestamp;
  
  // handle response called within IfReadResponse
  virtual void HandleResponse( int seq, const cIpmiAddr &addr, const cIpmiMsg &msg );

  // handle event called within IfReadResponse
  virtual void HandleEvent( const cIpmiAddr &addr, const cIpmiMsg &msg );

  virtual void HandleAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg ) = 0;

  // handle a check connection response
  virtual void HandleCheckConnection( bool state );

public:
  bool  Open();
  void  Close();
  SaErrorT Cmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
                cIpmiAddr &rsp_addr, cIpmiMsg &rsp_msg,
                int retries = dIpmiDefaultRetries );

  SaErrorT ExecuteCmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
                       cIpmiMsg &rsp_msg,
                       int retries = dIpmiDefaultRetries );

  int GetMaxOutstanding() { return m_max_outstanding; }
  bool SetMaxOutstanding( int max )
  {
    if ( max < 1 || max > 32 )
         return false;

    m_max_outstanding = max;

    return true;
  }
};


void IpmiLogDataMsg( const cIpmiAddr &addr, const cIpmiMsg &msg );


#endif
