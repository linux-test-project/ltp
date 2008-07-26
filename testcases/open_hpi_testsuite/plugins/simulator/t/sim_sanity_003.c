/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Sean Dague <http://dague.net/sean>
*/

#include <stdlib.h>
#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>


/**
 * Run a series of sanity tests on the simulator
 * Return 0 on success, otherwise return -1
 **/

int main(int argc, char **argv)
{
        SaHpiSessionIdT sid = 0;
        SaHpiRptEntryT res;
        SaHpiEntryIdT rptid = SAHPI_FIRST_ENTRY;
        SaHpiEntryIdT rdrid;
	SaHpiRdrT rdr;
        SaErrorT rc = SA_OK;
        int rptctr = 0;
        int rdrctr;

        rc = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL);
	if(rc != SA_OK) {
		err("Failed to open session");
                return -1;
	}

	rc = saHpiDiscover(sid);
	if(rc != SA_OK) {
		err("Failed to run discover");
                return -1;
	}


        /* loop over all resources, ensure that ResourceTag and
	 *            ManufacturerId have been set */
        while(saHpiRptEntryGet(sid, rptid, &rptid, &res) == SA_OK) {
                /* verify we have a valid rptentry */
		if(!res.ResourceTag.DataLength) {
			err("Resource Tag has zero length");
                        return -1;
		}
		if(!res.ResourceInfo.ManufacturerId) {
			err("Resource has no Manufacturer Id");
                        return -1;
		}

                /* just check for the first rdr */
                rdrid = SAHPI_FIRST_ENTRY;
                rdrctr = 0;
		while (saHpiRdrGet(sid, res.ResourceId, rdrid, &rdrid, &rdr) == SA_OK) {
                        if (rdr.RecordId == 0) {
                                err("Invalid rdr entry found");
                                return -1;
                        }
                        rdrctr++;
                }
                // note that the hot swap resource has no rdrs
                if (rdrctr == 0 &&
                    res.ResourceEntity.Entry[0].EntityType != SAHPI_ENT_DISK_DRIVE_BAY) {
                        err("No rdr entries found");
                        return -1;
                }
                err("%d rdrs found for resource %d", rdrctr, res.ResourceId);
                rptctr++;
	}
        if (rptctr == 0) {
                err("No rpt entries found");
                return -1;
        }

        return 0;
}
