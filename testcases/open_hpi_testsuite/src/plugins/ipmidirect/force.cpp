/*
 * Force specific code
 *
 * Copyright (c) 2004 by FORCE Computers.
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

#include "force.h"
#include "ipmi_utils.h"
#include <errno.h>


int
ForceShMcSetup( cIpmiDomain *domain, const cIpmiMsg &devid,
                unsigned int /*mid*/, unsigned int /*pid*/ )
{
  // because we are talking to a ShMC and not the ShM,
  // we need to do some setup:
  // 1.) set the MC in ShMc mode
  // 2.) clear repository SDR

  // we want to go into BMC mode
  cIpmiMsg msg( (tIpmiNetfn)0x30, (tIpmiCmd)0x03 );
  msg.m_data[0] = 0;
  msg.m_data_len = 1;

  cIpmiMsg rsp;

  int rv = domain->SiMc()->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send set BMC mode: %d\n", rv );
       return rv;
     }

  if ( rsp.m_data_len <= 0 || rsp.m_data[0] != eIpmiCcOk )
     {
       IpmiLog( "cannot go into BMC mode: %02x\n", rsp.m_data[0] );
       return EINVAL;
     }

  // check if there is a repository SDR
  if ( devid.m_data[6] & 2 )
     {
       // clear repository SDR

       // get a reservation
       msg.m_netfn = eIpmiNetfnStorage;
       msg.m_cmd   = eIpmiCmdReserveSdrRepository;
       msg.m_data_len = 0;

       rv = domain->SiMc()->SendCommand( msg, rsp );

       if ( rv )
          {
            IpmiLog( "cannot send reserve reposotory SDR: %d\n", rv );
            return rv;
          }

       if ( rsp.m_data_len != 3 || rsp.m_data[0] != eIpmiCcOk )
          {
            IpmiLog( "cannot reserve repository SDR: %02x\n",
                     rsp.m_data[0] );

            return EINVAL;
          }

       unsigned short reservation = IpmiGetUint16( rsp.m_data + 1 );

       // create repository SDR and wait until erase completed
       bool first = true;

       msg.m_netfn = eIpmiNetfnStorage;
       msg.m_cmd   = eIpmiCmdClearSdrRepository;
       IpmiSetUint16( msg.m_data, reservation );
       msg.m_data[2] = 'C';
       msg.m_data[3] = 'L';
       msg.m_data[4] = 'R';
       msg.m_data_len = 6;

       do
          {
            msg.m_data[5] = first ? 0xaa : 0; // initiate erase/ erase status
            first = false;

            rv = domain->SiMc()->SendCommand( msg, rsp );

            if ( rv )
               {
                 IpmiLog( "cannot send clear SDR reposotory: %d\n", rv );
                 return rv;
               }

            if ( rsp.m_data_len != 2 || rsp.m_data[0] != eIpmiCcOk )
               {
                 IpmiLog( "cannot reserve repository SDR: %02x\n",
                          rsp.m_data[0] );

                 return EINVAL;
               }
          }
       // wait until erase is complete
       while( (rsp.m_data[1] & 0x7) != 0x1 );
     }

  return 0;
}
