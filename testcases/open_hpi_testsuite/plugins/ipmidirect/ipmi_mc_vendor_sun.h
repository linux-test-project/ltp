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

#ifndef dIpmiMcVendorSun_h
#define dIpmiMcVendorSun_h

#include "ipmi_control.h"
#include "ipmi_mc.h"
#include "ipmi_mc_vendor.h"

class cIpmiMcVendorSunBmc : public cIpmiMcVendor
{
public:
  cIpmiMcVendorSunBmc(unsigned int product_id);
  virtual ~cIpmiMcVendorSunBmc();

  virtual bool InitMc(cIpmiMc *mc, const cIpmiMsg &devid);
  bool CreateControls(cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs);
};

#endif
