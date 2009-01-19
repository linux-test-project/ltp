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
 *	  Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 */


#ifndef _SIM_INIT_H
#define _SIM_INIT_H

#include <string.h>
#include <sys/time.h>
#include <uuid/uuid.h>
#include <unistd.h>

#include <SaHpi.h>
#include <oh_handler.h>
#include <oh_domain.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <sim_injector.h>
#include <sim_sensors.h>
#include <sim_sensor_func.h>
#include <sim_controls.h>
#include <sim_control_func.h>
#include <sim_annunciators.h>
#include <sim_annunciator_func.h>
#include <sim_dimi_func.h>
#include <sim_el.h>
#include <sim_power.h>
#include <sim_reset.h>
#include <sim_inventory.h>
#include <sim_watchdog.h>
#include <sim_hotswap.h>
#include <sim_resources.h>
#include <sim_dimi.h>
#include <sim_fumi.h>
#include <sim_fumi_func.h>


/* handler state list */
extern GSList *sim_handler_states;


void *sim_open(GHashTable *handler_config,
               unsigned int hid,
               oh_evt_queue *eventq);
SaErrorT sim_discover(void *hnd);
SaErrorT sim_get_event(void *hnd);
SaErrorT sim_close(void *hnd);
SaErrorT sim_set_resource_tag(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiTextBufferT *tag);
SaErrorT sim_set_resource_severity(void *hnd,
                                   SaHpiResourceIdT rid,
                                   SaHpiSeverityT sev);
SaErrorT sim_resource_failed_remove(void *hnd,
				    SaHpiResourceIdT rid);

#endif
