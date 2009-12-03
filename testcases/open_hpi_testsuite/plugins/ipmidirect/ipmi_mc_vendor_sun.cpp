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

#include <string>
#include "ipmi_control_sun_led.h"
#include "ipmi_entity.h"
#include "ipmi_log.h"
#include "ipmi_mc_vendor.h"
#include "ipmi_mc_vendor_sun.h"
#include "ipmi_resource.h"
#include "ipmi_sdr.h"
#include "ipmi_text_buffer.h"
#include "SaHpi.h"

cIpmiMcVendorSunBmc::cIpmiMcVendorSunBmc(unsigned int product_id) :
    cIpmiMcVendor(0x2a, product_id, "Sun BMC")
{
}

cIpmiMcVendorSunBmc::~cIpmiMcVendorSunBmc()
{
}

bool
cIpmiMcVendorSunBmc::InitMc(cIpmiMc *mc, const cIpmiMsg &dvid)
{
  stdlog << "Sun BMC Init[" << mc->ManufacturerId() << ","
      << mc->ProductId() << "]: addr = " << mc->GetAddress() << "\n";

  mc->IsRmsBoard() = true;

  return true;
}

bool
cIpmiMcVendorSunBmc::CreateControls(cIpmiDomain *dom, cIpmiMc *mc,
    cIpmiSdrs *sdrs)
{
  cIpmiSdr *sdr;

  for (unsigned int i = 0; i < sdrs->NumSdrs(); i++)
  {
    sdr = sdrs->Sdr(i);

    if (sdr->m_type != eSdrTypeGenericDeviceLocatorRecord) continue;

    SaHpiEntityTypeT type = (SaHpiEntityTypeT) sdr->m_data[12];
    SaHpiEntityLocationT instance = (SaHpiEntityLocationT) sdr->m_data[13];
    SaHpiEntityTypeT parent_type;
    SaHpiEntityLocationT parent_instance;

    unsigned int parent_fru_id;
    parent_fru_id = sdrs->FindParentFru(type, instance, parent_type,
        parent_instance);

    cIpmiResource *res = FindResource(dom, mc, parent_fru_id, parent_type,
        parent_instance, sdrs);

    uint8_t dev_slave_addr = sdr->m_data[6];
    uint8_t dev_access_addr = sdr->m_data[5];
    uint8_t oem = sdr->m_data[14];
    uint8_t entity_id = sdr->m_data[12];
    uint8_t entity_inst = sdr->m_data[13];

    cIpmiTextBuffer tb;
    tb.SetIpmi(sdr->m_data + 15);
    char id[16];
    tb.GetAscii(id, sizeof (id));

    cIpmiControlSunLed *led = new cIpmiControlSunLed(mc, i,
        dev_access_addr, dev_slave_addr, entity_id, entity_inst, oem,
        SAHPI_FALSE);
    led->EntityPath() = res->EntityPath();
    led->IdString().SetAscii(id, SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH);
    res->AddRdr(led);
  }

  return true;
}
