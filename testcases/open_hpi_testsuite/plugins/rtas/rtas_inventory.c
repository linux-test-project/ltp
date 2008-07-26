/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005,2006
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

#include <rtas_inventory.h>
#include <oh_error.h>
#include <oh_handler.h>
#include <oh_utils.h>
#include <strings.h>

#define MAX_FIELDS 35
static struct {
        char *type;
        SaHpiIdrFieldTypeT hpi_type;
} field_map[] = {
        { .type = "PN", .hpi_type = SAHPI_IDR_FIELDTYPE_PART_NUMBER },
        { .type = "FN", .hpi_type = SAHPI_IDR_FIELDTYPE_PART_NUMBER },
        { .type = "EC", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "MN", .hpi_type = SAHPI_IDR_FIELDTYPE_MANUFACTURER },
        { .type = "MF", .hpi_type = SAHPI_IDR_FIELDTYPE_MANUFACTURER },
        { .type = "SN", .hpi_type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER },
        { .type = "LI", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "RL", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "RM", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "NA", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "DD", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "DG", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "LL", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "VI", .hpi_type = SAHPI_IDR_FIELDTYPE_MANUFACTURER },
        { .type = "FU", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "SI", .hpi_type = SAHPI_IDR_FIELDTYPE_MANUFACTURER },
        { .type = "VK", .hpi_type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION },
        { .type = "TM", .hpi_type = SAHPI_IDR_FIELDTYPE_PART_NUMBER},
        { .type = "YL", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "BR", .hpi_type = SAHPI_IDR_FIELDTYPE_ASSET_TAG },
        { .type = "CI", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "RD", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "PA", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "NN", .hpi_type = SAHPI_IDR_FIELDTYPE_ASSET_TAG },
        { .type = "DS", .hpi_type = SAHPI_IDR_FIELDTYPE_PRODUCT_NAME },
        { .type = "AX", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "CC", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "CD", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "CL", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "MP", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "SE", .hpi_type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER },
        { .type = "SZ", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
        { .type = "PI", .hpi_type = SAHPI_IDR_FIELDTYPE_PRODUCT_NAME },
        { .type = "VC", .hpi_type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION },
        { .type = "OS", .hpi_type = SAHPI_IDR_FIELDTYPE_CUSTOM },
};

SaHpiIdrFieldTypeT rtas_get_idr_field_type(char *type)
{
        int i;
        SaHpiIdrFieldTypeT hpi_type = SAHPI_IDR_FIELDTYPE_UNSPECIFIED;

        if (!type) return hpi_type-1;

        for (i = 0; i < MAX_FIELDS; i++) {
                if (strcasecmp(type, field_map[i].type) == 0) {
                        hpi_type = field_map[i].hpi_type;
                        break;
                }
        }

        return hpi_type;
}

SaHpiIdrAreaTypeT rtas_get_idr_area_type(char *type)
{
        SaHpiIdrAreaTypeT area_type = SAHPI_IDR_FIELDTYPE_UNSPECIFIED;

        if (strcasecmp(type, "VC") == 0) {
                area_type = SAHPI_IDR_AREATYPE_PRODUCT_INFO;
        } else if (strcasecmp(type, "DS") == 0) {
                area_type = SAHPI_IDR_AREATYPE_BOARD_INFO;
        }

        return area_type;
}

SaErrorT rtas_get_idr_info(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiIdrInfoT *idrinfo)
{
        struct oh_rtas_idr *idr_info = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oh_handler_state *h = hnd;

        if (!hnd) return SA_ERR_HPI_INTERNAL_ERROR;

        rdr = oh_get_rdr_by_type(h->rptcache, rid, SAHPI_INVENTORY_RDR, idrid);
        if (!rdr) return SA_ERR_HPI_NOT_PRESENT;
        idr_info = oh_get_rdr_data(h->rptcache, rid, rdr->RecordId);
        if (!idr_info) return SA_ERR_HPI_INTERNAL_ERROR;

        *idrinfo = idr_info->hpi_idr;

        return SA_OK;
}

SaErrorT rtas_get_idr_area_header(void *hnd,
                                  SaHpiResourceIdT rid,
                                  SaHpiIdrIdT idrid,
                                  SaHpiIdrAreaTypeT areatype,
                                  SaHpiEntryIdT areaid,
                                  SaHpiEntryIdT *nextareaid,
                                  SaHpiIdrAreaHeaderT *header)
{
        struct oh_rtas_idr *idr_info = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oh_handler_state *h = hnd;
        GSList *node = NULL;

        if (!hnd) return SA_ERR_HPI_INTERNAL_ERROR;

        rdr = oh_get_rdr_by_type(h->rptcache, rid, SAHPI_INVENTORY_RDR, idrid);
        if (!rdr) {
                err("RDR not found. %u->inventory->%u", rid, idrid);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        idr_info = oh_get_rdr_data(h->rptcache, rid, rdr->RecordId);
        if (!idr_info) {
                err("IDR data not found! %u->%u", rid, rdr->RecordId);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        for (node = idr_info->areas; node; node = node->next) {
                struct oh_rtas_idr_area *area = node->data;
                if ((areatype != SAHPI_IDR_AREATYPE_UNSPECIFIED &&
                     (areaid == SAHPI_FIRST_ENTRY || areaid == area->hpi_idr_area.AreaId) &&
                     areatype == area->hpi_idr_area.Type) ||
                    (areaid == SAHPI_FIRST_ENTRY || areaid == area->hpi_idr_area.AreaId)) {
                        *header = area->hpi_idr_area;
                        if (node->next) {
                                struct oh_rtas_idr_area *narea = node->next->data;
                                *nextareaid = narea->hpi_idr_area.AreaId;
                        } else {
                                *nextareaid = SAHPI_LAST_ENTRY;
                        }
                        return SA_OK;
                }

        }

        return SA_ERR_HPI_NOT_PRESENT;
}

SaErrorT rtas_add_idr_area(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiIdrAreaTypeT areatype,
                           SaHpiEntryIdT *areaid)
{
        return SA_ERR_HPI_READ_ONLY;
}

SaErrorT rtas_del_idr_area(void *hnd,
                           SaHpiResourceIdT rid,
                           SaHpiIdrIdT idrid,
                           SaHpiEntryIdT areaid)
{
        return SA_ERR_HPI_READ_ONLY;
}

SaErrorT rtas_get_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiEntryIdT areaid,
                            SaHpiIdrFieldTypeT fieldtype,
                            SaHpiEntryIdT fieldid,
                            SaHpiEntryIdT *nextfieldid,
                            SaHpiIdrFieldT *field)
{
        struct oh_rtas_idr *idr_info = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oh_handler_state *h = hnd;
        GSList *node = NULL;

        if (!hnd) return SA_ERR_HPI_INTERNAL_ERROR;

        rdr = oh_get_rdr_by_type(h->rptcache, rid, SAHPI_INVENTORY_RDR, idrid);
        if (!rdr) return SA_ERR_HPI_NOT_PRESENT;
        idr_info = oh_get_rdr_data(h->rptcache, rid, rdr->RecordId);
        if (!idr_info) return SA_ERR_HPI_INTERNAL_ERROR;

        for (node = idr_info->areas; node; node = node->next) {
                struct oh_rtas_idr_area *area = node->data;
                if (areaid == area->hpi_idr_area.AreaId || areaid == SAHPI_FIRST_ENTRY) {
                        GSList *node2 = NULL;
                        for (node2 = area->fields; node2; node2 = node2->next) {
                                SaHpiIdrFieldT *tfield = node2->data;
                                if ((fieldtype != SAHPI_IDR_FIELDTYPE_UNSPECIFIED &&
                                     (fieldid == SAHPI_FIRST_ENTRY || fieldid == tfield->FieldId) &&
                                     fieldtype == tfield->Type) ||
                                    (fieldid == SAHPI_FIRST_ENTRY || fieldid == tfield->FieldId)) {
                                        *field = *tfield;
                                        if (node2->next) {
                                                SaHpiIdrFieldT *nfield = node2->next->data;
                                                *nextfieldid = nfield->FieldId;
                                        } else {
                                                *nextfieldid = SAHPI_LAST_ENTRY;
                                        }
                                        return SA_OK;
                                }
                        }
                }

        }

        return SA_ERR_HPI_NOT_PRESENT;
}

SaErrorT rtas_add_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiIdrFieldT *field)
{
        return SA_ERR_HPI_READ_ONLY;
}

SaErrorT rtas_set_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiIdrFieldT *field)
{
        return SA_ERR_HPI_READ_ONLY;
}

SaErrorT rtas_del_idr_field(void *hnd,
                            SaHpiResourceIdT rid,
                            SaHpiIdrIdT idrid,
                            SaHpiEntryIdT areaid,
                            SaHpiEntryIdT fieldid)
{
        return SA_ERR_HPI_READ_ONLY;
}

void * oh_get_idr_info (void *hnd, SaHpiResourceIdT, SaHpiIdrIdT,SaHpiIdrInfoT)
        __attribute__ ((weak, alias("rtas_get_idr_info")));
void * oh_get_idr_area_header (void *, SaHpiResourceIdT, SaHpiIdrIdT,
                               SaHpiIdrAreaTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                               SaHpiIdrAreaHeaderT)
        __attribute__ ((weak, alias("rtas_get_idr_area_header")));
void * oh_add_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrAreaTypeT,
                        SaHpiEntryIdT)
        __attribute__ ((weak, alias("rtas_add_idr_area")));
void * oh_del_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT)
        __attribute__ ((weak, alias("rtas_del_idr_area")));
void * oh_get_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldT)
        __attribute__ ((weak, alias("rtas_get_idr_field")));
void * oh_add_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
        __attribute__ ((weak, alias("rtas_add_idr_field")));
void * oh_set_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
        __attribute__ ((weak, alias("rtas_set_idr_field")));
void * oh_del_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiEntryIdT)
        __attribute__ ((weak, alias("rtas_del_idr_field")));
