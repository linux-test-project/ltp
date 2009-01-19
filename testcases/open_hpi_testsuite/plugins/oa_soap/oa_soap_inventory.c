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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Bhaskara Bhatt <bhaskara.hg@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *
 * This file supports the functions related to HPI Inventory Data repositories.
 * The file covers three general classes of function: IDR ABI functions,
 * Build functions for creating IDRs for resources and IDR utility functions to
 * perform different operations (such as addition, deletion, modification) of
 * IDR areas and IDR fields.
 *
 * Data structure usage in Inventory:
 * - For each resource, one IDR header structure is created and associated in
 *   the private data area of the Inventory RDR.
 * - Area list is implemented using linked list datastructure.
 * - Each area in list will contain an area header and field list.
 * - Field list is also implemented using linked list datastructure.
 *
 * IDR ABI functions:
 *
 *      oa_soap_get_idr_info()          - Gets the Inventory Data
 *                                        Repository(IDR) information
 *
 *      oa_soap_get_idr_area_header()   - Gets the Inventory Data
 *                                        Repository(IDR) area header
 *                                        information for a specific area
 *
 *      oa_soap_add_idr_area()          - Creates the Inventory Data
 *                                        Repository(IDR) area of requested type
 *
 *      oa_soap_add_idr_area_by_id()    - Creates the Inventory Data
 *                                        Repository(IDR) area of requested type
 *                                        with specified id
 *
 *      oa_soap_delete_idr_area()       - Deletes the Inventory Data
 *                                        Repository(IDR) area of specified id
 *
 *      oa_soap_get_idr_field()         - Gets Inventory Data Repository(IDR)
 *                                        field from a particular IDR area
 *
 *      oa_soap_add_idr_field()         - Adds an IDR field to the specified
 *                                        Inventory Data Repository(IDR) Area
 *
 *      oa_soap_add_idr_field_by_id()   - Add a field to a specified Inventory
 *                                        Data Repository(IDR) Area with the
 *                                        specified field id
 *
 *      oa_soap_set_idr_field()         - Updates the specified Inventory Data
 *                                        Repository(IDR) field
 *
 *      oa_soap_del_idr_field()         - Deletes the Inventory Data Repository
 *                                        (IDR) field with specific identifier
 *                                        in IDR area
 * Build functions:
 *
 *      build_enclosure_inv_rdr()       - Creates an inventory rdr for enclosure
 *
 *      build_oa_inv_rdr()              - Creates an inventory rdr for OA
 *
 *      build_server_inv_rdr()          - Creates an inventory rdr for server
 *
 *      build_interconnect_inv_rdr()    - Creates an inventory rdr for
 *                                        interconnect
 *
 *      build_fan_inv_rdr()             - Creates an inventory rdr for fan
 *
 *      build_power_inv_rdr()           - Creates an inventory rdr for power
 *                                        supply
 *
 *	oa_soap_build_fan_zone_inventory_rdr() - Creates inventory rdr for Fan
 *						 Zone
 *
 * IDR Utility functions:
 *
 *      add_product_area(),
 *      add_chassis_area(),
 *      add_board_area(),
 *      add_internal_area()             - IDR utility functions to add IDR areas
 *
 *      idr_area_add()                  - Adds an IDR area to Inventory
 *                                        repository
 *
 *      idr_area_add_by_id()            - Adds an IDR area to Inventory
 *                                        repository with the specified area id
 *
 *      fetch_idr_area_header()         - Gets an Inventory Data Repository(IDR)
 *                                        area header from Inventory Data
 *                                        Repository(IDR)
 *
 *      idr_area_delete()               - Deletes an IDR area from Inventory
 *                                        Data Repository(IDR)
 *
 *      idr_field_add()                 - Adds an IDR field to an IDR area
 *
 *      idr_field_add_by_id()           - Adds an IDR field to an IDR area with
 *                                        the specified field id
 *
 *      idr_field_delete()              - Deletes an IDR field from an IDR area
 *
 *      idr_field_update()              - Updates an IDR field of an IDR area
 *
 *      fetch_idr_field()               - Gets an IDR field from IDR area in
 *                                        Inventory repository
 *
 *	oa_soap_inv_set_field()		- Sets the field during discovery
 *
 *	oa_soap_add_inv_fields()	- Adds the fields to area during
 *					  discovery
 *
 *	oa_soap_add_inv_areas()		- Adds the area to area_list during
 *					  discovery
 *
 * 	oa_soap_build_inventory_rdr()	- Creates the inventory RDR
 *                                        
 */

#include "oa_soap_inventory.h"

/* Array defined in oa_soap_resources.c */
extern const struct oa_soap_inv_rdr oa_soap_inv_arr[];
extern const struct oa_soap_fz_map oa_soap_fz_map_arr[][OA_SOAP_MAX_FAN];

/* Forward declarations for static functions */
static void oa_soap_inv_set_field(struct oa_soap_area *area_list,
				  SaHpiIdrAreaTypeT area_type,
				  SaHpiIdrFieldTypeT field_type,
				  const char *data);
static void oa_soap_add_inv_fields(struct oa_soap_area *area,
				   const struct oa_soap_field *field_list);
static void oa_soap_add_inv_areas(struct oa_soap_inventory *inventory,
				  SaHpiInt32T resource_type);
static SaErrorT oa_soap_build_inv(struct oh_handler_state *oh_handler,
				  SaHpiInt32T resource_type,
				  SaHpiResourceIdT resource_id,
				  struct oa_soap_inventory **inventory);

/**
 * oa_soap_get_idr_info
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @idr_info: Structure for receiving idr information
 *
 * Purpose:
 *      Gets the Inventory Data Repository(IDR) information
 *
 * Detailed Description:
 *      - Fetches idr info structure from the private data area of
 *        inventory rdr of the specified resource_id
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT oa_soap_get_idr_info(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiIdrIdT idr,
                              SaHpiIdrInfoT *idr_info)
{
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;

        if (oh_handler == NULL || idr_info == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_INVENTORY_RDR,
                                 idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        memcpy(idr_info, &inventory->info.idr_info, sizeof(SaHpiIdrInfoT));
        return SA_OK;
}

/**
 * oa_soap_get_idr_area_header
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @area_type: Type of the inventory data area
 *      @area_id: Identifier of the area entry
 *      @next_area_id: Structure for receiving the next area of the requested
 *                     type
 *      @header: Structure for receiving idr information
 *
 * Purpose:
 *      Gets the Inventory Data Repository(IDR) area header information
 *      for a specific area
 *
 * Detailed Description:
 *      - This function gets the inventory data area header information
 *        of a particular area associated with IDR of resource_id
 *
 * Return values:
 *      SA_OK - Normal case.
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT oa_soap_get_idr_area_header(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiIdrIdT idr,
                                     SaHpiIdrAreaTypeT area_type,
                                     SaHpiEntryIdT area_id,
                                     SaHpiEntryIdT *next_area_id,
                                     SaHpiIdrAreaHeaderT *header)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        char *type;

        if (oh_handler == NULL || next_area_id == NULL || header == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether area_type supplied is in list of
         * valid area types specified by the framework
         */
        type = oh_lookup_idrareatype(area_type);
        if (type == NULL) {
                err("Invalid area type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (area_id == SAHPI_LAST_ENTRY) {
                err("Invalid area id.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

         /* Check whether the area list of the resource IDR is empty */
        if (inventory->info.idr_info.NumAreas == 0) {
                err("IDR Area not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Fetch the area header of IDR area of the specified area_id.
         * Next area shall contain reference to next area of
         * the same area type if existing, else it will be set to NULL
         */
        rv = fetch_idr_area_header(&(inventory->info), area_id, area_type,
                                   header, next_area_id);
        if (rv != SA_OK) {
                err("IDR Area not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }
        return rv;
}

/**
 * oa_soap_add_idr_area
 *      @oh_handler: Handler data pointer.
 *      @resource_id: Resource ID.
 *      @idr: IDR ID.
 *      @area_type: Type of the inventory data area.
 *      @area_id: Area id of the newly created area.
 *
 * Purpose:
 *      Creates the Inventory Data Repository(IDR) area of requested type
 *
 * Detailed Description:
 *      - Creates an IDR area of the specified area type and adds it to end of
 *        area list of the resource IDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_DATA - On Invalid area type
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 *      SA_ERR_HPI_OUT_OF_SPACE - Request failed due to insufficient memory
 **/
SaErrorT oa_soap_add_idr_area(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiIdrIdT idr,
                              SaHpiIdrAreaTypeT area_type,
                              SaHpiEntryIdT *area_id)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_inventory *inventory = NULL;
        char *type;

        if (oh_handler == NULL || area_id == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether area_type supplied is in list of
         * valid area types specified by the framework
         */
        type = oh_lookup_idrareatype(area_type);
        if (type == NULL) {
                err("Invalid area_type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* It is not valid to create the area of UNSPECIFIED type */
        if (area_type == SAHPI_IDR_AREATYPE_UNSPECIFIED) {
                err("Invalid area_type.");
                return SA_ERR_HPI_INVALID_DATA;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the resource IDR is read only */
        if (inventory->info.idr_info.ReadOnly == SAHPI_TRUE) {
                err("IDR is read only");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Create and add the new area to the end of resource IDR area list */
        rv = idr_area_add(&(inventory->info.area_list), area_type, &local_area);
        if (rv != SA_OK) {
                err("Addition of IDR area failed");
                if (rv == SA_ERR_HPI_OUT_OF_MEMORY) {
                        return SA_ERR_HPI_OUT_OF_SPACE;
                }
                return rv;
        }

        /* Increment area count in resource IDR  */
        inventory->info.idr_info.NumAreas++;

        /* Increment modification count of resource IDR */
        inventory->info.idr_info.UpdateCount++;

        *area_id = local_area->idr_area_head.AreaId;

        return SA_OK;
}

/**
 * oa_soap_add_idr_area_by_id:
 *      @oh_handler: Handler data pointer.
 *      @resource_id: Resource ID.
 *      @idr: IDR ID.
 *      @area_type: Type of the inventory data area.
 *      @area_id: Area id of the newly created area.
 *
 * Purpose:
 *      Creates the Inventory Data Repository(IDR) area of requested type
 *
 * Detailed Description:
 *      - Creates an IDR area of the specified area type and area id and adds
 *        to area list of the resource IDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource don't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 *      SA_ERR_HPI_DUPLICATE - Area ID already exists
 *      SA_ERR_HPI_OUT_OF_SPACE - Request failed due to insufficient memory
 **/
SaErrorT oa_soap_add_idr_area_by_id (void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiIdrIdT idr,
                                     SaHpiIdrAreaTypeT area_type,
                                     SaHpiEntryIdT area_id)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        SaHpiEntryIdT *next_area =  NULL;
        SaHpiIdrAreaHeaderT *area_header = NULL;
        char *type;

        if (oh_handler == NULL || area_id == SAHPI_LAST_ENTRY) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether supplied area_type is in list of
         * valid area types specified by the framework
         */
        type = oh_lookup_idrareatype(area_type);
        if (type == NULL) {
                err("Invalid area_type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* It is not valid to create the area of UNSPECIFIED type */
        if (area_type == SAHPI_IDR_AREATYPE_UNSPECIFIED) {
                err("Invalid area_type.");
                return SA_ERR_HPI_INVALID_DATA;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache,
                                resource_id,
                                rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the resource IDR is read only */
        if (inventory->info.idr_info.ReadOnly == SAHPI_TRUE) {
                err("IDR is read only");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Check if the Area ID already exists */
        rv = fetch_idr_area_header(&(inventory->info),
                                   area_id,area_type,
                                   area_header,
                                   next_area);
        if (rv == SA_OK) {
                err("AreaId already exists in the IDR");
                return SA_ERR_HPI_DUPLICATE;
        }

        /* Create and add the new area to the resource IDR area list */
        rv = idr_area_add_by_id(&(inventory->info.area_list),
                                area_type, area_id);
        if (rv != SA_OK) {
                err("Addition of IDR area failed");
                if (rv == SA_ERR_HPI_OUT_OF_MEMORY) {
                        return SA_ERR_HPI_OUT_OF_SPACE;
                }
                return rv;
        }

        /* Increment area count in resource IDR  */
        inventory->info.idr_info.NumAreas++;

        /* Increment modification count of resource IDR */
        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * oa_soap_del_idr_area
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @area_id: Area id of the newly created area
 *
 * Purpose:
 *      Deletes the Inventory Data Repository(IDR) area with specific identifier
 *
 * Detailed Description:
 *      - Check whether the IDR area of specified area id exists
 *      - If specified IDR area does not exist, then it is deleted from
 *        the area list else an appropriate error is returned
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_del_idr_area(void *oh_handler,
                              SaHpiResourceIdT resource_id,
                              SaHpiIdrIdT idr,
                              SaHpiEntryIdT area_id)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        if (area_id == SAHPI_LAST_ENTRY) {
                err("Invalid area id.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the resource IDR is read only */
        if (inventory->info.idr_info.ReadOnly == SAHPI_TRUE) {
                err("IDR is read only");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Delete the specified area if it exists, else return an error */
        rv = idr_area_delete(&(inventory->info.area_list), area_id);
        if (rv != SA_OK) {
                err("IDR Area not found");
                return rv;
        }

        /* Decrement area count in resource IDR  */
        inventory->info.idr_info.NumAreas--;

        /* Increment modification count of resource IDR */
        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * oa_soap_get_idr_field
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @area_id: Area id
 *      @field_type: Type of the IDR field
 *      @field_id: Identifier of the field to be retrieved
 *      @next_field_id: Identifier of the next field of the requested type
 *      @field: Structure to retrieve the field contents
 *
 * Purpose:
 *      Gets Inventory Data Repository(IDR) field from a particular IDR area
 *
 * Detailed Description:
 *      - Check whether the IDR field of the specified field id exists
 *      - If specified IDR field exists, then field structure is returned
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_get_idr_field(void *oh_handler,
                               SaHpiResourceIdT resource_id,
                               SaHpiIdrIdT idr,
                               SaHpiEntryIdT area_id,
                               SaHpiIdrFieldTypeT field_type,
                               SaHpiEntryIdT field_id,
                               SaHpiEntryIdT *next_field_id,
                               SaHpiIdrFieldT *field)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        char *type;

        if (oh_handler == NULL || next_field_id == NULL || field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ((area_id == SAHPI_LAST_ENTRY) || (field_id == SAHPI_LAST_ENTRY)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

         /* Check whether field_type supplied is in list of
          * valid field types specified by the framework
          */
        type = oh_lookup_idrfieldtype(field_type);
        if (type == NULL) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache,
                                 resource_id,
                                 SAHPI_INVENTORY_RDR,
                                 idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the area list of the resource IDR is empty */
        if (inventory->info.idr_info.NumAreas == 0) {
                err("IDR Area not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Fetch the IDR field of the specified field_id.
         * next_field shall contain reference to next field of
         * the same field type if is exists, else it will be set to NULL
         */
        rv = fetch_idr_field(&(inventory->info),
                           area_id,
                           field_type,
                           field_id,
                           next_field_id,
                           field);
        if (rv != SA_OK) {
                err("IDR Field not present");
                return rv;
        }
        return SA_OK;
}

/**
 * oa_soap_add_idr_field
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @field: structure containing the new field information
 *
 * Purpose:
 *      Add a field to a specified Inventory Data Repository(IDR) Area
 *
 * Detailed Description:
 *      - Creates an IDR field of the specified field type and adds to end of
 *        field list of the specified IDR area id in resource IDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_DATA - On Invalid field type
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 *      SA_ERR_HPI_OUT_OF_SPACE - Request failed due to insufficient memory
 **/
SaErrorT oa_soap_add_idr_field(void *oh_handler,
                               SaHpiResourceIdT resource_id,
                               SaHpiIdrIdT idr,
                               SaHpiIdrFieldT *field)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_area *local_area = NULL;
        char *type;

        if (oh_handler == NULL || field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        type = oh_lookup_idrfieldtype(field->Type);
        if (type == NULL) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* It is not valid to create the field of UNSPECIFIED type */
        if (field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_DATA;
        }
        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (inventory->info.idr_info.NumAreas == 0) {
                err("No areas in the specified IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Start traversing the area list of resource IDR from the
         * head node
         */
        local_area = inventory->info.area_list;
        while (local_area != NULL) {
                if ((field->AreaId == local_area->idr_area_head.AreaId)) {
                         break;
                }
                local_area = local_area->next_area;
        }

        /* If the area id specified in field structure does exist, then
         * local_area will point to that area, else it is NULL
         */
        if (!local_area) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the specified IDR area is read only */
        if (local_area->idr_area_head.ReadOnly == SAHPI_TRUE) {
                err("IDR Area is read only");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Create and add the new field to end of filed list in IDR area */
        rv = idr_field_add(&(local_area->field_list),
                           field);
        if (rv != SA_OK) {
                err("IDR field add failed");
                if (rv == SA_ERR_HPI_OUT_OF_MEMORY) {
                        return SA_ERR_HPI_OUT_OF_SPACE;
                }
                return rv;
        }

        /* Increment the field count in IDR area */
        local_area->idr_area_head.NumFields++;

        /* Increment the update cound of resource IDR */
        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * oa_soap_add_idr_field_by_id:
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @field: structure containing the new field information
 *
 * Purpose:
 *      Add a field to a specified Inventory Data Repository(IDR)
 *      Area with the specified field id
 *
 * Detailed Description:
 *      - Creates an IDR field of the specified field type and adds to
 *        field list of the specified IDR area id in resource IDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 *      SA_ERR_HPI_OUT_OF_SPACE - Request failed due to insufficient memory
 **/
SaErrorT oa_soap_add_idr_field_by_id(void *oh_handler,
                                     SaHpiResourceIdT resource_id,
                                     SaHpiIdrIdT idr_id,
                                     SaHpiIdrFieldT *field)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_area *local_area = NULL;
        char *type;

        if (oh_handler == NULL || field == NULL ||
            field->AreaId == SAHPI_LAST_ENTRY ||
            field->FieldId == SAHPI_LAST_ENTRY) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        type = oh_lookup_idrfieldtype(field->Type);
        if (type == NULL) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* It is not valid to create the field of UNSPECIFIED type */
        if (field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr_id);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache,
                                resource_id,
                                rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (inventory->info.idr_info.NumAreas == 0) {
                err("No areas in the specified IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Start traversing the area list of resource IDR from the
         * head node of the area linked list
         */
        local_area = inventory->info.area_list;
        while (local_area != NULL) {
                if ((field->AreaId == local_area->idr_area_head.AreaId)) {
                        break;
                }
                local_area = local_area->next_area;
        }

        /* If the area id specified in field structure is existing, then
         * local_area will point to that area, else it is NULL
         */
        if (!local_area) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the specified IDR area is read only */
        if (local_area->idr_area_head.ReadOnly == SAHPI_TRUE) {
                err("IDR Area is read only");
        }

        /* Create and add the new field to  field list in IDR area */
        rv = idr_field_add_by_id(&(local_area->field_list),
                                 field->AreaId,
                                 field->Type,
                                 (char *)field->Field.Data,
                                 field->FieldId);
        if (rv != SA_OK) {
                err("IDR field add failed");
                if (rv == SA_ERR_HPI_OUT_OF_MEMORY) {
                        return SA_ERR_HPI_OUT_OF_SPACE;
                }
                return rv;
        }

        /* Increment the field count in IDR area */
        local_area->idr_area_head.NumFields++;

        /* Increment the update cound of resource IDR */
        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * oa_soap_set_idr_field
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @field: Structure containing the modification field information
 *
 * Purpose:
 *      Updates the specified Inventory Data Repository(IDR) field
 *
 * Detailed Description:
 *      - Updates the field contents of the IDR field of specified
 *        field id from field list of the specified IDR area id in
 *        resource IDR
 *      - If the specified field does not exist, then an appropriate error is
 *        returned
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_set_idr_field(void *oh_handler,
                               SaHpiResourceIdT resource_id,
                               SaHpiIdrIdT idr,
                               SaHpiIdrFieldT *field)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_area *local_area = NULL;
        char *type;

        if (oh_handler == NULL || field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check whether the specified field type is in valid list
         * of field types
         */
        type = oh_lookup_idrfieldtype(field->Type);
        if (type == NULL) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* It is invalid to specify the field type as
         * SAHPI_IDR_FIELDTYPE_UNSPECIFIED
         */
        if (field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
                err("Invalid field type.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (inventory->info.idr_info.NumAreas == 0) {
                err("No areas in the specified IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Start traversing the area list of resource IDR from the
         * head node of the area linked list
         */
        local_area = inventory->info.area_list;
        while (local_area != NULL) {
                if ((field->AreaId == local_area->idr_area_head.AreaId)) {
                         break;
                }
                local_area = local_area->next_area;
        }

        /* If the area id specified in field structure exists, then
         * local_area will point to that area, else it is NULL
         */
        if (!local_area) {
                err("IDR area not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Update the specified field with latest data */
        rv = idr_field_update(local_area->field_list, field);
        if (rv != SA_OK) {
                err("IDR field update failed");
                return rv;
        }

        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * oa_soap_del_idr_field
 *      @oh_handler: Handler data pointer
 *      @resource_id: Resource ID
 *      @idr: IDR ID
 *      @area_id: Area id
 *      @field_id: Identifier of the field to be deleted
 *
 * Purpose:
 *      Deletes the Inventory Data Repository(IDR) field with
 *      specific identifier from IDR area
 *
 * Detailed Description:
 *      - Deletes an IDR field of the specified field id from field list
 *        of the specified IDR area id in resource IDR
 *      - If the specified field does not exist, then an appropriate error is
 *        returned
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_INVENTORY
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INVALID_RESOURCE - Invalid resource id specified
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_READ_ONLY - The data to be operated upon is read only
 **/
SaErrorT oa_soap_del_idr_field(void *oh_handler,
                               SaHpiResourceIdT resource_id,
                               SaHpiIdrIdT idr,
                               SaHpiEntryIdT area_id,
                               SaHpiEntryIdT field_id)
{
        SaErrorT rv = SA_OK;
        struct oh_handler_state *handler;
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_area *local_area = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ((area_id == SAHPI_LAST_ENTRY) || (field_id == SAHPI_LAST_ENTRY)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *)oh_handler;

        rpt = oh_get_resource_by_id(handler->rptcache, resource_id);
        if (!rpt) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }
        if (! (rpt->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)) {
                err("INVALID RESOURCE CAPABILITY");
                return SA_ERR_HPI_CAPABILITY;
        }

        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR, idr);
        if (rdr == NULL) {
                err("INVALID RDR NUMBER");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. idr=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (inventory->info.idr_info.NumAreas == 0) {
                err("No areas in the specified IDR");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Start traversing the area list of resource IDR from the
         * head node of the area linked list
         */
        local_area = inventory->info.area_list;
        while (local_area != NULL) {
                if ((area_id == local_area->idr_area_head.AreaId)) {
                         break;
                }
                local_area = local_area->next_area;
        }
        if (!local_area) {
                err("IDR Area not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* Check whether the specified IDR area is read only */
        if (local_area->idr_area_head.ReadOnly == SAHPI_TRUE) {
                err("IDR area is read only");
                return SA_ERR_HPI_READ_ONLY;
        }

        /* Delete the specified field from the field list*/
        rv = idr_field_delete(&(local_area->field_list),
                              field_id);
        if (rv != SA_OK) {
                return rv;
        }

        /* Decrement the field count in IDR area */
        local_area->idr_area_head.NumFields--;
        inventory->info.idr_info.UpdateCount++;

        return SA_OK;
}

/**
 * build_enclosure_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @response: Pointer to get enclosure info response data structure
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for enclosure
 *
 * Detailed Description:
 *      - Populates the enclosure inventory rdr with default values
 *      - Inventory data repository is created and associated in the private
 *        data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_enclosure_inv_rdr(struct oh_handler_state *oh_handler,
                                 struct enclosureInfo *response,
                                 SaHpiRdrT *rdr,
                                 struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        char enclosure_inv_str[] = ENCLOSURE_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T product_area_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || response == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->oa_soap_resources.enclosure_rid;
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populating the inventory rdr with default values and resource name */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(response->name) + 1;
        snprintf((char *)rdr->IdString.Data,
                  strlen(response->name) + 1,
                  "%s", response->name);

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment =
                (char *)g_malloc0(strlen(enclosure_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(enclosure_inv_str) + 1,
                 "%s", enclosure_inv_str);

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response->name,
                              response->manufacturer,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                product_area_success_flag = SAHPI_TRUE;
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add chassis area if resource part number and/or
         * serial number exist
         */
        rv = add_chassis_area(&local_inventory->info.area_list,
                              response->partNumber,
                              response->serialNumber,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add chassis area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add internal area if all/atleast one of required
         * information of resource for internal is available
         */
        rv = add_internal_area(&local_inventory->info.area_list,
                               response->interposerManufacturer,
                               response->interposerName,
                               response->interposerPartNumber,
                               response->interposerSerialNumber,
                               &add_success_flag);
        if (rv != SA_OK) {
                err("Add internal area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }
        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;

        /* Adding the product version in IDR product area.  It is added at
         * the end of the field list.
         */
         if (product_area_success_flag == SAHPI_TRUE) {
                /* Add the product version field if the enclosure hardware info
                 * is available
                 */
                if (response->hwVersion != NULL) {
                        memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                        hpi_field.AreaId = local_inventory->info.area_list->
                                           idr_area_head.AreaId;
                        hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION;
                        strcpy ((char *)hpi_field.Field.Data,
                                response->hwVersion);

                        rv = idr_field_add(&(local_inventory->info.area_list
                                           ->field_list),
                                           &hpi_field);
                        if (rv != SA_OK) {
                                err("Add idr field failed");
                                return rv;
                        }

                        /* Increment the field counter */
                        local_inventory->info.area_list->idr_area_head.
                        NumFields++;
                }
        }
        return SA_OK;
}

/**
 * build_oa_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @response: Handler data pointer
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for Onboard Administator
 *
 * Detailed Description:
 *      - Populates the OA inventory rdr with default values
 *      - Inventory data repository is created and associated in the private
 *        data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_oa_inv_rdr(struct oh_handler_state *oh_handler,
                          struct oaInfo *response,
                          SaHpiRdrT *rdr,
                          struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        char oa_inv_str[] = OA_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T product_area_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || response == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->
                oa_soap_resources.oa.resource_id[response->bayNumber - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populating the inventory rdr with default values and resource name */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(response->name) + 1;
        snprintf((char *)rdr->IdString.Data,
                        strlen(response->name)+ 1,"%s",
                        response->name );

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment = (char *)g_malloc0(strlen(oa_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(oa_inv_str) + 1,
                 "%s", oa_inv_str);

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response->name,
                              response->manufacturer,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                product_area_success_flag = SAHPI_TRUE;
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add board area if resource part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response->partNumber,
                            response->serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;

        /* Adding the product version in IDR product area.  It is added at
         * the end of the field list.
         */
         if (product_area_success_flag == SAHPI_TRUE) {
                /* Add the product version field if the firmware info
                 * is available
                 */
                if (response->fwVersion != NULL) {
                        memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                        hpi_field.AreaId = local_inventory->info.area_list->
                                           idr_area_head.AreaId;
                        hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION;
                        strcpy ((char *)hpi_field.Field.Data,
                                response->fwVersion);

                        rv = idr_field_add(&(local_inventory->info.area_list
                                           ->field_list),
                                           &hpi_field);
                        if (rv != SA_OK) {
                                err("Add idr field failed");
                                return rv;
                        }

                        /* Increment the field counter */
                        local_inventory->info.area_list->idr_area_head.
                        NumFields++;
                }
        }
        return SA_OK;
}

/**
 * build_server_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the SOAP_CON
 *      @bay_number: Bay number of the server
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for server blade
 *
 * Detailed Description:
 *      - Populates the server inventory rdr with default values
 *      - Inventory data repository is created and associated in the private
 *        data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_server_inv_rdr(struct oh_handler_state *oh_handler,
                              SOAP_CON *con,
                              SaHpiInt32T bay_number,
                              SaHpiRdrT *rdr,
                              struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        char server_inv_str[] = SERVER_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T product_area_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct getBladeInfo request;
        struct bladeInfo response;
        struct getBladeMpInfo blade_mp_request;
        struct bladeMpInfo blade_mp_response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || con == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (!rpt) {
                err("Could not find blade resource rpt");
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }
        rdr->Entity = rpt->ResourceEntity;

        request.bayNumber = bay_number;
        rv = soap_getBladeInfo(con,
                               &request, &response);
        if (rv != SOAP_OK) {
                err("Get blade info failed");
                return rv;
        }

        /* Populating the inventory rdr with rpt values for the resource */
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(response.name) + 1;
        snprintf((char *)rdr->IdString.Data,
                        strlen(response.name) + 1,"%s",
                        response.name );

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment =
                (char *)g_malloc0(strlen(server_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(server_inv_str) + 1,
                 "%s", server_inv_str);

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response.name,
                              response.manufacturer,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                product_area_success_flag = SAHPI_TRUE;
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add board area if resource part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response.partNumber,
                            response.serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }
        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;

        /* Adding the product version in IDR product area.  It is added at
         * the end of the field list.
         */
         if (product_area_success_flag == SAHPI_TRUE) {
                /* Making getBladeMpInfo soap call for getting the
                 * product version
                 */
                blade_mp_request.bayNumber = bay_number;
                rv = soap_getBladeMpInfo(con,
                               &blade_mp_request, &blade_mp_response);
                if (rv != SOAP_OK) {
                        err("Get blade mp info failed");
                        return rv;
                }

                /* Add the product version field if the firmware info
                 * is available
                 */
                if (blade_mp_response.fwVersion != NULL) {
                        memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                        hpi_field.AreaId = local_inventory->info.area_list->
                                           idr_area_head.AreaId;
                        hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION;
                        strcpy ((char *)hpi_field.Field.Data,
                                blade_mp_response.fwVersion);

                        rv = idr_field_add(&(local_inventory->info.area_list
                                           ->field_list),
                                           &hpi_field);
                        if (rv != SA_OK) {
                                err("Add idr field failed");
                                return rv;
                        }

                        /* Increment the field counter */
                        local_inventory->info.area_list->idr_area_head.
                        NumFields++;
                }
        }
        return SA_OK;
}

/**
 * build_inserted_server_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @bay_number: Bay number of the inserted server
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *        Creates an basic inventory rdr without area information for server
 *        blade when it is inserted
 *
 * Detailed Description:
 *      - Populates the server inventory rdr with default values when server is
 *        inserted
 *      - When the server blade is inserted, inventory information is not
 *        available until it stabilizes, a seperate event is sent by OA when
 *        the server stabilizes
 *        Hence, the inventory IDR creation is done in two stages:
 *        Stage 1: Create the IDR header when the server is inserted and
 *        keep area list empty. This stage1 functionality is achieved by this
 *        module
 *        Stage 2: When OA intimates that the server is stable, then retrieve
 *        inventory information and create appropriate area list and attach it
 *        to the IDR created in stage 1. This stage 2 functionality is achieved
 *        by "build_server_inventory_area" module
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_inserted_server_inv_rdr(struct oh_handler_state *oh_handler,
                                       SaHpiInt32T bay_number,
                                       SaHpiRdrT *rdr,
                                       struct oa_soap_inventory **inventory)
{
        char server_inv_str[] = SERVER_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || rdr == NULL || inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.server.resource_id[bay_number - 1];
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (!rpt) {
                err("Could not find blade resource rpt");
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }
        rdr->Entity = rpt->ResourceEntity;

        /* Populating the inventory rdr with default values and resource name */
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(server_inv_str) + 1;
        snprintf((char *)rdr->IdString.Data, strlen(server_inv_str) + 1,"%s",
                 server_inv_str);

        /* Create inventory IDR and populate the IDR header and keep
         * area list as empty
         */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment =
                (char *)g_malloc0(strlen(server_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(server_inv_str) + 1,
                 "%s", server_inv_str);

        *inventory = local_inventory;
        return SA_OK;
}

/**
 * build_server_inventory_area
 *      @con: Pointer to the SOAP_CON
 *      @response: Handler data pointer
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Builds the server IDR areas and attaches these area to already created
 *      inventory rdr of server
 *
 * Detailed Description:
 *      - Populates the server inventory rdr's IDR with available inventory
 *        information
 *      - When the server blade is inserted, inventory information is not
 *        available until it stabilizes, a seperate event is sent by OA when
 *        the server stabilizes
 *        Hence, the inventory IDR creation is done in two stages:
 *        Stage 1: Create the IDR header when the server is inserted and
 *        keep area list empty, this functionality is achieved by
 *        "build_inserted_server_inv_rdr" module
 *        Stage 2: When OA intimates that the server is stable, then retrieve
 *        inventory information and create appropriate area list and attach it
 *        to the IDR created in stage 1. This stage 2 functionality is achieved
 *        by this module
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_server_inventory_area(SOAP_CON *con,
                                     struct bladeInfo *response,
                                     SaHpiRdrT *rdr,
                                     struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        struct oa_soap_inventory *local_inventory = *inventory;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T product_area_success_flag = 0;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct getBladeMpInfo blade_mp_request;
        struct bladeMpInfo blade_mp_response;

        if (response == NULL || rdr == NULL || inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response->name,
                              response->manufacturer,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                product_area_success_flag = SAHPI_TRUE;
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add board area if resource's part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response->partNumber,
                            response->serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;

        /* Adding the product version in IDR product area.  It is added at
         * the end of the field list.
         */
         if (product_area_success_flag == SAHPI_TRUE) {
                /* Making getBladeMpInfo soap call for getting the
                 * product version
                 */
                blade_mp_request.bayNumber = response->bayNumber;
                rv = soap_getBladeMpInfo(con,
                               &blade_mp_request, &blade_mp_response);
                if (rv != SOAP_OK) {
                        err("Get blade mp info failed");
                        return rv;
                }

                /* Add the product version field if the firmware info
                 * is available
                 */
                if (blade_mp_response.fwVersion != NULL) {
                        memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                        hpi_field.AreaId = local_inventory->info.area_list->
                                           idr_area_head.AreaId;
                        hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION;
                        strcpy ((char *)hpi_field.Field.Data,
                                blade_mp_response.fwVersion);

                        rv = idr_field_add(&(local_inventory->info.area_list
                                           ->field_list),
                                           &hpi_field);
                        if (rv != SA_OK) {
                                err("Add idr field failed");
                                return rv;
                        }

                        /* Increment the field counter */
                        local_inventory->info.area_list->idr_area_head.
                        NumFields++;
                }
        }
        return SA_OK;
}

/**
 * build_interconnect_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @con: Pointer to the SOAP_CON
 *      @bay_number: Bay number of the server
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for interconnect blade
 *
 * Detailed Description:
 *      - Populates the interconnect inventory rdr with default values
 *      - Inventory data repository is created and associated as part of the
 *        private data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_interconnect_inv_rdr(struct oh_handler_state *oh_handler,
                                    SOAP_CON *con,
                                    SaHpiInt32T bay_number,
                                    SaHpiRdrT *rdr,
                                    struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        char interconnect_inv_str[] = INTERCONNECT_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct getInterconnectTrayInfo request;
        struct interconnectTrayInfo response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || con == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->
                oa_soap_resources.interconnect.resource_id[bay_number - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        request.bayNumber = bay_number;
        rv = soap_getInterconnectTrayInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get Interconnect tray info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populating the inventory rdr with rpt values for the resource */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(response.name) + 1;
        snprintf((char *)rdr->IdString.Data,
                strlen(response.name)+ 1,
                "%s",response.name );

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment =
                (char *)g_malloc0(strlen(interconnect_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(interconnect_inv_str) + 1,
                 "%s", interconnect_inv_str);

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response.name,
                              response.manufacturer,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add board area if resource part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response.partNumber,
                            response.serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;
        return SA_OK;
}

/**
 * build_fan_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @response: Pointer to the fan info response
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for fan
 *
 * Detailed Description:
 *      - Populates the fan inventory rdr with default values
 *      - Inventory data repository is created and associated as part of the
 *        private data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_fan_inv_rdr(struct oh_handler_state *oh_handler,
                           struct fanInfo *response,
                           SaHpiRdrT *rdr,
                           struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        char fan_inv_str[] = FAN_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || response == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id = oa_handler->
                oa_soap_resources.fan.resource_id[response->bayNumber - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populating the inventory rdr with default values and resource name */
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->Entity = rpt->ResourceEntity;
        rdr->RecordId = 0;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(response->name) + 1;
        snprintf((char *)rdr->IdString.Data,
                strlen(response->name)+ 1, "%s",
                response->name );

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment = (char *)g_malloc0(strlen(fan_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(fan_inv_str) + 1,
                 "%s", fan_inv_str);

        /* Create and add product area if resource name and/or manufacturer
         * information exist
         */
        rv = add_product_area(&local_inventory->info.area_list,
                              response->name,
                              NULL,
                              &add_success_flag);
        if (rv != SA_OK) {
                err("Add product area failed");
                return rv;
        }

        /* add_success_flag will be true if product area is added,
         * if this is the first successful creation of IDR area, then have
         * area pointer stored as the head node for area list
         */
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        /* Create and add board area if resource part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response->partNumber,
                            response->serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;
        return SA_OK;
}

/**
 * build_power_inv_rdr
 *      @oh_handler: Handler data pointer
 *      @response: Pointer to the power sypply info response
 *      @rdr: Rdr Structure for inventory data
 *      @inventory: Rdr private data structure
 *
 * Purpose:
 *      Creates an inventory rdr for power supply
 *
 * Detailed Description:
 *      - Populates the power supply inventory rdr with default values
 *      - Inventory data repository is created and associated as part of the
 *        private data area of the Inventory RDR
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - oa_soap plugin has encountered an internal
 *                                  error
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT build_power_inv_rdr(struct oh_handler_state *oh_handler,
                             struct powerSupplyInfo *response,
                             SaHpiRdrT *rdr,
                             struct oa_soap_inventory **inventory)
{
        SaErrorT rv = SA_OK;
        char *power_rdr_str = POWER_SUPPLY_RDR_STRING;
        char power_inv_str[] = POWER_SUPPLY_INVENTORY_STRING;
        struct oa_soap_inventory *local_inventory = NULL;
        struct oa_soap_area *head_area = NULL;
        SaHpiInt32T add_success_flag = 0;
        SaHpiInt32T area_count = 0;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;
        SaHpiRptEntryT *rpt = NULL;

        if (oh_handler == NULL || response == NULL || rdr == NULL ||
            inventory == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        resource_id =
           oa_handler->oa_soap_resources.ps_unit.resource_id[response->bayNumber - 1];
        /* Get the rpt entry of the resource */
        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("resource RPT is NULL");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populating the inventory rdr with default values and resource name */
        rdr->Entity = rpt->ResourceEntity;
        rdr->RecordId = 0;
        rdr->RdrType  = SAHPI_INVENTORY_RDR;
        rdr->RdrTypeUnion.InventoryRec.IdrId = SAHPI_DEFAULT_INVENTORY_ID;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(power_rdr_str) + 1;
        snprintf((char *)rdr->IdString.Data,
                strlen(power_rdr_str)+ 1,
                "%s", power_rdr_str);

        /* Create inventory IDR and populate the IDR header */
        local_inventory = (struct oa_soap_inventory*)
                g_malloc0(sizeof(struct oa_soap_inventory));
        if (!local_inventory) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        local_inventory->inv_rec.IdrId = rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.IdrId =
                rdr->RdrTypeUnion.InventoryRec.IdrId;
        local_inventory->info.idr_info.UpdateCount = 1;
        local_inventory->info.idr_info.ReadOnly = SAHPI_FALSE;
        local_inventory->info.idr_info.NumAreas = 0;
        local_inventory->info.area_list = NULL;
        local_inventory->comment = (char *)g_malloc0(strlen(power_inv_str) + 1);
        snprintf(local_inventory->comment, strlen(power_inv_str) + 1,
                 "%s", power_inv_str);

        /* Create and add board area if resource part number and/or
         * serial number exist
         */
        rv = add_board_area(&local_inventory->info.area_list,
                            response->modelNumber,
                            response->serialNumber,
                            &add_success_flag);
        if (rv != SA_OK) {
                err("Add board area failed");
                return rv;
        }
        if (add_success_flag != SAHPI_FALSE) {
                (local_inventory->info.idr_info.NumAreas)++;
                if (area_count == 0) {
                        head_area = local_inventory->info.area_list;
                }
                ++area_count;
        }

        local_inventory->info.area_list = head_area;
        *inventory = local_inventory;
        return SA_OK;
}

/**
 * add_product_area
 *      @area: IDR area pointer
 *      @name: Resource name
 *      @manufacturer: Resource manufacturer
 *      @success_flag: Flag for checking area creation
 *
 * Purpose:
 *      Creates an IDR product area with required fields
 *
 * Detailed Description:
 *      - Checks whether the name and manufacturer details are available for
 *        the resource which has called this module
 *      - If either or both of these information is available then IDR area
 *        of Product info type is created and these informations are added as
 *        individual IDR fields in the newly create product area
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT add_product_area(struct oa_soap_area **area,
                          char *name,
                          char *manufacturer,
                          SaHpiInt32T *success_flag)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_field *field = NULL;
        struct oa_soap_field *head_field = NULL;
        SaHpiInt32T field_count = 0;

        if (area == NULL || success_flag == NULL) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* If both name and manufacturer information is NULL
         * then product area is not created
         */
        if (name == NULL && manufacturer == NULL) {
                err("Product Area:Required information not available");
                err("Product area not created");
                *success_flag = SAHPI_FALSE;
                return SA_OK;
        }

        /* Create area of type PRODUCT INFO */
        rv = idr_area_add(area,
                          SAHPI_IDR_AREATYPE_PRODUCT_INFO,
                          &local_area);
        if (rv != SA_OK) {
                err("Add idr area failed");
                return rv;
        }
        *success_flag = SAHPI_TRUE;

        /* Add the fields to the newly created product area */
        field = local_area->field_list;
        if (name != NULL) {
        /* Add product name field to the IDR product area */
                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_NAME;
                strcpy ((char *)hpi_field.Field.Data, name);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;

                /* if this is the first successful creation of IDR field in
                 * the IDR area, then have field pointer stored as the head
                 * node for field list
                 */
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        if (manufacturer != NULL) {

                /* Add manufacturer field to the IDR product area */
                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_MANUFACTURER;
                strcpy ((char *)hpi_field.Field.Data, manufacturer);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        local_area->field_list = head_field;
        return SA_OK;
}

/**
 * add_chassis_area
 *      @area: IDR area pointer
 *      @part_number: Resource part number
 *      @serial_number: Resource serial_number
 *      @success_flag: Flag for checking area creation
 *
 * Purpose:
 *      Creates an IDR chassis area with required fields
 *
 * Detailed Description:
 *      - Checks whether the part number and serial number details are
 *        available for the resource which has called this module
 *      - If either or both of these information is available then IDR area
 *        of Chassis info type is created and these informations are added as
 *        individual IDR fields in the newly create chassis area
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT add_chassis_area(struct oa_soap_area **area,
                          char *part_number,
                          char *serial_number,
                          SaHpiInt32T *success_flag)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_field *field = NULL;
        struct oa_soap_field *head_field = NULL;
        SaHpiInt32T field_count = 0;

        if (area == NULL || success_flag == NULL) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* If both part number and serial number information is NULL
         * then chassis area is not created
         */
        if (part_number == NULL && serial_number == NULL) {
                err("Chassis Area:Required information not available");
                err("Chassis area not created");
                *success_flag = SAHPI_FALSE;
                return SA_OK;
        }

        rv = idr_area_add(area,
                          SAHPI_IDR_AREATYPE_CHASSIS_INFO,
                          &local_area);

        if (rv != SA_OK) {
                err("Add idr area failed");
                return rv;
        }
        field_count = 0;
        *success_flag = SAHPI_TRUE;

        /* Add the fields to the newly created chassis area */
        field = local_area->field_list;
        if (part_number != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_PART_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, part_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        if (serial_number != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, serial_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        local_area->field_list = head_field;
        return SA_OK;
}

/**
 * add_board_area
 *      @area: IDR area pointer
 *      @part_number: Resource part number
 *      @serial_number: Resource serial_number
 *      @success_flag: Flag for checking area creation
 *
 * Purpose:
 *      Creates an IDR board area with required fields
 *
 * Detailed Description:
 *      - Checks whether the part number and serial number details are
 *        available for the resource which has called this module
 *      - If either or both of these information is available then IDR area
 *        of board info type is created and these informations are added as
 *        individual IDR fields in the newly create broad area
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT add_board_area(struct oa_soap_area **area,
                        char *part_number,
                        char *serial_number,
                        SaHpiInt32T *success_flag)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_field *field = NULL;
        struct oa_soap_field *head_field = NULL;
        SaHpiInt32T field_count = 0;

        if (area == NULL || success_flag == NULL) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* If both part number and serial number information is NULL
         * then board area is not created
         */
        if (part_number == NULL && serial_number == NULL) {
                err("Board Area:Required information not available");
                err("Board area not created");
                *success_flag = SAHPI_FALSE;
                return SA_OK;
        }

        rv = idr_area_add(area,
                          SAHPI_IDR_AREATYPE_BOARD_INFO,
                          &local_area);
        if (rv != SA_OK) {
                err("Add idr area failed");
                return rv;
        }
        *success_flag = SAHPI_TRUE;

        field_count = 0;

        /* Add the fields to the newly created product area */
        field = local_area->field_list;
        if (part_number != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_PART_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, part_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        if (serial_number != NULL) {
                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, serial_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        local_area->field_list = head_field;
        return SA_OK;
}

/**
 * add_internal_area
 *      @area: IDR area pointer
 *      @manufacturer: Resource manufacturer
 *      @name: Resource name
 *      @part_number: Resource part number
 *      @serial_number: Resource serial_number
 *      @success_flag: Flag for checking area creation
 *
 * Purpose:
 *      Creates an IDR internal area with required fields
 *
 * Detailed Description:
 *      - Checks whether the required details for internal area are
 *        available for the enclosure resource which has called this module
 *      - If any of these information is available then IDR area
 *        of INTERNAL USE type is created and available informations are added
 *        as individual IDR fields in the newly create chassis area
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT add_internal_area(struct oa_soap_area **area,
                           char *manufacturer,
                           char *name,
                           char *part_number,
                           char *serial_number,
                           SaHpiInt32T *success_flag)
{
        SaErrorT rv = SA_OK;
        SaHpiIdrFieldT hpi_field;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_field *field = NULL;
        struct oa_soap_field *head_field = NULL;
        SaHpiInt32T field_count = 0;

        if (area == NULL || success_flag == NULL) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* If none of the required inventory info is availble, then INTERNAL
         * area is not created
         */
        if (manufacturer == NULL && name == NULL &&
           part_number == NULL && serial_number == NULL) {
                err("Internal Area:Required information not available");
                err("Internal area not created");
                *success_flag = SAHPI_FALSE;
                return SA_OK;
        }

        /* Create IDR area of INTERNAL USE type */
        rv = idr_area_add(area,
                          (SaHpiIdrAreaTypeT)SAHPI_IDR_AREATYPE_INTERNAL_USE,
                          &local_area);
        if (rv != SA_OK) {
                err("Add idr area failed");
                return rv;
        }
        *success_flag = SAHPI_TRUE;

        field_count = 0;

        /* Add the fields to the newly created internal use area */
        field = local_area->field_list;
        if (manufacturer != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_MANUFACTURER;
                strcpy ((char *)hpi_field.Field.Data, manufacturer);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }

        if (name != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_PRODUCT_NAME;
                strcpy ((char *)hpi_field.Field.Data, name);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }

        if (part_number != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_PART_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, part_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                        head_field = local_area->field_list = field;
                }
                local_area->idr_area_head.NumFields++;
        }

        if (serial_number != NULL) {

                memset(&hpi_field, 0, sizeof(SaHpiIdrFieldT));
                hpi_field.AreaId = local_area->idr_area_head.AreaId;
                hpi_field.Type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER;
                strcpy ((char *)hpi_field.Field.Data, serial_number);

                rv = idr_field_add(&(local_area->field_list),
                                   &hpi_field);
                if (rv != SA_OK) {
                        err("Add idr field failed");
                        return rv;
                }
                ++field_count;
                if (field_count == 1) {
                        head_field = field = local_area->field_list;
                }
                local_area->idr_area_head.NumFields++;
        }
        local_area->field_list = head_field;
        return SA_OK;
}

/**
 * idr_area_add
 *      @head_area: Pointer to IDR area
 *      @area_type: Type of IDR area
 *      @area: Pointer to new allocated area
 *
 * Purpose:
 *      Adds an Inventory Data Repository(IDR) area to Inventory data repository
 *      and returns the area pointer
 *
 * Detailed Description:
 *      - Creates an IDR area of the specified type
 *        If the area list for the resource IDR exists, then the
 *        newly created area will be added to end of area list
 *        If the area list is empty then the created area will become head node
 *        (first area) for the area list
 *      - Area id is will start from 0  and will increment for every new area
 *        added
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT idr_area_add(struct oa_soap_area **head_area,
                      SaHpiIdrAreaTypeT area_type,
                      struct oa_soap_area **area)
{
        struct oa_soap_area *local_area = NULL;
        SaHpiEntryIdT local_area_id;

        if (head_area == NULL || area == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_area = *head_area;
        /* Check whether the area list is empty */
        if (local_area == NULL) {
                local_area = (struct oa_soap_area*)
                        g_malloc0(sizeof(struct oa_soap_area));
                if (!local_area) {
                        err("OA SOAP out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                /* Create the area and make it as head node(first area) in
                 * the area list
                 */
                *head_area = local_area;
                local_area_id = 0;
        } else {
                /* Area list is not empty, traverse to the end of the list
                 * and add the IDR area
                 */
                while (local_area->next_area != NULL) {
                        local_area = local_area->next_area;
                }
                local_area->next_area = (struct oa_soap_area*)
                        g_malloc0(sizeof(struct oa_soap_area));
                if (!local_area->next_area) {
                        err("OA SOAP out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                local_area_id = local_area->idr_area_head.AreaId + 1;
                local_area = local_area->next_area;
        }
        /* Initialize the area with specified area type and generated area id
         */
        local_area->idr_area_head.AreaId = local_area_id;
        local_area->idr_area_head.Type = area_type;
        local_area->idr_area_head.ReadOnly = SAHPI_FALSE;
        local_area->idr_area_head.NumFields = 0;
        local_area->field_list = NULL;
        local_area->next_area = NULL;

        *area = local_area;
        return SA_OK;
}

/**
 * idr_area_add_by_id:
 *      @head_area: Pointer to IDR area
 *      @area_type: Type of IDR area
 *      @area_id: area id to be added
 *
 * Purpose:
 *      Adds an Inventory Data Repository(IDR) area to Inventory data repository
 *      with the specified area id
 *
 * Detailed Description:
 *      - Creates an IDR area of a specified type with specified id and adds it
 *        to Inventory Data Repository(IDR).
 *        If the area list is empty then the created area will become head node
 *        (first area) for the area list.
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - Input parameters are not valid
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT idr_area_add_by_id(struct oa_soap_area **head_area,
                            SaHpiIdrAreaTypeT area_type,
                            SaHpiEntryIdT area_id)
{
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_area *temp_area = NULL;

        if (head_area == NULL || area_id == SAHPI_LAST_ENTRY) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        temp_area = *head_area;
        local_area = (struct oa_soap_area*)g_malloc0(
                      sizeof(struct oa_soap_area));
        if (!local_area) {
               err("OA SOAP out of memory");
               return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* Initialize the area with specified area type and ID */
        local_area->idr_area_head.AreaId = area_id;
        local_area->idr_area_head.Type = area_type;
        local_area->idr_area_head.ReadOnly = SAHPI_FALSE;
        local_area->idr_area_head.NumFields = 0;
        local_area->field_list = NULL;

        /* Check whether the area list is empty  or if the new area
         * is to be inserted before first area
         */
        if (*head_area == NULL ||
            (*head_area)->idr_area_head.AreaId > area_id) {
                *head_area = local_area;
                (*head_area)->next_area = temp_area;
        } else {
                /* Traverse through the area list and insert the area
                 * at appropriate place
                 */
                while (temp_area != NULL) {
                        if ((temp_area->idr_area_head.AreaId < area_id) &&
                                ((temp_area->next_area == NULL) ||
                                 (temp_area->next_area->idr_area_head.AreaId >
                                  area_id))) {
                                local_area->next_area = temp_area->next_area;
                                temp_area->next_area = local_area;
                                break;
                        }
                        temp_area = temp_area->next_area;
                }
        }
        return SA_OK;
}

/**
 * idr_area_delete
 *      @head_area: Pointer to IDR area
 *      @area_id: Identifier of the area to be deleted
 *
 * Purpose:
 *      Deletes an Inventory Data Repository(IDR) area from Inventory data
 *      repository
 *
 * Detailed Description:
 *      - Deleted an IDR area of a specified id if it exists in the area
 *        list else an appropriate error will be returned
 *      - If the specified area id exists, then all the IDR fields in IDR
 *        area is deleted first and then the IDR area is deleted
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT idr_area_delete(struct oa_soap_area **head_area,
                         SaHpiEntryIdT area_id)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_area *tmp_area = NULL;
        struct oa_soap_area *next_area = NULL;
        SaHpiInt32T count = -1;

        if (head_area == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_area = *head_area;
        /* If area list is empty, then return error */
        if (local_area == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        } else {
                /* Check whether specified area id matches with first area id */
                if (area_id == local_area->idr_area_head.AreaId) {
                        /* If the specified IDR area is read only, then delete
                         * will be failed and READ ONLY error will be returned
                         */
                        if (local_area->idr_area_head.ReadOnly == SAHPI_TRUE) {
                                return SA_ERR_HPI_READ_ONLY;
                        }
                        tmp_area = local_area;
                        /* If the specified area id is found,
                         * then traverse the IDR field list of this area and
                         * delete the fields
                         */
                        for (count = 0;
                             count < local_area->idr_area_head.NumFields;
                             count++) {
                                rv = idr_field_delete(&(local_area->field_list),
                                                      local_area->field_list->
                                                              field.FieldId);
                                if (rv != SA_OK) {
                                        return rv;
                                }
                        }
                        tmp_area = local_area;
                        local_area = local_area->next_area;
                        *head_area = local_area;
                        g_free(tmp_area);
                        return SA_OK;
                }
                /* Traverse the area list to find the specified IDR area */
                while (local_area->next_area) {
                        next_area = local_area->next_area;
                        if (area_id == next_area->idr_area_head.AreaId) {
                                if (next_area->idr_area_head.ReadOnly ==
                                    SAHPI_TRUE) {
                                        return SA_ERR_HPI_READ_ONLY;
                                }
                                /* If the specified area id is found, then
                                 * traverse the IDR field list of this area and
                                 * delete the fields
                                 */
                                for (count = 0;
                                     count < next_area->idr_area_head.NumFields;
                                     count++) {
                                        rv = idr_field_delete(
                                                &(next_area->field_list),
                                                next_area->field_list->
                                                        field.FieldId);
                                        if (rv != SA_OK) {
                                                return rv;
                                        }
                                }
                                local_area->next_area = next_area->next_area;
                                g_free(next_area);
                                return SA_OK;
                       } else {
                               local_area = local_area->next_area;
                       }
                }
        }
        return SA_ERR_HPI_NOT_PRESENT;
}

/**
 * fetch_idr_area_header
 *      @inventory_info: Pointer to rdr private data
 *      @area_id: Identifier of the area to be deleted
 *      @area_type: Type of IDR area
 *      @area_header: Structure to receive area header information
 *      @next_area_id: Next area Id of requested type
 *
 * Purpose:
 *      Gets an Inventory Data Repository(IDR) area header from Inventory data
 *      repository
 *
 * Detailed Description:
 *      - This function allows retrieval of an IDR Area Header by one of
 *        two ways: by AreaId regardless of type or by AreaType and AreaId
 *      - All areas within an IDR can be retrieved by setting area type as
 *        SAHPI_IDR_AREATYPE_UNSPECIFIED, area id set to  SAHPI_FIRST_ENTRY for
 *        the first call. For each subsequent call, the area id will be set to
 *        the value returned in the Next area id parameter,
 *        this will work until next area id becomes SAHPI_LAST_ENTRY
 *     -  To retrieve all areas of specific type,  the above step is repeated
 *        with the specified area type
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT fetch_idr_area_header(struct oa_soap_inventory_info *inventory_info,
                               SaHpiEntryIdT area_id,
                               SaHpiIdrAreaTypeT area_type,
                               SaHpiIdrAreaHeaderT *area_header,
                               SaHpiEntryIdT *next_area_id)
{
        SaHpiInt32T i;
        struct oa_soap_area *local_area = NULL;
        SaHpiInt32T found = SAHPI_FALSE;
        SaHpiInt32T area_found = SAHPI_FALSE;

        if (inventory_info == NULL) {
                return SA_ERR_HPI_ERROR;
        }

        if ((area_header == NULL) && (next_area_id == NULL)) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_area = inventory_info->area_list;

        /* Check whether area id is set to return all areas
         * (of a particular type if specified)
         */
        if (area_id == SAHPI_FIRST_ENTRY) {
                i = 1;
                /* Traversing the IDR area list to find the area and next area
                 * of specified area type
                 */
                while ((i <= inventory_info->idr_info.NumAreas) &&
                       (local_area != NULL)) {
                        if (area_type == SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                            area_type == local_area->idr_area_head.Type) {
                                area_found = SAHPI_TRUE;
                                memcpy(area_header, &local_area->idr_area_head,
                                       sizeof(SaHpiIdrAreaHeaderT));
                                local_area = local_area->next_area;
                                *next_area_id = SAHPI_LAST_ENTRY;
                                while (local_area) {
                                        if (area_type ==
                                              SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                                            area_type ==
                                              local_area->idr_area_head.Type) {
                                                *next_area_id = local_area->
                                                        idr_area_head.AreaId;
                                                found = SAHPI_TRUE;
                                                break;
                                        }
                                        local_area = local_area->next_area;
                                }
                                break;
                        } else {
                                local_area = local_area->next_area;
                        }
                        i++;
                }
        } else {
                /* Traverse the area list to find area of specified id and
                 * type
                 */
                while (local_area != NULL) {
                        if (found == SAHPI_TRUE) {
                                break;
                        }
                        /* If specified area is present then retrive area header
                         * along with next area of same type
                         */
                        if ((area_id == local_area->idr_area_head.AreaId)) {
                                if (area_type ==
                                      SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                                    area_type ==
                                      local_area->idr_area_head.Type) {
                                        area_found = SAHPI_TRUE;
                                        memcpy(area_header,
                                               &local_area->idr_area_head,
                                               sizeof(SaHpiIdrAreaHeaderT));
                                        *next_area_id = SAHPI_LAST_ENTRY;
                                        while (local_area->next_area != NULL) {
                                                local_area =
                                                        local_area->next_area;
                                                if (area_type ==
                                                    SAHPI_IDR_AREATYPE_UNSPECIFIED ||
                                                    area_type == local_area->
                                                          idr_area_head.Type) {
                                                        *next_area_id =
                                                                local_area->
                                                                idr_area_head.
                                                                AreaId;
                                                        found = SAHPI_TRUE;
                                                        break;
                                                }
                                        }
                                        break;
                                } else {
                                        break;
                                }
                        }
                        local_area = local_area->next_area;
                }
        }
        if (!area_found) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        return SA_OK;
}

/**
 * idr_field_add
 *      @oa_field: Pointer to field structure
 *      @area_id: Identifier of the area to be deleted
 *      @field_type: Type of IDR field
 *      @str: Field text content
 *      @hpi_field: Pointer to hpi field structure
 *
 * Purpose:
 *      Adds an Inventory Data Repository(IDR) field to Inventory data
 *      repository Area
 *
 * Detailed Description:
 *      - Creates an IDR field of a specified type in an IDR area
 *        If the field list in the IDR area exists, then the
 *        newly created field will be added to end of field list.
 *        If the field list is empty then the created field will become head
 *        node (first field) of the field list
 *      - Field id is will start from 0  and will increment for every new field
 *        added
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT  idr_field_add(struct oa_soap_field **oa_field,
                        SaHpiIdrFieldT *hpi_field)
{
        SaHpiEntryIdT field_id;
        struct oa_soap_field *field = NULL;

        if (oa_field == NULL || hpi_field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        field = *oa_field;
        /* Check whether the field list is empty */
        if (field == NULL) {
                /* Create the area and make it as head node(first area) in
                 * the area list
                 */
                field = (struct oa_soap_field*)
                        g_malloc0(sizeof(struct oa_soap_field));
                if (! (field)) {
                        err("OA SOAP out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                *oa_field = field;
                field_id = 0;
        } else {
                /* Field list is not empty, traverse to the end of the list
                 * and add the IDR field
                 */
                while (field->next_field != NULL) {
                        field = field->next_field;
                }
                field->next_field = (struct oa_soap_field*)
                        g_malloc0(sizeof(struct oa_soap_field));
                if (!(field->next_field)) {
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
                field_id = field->field.FieldId + 1;
                field = field->next_field;
        }
        /* Initialize the field of specified field type and generated
         * field id
         */
        field->field.AreaId = hpi_field->AreaId;
        field->field.FieldId = field_id;
        field->field.Type = hpi_field->Type;
        field->field.ReadOnly = SAHPI_FALSE;
        hpi_field->ReadOnly = SAHPI_FALSE;
        field->field.Field.DataType = SAHPI_TL_TYPE_TEXT;
        field->field.Field.Language = SAHPI_LANG_ENGLISH;
        field->field.Field.DataLength =
                strlen ((char *)hpi_field->Field.Data) + 1;
        snprintf((char *)field->field.Field.Data,
                 field->field.Field.DataLength,
                 "%s", hpi_field->Field.Data);

        field->next_field = NULL;
        hpi_field->FieldId = field_id;
        return SA_OK;
}

/**
 * idr_field_add_by_id:
 *      @oa_field: Pointer to field structure
 *      @area_id: Identifier of the area to be added
 *      @field_type: Type of IDR field
 *      @field_data: pointer to field text content
 *      @fied_id: field id to be added
 *
 * Purpose:
 *      Adds an Inventory Data Repository(IDR) field with specified id to
 *      Inventory data repository Area
 *
 * Detailed Description:
 *      - Creates an IDR field of a specified type in an IDR area
 *        Newly created field will be inserted at the proper position
 *        If the field list is empty then the created field will become head
 *        node (first field) for the field list
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - Input parameters are not valid
 *      SA_ERR_HPI_OUT_OF_MEMORY - Request failed due to insufficient memory
 **/
SaErrorT idr_field_add_by_id(struct oa_soap_field **head_field,
                             SaHpiEntryIdT area_id,
                             SaHpiIdrFieldTypeT field_type,
                             char *field_data,
                             SaHpiEntryIdT field_id)
{
        struct oa_soap_field *field = NULL;
        struct oa_soap_field *temp_field = NULL;

        if (head_field == NULL || field_data == NULL ||
            area_id == SAHPI_LAST_ENTRY ||
            field_id == SAHPI_LAST_ENTRY) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        temp_field = *head_field;
        field = (struct oa_soap_field*)g_malloc0(sizeof(struct oa_soap_field));
        if (!(field)) {
                err("OA SOAP out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        /* Initialize the field with specified field type and ID */
        field->field.AreaId = area_id;
        field->field.FieldId = field_id;
        field->field.Type = field_type;
        field->field.ReadOnly =  SAHPI_FALSE;
        field->field.Field.DataType = SAHPI_TL_TYPE_TEXT;
        field->field.Field.Language = SAHPI_LANG_ENGLISH;
        field->field.Field.DataLength = strlen (field_data) + 1;
        snprintf((char *)field->field.Field.Data,
                 field->field.Field.DataLength,
                 "%s", field_data);

        /* Check whether the field list is empty  or if the new field is
         * to be inserted before first field
         */
        if (*head_field == NULL || (*head_field)->field.FieldId > field_id) {
                *head_field = field;
                (*head_field)->next_field = temp_field;
        } else {
                while (temp_field != NULL) {
                        if ((temp_field->field.FieldId < field_id) &&
                                ((temp_field->next_field == NULL) ||
                                 (temp_field->next_field->field.FieldId >
                                        field_id))) {
                                field->next_field = temp_field->next_field;
                                temp_field->next_field = field;
                                break;
                        }
                        temp_field = temp_field->next_field;
               }
        }
        return SA_OK;
}

/**
 * idr_field_delete
 *      @oa_field: Pointer to field structure
 *      @field_id: Identifier of the IDR field
 *
 * Purpose:
 *      Deletes an Inventory Data Repository(IDR) field from Inventory data
 *      repository Area
 *
 * Detailed Description:
 *      - Deleted an IDR field of a specified id if it exists in the area
 *        list else an appropriate error will be returned
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT idr_field_delete(struct oa_soap_field **oa_field,
                          SaHpiEntryIdT field_id)
{
        struct oa_soap_field *field = NULL, *tmp_field = NULL;

        if (oa_field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        field = *oa_field;
        /* If field list is empty, then return an error */
        if (field == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        } else {
                /* Check whether specified field id matches with first field id
                 */
                if (field_id == field->field.FieldId) {
                        tmp_field = field;
                        /* If the specified IDR field is read only, then delete
                         * will be failed and READ ONLY error will be returned
                         */
                        if (field->field.ReadOnly == SAHPI_TRUE) {
                                return SA_ERR_HPI_READ_ONLY;
                        }
                        field = field->next_field;
                        *oa_field = field;
                        g_free(tmp_field);
                        return SA_OK;
                }

                /* Traverse the field list to find the specified IDR field */
                while (field->next_field) {
                        tmp_field = field->next_field;
                        if (field_id == tmp_field->field.FieldId) {
                                if (tmp_field->field.ReadOnly == SAHPI_TRUE) {
                                        return SA_ERR_HPI_READ_ONLY;
                                }
                                field->next_field = tmp_field->next_field;
                                g_free(tmp_field);
                                return SA_OK;
                        } else {
                                field = field->next_field;
                        }
                }
        }
        return SA_ERR_HPI_NOT_PRESENT;
}

/**
 * idr_field_update
 *      @oa_field: Pointer to field structure
 *      @field: Field structure containing modification information
 *
 * Purpose:
 *      Modifies an Inventory data repository field in Inventory data repository
 *      Area
 *
 * Detailed Description:
 *      - Sets an IDR field of a specified field contents if it exists
 *        else an appropriate error will be returned
 *      - Validation of the field content is not handled in OA SOAP plug-in
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT idr_field_update(struct oa_soap_field *oa_field,
                          SaHpiIdrFieldT *field)
{
        if (oa_field == NULL) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (field == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Traverse the field list to find the IDR field with specified id */
        while (oa_field) {

                if (field->FieldId == oa_field->field.FieldId) {
                        /* If the specified IDR field is read only, then delete
                         * will be failed and READ ONLY error will be returned
                         */
                        if (oa_field->field.ReadOnly == SAHPI_TRUE) {
                                return SA_ERR_HPI_READ_ONLY;
                        }
                        /* Update the field contents with the new data */
                        oa_field->field.Type = field->Type;
                        oa_field->field.Field.DataType = field->Field.DataType;
                        oa_field->field.Field.Language = field->Field.Language;
                        oa_field->field.Field.DataLength =
                                field->Field.DataLength;
                        memset (oa_field->field.Field.Data, 0,
                                SAHPI_MAX_TEXT_BUFFER_LENGTH);
                        snprintf((char *)oa_field->field.Field.Data,
                                 oa_field->field.Field.DataLength,
                                 "%s", field->Field.Data);
                        return SA_OK;
                } else {
                        oa_field = oa_field->next_field;
                }
        }
        return SA_ERR_HPI_NOT_PRESENT;
}

/**
 * fetch_idr_field
 *      @inventory_info: Pointer to rdr private data
 *      @area_id: Identifier of the area to be deleted
 *      @field_type: Type of IDR field
 *      @field_id: Identifier of the IDR field
 *      @next_field_id: Identifier of the next IDR field of the requested type
 *      @field: Pointer to field structure
 *
 * Purpose:
 *      Gets an Inventory Data Repository(IDR) field from Inventory data
 *      repository area
 *
 * Detailed Description:
 *      - This function allows retrieval of an IDR field by one of
 *        two ways: by field id regardless of type or by field type and id
 *      - All fields within an IDR area can be retrieved by setting area type
 *        as SAHPI_IDR_FIELDTYPE_UNSPECIFIED, field id set to  SAHPI_FIRST_ENTRY
 *        for the first call. For each subsequent call, the field id will be set
 *        to the value returned in the next field id parameter,
 *        this will work until next field id becomes SAHPI_LAST_ENTRY
 *     -  To retrieve all field of specific type,  the above step is repeated
 *        with the specified field type
 *
 * Return values:
 *      SA_OK - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT - Requested object not present
 **/
SaErrorT fetch_idr_field(struct oa_soap_inventory_info *inventory_info,
                         SaHpiEntryIdT area_id,
                         SaHpiIdrFieldTypeT field_type,
                         SaHpiEntryIdT field_id,
                         SaHpiEntryIdT *next_field_id,
                         SaHpiIdrFieldT *field)
{
        SaHpiInt32T i;
        struct oa_soap_area *local_area = NULL;
        struct oa_soap_field *local_field = NULL;
        SaHpiInt32T found = SAHPI_FALSE;
        SaHpiInt32T fieldFound = SAHPI_FALSE;

        if (inventory_info == NULL) {
                err("IDR not present");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        if (field == NULL || next_field_id == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        local_area = inventory_info->area_list;
        while (local_area != NULL) {
                if ((area_id == local_area->idr_area_head.AreaId)) {
                        break;
                }
                local_area = local_area->next_area;
        }

        if (!local_area) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        local_field = local_area->field_list;

        /* Check whether field id is set to return all fields
         * (of a particular type if specified)
         */
        if (field_id == SAHPI_FIRST_ENTRY) {
                i = 1;

                while ((i <= local_area->idr_area_head.NumFields) &&
                       (local_field != NULL)) {
                        if (found == SAHPI_TRUE) {
                                break;
                        }
                        if (field_type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED ||
                            field_type == local_field->field.Type) {
                                fieldFound = SAHPI_TRUE;
                                memcpy(field, &local_field->field,
                                       sizeof(SaHpiIdrFieldT));
                                *next_field_id = SAHPI_LAST_ENTRY;
                                while (local_field->next_field != NULL) {
                                        local_field = local_field->next_field;
                                        if (field_type ==
                                              SAHPI_IDR_FIELDTYPE_UNSPECIFIED ||
                                            field_type ==
                                              local_field->field.Type) {
                                                *next_field_id =
                                                        local_field->
                                                        field.FieldId;
                                                found = SAHPI_TRUE;
                                                break;
                                        }
                                }
                                break;
                        } else {
                                local_field = local_field->next_field;
                        }
                        i++;
                }
        } else {
                while (local_field != NULL) {
                        if (found == SAHPI_TRUE) {
                                break;
                        }
                        if ((field_id == local_field->field.FieldId)) {
                                if (field_type ==
                                      SAHPI_IDR_FIELDTYPE_UNSPECIFIED ||
                                    field_type == local_field->field.Type) {
                                        fieldFound = SAHPI_TRUE;
                                        memcpy(field, &local_field->field,
                                               sizeof(SaHpiIdrFieldT));
                                        *next_field_id = SAHPI_LAST_ENTRY;
                                        while (local_field->next_field !=
                                               NULL) {
                                                local_field =
                                                        local_field->next_field;
                                                if (field_type ==
                                                    SAHPI_IDR_FIELDTYPE_UNSPECIFIED ||
                                                    field_type ==
                                                      local_field->field.Type) {
                                                        *next_field_id =
                                                                local_field->
                                                                field.FieldId;
                                                        found = SAHPI_TRUE;
                                                        break;
                                                }
                                        }
                                        break;
                                } else {
                                break;
                                }
                        }
                        local_field = local_field->next_field;
                }
        }
        if (fieldFound == SAHPI_FALSE) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
        return SA_OK;
}

/*
 * free_inventory_info
 *      @handler:     Handler data pointer
 *      @resource_id: Resource ID
 *
 * Purpose:
 *     To  delete the Areas and Fields present in
 *     Inventory Data Repository(IDR)
 *
 * Detailed Description:
 *     Get the IDR and traverse through each area and deletes the area.
 *     If any error occours while deleting appropriate error is returned
 *
 * Return values:
 *      SA_OK                     - Normal case
 *      SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *      SA_ERR_HPI_NOT_PRESENT    - Requested object not present
 */
SaErrorT free_inventory_info(struct oh_handler_state *handler,
                             SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_inventory *inventory;
        SaHpiEntryIdT area_id;
        SaHpiRdrT *rdr = NULL;

        if (handler == NULL) {
                err("Invalid parameter.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Get the inventory RDR */
        rdr = oh_get_rdr_by_type(handler->rptcache, resource_id,
                                 SAHPI_INVENTORY_RDR,
                                 SAHPI_DEFAULT_INVENTORY_ID);
        if (rdr == NULL) {
                err("Inventory RDR is not found");
                return SA_ERR_HPI_NOT_PRESENT;
        }

        inventory = (struct oa_soap_inventory *)
                oh_get_rdr_data(handler->rptcache, resource_id, rdr->RecordId);
        if (inventory == NULL) {
                err("No inventory data. IdrId=%s", rdr->IdString.Data);
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /*Traverse through area list  and delete the area*/
        while (inventory->info.area_list) {
                area_id = inventory->info.area_list->idr_area_head.AreaId;
                rv = idr_area_delete(&(inventory->info.area_list),
                                     area_id);
                if (rv != SA_OK) {
                        err("IDR Area delete failed");
                        return rv;
                }
        }

        return SA_OK;
}

/*
 * oa_soap_inv_set_field
 *	@area		: Pointer to the area list
 *	@area_type	: Type of the area
 *	@field_type	: Type of the field
 *	@comment	: Pointer to the data
 *
 * Purpose:
 *	Generic function to set field data
 *
 * Detailed Description:
 * 	- Searches the field based on area type and field type.
 * 	- Assigns the field data with inventory information.
 *
 * Return values:
 *	NONE 
 */
static void oa_soap_inv_set_field(struct oa_soap_area *area_list,
				  SaHpiIdrAreaTypeT area_type,
				  SaHpiIdrFieldTypeT field_type,
				  const char *data)
{
	struct oa_soap_area *area;
	struct oa_soap_field *field;

	if (area_list == NULL) {
		err("Invalid parameter");
		return;
	} 
	
	/* Data can be NULL if the device is faulty */
	if (data == NULL) {
		dbg("Can not set the field data for the field type %d",
		    field_type);
		dbg("Data passed is NULL");
		return;
	}

	area = area_list;
	/* Traverse the areas till we get area_type */
	while (area) {
		if (area->idr_area_head.Type == area_type) {
			field = area->field_list;
			/* Traverse the fields till we get field_type */
			while (field) {
				if (field->field.Type == field_type) {
					field->field.Field.DataLength =
						strlen(data) + 1;
					strcpy((char *) field->field.Field.Data,
						data);
					return;
				}
				field = field->next_field;
			}
		}
		area = area->next_area;
	}

	err("Failed to find the field type %d in area %d", field_type,
	    area_type);
	return;
}

/*
 * oa_soap_add_inv_fields
 *	@area		: Pointer to the area
 *	@field_list	: Pointer to the field list in the global array
 *
 * Purpose:
 *	Generic function add the fields to the area
 *
 * Detailed Description:
 *	- Gets the number of fields from the area
 *	- Allocates the memory for the field and constructs the field list
 *
 * Return values:
 *	NONE 
 */
static void oa_soap_add_inv_fields(struct oa_soap_area *area,
				   const struct oa_soap_field field_array[])
{
	struct oa_soap_field **field;
	SaHpiInt32T i;

	if (area == NULL || field_array == NULL) {
		err("Invalid parameters");
		return;
	}
	
	field = &(area->field_list);
	for (i = 0; i < area->idr_area_head.NumFields; i++) {
		*field = g_memdup(&(field_array[i].field),
				  sizeof(struct oa_soap_field));
		field = &((*field)->next_field);
	}
	
	return;
}

/*
 * oa_soap_add_inv_areas
 *	@area		: Pointer to the inventory structure
 *	@resource_type	: Resource type
 *
 * Purpose:
 *	To add the areas to the area_list
 *
 * Detailed Description:
 *	- Gets the number of areas from the IDR header
 *	- Allocates the memory for the areas and constructs the area list
 *
 * Return values:
 *	NONE 
 */
static void oa_soap_add_inv_areas(struct oa_soap_inventory *inventory,
				  SaHpiInt32T resource_type)
{
	struct oa_soap_area **area;
	SaHpiInt32T i, num_areas;

	if (inventory == NULL) {
		err("Invalid parameter");
		return;
	}

	/* Point to the location of the area_list pointer of inventory */
	area = &(inventory->info.area_list);
	/* Get the number of areas supported for the resource type */
	num_areas =
		oa_soap_inv_arr[resource_type].inventory.info.idr_info.NumAreas;
	for (i = 0; i < num_areas; i++) {
		*area = g_memdup(&(oa_soap_inv_arr[resource_type].area_array[i].
					area),
				sizeof(struct oa_soap_area));
		/* Add the fields to the newly added area */
		oa_soap_add_inv_fields(*area,
					oa_soap_inv_arr[resource_type].
						area_array[i].field_array);
		/* Point to the location of the next area pointer */
		area = &((*area)->next_area);
	}
	
	return;
}

/*
 * oa_soap_build_inv
 *	@oh_handler	: Pointer to the handler
 *	@resource_type	: Resource type
 *	@resource_id	: Resource Id
 *	@rdr		: Pointer to the rdr structure
 *
 * Purpose:
 *	Generic function to build the inventory RDR
 *
 * Detailed Description:
 * 	- Allocates the memory for inventory info
 * 	- Builds the area and field list from global inventory array
 * 	- Copies the inventory RDR information from global inventory array
 * 	- Pushes the inventory RDR to plugin rptcache
 *
 * Return values:
 *	SA_OK                     - On success
 *	SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *	SA_ERR_HPI_OUT_OF_MEMORY  - On memory allocatin failure
 *	SA_ERR_HPI_NOT_PRESENT    - On wrong resource id
 */
static SaErrorT oa_soap_build_inv(struct oh_handler_state *oh_handler,
				  SaHpiInt32T resource_type,
				  SaHpiResourceIdT resource_id,
				  struct oa_soap_inventory **inventory)
{
	SaHpiRdrT rdr;
	SaHpiRptEntryT *rpt;
	SaErrorT rv;

	if (oh_handler == NULL || inventory == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
	/* Get the rpt entry of the resource */
	rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
	if (rpt == NULL) {
		err("resource RPT is NULL");
		return SA_ERR_HPI_NOT_PRESENT;
	}

	/* Get the inventory from the global array */
	*inventory =
		g_memdup(&(oa_soap_inv_arr[resource_type].inventory),
			 sizeof(struct oa_soap_inventory));

	if (*inventory == NULL) {
		err("Out of memory");
		return SA_ERR_HPI_OUT_OF_MEMORY;
	}
	
	rdr = oa_soap_inv_arr[resource_type].rdr;
	rdr.Entity = rpt->ResourceEntity;

	/* Populate the areas */
	oa_soap_add_inv_areas(*inventory, resource_type);

	rv = oh_add_rdr(oh_handler->rptcache, resource_id, 
			&rdr, *inventory, 0); 
	if (rv != SA_OK) { 
		err("Failed to add rdr"); 
		return rv; 
	} 

	return SA_OK;
}

/*
 * oa_soap_build_fz_inv
 *	@oh_handler	: Pointer to the handler
 *	@resource_id	: Resource Id
 *	@fanZone	: Pointer to structure fanZone
 *
 * Purpose:
 *	Builds the inventory RDR for fan zone
 *
 * Detailed Description:
 *	- Gets the fan inventory information
 *	- Builds the inventory RDR
 *	- Populates the device bays and fan bays
 *
 * Return values:
 *	SA_OK                     - On success
 *	SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *	SA_ERR_HPI_INERNAL_ERROR  - On soap call failure
 */
SaErrorT oa_soap_build_fz_inv(struct oh_handler_state *oh_handler,
			      SaHpiResourceIdT resource_id,
			      struct fanZone *fan_zone)
{
	SaErrorT rv;
	struct oa_soap_handler *oa_handler;
	struct oa_soap_inventory *inventory = NULL;
	char *temp, field_data[MAX_BUF_SIZE]; 
	SaHpiInt32T len, write_size = OA_SOAP_MAX_FZ_NUM_SIZE + 1;
	struct fanInfo info;
	byte bay;

	if (oh_handler == NULL || fan_zone == NULL) {
		err("Invalid Parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
	rv = oa_soap_build_inv(oh_handler, OA_SOAP_ENT_FZ, resource_id,
			       &inventory);
	if (rv != SA_OK) {
		err("Building inventory RDR for Fan Zone failed");
		return rv;
	}
	
	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	/* Construct the device bays field data*/
	/* Set the field_data to 0. This helps the strlen (5 lines down) to get
	 * correct string length
	 */
	memset(field_data, 0, OA_SOAP_MAX_FZ_INV_SIZE);
	temp = field_data;
	while (fan_zone->deviceBayArray) {
		soap_deviceBayArray(fan_zone->deviceBayArray, &bay);
		/* Check whether we have reached the end of field_data array */
		if ((strlen(field_data) + write_size) >=
		     OA_SOAP_MAX_FZ_INV_SIZE) {
			err("The field_data size smaller, it may lead to "
			    "potential memory overflow problem");
			return SA_ERR_HPI_INTERNAL_ERROR;
		}
		snprintf(temp, write_size, "%d,", bay);
		/* Point the temp to end of data */
		temp += strlen(temp);
		fan_zone->deviceBayArray =
			soap_next_node(fan_zone->deviceBayArray);
	}
	/* Remove the last ',' from data */
	len = strlen(field_data);
	field_data[len - 1] = '\0';

	/* Set the device bays field data */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_OEM,
			      OA_SOAP_INV_FZ_DEV_BAY,
			      field_data);

	/* Construct the fan bays field data*/
	/* Set the field_data to 0. This helps the strlen (5 lines down) to get
	 * correct string length
	 */
	memset(field_data, 0, OA_SOAP_MAX_FZ_INV_SIZE);
	temp = field_data;
	while (fan_zone->fanInfoArray) {
		soap_fanInfo(fan_zone->fanInfoArray, &info);
		/* Check whether we have reached the end of field_data array */
		if ((strlen(field_data) + write_size) >=
		     OA_SOAP_MAX_FZ_INV_SIZE) {
			err("The field_data size smaller, it may lead to "
			    "potential memory overflow problem");
			return SA_ERR_HPI_INTERNAL_ERROR;
		}
		snprintf(temp, write_size, "%d,", info.bayNumber);
		/* Point the temp to end of data */
		temp += strlen(temp);
		fan_zone->fanInfoArray = soap_next_node(fan_zone->fanInfoArray);
	}
	/* Remove the last ',' from data */
	len = strlen(field_data);
	field_data[len - 1] = '\0';

	/* Set the fan bays field data */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_OEM,
			      OA_SOAP_INV_FZ_FAN_BAY,
			      field_data);

	return SA_OK;
}

/*
 * oa_soap_build_fan_inv
 *	@oh_handler	: Pointer to the handler
 *	@resource_id	: Resource Id
 *	@fan_info	: Pointer to fanInfo structure
 *
 * Purpose:
 *	Builds the inventory RDR for fan
 *
 * Detailed Description:
 *	- Gets the fan inventory information
 *	- Builds the inventory RDR
 *	- Populates the inventory info with product name, part number, serial
 *	  number, primary fan zone, secondary fan zone and shared status
 *
 * Return values:
 *	SA_OK                     - On success
 *	SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *	SA_ERR_HPI_INERNAL_ERROR  - On soap call failure
 */
SaErrorT oa_soap_build_fan_inv(struct oh_handler_state *oh_handler,
			       SaHpiResourceIdT resource_id,
			       struct fanInfo *fan_info)
{
	SaErrorT rv;
	struct oa_soap_handler *oa_handler;
	struct oa_soap_inventory *inventory = NULL;
	char field_data[OA_SOAP_MAX_FZ_INV_SIZE];
	SaHpiInt32T slot;

	if (oh_handler == NULL || fan_info == NULL) {
		err("Invalid Parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	rv = oa_soap_build_inv(oh_handler, OA_SOAP_ENT_FAN, resource_id,
			       &inventory);
	if (rv != SA_OK) {
		err("Building inventory RDR for Fan failed");
		return rv;
	}


	/* Set the product name */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_PRODUCT_INFO,
			      SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
			      fan_info->name);
	/* Set the part number */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_BOARD_INFO,
			      SAHPI_IDR_FIELDTYPE_PART_NUMBER,
			      fan_info->partNumber);
	/* Set the serial number */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_PRODUCT_INFO,
			      SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
			      fan_info->serialNumber);

	memset(field_data, 0, OA_SOAP_MAX_FZ_INV_SIZE);
	slot = fan_info->bayNumber;
	/* Construct the fan shared field data */
	if (oa_soap_fz_map_arr[oa_handler->enc_type][slot].shared == SAHPI_TRUE)
		strcpy(field_data, "Shared=TRUE");
	else
		strcpy(field_data, "Shared=FALSE");

	/* Set the fan shared field */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_OEM,
			      OA_SOAP_INV_FAN_SHARED,
			      field_data);

	/* Construct the fan zone number field data */
	memset(field_data, 0, OA_SOAP_MAX_FZ_INV_SIZE);
	if (oa_soap_fz_map_arr[oa_handler->enc_type][slot].secondary_zone) {
		snprintf(field_data, 13, "Fan Zone=%d,%d",
		 	oa_soap_fz_map_arr[oa_handler->enc_type][slot].zone,
			oa_soap_fz_map_arr[oa_handler->enc_type][slot].
				secondary_zone);
	} else {
		snprintf(field_data, 11, "Fan Zone=%d",
		 	oa_soap_fz_map_arr[oa_handler->enc_type][slot].zone);
	}

	/* Set the shared field */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_OEM,
			      OA_SOAP_INV_FZ_NUM,
			      field_data);
	
	return SA_OK;
}

/*
 * oa_soap_build_lcd_inv
 *	@oh_handler	: Pointer to the handler
 *	@resource_id	: Resource Id
 *
 * Purpose:
 *	Builds the inventory RDR for LCD
 *
 * Detailed Description:
 *	- Gets the LCD inventory information
 *	- Builds the inventory RDR
 *	- Populates the inventory info with product name, manufacturer name,
 *	  part number and firmware version
 *
 * Return values:
 *	SA_OK                     - On success
 *	SA_ERR_HPI_INVALID_PARAMS - On wrong parameters
 *	SA_ERR_HPI_INERNAL_ERROR  - On soap call failure
 */
SaErrorT oa_soap_build_lcd_inv(struct oh_handler_state *oh_handler,
			       SaHpiResourceIdT resource_id)
{
	SaErrorT rv;
	struct oa_soap_handler *oa_handler;
	struct oa_soap_inventory *inventory = NULL;
	struct lcdInfo info;

	if (oh_handler == NULL) {
		err("Invalid Parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
	rv = oa_soap_build_inv(oh_handler, OA_SOAP_ENT_LCD,
			       resource_id, &inventory);
	if (rv != SA_OK) {
		err("Building inventory RDR for LCD failed");
		return rv;
	}

	
	oa_handler = (struct oa_soap_handler *) oh_handler->data;
	/* Get the LCD info */
	rv = soap_getLcdInfo(oa_handler->active_con, &info);
	if (rv != SOAP_OK) {
		err("Get LCD Info SOAP call has failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Set the product name */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_PRODUCT_INFO,
			      SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
			      info.name);
	/* Set the manufacturer name */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_PRODUCT_INFO,
			      SAHPI_IDR_FIELDTYPE_MANUFACTURER,
			      info.manufacturer);
	/* Set the part number */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_BOARD_INFO,
			      SAHPI_IDR_FIELDTYPE_PART_NUMBER,
			      info.partNumber);
	/* Set the firmware version */
	oa_soap_inv_set_field(inventory->info.area_list,
			      SAHPI_IDR_AREATYPE_PRODUCT_INFO,
			      SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION,
			      info.fwVersion);
	return SA_OK;
}

void * oh_get_idr_info(void *,
                       SaHpiResourceIdT,
                       SaHpiIdrIdT,
                       SaHpiIdrInfoT)
                __attribute__ ((weak, alias("oa_soap_get_idr_info")));

void * oh_get_idr_area_header(void *,
                              SaHpiResourceIdT,
                              SaHpiIdrIdT,
                              SaHpiIdrAreaTypeT,
                              SaHpiEntryIdT,
                              SaHpiEntryIdT,
                              SaHpiIdrAreaHeaderT)
                __attribute__ ((weak, alias("oa_soap_get_idr_area_header")));

void * oh_add_idr_area(void *,
                       SaHpiResourceIdT,
                       SaHpiIdrIdT,
                       SaHpiIdrAreaTypeT,
                       SaHpiEntryIdT)
                __attribute__ ((weak, alias("oa_soap_add_idr_area")));

void * oh_add_idr_area_id(void *,
                       SaHpiResourceIdT,
                       SaHpiIdrIdT,
                       SaHpiIdrAreaTypeT,
                       SaHpiEntryIdT)
                __attribute__ ((weak, alias("oa_soap_add_idr_area_by_id")));

void * oh_del_idr_area(void *,
                       SaHpiResourceIdT,
                       SaHpiIdrIdT,
                       SaHpiEntryIdT)
                __attribute__ ((weak, alias("oa_soap_del_idr_area")));

void * oh_get_idr_field(void *,
                        SaHpiResourceIdT,
                        SaHpiIdrIdT,
                        SaHpiEntryIdT,
                        SaHpiIdrFieldTypeT,
                        SaHpiEntryIdT,
                        SaHpiEntryIdT,
                        SaHpiIdrFieldT)
                __attribute__ ((weak, alias("oa_soap_get_idr_field")));

void * oh_add_idr_field(void *,
                        SaHpiResourceIdT,
                        SaHpiIdrIdT,
                        SaHpiIdrFieldT)
                __attribute__ ((weak, alias("oa_soap_add_idr_field")));

void * oh_add_idr_field_id(void *,
                        SaHpiResourceIdT,
                        SaHpiIdrIdT,
                        SaHpiIdrFieldT)
                __attribute__ ((weak, alias("oa_soap_add_idr_field_by_id")));

void * oh_set_idr_field(void *,
                        SaHpiResourceIdT,
                        SaHpiIdrIdT,
                        SaHpiIdrFieldT)
                __attribute__ ((weak, alias("oa_soap_set_idr_field")));

void * oh_del_idr_field(void *,
                        SaHpiResourceIdT,
                        SaHpiIdrIdT,
                        SaHpiEntryIdT,
                        SaHpiEntryIdT)
                __attribute__ ((weak, alias("oa_soap_del_idr_field")));
