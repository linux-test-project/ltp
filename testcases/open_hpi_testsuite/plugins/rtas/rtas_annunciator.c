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

#include <rtas_annunciator.h>

SaErrorT rtas_get_next_announce(void *hnd,
                                   SaHpiResourceIdT id,
                                   SaHpiAnnunciatorNumT num,
                                   SaHpiSeverityT sev,
                                   SaHpiBoolT unack,
                                   SaHpiAnnouncementT *a)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiAnnouncementT *a)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_ack_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiSeverityT sev)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_add_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiAnnouncementT *a)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_del_announce(void *hnd,
                              SaHpiResourceIdT id,
                              SaHpiAnnunciatorNumT num,
                              SaHpiEntryIdT aid,
                              SaHpiSeverityT sev)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_get_annunc_mode(void *hnd,
                                 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
                                 SaHpiAnnunciatorModeT *mode)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

SaErrorT rtas_set_annunc_mode(void *hnd,
                                 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
                                 SaHpiAnnunciatorModeT mode)
{
        return SA_ERR_HPI_INTERNAL_ERROR;
}

void * oh_get_next_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                             SaHpiSeverityT, SaHpiBoolT, SaHpiAnnouncementT)
        __attribute__ ((weak, alias("rtas_get_next_announce")));
void * oh_get_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiEntryIdT, SaHpiAnnouncementT *)
        __attribute__ ((weak, alias("rtas_get_announce")));
void * oh_ack_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiEntryIdT, SaHpiSeverityT)
        __attribute__ ((weak, alias("rtas_ack_announce")));
void * oh_add_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                        SaHpiAnnouncementT *)
        __attribute__ ((weak, alias("rtas_add_announce")));
void * oh_del_announce (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                       SaHpiEntryIdT, SaHpiSeverityT)
        __attribute__ ((weak, alias("rtas_del_announce")));
void * oh_get_annunc_mode (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                           SaHpiAnnunciatorModeT *)
        __attribute__ ((weak, alias("rtas_get_annunc_mode")));
void * oh_set_annunc_mode (void *, SaHpiResourceIdT, SaHpiAnnunciatorNumT,
                           SaHpiAnnunciatorModeT)
        __attribute__ ((weak, alias("rtas_set_annunc_mode")));
