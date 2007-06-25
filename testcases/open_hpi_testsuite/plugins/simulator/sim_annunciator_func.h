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
 * Author(s):
 *        W. David Ashley <dashley@us.ibm.com>
 */

#ifndef __SIM_ANNUNCIATOR_FUNC_H
#define __SIM_ANNUNCIATOR_FUNC_H

SaErrorT sim_get_next_announce(void *hnd,
	        	       SaHpiResourceIdT rid,
			       SaHpiAnnunciatorNumT aid,
                    	       SaHpiSeverityT sev,
			       SaHpiBoolT unackonly,
			       SaHpiAnnouncementT *announcement);

SaErrorT sim_get_announce(void *hnd,
			  SaHpiResourceIdT rid,
			  SaHpiAnnunciatorNumT aid,
			  SaHpiEntryIdT entry,
			  SaHpiAnnouncementT *announcement);

SaErrorT sim_ack_announce(void *hnd,
			  SaHpiResourceIdT rid,
			  SaHpiAnnunciatorNumT aid,
			  SaHpiEntryIdT entry,
			  SaHpiSeverityT sev);

SaErrorT sim_add_announce(void *hnd,
			  SaHpiResourceIdT rid,
			  SaHpiAnnunciatorNumT aid,
			  SaHpiAnnouncementT *announcement);

SaErrorT sim_del_announce(void *hnd,
			  SaHpiResourceIdT rid,
			  SaHpiAnnunciatorNumT aid,
			  SaHpiEntryIdT entry,
			  SaHpiSeverityT sev);

SaErrorT sim_get_annunc_mode(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiAnnunciatorNumT aid,
			     SaHpiAnnunciatorModeT *mode);

SaErrorT sim_set_annunc_mode(void *hnd,
			     SaHpiResourceIdT rid,
			     SaHpiAnnunciatorNumT aid,
			     SaHpiAnnunciatorModeT mode);
#endif

