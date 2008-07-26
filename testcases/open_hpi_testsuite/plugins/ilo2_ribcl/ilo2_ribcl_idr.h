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
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */
#ifndef _INC_ILO2_RIBCL_IDR_H_
#define _INC_ILO2_RIBCL_IDR_H_

/* These are the functions exported by ilo2_ribcl_idr.c for other modules
 * to use. */

extern SaErrorT ilo2_ribcl_add_idr( struct oh_handler_state *,
                        struct oh_event *,
                        SaHpiIdrIdT,
                        struct ilo2_ribcl_idr_info *,
                        char *);

extern void ilo2_ribcl_discover_chassis_idr( struct oh_handler_state *,
                        struct oh_event *,
			char *);

extern void ilo2_ribcl_update_fru_idr( struct oh_handler_state *,
			SaHpiEntityPathT *,
			struct ilo2_ribcl_idr_info *);


extern void ilo2_ribcl_update_chassis_idr( struct oh_handler_state *,
                                SaHpiEntityPathT *);

extern void ilo2_ribcl_build_chassis_idr( ilo2_ribcl_handler_t *,
                        struct ilo2_ribcl_idr_info *);

extern void ilo2_ribcl_build_cpu_idr( ilo2_ribcl_handler_t *ir_handler,
                        struct ilo2_ribcl_idr_info *);

extern void ilo2_ribcl_build_memory_idr( ir_memdata_t *,
                        struct ilo2_ribcl_idr_info *);

#endif /* _INC_ILO2_RIBCL_IDR_H_ */

