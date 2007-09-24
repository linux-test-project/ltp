/*
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
 * Copyright (c) 2005 by ESO Technologies.
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

#include "ipmi_utils.h"
#include "ipmi.h"
#include "ipmi_sensor_hotswap.h"


static const char *hotswap_states[] =
{
  "inactive",
  "insertion_pending",
  "active",
  "extraction_pending",
  "not_present"
};

static int hotswap_states_num = sizeof( hotswap_states ) / sizeof( char * );


const char *
HotswapStateToString( SaHpiHsStateT state )
{
  if ( state >= hotswap_states_num )
       return "invalid";

  return hotswap_states[state];
}


SaErrorT
cIpmi::IfGetHotswapState( cIpmiResource *res, SaHpiHsStateT &state )
{
  // get hotswap sensor
  cIpmiSensorHotswap *hs = res->GetHotswapSensor();

  if ( !hs )
       return SA_ERR_HPI_INVALID_PARAMS;

  // get hotswap state
  return hs->GetHpiState( state );
}


// state == SAHPI_HS_STATE_ACTIVE
//    => M2 -> M3
//    => M5 -> M4
// state == SAHPI_HS_STATE_INACTIVE
//    => M5 -> M6
//    => M2 -> M1

SaErrorT
cIpmi::IfSetHotswapState( cIpmiResource *res, SaHpiHsStateT state )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  if (res->PolicyCanceled() != true)
  {
    return SA_ERR_HPI_INVALID_REQUEST;
  }

  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruActivation );
  msg.m_data_len = 3;
  msg.m_data[0]  = dIpmiPicMgId;
  msg.m_data[1]  = res->FruId();

  if ( state == SAHPI_HS_STATE_ACTIVE )
  {
      msg.m_data[2] = dIpmiActivateFru;
  }
  else
  {
      msg.m_data[2] = dIpmiDeactivateFru;
  }

  cIpmiMsg rsp;

  SaErrorT r = res->SendCommandReadLock( msg, rsp );

  if ( r != SA_OK )
     {
       stdlog << "IfSetHotSwapState: could not send set FRU activation: " << r << " !\n";
       return r;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfSetHotSwapState: IPMI error set FRU activation: "
	      << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INTERNAL_ERROR;
     }

  return SA_OK;
}


// act == SAHPI_HS_ACTION_INSERTION  => M1->M2
// act == SAHPI_HS_ACTION_EXTRACTION => M4->M5

SaErrorT
cIpmi::IfRequestHotswapAction( cIpmiResource *res,
                               SaHpiHsActionT act )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_REQUEST;
     }

  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruActivationPolicy );
  msg.m_data_len = 4;
  msg.m_data[0]  = dIpmiPicMgId;
  msg.m_data[1]  = res->FruId();

  if ( act == SAHPI_HS_ACTION_INSERTION )
     {
       // m1 -> m2
       msg.m_data[2] = 1; // M1->M2 lock bit
       msg.m_data[3] = 0; // clear locked bit M1->M2
     }
  else
     {
       msg.m_data[2]  = 2; // M4->M5 lock bit
       msg.m_data[3]  = 0; // clear lock bit M4->M5
     }

  cIpmiMsg  rsp;

  SaErrorT r = res->SendCommandReadLock( msg, rsp );

  if ( r != SA_OK )
     {
       stdlog << "IfRequestHotswapAction: could not send set FRU activation policy: "
	      << r << " !\n";
       return r;
     }

  if (    rsp.m_data_len != 2 
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfRequestHotswapAction: set FRU activation: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}

SaErrorT
cIpmi::IfHotswapPolicyCancel( cIpmiResource *res,
                               SaHpiTimeoutT timeout )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_REQUEST;
     }

  res->PolicyCanceled() = true;

  return SA_OK;
}

SaErrorT
cIpmi::IfSetAutoInsertTimeout( SaHpiTimeoutT timeout )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_REQUEST;
     }

  InsertTimeout() = timeout;

  return SA_OK;
}

SaErrorT
cIpmi::IfGetAutoExtractTimeout( cIpmiResource *res, SaHpiTimeoutT &timeout )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_REQUEST;
     }

  timeout = res->ExtractTimeout();

  return SA_OK;
}

SaErrorT
cIpmi::IfSetAutoExtractTimeout( cIpmiResource *res, SaHpiTimeoutT timeout )
{
  if ( !m_is_tca )
     {
       stdlog << "ATCA not supported by SI !\n";
       return SA_ERR_HPI_INVALID_REQUEST;
     }

  res->ExtractTimeout() = timeout;

  return SA_OK;
}

SaErrorT
cIpmi::IfGetPowerState( cIpmiResource *res, SaHpiPowerStateT &state )
{
  if (res->Mc()->IsRmsBoard()) {
     cIpmiMsg msg( eIpmiNetfnChassis, eIpmiCmdGetChassisStatus );
     cIpmiMsg rsp;
     msg.m_data_len = 0;
     SaErrorT rv = res->SendCommandReadLock( msg, rsp );
     if (rv != SA_OK) {
        stdlog << "IfGetPowerState:  error " << rv << "\n";
     } else if (rsp.m_data[0] != eIpmiCcOk) {
        stdlog << "IfGetPowerState:  ccode " << rsp.m_data[0] << "\n";
        return (SA_ERR_HPI_INVALID_DATA);
     } else {
        if (rsp.m_data[1] & 0x01) state = SAHPI_POWER_ON;
        else   state = SAHPI_POWER_OFF;
        // if ((rsp.mdata[1] & 0x1E) != 0) /*power fault*/;
     }
     return rv;
  }

  // get power level
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPowerLevel );
  cIpmiMsg rsp;

  msg.m_data[0] = dIpmiPicMgId;
  msg.m_data[1] = res->FruId();
  msg.m_data[2] = 0x01; // desired steady power
  msg.m_data_len = 3;

  SaErrorT rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "cannot send get power level: " << rv << " !\n";
       return rv;
     }

  if (    rsp.m_data_len < 3
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[0] != dIpmiPicMgId )
     {
       stdlog << "cannot get power level: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  unsigned char power_level = rsp.m_data[2] & 0x1f;

  // get current power level
  msg.m_data[2]  = 0; // steady state power

  rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "IfGetPowerState: could not send get power level: " << rv << " !\n";
       return rv;
     }

  if (    rsp.m_data_len < 6
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfGetPowerState: IPMI error get power level: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
     }

  unsigned char current_power_level = rsp.m_data[2] & 0x1f;
  
  if ( current_power_level >= power_level )
       state = SAHPI_POWER_ON;
  else
       state = SAHPI_POWER_OFF;

  return SA_OK;
}


SaErrorT
cIpmi::IfSetPowerState( cIpmiResource *res, SaHpiPowerStateT state )
{
  SaErrorT rv;
  unsigned int power_level = 0;

  if (res->Mc()->IsRmsBoard()) {
     unsigned char power_state = 0;
     switch (state) {
        case SAHPI_POWER_CYCLE:  power_state = 0x02; break;
        case SAHPI_POWER_ON:     power_state = 0x01; break;
        case SAHPI_POWER_OFF:    power_state = 0x00; break;
        default:  power_state = 0x02; break;
     }
     cIpmiMsg msg( eIpmiNetfnChassis, eIpmiCmdChassisControl );
     msg.m_data[0]  = power_state;
     msg.m_data_len = 1;
     cIpmiMsg rsp;
     rv = res->SendCommandReadLock( msg, rsp );
     if (rv != SA_OK)
        stdlog << "IfSetPowerState: state " << power_state << " error " << rv << "\n";
     return rv;
  }

  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPowerLevel );
  msg.m_data[0] = dIpmiPicMgId;
  msg.m_data[1] = res->FruId();
  cIpmiMsg rsp;

  if ( state == SAHPI_POWER_CYCLE )
     {
       // power off
       msg.m_cmd = eIpmiCmdSetPowerLevel;

       msg.m_data[2] = power_level;
       msg.m_data[3] = 0x01; // copy desierd level to present level
       msg.m_data_len = 4;

       rv = res->SendCommandReadLock( msg, rsp );

       if ( rv != SA_OK )
          {
            stdlog << "cannot send set power level: " << rv << " !\n";
            return rv;
          }

       if (    rsp.m_data_len < 2
               || rsp.m_data[0] != eIpmiCcOk 
               || rsp.m_data[1] != dIpmiPicMgId )
          {
            stdlog << "cannot set power level: " <<  rsp.m_data[0] << " !\n";
            return SA_ERR_HPI_INVALID_CMD;
          }

       // power on
       state = SAHPI_POWER_ON;
     }

  if ( state == SAHPI_POWER_ON )
     {
       // get power level
       msg.m_cmd = eIpmiCmdGetPowerLevel;

       msg.m_data[2] = 0x01; // desired steady power
       msg.m_data_len = 3;

       rv = res->SendCommandReadLock( msg, rsp );

       if ( rv != SA_OK )
          {
            stdlog << "cannot send get power level: " << rv << " !\n";
            return SA_ERR_HPI_INVALID_CMD;
          }

       if (    rsp.m_data_len < 3
            || rsp.m_data[0] != eIpmiCcOk 
            || rsp.m_data[1] != dIpmiPicMgId )
          {
            stdlog << "cannot get power level: " << rsp.m_data[0] << " !\n";
            return SA_ERR_HPI_INVALID_CMD;
          }

       power_level = rsp.m_data[2] & 0x1f;
     }
  else if ( state != SAHPI_POWER_OFF )
       return SA_ERR_HPI_INVALID_PARAMS;

  // set power level
  msg.m_cmd = eIpmiCmdSetPowerLevel;

  msg.m_data[2] = power_level;
  msg.m_data[3] = 0x01; // copy desierd level to present level
  msg.m_data_len = 4;

  rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "cannot send set power level: " << rv << "! \n";
       return rv;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != eIpmiCcOk 
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "cannot set power level: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfGetIndicatorState( cIpmiResource *res, SaHpiHsIndicatorStateT &state )
{
  cIpmiMsg  msg( eIpmiNetfnPicmg, eIpmiCmdGetFruLedState );
  cIpmiMsg  rsp;

  msg.m_data_len = 3;
  msg.m_data[0]  = dIpmiPicMgId;
  msg.m_data[1]  = res->FruId();
  msg.m_data[2]  = 0; // blue led;

  SaErrorT rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "IfGetIndicatorState: could not send get FRU LED state: " << rv << " !\n";
       return rv;
     }

  if (    rsp.m_data_len < 6
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfGetIndicatorState: IPMI error set FRU LED state: "
	      <<  rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  // lamp test
  if ( rsp.m_data[2] & 4 )
     {
       if ( rsp.m_data_len < 10 )
          {
            stdlog << "IfGetIndicatorState: IPMI error (lamp test) message to short: "
		   << rsp.m_data_len << " !\n";

            return SA_ERR_HPI_INVALID_DATA;
          }

       state = SAHPI_HS_INDICATOR_ON;
       return SA_OK;
     }
  
  // overwrite state
  if ( rsp.m_data[2] & 2 )
     {
       if ( rsp.m_data_len < 9 )
          {
            stdlog << "IfGetIndicatorState: IPMI error (overwrite) message to short: " 
		   << rsp.m_data_len << " !\n";

            return SA_ERR_HPI_INVALID_DATA;
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
cIpmi::IfSetIndicatorState( cIpmiResource *res, SaHpiHsIndicatorStateT state )
{
  cIpmiMsg  msg( eIpmiNetfnPicmg, eIpmiCmdSetFruLedState );
  msg.m_data_len = 6;
  msg.m_data[0]  = dIpmiPicMgId;
  msg.m_data[1]  = res->FruId();
  msg.m_data[2]  = 0; // blue led;

  msg.m_data[3]  = (state == SAHPI_HS_INDICATOR_ON) ? 0xff : 0;
  msg.m_data[4]  = 0;
  msg.m_data[5]  = 1; // blue

  cIpmiMsg rsp;
  SaErrorT rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "IfGetIndicatorState: could not send get FRU LED state: "
	      << rv << " !\n";
       return rv;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfGetIndicatorState: IPMI error set FRU LED state: " 
	      << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfGetResetState( cIpmiResource * /*res*/, SaHpiResetActionT &state )
{
  state = SAHPI_RESET_DEASSERT;

  return SA_OK;
}


SaErrorT
cIpmi::IfSetResetState( cIpmiResource *res, SaHpiResetActionT state )
{
  unsigned char reset_state;
  unsigned char chassis_state;

  switch( state )
     {
       case SAHPI_COLD_RESET:
            reset_state = 0x00;
            chassis_state = 0x02;
            break;

       case SAHPI_WARM_RESET:
            // There is no way to know whether an ATCA FRU supports
            // warm reset -> Let's use cold reset all the time for now
            reset_state = 0x00;
            chassis_state = 0x03; 
            break;

       case SAHPI_RESET_DEASSERT:
            // Reset is *always* deasserted on ATCA
            return SA_OK;

       default:
            stdlog << "IfSetResetState: unsupported state " << state << " !\n";
            return SA_ERR_HPI_INVALID_CMD;
     }

  if (res->Mc()->IsRmsBoard()) {
     cIpmiMsg msg( eIpmiNetfnChassis, eIpmiCmdChassisControl );
     msg.m_data[0]  = chassis_state;
     msg.m_data_len = 1;
     cIpmiMsg rsp;
     SaErrorT rv = res->SendCommandReadLock( msg, rsp );
     if (rv != SA_OK)
        stdlog << "IfSetResetState: could not send Chassis Reset: " << rv << "\n";
     return rv;
  }

  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdFruControl );
  msg.m_data[0]  = dIpmiPicMgId;
  msg.m_data[1]  = res->FruId();
  msg.m_data[2]  = reset_state;
  msg.m_data_len = 3;

  cIpmiMsg rsp;
  SaErrorT rv = res->SendCommandReadLock( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "IfSetResetState: could not send FRU control: " << rv << " !\n";
       return rv;
     }

  if (    rsp.m_data_len < 2
       || rsp.m_data[0] != 0
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "IfSetResetState: IPMI error FRU control: "
	      << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}
