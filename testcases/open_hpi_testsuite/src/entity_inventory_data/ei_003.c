/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Kevin Gao <kevin.gao@intel.com>
 */

/* temporarily defined here. */ 
#define SA_ERR_INVENT_DATA_TRUNCATED    (SaErrorT)(SA_HPI_ERR_BASE - 1000)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hpitest.h>

#define INVENTORY_TEST_DATA	0xff

int do_inventory(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id, SaHpiRdrT rdr)
{
	SaHpiUint32T		buffer_size;
	SaHpiInventoryDataT     *data_read;
	SaHpiUint32T		actual_size;	
	SaHpiEirIdT		num;
	SaErrorT       		val;
	int            		ret = HPI_TEST_UNKNOW;

	if (rdr.RdrType == SAHPI_INVENTORY_RDR) {
		ret = HPI_TEST_PASS;
		num = rdr.RdrTypeUnion.InventoryRec.EirId;

		data_read = NULL;
		buffer_size = 0;
		val = saHpiEntityInventoryDataRead(session_id, resource_id,
				num, buffer_size, data_read, &actual_size);
		if (val != SA_ERR_INVENT_DATA_TRUNCATED) {
			printf("  Function \"saHpiEntityInventoryDataRead\" works abnormally!\n");
			printf("  Cannot check the actual size of the inventory data!");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
			
		buffer_size = actual_size - 4;
		data_read = (SaHpiInventoryDataT *) malloc(buffer_size);
		if (data_read == NULL) {
			printf("  Function \"malloc\" works abnormally!\n");
			printf("  memory allocing failed!\n");
			ret = HPI_TEST_FAIL;
			goto out;
		}
		
		val = saHpiEntityInventoryDataRead(session_id, resource_id, 
				num, buffer_size, data_read, &actual_size);
		if (val != SA_ERR_INVENT_DATA_TRUNCATED) {
			printf(" Does not conform the expected behaviors!\n"); 
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
		}
			
		free(data_read);
	}
out:
	return ret;
}

int main()
{
	SaHpiVersionT 	version;
	SaErrorT        val;
	int             ret = HPI_TEST_PASS;
	
	val = saHpiInitialize(&version);
	if (val != SA_OK) {
		printf("  Function \"saHpiInitialize\" works abnormally!\n");
		printf("  Cannot initialize HPI!\n");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
		goto out;
	}
		
	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, &do_resource, 
			&do_inventory, NULL);
	if (ret == HPI_TEST_UNKNOW) {
		printf("  No Inventory data in SAHPI_DEFAULT_DOMAIN_ID!\n");
		ret = HPI_TEST_FAIL;
	}
	
	val = saHpiFinalize();
	if (val != SA_OK) {
		printf("  Function \"saHpiFinalize\" works abnormally!\n");
		printf("  Cannot cleanup HPI");
		printf("  Return value: %s\n", get_error_string(val));
		ret = HPI_TEST_FAIL;
	}

out:	
	return ret;	
}
