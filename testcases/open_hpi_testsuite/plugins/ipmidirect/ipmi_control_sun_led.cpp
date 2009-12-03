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

#include "ipmi_control_sun_led.h"
#include "ipmi_resource.h"

cIpmiControlSunLed::cIpmiControlSunLed(cIpmiMc* mc, unsigned int num,
    uint8_t dev_access_addr, uint8_t dev_slave_addr, uint8_t entity_id,
    uint8_t entity_inst, uint8_t oem, SaHpiBoolT read_only) :
    cIpmiControl(mc, num, SAHPI_CTRL_LED, SAHPI_CTRL_TYPE_OEM),
    m_dev_access_addr(dev_access_addr), m_dev_slave_addr(dev_slave_addr),
    m_entity_id(entity_id), m_entity_inst(entity_inst), m_oem(oem),
    m_read_only(read_only)
{
}

cIpmiControlSunLed::~cIpmiControlSunLed()
{
}

bool
cIpmiControlSunLed::CreateRdr(SaHpiRptEntryT& resource, SaHpiRdrT& rdr)
{
  if (cIpmiControl::CreateRdr(resource, rdr) == false)
    return false;

  rdr.RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_LED;
  rdr.RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_OEM;
  rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem.MId = 0x2a;
  rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem.ConfigData[0] = m_oem;
  rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.MId = 0x2a;
  rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.BodyLength = 1;
  rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default.Body[0] = 0;
  rdr.RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
  rdr.RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = m_read_only;
  rdr.RdrTypeUnion.CtrlRec.WriteOnly = SAHPI_FALSE;

  return true;
}

SaErrorT
cIpmiControlSunLed::GetState(SaHpiCtrlModeT& mode, SaHpiCtrlStateT& state)
{
  mode = SAHPI_CTRL_MODE_AUTO;

  state.Type = SAHPI_CTRL_TYPE_OEM;

  cIpmiMsg ledmsg(eIpmiNetfnOem, eIpmiCmdSunOemLedGet);
  ledmsg.m_data[0] = m_dev_slave_addr;
  ledmsg.m_data[1] = m_oem;
  ledmsg.m_data[2] = m_dev_access_addr;
  ledmsg.m_data[3] = m_oem;
  ledmsg.m_data[4] = m_entity_id;
  ledmsg.m_data[5] = m_entity_inst;
  ledmsg.m_data[6] = 0;
  ledmsg.m_data_len = 7;

  cIpmiMsg ledrsp;
  SaErrorT rv = Resource()->SendCommandReadLock(this, ledmsg, ledrsp);

  if (rv != SA_OK)
    return rv;

  if (ledrsp.m_data_len != 2 || ledrsp.m_data[0] != eIpmiCcOk)
    return SA_ERR_HPI_ERROR;

  state.StateUnion.Oem.MId = 0x2a;
  state.StateUnion.Oem.BodyLength = 1;
  state.StateUnion.Oem.Body[0] = ledrsp.m_data[1];

  return SA_OK;
}

SaErrorT
cIpmiControlSunLed::SetState(const SaHpiCtrlModeT& mode,
	const SaHpiCtrlStateT& state)
{
  if (state.StateUnion.Oem.Body[0] > eLedStateFastBlink)
  {
    return SA_ERR_HPI_INVALID_DATA;
  }

  cIpmiMsg ledmsg(eIpmiNetfnOem, eIpmiCmdSunOemLedSet);
  ledmsg.m_data[0] = m_dev_slave_addr;
  ledmsg.m_data[1] = m_oem;
  ledmsg.m_data[2] = m_dev_access_addr;
  ledmsg.m_data[3] = m_oem;
  ledmsg.m_data[4] = state.StateUnion.Oem.Body[0]; // mode
  ledmsg.m_data[5] = m_entity_id;
  ledmsg.m_data[6] = m_entity_inst;
  ledmsg.m_data[7] = 0; // force
  ledmsg.m_data[8] = 0; // role
  ledmsg.m_data_len = 9;

  cIpmiMsg ledrsp;
  SaErrorT rv = Resource()->SendCommandReadLock(this, ledmsg, ledrsp);

  if (rv != SA_OK)
    return rv;

  if (ledrsp.m_data[0] == eIpmiCcInvalidCmd)
    return SA_ERR_HPI_UNSUPPORTED_PARAMS;

  if (ledrsp.m_data[0] == eIpmiCcInsufficientPrivilege)
    return SA_ERR_HPI_READ_ONLY;

  if (ledrsp.m_data[0] != eIpmiCcOk)
    return SA_ERR_HPI_ERROR;

  return SA_OK;
}

void
cIpmiControlSunLed::Dump(cIpmiLog& dump, const char* name) const
{
  dump.Begin("Sun LED", name);
  dump.End();
}
