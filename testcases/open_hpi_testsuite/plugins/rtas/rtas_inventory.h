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

#ifndef RTAS_INVENTORY_H
#define RTAS_INVENTORY_H

#include <glib.h>
#include <SaHpi.h>

struct oh_rtas_idr_area {
        SaHpiIdrAreaHeaderT hpi_idr_area;
        GSList *fields;
};

struct oh_rtas_idr {
        SaHpiIdrInfoT hpi_idr;
        GSList *areas;
};

SaHpiIdrFieldTypeT rtas_get_idr_field_type(char *type);
SaHpiIdrAreaTypeT rtas_get_idr_area_type(char *type);

SaErrorT rtas_get_idr_info(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiIdrInfoT *idrinfo);

SaErrorT rtas_get_idr_area_header(void *hnd,
                                  SaHpiResourceIdT rid,
                                  SaHpiIdrIdT idrid,
                                  SaHpiIdrAreaTypeT areatype,
                                  SaHpiEntryIdT areaid,
                                  SaHpiEntryIdT *nextareaid,
                                  SaHpiIdrAreaHeaderT *header);

SaErrorT rtas_add_idr_area(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiIdrAreaTypeT areatype,
                           SaHpiEntryIdT *areaid);

SaErrorT rtas_del_idr_area(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiEntryIdT areaid);

SaErrorT rtas_get_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiEntryIdT areaid,
                            SaHpiIdrFieldTypeT fieldtype,
                            SaHpiEntryIdT fieldid,
                            SaHpiEntryIdT *nextfieldid,
                            SaHpiIdrFieldT *field);

SaErrorT rtas_add_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiIdrFieldT *field);

SaErrorT rtas_set_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiIdrFieldT *field);

SaErrorT rtas_del_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiEntryIdT areaid,
                            SaHpiEntryIdT fieldid);

#endif
