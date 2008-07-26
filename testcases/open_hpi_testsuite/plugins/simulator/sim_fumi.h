/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 *  Authors:
 *  Suntrupth S Yadav <suntrupth@in.ibm.com>
 */
 
#ifndef __SIM_FUMI_H
#define __SIM_FUMI_H

struct sim_fumi_info {
        SaHpiFumiSourceInfoT srcinfo;
        SaHpiFumiBankInfoT info;
};

struct sim_fumi {  
        SaHpiFumiRecT fumirec;
        SaHpiFumiSourceInfoT srcinfo;
        SaHpiFumiBankInfoT info;
        const char *comment;
};

extern struct sim_fumi sim_chassis_fumis[];

SaErrorT sim_discover_chassis_fumis(struct oh_handler_state *state,
                                       struct oh_event *e);
    
#endif
