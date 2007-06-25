/*
 * Stress test for the connection layer
 *
 * Copyright (c) 2004 by FORCE Computers
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
 *
 * This test requires:
 * - IPMI hardware with RMCP
 * - RMCP configuration like IP address, port, user, password
 * 
 * So it is not enabled by default.
 */

#include <stdio.h>
#include "ipmi_con_lan.h"
#include <netdb.h>
#include <string.h>


#define dNumThreads 30
#define dNumCmdsPerThread 100

static const char    *host   = "192.168.110.187";
static int            port   = 623;
static const char    *user   = "kanne";
static const char    *passwd = "kanne";
static tIpmiAuthType  auth   = eIpmiAuthTypeNone;
static tIpmiPrivilege priv   = eIpmiPrivilegeAdmin;
static cIpmiConLan   *con  = 0;

static int num_threads = 0;
static int num_cmds = 0;

cThreadLockRw lock;


class cThreadTest : public cThread
{
  int m_id;
  cIpmiAddr m_addr;

public:
  cThreadTest( int id, unsigned char slave_addr )
    : m_id( id ), m_addr( eIpmiAddrTypeIpmb, 0, 0, slave_addr )
  {
  }

  int SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp )
  {
    num_cmds++;

    return con->ExecuteCmd( m_addr, msg, rsp );
  }

  void ClearSel()
  {
    lock.WriteLock();

    // get a reservation
    cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdReserveSel );
    msg.m_data_len = 0;

    cIpmiMsg rsp;

    int rv = SendCommand( msg, rsp );

    unsigned short reservation = IpmiGetUint16( rsp.m_data + 1 );

    msg.m_netfn = eIpmiNetfnStorage;
    msg.m_cmd   = eIpmiCmdClearSel;
    IpmiSetUint16( msg.m_data, reservation );
    msg.m_data[2] = 'C';
    msg.m_data[3] = 'L';
    msg.m_data[4] = 'R';
    msg.m_data_len = 6;

    bool first = true;
    int count = 100;

    do
       {
         msg.m_data[5] = first ? 0xaa : 0; // initiate erase/ erase status
         first = false;

         rsp.m_data[0] = 0xff;

         rv = SendCommand( msg, rsp );
       }
    while( (rsp.m_data[1] & 0x7) != 0x1 && count-- > 0 );

    lock.WriteUnlock();
  }

  void Cmd()
  {
    cIpmiMsg  msg( eIpmiNetfnStorage, eIpmiCmdAddSelEntry );
    msg.m_data[0] = 0;
    msg.m_data[1] = 0;
    msg.m_data[2] = 0xc0;
    msg.m_data[3] = 0;
    msg.m_data[4] = 0;
    msg.m_data[5] = 0;
    msg.m_data[6] = 0;
    msg.m_data[7] = 1;
    msg.m_data[8] = 2;
    msg.m_data[9] = 3;
    msg.m_data[10] = 0;
    msg.m_data[11] = 0;
    msg.m_data[12] = 0;
    msg.m_data[13] = 0;
    msg.m_data[14] = 0;
    msg.m_data[15] = 0;
    msg.m_data_len = 16;

    cIpmiMsg rsp;

    lock.ReadLock();

    for( int i = 0; i < 10; i++ )
       {
         int rv = SendCommand( msg, rsp );

         if ( rv || rsp.m_data[0] )
            {
              lock.ReadUnlock();

              ClearSel();

              lock.ReadLock(); 
            }
       }

    lock.ReadUnlock();
  }

  virtual void *Run()
  {
    num_threads++;

    for( int i = 0; i < dNumCmdsPerThread; i++ )
         Cmd();

    num_threads--;

    delete this;

    return 0;
  }
};


class cIpmiConLanTest : public cIpmiConLan
{
public:
  cIpmiConLanTest( unsigned int timeout, 
                     struct in_addr addr, int por,
                     tIpmiAuthType aut, tIpmiPrivilege pri, 
                     char *u, char *p )
    : cIpmiConLan( timeout, 0, addr, por, aut, pri, 
                   u, p )
  {
  }

  virtual ~cIpmiConLanTest()
  {
  }

  virtual void HandleAsyncEvent( const cIpmiAddr & /*addr*/, const cIpmiMsg & /*msg*/ )
  {
  }
};


int
main( int /*argc*/, char * /*argv*/[] )
{
  stdlog.Open( dIpmiLogFile|dIpmiLogStdOut );

  struct hostent *ent = gethostbyname( host );

  if ( !ent )
     {
       stdlog << "unable to resolve IPMI LAN address: " << host << " !\n";
       return 1;
     }

  struct in_addr lan_addr;
  memcpy( &lan_addr, ent->h_addr_list[0], ent->h_length );

  con = new cIpmiConLanTest( 5000,
                             lan_addr, port,
                             auth, priv,
                             const_cast<char *>(user),
                             const_cast<char *>(passwd) );

  con->SetMaxOutstanding( 4 );

  int rv = con->Open() ? 0 : 1;

  if ( !rv )
     {
       time_t t0 = time( 0 );
       
       for( int i = 0; i < dNumThreads; i++ )
          {
            cThreadTest *t = new cThreadTest( i, 0x20 );
            t->Start();
          }

       sleep( 1 );

       while( num_threads )
          {
            sleep( 1 );
            stdlog << "### " << num_cmds << "\n";
          }

       time_t t = time( 0 ) - t0;

       stdlog << "time: " << (int)t << "s\n"; 
     }

  delete con;

  stdlog << num_cmds << " GetDeviceId\n";

  stdlog.Close();

  return rv;
}
