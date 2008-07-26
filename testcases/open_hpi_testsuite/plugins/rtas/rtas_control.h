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

#ifndef RTAS_CONTROL_H
#define RTAS_CONTROL_H
 
#include <glib.h>
#include <SaHpi.h>

SaErrorT rtas_get_control_state(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiCtrlNumT num,
                                   SaHpiCtrlModeT *mode,
                                   SaHpiCtrlStateT *state);
				   
SaErrorT rtas_set_control_state(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiCtrlNumT num,
                                   SaHpiCtrlModeT mode,
                                   SaHpiCtrlStateT *state); 
				   
SaErrorT rtas_control_parm(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiParmActionT act);				   

#endif
