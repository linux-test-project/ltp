/*
 * ipmi_watchdog.cpp
 *
 * Copyright (c) 2006-2008 by ESO Technologies.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#include "ipmi_watchdog.h"
#include "ipmi_domain.h"


cIpmiWatchdog::cIpmiWatchdog( cIpmiMc *mc,
			      unsigned int num,
                              unsigned int oem )
  : cIpmiRdr( mc, SAHPI_WATCHDOG_RDR ),
    m_num( num ),
    m_oem( oem )
{
}


cIpmiWatchdog::~cIpmiWatchdog()
{
}

bool
cIpmiWatchdog::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  if ( cIpmiRdr::CreateRdr( resource, rdr ) == false )
       return false;

  // update resource
  resource.ResourceCapabilities |= SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_WATCHDOG;

  // watchdog record
  SaHpiWatchdogRecT &rec = rdr.RdrTypeUnion.WatchdogRec;

  rec.WatchdogNum = m_num;
  rec.Oem         = m_oem;

  return true;
}

/*
 * Watchdog Timer routines 
 *               - added 10/10/2006 ARCress
 * 
 */
SaHpiWatchdogPretimerInterruptT
WDPI2Hpi(unsigned char b)
{
   switch(b) {
        case 0x10:  return SAHPI_WPI_SMI;
        case 0x20:  return SAHPI_WPI_NMI;
        case 0x30:  return SAHPI_WPI_MESSAGE_INTERRUPT;
        case 0x70:  return SAHPI_WPI_OEM;
        default:    return SAHPI_WPI_NONE;
   }
}

SaHpiWatchdogTimerUseT
WDTimerUse2Hpi(unsigned char b)
{
   switch(b) {
       case 0:  return SAHPI_WTU_NONE;
       case 1:  return SAHPI_WTU_BIOS_FRB2;
       case 2:  return SAHPI_WTU_BIOS_POST;
       case 3:  return SAHPI_WTU_OS_LOAD;
       case 4:  return SAHPI_WTU_SMS_OS;
       case 5:  return SAHPI_WTU_OEM;
       default: return SAHPI_WTU_UNSPECIFIED;
   }
}

SaHpiWatchdogActionT
WDAction2Hpi(unsigned char b)
{
   switch(b) {
       case 0:  return SAHPI_WA_NO_ACTION;
       case 1:  return SAHPI_WA_RESET;
       case 2:  return SAHPI_WA_POWER_DOWN;
       case 3:  return SAHPI_WA_POWER_CYCLE;
       default: return SAHPI_WA_RESET;
   }
}

SaErrorT
cIpmiWatchdog::GetWatchdogInfo( SaHpiWatchdogT &watchdog)
{
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdGetWatchdogTimer );
  cIpmiMsg  rsp;
  SaErrorT rv;
  stdlog << "GetWatchdogInfo: num " << m_num << "\n";

  msg.m_data_len = 0;
  rv = Resource()->SendCommandReadLock( msg, rsp );
  if (rv != SA_OK ||  rsp.m_data[0] != eIpmiCcOk) {
      stdlog << "GetWatchdogInfo error " << rv <<
                " cc=" << rsp.m_data[0] << "\n";
      if (rv == SA_OK)
        rv = SA_ERR_HPI_INTERNAL_ERROR;
  } else {
      if ((rsp.m_data[1] & 0x40) == 0) watchdog.Running = 0;
      else watchdog.Running = 1;
      if ((rsp.m_data[1] & 0x80) == 0) watchdog.Log = 1;
      else watchdog.Log = 0;
      watchdog.TimerUse = WDTimerUse2Hpi(rsp.m_data[1] & 0x07);
      watchdog.TimerAction = WDAction2Hpi(rsp.m_data[2] & 0x07);
      watchdog.PretimerInterrupt = WDPI2Hpi(rsp.m_data[2] & 0x70);
      watchdog.PreTimeoutInterval = rsp.m_data[3] * 1000;
      watchdog.TimerUseExpFlags = rsp.m_data[4];
      watchdog.InitialCount = (rsp.m_data[5] + (rsp.m_data[6] << 8)) * 100;
      watchdog.PresentCount = (rsp.m_data[7] + (rsp.m_data[8] << 8)) * 100;
  }
  return rv;
}

SaErrorT
cIpmiWatchdog::SetWatchdogInfo( SaHpiWatchdogT &watchdog)
{
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdSetWatchdogTimer );
  cIpmiMsg  rsp;
  SaErrorT rv;
  unsigned char blog; 

  int tval = watchdog.InitialCount / 100;
  stdlog << "SetWatchdogInfo to " << watchdog.InitialCount << " msec\n";
  msg.m_data_len = 6;
  if (watchdog.Log) blog = 0;
  else blog = 0x80;  /* DontLog bit */
  if (watchdog.TimerAction != 0) blog |= 0x40;
  msg.m_data[0]  = blog | (watchdog.TimerUse & 0x07);
  msg.m_data[1]  = (watchdog.TimerAction & 0x07) |
                   ((watchdog.PretimerInterrupt & 0x07) << 4);
  msg.m_data[2]  = (watchdog.PreTimeoutInterval / 1000);
  msg.m_data[3]  = watchdog.TimerUseExpFlags;
  msg.m_data[4]  = tval & 0x00ff;
  msg.m_data[5]  = (tval & 0xff00) >> 8;
  rv = Resource()->SendCommandReadLock( msg, rsp );
  if (rv != SA_OK ||  rsp.m_data[0] != eIpmiCcOk) {
      stdlog << "SetWatchdogInfo error " << rv <<
                " cc=" << rsp.m_data[0] << "\n";
      if (rv == SA_OK)
        rv = SA_ERR_HPI_INTERNAL_ERROR;
  }

  return (rv);
}

SaErrorT
cIpmiWatchdog::ResetWatchdog()
{
  cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdResetWatchdogTimer );
  cIpmiMsg  rsp;
  SaErrorT rv;

  stdlog << "ResetWatchdog: num " << m_num << "\n";
  /* add functionality here */
  msg.m_data_len = 0;
  rv = Resource()->SendCommandReadLock( msg, rsp );
  if (rv != SA_OK ||  rsp.m_data[0] != eIpmiCcOk) {
      stdlog << "ResetWatchdog error " << rv <<
                " cc=" << rsp.m_data[0] << "\n";
      if (rv == SA_OK)
        rv = SA_ERR_HPI_INTERNAL_ERROR;
  }
  return (rv);
}
