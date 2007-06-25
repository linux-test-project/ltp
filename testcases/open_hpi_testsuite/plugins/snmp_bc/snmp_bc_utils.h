/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Chris Chia <cchia@users.sf.net>
 */

#ifndef __SNMP_BC_UTILS_H
#define __SNMP_BC_UTILS_H

#define  UUID_SUBSTRINGS_CNT1  8
#define  UUID_SUBSTRINGS_CNT2  5

SaErrorT snmp_bc_get_guid(struct snmp_bc_hnd *custom_handle,
			  struct oh_event *e,
			  struct ResourceInfo *res_info_ptr);
			  
SaErrorT snmp_bc_extract_slot_ep(SaHpiEntityPathT *resource_ep,
				 SaHpiEntityPathT *slot_ep);
				 
SaErrorT snmp_bc_extend_ep(struct oh_event *e,
			   guint resource_index, 
			   gchar *interposer_intalled_mask); 
				 
struct oh_event *snmp_bc_alloc_oh_event(void);

void snmp_bc_free_oh_event(struct oh_event *e);

SaErrorT snmp_bc_set_resource_add_oh_event(struct oh_event *e, 
					struct ResourceInfo *res_info_ptr);

SaErrorT snmp_bc_copy_oh_event(struct oh_event *new_event, struct oh_event *old_event);						  			  

#endif
