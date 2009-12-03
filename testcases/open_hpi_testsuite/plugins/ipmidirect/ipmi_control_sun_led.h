/*
 * Copyright (c) 2009 by Sun Microsystems, Inc.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Chris Rinaldo <chris.rinaldo@sun.com>
 */

#ifndef dIpmiControlSunLed_h
#define dIpmiControlSunLed_h

#include <stdint.h>
#include "ipmi_control.h"
#include "ipmi_log.h"
#include "ipmi_mc.h"
#include "SaHpi.h"

class cIpmiControlSunLed : public cIpmiControl
{
protected:
  uint8_t m_dev_access_addr;
  uint8_t m_dev_slave_addr;
  uint8_t m_entity_id;
  uint8_t m_entity_inst;
  uint8_t m_oem;
  SaHpiBoolT m_read_only;

public:
  cIpmiControlSunLed(cIpmiMc *mc, unsigned int num, uint8_t dev_access_addr,
      uint8_t dev_slave_addr, uint8_t entity_id, uint8_t entity_inst,
      uint8_t oem, SaHpiBoolT read_only);
  virtual ~cIpmiControlSunLed();

  virtual bool CreateRdr(SaHpiRptEntryT &resource, SaHpiRdrT &rdr);

  virtual SaErrorT SetState(const SaHpiCtrlModeT &mode,
          const SaHpiCtrlStateT &state);
  virtual SaErrorT GetState(SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state);

  virtual void Dump(cIpmiLog &dump, const char *name) const;

private:
  static const tIpmiCmd eIpmiCmdSunOemLedGet = static_cast<tIpmiCmd> (0x21);
  static const tIpmiCmd eIpmiCmdSunOemLedSet = static_cast<tIpmiCmd> (0x22);

  enum tLedState
  {
    eLedStateOff,
    eLedStateOn,
    eLedStateStandByBlink,
    eLedStateSlowBlink,
    eLedStateFastBlink
  };
};

#endif

