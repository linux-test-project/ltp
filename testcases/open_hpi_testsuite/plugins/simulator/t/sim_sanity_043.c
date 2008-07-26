/*      -*- linux-c -*-
 *
 *(C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 *	Authors:
 *     	Sean Dague <http://dague.net/sean>
*/

#include <stdlib.h>
#include <SaHpi.h>
#include <oh_utils.h>
#include <sahpi_struct_utils.h>
#include <oh_error.h>


/**
 * Run a series of sanity tests on the simulator
 * Return 0 on success, otherwise return -1
 **/


static SaHpiResourceIdT get_resid(SaHpiSessionIdT sid,
                           SaHpiEntryIdT srchid) {
        SaHpiRptEntryT res;
        SaHpiEntryIdT rptid = SAHPI_FIRST_ENTRY;

        while(saHpiRptEntryGet(sid, rptid, &rptid, &res) == SA_OK) {
                if (srchid == res.ResourceEntity.Entry[0].EntityType) {
                        return res.ResourceId;
                }
        }
        return 0;
}


int main(int argc, char **argv)
{
	SaHpiSessionIdT sid = 0;
        SaHpiEntryIdT newid;
        SaHpiIdrFieldT field;
	SaErrorT rc = SA_OK;

        rc = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL);
	if (rc != SA_OK) {
		err("Failed to open session");
                return -1;
	}
	rc = saHpiDiscover(sid);
	if (rc != SA_OK) {
		err("Failed to run discover");
                return -1;
	}

        /* get the resource id of the chassis */
        SaHpiResourceIdT resid = get_resid(sid, SAHPI_ENT_SYSTEM_CHASSIS);
        if (resid == 0) {
		err("Couldn't find the resource id of the chassis");
                return -1;
	}

        rc = saHpiIdrAreaAdd(sid, resid, 1, SAHPI_IDR_AREATYPE_PRODUCT_INFO,
                             &newid);
        if (rc == SA_ERR_HPI_READ_ONLY) {
                return 0;
        } else if (rc != SA_OK) {
		err("Couldn't add new area");
		err("Error %s",oh_lookup_error(rc));
                return -1;
	}

        field.AreaId = newid;
        field.Type = SAHPI_IDR_FIELDTYPE_PART_NUMBER;
        field.Field.DataType = SAHPI_TL_TYPE_TEXT;
        field.Field.Language = SAHPI_LANG_ENGLISH;
        field.Field.DataLength = 6;
        field.Field.Data[0] = '1';
        field.Field.Data[1] = '2';
        field.Field.Data[2] = '3';
        field.Field.Data[3] = '4';
        field.Field.Data[4] = '5';
        field.Field.Data[5] = '6';
        field.Field.Data[6] = '\0';
        rc = saHpiIdrFieldAdd(sid, resid, 1, &field);
        if (rc != SA_OK) {
		err("Couldn't add field");
		err("Error %s",oh_lookup_error(rc));
                return -1;
	}

        field.Type = SAHPI_IDR_FIELDTYPE_CUSTOM;
        rc = saHpiIdrFieldSet(sid, resid, 1, &field);
        if (rc != SA_OK) {
		err("Couldn't set field");
		err("Error %s",oh_lookup_error(rc));
                return -1;
	}

        field.AreaId = 1;
        field.FieldId = 1;
        rc = saHpiIdrFieldAdd(sid, resid, 1, &field);
        if (rc == SA_OK) {
		err("Able to set field to a read only area");
                return -1;
	}

	return 0;
}

