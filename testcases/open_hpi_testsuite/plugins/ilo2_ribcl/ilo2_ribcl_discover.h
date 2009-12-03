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
#ifndef _INC_ILO2_RIBCL_DISCOVER_H_
#define _INC_ILO2_RIBCL_DISCOVER_H_

extern void ilo2_ribcl_free_discoverydata( ilo2_ribcl_handler_t *);

/* The size used for the temporary buffer to contain the response
 * of the IR_CMD_GET_SERVER_DATA command. The current return size is
 * a little over 24K, so we use 48K to give us some margin for the
 * future.
 */
#define ILO2_RIBCL_DISCOVER_RESP_MAX 1024*48

/* This define is the IANA-assigned private enterprise number for 
   Hewlett-Packard. A complete list of IANA numbers can be found at
   http://www.iana.org/assignments/enterprise-numbers
*/
#define HP_MANUFACTURING_ID 11

/* Prototypes for functions within ilo2_ribcl_discovery.c that can be
 * called from within other modules */

extern void ilo2_ribcl_add_resource_capability( struct oh_handler_state *,
        struct oh_event *, SaHpiCapabilitiesT); 

#endif /* _INC_ILO2_RIBCL_DISCOVER_H_ */
