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

#include <stdio.h>
#include <string.h>
#include <hpitest.h>

#define CONTROL_TEST_DATA 	2
#define CONTROL_TEST_STR	"Test Control component"
#define CONTROL_TEST_LEN	23
#define OEM_CONTROL_BODY_LENGTH	1

void value_init(SaHpiCtrlStateT *state, SaHpiCtrlTypeT type)
{
	state->Type = type;
	switch(type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			state->StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
			break;
		case SAHPI_CTRL_TYPE_DISCRETE:
			state->StateUnion.Discrete = CONTROL_TEST_DATA;
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			state->StateUnion.Analog = CONTROL_TEST_DATA;
			break;
		case SAHPI_CTRL_TYPE_STREAM:
			state->StateUnion.Stream.Repeat = SAHPI_TRUE;
			memcpy(&(state->StateUnion.Stream.Stream), 
				CONTROL_TEST_STR, 
				sizeof(state->StateUnion.Stream.Stream));
			state->StateUnion.Stream.StreamLength = 
				CONTROL_TEST_LEN;
			break;
		case SAHPI_CTRL_TYPE_TEXT:
			state->StateUnion.Text.Line = SAHPI_TLN_ALL_LINES;
			state->StateUnion.Text.Text.DataType = 
				SAHPI_TL_TYPE_BINARY;
			state->StateUnion.Text.Text.Language =
				SAHPI_LANG_ENGLISH;
			state->StateUnion.Text.Text.DataLength = 
				CONTROL_TEST_LEN;
			memcpy(&(state->StateUnion.Text.Text.Data),
				CONTROL_TEST_STR, 
				sizeof(state->StateUnion.Text.Text.Data));
			break;
		case SAHPI_CTRL_TYPE_OEM:
			state->StateUnion.Oem.MId = CONTROL_TEST_DATA;
			state->StateUnion.Oem.BodyLength = 
				OEM_CONTROL_BODY_LENGTH;
			state->StateUnion.Oem.Body[0] = CONTROL_TEST_DATA;
			break;
	}
}

int do_ctrl(SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id, SaHpiRdrT rdr)
{
	SaHpiCtrlStateT	ctrl_state, ctrl_state_new, ctrl_state_old;
	SaErrorT       	val;
	SaHpiCtrlNumT	num;
	int            	ret = HPI_TEST_UNKNOW;

	if (rdr.RdrType == SAHPI_CTRL_RDR) {
		ret = HPI_TEST_PASS;
		num = rdr.RdrTypeUnion.CtrlRec.Num; 

		val = saHpiControlStateGet(session_id, resource_id, num, 
				&ctrl_state_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the old control state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}

		memset(&ctrl_state, 0, sizeof(ctrl_state));
		value_init(&ctrl_state, ctrl_state_old.Type);
			
		val = saHpiControlStateSet(session_id, resource_id, num, 
				&ctrl_state);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the specified control state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
		
		val = saHpiControlStateGet(session_id, resource_id, num, 
				&ctrl_state_new);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot get the new control state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out1;
		}

		if (memcmp(&ctrl_state, &ctrl_state_new, sizeof(ctrl_state))) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  The new control state is invalid!\n");
			ret = HPI_TEST_FAIL;
		}
			
out1:
		val = saHpiControlStateSet(session_id, resource_id, num, 
				&ctrl_state_old);
		if (val != SA_OK) {
			printf("  Does not conform the expected behaviors!\n");
			printf("  Cannot set the old control state!\n");
			printf("  Return value: %s\n", get_error_string(val));
			ret = HPI_TEST_FAIL;
			goto out;
		}
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
		
	ret = process_domain(SAHPI_DEFAULT_DOMAIN_ID, &do_resource, &do_ctrl,
			NULL);
	if (ret == HPI_TEST_UNKNOW) {
		printf("  No Control in SAHPI_DEFAULT_DOMAIN_ID!\n");
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
