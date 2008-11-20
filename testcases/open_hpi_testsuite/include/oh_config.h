/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Sean Dague <http://dague.net/sean>
 *     Renier Morales <renier@openhpi.org>
 */

#ifndef __OH_CONFIG_H
#define __OH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <SaHpi.h>
#include <oh_utils.h>
	
struct oh_parsed_config {
        GSList *handler_configs;
        guint handlers_defined;
        guint handlers_loaded;
};

typedef enum {
        OPENHPI_ON_EP = 1,
        OPENHPI_LOG_ON_SEV,
        OPENHPI_EVT_QUEUE_LIMIT,
        OPENHPI_DEL_SIZE_LIMIT,
        OPENHPI_DEL_SAVE,
        OPENHPI_DAT_SIZE_LIMIT,
        OPENHPI_DAT_USER_LIMIT,
	OPENHPI_DAT_SAVE,
        OPENHPI_PATH,
        OPENHPI_VARPATH,
        OPENHPI_CONF, 
	OPENHPICLIENT_CONF
} oh_global_param_type;

typedef union {
        SaHpiEntityPathT on_ep;
        SaHpiSeverityT log_on_sev;
        SaHpiUint32T evt_queue_limit;
        SaHpiUint32T del_size_limit;
        SaHpiBoolT del_save;
        SaHpiUint32T dat_size_limit;
        SaHpiUint32T dat_user_limit;
	SaHpiBoolT dat_save;
        char path[OH_MAX_TEXT_BUFFER_LENGTH];
        char varpath[OH_MAX_TEXT_BUFFER_LENGTH];
        char conf[OH_MAX_TEXT_BUFFER_LENGTH];
} oh_global_param_union;

struct oh_global_param {
        oh_global_param_type type;
        oh_global_param_union u;
};

/* Plugin configuration information prototypes */
int oh_load_config(char *filename, struct oh_parsed_config *config);
SaErrorT oh_process_config(struct oh_parsed_config *config);
void oh_clean_config(struct oh_parsed_config *config);

/* For handling global parameters */
int oh_get_global_param(struct oh_global_param *param);
int oh_set_global_param(struct oh_global_param *param);

#ifdef __cplusplus
}
#endif

#endif/*__OH_CONFIG_H*/
