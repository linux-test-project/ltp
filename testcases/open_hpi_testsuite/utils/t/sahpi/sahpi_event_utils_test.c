/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees.ibm.com> 
 */

#include <stdio.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

#define BAD_CAT -1
#define BAD_TYPE -1

int main(int argc, char **argv) 
{
        const char *expected_str;
        SaErrorT err, expected_err;
        SaHpiEventStateT event_state, expected_state;
        SaHpiEventCategoryT event_cat, expected_cat;
        SaHpiTextBufferT buffer;

	/******************************** 
	 * oh_decode_eventstate testcases
         ********************************/

	/* oh_decode_eventstate: Bad event category testcase */
	{
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
	
		err = oh_decode_eventstate(SAHPI_ES_UNSPECIFIED, BAD_CAT, &buffer);
		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

	/* oh_decode_eventstate: NULL buffer testcase */
	{
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		
		err = oh_decode_eventstate(SAHPI_ES_UNSPECIFIED, SAHPI_EC_UNSPECIFIED, 0);
		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

	/* oh_decode_eventstate: print UNSPECIFIED testcase */
	{
		event_state = SAHPI_ES_UNSPECIFIED;
		event_cat = SAHPI_EC_GENERIC;
		expected_str = "UNSPECIFIED";
		
		err = oh_decode_eventstate(event_state, event_cat, &buffer);
		if (err != SA_OK) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

                if (strcmp(expected_str, (char *)buffer.Data)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        printf("  Received string=%s; Expected string=%s\n", buffer.Data, expected_str);
                        return -1;             
                }
	}

	/* oh_decode_eventstate: strip extra UNSPECIFIED testcase */
	{
		event_state = SAHPI_ES_STATE_01 | SAHPI_ES_STATE_03 | SAHPI_ES_UNSPECIFIED;
		expected_str = "STATE_01 | STATE_03";

		err = oh_decode_eventstate(event_state, event_cat, &buffer);
		if (err != SA_OK) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
			return -1;
		}

                if (strcmp(expected_str, (char *)buffer.Data)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        printf("  Received string=%s; Expected string=%s\n", buffer.Data, expected_str);
			return -1;             
                }
	}

	/******************************** 
	 * oh_encode_eventstate testcases
         ********************************/

        /* oh_encode_eventstate testcases - NULL parameters testcase */
        {
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
  
                err = oh_encode_eventstate(0, 0, 0);
		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

        /* oh_encode_eventstate testcases - No Data testcase */
        {
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
		buffer.Data[0] = 0x00;

                err = oh_encode_eventstate(&buffer, &event_state, &event_cat);

		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

        /* oh_encode_eventstate testcases - handle blanks testcase */
        {
		strcpy((char *)buffer.Data, "  LOWER_MINOR  |  LOWER_MAJOR|LOWER_CRIT ");
		buffer.DataLength = strlen("  LOWER_MINOR  |  LOWER_MAJOR|LOWER_CRIT ");
                expected_cat = SAHPI_EC_THRESHOLD;
                expected_state = SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_MAJOR | SAHPI_ES_LOWER_CRIT;
  
                err = oh_encode_eventstate(&buffer, &event_state, &event_cat);
                if (err != SA_OK) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d\n", err);
                        return -1; 
                }

                if ((expected_state != event_state) || (expected_cat != event_cat)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        printf("  Receive state:d state=%x; Received cat=%x\n", event_state, event_cat);
                        return -1;
                }
	}

	/* oh_encode_eventstate testcases - valid states but different categories testcase */
        {
		strcpy((char *)buffer.Data, "LOWER_MINOR | STATE_13 | IDLE");
		buffer.DataLength = strlen("LOWER_MINOR | STATE_13 | IDLE");
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
  
                err = oh_encode_eventstate(&buffer, &event_state, &event_cat);
		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

	/* oh_encode_eventstate testcases - garbage state testcase */	
        {
		strcpy((char *)buffer.Data, "GARBAGE_STATE");
		buffer.DataLength = strlen("GARBAGE_STATE");
		expected_err = SA_ERR_HPI_INVALID_PARAMS;
  
                err = oh_encode_eventstate(&buffer, &event_state, &event_cat);
		if (expected_err != err) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%d, Expected error=%d\n", err, expected_err);
			return -1;
		}
	}

	/******************************* 
	 * oh_valid_eventstate testcases
         *******************************/
#if 0
	/* oh_valid_eventstate: SAHPI_EC_THRESHOLD Completeness (lower crit; no lower major) testcase */
	{
		event_state = SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_CRIT;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_THRESHOLD, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_THRESHOLD Completeness (lower major; no lower minor) testcase */
	{
		event_state = SAHPI_ES_LOWER_MAJOR;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_THRESHOLD, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_THRESHOLD Completeness (upper crit; no upper major) testcase */
	{
		event_state = SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_CRIT;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_THRESHOLD, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_THRESHOLD Completeness (upper major; no upper minor) testcase */
	{
		event_state = SAHPI_ES_UPPER_MAJOR;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_THRESHOLD, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}
	
#endif
	/* oh_valid_eventstate: SAHPI_EC_STATE exclusion testcase */
	{
		event_state = SAHPI_ES_STATE_DEASSERTED | SAHPI_ES_STATE_ASSERTED;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_STATE, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_PRED_FAIL exclusion testcase */
	{
		event_state = SAHPI_ES_PRED_FAILURE_DEASSERT | SAHPI_ES_PRED_FAILURE_ASSERT;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_PRED_FAIL, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}
	
	/* oh_valid_eventstate: SAHPI_EC_LIMIT exclusion testcase */
	{
		event_state = SAHPI_ES_LIMIT_NOT_EXCEEDED | SAHPI_ES_LIMIT_EXCEEDED;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_LIMIT, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}
	
	/* oh_valid_eventstate: SAHPI_EC_PERFORMANCE exclusion testcase */
	{
		event_state = SAHPI_ES_PERFORMANCE_MET | SAHPI_ES_PERFORMANCE_LAGS;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_PERFORMANCE, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }

	}

	/* oh_valid_eventstate: SAHPI_EC_PRESENCE exclusion testcase */
	{
		event_state = SAHPI_ES_ABSENT | SAHPI_ES_PRESENT;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_PRESENCE, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_ENABLE exclusion testcase */
	{
		event_state = SAHPI_ES_DISABLED | SAHPI_ES_ENABLED;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_ENABLE, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_REDUNDANCY - SAHPI_ES_FULLY_REDUNDANT exclusion testcase */
	{
		event_state = SAHPI_ES_FULLY_REDUNDANT | SAHPI_ES_REDUNDANCY_LOST;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_REDUNDANCY, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_REDUNDANCY - SAHPI_ES_REDUNDANCY_DEGRADED exclusion testcase */
	{
		event_state =  SAHPI_ES_REDUNDANCY_DEGRADED | SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_REDUNDANCY, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_REDUNDANCY - SAHPI_ES_REDUNDANCY_LOST exclusion testcase */
	{
		event_state = SAHPI_ES_REDUNDANCY_LOST | SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_REDUNDANCY, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_REDUNDANCY - SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL exclusion testcase */
	{
		event_state = SAHPI_ES_REDUNDANCY_DEGRADED | 
			      SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL |
			      SAHPI_ES_REDUNDANCY_DEGRADED_FROM_NON;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_REDUNDANCY, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/* oh_valid_eventstate: SAHPI_EC_REDUNDANCY - SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES exclusion testcase */
	{
		event_state = SAHPI_ES_REDUNDANCY_LOST | 
			      SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES |
 			      SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES;
		
		if (oh_valid_eventstate(event_state, SAHPI_EC_REDUNDANCY, SAHPI_TRUE)) {
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
                        return -1;
                }
	}

	/******************************
	 * oh_valid_add_event testcases
         ******************************/
	{
		SaHpiEventT default_event, event;
		memset(&default_event, 0, sizeof(SaHpiEventT));
		
		default_event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
		default_event.EventType = SAHPI_ET_USER;
		default_event.Severity = SAHPI_CRITICAL;
		default_event.EventDataUnion.UserEvent.UserEventData.DataType = SAHPI_TL_TYPE_TEXT;
		default_event.EventDataUnion.UserEvent.UserEventData.Language = SAHPI_LANG_ENGLISH;
		default_event.EventDataUnion.UserEvent.UserEventData.DataLength = strlen("Test");
		strncpy((char *)(default_event.EventDataUnion.UserEvent.UserEventData.Data), "Test",
			strlen("Test"));

		/* oh_valid_add_event: Normal testcase */
		event = default_event;
		expected_err = SA_OK;

		err = oh_valid_addevent(&event);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
		
		/* oh_valid_add_event: Bad Source testcase */
		event = default_event;
		event.Source = 1;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_addevent(&event);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_add_event: Bad Type testcase */
		event = default_event;
		event.EventType = BAD_TYPE;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_addevent(&event);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}

		/* oh_valid_add_event: Bad Text testcase */
		event = default_event;
		event.EventDataUnion.UserEvent.UserEventData.DataType = SAHPI_TL_TYPE_TEXT;
		event.EventDataUnion.UserEvent.UserEventData.Language = BAD_TYPE;
		expected_err = SA_ERR_HPI_INVALID_PARAMS;

		err = oh_valid_addevent(&event);
		if (err != expected_err) {	
			printf("  Error! Testcase failed. Line=%d\n", __LINE__);
			printf("  Received error=%s\n", oh_lookup_error(err));
			return -1;
		}
	}

        return 0;
}
