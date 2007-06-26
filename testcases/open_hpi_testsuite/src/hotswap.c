/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003-2006
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
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 *     Racing Guo <racing.guo@intel.com>
 *     David Judkovics <djudkovi@us.ibm.com>
 *     Renier Morales <renier@openhpi.org>
 */

#include <oh_hotswap.h>

SaHpiTimeoutT get_hotswap_auto_insert_timeout(struct oh_domain *d)
{
        return d->ai_timeout;
}

void set_hotswap_auto_insert_timeout(struct oh_domain *d,
                                     SaHpiTimeoutT to)
{
        d->ai_timeout = to;
}

/*
 * this function determines whether a hotswap transition is allowed
 */

SaHpiBoolT oh_allowed_hotswap_transition(SaHpiHsStateT from, SaHpiHsStateT to)
{
        switch(from) {
        case SAHPI_HS_STATE_INACTIVE:
                if((to == SAHPI_HS_STATE_INSERTION_PENDING) ||
                   (to == SAHPI_HS_STATE_NOT_PRESENT)) {
                        return SAHPI_TRUE;
                } else {
                        return SAHPI_FALSE;
                }
                break;
        case SAHPI_HS_STATE_INSERTION_PENDING:
                if((to == SAHPI_HS_STATE_INACTIVE) ||
                   (to == SAHPI_HS_STATE_NOT_PRESENT) ||
                   (to == SAHPI_HS_STATE_ACTIVE)) {
                        return SAHPI_TRUE;
                } else {
                        return SAHPI_FALSE;
                }
                break;
        case SAHPI_HS_STATE_ACTIVE:
                if((to == SAHPI_HS_STATE_EXTRACTION_PENDING) ||
                   to == SAHPI_HS_STATE_NOT_PRESENT) {
                        return SAHPI_TRUE;
                } else {
                        return SAHPI_FALSE;
                }
                break;
        case SAHPI_HS_STATE_EXTRACTION_PENDING:
                if((to == SAHPI_HS_STATE_ACTIVE) ||
                   (to == SAHPI_HS_STATE_NOT_PRESENT) ||
                   (to == SAHPI_HS_STATE_INACTIVE)) {
                        return SAHPI_TRUE;
                } else {
                        return SAHPI_FALSE;
                }
                break;
        case SAHPI_HS_STATE_NOT_PRESENT:
                if(to == SAHPI_HS_STATE_INSERTION_PENDING) {
                        return SAHPI_TRUE;
                } else {
                        return SAHPI_FALSE;
                }
                break;
        default:
                return SAHPI_FALSE;
        }
}
