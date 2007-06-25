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

#ifndef __SNMP_BC_ANNUNCIATOR_H
#define __SNMP_BC_ANNUNCIATOR_H

SaErrorT snmp_bc_get_next_announce(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiAnnunciatorNumT aid,
				   SaHpiSeverityT sev,
				   SaHpiBoolT unackonly,
				   SaHpiAnnouncementT *announcement);

SaErrorT snmp_bc_get_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiAnnouncementT *announcement);

SaErrorT snmp_bc_ack_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiSeverityT sev);

SaErrorT snmp_bc_add_announce(void *hnd,
			      SaHpiResourceIdT rid, 
			      SaHpiAnnunciatorNumT aid,
			      SaHpiAnnouncementT *announcement);

SaErrorT snmp_bc_del_announce(void *hnd,
			      SaHpiResourceIdT rid,
			      SaHpiAnnunciatorNumT aid,
			      SaHpiEntryIdT entry,
			      SaHpiSeverityT sev);

SaErrorT snmp_bc_get_annunc_mode(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiAnnunciatorNumT aid,
				 SaHpiAnnunciatorModeT *mode);

SaErrorT snmp_bc_set_annunc_mode(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiAnnunciatorNumT aid,
				 SaHpiAnnunciatorModeT mode);
#endif
