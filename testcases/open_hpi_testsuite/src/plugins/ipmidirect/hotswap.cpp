/*
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ipmi_utils.h"
#include "ipmi.h"


static const char *hs_state[] =
{
  "inactive",
  "insertion_pending",
  "active_healthy",
  "active_unhealthy",
  "extraction_pending",
  "not_present"
};


void
cIpmi::HotswapGenerateEvent( cIpmiSensor *sensor, cIpmiEntity *ent,
                             cIpmiEvent *event, SaHpiHsStateT state )
{
  oh_event *e = (oh_event *)g_malloc0( sizeof( oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );
  e->type = oh_event::OH_ET_HPI;
  e->u.hpi_event.parent = ent->m_resource_id;

  if ( sensor )
       e->u.hpi_event.id = sensor->m_record_id;
  else
       e->u.hpi_event.id = 0;

  e->u.hpi_event.event.Source = 0;

  e->u.hpi_event.event.EventType = SAHPI_ET_HOTSWAP;
  e->u.hpi_event.event.Timestamp = IpmiGetUint32( event->m_data );

  // Do not find the severity of hotswap event
  e->u.hpi_event.event.Severity = SAHPI_MAJOR;

  e->u.hpi_event.event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = (SaHpiHsStateT)ent->HsState();
  e->u.hpi_event.event.EventDataUnion.HotSwapEvent.HotSwapState = state;

  IpmiLog( "Creating HPI hotswap event: %s -> %s.\n",
           hs_state[ent->HsState()], hs_state[state] );

  dbg( "creating Creating HPI hotswap event: %s -> %s.\n",
       hs_state[ent->HsState()], hs_state[state] );

  //assert( (SaHpiHsStateT)ent->HsState() != state );

  ent->HsState() = state;

  AddHpiEvent( e );
}


void
cIpmi::IfHotswapEvent( cIpmiMc *mc, cIpmiSensor *sensor,
                       cIpmiEvent *event )
{
  tIpmiFruState current_state = (tIpmiFruState)(event->m_data[10] & 0x0f);
  tIpmiFruState prev_state    = (tIpmiFruState)(event->m_data[11] & 0x0f);

  if ( mc == 0 )
     {
       IpmiLog( "hotswap event from unknown mc !\n" );
       return;
     }

  if ( sensor == 0 )
     {
       IpmiLog( "hotswap event from unknown sensor !\n" );
       assert( 0 );
     }

  // Not a state change. SetEventReceiver => sensor rearm
  if ( current_state == prev_state )
     {
       assert( current_state != prev_state );
       return;
     }

  cIpmiEntity *ent = sensor->GetEntity();
  assert( ent );

  SaHpiHsStateT state;

  switch( current_state )
     {
       case eIpmiFruStateNotInstalled:
            // prev == eIpmiFruStateInactive or eIpmiFruStateCommunicationLost
            state = SAHPI_HS_STATE_NOT_PRESENT;
            break;

       case eIpmiFruStateInactive:
            // eIpmiFruStateNotInstalled
            // => eIpmiFruStateInactive
            // => eIpmiFruStateActivationRequest
            if ( prev_state == eIpmiFruStateNotInstalled )
                 return;

            state = SAHPI_HS_STATE_INACTIVE;
            break;

       case eIpmiFruStateActivationRequest:
            state = SAHPI_HS_STATE_INSERTION_PENDING;
            break;

       case eIpmiFruStateActivationInProgress:
            return;

       case eIpmiFruStateActive:
            state = SAHPI_HS_STATE_ACTIVE_HEALTHY;
            break;

       case eIpmiFruStateDeactivationRequest:
            state = SAHPI_HS_STATE_EXTRACTION_PENDING;
            break;

       case eIpmiFruStateDeactivationInProgress:
            return;

       case eIpmiFruStateCommunicationLost:
            state = SAHPI_HS_STATE_NOT_PRESENT;
            break;

       default:
            assert( 0 );
            return;
     }

  if ( ent->HsState() == state )
     {
       //assert( ent->HsState() != state );
       return;
     }

  HotswapGenerateEvent( sensor, ent, event, state );
}


SaErrorT
cIpmi::IfGetHotswapState( cIpmiEntity *ent, SaHpiHsStateT &state )
{
  // is it better to get this from hot swap sensor ?
  state = ent->HsState();

  return SA_OK;
}


SaErrorT
cIpmi::IfSetHotswapState( cIpmiEntity *ent, SaHpiHsStateT state )
{
  if ( !m_is_atca )
     {
       IpmiLog( "ATCA not supported by SI !\n" );
       return SA_ERR_HPI_INVALID;
     }

  unsigned char fru_state;

  switch( state )
     {
       case SAHPI_HS_STATE_ACTIVE_HEALTHY:
            fru_state = dIpmiActivateFru;
            break;

       case SAHPI_HS_STATE_INACTIVE:
            fru_state = dIpmiDeactivateFru;
            break;

       default:
            IpmiLog( "IfSetHotSwapState: illegal state %d !\n", state );
            return SA_ERR_HPI_INVALID;
     }

  cIpmiMsg  cmd_msg;
  cIpmiMsg  rsp;
  cIpmiAddr addr;

  addr.m_type       = eIpmiAddrTypeIpmb;
  addr.m_channel    = ent->Channel();
  addr.m_slave_addr = ent->AccessAddress();
  addr.m_lun        = 0; //ent->lun;

  cmd_msg.m_netfn    = eIpmiNetfnPicmg;
  cmd_msg.m_cmd      = eIpmiCmdSetFruActivation;
  cmd_msg.m_data_len = 3;
  cmd_msg.m_data[0]  = dIpmiPigMgId;
  cmd_msg.m_data[1]  = ent->FruId();
  cmd_msg.m_data[2]  = fru_state;

  int rv = SendCommand( addr, cmd_msg, rsp );

  if ( rv )
     {
       IpmiLog( "IfSetHotSwapState: could not send set FRU activation: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "IfSetHotSwapState: IPMI error set FRU activation: %x !\n",
                rsp.m_data[0]);

       return SA_ERR_HPI_DATA_LEN_INVALID;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfRequestHotswapAction( cIpmiEntity *ent,
                               SaHpiHsActionT act )
{
  if ( !m_is_atca )
     {
       IpmiLog( "ATCA not supported by SI !\n" );
       return SA_ERR_HPI_INVALID;
     }

  unsigned char bit_mask = 0;

  if ( act == SAHPI_HS_ACTION_INSERTION )
     {
       if ( (SaHpiHsStateT)ent->HsState() != SAHPI_HS_STATE_INACTIVE )
            return SA_ERR_HPI_INVALID;

       bit_mask = 1;
     }
  else
     {
       if ( (SaHpiHsStateT)ent->HsState() != SAHPI_HS_STATE_ACTIVE_HEALTHY )
            return SA_ERR_HPI_INVALID;

       bit_mask = 2;
     }

  cIpmiMsg  msg;
  cIpmiMsg  rsp;
  cIpmiAddr addr;

  addr.m_type       = eIpmiAddrTypeIpmb;
  addr.m_channel    = ent->Channel();
  addr.m_slave_addr = ent->AccessAddress();
  addr.m_lun        = 0; //ent->lun;

  msg.m_netfn    = eIpmiNetfnPicmg;
  msg.m_cmd      = eIpmiCmdSetFruActivationPolicy;
  msg.m_data_len = 4;
  msg.m_data[0]  = dIpmiPigMgId;
  msg.m_data[1]  = ent->FruId();
  msg.m_data[2]  = bit_mask;
  msg.m_data[3]  = 0; // clear the activation/deactivation locked

  int rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "IfRequestHotswapAction: could not send set FRU activation policy: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfGetPowerState( cIpmiEntity *ent, SaHpiHsPowerStateT &state )
{
  cIpmiAddr addr( eIpmiAddrTypeIpmb, ent->Channel(), 
                  0, ent->AccessAddress() );

  // get power level
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPowerLevel );
  cIpmiMsg rsp;

  msg.m_data[0] = dIpmiPigMgId;
  msg.m_data[1] = ent->FruId();
  msg.m_data[2] = 0x01; // desired steady power
  msg.m_data_len = 3;

  int rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send get power level: %d\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 3
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[0] != dIpmiPigMgId )
     {
       IpmiLog( "cannot get power level: 0x%02x !\n", rsp.m_data[0] );
       return SA_ERR_HPI_INVALID_CMD;
     }

  unsigned char power_level = rsp.m_data[2] & 0x1f;

  // get current power level
  msg.m_data[2]  = 0; // steady state power

  rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "SetHotSwapState: could not send get power level: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 6
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "SetHotSwapState: IPMI error get power level: 0x%02x !\n",
                rsp.m_data[0]);

       return SA_ERR_HPI_INVALID_CMD;
     }

  unsigned char current_power_level = rsp.m_data[2] & 0x1f;
  
  if ( current_power_level >= power_level )
       state = SAHPI_HS_POWER_ON;
  else
       state = SAHPI_HS_POWER_OFF;

  return SA_OK;
}


SaErrorT
cIpmi::IfSetPowerState( cIpmiEntity *ent, SaHpiHsPowerStateT state )
{
  int rv;
  unsigned int power_level = 0;

  cIpmiAddr addr( eIpmiAddrTypeIpmb, ent->Channel(), 
                  0, ent->AccessAddress() );
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPowerLevel );
  msg.m_data[0] = dIpmiPigMgId;
  msg.m_data[1] = ent->FruId();
  cIpmiMsg rsp;

  if ( state == SAHPI_HS_POWER_CYCLE )
     {
       // power off
       msg.m_cmd = eIpmiCmdSetPowerLevel;

       msg.m_data[2] = power_level;
       msg.m_data[3] = 0x01; // copy desierd level to present level
       msg.m_data_len = 4;

       rv = SendCommand( addr, msg, rsp );

       if ( rv )
          {
            IpmiLog( "cannot send set power level: %d\n", rv );
            return SA_ERR_HPI_INVALID_CMD;
          }

       if (    rsp.m_data_len < 2
               || rsp.m_data[0] != eIpmiCcOk 
               || rsp.m_data[1] != dIpmiPigMgId )
          {
            IpmiLog( "cannot set power level: 0x%02x !\n", rsp.m_data[0] );
            return SA_ERR_HPI_INVALID_CMD;
          }

       // power on
       state = SAHPI_HS_POWER_ON;
     }

  if ( state == SAHPI_HS_POWER_ON )
     {
       // get power level
       msg.m_cmd = eIpmiCmdGetPowerLevel;

       msg.m_data[2] = 0x01; // desired steady power
       msg.m_data_len = 3;

       rv = SendCommand( addr, msg, rsp );

       if ( rv )
          {
            IpmiLog( "cannot send get power level: %d\n", rv );
            return SA_ERR_HPI_INVALID_CMD;
          }

       if (    rsp.m_data_len < 3
            || rsp.m_data[0] != eIpmiCcOk 
            || rsp.m_data[1] != dIpmiPigMgId )
          {
            IpmiLog( "cannot get power level: 0x%02x !\n", rsp.m_data[0] );
            return SA_ERR_HPI_INVALID_CMD;
          }

       power_level = rsp.m_data[2] & 0x1f;
     }
  else
       assert( state == SAHPI_HS_POWER_OFF );

  // set power level
  msg.m_cmd = eIpmiCmdSetPowerLevel;

  msg.m_data[2] = power_level;
  msg.m_data[3] = 0x01; // copy desierd level to present level
  msg.m_data_len = 4;

  rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot send set power level: %d\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "cannot set power level: 0x%02x !\n", rsp.m_data[0] );
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfGetIndicatorState( cIpmiEntity *ent, SaHpiHsIndicatorStateT &state )
{
  cIpmiMsg  msg;
  cIpmiMsg  rsp;
  cIpmiAddr addr;

  addr.m_type       = eIpmiAddrTypeIpmb;
  addr.m_channel    = ent->Channel();
  addr.m_slave_addr = ent->AccessAddress();
  addr.m_lun        = 0; //ent->lun;

  msg.m_netfn    = eIpmiNetfnPicmg;
  msg.m_cmd      = eIpmiCmdGetFruLedState;
  msg.m_data_len = 3;
  msg.m_data[0]  = dIpmiPigMgId;
  msg.m_data[1]  = ent->FruId();
  msg.m_data[2]  = 0; // blue led;

  int rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "IfGetIndicatorState: could not send get FRU LED state: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 6
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "IfGetIndicatorState: IPMI error set FRU LED state: %x !\n",
                rsp.m_data[0]);

       return SA_ERR_HPI_DATA_LEN_INVALID;
     }

  // lamp test
  if ( rsp.m_data[2] & 4 )
     {     
       if ( rsp.m_data_len < 10 )
          {
            IpmiLog( "IfGetIndicatorState: IPMI error (lamp test) message to short: %d !\n",
                     rsp.m_data_len);

            return SA_ERR_HPI_DATA_LEN_INVALID;
          }
       
       state = SAHPI_HS_INDICATOR_ON;
       return SA_OK;
     }
  
  // overwrite state
  if ( rsp.m_data[2] & 2 )
     {
       if ( rsp.m_data_len < 9 )
          {
            IpmiLog( "IfGetIndicatorState: IPMI error (overwrite) message to short: %d !\n",
                     rsp.m_data_len );

            return SA_ERR_HPI_DATA_LEN_INVALID;
          }

       if ( rsp.m_data[6] == 0 )
            state = SAHPI_HS_INDICATOR_OFF;
       else
            state = SAHPI_HS_INDICATOR_ON;

       return SA_OK;
     }

  // local control state
  if ( rsp.m_data[3] == 0 )
       state = SAHPI_HS_INDICATOR_OFF;
  else
       state = SAHPI_HS_INDICATOR_ON;

  return SA_OK;
}


SaErrorT
cIpmi::IfSetIndicatorState( cIpmiEntity *ent, SaHpiHsIndicatorStateT state )
{
  cIpmiMsg  msg;
  cIpmiMsg  rsp;
  cIpmiAddr addr;

  addr.m_type       = eIpmiAddrTypeIpmb;
  addr.m_channel    = ent->Channel();
  addr.m_slave_addr = ent->AccessAddress();
  addr.m_lun        = 0; //ent->lun;

  msg.m_netfn    = eIpmiNetfnPicmg;
  msg.m_cmd      = eIpmiCmdSetFruLedState;
  msg.m_data_len = 6;
  msg.m_data[0]  = dIpmiPigMgId;
  msg.m_data[1]  = ent->FruId();
  msg.m_data[2]  = 0; // blue led;

  msg.m_data[3]  = (state == SAHPI_HS_INDICATOR_ON) ? 0xff : 0;
  msg.m_data[4]  = 0;
  msg.m_data[5]  = 1; // blue

  int rv = SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "IfGetIndicatorState: could not send get FRU LED state: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "IfGetIndicatorState: IPMI error set FRU LED state: %x !\n",
                rsp.m_data[0] );

       return SA_ERR_HPI_DATA_LEN_INVALID;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfGetResetState( cIpmiEntity * /*ent*/, SaHpiResetActionT &state )
{
  state = SAHPI_RESET_DEASSERT;

  return SA_OK;
}


SaErrorT
cIpmi::IfSetResetState( cIpmiEntity *ent, SaHpiResetActionT state )
{
  unsigned char reset_state;

  switch( state )
     {
       case SAHPI_COLD_RESET:
            reset_state = 0x00;
            break;

       case SAHPI_WARM_RESET:
            reset_state = 0x01;
            break;

       default:
            IpmiLog( "IfSetResetState: unsupported state 0x%02x !\n", state );
            return SA_ERR_HPI_INVALID_CMD;
     }

  cIpmiMsg  cmd_msg;
  cIpmiMsg  rsp;
  cIpmiAddr addr;

  addr.m_type       = eIpmiAddrTypeIpmb;
  addr.m_channel    = ent->Channel();
  addr.m_slave_addr = ent->AccessAddress();
  addr.m_lun        = ent->Lun();

  cmd_msg.m_netfn    = eIpmiNetfnPicmg;
  cmd_msg.m_cmd      = eIpmiCmdFruControl;
  cmd_msg.m_data[0]  = dIpmiPigMgId;
  cmd_msg.m_data[1]  = ent->FruId();
  cmd_msg.m_data[2]  = state;
  cmd_msg.m_data_len = 3;

  int rv = SendCommand( addr, cmd_msg, rsp );

  if ( rv )
     {
       IpmiLog( "SetHotSwapState: could not send FRU control: 0x%02x !\n", rv );
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPigMgId )
     {
       IpmiLog( "SetHotSwapState: IPMI error set FRU activation: %x !\n",
                rsp.m_data[0]);

       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}
