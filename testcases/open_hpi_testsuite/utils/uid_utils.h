/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      David Judkovics <djudkovi@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 */

#ifndef __UID_UTILS_H
#define __UID_UTILS_H

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#include <SaHpi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Unique Resource ID utility functions */
SaErrorT oh_uid_initialize(void);
SaHpiBoolT oh_uid_is_initialized(void);
SaHpiUint32T oh_uid_from_entity_path(SaHpiEntityPathT *ep);
SaErrorT oh_uid_remove(SaHpiUint32T uid);
SaHpiUint32T oh_uid_lookup(SaHpiEntityPathT *ep);
SaErrorT oh_entity_path_lookup(SaHpiUint32T id, SaHpiEntityPathT *ep);
SaErrorT oh_uid_map_to_file(void);
#ifdef __cplusplus
}
#endif

#endif
