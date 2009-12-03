/*
 * Copyright (C) 2007-2009, Hewlett-Packard Development Company, LLP
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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 */

#ifndef _OA_SOAP_SERVER_EVENT_H
#define _OA_SOAP_SERVER_EVENT_H

/* Include files */
#include "oa_soap_re_discover.h"

SaErrorT process_server_power_off_event(struct oh_handler_state *oh_handler,
                                        struct oh_event *event);

SaErrorT process_server_power_on_event(struct oh_handler_state *oh_handler,
                                       SOAP_CON *con,
                                       struct oh_event *event,
                                       int bay_number);

SaErrorT process_server_power_event(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    struct eventInfo *oa_event);

SaErrorT process_server_insertion_event(struct oh_handler_state *oh_handler,
                                        SOAP_CON *con,
                                        struct eventInfo *oa_event,
                                        SaHpiInt32T loc);

SaErrorT process_server_extraction_event(struct oh_handler_state *oh_handler,
                                         struct eventInfo *oa_event);

SaErrorT build_inserted_server_rpt(struct oh_handler_state *oh_handler,
                                   struct bladeInfo *response,
                                   SaHpiRptEntryT *rpt);

void oa_soap_proc_server_status(struct oh_handler_state *oh_handler,
				SOAP_CON *con,
				struct bladeStatus *status);

void oa_soap_serv_post_comp(struct oh_handler_state *oh_handler,
			    SOAP_CON *con,
			    SaHpiInt32T bay_number);
 
SaErrorT oa_soap_set_thermal_sensor(struct oh_handler_state *oh_handler,
 				    SaHpiRptEntryT *rpt,
 				    struct bladeThermalInfoArrayResponse
 					*thermal_response,
 				    SaHpiBoolT enable_flag);
 
#endif
