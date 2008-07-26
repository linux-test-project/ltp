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
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

/**
 * oh_decode_eventstate:
 * @event_state: Event state bit map to be converted to a string.
 * @event_cat: Event category of @event_state.
 * @buffer: Pointer to buffer to store generated string.
 *
 * Converts @event_state into a string based on @event_state's HPI definition.
 * For example, @event_state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR
 * is returned as the string "UPPER_MINOR | UPPER_MAJOR".
 * String is stored in an SaHpiTextBufferT data structure.
 *
 * Function validates that the @event_state bit map is valid for @event_cat.
 *
 * SAHPI_ES_UNSPECIFIED definitions are stripped from @event_state, if there are 
 * other valid non-global states defined. For example, @event_state = 
 * SAHPI_ES_IDLE | SAHPI_ES_UNSPECIFIED, returns the string "IDLE".
 *
 * Returns:
 * SA_OK - normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer is NULL; Invalid @event_state or @event_cat.
 * SA_ERR_HPI_OUT_OF_SPACE - @buffer too small.
  **/
SaErrorT oh_decode_eventstate(SaHpiEventStateT event_state,
			      SaHpiEventCategoryT event_cat,
			      SaHpiTextBufferT *buffer)
{
	int i, found;
	SaErrorT err;
	SaHpiTextBufferT working;

	/* Don't check for mutual exclusive events, since we want to see them all */
	if (!buffer || !oh_valid_eventstate(event_state, event_cat, SAHPI_FALSE)) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	err = oh_init_textbuffer(&working);
	if (err != SA_OK) { return(err); }

	found = 0;
	/* Look for category's event states */
	for (i=0; i<OH_MAX_STATE_STRINGS; i++) {
		if (state_strings[i].category == event_cat) {
			if ((state_strings[i].state & event_state) == state_strings[i].state) {
				found++;
				err = oh_append_textbuffer(&working, (char *)state_strings[i].str);
				if (err != SA_OK) { return(err); }
				err = oh_append_textbuffer(&working, OH_ENCODE_DELIMITER);
				if (err != SA_OK) { return(err); }
			}
		}
	}
	
	/* Look for global event states */
	for (i=0; i<OH_MAX_STATE_GLOBAL_STRINGS; i++) {
		if ((state_global_strings[i].state & event_state) == state_global_strings[i].state) {
			/* Strip any UNSPECIFIED definitions, if another definition found */
			if (!(found && state_global_strings[i].state == SAHPI_ES_UNSPECIFIED)) {
				found++;
				err = oh_append_textbuffer(&working, (char *)state_global_strings[i].str);
				if (err != SA_OK) { return(err); }
				err = oh_append_textbuffer(&working, OH_ENCODE_DELIMITER);
				if (err != SA_OK) { return(err); }
			}
		}
	}

	/* Remove last delimiter */
	if (found) {
		for (i=0; i<OH_ENCODE_DELIMITER_LENGTH + 1; i++) {
			working.Data[working.DataLength - i] = 0x00;
		}
		working.DataLength = working.DataLength - OH_ENCODE_DELIMITER_LENGTH;
	}

	err = oh_copy_textbuffer(buffer, &working);

	return(SA_OK);
}

/**
 * oh_encode_eventstate:
 * @buffer: Pointer to buffer containing string representation of event
 *          state bit map. Generally received from oh_decode_eventstate().
 * @event_state: Pointer to store generated event state bit map.
 * @event_cat: Pointer to store generated event category.
 *
 * Converts string representation of an event state bit map (usually generated
 * by oh_decode_eventstate() back into an HPI event state and category 
 * structure. For example, the string "UPPER_MINOR | UPPER_MAJOR" generates
 * event state = SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_MINOR and
 * event category = SAHPI_ES_THRESHOLD.

 * Function validates that the @event_state bit map is valid for @event_cat. And
 * that the states are complete and not mutually exclusive.
 *
 * NOTE!
 * @event_cat cannot always be deterministically calculated.
 *
 * - if @event_state is SAHPI_ES_UNSPECIFIED, @event_cat will be SAHPI_EC_UNSPECIFIED
 *   (though any category is valid).
 * - For event categories with the same events defined (e.g. SAHPI_EC_GENERIC and
 *   SAHPI_EC_SENSOR_SPECIFIC), the category returned may be either one.
 *
 * Returns:
 * SA_OK - normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - @buffer, @event_state, or @event_cat NULL.
 *                             No Data in @buffer, invalid format, or
 *                             @event_state bit map not valid for @event_cat. 
 * SA_ERR_HPI_OUT_OF_SPACE - @buffer too small.
 **/
SaErrorT oh_encode_eventstate(SaHpiTextBufferT *buffer,
			      SaHpiEventStateT *event_state,
			      SaHpiEventCategoryT *event_cat)
{
	gchar *gstr = NULL;
	gchar **eventdefs = NULL;
	int i, j, found_event, found_global_event;
	SaErrorT rtncode = SA_OK;
	SaHpiEventStateT working_state=0;
	SaHpiEventCategoryT working_cat=0;
		
	if (!buffer || !event_state || !event_cat) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	if (buffer->Data == NULL || buffer->Data[0] == '\0') {
		err("Invalid Data buffer parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	if (buffer->DataLength < SAHPI_MAX_TEXT_BUFFER_LENGTH) {
		buffer->Data[buffer->DataLength] = '\0';
	}
	
	/* Split out event definitions */
	if (buffer->DataLength < SAHPI_MAX_TEXT_BUFFER_LENGTH) {
		buffer->Data[buffer->DataLength] = '\0';
	}
	gstr = g_strstrip(g_strndup((gchar *)buffer->Data, SAHPI_MAX_TEXT_BUFFER_LENGTH));
	if (gstr == NULL || gstr[0] == '\0') {
		err("g_strstrip failed");
		rtncode = SA_ERR_HPI_INTERNAL_ERROR;
		goto CLEANUP;
	}

	eventdefs = g_strsplit(gstr, OH_ENCODE_DELIMITER_CHAR, -1);
	if (eventdefs == NULL) {
		err("No event definitions");
		rtncode = SA_ERR_HPI_INVALID_PARAMS;
		goto CLEANUP;
	}

	for (i=0; eventdefs[i] != NULL && eventdefs[i][0] != '\0'; i++) {
		eventdefs[i] = g_strstrip(eventdefs[i]);

		found_event = found_global_event = 0;
		/* Look for category event states */
		for (j=0; j<OH_MAX_STATE_STRINGS; j++) {
			if (strcasecmp(eventdefs[i], (char *)state_strings[j].str) == 0) {
				found_event++;
                                /* Don't add twice for categories with duplicate events */
				if (!(working_state & state_strings[j].state)) { 
					working_state = working_state + state_strings[j].state;
				}
				working_cat = state_strings[j].category;
			}
		}
		/* Look for global event states */
		for (j=0; j<OH_MAX_STATE_GLOBAL_STRINGS; j++) {
			if (strcasecmp(eventdefs[i], (char *)state_global_strings[j].str) == 0) {
				found_global_event++;
                                /* Don't add twice for categories with duplicate events */
				if (!(working_state & state_global_strings[j].state)) { 
					working_state = working_state + state_global_strings[j].state;
				}
				working_cat = state_global_strings[j].category;
			}
		}

		if (!found_event && !found_global_event) {
			err("No events found");
			rtncode = SA_ERR_HPI_INVALID_PARAMS;
			goto CLEANUP;
		}
	}
	
	/* Check for mutually exclusive event states */
	if (oh_valid_eventstate(working_state, working_cat, SAHPI_TRUE)) {
		*event_state = working_state;
		*event_cat = working_cat;
	}
	else {
		rtncode = SA_ERR_HPI_INVALID_PARAMS;
	}

 CLEANUP:
	g_free(gstr);
	g_strfreev(eventdefs);

	return(rtncode);
}

/**
 * oh_valid_eventstate:
 * @event_state: Event state bit field.
 * @event_cat: Event's category.
 * @check_mutal_exclusion: Boolean.
 * 
 * Validates that all the events in the event_state bit field are valid
 * for the event category. If @check_mutal_exclusion is true, the routine 
 * also checks for mutually exclusive events and that thresholds events 
 * have all the appropriate lower-level threshold event states set.
 * 
 * Returns:
 * SAHPI_TRUE - Event(s) valid for category and met HPI spec criteria
 *              for exclusiveness and completeness.
 * SAHPI_FALSE - Any event(s) in bit field not valid for category or
 *               category is invalid.
 **/
SaHpiBoolT oh_valid_eventstate(SaHpiEventStateT event_state,
			       SaHpiEventCategoryT event_cat,
			       SaHpiBoolT check_mutal_exclusion)
{
	SaHpiEventStateT valid_states;

	switch(event_cat) {
	case SAHPI_EC_UNSPECIFIED:
		/* Only SAHPI_ES_UNSPECIFIED valid for this category */
		if (event_state) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		return(SAHPI_TRUE);

	case SAHPI_EC_THRESHOLD:
		valid_states = SAHPI_ES_LOWER_MINOR |
			       SAHPI_ES_LOWER_MAJOR |
			       SAHPI_ES_LOWER_CRIT |
			       SAHPI_ES_UPPER_MINOR |
			       SAHPI_ES_UPPER_MAJOR |
  			       SAHPI_ES_UPPER_CRIT;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

#if 0
		/* Check that all lower-level thresholds are set */
		if (check_mutal_exclusion) {
			if (event_state & SAHPI_ES_LOWER_CRIT) {
				if (!(event_state & SAHPI_ES_LOWER_MAJOR)) {
					err("Critical lower threshold event set; but not major");
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_LOWER_MAJOR) {
				if (!(event_state & SAHPI_ES_LOWER_MINOR)) {
					err("Major lower threshold event set; but not minor");
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_UPPER_CRIT) {
				if (!(event_state & SAHPI_ES_UPPER_MAJOR)) {
					err("Critical upper threshold event set; but not major");
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_UPPER_MAJOR) {
				if (!(event_state & SAHPI_ES_UPPER_MINOR)) {
					err("Major upper threshold event set; but not minor");
					return(SAHPI_FALSE);
				}
			}
		}
#endif	
		return(SAHPI_TRUE);

	case SAHPI_EC_USAGE:
		valid_states = SAHPI_ES_IDLE |
			       SAHPI_ES_ACTIVE |
			       SAHPI_ES_BUSY;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		/* ??? Any of these mutually exclusive */
		
		return(SAHPI_TRUE);

	case SAHPI_EC_STATE:
		valid_states = SAHPI_ES_STATE_DEASSERTED |
			       SAHPI_ES_STATE_ASSERTED;
		
		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_STATE_DEASSERTED) &&
			    (event_state & SAHPI_ES_STATE_ASSERTED)) {
				err("Mutally exclusive STATE event states defined");
				return(SAHPI_FALSE);
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_PRED_FAIL:
		valid_states = SAHPI_ES_PRED_FAILURE_DEASSERT |
			       SAHPI_ES_PRED_FAILURE_ASSERT;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		
		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_PRED_FAILURE_DEASSERT) &&
			    (event_state & SAHPI_ES_PRED_FAILURE_ASSERT)) {
				err("Mutally exclusive PRED_FAIL event states defined");
				return(SAHPI_FALSE);		
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_LIMIT:
		valid_states = SAHPI_ES_LIMIT_NOT_EXCEEDED |
			       SAHPI_ES_LIMIT_EXCEEDED;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		
		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_LIMIT_NOT_EXCEEDED) &&
			    (event_state & SAHPI_ES_LIMIT_EXCEEDED)) {
				err("Mutally exclusive LIMIT event states defined");
				return(SAHPI_FALSE);		
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_PERFORMANCE:
		valid_states = SAHPI_ES_PERFORMANCE_MET |
			       SAHPI_ES_PERFORMANCE_LAGS;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		
		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_PERFORMANCE_MET) &&
			    (event_state & SAHPI_ES_PERFORMANCE_LAGS)) {
				err("Mutally exclusive PERFORMANCE event states defined");
				return(SAHPI_FALSE);		
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_SEVERITY:
		valid_states = SAHPI_ES_OK |
			       SAHPI_ES_MINOR_FROM_OK |
                               SAHPI_ES_MAJOR_FROM_LESS |
                               SAHPI_ES_CRITICAL_FROM_LESS |
                               SAHPI_ES_MINOR_FROM_MORE |
                               SAHPI_ES_MAJOR_FROM_CRITICAL |
                               SAHPI_ES_CRITICAL |
                               SAHPI_ES_MONITOR |
                               SAHPI_ES_INFORMATIONAL;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		/* ?? Any of these exclusive */

		return(SAHPI_TRUE);

	case SAHPI_EC_PRESENCE:
		valid_states = SAHPI_ES_ABSENT |
			       SAHPI_ES_PRESENT;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		
		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_ABSENT) &&
			    (event_state & SAHPI_ES_PRESENT)) {
				err("Mutally exclusive PRESENCE event states defined");
				return(SAHPI_FALSE);		
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_ENABLE:
		valid_states = SAHPI_ES_DISABLED |
			       SAHPI_ES_ENABLED;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		if (check_mutal_exclusion) {
			if ((event_state & SAHPI_ES_DISABLED) &&
			    (event_state & SAHPI_ES_ENABLED)) {
				err("Mutally exclusive ENABLE event states defined");
				return(SAHPI_FALSE);		
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_AVAILABILITY:
		valid_states = SAHPI_ES_RUNNING |
			       SAHPI_ES_TEST |
			       SAHPI_ES_POWER_OFF |
			       SAHPI_ES_ON_LINE |
			       SAHPI_ES_OFF_LINE |
			       SAHPI_ES_OFF_DUTY |
			       SAHPI_ES_DEGRADED |
			       SAHPI_ES_POWER_SAVE |
			       SAHPI_ES_INSTALL_ERROR;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		/* ?? Any of these exclusive */
		
		return(SAHPI_TRUE);

	case SAHPI_EC_REDUNDANCY:
		valid_states = SAHPI_ES_FULLY_REDUNDANT |
			       SAHPI_ES_REDUNDANCY_LOST |
			       SAHPI_ES_REDUNDANCY_DEGRADED |
		  	       SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES |
			       SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES |
			       SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES |
			       SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL |
			       SAHPI_ES_REDUNDANCY_DEGRADED_FROM_NON;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}

		if (check_mutal_exclusion) {
			/* Assume SAHPI_ES_REDUNDANCY_LOST or SAHPI_ES_REDUNDANCY_DEGRADED
			   must be set in addition to the bits that establish direction */
			if (event_state & SAHPI_ES_FULLY_REDUNDANT) {
				if (event_state != SAHPI_ES_FULLY_REDUNDANT) {
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_REDUNDANCY_LOST) {
				valid_states = SAHPI_ES_REDUNDANCY_LOST |
					SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES |
					SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES |
					SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES;
				if (event_state & (~valid_states)) {
					err("Mutally exclusive REDUNDANCY_LOST event states defined");
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_REDUNDANCY_DEGRADED) {
				valid_states = SAHPI_ES_REDUNDANCY_DEGRADED |
					SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL |
					SAHPI_ES_REDUNDANCY_DEGRADED_FROM_NON;
				if (event_state & (~valid_states)) {
					err("Mutally exclusive REDUNDANCY_LOST event states defined");
					return(SAHPI_FALSE);
				}
			}
			if (event_state & SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL) {
				if (event_state & SAHPI_ES_REDUNDANCY_DEGRADED_FROM_NON) {
					err("Mutally exclusive REDUNDANCY_LOST event states defined");
					return(SAHPI_FALSE);	
				}
			}
			if (event_state & SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES) {
				if (event_state & SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES) {
					err("Mutally exclusive REDUNDANCY_LOST event states defined");
					return(SAHPI_FALSE);	
				}
			}
		}

		return(SAHPI_TRUE);

	case SAHPI_EC_SENSOR_SPECIFIC:
	case SAHPI_EC_GENERIC:
		valid_states = SAHPI_ES_STATE_00 |
			       SAHPI_ES_STATE_01 |
			       SAHPI_ES_STATE_02 |
			       SAHPI_ES_STATE_03 |
			       SAHPI_ES_STATE_04 |
			       SAHPI_ES_STATE_05 |
			       SAHPI_ES_STATE_06 |
			       SAHPI_ES_STATE_07 |
			       SAHPI_ES_STATE_08 |
			       SAHPI_ES_STATE_09 |
			       SAHPI_ES_STATE_10 |
			       SAHPI_ES_STATE_11 |
			       SAHPI_ES_STATE_12 |
			       SAHPI_ES_STATE_13 |
			       SAHPI_ES_STATE_14;

		if (event_state & (~valid_states)) {
			err("Invalid event state.");
			return(SAHPI_FALSE);
		}
		
		return(SAHPI_TRUE);

	default:
		return(SAHPI_FALSE);
	}
}

/**
 * oh_valid_addevent:
 * @event: Pointer to add event.
 * 
 * Validates @event is a valid event for SaHpiEventAdd. This routines makes 
 * all the checks specified in the HPI spec to see if @event is valid.
 * 
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - See HPI spec.
 **/
SaErrorT oh_valid_addevent(SaHpiEventT *event)
{
	if (!event) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	if (event->Source != SAHPI_UNSPECIFIED_RESOURCE_ID ||
	    event->EventType != SAHPI_ET_USER ||
	    NULL == oh_lookup_severity(event->Severity) ||
	    event->Severity == SAHPI_ALL_SEVERITIES ||
	    !oh_valid_textbuffer(&(event->EventDataUnion.UserEvent.UserEventData))) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	/* No check for implementation-specific restriction on to how much data may 
           be provided in the SAHPI_ET_USER event */
   
	return(SA_OK);
}
