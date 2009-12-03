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
 *      Mohan Devarajulu <mohan@fc.hp.com>
 */

#ifndef _OA_SOAP_RE_DISCOVER_H
#define _OA_SOAP_RE_DISCOVER_H

/* Include files */
#include "oa_soap_discover.h"
#include "oa_soap_server_event.h"
#include "oa_soap_interconnect_event.h"
#include "oa_soap_fan_event.h"
#include "oa_soap_ps_event.h"
#include "oa_soap_enclosure_event.h"
#include "oa_soap_lcd_event.h"

SaErrorT oa_soap_re_discover_resources(struct oh_handler_state *oh_handler,
                                       struct oa_info *oa, int oa_switched);

SaErrorT re_discover_oa(struct oh_handler_state *oh_handler,
                        SOAP_CON *con);

SaErrorT remove_oa(struct oh_handler_state *oh_handler,
                   SaHpiInt32T bay_number);

SaErrorT add_oa(struct oh_handler_state *oh_handler,
                SOAP_CON *con,
                SaHpiInt32T bay_number);

SaErrorT re_discover_blade(struct oh_handler_state *oh_handler,
                           SOAP_CON *con);

SaErrorT update_server_hotswap_state(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con,
                                     SaHpiInt32T bay_number);

SaErrorT remove_server_blade(struct oh_handler_state *oh_handler,
                             SaHpiInt32T bay_number);

SaErrorT add_server_blade(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          struct bladeInfo *info);

SaErrorT re_discover_interconnect(struct oh_handler_state *oh_handler,
                                  SOAP_CON *con);

SaErrorT update_interconnect_hotswap_state(struct oh_handler_state *oh_handler,
                                           SOAP_CON *con,
                                           SaHpiInt32T bay_number);

SaErrorT remove_interconnect(struct oh_handler_state *oh_handler,
                             SaHpiInt32T bay_number);

SaErrorT add_interconnect(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          SaHpiInt32T bay_number);

SaErrorT re_discover_fan(struct oh_handler_state *oh_handler,
                         SOAP_CON *con);

SaErrorT remove_fan(struct oh_handler_state *oh_handler,
                    SaHpiInt32T bay_number);

SaErrorT add_fan(struct oh_handler_state *oh_handler,
                 SOAP_CON *con,
                 struct fanInfo *info);

SaErrorT re_discover_ps_unit(struct oh_handler_state *oh_handler,
                             SOAP_CON *con);

SaErrorT remove_ps_unit(struct oh_handler_state *oh_handler,
                        SaHpiInt32T bay_number);

SaErrorT add_ps_unit(struct oh_handler_state *oh_handler,
                     SOAP_CON *con,
                     struct powerSupplyInfo *info);

#endif
