/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <glib.h>
#include <SaHpi.h>

#include <openhpi.h>
#include <oh_plugin.h>
#include <snmp_util.h>

#include <bc_resources.h>
#include <snmp_bc.h>
#include <snmp_bc_utils.h>
#include <snmp_bc_hotswap.h>

SaErrorT snmp_bc_get_hotswap_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiHsStateT *state)
{
	gchar *oid;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRptEntryT *res = oh_get_resource_by_id(handle->rptcache, id);
	if(res == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
        struct ResourceMibInfo *s =
                (struct ResourceMibInfo *)oh_get_resource_data(handle->rptcache, id);
	if(s == NULL) {
		return -1;
	}
	if(s->OidHealth == NULL) { 
		return SA_ERR_HPI_INVALID_CMD;
	}

	oid = snmp_derive_objid(res->ResourceEntity, s->OidHealth);
	if(oid == NULL) {
		dbg("NULL SNMP OID returned for %s\n",s->OidHealth);
		return -1;
	}

	if((snmp_get(custom_handle->ss,oid,&get_value) == 0) && (get_value.type == ASN_INTEGER)) {
		if(get_value.integer == s->HealthyValue) { 
			*state = SAHPI_HS_STATE_ACTIVE_HEALTHY;
		}
		else { 
			*state = SAHPI_HS_STATE_ACTIVE_UNHEALTHY;
		}
        } else {
		dbg("Couldn't fetch SNMP %s vector; Type=%d\n",oid,get_value.type);
		g_free(oid);
		return -1;
	}

	g_free(oid);
        return SA_OK;
}

SaErrorT snmp_bc_set_hotswap_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiHsStateT state)
{
        return SA_ERR_HPI_UNSUPPORTED_API;
}

SaErrorT snmp_bc_request_hotswap_action(void *hnd, SaHpiResourceIdT id,
					SaHpiHsActionT act)
{
        return SA_ERR_HPI_UNSUPPORTED_API;
}

SaErrorT snmp_bc_get_reset_state(void *hnd, SaHpiResourceIdT id,
				 SaHpiResetActionT *act)
{
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct ResourceMibInfo *s =
                (struct ResourceMibInfo *)oh_get_resource_data(handle->rptcache, id);
	if(s == NULL) {
		return -1;
	}
	if(s->OidReset == NULL) { 
		return SA_ERR_HPI_INVALID_CMD;
	}

	*act = SAHPI_RESET_DEASSERT;

	return SA_OK;
}

SaErrorT snmp_bc_set_reset_state(void *hnd, SaHpiResourceIdT id,
				 SaHpiResetActionT act)
{
	gchar *oid;
        struct snmp_value set_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRptEntryT *res = oh_get_resource_by_id(handle->rptcache, id);
	if(res == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
        struct ResourceMibInfo *s =
                (struct ResourceMibInfo *)oh_get_resource_data(handle->rptcache, id);
	if(s == NULL) {
		return -1;
	}
	if(s->OidReset == NULL) { 
		return SA_ERR_HPI_INVALID_CMD;
	}

	switch (act) {
	case SAHPI_RESET_ASSERT: /* RESET_ASSERT = RESET_DEASSERT Action */
	case SAHPI_RESET_DEASSERT:
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_COLD_RESET: /* COLD = WARM Reset Action */
	case SAHPI_WARM_RESET:
		oid = snmp_derive_objid(res->ResourceEntity, s->OidReset);
		if(oid == NULL) {
			dbg("NULL SNMP OID returned for %s\n",s->OidReset);
			return -1;
		}
		
		set_value.type = ASN_INTEGER;
		set_value.integer = 1;
		
		if((snmp_set(custom_handle->ss, oid, set_value) != 0)) {
			dbg("SNMP could not set %s; Type=%d.\n",s->OidReset,set_value.type);
			g_free(oid);
			return SA_ERR_HPI_NO_RESPONSE;
		}
		g_free(oid);
		break;
	default:
		dbg("Invalid Reset Action Type - %d\n", act);
		return SA_ERR_HPI_INVALID_PARAMS;
	}

        return SA_OK;
}

SaErrorT snmp_bc_get_power_state(void *hnd, SaHpiResourceIdT id,
				 SaHpiHsPowerStateT *state)
{
	gchar *oid;
	int rtn_code = SA_OK;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRptEntryT *res = oh_get_resource_by_id(handle->rptcache, id);
	if(res == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
         struct ResourceMibInfo *s =
                (struct ResourceMibInfo *)oh_get_resource_data(handle->rptcache, id);
	if(s == NULL) {
		return -1;
	}
	if(s->OidPowerState == NULL) { 
		return SA_ERR_HPI_INVALID_CMD;
	}

	oid = snmp_derive_objid(res->ResourceEntity, s->OidPowerState);
	if(oid == NULL) {
		dbg("NULL SNMP OID returned for %s\n",s->OidPowerState);
		return -1;
	}

	if((snmp_get(custom_handle->ss,oid,&get_value) == 0) && (get_value.type == ASN_INTEGER)) {
		switch (get_value.integer) {
		case 0:
			*state = SAHPI_HS_POWER_OFF;
			break;
		case 1:
			*state = SAHPI_HS_POWER_ON;
			break;
		default:
			dbg("Invalid power state read for oid=%s\n",oid);
			rtn_code = -1;
		}
        } else {
		dbg("Couldn't fetch SNMP %s vector; Type=%d\n",oid,get_value.type);
		rtn_code = -1;
	}

	g_free(oid);
        return rtn_code;
}

SaErrorT snmp_bc_set_power_state(void *hnd, SaHpiResourceIdT id,
				 SaHpiHsPowerStateT state)
{
	gchar *oid;
	int rtn_code = SA_OK;
        struct snmp_value set_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRptEntryT *res = oh_get_resource_by_id(handle->rptcache, id);
	if(res == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
        struct ResourceMibInfo *s =
                (struct ResourceMibInfo *)oh_get_resource_data(handle->rptcache, id);
	if(s == NULL) {
		return -1;
	}
	if(s->OidPowerOnOff == NULL) { 
		return SA_ERR_HPI_INVALID_CMD; 
	}

	oid = snmp_derive_objid(res->ResourceEntity, s->OidPowerOnOff);
	if(oid == NULL) {
		dbg("NULL SNMP OID returned for %s\n",s->OidPowerOnOff);
		return -1;
	}

	set_value.type = ASN_INTEGER;
	switch (state) {
	case SAHPI_HS_POWER_OFF:
		set_value.integer = 0;
		if((snmp_set(custom_handle->ss, oid, set_value) != 0)) {
			dbg("SNMP could not set %s; Type=%d.\n",s->OidPowerOnOff,set_value.type);
			rtn_code = SA_ERR_HPI_NO_RESPONSE;
		}
		break;
		
	case SAHPI_HS_POWER_ON:
		set_value.integer = 1;
		if((snmp_set(custom_handle->ss, oid, set_value) != 0)) {
			dbg("SNMP could not set %s; Type=%d.\n",s->OidPowerOnOff,set_value.type);
			rtn_code = SA_ERR_HPI_NO_RESPONSE;
		}
		break;
		
	case SAHPI_HS_POWER_CYCLE:
	        {
			SaHpiResetActionT act = SAHPI_COLD_RESET;
			rtn_code=snmp_bc_set_reset_state(hnd, id, act);
	        }
		break;
	default:
		dbg("Invalid Power Action Type - %d\n", state);
		rtn_code = SA_ERR_HPI_INVALID_PARAMS;
	}

	g_free(oid);
        return rtn_code;
}

SaErrorT snmp_bc_get_indicator_state(void *hnd, SaHpiResourceIdT id,
				     SaHpiHsIndicatorStateT *state)
{
        return SA_ERR_HPI_INVALID_CMD;
}

SaErrorT snmp_bc_set_indicator_state(void *hnd, SaHpiResourceIdT id,
				     SaHpiHsIndicatorStateT state)
{
        return SA_ERR_HPI_INVALID_CMD;
}
