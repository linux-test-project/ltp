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


#include <string.h>
#include <assert.h>

#include "ipmi_msg.h"


cIpmiMsg::cIpmiMsg()
  : m_netfn( eIpmiNetfnChassis ), m_cmd( eIpmiCmdGetChassisCapabilities ),
    m_data_len( 0 )
{
}


cIpmiMsg::cIpmiMsg( tIpmiNetfn netfn, tIpmiCmd cmd,
                    unsigned short data_len, unsigned char *data )
  : m_netfn( netfn ), m_cmd( cmd ),
    m_data_len( data_len )
{
  if ( data )
     {
       assert( data_len < dIpmiMaxMsgLength );
       memcpy( m_data, data, data_len );
     }
}


bool
cIpmiMsg::Equal( const cIpmiMsg &msg2 ) const
{
  if (    m_netfn    != msg2.m_netfn
       || m_cmd      != msg2.m_cmd
       || m_data_len != msg2.m_data_len )
       return false;

  if ( m_data_len )
       if ( memcmp( m_data, msg2.m_data, m_data_len ) )
	    return false;

  return true;
}
