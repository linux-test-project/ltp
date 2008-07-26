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
 *        Renier Morales <renier@openhpi.org>
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 */

#ifndef RTAS_RESOURCE_H
#define RTAS_RESOURCE_H

#include <glib.h>
#include <SaHpi.h>

SaErrorT rtas_set_resource_tag(void *hnd,
                                  SaHpiResourceIdT id,
                                  SaHpiTextBufferT *tag);
				  
SaErrorT rtas_set_resource_severity(void *hnd,
                                       SaHpiResourceIdT id,
                                       SaHpiSeverityT sev); 

#endif
