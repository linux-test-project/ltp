/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	  Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 *	  Renier Morales <renier@openhpi.org>
 */


#ifndef __SIM_ANNUNCIATORS_H
#define __SIM_ANNUNCIATORS_H


/*******************
Annunciator Definitions
*********************/

#define ANNOUN_MAX      5

struct simAnnunciatorInfo {
        SaHpiAnnunciatorModeT mode;
        oh_announcement       *announs;
};

struct sim_annunciator {
        int                  index;
        SaHpiAnnunciatorRecT annun;
        SaHpiAnnouncementT   announs[ANNOUN_MAX + 1];
	const char           *comment;
};


extern struct sim_annunciator sim_chassis_annunciators[];
extern struct sim_annunciator sim_cpu_annunciators[];
extern struct sim_annunciator sim_dasd_annunciators[];
extern struct sim_annunciator sim_hs_dasd_annunciators[];
extern struct sim_annunciator sim_fan_annunciators[];

SaErrorT sim_discover_chassis_annunciators(struct oh_handler_state *state,
                                           struct oh_event *e);
SaErrorT sim_discover_cpu_annunciators(struct oh_handler_state *state,
                                       struct oh_event *e);
SaErrorT sim_discover_dasd_annunciators(struct oh_handler_state *state,
                                        struct oh_event *e);
SaErrorT sim_discover_hs_dasd_annunciators(struct oh_handler_state *state,
                                           struct oh_event *e);
SaErrorT sim_discover_fan_annunciators(struct oh_handler_state *state,
                                       struct oh_event *e);


#endif

