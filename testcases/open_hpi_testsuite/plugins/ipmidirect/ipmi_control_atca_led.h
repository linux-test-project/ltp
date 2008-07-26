/*
 * ipmi_control_atca_led.h
 *
 * Copyright (c) 2006 by ESO Technologies.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */


#ifndef dIpmiControlAtcaLed_h
#define dIpmiControlAtcaLed_h


#ifndef dIpmiControl_h
#include "ipmi_control.h"
#endif

class cIpmiControlAtcaLed : public cIpmiControl
{
protected:
  unsigned int m_num;
  unsigned char m_led_color_capabilities;
  unsigned char m_led_default_local_color;
  unsigned char m_led_local_color;
  unsigned char m_led_default_override_color;
  unsigned char m_led_override_color;
  bool m_set_led_state_supported;

public:
  cIpmiControlAtcaLed( cIpmiMc *mc,
                       unsigned int num,
                       unsigned char led_color_capabilities,
                       unsigned char led_color_local_control_state,
                       unsigned char led_color_override_state);

  virtual ~cIpmiControlAtcaLed();

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  // virtual void Log();

  virtual SaErrorT SetState( const SaHpiCtrlModeT &mode, const SaHpiCtrlStateT &state );
  virtual SaErrorT GetState( SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state );

  virtual void Dump( cIpmiLog &dump, const char *name ) const;

protected:
  bool IsSupportedColor(AtcaHpiLedColorT hpi_color);
};


#endif
