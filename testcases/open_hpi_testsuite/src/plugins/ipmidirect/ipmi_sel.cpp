/*
 * ipmi_sel.cpp
 *
 * Copyright (c) 2003 by FORCE Computers
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ipmi_cmd.h"
#include "ipmi_sel.h"
#include "ipmi_mc.h"
#include "ipmi_log.h"
#include "ipmi_utils.h"
#include "ipmi_event.h"
#include "ipmi_domain.h"


cIpmiSel::cIpmiSel( cIpmiMc *mc, unsigned int lun )
  : m_mc( mc ), m_lun( lun ),
    m_major_version( 0 ), m_minor_version( 0 ),
    m_entries( 0 ),
    m_last_addition_timestamp( 0 ),
    m_last_erase_timestamp( 0 ),
    m_overflow( false ),
    m_supports_delete_sel( false ), m_supports_partial_add_sel( false ),
    m_supports_reserve_sel( false ), m_supports_get_sel_allocation( false ),
    m_entity( 0 ), m_fetched( false ), m_reservation( 0 ),
    m_sels_changed( false ),
    m_sel( 0 ), m_sel_num( 0 ),
    m_async_events( 0 ), m_async_events_num( 0 )
{
  assert( lun < 4 );
}


cIpmiSel::~cIpmiSel()
{
  m_sel_lock.Lock();

  if ( m_sel )
       ClearList( m_sel );

  if ( m_async_events )
       ClearList( m_async_events );

  m_sel_lock.Unlock();
}


int
cIpmiSel::ClearSel()
{
  cThreadLockAuto al( m_sel_lock );

  cIpmiMsg cmd_msg;
  cIpmiMsg rsp;
  int rv;

  // do a reservation only when needed
  if (    m_supports_reserve_sel 
       && m_reservation == 0 )
     {
       rv = Reserve();

       if ( rv )
            return rv;
     }

  IpmiLog( "clear SEL.\n" );

  cmd_msg.m_netfn    = eIpmiNetfnStorage;
  cmd_msg.m_cmd      = eIpmiCmdClearSel;
  cmd_msg.m_data_len = 6;
  IpmiSetUint16( cmd_msg.m_data, m_reservation );
  cmd_msg.m_data[2]  = 'C';
  cmd_msg.m_data[3]  = 'L';
  cmd_msg.m_data[4]  = 'R';
  cmd_msg.m_data[5]  = 0xaa;

  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
       return rv;

  if ( rsp.m_data[0] == 0 )
     {
       m_sel = ClearList( m_sel );
       m_sel_num = 0;
     }

  return 0;
}


int
cIpmiSel::GetInfo()
{
  cIpmiMsg cmd_msg( eIpmiNetfnStorage, eIpmiCmdGetSelInfo );
  cIpmiMsg rsp;
  int rv;
  unsigned int add_timestamp;
  unsigned int erase_timestamp;

  // Fetch the repository info.
  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "could not send get sel info: 0x%02x !\n", rv );
       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       IpmiLog( "IpmiSelGetInfo: IPMI error from SEL info fetch: %x !\n",
                rsp.m_data[0]);

       return EINVAL;
     }

  if ( rsp.m_data_len < 15 )
     {
       IpmiLog( "handle_sel_info: SEL info too short !\n");
       return EINVAL;
     }

  unsigned int num = m_entries;

  // Pull pertinant info from the response.
  m_major_version               = rsp.m_data[1] & 0xf;
  m_major_version               = (rsp.m_data[1] >> 4) & 0xf;
  m_entries                     = IpmiGetUint16( rsp.m_data + 2 );
  m_overflow                    = (rsp.m_data[14] & 0x80) == 0x80;
  m_supports_delete_sel         = (rsp.m_data[14] & 0x08) == 0x08;
  m_supports_partial_add_sel    = (rsp.m_data[14] & 0x04) == 0x04;
  m_supports_reserve_sel        = (rsp.m_data[14] & 0x02) == 0x02;
  m_supports_get_sel_allocation = (rsp.m_data[14] & 0x01) == 0x01;

  add_timestamp   = IpmiGetUint32( rsp.m_data + 6 );
  erase_timestamp = IpmiGetUint32( rsp.m_data + 10 );

  // If the timestamps still match, no need to re-fetch the repository
  if (    m_fetched
       && m_entries == num
       && (add_timestamp   == m_last_addition_timestamp )
       && (erase_timestamp == m_last_erase_timestamp    ) )
     {
       // no need to read sel
       return -1;
     }

  m_last_addition_timestamp = add_timestamp;
  m_last_erase_timestamp    = erase_timestamp;

  m_sels_changed = true;
  m_fetched      = true;

  return 0;
}


int
cIpmiSel::Reserve()
{
  cIpmiMsg cmd_msg( eIpmiNetfnStorage, eIpmiCmdReserveSel );
  cIpmiMsg rsp;
  int rv;

  // Get a reservation.
  rv = m_mc->SendCommand( cmd_msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "cannot send reserve sel: 0x%02x !\n", rv );
       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       IpmiLog( "sel_handle_reservation: Failed getting reservation !\n" );
       return ENOSYS;
     }

  if ( rsp.m_data_len < 3 )
     {
       IpmiLog( "sel_handle_reservation: got invalid reservation length !\n" );
       return EINVAL;
     }

  m_reservation = IpmiGetUint16( rsp.m_data + 1 );

  return 0;
}


GList *
cIpmiSel::ClearList( GList *list )
{
  while( list )
     {
       cIpmiEvent *e = (cIpmiEvent *)list->data;
       list = g_list_remove( list, e );
       delete e;
     }

  return 0;
}


int
cIpmiSel::ReadSelRecord( cIpmiEvent &event, unsigned int &next_rec_id )
{
  // read record
  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdGetSelEntry );
  cIpmiMsg rsp;

  IpmiSetUint16( msg.m_data, m_reservation );
  IpmiSetUint16( msg.m_data+2, next_rec_id );
  msg.m_data[4] = 0;
  msg.m_data[5] = 0xff;
  msg.m_data_len = 6;

  int rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Could not send SEL fetch command: %x !\n", rv );
       return -1;
     }
  
  if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
     {
       IpmiLog( "SEL reservation lost !\n" );
       m_reservation = 0;

       return eIpmiCcInvalidReservation;
     }

  if ( rsp.m_data[0] != 0 )
     {
       IpmiLog( "IPMI error from SEL fetch: %x !\n",
                rsp.m_data[0] );

       return -1;
     }

  next_rec_id = IpmiGetUint16( rsp.m_data + 1 );

  event.m_mc        = m_mc;
  event.m_record_id = IpmiGetUint16( rsp.m_data + 3 );
  event.m_type      = rsp.m_data[5];
  memcpy( event.m_data, rsp.m_data + 6, 13 );

  return 0;
}


GList *
cIpmiSel::ReadSel( unsigned int &num, bool &uptodate )
{
  int rv = 0;
  GList *new_events = 0;
  num = 0;
  int fetch_retry_count = 0;
  uptodate = false;

  while( true )
     {
       if ( fetch_retry_count >= dMaxSelFetchRetries )
          {
            IpmiLog( "too many lost reservations in SEL fetch !\n");
            return 0;
          }

       fetch_retry_count++;

       // get reservation
       m_reservation = 0;

       rv = GetInfo();

       if ( rv == -1 )
          {
            // no new entries
            uptodate = true;
            return 0;
          }

       if ( rv || m_entries == 0 )
            return 0;

       if ( m_supports_reserve_sel )
          {
            rv = Reserve();

            if ( rv )
                 continue;
          }

       // read records
       unsigned int next_rec_id = 0;

       do
          {
            cIpmiEvent *event = new cIpmiEvent;

            rv = ReadSelRecord( *event, next_rec_id );

            if ( rv )
               {
                 delete event;
                 ClearList( new_events );
                 new_events = 0;
                 num = 0;

                 if ( rv == eIpmiCcInvalidReservation )
                      break;

                 return 0;
               }

            new_events = g_list_append( new_events, event );
            num++;
          }
       while( next_rec_id != 0xffff );

       if ( next_rec_id == 0xffff )
            break;
     }

  return new_events;
}


cIpmiEvent *
cIpmiSel::FindEvent( GList *list, unsigned int record_id )
{
  while( list )
     {
       cIpmiEvent *e = (cIpmiEvent *)list->data;
       
       if ( e ->m_record_id == record_id )
            return e;
       
       list = g_list_next( list );
     }

  return 0;
}


bool
cIpmiSel::CheckEvent( GList *&list, cIpmiEvent *event )
{
  cIpmiEvent *e = FindEvent( list, event->m_record_id );

  if ( !e )
       return false;

  // remove old event from list
  list = g_list_remove( list, e );

  // return true if event is old event
  bool rv = event->Cmp( *e ) == 0 ? true : false;

  delete e;

  return rv;
}


GList *
cIpmiSel::GetEvents()
{
  cThreadLockAuto al( m_sel_lock );

  IpmiLog( "reading SEL.\n" );

  // read sel
  bool uptodate = false;
  unsigned int events_num = 0;
  GList *events = ReadSel( events_num, uptodate );

  if ( uptodate )
     {
       assert( events == 0 );

       return 0;
     }

  // build a list of new events
  GList *new_events = 0;

  for( GList *item = events; item; item = g_list_next( item ) )
     {
       cIpmiEvent *current = (cIpmiEvent *)item->data;

       if ( CheckEvent( m_sel, current ) == false )
          {
            m_async_events_lock.Lock();
            bool rv = CheckEvent( m_async_events, current );
            m_async_events_lock.Unlock();

            if ( rv == false )
               {
                 // new event found
                 cIpmiEvent *e = new cIpmiEvent( *current );
                 new_events = g_list_append( new_events, e );
               }
          }
     }

  ClearList( m_sel );
  m_sel     = events;
  m_sel_num = events_num;

  return new_events;
}


int
cIpmiSel::GetSelEntry( unsigned short rid, unsigned short &prev, 
                       unsigned short &next, cIpmiEvent &event )
{
  cThreadLockAuto al( m_sel_lock );

  // empty sel
  if ( m_sel == 0 )
     {
       prev = 0;
       next = 0xffff;

       return -1;
     }

  cIpmiEvent *e = 0;
  GList *item;
  GList *i;

  if ( rid == 0 )
     {
       // first entry
       e = (cIpmiEvent *)m_sel->data;
       event = *e;

       // prev
       prev = 0;

       // next
       item = g_list_next( m_sel );
 
       if ( item )
          {
            e = (cIpmiEvent *)item->data;
            next = e->m_record_id;
          }
       else
            next = 0xffff;

       return 0;
     }

  if ( rid == 0xffff )
     {
       // last entry
       item = g_list_last( m_sel );

       e = (cIpmiEvent *)item->data;

       event = *e;
       
       // prev
       item = g_list_previous( item );
       
       if ( item )
          {
            e = (cIpmiEvent *)item->data;
            prev = e->m_record_id;
          }
       else
            prev = 0;

       next = 0xffff;

       return 0;       
     }

  item = 0;

  // find rid event
  for( i = m_sel; i; i = g_list_next( i ) )
     {
       e = (cIpmiEvent *)i->data;

       if ( e->m_record_id == rid )
          {
            item = i;
            break;
          }
     }

  if ( item == 0 )
       return -1;
  
  // event found
  e = (cIpmiEvent *)item->data;
  
  event = *e;
  
  // prev
  i = g_list_previous( item );

  if ( i )
     {
       e = (cIpmiEvent *)i->data;
       prev = e->m_record_id;
     }
  else
       prev = 0;
  
  // next
  i = g_list_next( item );

  if ( i )
     {
       e = (cIpmiEvent *)i->data;
       next = e->m_record_id;
     }
  else
       next = 0xffff;

  return 0;
}


int
cIpmiSel::DeleteSelEntry( unsigned short rid )
{
  cThreadLockAuto al( m_sel_lock );

  for( int i = 0; i < dMaxSelFetchRetries; i++ )
     {
       int rv = Reserve();

       if ( rv )
            return rv;

       cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdDeleteSelEntry );
       cIpmiMsg rsp;

       IpmiSetUint16( msg.m_data, m_reservation );
       IpmiSetUint16( msg.m_data + 2, rid );
       msg.m_data_len = 4;

       rv = m_mc->SendCommand( msg, rsp );

       if ( rv )
          {
            IpmiLog( "Could not send delete SEL entry: %x !\n", rv );
            return EINVAL;
          }

       if ( rsp.m_data[0] != eIpmiCcOk )
          {
            if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
                 // reservation lost
                 continue;

            IpmiLog( "IPMI error from delete SEL entry: %x !\n",
                     rsp.m_data[0] );

            return EINVAL;
          }

       if ( rsp.m_data_len < 3 )
          {
            IpmiLog( "IPMI error from delete SEL entry: message to short (%d) !\n", 
                     rsp.m_data_len );

            return EINVAL;
          }

       // deleted record id
       rid = IpmiGetUint16( rsp.m_data + 1 );

       // remove record from m_sel
       cIpmiEvent *e = FindEvent( m_sel, rid );

       if ( e )
          {
            m_sel = g_list_remove( m_sel, e );
            m_sel_num--;
          }

       // remove record from async event list
       m_async_events_lock.Lock();

       e = FindEvent( m_async_events, rid );

       if ( e )
          {
            m_async_events = g_list_remove( m_async_events, e );
            m_async_events_num--;
          }

       m_async_events_lock.Unlock();

       return 0;
     }

  // reservation lost too many times
  IpmiLog( "IPMI error from delete SEL entry: reservation lost too many times !\n" );

  return EINVAL;
}


int
cIpmiSel::GetSelTime( time_t &t )
{
  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdGetSelTime );
  cIpmiMsg rsp;

  int rv = m_mc->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "Could not send get SEL time: %x !\n", rv );
       return EINVAL;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       IpmiLog( "IPMI error from get SEL time: %x !\n",
                rsp.m_data[0] );

       return EINVAL;
     }

  if ( rsp.m_data_len < 5 )
     {
       IpmiLog( "IPMI error from get SEL time: message to short (%d) !\n", 
                rsp.m_data_len );

       return EINVAL;
     }

  t = IpmiGetUint32( rsp.m_data + 1 );

  return 0;
}


int
cIpmiSel::SetSelTime( time_t t )
{
  cIpmiMsg       msg( eIpmiNetfnStorage, eIpmiCmdSetSelTime );
  cIpmiMsg       rsp;

  IpmiSetUint32( msg.m_data, t );
  msg.m_data_len = 4;

  int rv = m_mc->SendCommand( msg, rsp );

  if ( rv )
     {
       IpmiLog( "Could not send set SEL time: %x !\n", rv );
       return EINVAL;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       IpmiLog( "IPMI error from set SEL time: %x !\n",
                rsp.m_data[0] );

       return EINVAL;
     }

  return 0;
}


int
cIpmiSel::AddAsyncEvent( cIpmiEvent *new_event )
{
  cIpmiEvent *e = FindEvent( m_sel, new_event->m_record_id );

  // event is already in the sel
  if ( e && new_event->Cmp( *e ) == 0 )
       return 0;

  m_async_events_lock.Lock();

  e = FindEvent( m_async_events, new_event->m_record_id );

  if ( !e )
     {
       // add new event to list
       e = new cIpmiEvent;
       *e = *new_event;
       m_async_events = g_list_append( m_async_events, e );
       m_async_events_num++;
       m_async_events_lock.Unlock();

       return 0;
     }

  m_async_events_lock.Unlock();

  if ( new_event->Cmp( *e ) == 0 )
       // event is already in the list of async events
       return 0;

  // overwrite old event
  *e = *new_event;

  return 0;
}


void
IpmiSelHandleSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs )
{
  cIpmiSel *sel = mc->Sel();
  cIpmiSdr *sdr = 0;
  unsigned char slave_addr = 0;
  unsigned char channel = 0;

  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *s = sdrs->Sdr( i );

       if ( s->m_type != eSdrTypeMcDeviceLocatorRecord )
            continue;

       slave_addr = s->m_data[5];
       channel    = s->m_data[6] & 0xf;

       cIpmiAddr addr( eIpmiAddrTypeIpmb, channel, 0, slave_addr );

       if ( addr != mc->Addr() )
            continue;

       sdr = s;
       break;
     }

  if ( sdr == 0 )
       return;

  tIpmiEntityId id      = (tIpmiEntityId)sdr->m_data[12];
  unsigned int instance = sdr->m_data[13];

  cIpmiMc     *m   = domain->FindOrCreateMcBySlaveAddr( slave_addr );
  cIpmiEntity *ent = domain->Entities().Add( m, 0, id, instance, "" );

  // check this
  assert( ent->Sel() == 0 );
  ent->Sel() = sel;

  assert( sel->Entity() == 0 );
  sel->Entity() = ent;

  ent->Domain()->IfSelAdd( ent, sel );
}
