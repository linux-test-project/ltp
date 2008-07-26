#include <glib.h>
#include <string.h>
 
#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>
 
#include "el_test.h"
 
 /* function to compare two event logs */
 
int el_compare(oh_el *el1, oh_el *el2)
{
        oh_el_entry *entry1, *entry2; 
	SaHpiEventLogEntryIdT prev1, prev2, next1, next2, cur1, cur2;
 	SaErrorT retc;
 
        if (g_list_length(el1->list) != g_list_length(el2->list)) {
        	err("ERROR: el1->list != el2->list.");
        	return 1;
        }

	if ((g_list_length(el1->list) == 0) &&
	    (g_list_length(el2->list) == 0)) {
		return 0;
	}

        next1 = SAHPI_OLDEST_ENTRY;
	next2 = SAHPI_OLDEST_ENTRY;
	while (next1 != SAHPI_NO_MORE_ENTRIES) {
		cur1 = next1;
		cur2 = next2;
        
		/* fetch the event for el1*/
		retc = oh_el_get(el1, cur1, &prev1, &next1, &entry1);
		if (retc != SA_OK) {
			err("ERROR: oh_el_get failed.");
                	return 1;
		}
 
		/* fetch the event for el2*/
		retc = oh_el_get(el2, cur2, &prev2, &next2, &entry2);
		if (retc != SA_OK) {
			err("ERROR: oh_el_get failed.");
			return 1;
		}

		if (memcmp(&entry1->event.Event, &entry2->event.Event, sizeof(SaHpiEventT))) {
			err("ERROR: Data from el1 and el2 do not match");
			return 1;
		}
   
		/* Compare resource from el1 and el2 */
		if (memcmp(&entry1->res, &entry2->res, sizeof(SaHpiRptEntryT))) {
                	err("ERROR: Res from el1 and el2 do not match.");
                	return 1;
		}
 
		/* Compare rdr from el1 and el2 */
		if (memcmp(&entry1->rdr, &entry2->rdr, sizeof(SaHpiRdrT))) {
                	err("ERROR: Rdr from el1 and el2 do not match.");
                	return 1;
		}
        }

        return 0;
}
 
