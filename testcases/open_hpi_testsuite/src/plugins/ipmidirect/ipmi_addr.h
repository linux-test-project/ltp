/*
 * ipmi_addr.h
 *
 * Interface for IPMI file connection
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

#ifndef dIpmiAddr_h
#define dIpmiAddr_h


#include <assert.h>

// This is an overlay for all the address types, so it's easy to
// determine the actual address type.  This is kind of like addresses
// work for sockets.
#define dIpmiMaxAddrSize 32

// When the address is not used, the type will be set to this value.
// The channel is the BMC's channel number for the channel (usually
// 0), or dIpmcBmcChannel if communicating directly with the BMC.

// Channel for talking directly with the BMC.  When using this
// channel, This is for the system interface address type only.
// FIXME - is this right, or should we use -1?
#define dIpmiBmcChannel  0xf
#define dIpmiNumChannels 0x10


enum tIpmiAddrType
{
  eIpmiAddrTypeIpmb            = 1,
  eIpmiAddrTypeSystemInterface = 0xc,
  eIpmiAddrTypeIpmbBroadcast   = 0x41,
};


#define dIpmiMaxChannel 16


class cIpmiAddr
{
public:
  // Try to take these from the "Channel Medium Type" table
  // in section 6.5 of the IPMI 1.5 manual.
  tIpmiAddrType  m_type;
  unsigned short m_channel;
  unsigned char  m_lun;
  unsigned char  m_slave_addr;

  cIpmiAddr( tIpmiAddrType type = eIpmiAddrTypeIpmb,
             short channel = 0, unsigned char lun = 0,
             unsigned char slave_addr = 0x20 )
    : m_type( type ), m_channel( channel ),
      m_lun( lun ), m_slave_addr( slave_addr )
  {
    if ( type == eIpmiAddrTypeSystemInterface )
         Si();
  }

  int Cmp( const cIpmiAddr &addr ) const;
  bool operator==( const cIpmiAddr &addr ) const { return !Cmp( addr ); }
  bool operator!=( const cIpmiAddr &addr ) const { return Cmp( addr ); }  

  void Log() const;
  short Channel() const  { return m_channel; }
  bool IsType( tIpmiAddrType type ) const { return m_type == type; }
  unsigned char Lun() const  { return m_lun; }

  unsigned char SlaveAddr() const
  { 
    assert( m_type == eIpmiAddrTypeIpmb ); 
    return m_slave_addr;
  }

  void Si()
  {
    m_type    = eIpmiAddrTypeSystemInterface;
    m_channel = dIpmiBmcChannel;
    m_lun     = 0;
  }
};


#endif
