/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      W. David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
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


/**
 * main: Announcement test
 *
 * This test adds one announcement to the list
 *
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv)
{
        oh_announcement *ann;
        SaHpiAnnouncementT announ;
        SaErrorT rc;

        announ.EntryId = 0;         // modified by oh_announcement_append
        announ.Timestamp = 0;       // modified by oh_announcement_append
        announ.AddedByUser = FALSE; // modified by oh_announcement_append
        announ.Severity = SAHPI_CRITICAL;
        announ.Acknowledged = FALSE;
        announ.StatusCond.Type= SAHPI_STATUS_COND_TYPE_SENSOR;
        announ.StatusCond.Entity.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD;
        announ.StatusCond.Entity.Entry[0].EntityLocation = 1;
        announ.StatusCond.Entity.Entry[1].EntityType = SAHPI_ENT_ROOT;
        announ.StatusCond.Entity.Entry[1].EntityLocation = 0;
        announ.StatusCond.DomainId = 1;
        announ.StatusCond.ResourceId = 1;
        announ.StatusCond.SensorNum = 1;
        announ.StatusCond.EventState = SAHPI_ES_UNSPECIFIED;
        announ.StatusCond.Name.Length = 5;
        memcpy(&announ.StatusCond.Name.Value,"announ", 5);
        announ.StatusCond.Mid = 123;
        /* we will not worry about the Data field for this test */

        ann = oh_announcement_create();

        rc = oh_announcement_append(ann, &announ);
	if (rc != SA_OK) {
		err("ERROR: 1 oh_announcement_append failed.");
		return 1;
	}

        announ.Severity = SAHPI_MAJOR;
        rc = oh_announcement_append(ann, &announ);
	if (rc != SA_OK) {
		err("ERROR: 2 oh_announcement_append failed.");
		return 1;
	}

        announ.Severity = SAHPI_MINOR;
        rc = oh_announcement_append(ann, &announ);
	if (rc != SA_OK) {
		err("ERROR: 3 oh_announcement_append failed.");
		return 1;
	}

	announ.Severity = SAHPI_CRITICAL;
	rc = oh_announcement_append(ann, &announ);
	if (rc != SA_OK) {
		err("ERROR: 4 oh_announcement_append failed.");
		return 1;
	}

        announ.EntryId = SAHPI_FIRST_ENTRY;
        announ.Timestamp = 0;
        rc = oh_announcement_get_next(ann, SAHPI_ALL_SEVERITIES, FALSE, &announ);
        if(rc != SA_OK) {
                err("ERROR: on_announcement_get_next returned %d.", rc);
                return 1;
        }
        err("EntryId %d returned with Severity %d.", announ.EntryId, announ.Severity);

        rc = oh_announcement_get_next(ann, SAHPI_ALL_SEVERITIES, FALSE, &announ);
        if(rc != SA_OK) {
                err("ERROR: on_announcement_get_next returned %d.", rc);
                return 1;
        }
        err("EntryId %d returned with Severity %d.", announ.EntryId, announ.Severity);

        rc = oh_announcement_get(ann, 1, &announ);
	if (rc != SA_OK) {
		err("ERROR: oh_announcement_get did not find anything.");
		return 1;
	}
	rc = oh_announcement_get_next(ann, SAHPI_CRITICAL, FALSE, &announ);
        if(rc != SA_OK || announ.Severity != SAHPI_CRITICAL) {
                err("ERROR: on_announcement_get_next returned %d."
                    " Severity returned is %d. EntryId %d.",
                    rc, announ.Severity, announ.EntryId);
                return 1;
        }
        err("EntryId %d returned.", announ.EntryId);

        return 0;
}
