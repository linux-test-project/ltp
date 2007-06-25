/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	Peter Phan  <pdphan@sourceforge.net>
 *      Renier Morales <renier@openhpi.org>
 */

#ifndef __SNMP_BC_INVENTORY_H
#define __SNMP_BC_INVENTORY_H

#include <snmp_bc.h>

#define NOS_BC_INVENTORY_FIELDS 10

/************************************************************************/
/* Resource one inventory data   					*/
/************************************************************************/
struct  bc_idr_area {
        SaHpiIdrAreaHeaderT  idrareas;
        SaHpiIdrFieldT  field[NOS_BC_INVENTORY_FIELDS];
};

struct bc_inventory_record {
        SaHpiIdrInfoT   idrinfo;
        struct bc_idr_area area[3];
};

/* 
 * Functions prototype
 */

/**
 * snmp_bc_get_idr_info:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_get_idr_info( void *hnd,  
		SaHpiResourceIdT        ResourceId,
		SaHpiIdrIdT             IdrId,
		SaHpiIdrInfoT          *IdrInfo);

/**
 * snmp_bc_get_idr_area_header:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_get_idr_area_header( void *hnd,
		SaHpiResourceIdT         ResourceId,
		SaHpiIdrIdT              IdrId,
		SaHpiIdrAreaTypeT        AreaType,
		SaHpiEntryIdT            AreaId,
		SaHpiEntryIdT           *NextAreaId,
		SaHpiIdrAreaHeaderT     *Header);

/**
 * snmp_bc_add_idr_area:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_add_idr_area( void *hnd,
		SaHpiResourceIdT         ResourceId,
		SaHpiIdrIdT              IdrId,
		SaHpiIdrAreaTypeT        AreaType,
		SaHpiEntryIdT           *AreaId);

/**
 * snmp_bc_del_idr_area:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_del_idr_area( void *hnd,
		SaHpiResourceIdT       ResourceId,
		SaHpiIdrIdT            IdrId,
		SaHpiEntryIdT          AreaId);

/**
 * snmp_bc_get_idr_field:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_get_idr_field( void *hnd,
		SaHpiResourceIdT       ResourceId,
		SaHpiIdrIdT             IdrId,
		SaHpiEntryIdT           AreaId,
		SaHpiIdrFieldTypeT      FieldType,
		SaHpiEntryIdT           FieldId,
		SaHpiEntryIdT          *NextFieldId,
		SaHpiIdrFieldT         *Field);

/**
 * snmp_bc_add_idr_field:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_add_idr_field( void *hnd,
		SaHpiResourceIdT         ResourceId,
		SaHpiIdrIdT              IdrId,
		SaHpiIdrFieldT        	*Field);

/**
 * snmp_bc_set_idr_field:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_set_idr_field( void *hnd,
		SaHpiResourceIdT         ResourceId,
		SaHpiIdrIdT              IdrId,
		SaHpiIdrFieldT           *Field);

/**
 * snmp_bc_del_idr_field:
 * @hnd:
 * @event:
 * @timeout:
 *
 * Return value:
 **/
SaErrorT snmp_bc_del_idr_field( void *hnd, 
		SaHpiResourceIdT         ResourceId,
		SaHpiIdrIdT              IdrId,
		SaHpiEntryIdT            AreaId,
		SaHpiEntryIdT            FieldId);

/**
 * vpd_exists:           
 * @thisMib:
 *
 * Return value:
 **/	
SaHpiBoolT vpd_exists(struct InventoryMibInfo *thisMib);			
#endif
