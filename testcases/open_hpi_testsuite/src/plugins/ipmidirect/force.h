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

#ifndef dForce_h
#define dForce_h


#ifndef dIpmiDomain_h
#include "ipmi_domain.h"
#endif


int ForceShMcSetup( cIpmiDomain *domain,const cIpmiMsg &devid,
                    unsigned int manufacturer_id,
                    unsigned int product_id );


#endif
