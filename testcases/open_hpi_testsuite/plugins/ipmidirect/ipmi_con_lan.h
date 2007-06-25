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


#ifndef dIpmiConLan_h
#define dIpmiConLan_h


#include <netinet/in.h>

#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif

#ifndef dIpmiAuth_h
#include "ipmi_auth.h"
#endif


// standard RMCP port
#define dIpmiConLanStdPort 623


// # of times to try a message before we fail it.
#define dLanRspRetries 2

#define dLanPingTimeout 2000

#define dAsfIana 0xbe110000


class cIpmiConLan : public cIpmiCon
{
protected:
  struct sockaddr_in m_ip_addr;
  int                m_port;

  tIpmiAuthType      m_auth;
  tIpmiPrivilege     m_priv;
  cIpmiAuth         *m_auth_method;

  char               m_username[dIpmiUsernameMax+1];
  char               m_passwd[dIpmiPasswordMax+1];

  uint32_t           m_session_id;
  tIpmiAuthType      m_working_auth;

  // connection challange
  unsigned char      m_challenge_string[16];

  // outstanding pongs
  int                m_ping_count;

  uint32_t           m_outbound_seq_num;
  uint32_t           m_inbound_seq_num;
  uint16_t           m_recv_msg_map;

  int AuthGen( unsigned char *out, uint8_t *ses_id,
               uint8_t *seq, unsigned char *data,
               unsigned int data_len );
  int AuthCheck( uint8_t *ses_id, uint8_t *seq,
                 unsigned char *data, unsigned int data_len,
                 unsigned char *code );

  int OpenLanFd();
  unsigned char Checksum( unsigned char *data, int size );

  int SendPing();
  bool WaitForPong( unsigned int timeout_ms );

  enum tResponseType
  {
    eResponseTypeError,
    eResponseTypePong,
    eResponseTypeMessage,
    eResponseTypeEvent,
    eResponseTypeTimeout
  };
  
  tResponseType ReadResponse( int &seq, cIpmiAddr &addr, cIpmiMsg &msg );

  tResponseType HandleData( int fd, cIpmiAddr &addr, cIpmiMsg &msg );
  tResponseType WaitForResponse( unsigned int timeout_ms, int &seq,
                                 cIpmiAddr &addr, cIpmiMsg &msg );
  SaErrorT SendMsgAndWaitForResponse( const cIpmiAddr &addr, const cIpmiMsg &msg,
                                      cIpmiAddr &rsp_addr, cIpmiMsg &rsp_msg );

  SaErrorT AuthCap();
  SaErrorT SetSessionPriv();
  SaErrorT ActiveSession();
  SaErrorT Challange();
  void     Reconnect();
  void     SendCloseSession();
  SaErrorT CreateSession();

public:
  cIpmiConLan( unsigned int timeout, int log_level,
               struct in_addr addr, int port,
               tIpmiAuthType auth, tIpmiPrivilege priv,
               char *user, char *passwd );
  virtual ~cIpmiConLan();

protected:
  virtual int  IfGetMaxSeq();
  virtual int  IfOpen();
  virtual void IfClose();
  virtual SaErrorT IfSendCmd( cIpmiRequest *r );
  virtual void IfReadResponse();

  virtual bool IfCheckConnection( cTime &timeout );
  virtual void IfCheckConnectionTimeout();
};


#endif
