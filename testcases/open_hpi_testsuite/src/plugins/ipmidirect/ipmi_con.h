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

#ifndef dThread_h
#include "thread.h"
#endif

#include <glib.h>


#define dIpmiUsernameMax   16

class cIpmiDomain;


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
  int            m_error;  // if != 0 => error
  cThreadCond   *m_signal; // the calling thread is waiting for this
  struct timeval m_timeout;
  int            m_retries_left;

  cIpmiRequest( const cIpmiAddr &addr, const cIpmiMsg &msg )
    : m_addr( addr ), m_send_addr( addr ), m_msg( msg ), m_rsp_addr( 0 ), m_rsp( 0 ),
    m_error( 0 ), m_signal( 0 ), m_retries_left( 3 )  {}

  virtual ~cIpmiRequest() {}
};


#define dMaxSeq 256


class cIpmiCon : public cThread
{
protected:
  cIpmiDomain  *m_domain;
  bool          m_is_open;

  // file handle returned by IfOpen
  int           m_fd;

  // address of SMI
  unsigned char m_slave_addr;

  cThreadLock   m_log_lock;

  int           m_max_outstanding; // must be <= dMaxSeq

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
  int SendCmd( cIpmiRequest *request );

  // send the first command of the given queue
  void SendCmds();

  // thread entry function
  virtual void *Run();

  // signal the exit of the thread
  bool m_exit;

public:
  // default timeouts
  unsigned int m_default_ipmi_timeout; // for IPMI
  unsigned int m_default_atca_timeout; // for ATCA

  // current timeout
  unsigned int m_timeout;

  bool CheckPending()
  {
    bool rv = false;

    m_queue_lock.Lock();

    if ( m_num_outstanding || m_queue )
         rv = true;
    
    m_queue_lock.Unlock();

    return rv;
  }

public:
  cIpmiCon( unsigned int ipmi_timeout, unsigned int atca_timeout,
            unsigned int max_outstanding );
  virtual ~cIpmiCon();

  void SetDomain( cIpmiDomain *domain )
  {
    m_domain = domain;
  }

  bool IsOpen() { return m_is_open; }

protected:
  // connection interface functions

  // open connection return file handle
  virtual int  IfOpen() = 0;

  // close connection
  virtual void IfClose();

  // alloc/free requests
  virtual cIpmiRequest *IfAllocRequest( const cIpmiAddr &addr, const cIpmiMsg &msg );
  virtual void          IfDestroyRequest( cIpmiRequest *r );

  // convertion from addr to send addr
  virtual void IfAddrToSendAddr( const cIpmiAddr &addr, cIpmiAddr &send_addr );

  // send an ipmi command
  virtual int  IfSendCmd( cIpmiRequest *r ) = 0;

  // read ipmi response
  virtual void IfReadResponse() = 0;

protected:
  // handle response called within IfReadResponse
  virtual void HandleResponse( int seq, const cIpmiAddr &addr, const cIpmiMsg &msg );

  // handle event called within IfReadResponse
  virtual void HandleEvent( const cIpmiAddr &addr, const cIpmiMsg &msg );

public:
  bool  Open();
  void  Close();
  int   Cmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
             cIpmiAddr &rsp_addr, cIpmiMsg &rsp_msg );

  int   ExecuteCmd( const cIpmiAddr &addr, const cIpmiMsg &msg,
                    cIpmiMsg &rsp_msg );
};


void IpmiLogDataMsg( const cIpmiAddr &addr, const cIpmiMsg &msg );


#endif
