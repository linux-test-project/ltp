/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 * (C) Copyright IBM Corp. 2005-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Racing Guo <racing.guo@intel.com>
 *     Renier Morales <renier@openhpi.org>
 */

#ifndef __OH_HOTSWAP_H
#define __OH_HOTSWAP_H

#include <SaHpi.h>
#include <oh_domain.h>

#ifdef __cplusplus
extern "C" {
#endif

SaHpiTimeoutT get_hotswap_auto_insert_timeout(struct oh_domain *d);
void set_hotswap_auto_insert_timeout(struct oh_domain *d, SaHpiTimeoutT t);
SaHpiBoolT oh_allowed_hotswap_transition(SaHpiHsStateT from, SaHpiHsStateT to);


#ifdef __cplusplus
}
#endif

#endif /* __OH_HOTSWAP_H */
