#ifndef _UID_UTILS_
#define _UID_UTILS_
/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
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
 */

/* default location of uid map file is defined in config.h.
 * use configure --with-varpath=/usr/var/lib/openhpi
 * to change the path.
 */

/* hpi internal apis */
SaErrorT oh_uid_initialize(void); 
guint oh_uid_from_entity_path(SaHpiEntityPathT *ep); 
guint oh_uid_remove(guint uid);
guint oh_uid_lookup(SaHpiEntityPathT *ep);
guint oh_entity_path_lookup(guint *id, SaHpiEntityPathT *ep);
guint oh_uid_map_to_file(void);

/* uid to entity path cross reference (xref) data structure */ 
typedef struct {
        SaHpiResourceIdT resource_id;
        SaHpiEntityPathT entity_path;
} EP_XREF;

#endif /*_UID_UTILS_ */
