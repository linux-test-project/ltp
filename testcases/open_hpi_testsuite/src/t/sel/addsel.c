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
#include <sel_utils.h>


#include "sel_test.h"


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
* add an SaHpiUserEventT event to the SEL
* 
* Note that the event data we use is just a string from the above array
*/
int add_event(oh_sel *sel, int idx) {
        SaHpiSelEntryT entry, *fetchentry;
        SaHpiEventT event;
        SaErrorT retc;
        SaHpiSelEntryIdT oldId = sel->nextId, next, prev;

        if (idx >= sizeof(data) / sizeof(char *)) {
                dbg("ERROR: idx invalid.");
                return 1;
        }

        /* add a single event */
        event.Source = 1;
        event.EventType = SAHPI_ET_USER;
        event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        event.Severity = SAHPI_DEBUG;
        strcpy((char *) &event.EventDataUnion.UserEvent.UserEventData, data[idx]);
        entry.EntryId = 0;
        entry.Timestamp = 0;
        memcpy(&entry.Event, &event, sizeof(SaHpiEventT));
        retc = oh_sel_add(sel, &entry);

        if (retc != SA_OK) {
                dbg("ERROR: oh_sel_add failed.");
                return 1;
        }

        /* inspect the event struct */
        if (entry.EntryId != sel->nextId - 1) {
                dbg("ERROR: entry.EntryId invalid.");
                return 1;
        }

        if (entry.Timestamp == 0) {
                dbg("ERROR: entry.Timestamp invalid.");
                return 1;
        }
        
        /* inspect oh_sel struct values */
        if(sel->enabled != TRUE) {
                dbg("ERROR: sel->enabled invalid.");
                return 1;
        }
        
        if(sel->overflow != FALSE) {
                dbg("ERROR: sel->overflow invalid.");
                return 1;
        }
        
        if(sel->deletesupported != FALSE) {
                dbg("ERROR: sel->deletesupported invalid.");
                return 1;
        }
        
        if(sel->lastUpdate != entry.Timestamp) {
                dbg("ERROR: sel->lastUpdate invalid.");
                return 1;
        }
        
        if(sel->offset != 0) {
                dbg("ERROR: sel->offset invalid.");
                return 1;
        }
        
        if(sel->nextId != oldId + 1) {
                dbg("ERROR: sel->nextId invalid.");
                return 1;
        }
        
        if(sel->selentries == NULL) {
                dbg("ERROR: sel->selentries == NULL.");
                return 1;
        }

        /* now fetch the event and compare it */
        retc = oh_sel_get(sel, entry.EntryId, &prev, &next, &fetchentry);
        if (retc != SA_OK) {
                dbg("ERROR: oh_sel_get failed.");
                return 1;
        }

        if (entry.EntryId != fetchentry->EntryId) {
                dbg("ERROR: entry->EntryId invalid.");
                return 1;
        }

        if (prev != entry.EntryId - 1) {
                if (prev != SAHPI_NO_MORE_ENTRIES) {
                        dbg("ERROR: prev invalid.");
                        return 1;
                }
        }

        if (next != entry.EntryId + 1) {
                if (next != SAHPI_NO_MORE_ENTRIES) {
                        dbg("ERROR: next invalid.");
                        return 1;
                }
        }

        if (fetchentry->Timestamp == 0) {
                dbg("ERROR: fetchentry->Timestamp invalid.");
                return 1;
        }

        if (fetchentry->Event.Source != 1) {
                dbg("ERROR: fetchentry->Event.Source invalid.");
                return 1;
        }

        if (fetchentry->Event.EventType != SAHPI_ET_USER) {
                dbg("ERROR: fetchentry->Event.EventType invalid.");
                return 1;
        }

        if (fetchentry->Event.Timestamp != SAHPI_TIME_UNSPECIFIED) {
                dbg("ERROR: fetchentry->Event.Timestamp invalid.");
                return 1;
        }

        if (fetchentry->Event.Severity != SAHPI_DEBUG) {
                dbg("ERROR: fetchentry->Event.Severity invalid.");
                return 1;
        }

        if (strcmp((char *)fetchentry->Event.EventDataUnion.UserEvent.UserEventData,
                    data[idx])) {
                dbg("ERROR: fetchentry->Event.EventDataUnion.UserEvent.UserEventData invalid.");
                return 1;
        }

        return 0;
}

