/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#ifndef __OH_DOMAIN_H
#define __OH_DOMAIN_H

#define OH_DEFAULT_DOMAIN_ID 0

#ifdef __cplusplus
extern "C" {
#endif

#include <SaHpi.h>
#include <glib.h>
#include <oh_utils.h>

/*
 *  Global table of all active domains (oh_domain).
 *  Encapsulated in a struct to store a lock alongside of it.
 */
struct oh_domain_table {
        GHashTable *table;
        GList *list;
        GStaticRecMutex lock;
};

#define OH_DOMAIN_SINGLE (SaHpiUint8T)0x00
#define OH_DOMAIN_CHILD  (SaHpiUint8T)0x01
#define OH_DOMAIN_PARENT (SaHpiUint8T)0x02
#define OH_DOMAIN_PEER   (SaHpiUint8T)0x04

struct oh_dat { /* Domain Alarm Table */
        SaHpiAlarmIdT next_id;
        GSList *list;
        SaHpiUint32T update_count;
        SaHpiTimeT update_timestamp;
        SaHpiBoolT overflow;
};

struct oh_drt { /* Domain Reference Table */
        SaHpiEntryIdT next_id;
        SaHpiDomainIdT parent_id;
        GSList *list;
        SaHpiUint32T update_count;
        SaHpiTimeT update_timestamp;
};

extern struct oh_domain_table oh_domains;

/*
 * Representation of an domain
 */
struct oh_domain {
        /* id number of domain */
        SaHpiDomainIdT id;
	/* Name tag of this domain */
	SaHpiTextBufferT tag;
	/* Auto insert timeout for this domain */
	SaHpiTimeoutT ai_timeout;
        /* Domain Capabilities */
	SaHpiDomainCapabilitiesT capabilities;
        
        SaHpiGuidT guid;

        /* Resource Presence Table */
        RPTable rpt;
        /* Domain Alarm Table */
        struct oh_dat dat;
        /* Domain Reference Table */
        struct oh_drt drt;
        /* Domain Event Log */
        oh_el *del;

        /* Synchronization - used internally by domain interfaces */
        GStaticRecMutex lock;
        int refcount;
        GStaticRecMutex refcount_lock;
};

SaErrorT oh_create_domain(SaHpiDomainIdT id,
                          char *tag,
                          SaHpiDomainIdT tier_of,
                          SaHpiDomainIdT peer_of,
                          SaHpiDomainCapabilitiesT capabilities,
                          SaHpiTimeoutT ai_timeout
                         );
SaErrorT oh_destroy_domain(SaHpiDomainIdT did);
struct oh_domain *oh_get_domain(SaHpiDomainIdT did);
SaErrorT oh_release_domain(struct oh_domain *domain);
GArray *oh_query_domains(void);
SaErrorT oh_drt_entry_get(SaHpiDomainIdT did,
                          SaHpiEntryIdT entryid,
                          SaHpiEntryIdT *nextentryid,
                          SaHpiDrtEntryT *drt);

#ifdef __cplusplus
}
#endif

#endif /* __OH_DOMAIN_H */
