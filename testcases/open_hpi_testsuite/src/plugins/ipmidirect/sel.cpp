/*
 * sel.cpp
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


#include "ipmi.h"
#include "ipmi_utils.h"


void
cIpmi::IfSelAdd( cIpmiEntity *ent, cIpmiSel * /*sel*/ )
{
  IpmiLog( "adding SEL\n" );

  dbg( "adding SEL %d.%d (%s)",
       ent->EntityId(), ent->EntityInstance(),
       ent->EntityIdString() );

   // find resource
  SaHpiRptEntryT *resource = ent->Domain()->FindResource( ent->m_resource_id );

  if ( !resource )
     {
       assert( 0 );
       return;
     }

  assert( (resource->ResourceCapabilities & SAHPI_CAPABILITY_SEL ) == 0 );
  
  // update resource
  resource->ResourceCapabilities |= SAHPI_CAPABILITY_SEL;

  struct oh_event *e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );
  e->type               = oh_event::OH_ET_RESOURCE;
  e->u.res_event.entry = *resource;

  AddHpiEvent( e );
}


SaErrorT
cIpmi::IfGetSelInfo( cIpmiSel *sel, SaHpiSelInfoT &info )
{
  cIpmiMc *mc = sel->Mc();
  int lun = sel->Lun();

  cIpmiMsg msg( eIpmiNetfnStorage, eIpmiCmdGetSelTime );
  cIpmiMsg rsp;

  int rv = mc->SendCommand( msg, rsp, lun );

  if ( rv || rsp.m_data[0] != eIpmiCcOk )
       return SA_ERR_HPI_INVALID_DATA;

  sel->Lock();

  info.Entries              = sel->SelNum();
  info.Size                 = 0xffff;

  if ( sel->AdditionTimestamp() > sel->EraseTimestamp() )
       info.UpdateTimestamp      = sel->AdditionTimestamp();
  else
       info.UpdateTimestamp      = sel->EraseTimestamp();

  info.UpdateTimestamp     *= 1000000000;
  info.CurrentTime          = IpmiGetUint32( rsp.m_data + 1 );
  info.CurrentTime         *= 1000000000;
  info.Enabled              = SAHPI_TRUE; // ?????
  info.OverflowFlag         = sel->Overflow() ? SAHPI_TRUE : SAHPI_FALSE;
  info.OverflowAction       = SAHPI_SEL_OVERFLOW_DROP;
  info.DeleteEntrySupported = sel->SupportsDeleteSel() ? SAHPI_TRUE : SAHPI_FALSE;

  sel->Unlock();

  return SA_OK;
}


SaErrorT
cIpmi::IfGetSelTime( cIpmiSel *sel, SaHpiTimeT &time )
{
  time_t t;
  
  int rv = sel->GetSelTime( t );
  
  if ( rv )
       return SA_ERR_HPI_ERROR;

  time = t;
  time *= 1000000000;

  return SA_OK;
}


static time_t
CovertToAbsTimeT( SaHpiTimeT time )
{
  if ( time <= SAHPI_TIME_MAX_RELATIVE )
     {
       timeval tv;
       gettimeofday( &tv, 0 );
       
       tv.tv_sec += time / 1000000000;
       tv.tv_usec += time % 1000000000 / 1000;

       while( tv.tv_usec > 1000000 )
          {
            tv.tv_sec++;
            tv.tv_usec -= 1000000;
          }
 
       return tv.tv_sec;
     }

  return time / 1000000000;
}


SaErrorT
cIpmi::IfSetSelTime( cIpmiSel *sel, SaHpiTimeT time )
{
  if ( time == SAHPI_TIME_UNSPECIFIED )
       return SA_ERR_HPI_ERROR;

  time_t t = CovertToAbsTimeT( time );

  int rv = sel->SetSelTime( t );

  if ( rv )
       return SA_ERR_HPI_ERROR;

  return SA_OK;
}


SaErrorT
cIpmi::IfAddSelEntry( cIpmiSel * /*sel*/, const SaHpiSelEntryT & /*Event*/ )
{
  return SA_ERR_HPI_UNSUPPORTED_API;
}


SaErrorT
cIpmi::IfDelSelEntry( cIpmiSel *sel, SaHpiSelEntryIdT sid )
{
  unsigned short rid = (unsigned short)sid;

  if ( sid == SAHPI_OLDEST_ENTRY )
       rid = 0;
  else if ( sid == SAHPI_NEWEST_ENTRY )
       rid = 0xffff;

  int rv = sel->DeleteSelEntry( rid );

  if ( rv )
       return SA_ERR_HPI_ERROR;

  return SA_OK;
}


static SaHpiHsStateT
MapAtcaToHpiHotswapState( tIpmiFruState state )
{
  switch( state )
     {
       case eIpmiFruStateNotInstalled:
            return SAHPI_HS_STATE_NOT_PRESENT;

       case eIpmiFruStateInactive:
            return SAHPI_HS_STATE_INACTIVE;

       case eIpmiFruStateActivationRequest:
            return SAHPI_HS_STATE_INSERTION_PENDING;

       case eIpmiFruStateActivationInProgress:
            return SAHPI_HS_STATE_INSERTION_PENDING;

       case eIpmiFruStateActive:
            return SAHPI_HS_STATE_ACTIVE_HEALTHY;

       case eIpmiFruStateDeactivationRequest:
            return SAHPI_HS_STATE_EXTRACTION_PENDING;

       case eIpmiFruStateDeactivationInProgress:
            return SAHPI_HS_STATE_EXTRACTION_PENDING;

       case eIpmiFruStateCommunicationLost:
            return SAHPI_HS_STATE_NOT_PRESENT;
     }

  // not reached
  assert( 0 );

  return SAHPI_HS_STATE_NOT_PRESENT;
}


SaErrorT
cIpmi::IfGetSelEntry( cIpmiSel *sel, SaHpiSelEntryIdT current,
                      SaHpiSelEntryIdT &prev, SaHpiSelEntryIdT &next,
                      SaHpiSelEntryT &entry )
{
  unsigned short rid = (unsigned short)current;

  if ( current == SAHPI_OLDEST_ENTRY )
       rid = 0;
  else if ( current == SAHPI_NEWEST_ENTRY )
       rid = 0xffff;

  unsigned short p;
  unsigned short n;

  cIpmiEvent e;

  int rv = sel->GetSelEntry( rid, p, n, e );

  if ( rv == -1 )
       // sel empty
       return SA_ERR_HPI_NOT_PRESENT;

  if ( rv )
       return SA_ERR_HPI_ERROR;

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

  mc = FindMcByAddr( addr );

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

  if ( !sensor )
     {
       // this is possible an event of a resource
       // no longer present.
       entry.Event.Source    = 0;
       entry.Event.EventType = SAHPI_ET_OEM;
       entry.Event.Severity  = SAHPI_MAJOR;

       return SA_OK;
     }

  // resource id
  cIpmiEntity *ent = sensor->GetEntity();

  if ( ent )
       entry.Event.Source = ent->m_resource_id;

  if ( sensor->SensorType() == eIpmiSensorTypeAtcaHotSwap )
     {
       // ATCA hotswap sensor
       entry.Event.Severity  = SAHPI_MAJOR;
       entry.Event.EventType = SAHPI_ET_HOTSWAP;

       tIpmiFruState current_state = (tIpmiFruState)(e.m_data[10] & 0x0f);
       tIpmiFruState prev_state    = (tIpmiFruState)(e.m_data[11] & 0x0f);

       SaHpiHotSwapEventT *he = &entry.Event.EventDataUnion.HotSwapEvent;
       he->HotSwapState         = MapAtcaToHpiHotswapState( current_state );
       he->PreviousHotSwapState = MapAtcaToHpiHotswapState( prev_state );

       return SA_OK;
     }

  entry.Event.EventType = SAHPI_ET_SENSOR;

  SaHpiSensorEventT *se = &entry.Event.EventDataUnion.SensorEvent;
  se->SensorNum  = sensor->Num();
  se->SensorType = (SaHpiSensorTypeT)sensor->SensorType();

  tIpmiEventDir dir = (tIpmiEventDir)(e.m_data[9] >> 7);

  if ( sensor->IsThreshold() )
     {
       // threshold sensor
       se->EventCategory = SAHPI_EC_THRESHOLD;

       tIpmiThresh        threshold = (tIpmiThresh)((e.m_data[10] >> 1) & 0x07);
       tIpmiEventValueDir high_low  = (tIpmiEventValueDir)(e.m_data[10] & 1);

       SetThresholedSensorEventState( threshold, dir, high_low,
                                      se, &entry.Event.Severity );

       SetThresholdsSensorMiscEvent( sensor, &e, se );

       return SA_OK;
     }

  // digital event
  entry.Event.Severity = SAHPI_OK;

  if ( (e.m_data[10] >> 6 ) == 2 )
     {
       entry.Event.Severity = (SaHpiSeverityT)(e.m_data[11] >> 4);

       if ( entry.Event.Severity == 0xf )
            entry.Event.Severity = SAHPI_OK;
     }

  se->EventCategory = (SaHpiEventCategoryT)e.m_data[9] & 0x7f;
  se->Assertion     = (SaHpiBoolT)!(dir);

  SetDiscreteSensorEventState( &e, &se->EventState );
  SetDiscreteSensorMiscEvent( &e, se );

  return SA_OK;
}


SaErrorT
cIpmi::IfClearSel( cIpmiSel *sel )
{
  int rv = sel->ClearSel();

  if ( rv )
       return SA_ERR_HPI_ERROR;

  return SA_OK;
}
