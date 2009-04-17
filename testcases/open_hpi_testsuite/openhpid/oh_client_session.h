/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      W. David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 *
 */

#ifndef __OH_CLIENT_SESSION_H
#define __OH_CLIENT_SESSION_H

#include "strmsock.h"

extern "C"
{
#include <glib.h>
#include <SaHpi.h>
}

struct oh_client_session {
        SaHpiDomainIdT did; /* Domain Id */
        SaHpiSessionIdT csid; /* Client Session Id */
        SaHpiSessionIdT dsid; /* Domain Session Id */        
        GHashTable *connxs; /* Connections for this session (per thread) */
};

extern GHashTable *ohd_domains;
extern GHashTable *ohd_sessions;
extern GStaticRecMutex ohd_sessions_sem;

SaErrorT oh_create_connx(SaHpiDomainIdT, pcstrmsock *);
void oh_delete_connx(pcstrmsock);
SaErrorT oh_close_connx(SaHpiSessionIdT);
SaErrorT oh_get_connx(SaHpiSessionIdT, SaHpiSessionIdT *, pcstrmsock *, SaHpiDomainIdT *);

SaHpiSessionIdT oh_open_session(SaHpiDomainIdT, SaHpiSessionIdT, pcstrmsock);
SaErrorT oh_close_session(SaHpiSessionIdT);

#endif /* __OH_CLIENT_SESSION_H */
