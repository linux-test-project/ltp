/*
 * Intel specific code
 *
 * Copyright (c) 2004-2006 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Andy Cress <arcress@users.sourceforge.net>
 */

#include "ipmi_mc_vendor_intel.h"
#include "ipmi_utils.h"
#include "ipmi_log.h"
#include "ipmi_mc.h"
#include "ipmi_domain.h" 
// #include <errno.h>

#define  HSC_SA    0xc0  /*slave address of HSC mc*/
int g_enableHSC = 0;     /* flag to detect whether an HSC is present */

/*---------------------------------------
 *  cIpmiMcVendorIntelBmc object 
 *---------------------------------------*/

cIpmiMcVendorIntelBmc::cIpmiMcVendorIntelBmc( unsigned int product_id )
  : cIpmiMcVendor( 0x000157, product_id, "Intel BMC" )
{    /* instantiate the cIpmiMcVendorIntelBmc */
}


cIpmiMcVendorIntelBmc::~cIpmiMcVendorIntelBmc()
{
}

bool
cIpmiMcVendorIntelBmc::InitMc( cIpmiMc *mc, const cIpmiMsg &devid )
{
  stdlog << "Intel InitMc[" << mc->ManufacturerId() << "," 
         << mc->ProductId() << "]: addr = " << mc->GetAddress() << "\n"; 

  /* Set the m_busid for Leds */
  switch(mc->ProductId())
  {
     case 0x0022: m_busid = PRIVATE_BUS_ID5; break;   /*TIGI2U*/
     case 0x001B: m_busid = PRIVATE_BUS_ID; break;    /*TIGPR2U*/
     case 0x4311: m_busid = PERIPHERAL_BUS_ID; break; /*TIGPT1U mBMC*/
     case 0x0026:
     case 0x0028:
     case 0x0811: m_busid = PRIVATE_BUS_ID7; break;   /*TIGW1U*/
     case 0x0900:   /*TIGPR2U HSC*/
     case 0x0911:   /*TIGI2U HSC*/
     case 0x0A0C:   /*TIGW1U HSC*/
     default: 
              m_busid = PRIVATE_BUS_ID; break;  
  }

  if (mc->IsTcaMc()) return true;
  /* 
   * If here, the MC has (manuf_id == 0x000157) Intel, and  
   * product_id == one of these: { 0x000C, 0x001B, 0x0022, 0x4311, 
   *			           0x0100, 0x0026, 0x0028, 0x0811 }; 
   * These return GetDeviceID with ProvidesDeviceSdrs() == true, and
   * use the SDR Repository. 
   */
  mc->SetProvidesDeviceSdrs(false);
  mc->IsRmsBoard() = true;

  /*
   * The FRUSDR should be set at the factory, so don't change it here.
   * Don't clear the SEL here either. 
   */
  return true;
}


bool
cIpmiMcVendorIntelBmc::CreateControls(cIpmiDomain *dom,  cIpmiMc * mc, 
                                         cIpmiSdrs * sdrs )
{
  const char *name;
  char dstr[80];
  int i;

  if (mc->IsTcaMc()) return true;

  for ( int j = 0; j < mc->NumResources(); j++ )
  {
    cIpmiResource *res  = mc->GetResource( j );
    if ( res == 0 ) continue;
    /* Note that the RPT has not been Populated yet */

    if (res->FruId() == 0) 
    {
      /* Create the alarm LED RDRs for the baseboard */
      for (i = 0; i <= LED_IDENT; i++) 
      {
        cIpmiControlIntelRmsLed *led = new cIpmiControlIntelRmsLed( mc, i);
        led->EntityPath() = res->EntityPath();
        switch (i) { 
           case LED_POWER:  name = "Power Alarm LED"; break;
           case LED_CRIT:  name = "Critical Alarm LED"; break;
           case LED_MAJOR:  name = "Major Alarm LED"; break;
           case LED_MINOR:  name = "Minor Alarm LED"; break;
           case LED_IDENT:  name = "Chassis Identify LED"; break;
           default: snprintf(dstr,sizeof(dstr),"Control LED %d",i);
                    name = dstr;  
                    break;
        }
        led->IdString().SetAscii(name, SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH);
        res->AddRdr( led );
        led->m_busid = m_busid;
      }
      break;
    }
  }

  return true;
}

bool
cIpmiMcVendorIntelBmc::ProcessSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs )
{
  if ( mc->GetAddress() != 0x20 ) {
       stdlog << "Intel MC " << mc->GetAddress() << " skipped\n";
       return true;
  }
  stdlog << "Intel MC " << mc->GetAddress() << ", ProcessSdr\n";

  /* Cannot enable Watchdog here because RPT is not built yet. */
  
  /* 
   * Sort through the SDR DeviceLocatorRecords and handle them.
   * e.g.: slave address 0xc0 (HSC) or 0x28 (IPMB Bridge), 
   */
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       switch( sdr->m_type )
          {
            case eSdrTypeMcDeviceLocatorRecord:
		 stdlog << "Intel SDR[" << i << "] Locator " << sdr->m_data[5] << "\n";
                 if (sdr->m_data[5] == HSC_SA) g_enableHSC = 1;
                 break;

            default:
                 break;
          }
     }

  return true;
}

bool
cIpmiMcVendorIntelBmc::ProcessFru( cIpmiInventory *inv, cIpmiMc *mc, unsigned int sa, SaHpiEntityTypeT type )
{
  stdlog << "ProcessFru: Intel MC " << sa << " enableHSC " 
	   << g_enableHSC << "\n";

  if (mc->IsTcaMc()) return true;

  if (type == SAHPI_ENT_SYSTEM_BOARD) {
      cIpmiResource *res  = inv->Resource();
      stdlog << "ProcessFru: found " << inv->IdString() << 
             " id " << res->m_resource_id << "\n"; 
      /* RPTs are not built yet, so we can't enable RESET here */
      /* see ipmi_resource.cpp for that. */
  } 
  else if ((sa != mc->GetAddress()) && (type != SAHPI_ENT_SYSTEM_BOARD)) 
  {
      /* g_enableHSC == 1 */
      stdlog << "ProcessFru: " << inv->IdString() <<
              " setting addr " << mc->GetAddress() << " to "
                  << sa << " type " << type << "\n";
      cIpmiAddr addr(eIpmiAddrTypeIpmb,mc->GetChannel(),0,sa);
      inv->SetAddr(addr);
  }
  return true;
}

/*---------------------------------------
 *  cIpmiControlIntelRmsLed object 
 *---------------------------------------*/

cIpmiControlIntelRmsLed::cIpmiControlIntelRmsLed( cIpmiMc *mc,
                                  unsigned int num)
     : cIpmiControl( mc, num, SAHPI_CTRL_LED, SAHPI_CTRL_TYPE_DIGITAL )
{
}

cIpmiControlIntelRmsLed::~cIpmiControlIntelRmsLed()
{
}

bool
cIpmiControlIntelRmsLed::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  int n;
  if ( cIpmiControl::CreateRdr( resource, rdr ) == false )
       return false;
 
  n = rdr.RdrTypeUnion.CtrlRec.Num;
  // rdr->RdrTypeUnion.CtrlRec.Num    = n;
  rdr.RdrTypeUnion.CtrlRec.Oem          = OEM_ALARM_BASE + n;
  rdr.RdrTypeUnion.CtrlRec.Type         = SAHPI_CTRL_TYPE_DIGITAL;
  rdr.RdrTypeUnion.CtrlRec.OutputType   = SAHPI_CTRL_LED;
  if (n == LED_IDENT)    /* Identify LED */
     rdr.RdrTypeUnion.CtrlRec.WriteOnly    = SAHPI_TRUE;
  else
     rdr.RdrTypeUnion.CtrlRec.WriteOnly    = SAHPI_FALSE;
  stdlog << "Intel:CreateRdr(Led): num = " << n << 
            " oem_num = " << rdr.RdrTypeUnion.CtrlRec.Oem << "\n";
  return true;
}

SaErrorT
cIpmiControlIntelRmsLed::GetState( SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state )
{
  unsigned char mask = 0x01;
  int i;
  SaErrorT rv = SA_OK;

  /*TODO: add GetAlarmsPicmg() if ATCA */
  int idx = m_num; // m_oem - OEM_ALARM_BASE; 
  if (idx == LED_IDENT) {  /* Identify LED */
     mode = SAHPI_CTRL_MODE_MANUAL;
     state.Type = SAHPI_CTRL_TYPE_DIGITAL;
     state.StateUnion.Digital = SAHPI_CTRL_STATE_OFF; 
     return rv;
  }
  unsigned char val = GetAlarms();
  mode = SAHPI_CTRL_MODE_MANUAL; /* or SAHPI_CTRL_MODE_AUTO */
  state.Type = SAHPI_CTRL_TYPE_DIGITAL;
  for (i = 0; i < idx; i++) mask = mask << 1;
  if ((val & mask) == 0) 
        state.StateUnion.Digital = SAHPI_CTRL_STATE_ON; 
  else
        state.StateUnion.Digital = SAHPI_CTRL_STATE_OFF; 
  stdlog << "Led:GetState(" << idx << "): mode = " << mode << 
            " state = " << state.StateUnion.Digital << "\n";
  return rv;
}

SaErrorT
cIpmiControlIntelRmsLed::SetState( const SaHpiCtrlModeT &mode, const SaHpiCtrlStateT &state )
{
  static unsigned char id_time = 20;  /*id_time = 20 seconds*/
  unsigned char mask = 0x01;
  unsigned char val, newval;
  int i;
  SaErrorT rv = SA_OK;

  /*TODO: add SetAlarmsPicmg() if ATCA */
  int idx = m_num; // m_oem - OEM_ALARM_BASE; 
  if (idx == LED_IDENT) {  /* Identify LED */
     rv = SetIdentify(id_time);  /* turn ID on for id_time seconds */
     return rv;
  }
  val = GetAlarms();
  for (i = 0; i < idx; i++) mask = mask << 1;
  if (state.StateUnion.Digital == SAHPI_CTRL_STATE_ON) {
       mask = ~mask; /*NOT*/
       newval = val & mask;
  } else {
       newval = val | mask;
  }
  rv = SetAlarms(newval);
  stdlog << "Led:SetAlarms(" << idx << ") " << "state = " <<
            state.StateUnion.Digital << " rv = " << rv  << "\n";
  return rv;
}

void
cIpmiControlIntelRmsLed::Dump( cIpmiLog &dump, const char *name ) const
{
  dump.Begin( "LedControl", name );
  dump.End();
}

unsigned char
cIpmiControlIntelRmsLed::GetAlarms( void )
{
  cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdMasterReadWrite );
  msg.m_data[0] = m_busid;
  msg.m_data[1] = ALARMS_PANEL_READ;
  msg.m_data[2] = 0x01;
  msg.m_data_len = 3;
  cIpmiMsg rsp;

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );
  if (rv != SA_OK) return(0);
  // uchar cc = rsp.m_data[0]; 
  return(rsp.m_data[1]);
}

unsigned char
cIpmiControlIntelRmsLed::GetAlarmsPicmg( unsigned char picmg_id, unsigned char fruid)
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetFruLedState );
  cIpmiMsg rsp;
  msg.m_data[0] = picmg_id;
  msg.m_data[1] = fruid;
  msg.m_data[2] = 0;  /* blue LED */
  msg.m_data_len = 3;
  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );
  if (rv == 0 && rsp.m_data[0] != 0) rv = rsp.m_data[0];  /*comp code*/
  if (rv != 0) {
      stdlog << "GetAlarmsPicmg error rv = " << rv << "\n";
      return(0);
  }
  return(rsp.m_data[6]);  /*status byte*/
}

int
cIpmiControlIntelRmsLed::SetAlarmsPicmg( unsigned char picmg_id, unsigned char fruid,
                                 unsigned char val)
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruLedState );
  cIpmiMsg rsp;
  msg.m_data[0] = picmg_id;
  msg.m_data[1] = fruid;
  msg.m_data[2] = 0;  /* blue LED */
  msg.m_data[3] = val;
  msg.m_data[4] = 0; 
  msg.m_data[5] = 1;  /* color blue */
  msg.m_data_len = 6;
  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );
  if (rv != 0) return(rv);
  if (rsp.m_data[0] != 0) rv = rsp.m_data[0];  /*comp code*/
  return(rv);
}

int
cIpmiControlIntelRmsLed::SetAlarms( unsigned char value)
{
  cIpmiMsg msg( eIpmiNetfnApp, eIpmiCmdMasterReadWrite );
  msg.m_data[0] = m_busid;
  msg.m_data[1] = ALARMS_PANEL_WRITE;
  msg.m_data[2] = 0x01;
  msg.m_data[3] = value;
  msg.m_data_len = 4;
  cIpmiMsg rsp;

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );
  if (rv != 0) return(rv);
  if (rsp.m_data[0] != 0) rv = rsp.m_data[0];  /*comp code*/
  return(rv);
}

int
cIpmiControlIntelRmsLed::SetIdentify( unsigned char tval)
{
  cIpmiMsg msg( eIpmiNetfnChassis, eIpmiCmdChassisIdentify );
  msg.m_data[0] = tval; /*num seconds*/
  msg.m_data_len = 1;
  cIpmiMsg rsp;

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );
  if (rv != 0) return(rv);
  if (rsp.m_data[0] != 0) rv = rsp.m_data[0];  /*comp code*/
  return(rv);
}

/*end of ipmi_mc_vendor_intel.cpp */
