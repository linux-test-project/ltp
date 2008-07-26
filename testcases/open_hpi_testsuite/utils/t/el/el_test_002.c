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
 *      Christina Hernandez<hernanc@us.ibm.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <oh_utils.h>


#include "el_test.h"


/**
 * main: EL test
 *
 * This test tests creates an EL and adds one event.
 * It then verifies there is one event and compares the
 * one event in the EL with the original event.
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        oh_el *el;
        SaErrorT retc;
	SaHpiEventT event;
        oh_el_entry *entry; 
	SaHpiEventLogEntryIdT prev, next;
	static char *data[1] = {
        	"Test data one"
	};

        /* create the EL */
        el = oh_el_create(5);

        if(el == NULL) {
                err("ERROR: el == NULL.");
                return 1;
        }

        /* add a single event */
        event.Source = 1;
        event.EventType = SAHPI_ET_USER;
        event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        event.Severity = SAHPI_DEBUG;
	strcpy((char *) &event.EventDataUnion.UserEvent.UserEventData.Data, data[0]);


        retc = oh_el_append(el, &event, NULL, NULL);
        if (retc != SA_OK) {
                err("ERROR: oh_el_append failed.");
                return 1;
        } 

	entry = (oh_el_entry *)(g_list_first(el->list)->data);
	
        if(g_list_length(el->list) != 1){
                 err("ERROR: g_list_length does not return the correct number of entries.");
                 return 1;
         }

        /* fetch the event for el*/
        retc = oh_el_get(el, entry->event.EntryId, &prev, &next, &entry);
        if (retc != SA_OK) {
                err("ERROR: oh_el_get failed.");
                return 1;
        }

	if (strcmp((char *)entry->event.Event.EventDataUnion.UserEvent.UserEventData.Data, data[0])) {
                 err("ERROR: Data from el and what was entered into el do not match");
                 return 1;
        }
        
	/* close the EL */
        retc = oh_el_close(el);
        if (retc != SA_OK) {
                err("ERROR: oh_el_close failed.");
                return 1;
        }

        return 0;
}

