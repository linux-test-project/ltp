/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *      Vivek Kumar <vivek.kumar2@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 */

#ifndef _OA_SOAP_UTILS_H
#define _OA_SOAP_UTILS_H

/* Include files */
#include <math.h>
#include <ctype.h>

#include "oa_soap_calls.h"
#include "oa_soap.h"
#include "oa_soap_inventory.h"

/* OA takes around 90 seconds to stabilize */
#define OA_STABILIZE_MAX_TIME 90

/* Max timeout value for checking the availability of OA */
#define MAX_TIMEOUT 300

/* Function prototypes */

SaErrorT get_oa_soap_info(struct oh_handler_state *oh_handler);

SaErrorT get_oa_state(struct oh_handler_state *oh_handler,
                      char *ip);

void update_hotswap_event(struct oh_handler_state *oh_handler,
                          struct oh_event *event);

struct oh_event *copy_oa_soap_event(struct oh_event *event);

SaErrorT del_rdr_from_event(struct oh_event *event);

SaErrorT check_oa_status(struct oa_soap_handler *oa_handler,
                         struct oa_info *oa,
                         SOAP_CON *con);

SaErrorT check_oa_user_permissions(struct oa_soap_handler *oa_handler,
                                   SOAP_CON *con,
                                   char *user_name);

SaErrorT check_discovery_failure(struct oh_handler_state *oh_handler);

SaErrorT lock_oa_soap_handler(struct oa_soap_handler *oa_handler);

SaErrorT check_config_parameters(GHashTable *handler_config);

SaErrorT create_event_session(struct oa_info *oa);

void create_oa_connection(struct oa_soap_handler *oa_handler,
                          struct oa_info *oa,
                          char *user_name,
                          char *password);

SaErrorT initialize_oa_con(struct oa_info *oa,
                           char *user_name,
                           char *password);

SaErrorT delete_all_inventory_info(struct oh_handler_state *oh_handler);

void cleanup_plugin_rptable(struct oh_handler_state *oh_handler);

void release_oa_soap_resources(struct oa_soap_handler * oa_handler);

SaHpiFloat64T get_oa_fw_version(struct oh_handler_state *oh_handler);

SaErrorT update_oa_info(struct oh_handler_state *oh_handler,
                        struct oaInfo *response,
                        SaHpiResourceIdT resource_id);

SaErrorT convert_lower_to_upper(char *src,
                                SaHpiInt32T src_len,
                                char *dest,
                                SaHpiInt32T dest_len);

void oa_soap_update_resource_status(resource_status_t *res_status,
                                    SaHpiInt32T index,
                                    char *serial_number,
                                    SaHpiResourceIdT resource_id,
                                    resource_presence_status_t presence);
#endif
