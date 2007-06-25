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

#ifndef dIpmiMcVendorIntel_h
#define dIpmiMcVendorIntel_h


#ifndef dIpmiMcVendor_h
#include "ipmi_mc_vendor.h"
#endif
#ifndef dIpmiControl_h
#include "ipmi_control.h"
#endif

#define OEM_ALARM_BASE  0x10 
#define LED_POWER   0
#define LED_CRIT    1
#define LED_MAJOR   2
#define LED_MINOR   3
#define LED_IDENT   4

#define PRIVATE_BUS_ID      0x03 // w Sahalee,  the 8574 is on Private Bus 1
#define PRIVATE_BUS_ID5     0x05 // for Intel TIGI2U
#define PRIVATE_BUS_ID7     0x07 // for Intel Harbison
#define PERIPHERAL_BUS_ID   0x24 // w mBMC, the 8574 is on the Peripheral Bus
#define ALARMS_PANEL_WRITE  0x40
#define ALARMS_PANEL_READ   0x41
#define DISK_LED_WRITE      0x44 // only used for Chesnee mBMC
#define DISK_LED_READ       0x45 // only used for Chesnee mBMC
 
#define NETFN_PICMG               0x2c
#define PICMG_GET_LED_PROPERTIES  0x05
#define PICMG_SET_LED_STATE       0x07
#define PICMG_GET_LED_STATE       0x08
 

class cIpmiControlIntelRmsLed : public cIpmiControl
{
protected:

public:
  cIpmiControlIntelRmsLed( cIpmiMc *mc,
                   unsigned int num );
  virtual ~cIpmiControlIntelRmsLed();
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );
  virtual SaErrorT GetState( SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state );
  virtual SaErrorT SetState( const SaHpiCtrlModeT &mode, const SaHpiCtrlStateT &state );
  virtual void Dump( cIpmiLog &dump, const char *name ) const; 
  unsigned char m_busid;
private:
  unsigned char GetAlarms();
  int SetAlarms(unsigned char val);
  int SetIdentify(unsigned char tval);
  unsigned char GetAlarmsPicmg(unsigned char picmg_id, unsigned char fruid);
  int SetAlarmsPicmg(unsigned char picmg_id, unsigned char fruid, unsigned char val);
};

class cIpmiMcVendorIntelBmc : public cIpmiMcVendor
{
public:
  cIpmiMcVendorIntelBmc( unsigned int product_id );
  virtual ~cIpmiMcVendorIntelBmc();

  virtual bool InitMc( cIpmiMc *mc, const cIpmiMsg &devid );
  bool ProcessSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );
  bool ProcessFru( cIpmiInventory *inv, cIpmiMc *mc, unsigned int sa,
                        SaHpiEntityTypeT type);
  bool CreateControls( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );
  unsigned char m_busid;
private:

};


#endif
