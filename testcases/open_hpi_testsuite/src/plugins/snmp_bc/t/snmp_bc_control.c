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
#include <snmp_bc_control.h>

SaErrorT snmp_bc_get_control_state(void *hnd, SaHpiResourceIdT id,
				   SaHpiCtrlNumT num,
				   SaHpiCtrlStateT *state)
{
        gchar *oid;
	int i, found;
	SaHpiCtrlStateT working;
        struct snmp_value get_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_CTRL_RDR, num);
	if(rdr == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
        struct ControlMibInfo *s =
                (struct ControlMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);
	if(s == NULL) {
		return -1;
	}	

	memset(&working, 0, sizeof(SaHpiCtrlStateT));
	working.Type = rdr->RdrTypeUnion.CtrlRec.Type;
	
	switch (working.Type) {
	case SAHPI_CTRL_TYPE_DIGITAL:
		oid = snmp_derive_objid(rdr->Entity, s->oid);
		if(oid == NULL) {
			dbg("NULL SNMP OID returned for %s\n",s->oid);
			return -1;
		}
		if((snmp_get(custom_handle->ss, oid, &get_value) != 0) | (get_value.type != ASN_INTEGER)) {
			dbg("SNMP could not read %s; Type=%d.\n", oid, get_value.type);
			g_free(oid);
			return SA_ERR_HPI_NO_RESPONSE;
		}
		g_free(oid);
		
		found = 0;
		/* Icky dependency on SaHpiStateDigitalT enum */
		for(i=0; i<ELEMENTS_IN_SaHpiStateDigitalT; i++) {
			if(s->digitalmap[i] == get_value.integer) { 
				found++;
				break; 
			}
		}

		if(found) {
			switch (i) {
			case 0:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
				break;
			case 1:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_ON;
				break;
			case 2:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_OFF;
				break;
			case 3:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
				break;
			case 4:
				working.StateUnion.Digital = SAHPI_CTRL_STATE_AUTO;
				break;
			default:
				dbg("Spec Change: MAX_SaHpiStateDigitalT incorrect?\n");
				return -1;
			}
		} else {
			dbg("Control's value not defined\n");
			return -1;
		}
		break;

	case SAHPI_CTRL_TYPE_DISCRETE:
		dbg("Discrete controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_ANALOG:
		dbg("Analog controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_STREAM:
		dbg("Stream controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_TEXT:
		dbg("Text controls not supported\n");
		return -1;
	case SAHPI_CTRL_TYPE_OEM:	
		dbg("Oem controls not supported\n");
		return -1;
        default:
		dbg("%s has invalid control state=%d\n", s->oid,working.Type);
                return -1;
        }

	memcpy(state,&working,sizeof(SaHpiCtrlStateT));
	return SA_OK;
}

SaErrorT snmp_bc_set_control_state(void *hnd, SaHpiResourceIdT id,
                                     SaHpiCtrlNumT num,
                                     SaHpiCtrlStateT *state)
{
        gchar *oid;
	int value;
        struct snmp_value set_value;
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        struct snmp_bc_hnd *custom_handle = (struct snmp_bc_hnd *)handle->data;

        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, id, SAHPI_CTRL_RDR, num);
	if(rdr == NULL) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
        struct ControlMibInfo *s =
                (struct ControlMibInfo *)oh_get_rdr_data(handle->rptcache, id, rdr->RecordId);
	if(s == NULL) {
		return -1;
	}

	if(state->Type != rdr->RdrTypeUnion.CtrlRec.Type) {
		dbg("Control %s type %d cannot be changed\n",s->oid,state->Type);
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	switch (state->Type) {
	case SAHPI_CTRL_TYPE_DIGITAL:

		/* More icky dependencies on SaHpiStateDigitalT enum */
		switch (state->StateUnion.Digital) {
		case SAHPI_CTRL_STATE_OFF:
			value = s->digitalmap[SAHPI_CTRL_STATE_OFF];
			break;
		case SAHPI_CTRL_STATE_ON:
			value = s->digitalmap[SAHPI_CTRL_STATE_ON];
			break;	
		case SAHPI_CTRL_STATE_PULSE_OFF:
			value = s->digitalmap[SAHPI_CTRL_STATE_PULSE_OFF];
			break;
		case SAHPI_CTRL_STATE_PULSE_ON:
			value = s->digitalmap[SAHPI_CTRL_STATE_PULSE_ON];
			break;
		case SAHPI_CTRL_STATE_AUTO:
			value = s->digitalmap[ELEMENTS_IN_SaHpiStateDigitalT - 1];
			break;
		default:
			dbg("Spec Change: MAX_SaHpiStateDigitalT incorrect?\n");
			return -1;
		}

		if(value < 0) {
			dbg("Control state %d not allowed to be set\n",state->StateUnion.Digital);
			return SA_ERR_HPI_INVALID_CMD;
		}

		oid = snmp_derive_objid(rdr->Entity, s->oid);
		if(oid == NULL) {
			dbg("NULL SNMP OID returned for %s\n",s->oid);
			return -1;
		}

		set_value.type = ASN_INTEGER;
		set_value.integer = value;

		if((snmp_set(custom_handle->ss, oid, set_value) != 0)) {
			dbg("SNMP could not set %s; Type=%d.\n",s->oid,set_value.type);
			g_free(oid);
			return SA_ERR_HPI_NO_RESPONSE;
		}
		g_free(oid);

	case SAHPI_CTRL_TYPE_DISCRETE:
		dbg("Discrete controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_ANALOG:
		dbg("Analog controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_STREAM:
		dbg("Stream controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_TEXT:
		dbg("Text controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
	case SAHPI_CTRL_TYPE_OEM:	
		dbg("Oem controls not supported\n");
		return SA_ERR_HPI_INVALID_CMD;
        default:
		dbg("Request has invalid control state=%d\n", state->Type);
                return SA_ERR_HPI_INVALID_PARAMS;
        }
	
        return SA_OK;
}
