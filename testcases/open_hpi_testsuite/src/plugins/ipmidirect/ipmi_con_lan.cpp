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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>
#include <sys/time.h>

#include "ipmi_con_lan.h"
#include "ipmi_utils.h"
#include "ipmi_addr.h"
#include "ipmi_cmd.h"


#define dIpmiMaxLanLen (dIpmiMaxMsgLength + 42)


cIpmiConLan::cIpmiConLan( unsigned int timeout, unsigned int atca_timeout,
                          unsigned int max_outstanding, struct in_addr addr, int port,
                          tIpmiAuthType auth, tIpmiPrivilege priv,
                          char *user, char *passwd )
  : cIpmiCon( timeout, atca_timeout, max_outstanding ),
    m_port( port ), m_auth( auth ), m_priv( priv ), m_auth_method( 0 ),
    m_session_id( 0 ), m_working_auth( eIpmiAuthTypeNone ),
    m_ping_count( 0 ),
    m_outbound_seq_num( 0 ), m_inbound_seq_num( 0 ),
    m_recv_msg_map( 0 )
{
  assert( strlen( user ) < dIpmiUsernameMax );
  assert( strlen( passwd ) < dIpmiPasswordMax );

  m_ip_addr.sin_family = AF_INET;
  m_ip_addr.sin_port = htons( port );
  m_ip_addr.sin_addr = addr;
  m_port = port;

  memset( m_username, 0, dIpmiUsernameMax );
  strcpy( m_username, user );

  memset( m_passwd, 0, dIpmiPasswordMax );
  strcpy( m_passwd, passwd );
}


cIpmiConLan::~cIpmiConLan()
{
  if ( IsOpen() )
       Close();

  if ( m_auth_method )
       delete m_auth_method;
}


int
cIpmiConLan::AuthGen( unsigned char *out,
                      uint8_t       *ses_id,
                      uint8_t       *seq,
                      unsigned char *data,
                      unsigned int   data_len )
{
  assert( m_auth == m_working_auth );
  assert( m_auth_method );
  
  int rv;
  cIpmiAuthSg l[] =
  {
    { ses_id, 4 },
    { data,   data_len },
    { seq,    4 },
    { NULL,   0 }
  };

  rv = m_auth_method->Gen( l, out );

  return rv;
}


int
cIpmiConLan::AuthCheck( uint8_t       *ses_id,
                        uint8_t       *seq,
                        unsigned char *data,
                        unsigned int   data_len,
                        unsigned char *code )
{
  assert( m_auth == m_working_auth );
  assert( m_auth_method );

  int rv;
  cIpmiAuthSg l[] =
  {
    { ses_id, 4  },
    { data,   data_len },
    { seq,    4 },
    { NULL,   0 }
  };

  rv = m_auth_method->Check( l, code );

  return rv;
}


int
cIpmiConLan::OpenLanFd()
{
  int                fd;
  struct sockaddr_in addr;
  int                curr_port;
  int                rv;

  fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

  if ( fd == -1 )
       return fd;

  curr_port = 7000;

  do
     {
       curr_port++;
       addr.sin_family = AF_INET;
       addr.sin_port = htons(curr_port);
       addr.sin_addr.s_addr = INADDR_ANY;

       rv = bind( fd, (struct sockaddr *)&addr, sizeof( addr ) );
     }
  while( curr_port < 7100 && rv == -1 );

  if ( rv == -1 )
     {
       int tmp_errno = errno;
       close( fd );
       errno = tmp_errno;
       return -1;
     }

  IpmiLog( "using port %d.\n", curr_port );

  return fd;
}


unsigned char
cIpmiConLan::Checksum( unsigned char *data, int size )
{
  unsigned char csum = 0;

  for( ; size > 0; size--, data++ )
       csum += *data;

  return -csum;
}


int
cIpmiConLan::SendPing()
{
  unsigned char data[dIpmiMaxLanLen];

  data[0]  = 6; // RMCP version 1.0.
  data[1]  = 0;
  data[2]  = 0xff; // no RMCP ACK
  data[3]  = 0x06; // ASF
  IpmiSetUint32( data + 4,  dAsfIana );
  data[8]  = 0x80; // presence ping
  data[9]  = 0xff; // ????
  data[10] = 0x00;
  data[11] = 0x00;

  IpmiLog( "sending RMCP ping.\n" );

  int rv = sendto( m_fd, data, 12, 0,
                   (struct sockaddr *)&m_ip_addr,
                   sizeof( struct sockaddr_in ) );

  if ( rv == -1 )
       return errno;

  m_ping_count++;

  return 0;
}


cIpmiConLan::tResponseType
cIpmiConLan::WaitForResponse( unsigned int timeout_ms, int &seq,
                              cIpmiAddr &addr, cIpmiMsg &msg )
{
  struct timeval tv;
  struct timeval timeout;
  struct timeval t0;

  // create absolute timeout
  gettimeofday( &timeout, 0 );

  timeout.tv_sec += timeout_ms / 1000;
  timeout.tv_usec += (timeout_ms % 1000) * 1000;

  while( timeout.tv_usec > 10000000 )
     {
       timeout.tv_sec++;
       timeout.tv_usec -= 10000000;
     }

  tResponseType ret;

  // loop
  do
     {
       struct pollfd pfd;

       pfd.fd = m_fd;
       pfd.events = POLLIN;

       // relative timeout
       gettimeofday( &t0, 0 );

       timersub( &timeout, &t0, &tv );

       if ( tv.tv_sec < 0 || tv.tv_usec < 0 )
	  {
	    tv.tv_sec  = 0;
	    tv.tv_usec = 0;
	  }

       timeout_ms =  tv.tv_sec * 1000 + tv.tv_usec / 1000;

       int rv = poll( &pfd, 1, timeout_ms );

       // timeout
       if ( !rv )
	    return eResponseTypeTimeout;

       if ( rv == -1 )
	  {
	    assert( 0 );
	    return eResponseTypeError;
	  }

       assert( rv == 1 );
       ret = ReadResponse( seq, addr, msg );
     }
  while( ret != eResponseTypeMessage );

  return eResponseTypeMessage;
}


int
cIpmiConLan::SendMsgAndWaitForResponse( const cIpmiAddr &addr, const cIpmiMsg &msg,
                                        cIpmiAddr &rsp_addr, cIpmiMsg &rsp_msg )
{
  cIpmiRequest *r = new cIpmiRequest( addr, msg );
  int seq;

  while( r->m_retries_left > 0 )
     {
       int rv = SendCmd( r );

       if ( rv )
            continue;

       tResponseType rt;

       do
          {
            rt = WaitForResponse( m_timeout, seq, rsp_addr,
                                   rsp_msg );
          }
       while( rt == eResponseTypeEvent || rt == eResponseTypePong );

       RemOutstanding( r->m_seq );

       if ( rt == eResponseTypeMessage )
          {
            // check seq
            if ( seq == r->m_seq )
               {
                 delete r;
                 return 0;
               }
          }

       // resend message
       IpmiLog( "resending RMCP msg.\n" );
     }

  return ETIMEDOUT;
}


int
cIpmiConLan::AuthCap()
{
  int       rv;
  cIpmiAddr addr( eIpmiAddrTypeSystemInterface );
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdGetChannelAuthCapabilities );
  cIpmiAddr rsp_addr;
  cIpmiMsg  rsp_msg;

  msg.m_data[0] = 0xe;
  msg.m_data[1] = m_priv;
  msg.m_data_len = 2;

  // dbg( "RMCP: sending GetChannelAuthCapabilitiesCmd" );

  rv = SendMsgAndWaitForResponse( addr, msg,
                                  rsp_addr, rsp_msg );

  if ( rv )
       return rv;

  if (    (rsp_msg.m_data[0] != 0 )
       || (rsp_msg.m_data_len < 9 ) )
     {
       IpmiLog( "auth response = 0x%02x !\n", rsp_msg.m_data[0] );
       return EINVAL;
     }

  if ( !( rsp_msg.m_data[2] & (1 << m_auth ) ) )
     {
       IpmiLog( "Requested authentication not supported !\n" );

       char str[256] = "";

       if ( rsp_msg.m_data[2] & ( 1 << eIpmiAuthTypeNone ) )
            strcat( str, " none" );

       if ( rsp_msg.m_data[2] & ( 1 << eIpmiAuthTypeMd2 ) )
            strcat( str, " md2" );

       if ( rsp_msg.m_data[2] & ( 1 << eIpmiAuthTypeMd5 ) )
            strcat( str, " md5" );

       if ( rsp_msg.m_data[2] & ( 1 << eIpmiAuthTypeStraight ) )
            strcat( str, " straight" );

       if ( rsp_msg.m_data[2] & ( 1 << eIpmiAuthTypeOem ) )
            strcat( str, " oem" );

       IpmiLog( "supported authentication types:%s.\n", str );

       return EINVAL;
     }

  return 0;
}


int
cIpmiConLan::SetSessionPriv()
{
  int       rv;
  cIpmiAddr addr( eIpmiAddrTypeSystemInterface );
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdSetSessionPrivilege );
  cIpmiAddr rsp_addr;
  cIpmiMsg  rsp_msg;

  msg.m_data[0]  = m_priv;
  msg.m_data_len = 1;

  // dbg( "RMCP: sending SetSessionPrivilegeCmd" );

  rv = SendMsgAndWaitForResponse( addr, msg,
                                  rsp_addr, rsp_msg );

  if ( rv )
       return rv;

  if ( rsp_msg.m_data[0] != 0 )
     {
       IpmiLog( "set session priv: 0x%02x !\n", rsp_msg.m_data[0] );
       return EINVAL;
     }

  if ( rsp_msg.m_data_len < 2 )
     {
       IpmiLog( "set session priv: msg to small: %d !\n", rsp_msg.m_data_len );
       return EINVAL;
     }

  if ( (unsigned char)m_priv != (rsp_msg.m_data[1] & 0xf))
     {
       // Requested privilege level did not match.
       IpmiLog( "set session priv: Requested privilege level did not match: %d, %d !\n",
                m_priv, (rsp_msg.m_data[1] & 0xf ) );

       return EINVAL;
     }

  return 0;
}


int
cIpmiConLan::ActiveSession()
{
  int       rv;
  cIpmiAddr addr( eIpmiAddrTypeSystemInterface );
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdActivateSession );
  cIpmiAddr rsp_addr;
  cIpmiMsg  rsp_msg;

  msg.m_data[0] = m_auth;
  msg.m_data[1] = m_priv;
  memcpy( msg.m_data + 2, m_challenge_string, 16 );
  IpmiSetUint32( msg.m_data + 18, m_inbound_seq_num );
  msg.m_data_len = 22;

  // dbg( "RMCP: sending ActivateSessionCmd" );

  rv = SendMsgAndWaitForResponse( addr, msg,
                                  rsp_addr, rsp_msg );

  if ( rv )
       return rv;

  if ( rsp_msg.m_data[0] != 0 )
     {
       IpmiLog( "active session: 0x%02x !\n", rsp_msg.m_data[0] );
       return EINVAL;
     }

  if ( rsp_msg.m_data_len < 11 )
     {
       IpmiLog( "active session: msg to small: %d !\n", rsp_msg.m_data_len );
       return EINVAL;
     }

  m_working_auth = (tIpmiAuthType)(rsp_msg.m_data[1] & 0xf);

  if (    m_working_auth != 0
       && m_working_auth != m_auth )
     {
       // Eh?  It didn't return a valid authtype.
       IpmiLog( "active session: wrong auth: %d !\n", m_working_auth );
       return EINVAL;
     }

  m_session_id = IpmiGetUint32( rsp_msg.m_data + 2 );
  m_outbound_seq_num = IpmiGetUint32( rsp_msg.m_data + 6 );

  //        dbg( "reading: sid = 0x%x, seq = 0x%x", 
  //             m_session_id, m_outbound_seq_num );
        
  return 0;
}


int
cIpmiConLan::Challange()
{
  int       rv;
  cIpmiAddr addr( eIpmiAddrTypeSystemInterface );
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdGetSessionChallenge );
  cIpmiAddr rsp_addr;
  cIpmiMsg  rsp_msg;

  msg.m_data[0]  = m_auth;
  msg.m_data_len = 1;
  memcpy( msg.m_data + 1, m_username, dIpmiUsernameMax );
  msg.m_data_len += dIpmiUsernameMax;

  // dbg( "RMCP: sending GetSessionChallengeCmd" );

  rv = SendMsgAndWaitForResponse( addr, msg,
                                  rsp_addr, rsp_msg );

  if ( rv )
       return rv;

  if ( rsp_msg.m_data[0] != 0 )
     {
       IpmiLog( "Challange returns: 0x%02x !\n",  rsp_msg.m_data[0] );
       return EINVAL;
     }

  if ( rsp_msg.m_data_len < 21 )
     {
       IpmiLog( "Challange response to small !\n" );
       return EINVAL;
     }

  // Get the temporary session id.
  m_session_id = IpmiGetUint32( rsp_msg.m_data + 1 );

  m_outbound_seq_num = 0;
  m_working_auth     = m_auth;
  memcpy( m_challenge_string, rsp_msg.m_data + 5, 16 );

  // Get a random number of the other end to start sending me sequence
  // numbers at, but don't let it be zero.
  while( m_inbound_seq_num == 0 )
       m_inbound_seq_num = random();

  return 0;
}


int
cIpmiConLan::IfOpen()
{
  int rv;

  m_ping_count       = 0;
  m_outbound_seq_num = 0;
  m_session_id       = 0;
  m_working_auth     = eIpmiAuthTypeNone;
  m_recv_msg_map     = 0;
  m_inbound_seq_num  = 0;

  m_auth_method = IpmiAuthFactory( m_auth );

  if ( m_auth_method == 0 )
     {
       IpmiLog( "unknown authentication method %d!\n", m_auth );
       return -1;
     }

  m_auth_method->Init( (unsigned char *)m_passwd );

  m_fd = OpenLanFd();

  if ( m_fd < 0 )
       return -1;

  rv = AuthCap();

  if ( rv )
     {
       close( m_fd );
       return -1;
     }

  rv = Challange();
        
  if ( rv )
     {
       close( m_fd );
       return -1;
     }

  rv = ActiveSession();

  if ( rv )
     {
       close( m_fd );
       return -1;
     }

  rv = SetSessionPriv();

  if ( rv )
     {
       close( m_fd );
       return rv;
     }

  IpmiLog( "RMCP session is up.\n" );

  return m_fd;
}


// Send a final "close session" to shut down the connection.

void
cIpmiConLan::SendCloseSession()
{
  cIpmiAddr si( eIpmiAddrTypeSystemInterface );
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdCloseSession );

  IpmiSetUint32( msg.m_data, m_session_id );
  msg.m_data_len = 4;

  cIpmiRequest r( si, msg );
  r.m_seq = 1;

  IfSendCmd( &r );
}


void
cIpmiConLan::IfClose()
{
  if ( m_fd >= 0 )
     {
       SendCloseSession();

       close( m_fd );
       m_fd = -1;

       if ( m_auth_method )
          {
            delete m_auth_method;
            m_auth_method = 0;
          }
     }
}


void
cIpmiConLan::Reconnect()
{
  IpmiLog( "lan reconnection in progress.\n" );

  if ( m_fd >= 0 )
     {
       close( m_fd );
       m_fd = -1;
     }

  do
       sleep( 1 );
  while( IfOpen() );

  IpmiLog( "lan reconnection done.\n" );
}


int
cIpmiConLan::IfSendCmd( cIpmiRequest *r )
{
  IfAddrToSendAddr( r->m_addr, r->m_send_addr );

  unsigned char  data[dIpmiMaxLanLen];
  unsigned char *tmsg;
  int            pos;
  int            msgstart;
  int            rv;

  switch( r->m_send_addr.m_type )
     {
       case eIpmiAddrTypeSystemInterface:
       case eIpmiAddrTypeIpmb:
       case eIpmiAddrTypeIpmbBroadcast:
	    break;

       default:
            assert( 0 );
	    return EINVAL;
     }

  data[0] = 6; // RMCP version 1.0.
  data[1] = 0;
  data[2] = 0xff;
  data[3] = 0x07;
  data[4] = m_working_auth;

  IpmiSetUint32( data+5, m_outbound_seq_num );
  IpmiSetUint32( data+9, m_session_id );

  if ( m_working_auth == 0 )
       tmsg = data + 14;
  else
       tmsg = data + 30;

  if ( r->m_send_addr.m_type == eIpmiAddrTypeSystemInterface )
     {
       // It's a message straight to the BMC.
       tmsg[0] = 0x20; // To the BMC.
       tmsg[1] = (r->m_msg.m_netfn << 2) | r->m_send_addr.m_lun;
       tmsg[2] = Checksum( tmsg, 2 );
       tmsg[3] = 0x81; // Remote console IPMI Software ID
       tmsg[4] = r->m_seq << 2;
       tmsg[5] = r->m_msg.m_cmd;
       memcpy( tmsg + 6, r->m_msg.m_data, r->m_msg.m_data_len );
       pos = r->m_msg.m_data_len + 6;
       tmsg[pos] = Checksum( tmsg + 3, pos - 3 );
       pos++;
     }
  else
     {
       // It's an IPMB address, route it using a send message
       // command.
       pos = 0;
       tmsg[pos++] = 0x20; // BMC is the bridge.
       tmsg[pos++] = (eIpmiNetfnApp << 2) | 0;
       tmsg[pos++] = Checksum( tmsg, 2 );
       tmsg[pos++] = 0x81; // Remote console IPMI Software ID
       tmsg[pos++] = r->m_seq << 2;
       tmsg[pos++] = eIpmiCmdSendMsg;
       tmsg[pos++] = r->m_send_addr.m_channel & 0xf;

       if ( r->m_send_addr.m_type == eIpmiAddrTypeIpmbBroadcast )
	    tmsg[pos++] = 0; // Do a broadcast.

       msgstart = pos;
       tmsg[pos++] = r->m_send_addr.m_slave_addr;
       tmsg[pos++] = (r->m_msg.m_netfn << 2) | r->m_send_addr.m_lun;
       tmsg[pos++] = Checksum( tmsg + msgstart, 2 );
       msgstart = pos;
       tmsg[pos++] = 0x81;
       tmsg[pos++] = r->m_seq << 2;
       tmsg[pos++] = r->m_msg.m_cmd;
       memcpy( tmsg + pos, r->m_msg.m_data, r->m_msg.m_data_len );
       pos += r->m_msg.m_data_len;
       tmsg[pos] = Checksum( tmsg + msgstart, pos - msgstart );
       pos++;
       tmsg[pos] = Checksum( tmsg + 3, pos - 3 );
       pos++;
     }

  if ( m_working_auth == 0 )
     {
       // No authentication, so no authcode.
       data[13] = pos;
       pos += 14; // Convert to pos in data
     } 
  else
     {
       data[29] = pos;

       rv = AuthGen( data+13, data+9, data+5, tmsg, pos );

       if ( rv )
	    return rv;

       pos += 30; // Convert to pos in data
     }

  // Increment the outbound number, but make sure it's not zero.  If
  // it's already zero, ignore it, we are in pre-setup.
  if ( m_outbound_seq_num != 0 )
     {
       m_outbound_seq_num++;

       if ( m_outbound_seq_num == 0 )
	    m_outbound_seq_num++;
     }

  rv = sendto( m_fd, data, pos, 0,
	       (struct sockaddr *)&m_ip_addr,
	       sizeof(struct sockaddr_in) );

  if ( rv == -1 )
       rv = errno;
  else
       rv = 0;

  return rv;
}


cIpmiConLan::tResponseType
cIpmiConLan::ReadResponse( int &seq, cIpmiAddr &addr, cIpmiMsg &msg )
{
  unsigned char       data[dIpmiMaxLanLen];
  struct sockaddr     ipaddrd;
  struct sockaddr_in *ipaddr;
  int                 len;
  socklen_t           from_len;
  uint32_t            sess_id;
  unsigned char      *tmsg;
  unsigned int        data_len;

  from_len = sizeof( ipaddrd );
  len = recvfrom( m_fd, data, dIpmiMaxLanLen, 0, &ipaddrd, &from_len );

  if ( len < 0 )
       return eResponseTypeError;

  //IpmiLog( "rmcp msg: " );
  //IpmiLogHex( data, len );

  // Make sure the source IP matches what we expect the other end to
  // be.
  ipaddr = (struct sockaddr_in *)&ipaddrd;

  if (    (ipaddr->sin_port != m_ip_addr.sin_port)
       || (ipaddr->sin_addr.s_addr != m_ip_addr.sin_addr.s_addr) )
     {
       IpmiLog( "Dropped message due to invalid IP !\n");
       return eResponseTypeError;
     }

  // Validate the length first, so we know that all the data in the
  // buffer we will deal with is valid.
  if ( len < 21 )
     { 
       // Minimum size of an IPMI msg.
       IpmiLog( "Dropped message because too small(1)\n" );
       return eResponseTypeError;
     }

  // Validate the RMCP portion of the message.
  if (    data[0] != 6
       || data[2] != 0xff )
     {
       IpmiLog( "Dropped message not valid IPMI/RMCP !\n");
       return eResponseTypeError;
     }

  if ( data[3] == 0x06 )
     {
       unsigned int asf_iana = IpmiGetUint32( data+4 );

       if ( asf_iana != dAsfIana || data[8] != 0x40 )
          {
            IpmiLog( "Dropped message not valid RMCP pong message %04x, %04x, %02x !\n",
                     asf_iana, dAsfIana, data[8] );
            return eResponseTypeError;
          }

       m_ping_count--;

       IpmiLog( "reading RMCP pong.\n" );

       return eResponseTypePong;
     }

  if ( data[3] != 0x07 )
     {
       IpmiLog( "Dropped message not valid IPMI/RMCP\n");
       return eResponseTypeError;
     }
  
  if ( data[4] == 0 )
     {
       // No authentication.
       if ( len < data[13] + 14 ) 
	  {
	    // Not enough data was supplied, reject the message.
	    IpmiLog( "Dropped message because too small(2)\n");
	    return eResponseTypeError;
	  }

       data_len = data[13];
     }
  else
     {
       if ( len < 37 )
	  { 
	    // Minimum size of an authenticated IPMI msg.
	    IpmiLog( "Dropped message because too small(3)\n");
	    return eResponseTypeError;
	  }

       // authcode in message, add 16 to the above checks.
       if ( len < data[29] + 30 )
	  {
	    // Not enough data was supplied, reject the message.
	    IpmiLog( "Dropped message because too small(4)\n");
	    return eResponseTypeError;
	  }

       data_len = data[29];
     }

  // Drop if the authtypes are incompatible.
  if ( m_working_auth != data[4] )
     {
       IpmiLog( "Dropped message not valid authtype\n");
       return eResponseTypeError;
     }

  // Drop if sessions ID's don't match.
  sess_id = IpmiGetUint32( data+9 );
  if ( sess_id != m_session_id )
     {
       IpmiLog( "Dropped message not valid session id 0x%x != 0x%x\n",
		sess_id, m_session_id );
       return eResponseTypeError;
     }

  seq = IpmiGetUint32( data+5 );

  if ( data[4] != 0 )
     {
       // Validate the message's authcode.  Do this before checking
       // the session seq num so we know the data is valid.
       int rv = AuthCheck( data+9, data+5, data+30, data[29], data+13 );

       if ( rv )
          {
            IpmiLog( "Dropped message auth fail !\n");
            return eResponseTypeError;
          }

       tmsg = data + 30;
     }
  else
       tmsg = data + 14;

  // Check the sequence number.
  if ( seq - m_inbound_seq_num <= 8 )
     {
       // It's after the current sequence number, but within 8.  We
       // move the sequence number forward.
       m_recv_msg_map <<= seq - m_inbound_seq_num;
       m_recv_msg_map |= 1;
       m_inbound_seq_num = seq;
     }
  else if ( m_inbound_seq_num - seq <= 8 )
     {
       // It's before the current sequence number, but within 8.
       uint8_t bit = 1 << (m_inbound_seq_num - seq);
       if ( m_recv_msg_map & bit )
	  {
	    IpmiLog( "Dropped message duplicate\n" );
	    return eResponseTypeError;
	  }

       m_recv_msg_map |= bit;
     }
  else
     {
       // It's outside the current sequence number range, discard
       // the packet.
       IpmiLog( "Dropped message out of seq range\n");
       return eResponseTypeError;
     }

  // Now we have an authentic in-sequence message.

  // We don't check the checksums, because the network layer should
  // validate all this for us.

  if ( tmsg[5] == eIpmiCmdReadEventMsgBuffer
       && (tmsg[1] >> 2) == eIpmiNetfnAppRsp )
     {
       // event
       if ( tmsg[6] != 0 )
	  {
	    // An error getting the events, just ignore it.
	    IpmiLog( "Dropped message err getting event\n");
	    return eResponseTypeError;
	  }

       addr.m_type       = eIpmiAddrTypeIpmb;
       addr.m_slave_addr = tmsg[3];
       addr.m_lun        = tmsg[4] & 0x3;
       addr.m_channel    = 0;

       msg.m_netfn    = (tIpmiNetfn)(tmsg[1] >> 2);
       msg.m_cmd      = (tIpmiCmd)tmsg[5];
       msg.m_data_len = data_len - 6 - 2; /* Remove completion code and checksum */
       memcpy( msg.m_data, tmsg + 6 + 1, msg.m_data_len );

       return eResponseTypeEvent;
     }

  seq = tmsg[4] >> 2;

  if ( m_outstanding[seq] == 0 )
     {
       IpmiLog( "Dropped message seq not in use\n" );
       return eResponseTypeError;
     }

  if ( tmsg[5] == eIpmiCmdSendMsg
       && (tmsg[1] >> 2) == eIpmiNetfnAppRsp )
     {
       // It's a response to a sent message.

       // FIXME - this entire thing is a cheap hack.
       if ( tmsg[6] != 0 )
	  {
	    // Got an error from the send message.  We don't have any
	    // IPMB information to work with, so just extract it from
            // the message.
            addr = m_outstanding[seq]->m_send_addr;

            // Just in case it's a broadcast.
	    addr.m_type = eIpmiAddrTypeIpmb;

            msg.m_netfn    = (tIpmiNetfn)(m_outstanding[seq]->m_msg.m_netfn | 1);
            msg.m_cmd      = m_outstanding[seq]->m_msg.m_cmd;
            msg.m_data[0]  = tmsg[6];
            msg.m_data_len = 1;
	  }
       else
	  {
            if ( data_len < 15 )
                 return eResponseTypeError;

            if ( tmsg[10] == m_slave_addr )
                 addr.Si();
            else
               {
                 addr.m_type       = eIpmiAddrTypeIpmb;
                 addr.m_channel    = m_outstanding[seq]->m_send_addr.m_channel;
                 addr.m_slave_addr = tmsg[10];
               }

            addr.m_lun = tmsg[11] & 0x3;

	    msg.m_netfn    = (tIpmiNetfn)(tmsg[8] >> 2);
	    msg.m_cmd      = (tIpmiCmd)tmsg[12];
	    msg.m_data_len = data_len - 15;
	    memcpy( msg.m_data, tmsg + 13, msg.m_data_len );
	  }
     }
  else if ( m_outstanding[seq]->m_send_addr.m_type == eIpmiAddrTypeSystemInterface
            && tmsg[3] == m_slave_addr )
     {
       // In some cases, a message from the IPMB looks like it came
       // from the BMC itself, IMHO a misinterpretation of the
       // errata.  IPMIv1_5_rev1_1_0926 markup, section 6.12.4,
       // didn't clear things up at all.  Some manufacturers have
       // interpreted it this way, but IMHO it is incorrect.
       addr = m_outstanding[seq]->m_send_addr;

       msg.m_netfn    = (tIpmiNetfn)(tmsg[1] >> 2);
       msg.m_cmd      = (tIpmiCmd)tmsg[5];
       msg.m_data_len = data_len - 7;
       memcpy( msg.m_data, tmsg+6, msg.m_data_len );
     }
  else
     {
       // It's not encapsulated in a send message response.
       if ( tmsg[3] == m_slave_addr )
          {
            // It's directly from the BMC, so it's a system interface
            // message.
            addr.Si();
            addr.m_lun = tmsg[1] & 3;
          }
       else
          {
	    // A message from the IPMB.

	    addr.m_type       = eIpmiAddrTypeIpmb;

	    // This is a hack, but the channel does not come back in the
            // message.  So we use the channel from the original
            // instead.
	    addr.m_channel    = m_outstanding[seq]->m_send_addr.m_channel;
	    addr.m_slave_addr = tmsg[3];
	    addr.m_lun        = tmsg[4] & 0x3;
	}

       msg.m_netfn    = (tIpmiNetfn)(tmsg[1] >> 2);
       msg.m_cmd      = (tIpmiCmd)tmsg[5];
       msg.m_data_len = data_len - 6 - 1; // Remove the checksum
       memcpy( msg.m_data, tmsg+6, msg.m_data_len );
     }

  if ( (tIpmiNetfn)(m_outstanding[seq]->m_msg.m_netfn | 1) != msg.m_netfn
       || m_outstanding[seq]->m_msg.m_cmd != msg.m_cmd )
     {
       IpmiLog( "Message mismatch seq %d:\n", seq );
       IpmiLogData( "read " );       
       IpmiLogDataMsg( addr, msg );
       IpmiLogData( "\n" );

       IpmiLogData( "expt " );
       IpmiLogDataMsg( m_outstanding[seq]->m_send_addr,
                       m_outstanding[seq]->m_msg );
       IpmiLogData( "\n" );

       IpmiLogHex( data, len );

       IpmiLog( "len %d, m_num_outstanding %d, m_queue %s\n", 
                len, m_num_outstanding,
                m_queue ? "full" : "empty" );

       assert( 0 );
       return eResponseTypeError;
     }

  if ( m_outstanding[seq]->m_send_addr != m_outstanding[seq]->m_addr )
       addr = m_outstanding[seq]->m_addr;

  return eResponseTypeMessage;
}


void
cIpmiConLan::IfReadResponse()
{
  int       seq;
  cIpmiAddr addr;
  cIpmiMsg  msg;

  tResponseType rt = ReadResponse( seq, addr, msg );

  switch( rt )
     {
       case eResponseTypeError:
       case eResponseTypePong:
       case eResponseTypeTimeout:
            break;

       case eResponseTypeMessage:
            HandleResponse( seq, addr, msg );
            break;

       case eResponseTypeEvent:
            HandleEvent( addr, msg );
            break;
     }
}
