/*
 * Force specific code
 *
 * Copyright (c) 2004 by FORCE Computers.
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

#ifndef dIpmiMcVendorForce_h
#define dIpmiMcVendorForce_h


#ifndef dIpmiMcVendor_h
#include "ipmi_mc_vendor.h"
#endif


class cIpmiMcVendorForceShMc : public cIpmiMcVendor
{
public:
  cIpmiMcVendorForceShMc( unsigned int product_id );
  virtual ~cIpmiMcVendorForceShMc();

  virtual bool InitMc( cIpmiMc *mc, const cIpmiMsg &devid );
  bool ProcessSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );
};


#endif
