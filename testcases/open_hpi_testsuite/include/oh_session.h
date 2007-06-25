/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
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

#ifndef __OH_SESSION_H
#define __OH_SESSION_H

#include <SaHpi.h>
#include <oh_event.h>
#include <glib.h>

/*
 *  Global table of all active sessions (oh_session).  This table is
 *  populated and depopulated by calls to saHpiSessionOpen() and
 *  saHpiSessionClose(). The table has been encapsulated to have a lock
 *  alongside of it.
 */
struct oh_session_table {
        GHashTable *table;
        GSList *list;
        GStaticRecMutex lock;
};

extern struct oh_session_table oh_sessions;

/*
 * Representation of an HPI session
 */
struct oh_session {
        /*
          Session ID as returned by saHpiSessionOpen()
        */
        SaHpiSessionIdT id;

        /*
          A session is always associated with exactly one domain
        */
        SaHpiDomainIdT did;

        SaHpiBoolT subscribed;

        /* Initialized to false. Will be set to true*/
        SaHpiEvtQueueStatusT eventq_status;

        /*
          Even if multiple sessions are opened for the same domain,
          each session could receive different events depending on what
          events the caller signs up for.

          This is the session specific event queue
        */
        GAsyncQueue *eventq;

};

SaHpiSessionIdT oh_create_session(SaHpiDomainIdT did);
SaHpiDomainIdT oh_get_session_domain(SaHpiSessionIdT sid);
GArray *oh_list_sessions(SaHpiDomainIdT did);
SaErrorT oh_get_session_subscription(SaHpiSessionIdT sid, SaHpiBoolT *state);
SaErrorT oh_set_session_subscription(SaHpiSessionIdT sid, SaHpiBoolT state);
SaErrorT oh_queue_session_event(SaHpiSessionIdT sid, struct oh_event *event);
SaErrorT oh_dequeue_session_event(SaHpiSessionIdT sid,
                                  SaHpiTimeoutT timeout,
                                  struct oh_event *event,
                                  SaHpiEvtQueueStatusT *eventq_status);
SaErrorT oh_destroy_session(SaHpiSessionIdT sid);
SaErrorT oh_destroy_domain_sessions(SaHpiDomainIdT did);

#endif /* __OH_SESSION_H */

