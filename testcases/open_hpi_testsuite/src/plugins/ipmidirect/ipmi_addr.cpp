/*
 *
 * Copyright (c) 2003 by FORCE Computers.
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

#include "ipmi_addr.h"
#include "ipmi_log.h"

#include <errno.h>


int
cIpmiAddr::Cmp( const cIpmiAddr &addr ) const
{
  int v = (int)addr.m_type - (int)m_type;

  if ( v )
       return v;

  v = (int)addr.m_channel - (int)m_channel;

  if ( v )
       return v;

  v = (int)addr.m_lun - (int)m_lun;

  if ( v )
       return v;

  v = (int)addr.m_slave_addr - (int)m_slave_addr;

  return v;
}


void
cIpmiAddr::Log() const
{
  switch( m_type )
     {
       case eIpmiAddrTypeSystemInterface:
            IpmiLog( "si <%02x %02x>", m_channel, m_lun );
            break;
       case eIpmiAddrTypeIpmb:
            IpmiLog( "ipmb <%02x %02x %02x>", m_channel, m_lun, m_slave_addr );
            break;

       case eIpmiAddrTypeIpmbBroadcast:
            IpmiLog( "bc <%02x %02x %02x>", m_channel, m_lun, m_slave_addr );
            break;
     }
}
