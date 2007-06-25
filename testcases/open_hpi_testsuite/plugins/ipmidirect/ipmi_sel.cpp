/*
 * ipmi_sel.cpp
 *
 * Copyright (c) 2003 by FORCE Computers
 * Copyright (c) 2005 by ESO Technologies.
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
    m_fetched( false ), m_reservation( 0 ),
    m_sels_changed( false ),
    m_sel( 0 ), m_sel_num( 0 ),
    m_async_events( 0 ), m_async_events_num( 0 )
{
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


SaErrorT
cIpmiSel::ClearSel()
{
  cThreadLockAuto al( m_sel_lock );

  SaErrorT rv;

  // do a reservation only when needed
  if (    m_supports_reserve_sel 
       && m_reservation == 0 )
     {
       rv = Reserve();

       if ( rv != SA_OK )
            return rv;
     }

  stdlog << "clear SEL.\n";

  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdClearSel );
  msg.m_data_len = 6;
  IpmiSetUint16( msg.m_data, m_reservation );
  msg.m_data[2]  = 'C';
  msg.m_data[3]  = 'L';
  msg.m_data[4]  = 'R';
  msg.m_data[5]  = 0xaa;

  cIpmiMsg rsp;

  rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv != SA_OK )
       return rv;

  if ( rsp.m_data[0] == 0 )
     {
       m_sel = ClearList( m_sel );
       m_sel_num = 0;
     }

  return SA_OK;
}


SaErrorT
cIpmiSel::GetInfo()
{
  cIpmiMsg     msg( eIpmiNetfnStorage, eIpmiCmdGetSelInfo );
  cIpmiMsg     rsp;
  SaErrorT     rv;
  unsigned int add_timestamp;
  unsigned int erase_timestamp;

  // Fetch the repository info.
  rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "could not send get sel info: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       stdlog << "IpmiSelGetInfo: IPMI error from SEL info fetch: "
              << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_PARAMS;
     }

  if ( rsp.m_data_len < 15 )
     {
       stdlog << "handle_sel_info: SEL info too short !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  unsigned short num = m_entries;

  // Pull pertinant info from the response.
  m_major_version               = rsp.m_data[1] & 0xf;
  m_minor_version               = (rsp.m_data[1] >> 4) & 0xf;
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

  return SA_OK;
}


SaErrorT
cIpmiSel::Reserve()
{
  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdReserveSel );
  cIpmiMsg rsp;
  SaErrorT rv;

  // Get a reservation.
  rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "cannot send reserve sel: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
       stdlog << "sel_handle_reservation: Failed getting reservation !\n";
       return SA_ERR_HPI_INVALID_PARAMS;
     }

  if ( rsp.m_data_len < 3 )
     {
       stdlog << "sel_handle_reservation: got invalid reservation length !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  m_reservation = IpmiGetUint16( rsp.m_data + 1 );

  return SA_OK;
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

  SaErrorT rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Could not send SEL fetch command: " << rv << " !\n";
       return -1;
     }
  
  if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
     {
       stdlog << "SEL reservation lost !\n";
       m_reservation = 0;

       return eIpmiCcInvalidReservation;
     }

  if ( rsp.m_data[0] != 0 )
     {
       stdlog << "IPMI error from SEL fetch: " 
              << rsp.m_data[0] << " !\n";

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
  SaErrorT rv = SA_OK;
  GList *new_events = 0;
  num = 0;
  int fetch_retry_count = 0;
  uptodate = false;

  while( true )
     {
       if ( fetch_retry_count >= dMaxSelFetchRetries )
          {
            stdlog << "too many lost reservations in SEL fetch !\n";
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

       if ( rv != SA_OK || m_entries == 0 )
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

  stdlog << "reading SEL.\n";

  // read sel
  bool uptodate = false;
  unsigned int events_num = 0;
  GList *events = ReadSel( events_num, uptodate );

  if ( uptodate )
     {
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


SaErrorT
cIpmiSel::GetSelEntry( unsigned short rid, unsigned short &prev, 
                       unsigned short &next, cIpmiEvent &event )
{
  cThreadLockAuto al( m_sel_lock );

  // empty sel
  if ( m_sel == 0 )
     {
       prev = 0;
       next = 0xffff;

       return SA_ERR_HPI_NOT_PRESENT;
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

       return SA_OK;
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

       return SA_OK;  
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
       return SA_ERR_HPI_NOT_PRESENT;
  
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

  return SA_OK;
}


SaErrorT
cIpmiSel::DeleteSelEntry( SaHpiEventLogEntryIdT sid )
{
  cThreadLockAuto al( m_sel_lock );

  unsigned short rid = (unsigned short)sid;

  if ( sid == SAHPI_OLDEST_ENTRY )
       rid = 0;
  else if ( sid == SAHPI_NEWEST_ENTRY )
       rid = 0xffff;
  else
       rid = sid;

  for( int i = 0; i < dMaxSelFetchRetries; i++ )
     {
       SaErrorT rv = Reserve();

       if ( rv != SA_OK )
            return rv;

       cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdDeleteSelEntry );
       cIpmiMsg rsp;

       IpmiSetUint16( msg.m_data, m_reservation );
       IpmiSetUint16( msg.m_data + 2, rid );
       msg.m_data_len = 4;

       rv = m_mc->SendCommand( msg, rsp );

       if ( rv != SA_OK )
          {
            stdlog << "Could not send delete SEL entry: " << rv << " !\n";
            return rv;
          }

       if ( rsp.m_data[0] != eIpmiCcOk )
          {
            if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
                 // reservation lost
                 continue;

            stdlog << "IPMI error from delete SEL entry: "
                   << rsp.m_data[0] << " !\n";

            return SA_ERR_HPI_INVALID_CMD;
          }

       if ( rsp.m_data_len < 3 )
          {
            stdlog << "IPMI error from delete SEL entry: message to short "
                   << rsp.m_data_len << " !\n";

            return SA_ERR_HPI_INVALID_DATA;
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

       return SA_OK;
     }

  // reservation lost too many times
  stdlog << "IPMI error from delete SEL entry: reservation lost too many times !\n";

  return SA_ERR_HPI_INVALID_CMD;
}


SaErrorT
cIpmiSel::GetSelTime( SaHpiTimeT &ht )
{
  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdGetSelTime );
  cIpmiMsg rsp;

  SaErrorT rv = m_mc->SendCommand( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "Could not send get SEL time: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       stdlog << "IPMI error from get SEL time: " 
              << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
     }

  if ( rsp.m_data_len < 5 )
     {
       stdlog << "IPMI error from get SEL time: message to short "
              << rsp.m_data_len << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  ht = IpmiGetUint32( rsp.m_data + 1 );
  ht *= 1000000000;

  return SA_OK;
}


static time_t
CovertToAbsTimeT( SaHpiTimeT ti )
{
  if ( ti <= SAHPI_TIME_MAX_RELATIVE )
     {
       timeval tv;
       gettimeofday( &tv, 0 );
       
       tv.tv_sec  += ti / 1000000000;
       tv.tv_usec += ti % 1000000000 / 1000;

       while( tv.tv_usec > 1000000 )
          {
            tv.tv_sec++;
            tv.tv_usec -= 1000000;
          }
 
       return tv.tv_sec;
     }

  return ti / 1000000000;
}


SaErrorT
cIpmiSel::SetSelTime( SaHpiTimeT ht )
{
  if ( ht == SAHPI_TIME_UNSPECIFIED )
       return SA_ERR_HPI_ERROR;

  // convert HPI time to time_t
  time_t t = CovertToAbsTimeT( ht );

  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdSetSelTime );
  cIpmiMsg rsp;

  IpmiSetUint32( msg.m_data, t );
  msg.m_data_len = 4;

  SaErrorT rv = m_mc->SendCommand( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "Could not send set SEL time: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       stdlog << "IPMI error from set SEL time: "
              << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
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
cIpmiSel::Dump( cIpmiLog &dump, const char *name )
{
  if ( dump.IsRecursive() )
     {
       // dump events
       int i = 0;

       for( GList *list = m_sel; list; list = g_list_next( list ) )
	  {
	    cIpmiEvent *e = (cIpmiEvent *)list->data;

	    char str[80];
	    snprintf( str, sizeof(str), "Event%02x_%d", m_mc->GetAddress(), i++ );
	    e->Dump( dump, str );
	  }
     }

  dump.Begin( "Sel", name );
  dump.Entry( "Version" ) << (int)m_major_version << ", " << (int)m_minor_version << ";\n";
  dump.Entry( "Overflow" ) << m_overflow << ";\n";
  dump.Entry( "SupportsDeleteSel" ) << m_supports_delete_sel << ";\n";
  dump.Entry( "SupportsPartialAddSel" ) << m_supports_partial_add_sel << ";\n";
  dump.Entry( "SupportsReserveSel" ) << m_supports_reserve_sel << ";\n";
  dump.Entry( "SupportsGetSelAllocation" ) << m_supports_get_sel_allocation << ";\n";

  // dump events
  if ( dump.IsRecursive() && m_sel )
     {
       int i = 0;

       dump.Entry( "Event" );

       for( GList *list = m_sel; list; list = g_list_next( list ) )
          {
            if ( i != 0 )
                 dump << ", ";

            char str[80];
            snprintf( str, sizeof(str), "Event%02x_%d", m_mc->GetAddress(), i++ );
            dump << str;
          }

       dump << ";\n";
     }

  dump.End();
}

SaErrorT
cIpmiSel::GetSelInfo( SaHpiEventLogInfoT &info )
{
  cIpmiMc *mc = Mc();
  int lun = Lun();

  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdGetSelTime );
  cIpmiMsg rsp;

  SaErrorT rv = mc->SendCommand( msg, rsp, lun );

  if ( rv != SA_OK || rsp.m_data[0] != eIpmiCcOk )
       return (rv != SA_OK) ? rv : SA_ERR_HPI_INVALID_DATA;

  Lock();

  info.Entries              = SelNum();
  info.Size                 = 0xffff;
  // We don't support adding entries to SEL yet
  info.UserEventMaxSize     = 0;

  if ( AdditionTimestamp() > EraseTimestamp() )
       info.UpdateTimestamp = AdditionTimestamp();
  else
       info.UpdateTimestamp      = EraseTimestamp();

  info.UpdateTimestamp     *= 1000000000;
  info.CurrentTime          = IpmiGetUint32( rsp.m_data + 1 );
  info.CurrentTime         *= 1000000000;
  info.Enabled              = SAHPI_TRUE; // ?????
  info.OverflowFlag         = Overflow() ? SAHPI_TRUE : SAHPI_FALSE;
  info.OverflowResetable    = SAHPI_FALSE;
  info.OverflowAction       = SAHPI_EL_OVERFLOW_DROP;

  Unlock();

  return SA_OK;
}


SaErrorT
cIpmiSel::AddSelEntry( const SaHpiEventT & /*Event*/ )
{
  return SA_ERR_HPI_UNSUPPORTED_API;
}


SaErrorT
cIpmiSel::GetSelEntry( SaHpiEventLogEntryIdT current,
		       SaHpiEventLogEntryIdT &prev, SaHpiEventLogEntryIdT &next,
		       SaHpiEventLogEntryT &entry,
               SaHpiRdrT &rdr,
               SaHpiRptEntryT &rptentry )
{
  unsigned short rid = (unsigned short)current;

  if ( current == SAHPI_OLDEST_ENTRY )
       rid = 0;
  else if ( current == SAHPI_NEWEST_ENTRY )
       rid = 0xffff;

  unsigned short p;
  unsigned short n;

  cIpmiEvent e;

  SaErrorT rv = GetSelEntry( rid, p, n, e );

  if ( rv != SA_OK )
       return rv;

  cIpmiMc     *mc = 0;
  cIpmiSensor *sensor = 0;
  cIpmiAddr    addr;

  addr.m_type = eIpmiAddrTypeIpmb;

  if ( e.m_data[6] == 0x03 )
       addr.m_channel = 0;
  else
       addr.m_channel = e.m_data[5] >> 4;

  addr.m_slave_addr = e.m_data[4];
  addr.m_lun = 0;

  mc = Mc()->Domain()->FindMcByAddr( addr );

  if ( mc )
       sensor = mc->FindSensor( (e.m_data[5] & 0x3), e.m_data[8] );

  prev = p;
  next = n;

  if ( prev == 0 )
       prev = SAHPI_NO_MORE_ENTRIES;

  if ( next == 0xffff )
       next = SAHPI_NO_MORE_ENTRIES;

  entry.EntryId = e.m_record_id;
  entry.Timestamp = IpmiGetUint32( e.m_data );

  if ( entry.Timestamp == 0 )
       entry.Timestamp = SAHPI_TIME_UNSPECIFIED;
  else
       entry.Timestamp *= 1000000000;

  entry.Event.Timestamp = entry.Timestamp;

  if ( &rptentry != NULL )
    rptentry.ResourceCapabilities = 0;
  if ( &rdr != NULL )
    rdr.RdrType = SAHPI_NO_RECORD;

  if ( !sensor )
     {
       // this is possible an event of a resource
       // no longer present.
       entry.Event.Source    = 0;
       entry.Event.EventType = SAHPI_ET_OEM;
       entry.Event.Severity  = SAHPI_MAJOR;

       return SA_OK;
     }

  if ( &rptentry != NULL )
  {
      SaHpiRptEntryT *selrpt = oh_get_resource_by_id( sensor->Resource()->Domain()->GetHandler()->rptcache,
                                                      sensor->Resource()->m_resource_id );

      if ( selrpt != NULL )
          rptentry = *selrpt;
  }

  if ( &rdr != NULL )
  {
      SaHpiRdrT *selrdr = oh_get_rdr_by_id( sensor->Resource()->Domain()->GetHandler()->rptcache,
                                            sensor->Resource()->m_resource_id, sensor->RecordId() );

      if ( selrdr != NULL )
          rdr = *selrdr;
  }

  rv = sensor->CreateEvent( &e, entry.Event );

  if ( rv == SA_ERR_HPI_DUPLICATE )
      rv = SA_OK;

  return rv;
}
