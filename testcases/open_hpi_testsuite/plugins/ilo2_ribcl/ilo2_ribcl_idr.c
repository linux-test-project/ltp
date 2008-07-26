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

/***************
 * This source file contains IDR HPI ABI routines iLO2 RIBCL plug-in
 * implements. Other source files provide support functionality for
 * these ABIs.
***************/
#include <ilo2_ribcl.h>
#include <ilo2_ribcl_idr.h>
#include <ilo2_ribcl_discover.h>

/************************************
	Forward declarations for static functions in this file
************************************/
static SaErrorT ilo2_ribcl_get_idr_allinfo_by_ep( struct oh_handler_state *,
			SaHpiEntityPathT *,
			SaHpiIdrIdT ,
			struct ilo2_ribcl_idr_allinfo *);

static SaErrorT ilo2_ribcl_get_idr_allinfo( struct oh_handler_state *,
			SaHpiResourceIdT,
			SaHpiIdrIdT,
			struct ilo2_ribcl_idr_allinfo *);

static void ilo2_ribcl_field_catstring( I2R_FieldT *field, char *str);

static int ilo2_ribcl_update_idr( struct ilo2_ribcl_idr_info *,
			struct ilo2_ribcl_idr_info *);

/**
 * ilo2_ribcl_get_idr_info:
 * @hnd:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this IDR.
 * @IdrId:	IDR id number.
 * @IdrInfo:	Pointer used to return IDR information.
 *
 * Description:
 * Implements the plugin specific part of the saHpiIdrInfoGet() API.
 *
 * We make a call to ilo2_ribcl_get_idr_allinfo() to obtain the inventory RDR,
 * the rpt entry for the resource containing the IDR, and the struct
 * ilo2_ribcl_idr_info that contains all our IDR data.
 *
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support IDRs.
 * SA_ERR_HPI_INVALID_PARAMS - IdrInfo pointer or handler is NULL.
 * SA_ERR_HPI_NOT_PRESENT - The requested IDR is not present.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 *
 **/
SaErrorT ilo2_ribcl_get_idr_info(void *hnd,
			SaHpiResourceIdT        rid,
			SaHpiIdrIdT             IdrId,
			SaHpiIdrInfoT          *IdrInfo)
{
	SaErrorT ret;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_idr_allinfo idr_allinfo;

	if( !hnd){
		err(" ilo2_ribcl_get_idr_info: invalid handle.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	if( !IdrInfo){
		err(" ilo2_ribcl_get_idr_info: invalid IDR info pointer.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our IDR RDR, and get the IDR information */

	ret = ilo2_ribcl_get_idr_allinfo( oh_handler, rid, IdrId, &idr_allinfo);

	if( ret != SA_OK){
		return( ret);
	}

	IdrInfo->IdrId = IdrId;
	IdrInfo->ReadOnly = SAHPI_TRUE;
	IdrInfo->NumAreas = idr_allinfo.idrinfo->num_areas;
	IdrInfo->UpdateCount = idr_allinfo.idrinfo->update_count;

	return( SA_OK);
	
} /* end  ilo2_ribcl_get_idr_info() */



/**
 * lo2_ribcl_get_idr_area_header:
 * @hnd:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this IDR.
 * @IdrId:	IDR id number.
 * @AreaType:	Type of area to search for.
 * @AreaId:	Id of area to search for.
 * @NextAreaId:	Pointer to return Id of next area (if it exists)
 * @Header:	Pointer to return area header data.
 *
 * Description:
 * Implements the plugin specific part of the saHpiIdrAreaHeaderGet() API.
 *
 * We make a call to ilo2_ribcl_get_idr_allinfo() to obtain the inventory RDR,
 * the rpt entry for the resource containing the IDR, and the struct
 * ilo2_ribcl_idr_info that contains all our IDR data.
 *
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Currently, we only have one Area in all of our IDRs, but this code should
 * handle multiple Areas if we ever implement them.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support IDRs.
 * SA_ERR_HPI_INVALID_PARAMS - Handler parameter is NULL.
 *			     - AreaId is an invalid reserved value.
 *			     - NextAreaId or Header parameters are NULL.
 * SA_ERR_HPI_NOT_PRESENT - The requested IDR is not present.
 *			  - AreaType  is SAHPI_IDR_AREATYPE_UNSPECIFIED and
 *			    the area specified by AreaId does not exist.
 *			  - AreaType and AreaId are both set, but a matching
 *			    area cannot be found.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 **/
SaErrorT ilo2_ribcl_get_idr_area_header(void *hnd,
			SaHpiResourceIdT         rid,
			SaHpiIdrIdT              IdrId,
			SaHpiIdrAreaTypeT        AreaType,
			SaHpiEntryIdT            AreaId,
			SaHpiEntryIdT           *NextAreaId,
			SaHpiIdrAreaHeaderT     *Header)
{
	SaErrorT ret;
	int adx;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_idr_allinfo idr_allinfo;
	struct ilo2_ribcl_idr_info *idrinfo;
	I2R_AreaT *ir_area;
	SaHpiBoolT area_found = SAHPI_FALSE;
	
	/* Note: AreaType, AreaId, NextAreaId, and Header are checked
	 * for spec compliance in our caller (saHpiIdrAreaHeaderGet) */ 

	if( !hnd || !NextAreaId || !Header){
		err(" ilo2_ribcl_get_idr_area_header: invalid pointer.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our IDR RDR, and get the IDR information */

	ret = ilo2_ribcl_get_idr_allinfo( oh_handler, rid, IdrId, &idr_allinfo);

	if( ret != SA_OK){
		return( ret);
	}

	/* Note that HPI AreaId values begin with one, but our data structures
	 * begin at zero, so we need to translate the AreaId parameter. */

	if( AreaId == SAHPI_FIRST_ENTRY){
		AreaId = 0;
	} else {
		AreaId--;
	}

	idrinfo = idr_allinfo.idrinfo;
	ret =  SA_ERR_HPI_NOT_PRESENT;

	for( adx = 0; adx < idrinfo->num_areas; adx++){

		ir_area = &(idrinfo->idr_areas[adx]);

		if( (AreaType == ir_area->area_type) || 
		    (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED) ){

			if( AreaId == adx){ /* we have a match */
				Header->AreaId = adx+1;
				Header->Type = ir_area->area_type;
				Header->ReadOnly = SAHPI_TRUE;
				Header->NumFields = ir_area->num_fields;
				area_found = SAHPI_TRUE;
				ret = SA_OK;
				*NextAreaId = SAHPI_LAST_ENTRY;
			} else {
				if( area_found){ 
					/* Now, we have found the next match */
					if( adx < idrinfo->num_areas){
						*NextAreaId = adx+1;
						break;
					}
				}
			}
		}

	} /* end for adx */

	return( ret); 
	
	
} /* end ilo2_ribcl_get_idr_area_header() */



/**
 * ilo2_ribcl_get_idr_field:
 * @hnd:	Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this IDR.
 * @IdrId:	IDR id number.
 * @AreaId:	Id of area to search for.
 * @FieldType:	Type of field to search for.
 * @FieldId:	Id of field to search for.
 * @NextFieldId: Pointer to return Id of next area (if it exists)
 * @Field:	 Pointer to return field data.
 *
 * Description:
 * Implements the plugin specific part of the saHpiIdrFieldGet() API.
 *
 * We make a call to ilo2_ribcl_get_idr_allinfo() to obtain the inventory RDR,
 * the rpt entry for the resource containing the IDR, and the struct
 * ilo2_ribcl_idr_info that contains all our IDR data.
 *
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_CAPABILITY - Resource does not support IDRs.
 * SA_ERR_HPI_INVALID_PARAMS - Handler parameter is NULL.
 *			     - AreaId or FieldId is an invalid reserved value.
 *			     - NextField or Field parameters are NULL.
 * SA_ERR_HPI_NOT_PRESENT - The requested IDR is not present.
 *			  - Area identified by AreaId is not present. 
 *			  - FieldType is SAHPI_IDR_FIELDTYPE_UNSPECIFIED and
 *			    the field specified by FieldId does not exist.
 *			  - FieldType and FieldId are both set, but a matching
 *			    field can not be found.
 * SA_ERR_HPI_INVALID_RESOURCE - The resource does not exist.
 *
 **/
SaErrorT ilo2_ribcl_get_idr_field(void *hnd,
			SaHpiResourceIdT       rid,
			SaHpiIdrIdT            IdrId,
			SaHpiEntryIdT          AreaId,
			SaHpiIdrFieldTypeT     FieldType,
			SaHpiEntryIdT          FieldId,
			SaHpiEntryIdT          *NextFieldId,
			SaHpiIdrFieldT         *Field)
{
	SaErrorT ret;
	int fdx;
	SaHpiEntryIdT match_id;
	struct oh_handler_state *oh_handler;
	struct ilo2_ribcl_idr_allinfo idr_allinfo;
	struct ilo2_ribcl_idr_info *idrinfo;
	I2R_AreaT *ir_area;
	I2R_FieldT *ir_field;
	SaHpiBoolT field_found = SAHPI_FALSE;
	
	/* Note: FieldType, AreaId, FieldId, NextFieldId, and Field are checked
	 * for spec compliance in our caller (saHpiIdrFieldGet) */ 

	if( !hnd || !NextFieldId || !Field){
		err(" ilo2_ribcl_get_idr_field: invalid pointer parameter.");
		return( SA_ERR_HPI_INVALID_PARAMS);
	}

	oh_handler = (struct oh_handler_state *)hnd;

	/* Look up our IDR RDR, and get the IDR information */

	ret = ilo2_ribcl_get_idr_allinfo( oh_handler, rid, IdrId, &idr_allinfo);

	if( ret != SA_OK){
		return( ret);
	}

	/* Note that HPI AreaId and FieldId values begin with one,
	 * but our data structures begin at zero, so we need to translate the
	 * AreaId and FieldId parameters. */

	if( AreaId == SAHPI_FIRST_ENTRY){
		AreaId = 0;
	} else {
		AreaId--;
	}

	if( FieldId == SAHPI_FIRST_ENTRY){
		match_id = 0;
	} else {
		match_id = FieldId -1;
	}

	idrinfo = idr_allinfo.idrinfo;
	ret =  SA_ERR_HPI_NOT_PRESENT;

	/* Find the correct area */
	if( AreaId >= idrinfo->num_areas){ /* AreaId is too large */
		return( SA_ERR_HPI_NOT_PRESENT);
	}
	ir_area = &(idrinfo->idr_areas[AreaId]);

	/* Now, search for the matching field */
	for( fdx = 0; fdx < ir_area->num_fields; fdx++){

		ir_field = &(ir_area->area_fields[fdx]);

		if( (FieldType == ir_field->field_type) || 
		    (FieldType == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) ){

			if( field_found){
				/* Now, we have found the next match */
				if( fdx < ir_area->num_fields){
					*NextFieldId = fdx+1;
					break;
				}

			} else if( (match_id == fdx) || (FieldId == SAHPI_FIRST_ENTRY)){ 
				/* we have found the matching entry */
				Field->AreaId = AreaId+1;
				Field->FieldId = fdx+1;
				Field->Type = ir_field->field_type;
				Field->ReadOnly = SAHPI_TRUE;

				oh_init_textbuffer(&(Field->Field));
				oh_append_textbuffer(&(Field->Field),
						     ir_field->field_string);

				field_found = SAHPI_TRUE;
				ret = SA_OK;
				*NextFieldId = SAHPI_LAST_ENTRY;
			}
	
		} /* end if FieldType matched */

	} /* end for fdx */
	
	return( ret);
	 
} /* end ilo2_ribcl_get_idr_field() */



/**
 * ilo2_ribcl_add_idr_area:
 * @hnd: 	Pointer to handler's data
 * @ResourceId: Resource identifier for this operation
 * @IdrId:  	Identifier for the Inventory Data Repository
 * @AreaType: 	Type of Inventory Data Area
 * @AreaId: 	Pointer to store the identifier of the newly allocated
 *		Inventory Area
 *
 * This function is not suported/implemented for the ilo2_ribcl plugin.
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return value:
 * SA_ERR_HPI_READ_ONLY - Normal - ilo2_ribcl only supports "read only" IDRs.
 **/
SaErrorT ilo2_ribcl_add_idr_area( void *hnd,
			SaHpiResourceIdT	ResourceId,
			SaHpiIdrIdT		IdrId,
			SaHpiIdrAreaTypeT	AreaType,
			SaHpiEntryIdT		*AreaId)

{
	return SA_ERR_HPI_READ_ONLY;
}



/**
 * ilo2_ribcl_del_idr_area:
 * @hnd: 	Pointer to handler's data
 * @ResourceId: Resource identifier for this operation
 * @IdrId:  	Identifier for the Inventory Data Repository
 * @AreaId: 	Identifier of Area entry to delete from the IDR
 *
 * This function is not suported/implemented for the ilo2_ribcl plugin.
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return value:
 * SA_ERR_HPI_READ_ONLY - Normal - ilo2_ribcl only supports "read only" IDRs.
 **/
SaErrorT ilo2_ribcl_del_idr_area( void *hnd,
			SaHpiResourceIdT	ResourceId,
			SaHpiIdrIdT		IdrId,
			SaHpiEntryIdT		AreaId)
{
	return SA_ERR_HPI_READ_ONLY;
}



/**
 * ilo2_ribcl_add_idr_field:
 * @hnd: 	Pointer to handler's data
 * @ResourceId: Resource identifier for this operation
 * @IdrId:  	Identifier for the Inventory Data Repository
 * @Field: 	Pointer to Inventory Data Field which contains field
 *		information to be added.
 *
 * This function is not suported/implemented for the ilo2_ribcl plugin.
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return value:
 * SA_ERR_HPI_READ_ONLY - Normal - ilo2_ribcl only supports "read only" IDRs.
 **/
SaErrorT ilo2_ribcl_add_idr_field( void *hnd,
			SaHpiResourceIdT	ResourceId,
			SaHpiIdrIdT		IdrId,
			SaHpiIdrFieldT		*Field)
{
	return SA_ERR_HPI_READ_ONLY;
}



/**
 * ilo2_ribcl_set_idr_field:
 * @hnd: 	Pointer to handler's data
 * @ResourceId: Resource identifier for this operation
 * @IdrId:  	Identifier for the Inventory Data Repository
 * @Field: 	Pointer to Inventory Data Field which contains field
 *		information to be updated.
 *
 * This function is not suported/implemented for the ilo2_ribcl plugin.
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return value:
 * SA_ERR_HPI_READ_ONLY - Normal - ilo2_ribcl only supports "read only" IDRs.
 **/
SaErrorT ilo2_ribcl_set_idr_field( void *hnd,
			SaHpiResourceIdT	ResourceId,
			SaHpiIdrIdT		IdrId,
			SaHpiIdrFieldT		*Field)
{
	return SA_ERR_HPI_READ_ONLY;
}


/**
 * ilo2_ribcl_del_idr_field:
 * @hnd: Pointer to handler's data
 * @ResourceId:	Resource identifier for this operation
 * @IdrId:	Identifier for the Inventory Data Repository
 * @AreaId:	Identifier of Inventory Area whose field is to bo deleted
 * @FieldId:	Identifier of field to be deleted
 *
 * This function is not suported/implemented for the ilo2_ribcl plugin.
 * All IDRs, Areas, and Fields are read-only in this plugin.
 *
 * Return value:
 * SA_ERR_HPI_READ_ONLY - Normal - ilo2_ribcl only supports "read only" IDRs.
 **/
SaErrorT ilo2_ribcl_del_idr_field( void *hnd,
			SaHpiResourceIdT	ResourceId,
			SaHpiIdrIdT	IdrId,
			SaHpiEntryIdT	AreaId,
			SaHpiEntryIdT	FieldId)
{
	return SA_ERR_HPI_READ_ONLY;
}



/**
 * ilo2_ribcl_add_idr:
 * @oh_handler:	Pointer to the handler for this instance.
 * @event: 	Pointer to event structure for this resource's add event.
 * @idrid:	Index of this IDR.
 * @new_idr:	Pointer to structure containing all initial IDR data.
 * @description: String describing the resource containing this IDR.
 *
 * This routine will create a new inventory RDR, associated with the IDR
 * contents passed via parameter new_idr. The information is copied from
 * new_idr, so the caller is free to free or change the information later.
 *
 * The new inventory RDR will be added to the resource contained within the
 * 'event' parameter. The following inventory RDR elements will be at these
 * fixed values:
 *	IsFru   = SAHPI_FALSE
 *	InventoryRec.Persistent = SAHPI_FALSE
 *
 * The IdString for the new IDR will be constructed by appending the string
 * " Inventory" to the string passed in parameter 'description'.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_OUT_OF_MEMORY - Memory allocation failed.
 * SA_ERR_HPI_INTERNAL_ERROR - could not add inventory RDR
 **/
SaErrorT ilo2_ribcl_add_idr( struct oh_handler_state *oh_handler,
			struct oh_event *event,
			SaHpiIdrIdT idrid,
			struct ilo2_ribcl_idr_info *new_idr,
			char *description)
{
	SaErrorT ret = SA_OK;
	SaHpiRdrT *rdr;
	struct ilo2_ribcl_idr_info *idr;

	rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
	if( rdr == NULL){
		err("ilo2_ribcl_add_idr: Memory allocation failed.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	/* fill in generic RDR stuff */
	rdr->RdrType = SAHPI_INVENTORY_RDR; 
	rdr->Entity  = event->resource.ResourceEntity;
	rdr->IsFru   = SAHPI_FALSE;
	oh_init_textbuffer(&(rdr->IdString));
	oh_append_textbuffer(&(rdr->IdString), description);
	oh_append_textbuffer(&(rdr->IdString), " Inventory");

	/* Fill in the IDR sepcific stuff */
	rdr->RdrTypeUnion.InventoryRec.IdrId = idrid;
	rdr->RdrTypeUnion.InventoryRec.Persistent = SAHPI_FALSE;

	/* Copy the IDR information into a new allocation to be associated
	 * with this RDR */

	idr = g_memdup(new_idr, sizeof(struct ilo2_ribcl_idr_info));
	if( idr == NULL){
		g_free( rdr);
		err("ilo2_ribcl_add_idr: Memory allocation failed.");
		return(SA_ERR_HPI_OUT_OF_MEMORY);
	}

	ret = oh_add_rdr(oh_handler->rptcache, event->resource.ResourceId,
		rdr, idr, 0); 
	if( ret != SA_OK){
		err("ilo2_ribcl_add_idr: could not add RDR. Error = %s.",
			oh_lookup_error(ret));
		g_free( idr);
		g_free( rdr);
		return( SA_ERR_HPI_INTERNAL_ERROR);
	} else {
		event->rdrs = g_slist_append(event->rdrs, rdr);
	}

	ilo2_ribcl_add_resource_capability( oh_handler, event,
		      (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_INVENTORY_DATA));

	return( SA_OK);

} /* end ilo2_ribcl_add_idr() */



/**
 * ilo2_ribcl_discover_chassis_idr:
 * @oh_handler:		Pointer to the handler for this instance.
 * @chassis_ep:		Entity path for the chassis.
 *
 * This routine builds the IDR for the system chassis, and adds an inventory
 * RDR to the chassis resource associated with that IDR.
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_discover_chassis_idr( struct oh_handler_state *oh_handler,
			struct oh_event *event,
			char *description)
{
	ilo2_ribcl_handler_t *ir_handler = NULL;
	ilo2_ribcl_DiscoveryData_t *ddata;

	ir_handler = (ilo2_ribcl_handler_t *) oh_handler->data;
	ddata = &(ir_handler->DiscoveryData);

	/* Use the temporary ilo2_ribcl_idr_info structure in our
	 * private handler to collect the IDR information. We use
	 * this buffer in the handler because it's too large to put
	 * on the stack as a local valiable, and we don't want to be
	 * allocating/deallocating it frequently. */

	ilo2_ribcl_build_chassis_idr( ir_handler, &(ir_handler->tmp_idr));

	if( ilo2_ribcl_add_idr( oh_handler, event, SAHPI_DEFAULT_INVENTORY_ID, 
				&(ir_handler->tmp_idr), description) != SA_OK){
		err("ilo2_ribcl_discover_chassis_idr: could not add IDR for chassis.");
		return;
	}

	ilo2_ribcl_add_resource_capability( oh_handler, event,
		     (SAHPI_CAPABILITY_RDR | SAHPI_CAPABILITY_INVENTORY_DATA));

} /* end ilo2_ribcl_discover_chassis_idr() */



/**
 * ilo2_ribcl_update_chassis_idr:
 * @oh_handler:		Pointer to the handler for this instance.
 * @chassis_ep:		Entity path for the chassis.
 *
 * This routine updates the information in the system chassis IDR with
 * any differing information obtained during a discovery operation.
 *
 * We make a call to ilo2_ribcl_get_idr_allinfo_by_ep() to obtain the
 * inventory RDR, the rpt entry for the resource containing the IDR, and
 * the struct ilo2_ribcl_idr_info that contains all our IDR data.
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_update_chassis_idr( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *ep_root)
{
	struct ilo2_ribcl_idr_allinfo idr_allinfo;
	ilo2_ribcl_handler_t *ir_handler;

	ir_handler = (ilo2_ribcl_handler_t *)oh_handler->data;

	/* Look up our chassis IDR. */
	if( ilo2_ribcl_get_idr_allinfo_by_ep( oh_handler, ep_root,
			SAHPI_DEFAULT_INVENTORY_ID, &idr_allinfo) != SA_OK){
		err("ilo2_ribcl_update_chassis_idr: unable to locate chassis IDR.");
		return;
	}

	ilo2_ribcl_build_chassis_idr( ir_handler, &(ir_handler->tmp_idr));

	ilo2_ribcl_update_idr( &(ir_handler->tmp_idr), idr_allinfo.idrinfo);
 
} /* ilo2_ribcl_update_chassis_idr() */



/**
 * ilo2_ribcl_update_fru_idr:
 * @oh_handler:		Pointer to the handler for this instance.
 * @fru_ep:		Entity path for the fru resource.
 * @idr_info:		(Potentially) updated information for the resource IDR.
 *
 * This routine updates the information in an IDR of a FRU with any differing
 * information obtained during a discovery operation.
 *
 * We make a call to ilo2_ribcl_get_idr_allinfo_by_ep() to obtain the
 * inventory RDR, the rpt entry for the resource containing the IDR, and
 * the struct ilo2_ribcl_idr_info that contains all our IDR data.
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_update_fru_idr( struct oh_handler_state *oh_handler,
				SaHpiEntityPathT *fru_ep,
				struct ilo2_ribcl_idr_info * idr_info)
{
	struct ilo2_ribcl_idr_allinfo idr_allinfo;

	/* First, find the IDR for this resource, using the entity path */
	if( ilo2_ribcl_get_idr_allinfo_by_ep( oh_handler, fru_ep,
			SAHPI_DEFAULT_INVENTORY_ID, &idr_allinfo) != SA_OK){

		err("ilo2_ribcl_update_fru_idr: unable to locate IDR for FRU.");
		return;
	}
	ilo2_ribcl_update_idr( idr_info, idr_allinfo.idrinfo);

} /* end ilo2_ribcl_update_fru_idr() */



/**
 * ilo2_ribcl_get_idr_allinfo_by_ep:
 * @oh_handler: Pointer to the handler for this instance.
 * @ep:		Entity path for resource containing this IDR.
 * @irid:	Index of this IDR.
 * @idr_allinfo: Pointer to structure used to return other pointers.
 *
 * This is a support routine used within our plugin. It returns a pointer to
 * the inventory RDR, a pointer to the rpt entry for the resource containing
 * the IDR, and a pointer to the struct ilo2_ribcl_idr_info that is associated
 * with the inventory RDR. These pointers are returned via the idr_allinfo
 * parameter, which should point to a struct ilo2_ribcl_idr_allinfo that has
 * been allocated by our caller.
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_INVALID_RESOURCE - Could not locate rpt entry.
 * SA_ERR_HPI_CAPABILITY - Resource does not have inventory capability.
 * SA_ERR_HPI_INTERNAL_ERROR - No IDR data exists in the RDR.
 * SA_ERR_HPI_NOT_PRESENT - The requested IDR can not be found.
 *
 **/
static SaErrorT ilo2_ribcl_get_idr_allinfo_by_ep(
			struct oh_handler_state *oh_handler,
			SaHpiEntityPathT *ep,
			SaHpiIdrIdT irid,
			struct ilo2_ribcl_idr_allinfo *idr_allinfo)
{
	SaHpiResourceIdT rid;

	idr_allinfo->rpt = NULL;
	idr_allinfo->rdr = NULL;
	idr_allinfo->idrinfo = NULL;

	/* Check that the resource exists, and that it has IDR capability */

	idr_allinfo->rpt = oh_get_resource_by_ep(oh_handler->rptcache, ep);
	if( !idr_allinfo->rpt){
		err("ilo2_ribcl_get_idr_allinfo_by_ep: no rpt entry.");
		return( SA_ERR_HPI_INVALID_RESOURCE);
	}
	rid = idr_allinfo->rpt->ResourceId;

	if( !(idr_allinfo->rpt->ResourceCapabilities &
					     SAHPI_CAPABILITY_INVENTORY_DATA)){
		err("ilo2_ribcl_get_idr_allinfo_by_ep: no inventory capability for resource id %d.", rid);
		return( SA_ERR_HPI_CAPABILITY);
	}

	/* Get the RDR for the inventory */

	idr_allinfo->rdr = oh_get_rdr_by_type(oh_handler->rptcache, rid,
						SAHPI_INVENTORY_RDR, irid);

	if( idr_allinfo->rdr == NULL){
		err("ilo2_ribcl_get_idr_allinfo_by_ep: no inventory RDR for resource id %d, IDR %d.",
			rid, irid);
		return( SA_ERR_HPI_NOT_PRESENT);
	}

	/* Finally, get the assoicated data for this IDR */

	idr_allinfo->idrinfo = (struct ilo2_ribcl_idr_info *)oh_get_rdr_data(
		oh_handler->rptcache, rid, idr_allinfo->rdr->RecordId);

	if( idr_allinfo->idrinfo == NULL){
		err("ilo2_ribcl_get_idr_allinfo_by_ep: no inventory data found for resource id %d, IDR %d, label %s.",
			rid, irid, idr_allinfo->rdr->IdString.Data); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return( SA_OK);

} /* end ilo2_ribcl_get_idr_allinfo_by_ep() */



/**
 * ilo2_ribcl_get_idr_allinfo:
 * @oh_handler: Pointer to the handler for this instance.
 * @rid:	Resource ID for resource containing this IDR.
 * @irid:	Index of this IDR.
 * @idr_allinfo: Pointer to structure used to return other pointers.
 *
 * This is a support routine used within our plugin. It returns a pointer to
 * the inventory RDR, a pointer to the rpt entry for the resource containing
 * the IDR, and a pointer to the struct ilo2_ribcl_idr_info that is associated
 * with the inventory RDR. These pointers are returned via the idr_allinfo
 * parameter, which should point to a struct ilo2_ribcl_idr_allinfo that has
 * been allocated by our caller.
 *
 * This routine is similar to ilo2_ribcl_get_idr_allinfo_by_ep(), however here
 * we search by resource ID rather than by entity path. 
 *
 * Return values:
 * SA_OK - Normal, successful return.
 * SA_ERR_HPI_INVALID_RESOURCE - Could not locate rpt entry.
 * SA_ERR_HPI_CAPABILITY - Resource does not have inventory capability.
 * SA_ERR_HPI_INTERNAL_ERROR - No IDR data exists in the RDR.
 * SA_ERR_HPI_NOT_PRESENT - The requested IDR can not be found.
 **/
static SaErrorT ilo2_ribcl_get_idr_allinfo( struct oh_handler_state *oh_handler,
			SaHpiResourceIdT rid,
			SaHpiIdrIdT irid,
			struct ilo2_ribcl_idr_allinfo *idr_allinfo)
{
	idr_allinfo->rpt = NULL;
	idr_allinfo->rdr = NULL;
	idr_allinfo->idrinfo = NULL;

	/* Check that the resource exists, and that it has IDR capability */

	idr_allinfo->rpt = oh_get_resource_by_id(oh_handler->rptcache, rid);
	if( !idr_allinfo->rpt){
		err("ilo2_ribcl_get_idr_allinfo: no rpt entry for resource id %d.", rid);
		return( SA_ERR_HPI_INVALID_RESOURCE);
	}

	if( !(idr_allinfo->rpt->ResourceCapabilities &
					     SAHPI_CAPABILITY_INVENTORY_DATA)){
		err("ilo2_ribcl_get_idr_allinfo: no inventory capability for resource id %d.", rid);
		return( SA_ERR_HPI_CAPABILITY);
	}

	/* Get the RDR for the inventory */

	idr_allinfo->rdr = oh_get_rdr_by_type(oh_handler->rptcache, rid,
						SAHPI_INVENTORY_RDR, irid);

	if( idr_allinfo->rdr == NULL){
		err("ilo2_ribcl_get_idr_allinfo: no inventory RDR for resource id %d, IDR %d.",
			rid, irid);
		return( SA_ERR_HPI_NOT_PRESENT);
	}

	/* Finally, get the assoicated data for this IDR */

	idr_allinfo->idrinfo = (struct ilo2_ribcl_idr_info *)oh_get_rdr_data(
		oh_handler->rptcache, rid, idr_allinfo->rdr->RecordId);

	if( idr_allinfo->idrinfo == NULL){
		err("ilo2_ribcl_get_idr_allinfo: no inventory data found for resource id %d, IDR %d, label %s.",
			rid, irid, idr_allinfo->rdr->IdString.Data); 
		return( SA_ERR_HPI_INTERNAL_ERROR);
	}

	return( SA_OK);
} /* end ilo2_ribcl_get_idr_allinfo() */



/**
 * ilo2_ribcl_field_catstring:
 * @field:	Pointer to I2R_FieldT whoose string should be modified.
 * @str:	String to concat to the field string.
 *
 * Concats the string str onto the end of the fieldstring of the field
 * specified by the "field" parameter. If str is null, the string
 * "Unknown" will be added instead. The total length of the field string
 * will be limited to I2R_MAX_FIELDCHARS characters.
 *
 * Return values:
 * None
 **/
static void ilo2_ribcl_field_catstring( I2R_FieldT *field, char *str)
{
	char *tmpstr;
	int exist_len;

	exist_len = strlen( field->field_string) +1; /* account for the null */

	if( str){
		tmpstr = str;
	} else {
		tmpstr = "Unknown";
	}

	strncat( field->field_string, tmpstr, (I2R_MAX_FIELDCHARS - exist_len));
}



/**
 * ilo2_ribcl_build_chassis_idr:
 * @ir_handler: Pointer to plugin private handler.
 * @idr_info:	Pointer to structure that receives the IDR data/
 *
 * This routine reads information obtained during the prevoius discovery
 * operation, and fills in the IDR for a the system chassis. The idr_info
 * parameter must point to a struct ilo2_ribcl_idr_info that has been
 * allocated by our caller.
 *
 * The system chassis IDR has one Area of type SAHPI_IDR_AREATYPE_CHASSIS_INFO,
 * containing the following four fields:
 *	Field 1: SAHPI_IDR_FIELDTYPE_PRODUCT_NAME "<system product number>"
 *	Field 2: SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER "<system serial numner>" 
 *	Field 3: SAHPI_IDR_FIELDTYPE_MANUFACTURER "Hewlett Packard"
 *	Field 4: SAHPI_IDR_FIELDTYPE_CUSTOM "iLo2_Firmware: <firmware version>"
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_build_chassis_idr( ilo2_ribcl_handler_t *ir_handler,
			struct ilo2_ribcl_idr_info *idr_info)
{
	I2R_FieldT *field;
	ilo2_ribcl_DiscoveryData_t *ddata;

	memset( idr_info, 0, sizeof( struct ilo2_ribcl_idr_info));
	ddata = &(ir_handler->DiscoveryData);

	idr_info->num_areas = 1;
	idr_info->idr_areas[0].area_type = SAHPI_IDR_AREATYPE_CHASSIS_INFO;
	idr_info->idr_areas[0].num_fields = 4;

	field = &(idr_info->idr_areas[0].area_fields[I2R_CHASSIS_IF_PRODNAME]);
	field->field_type = SAHPI_IDR_FIELDTYPE_PRODUCT_NAME;
	ilo2_ribcl_field_catstring( field, ddata->product_name);

	field = &(idr_info->idr_areas[0].area_fields[I2R_CHASSIS_IF_SERNUM]);
	field->field_type = SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER;
	ilo2_ribcl_field_catstring( field, ddata->serial_number);

	field = &(idr_info->idr_areas[0].area_fields[I2R_CHASSIS_IF_MANUFACT]);
	field->field_type = SAHPI_IDR_FIELDTYPE_MANUFACTURER;
	ilo2_ribcl_field_catstring( field, "Hewlett Packard");

	field = &(idr_info->idr_areas[0].area_fields[I2R_CHASSIS_IF_ILO2VERS]);
	field->field_type = SAHPI_IDR_FIELDTYPE_CUSTOM;
	ilo2_ribcl_field_catstring( field, "iLo2_Firmware: ");
	ilo2_ribcl_field_catstring( field, ddata->fwdata.version_string);
	
} /* end ilo2_ribcl_build_chassis_idr() */ 



/**
 * ilo2_ribcl_build_cpu_idr:
 * @ir_handler: Pointer to plugin private handler.
 * @idr_info:	Pointer to structure that receives the IDR data/
 *
 * This routine reads information obtained during the prevoius discovery
 * operation, and fills in the IDR for a system processor. The idr_info
 * parameter must point to a struct ilo2_ribcl_idr_info that has been
 * allocated by our caller.
 *
 * The cpu IDR has one Area of type SAHPI_IDR_AREATYPE_BOARD_INFO,
 * containing the following field:
 *	Field 1: SAHPI_IDR_FIELDTYPE_CUSTOM "Speed: <cpu speed>"
 * 
 * Return values:
 * None
 **/
void ilo2_ribcl_build_cpu_idr(ilo2_ribcl_handler_t *ir_handler,
			struct ilo2_ribcl_idr_info *idr_info)
{
	I2R_FieldT *field;

	memset( idr_info, 0, sizeof( struct ilo2_ribcl_idr_info));
	idr_info->num_areas = 1;
	idr_info->idr_areas[0].area_type = SAHPI_IDR_AREATYPE_BOARD_INFO;
	idr_info->idr_areas[0].num_fields = 1;

	field = &(idr_info->idr_areas[0].area_fields[I2R_CPU_IF_SPEED]);
	field->field_type = SAHPI_IDR_FIELDTYPE_CUSTOM;
	ilo2_ribcl_field_catstring( field, "Speed: ");
	ilo2_ribcl_field_catstring( field,
				    ir_handler->DiscoveryData.system_cpu_speed);

} /* end ilo2_ribcl_build_cpu_idr() */



/**
 *  ilo2_ribcl_build_memory_idr:
 * @mem_data:	Pointer to ir_memdata_t info in DiscoveryData
 * @idr_info:	Pointer to structure that receives the IDR data/
 *
 * This routine reads information obtained during the prevoius discovery
 * operation, and fills in the IDR for a memory DIMM. The idr_info
 * parameter must point to a struct ilo2_ribcl_idr_info that has been
 * allocated by our caller.
 *
 * The memory IDR has one Area of type SAHPI_IDR_AREATYPE_BOARD_INFO,
 * containing the following two fields:
 *	Field 1: SAHPI_IDR_FIELDTYPE_CUSTOM "Size: <memory size>"
 *	Field 2: SAHPI_IDR_FIELDTYPE_CUSTOM "Speed: <memory speed>"
 *
 * Return values:
 * None
 **/
void ilo2_ribcl_build_memory_idr( ir_memdata_t *mem_data,
			struct ilo2_ribcl_idr_info *idr_info)
{
	I2R_FieldT *field;

	memset( idr_info, 0, sizeof( struct ilo2_ribcl_idr_info));

	idr_info->num_areas = 1;
	idr_info->idr_areas[0].area_type = SAHPI_IDR_AREATYPE_BOARD_INFO;
	idr_info->idr_areas[0].num_fields = 2;

	field = &(idr_info->idr_areas[0].area_fields[I2R_MEM_IF_SIZE]);
	field->field_type = SAHPI_IDR_FIELDTYPE_CUSTOM;
	ilo2_ribcl_field_catstring( field, "Size: ");
	ilo2_ribcl_field_catstring( field,  mem_data->memsize);

	field = &(idr_info->idr_areas[0].area_fields[I2R_MEM_IF_SPEED]);
	field->field_type = SAHPI_IDR_FIELDTYPE_CUSTOM;
	ilo2_ribcl_field_catstring( field, "Speed: ");
	ilo2_ribcl_field_catstring( field,  mem_data->speed);

} /* end ilo2_ribcl_build_memory_idr() */



/**
 * ilo2_ribcl_update_idr:
 * @new_info: Ptr to structure containing (potentially) new IDR information
 * @exist_info: Ptr to IDR info stored in a resource's RDR.
 *
 * This routine is given pointers to a new IDR and an existing IDR. It will
 * update the information in the existing IDR with any differing information
 * in the new IDR. If any of the information is actually updated, the update
 * count in the existing IDR wil be incremented.
 *
 * Currently, we only have one Area in all of our IDRs, but this code should
 * handle multiple Areas if we ever implement them.
 *
 * Return values:
 * Number of updates made to the existing IDR.
 *
 **/
static int ilo2_ribcl_update_idr( struct ilo2_ribcl_idr_info *new_info,
			struct ilo2_ribcl_idr_info *exist_info)
{
	I2R_AreaT *n_area;
	I2R_AreaT *e_area;
	I2R_FieldT *n_field;
	I2R_FieldT *e_field;
	int adx;
	int fdx;
	int updates = 0;

	for( adx = 0; adx < new_info->num_areas; adx++){
		n_area = &(new_info->idr_areas[adx]);
		e_area = &(exist_info->idr_areas[adx]);

		for( fdx = 0; fdx < n_area->num_fields; fdx++){
			n_field = &(n_area->area_fields[fdx]);
			e_field = &(e_area->area_fields[fdx]);
			if( strcmp( n_field->field_string,
						       e_field->field_string)){
	
				strcpy( e_field->field_string,
						 	n_field->field_string);
				updates++;
				exist_info->update_count++;
			}
		}
	
	}

	return( updates);

} /* end ilo2_ribcl_update_idr() */


/*****************************
	OpenHPI plug-in to iLO2 RIBCL plug-in ABI function mapping
*****************************/

void * oh_get_idr_info (void *hnd, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrInfoT *)
		__attribute__ ((weak, alias("ilo2_ribcl_get_idr_info")));

void * oh_get_idr_area_header (void *, SaHpiResourceIdT, SaHpiIdrIdT,
			       SaHpiIdrAreaTypeT, SaHpiEntryIdT,
			       SaHpiEntryIdT *, SaHpiIdrAreaHeaderT *)
                __attribute__ ((weak, alias("ilo2_ribcl_get_idr_area_header")));

void * oh_get_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldTypeT, SaHpiEntryIdT, SaHpiEntryIdT *,
                         SaHpiIdrFieldT *)
                __attribute__ ((weak, alias("ilo2_ribcl_get_idr_field")));

/* The following are just stubbs, since our IDRs are read only */
 
void * oh_add_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT,
			SaHpiIdrAreaTypeT, SaHpiEntryIdT)
		__attribute__ ((weak, alias("ilo2_ribcl_add_idr_area")));

void * oh_del_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT)
		__attribute__ ((weak, alias("ilo2_ribcl_del_idr_area")));

void * oh_add_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
		__attribute__ ((weak, alias("ilo2_ribcl_add_idr_field")));

void * oh_set_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
		__attribute__ ((weak, alias("ilo2_ribcl_set_idr_field")));

void * oh_del_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
			SaHpiEntryIdT)
		__attribute__ ((weak, alias("ilo2_ribcl_del_idr_field")));

