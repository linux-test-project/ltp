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
#include <rtas_event.h> 

SaErrorT rtas_get_event(void *hnd)
{
        if (!hnd) {
                err("Invalid parameter");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        return SA_OK;
}

void * oh_get_event (void *)
        __attribute__ ((weak, alias("rtas_get_event")));
