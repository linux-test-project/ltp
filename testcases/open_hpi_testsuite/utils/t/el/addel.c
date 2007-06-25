/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      David Ashley<dashley@us.ibm.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <oh_utils.h>


#include "el_test.h"


/* note: if OH_ELTEST_MAX_ENTRIES changes we may need additional members here */
static char *data[10] = {
        "Test data one",
        "Test data two",
        "Test data three",
        "Test data four",
        "Test data five",
        "Test data six",
        "Test data seven",
        "Test data eight",
        "Test data nine",
        "Test data ten"
};


/**
* add an SaHpiUserEventT event to the EL
*
* Note that the event data we use is just a string from the above array
*/
int add_event(oh_el *el, int idx) {
        oh_el_entry *entry, *fetchentry;
        SaHpiEventT event;
        SaErrorT retc;
        SaHpiEventLogEntryIdT oldId = el->nextid, next, prev;

        if (idx >= sizeof(data) / sizeof(char *)) {
                dbg("ERROR: idx invalid.");
                return 1;
        }

        /* add a single event */
        event.Source = 1;
        event.EventType = SAHPI_ET_USER;
        event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        event.Severity = SAHPI_DEBUG;
        strcpy((char *) &event.EventDataUnion.UserEvent.UserEventData.Data, data[idx]);
        retc = oh_el_append(el, &event, NULL, NULL);

        if (retc != SA_OK) {
                dbg("ERROR: oh_el_add failed.");
                return 1;
        }
	
	entry = (oh_el_entry *)(g_list_last(el->list)->data);
        /* check correct id */
        if (entry->event.EntryId != el->nextid - 1) {
                dbg("ERROR: entry.EntryId invalid.");
                return 1;
        }

        if (entry->event.Timestamp == 0) {
                dbg("ERROR: entry.Timestamp invalid.");
                return 1;
        }

        /* inspect oh_el struct values */
        if(el->info.Enabled != TRUE) {
                dbg("ERROR: el->info.Enabled invalid.");
                return 1;
        }

//      if(el->overflow != FALSE) {
//              dbg("ERROR: el->overflow invalid.");
//              return 1;
//      }

        if(el->info.UpdateTimestamp != entry->event.Timestamp) {
                dbg("ERROR: el->info.UpdateTimestamp invalid.");
                return 1;
        }

        if(el->basetime != 0) {
                dbg("ERROR: el->basetime invalid.");
                return 1;
        }

        if(el->nextid != oldId + 1) {
                dbg("ERROR: el->nextid invalid.");
                return 1;
        }

        if(el->list == NULL) {
                dbg("ERROR: el->list == NULL.");
                return 1;
        }

        /* now fetch the event and compare it */
        retc = oh_el_get(el, entry->event.EntryId, &prev, &next, &fetchentry);
        if (retc != SA_OK) {
                dbg("ERROR: oh_el_get failed.");
                return 1;
        }

        if (entry->event.EntryId != fetchentry->event.EntryId) {
                dbg("ERROR: entry->EntryId invalid.");
                return 1;
        }

        if (prev != entry->event.EntryId - 1) {
                if (prev != SAHPI_NO_MORE_ENTRIES) {
                        dbg("ERROR: prev invalid.");
                        return 1;
                }
        }

        if (next != entry->event.EntryId + 1) {
                if (next != SAHPI_NO_MORE_ENTRIES) {
                        dbg("ERROR: next invalid.");
                        return 1;
                }
        }

        if (fetchentry->event.Timestamp == 0) {
                dbg("ERROR: fetchentry->Timestamp invalid.");
                return 1;
        }

        if (fetchentry->event.Event.Source != 1) {
                dbg("ERROR: fetchentry->Event.Source invalid.");
                return 1;
        }

        if (fetchentry->event.Event.EventType != SAHPI_ET_USER) {
                dbg("ERROR: fetchentry->Event.EventType invalid.");
                return 1;
        }

        if (fetchentry->event.Event.Timestamp != SAHPI_TIME_UNSPECIFIED) {
                dbg("ERROR: fetchentry->Event.Timestamp invalid.");
                return 1;
        }

        if (fetchentry->event.Event.Severity != SAHPI_DEBUG) {
                dbg("ERROR: fetchentry->Event.Severity invalid.");
                return 1;
        }

        if (strcmp((char *)fetchentry->event.Event.EventDataUnion.UserEvent.UserEventData.Data,
                    data[idx])) {
                dbg("ERROR: fetchentry->Event.EventDataUnion.UserEvent.UserEventData invalid.");
                return 1;
        }

        return 0;
}

