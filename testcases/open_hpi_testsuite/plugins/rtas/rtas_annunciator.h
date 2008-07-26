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

#ifndef RTAS_ANNUNCIATOR_H
#define RTAS_ANNUNCIATOR_H
 
#include <glib.h>
#include <SaHpi.h> 
 
SaErrorT rtas_get_next_announce(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiAnnunciatorNumT num,
                                   SaHpiSeverityT sev,
                                   SaHpiBoolT unack,
                                   SaHpiAnnouncementT *a);
SaErrorT rtas_get_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiAnnouncementT *a);
SaErrorT rtas_ack_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiSeverityT sev);
SaErrorT rtas_add_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiAnnouncementT *a);
SaErrorT rtas_del_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiSeverityT sev);
SaErrorT rtas_get_annunc_mode(void *hnd,
                                 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
                                 SaHpiAnnunciatorModeT *mode);
SaErrorT rtas_set_annunc_mode(void *hnd,
                                 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
                                 SaHpiAnnunciatorModeT mode); 

#endif
