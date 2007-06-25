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

#ifndef SAHPI_EVENT_UTILS_H
#define SAHPI_EVENT_UTILS_H

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#ifdef __cplusplus
extern "C" {
#endif 

SaErrorT oh_decode_eventstate(SaHpiEventStateT event_state,
			      SaHpiEventCategoryT event_cat,
			      SaHpiTextBufferT *buffer);

SaErrorT oh_encode_eventstate(SaHpiTextBufferT *buffer,
			      SaHpiEventStateT *event_state,
			      SaHpiEventCategoryT *event_cat);

SaHpiBoolT oh_valid_eventstate(SaHpiEventStateT event_state,
			       SaHpiEventCategoryT event_cat,
			       SaHpiBoolT check_mutal_exclusion);

SaErrorT oh_valid_addevent(SaHpiEventT *event);

#ifdef __cplusplus
}
#endif

#endif
