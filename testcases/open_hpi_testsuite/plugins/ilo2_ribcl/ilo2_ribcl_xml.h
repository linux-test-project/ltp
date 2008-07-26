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
#ifndef _INC_ILO2_RIBCL_XML_H_
#define _INC_ILO2_RIBCL_XML_H_

#include "ilo2_ribcl.h"

#define RIBCL_SUCCESS 0
#define RIBCL_UNSUPPORTED 1	/* Unsupported RIBCL command */

extern int ir_xml_parse_status( char *, char *);
extern int ir_xml_parse_emhealth( ilo2_ribcl_handler_t *, char *);
extern int ir_xml_parse_hostdata( ilo2_ribcl_handler_t *, char *);
extern int ir_xml_parse_host_power_status(char *, int *, char *);
extern int ir_xml_parse_reset_server(char *, char *);
extern int ir_xml_parse_set_host_power(char *, char *);
extern int ir_xml_parse_uid_status(char *, int *, char *);
extern int ir_xml_parse_power_saver_status(char *, int *, char *);
extern int ir_xml_parse_auto_power_status(char *, int *, char *);
extern int ir_xml_parse_discoveryinfo( ilo2_ribcl_handler_t *, char *);
extern void ir_xml_free_cmdbufs( ilo2_ribcl_handler_t *);
extern int ir_xml_build_cmdbufs( ilo2_ribcl_handler_t *);
 
#endif /* _INC_ILO2_RIBCL_XML_H_ */
