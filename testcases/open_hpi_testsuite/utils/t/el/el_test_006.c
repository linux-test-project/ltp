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
#include <el_utils.h>


#include "el_test.h"

/**
 * main: EL test
 *
 * This test creates a new EL, adds 5 entries and saves the EL
 * to a file. It then retrieves the EL and adds 5 entries to both
 * the initial and retrieved ELs. It then checks the number of entries 
 * and compares the two ELs.
 *
 * Return value: 0 on success, 1 on failure
 **/


int main(int argc, char **argv)
{
        oh_el *el, *el2;
	int x;
        SaErrorT retc, retc1, retc2;
	SaHpiEventT event;
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



	/* create a new EL of size 20*/
	el = oh_el_create(20);
	el2 = oh_el_create(20);

	/* add 5 events to el */
	for(x=0;x<5;x++){
        	event.Source = 1;
        	event.EventType = SAHPI_ET_USER;
        	event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        	event.Severity = SAHPI_DEBUG;
		strcpy((char *)event.EventDataUnion.UserEvent.UserEventData.Data, data[x]);

        	retc = oh_el_append(el, &event, NULL, NULL);
        	if (retc != SA_OK) {
              	  err("ERROR: oh_el_append failed.");
               	  return 1;
        	}       
	}


        /* save the EL to file */
        retc1 = oh_el_map_to_file(el, "./elTest.data");
        if (retc1 != SA_OK) {
                err("ERROR: oh_el_map_to_file failed.");
                return 1;
        }


        /* get EL from file (el2) */
        retc2 = oh_el_map_from_file(el2, "./elTest.data");
        if (retc2 != SA_OK) {
                err("ERROR: oh_el_map_from_file failed.");
                return 1;
        }

	/* add 5 more events to el */
	for(x=5;x<10;x++){
        	event.Source = 1;
        	event.EventType = SAHPI_ET_USER;
        	event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        	event.Severity = SAHPI_DEBUG;
		strcpy((char *)event.EventDataUnion.UserEvent.UserEventData.Data, data[x]);

        	retc = oh_el_append(el, &event, NULL, NULL);
        	if (retc != SA_OK) {
              	  err("ERROR: oh_el_append failed.");
               	  return 1;
        	}       
	}

	/* add 5 more events to el2 */
	for(x=5;x<10;x++){
        	event.Source = 1;
        	event.EventType = SAHPI_ET_USER;
        	event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        	event.Severity = SAHPI_DEBUG;
		strcpy((char *)event.EventDataUnion.UserEvent.UserEventData.Data, data[x]);

        	retc = oh_el_append(el2, &event, NULL, NULL);
        	if (retc != SA_OK) {
              	  err("ERROR: oh_el_append failed.");
               	  return 1;
        	}       
	}


	/* verify number of entries in el and el2 is 10 */
        if(g_list_length(el->list) != 10) {
                 err("ERROR: el->list does not have the correct number of entries");
                 return 1;
         }

        if(g_list_length(el2->list) != 10) {
                 err("ERROR: el2->list does not have the correct number of entries");
                 return 1;
         }
 
	/* compare entry contents of el and el2 */
	retc = el_compare(el,el2);
	if (retc != SA_OK){
  		err("ERROR: el and el2 do not have matching entries.");
		return 1;
	}

        /* close el */
        retc1 = oh_el_close(el);
        if (retc1 != SA_OK) {
                err("ERROR: oh_el_close on el failed.");
                return 1;
        }

        /* close el2 */
        retc2 = oh_el_close(el2);
        if (retc2 != SA_OK) {
                err("ERROR: oh_el_close on el2 failed.");
                return 1;
        }

        return 0;
}

