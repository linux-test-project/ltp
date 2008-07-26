/*
 *
 * Copyright (c) 2004 by FORCE Computers
 * Copyright (c) 2007 by ESO Technologies.
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
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if defined(__sun) && defined(__SVR4)
#include <unistd.h>
#include <stropts.h>
#include <sys/ioccom.h>
#endif

#include "ipmi_con_smi.h"


// This is an overlay for all the address types, so it's easy to
// determine the actual address type.  This is kind of like addresses
// work for sockets.
#define IPMI_MAX_ADDR_SIZE 32
struct ipmi_addr
{
  // Try to take these from the "Channel Medium Type" table
  // in section 6.5 of the IPMI 1.5 manual.
  int   addr_type;
  short channel;
  char  data[IPMI_MAX_ADDR_SIZE];
};


// When the address is not used, the type will be set to this value.
// The channel is the BMC's channel number for the channel (usually
// 0), or IPMC_BMC_CHANNEL if communicating directly with the BMC.
#define IPMI_SYSTEM_INTERFACE_ADDR_TYPE	0x0c
struct ipmi_system_interface_addr
{
  int           addr_type;
  short         channel;
  unsigned char lun;
};


// An IPMB Address.
#define IPMI_IPMB_ADDR_TYPE		0x01
// Used for broadcast get device id as described in section 17.9 of the
// IPMI 1.5 manual.
#define IPMI_IPMB_BROADCAST_ADDR_TYPE	0x41
struct ipmi_ipmb_addr
{
  int           addr_type;
  short         channel;
  unsigned char slave_addr;
  unsigned char lun;
};


// Channel for talking directly with the BMC.  When using this
// channel, This is for the system interface address type only.  FIXME
// - is this right, or should we use -1?
#define IPMI_BMC_CHANNEL  0xf
#define IPMI_NUM_CHANNELS 0x10


// A raw IPMI message without any addressing.  This covers both
// commands and responses.  The completion code is always the first
// byte of data in the response (as the spec shows the messages laid
// out).
struct ipmi_msg
{
  unsigned char  netfn;
  unsigned char  cmd;
  unsigned short data_len;
  unsigned char  *data;
};


// Messages sent to the interface are this format.
struct ipmi_req
{
  unsigned char *addr; // Address to send the message to.
  unsigned int  addr_len;

  long    msgid; // The sequence number for the message.  This
                 // exact value will be reported back in the
                 // response to this request if it is a command.
                 // If it is a response, this will be used as
                 // the sequence value for the response.

  struct ipmi_msg msg;
};


// Receive types for messages coming from the receive interface.  This
// is used for the receive in-kernel interface and in the receive
// IOCTL.
#define IPMI_RESPONSE_RECV_TYPE		1 // A response to a command
#define IPMI_ASYNC_EVENT_RECV_TYPE	2 // Something from the event queue
#define IPMI_CMD_RECV_TYPE		3 // A command from somewhere else
// Note that async events and received commands do not have a completion
// code as the first byte of the incoming data, unlike a response.


// Messages received from the interface are this format.
struct ipmi_recv
{
  int     recv_type; // Is this a command, response or an
                     // asyncronous event.

  unsigned char *addr;    // Address the message was from is put
                          // here.  The caller must supply the
                          // memory.
  unsigned int  addr_len; // The size of the address buffer.
                          // The caller supplies the full buffer
                          // length, this value is updated to
                          // the actual message length when the
                          // message is received.

  long    msgid; // The sequence number specified in the request
                 // if this is a response.  If this is a command,
                 // this will be the sequence number from the
                 // command.

  struct ipmi_msg msg; // The data field must point to a buffer.
                       // The data_size field must be set to the
                       // size of the message buffer.  The
                       // caller supplies the full buffer
                       // length, this value is updated to the
                       // actual message length when the message
                       // is received.
};


// Get/set the default timing values for an interface.  You shouldn't
// generally mess with these.

struct ipmi_timing_parms
{
	int          retries;
	unsigned int retry_time_ms;
};


// The magic IOCTL value for this interface.
#define IPMI_IOC_MAGIC 'i'


// Send a message to the interfaces.  error values are:
//   - EFAULT - an address supplied was invalid.
//   - EINVAL - The address supplied was not valid, or the command
//              was not allowed.
//   - EMSGSIZE - The message to was too large.
//   - ENOMEM - Buffers could not be allocated for the command.
#define IPMICTL_SEND_COMMAND		_IOR(IPMI_IOC_MAGIC, 13,	\
					     struct ipmi_req)


// Like RECEIVE_MSG, but if the message won't fit in the buffer, it
// will truncate the contents instead of leaving the data in the
// buffer.
#define IPMICTL_RECEIVE_MSG_TRUNC	_IOWR(IPMI_IOC_MAGIC, 11,	\
					      struct ipmi_recv)


// Set whether this interface receives events.  Note that the first
// user registered for events will get all pending events for the
// interface.  error values:
//  - EFAULT - an address supplied was invalid.
#define IPMICTL_SET_GETS_EVENTS_CMD	_IOR(IPMI_IOC_MAGIC, 16, int)


#define IPMICTL_SET_TIMING_PARMS_CMD	_IOR(IPMI_IOC_MAGIC, 22, \
					     struct ipmi_timing_parms)


cIpmiConSmi::cIpmiConSmi( unsigned int timeout, int log_level, int if_num )
  : cIpmiCon( timeout, log_level ), 
    m_if_num( if_num )
{
}


cIpmiConSmi::~cIpmiConSmi()
{
  if ( IsOpen() )
       Close();
}


int
cIpmiConSmi::OpenSmiFd( int if_num )
{
  int fd;
  char devname[30];

  snprintf( devname, sizeof(devname), "/dev/ipmidev/%d", if_num );

  fd = open( devname, O_RDWR );

  if ( fd >= 0 )
       return fd;

  snprintf( devname, sizeof(devname), "/dev/ipmi/%d", if_num );
  fd = open( devname, O_RDWR );

  if ( fd >= 0 )
       return fd;

  snprintf( devname, sizeof(devname), "/dev/ipmi%d", if_num );
  fd = open( devname, O_RDWR );

  return fd;
}


int
cIpmiConSmi::IfGetMaxSeq()
{
  return dMaxSeq;
}


int
cIpmiConSmi::IfOpen()
{
  int fd = OpenSmiFd( m_if_num );

  if ( fd < 0 )
       return fd;

  struct ipmi_timing_parms parms;
  int                      rv;

  parms.retries       = 0;
  parms.retry_time_ms = 1000;

  rv = ioctl( fd, IPMICTL_SET_TIMING_PARMS_CMD, &parms );

  if ( rv == -1 )
       stdlog << "Warning: Could not set timing parms !\n";

  // we want async events
  int val = 1;
  rv = ioctl( fd, IPMICTL_SET_GETS_EVENTS_CMD, &val );

  if ( rv == -1 )
       stdlog << "Warning: Could not set gets events !\n";

  return fd;
}


void
cIpmiConSmi::IfClose()
{
}


SaErrorT
cIpmiConSmi::IfSendCmd( cIpmiRequest *r )
{
  cIpmiAddr send_addr = r->m_send_addr;

  ipmi_addr addr;
  unsigned int addr_len = 0;

  addr.addr_type = (int)send_addr.m_type;

  // convert addr
  switch( send_addr.m_type )
     {
       case eIpmiAddrTypeSystemInterface:
            {
              ipmi_system_interface_addr *si = (ipmi_system_interface_addr *)&addr;
              si->channel = send_addr.m_channel;
              si->lun     = send_addr.m_lun;
              addr_len = sizeof( ipmi_system_interface_addr );
            }
            break;

       case eIpmiAddrTypeIpmb:
       case eIpmiAddrTypeIpmbBroadcast:
            {
              ipmi_ipmb_addr *ipmb = (ipmi_ipmb_addr *)&addr;
              ipmb->channel    = send_addr.m_channel;
              ipmb->slave_addr = send_addr.m_slave_addr;
              ipmb->lun        = send_addr.m_lun;
              addr_len = sizeof( ipmi_ipmb_addr );
            }
            break;

       default:
            return SA_ERR_HPI_INVALID_PARAMS;
     }

  struct ipmi_req req;
  req.addr = (unsigned char *)&addr;
  req.addr_len = addr_len;

  req.msg.netfn    = r->m_msg.m_netfn;
  req.msg.cmd      = r->m_msg.m_cmd;  
  req.msg.data_len = r->m_msg.m_data_len;
  req.msg.data     = r->m_msg.m_data;

  req.msgid = r->m_seq;

  int rv = ioctl( m_fd, IPMICTL_SEND_COMMAND, &req );

  if ( rv )
       return SA_ERR_HPI_INVALID_REQUEST;

  return SA_OK;
}


void
cIpmiConSmi::IfReadResponse()
{
  unsigned char data[dIpmiMaxMsgLength];
  ipmi_addr     addr;
  ipmi_recv     recv;

  recv.msg.data     = data;
  recv.msg.data_len = dIpmiMaxMsgLength;
  recv.addr         = (unsigned char *)&addr;
  recv.addr_len     = sizeof( ipmi_addr );

  int rv = ioctl( m_fd, IPMICTL_RECEIVE_MSG_TRUNC, &recv );

  if ( rv == -1 )
     {
       if ( errno == EMSGSIZE )
            // The message was truncated, handle it as such.
            data[0] = eIpmiCcRequestedDataLengthExceeded;
       else
            return;
     }

  // convert addr
  cIpmiAddr rsp_addr;

  rsp_addr.m_type = (tIpmiAddrType)addr.addr_type;

  switch( rsp_addr.m_type )
     {
       case eIpmiAddrTypeSystemInterface:
            {
              ipmi_system_interface_addr *si = (ipmi_system_interface_addr *)&addr;

              rsp_addr.m_channel = si->channel;
              rsp_addr.m_lun     = si->lun;
            }
            break;

       case eIpmiAddrTypeIpmb:
       case eIpmiAddrTypeIpmbBroadcast:
            {
              ipmi_ipmb_addr *ipmb = (ipmi_ipmb_addr *)&addr;

              rsp_addr.m_channel    = ipmb->channel;
              rsp_addr.m_slave_addr = ipmb->slave_addr;
              rsp_addr.m_lun        = ipmb->lun;
            }
            break;

       default:
            return;
     }

  // convert msg
  cIpmiMsg rsp;
  rsp.m_netfn    = (tIpmiNetfn)recv.msg.netfn;
  rsp.m_cmd      = (tIpmiCmd)recv.msg.cmd;
  rsp.m_data_len = recv.msg.data_len;

  if ( rsp.m_data_len )
       memcpy( rsp.m_data, recv.msg.data, rsp.m_data_len );

  int seq = (int)recv.msgid;

  switch( recv.recv_type )
     {
       case IPMI_RESPONSE_RECV_TYPE:
	    HandleResponse( seq, rsp_addr, rsp );
	    break;

       case IPMI_ASYNC_EVENT_RECV_TYPE:
	    HandleEvent( rsp_addr, rsp );
	    break;

       case IPMI_CMD_RECV_TYPE:
            // incomming command
            stdlog << "SMI: incomming ipmi command " 
                   << IpmiCmdToString( rsp.m_netfn, rsp.m_cmd ) << ".\n";

	    break;

       default:
	    break;
    }
}
