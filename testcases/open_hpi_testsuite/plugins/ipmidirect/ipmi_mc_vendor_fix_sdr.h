/*
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
 *     Pierre Sangouard <psangouard@eso-tech.com>
 */

#ifndef dIpmiMcVendorFixSdr_h
#define dIpmiMcVendorFixSdr_h


#ifndef dIpmiMcVendor_h
#include "ipmi_mc_vendor.h"
#endif

#define ENTITY_DONT_CARE    0xff

typedef struct
{
    unsigned char old_entity_id;
    unsigned char old_entity_instance;
    unsigned char new_entity_id;
    unsigned char new_entity_instance;
    bool last_entry;
} mc_sdr_patch_t;

typedef struct
{
    unsigned int manufacturer_id;
    unsigned int product_id;
    mc_sdr_patch_t  *sdr_patch;
} mc_patch_t;

extern mc_patch_t mc_patch[];


class cIpmiMcVendorFixSdr : public cIpmiMcVendor
{
  mc_sdr_patch_t *m_sdr_patch;
public:
  cIpmiMcVendorFixSdr( unsigned int manufacturer_id, unsigned int product_id );
  virtual ~cIpmiMcVendorFixSdr();

  virtual bool InitMc( cIpmiMc *mc, const cIpmiMsg &devid );
  bool ProcessSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );
};

#endif
