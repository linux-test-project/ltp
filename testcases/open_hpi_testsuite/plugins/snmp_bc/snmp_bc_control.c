/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 */

#include <snmp_bc_plugin.h>

/**
 * snmp_bc_get_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Location to store control's operational mode.
 * @state: Location to store control's state.
 *
 * Retrieves a control's operational mode and/or state. Both @mode and @state
 * may be NULL (e.g. check for presence).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_CMD - Control is write-only.
 * SA_ERR_HPI_INVALID_DATA - @state contain invalid text line number.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 **/
SaErrorT snmp_bc_get_control_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiCtrlNumT cid,
				   SaHpiCtrlModeT *mode,
				   SaHpiCtrlStateT *state)
{
	SaErrorT err;
	SaHpiCtrlStateT working_state;
        struct ControlInfo *cinfo;
	struct snmp_value get_value;
	
        struct oh_handler_state *handle;	
        struct snmp_bc_hnd *custom_handle;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	handle = (struct oh_handler_state *)hnd;	
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	memset(&working_state, 0, sizeof(SaHpiCtrlStateT));

	/* Check if resource exists and has control capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Find control and its mapping data - see if it accessable */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	cinfo = (struct ControlInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (cinfo == NULL) {
		err("No control data. Control=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	if (rdr->RdrTypeUnion.CtrlRec.WriteOnly) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_CMD);
	}
	if (!mode && !state) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_OK);
	}
	
	if (state) {
		if (state->Type == SAHPI_CTRL_TYPE_TEXT) {
			if (state->StateUnion.Text.Line != SAHPI_TLN_ALL_LINES ||
			    state->StateUnion.Text.Line > rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines) {
			    	snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INVALID_DATA);
			}
		}

		/* Find control's state */
		working_state.Type = rdr->RdrTypeUnion.CtrlRec.Type;
		
		err = snmp_bc_oid_snmp_get(custom_handle, &(rdr->Entity), cinfo->mib.loc_offset,
					   cinfo->mib.oid, &get_value, SAHPI_TRUE);
		if (err  || get_value.type != ASN_INTEGER) {
			err("Cannot read SNMP OID=%s; Type=%d.", cinfo->mib.oid, get_value.type);
			snmp_bc_unlock_handler(custom_handle);
			return(err);
		}
		
		switch (rdr->RdrTypeUnion.CtrlRec.Type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			if (cinfo->mib.isDigitalReadStateConstant) {
				/* If control always returns a constant state */
				working_state.StateUnion.Digital = cinfo->mib.DigitalStateConstantValue;
			}
			else { /* Translate SNMP reading into digital state */
				int i, found;
				
				found = 0;
				for (i=0; i<OH_MAX_CTRLSTATEDIGITAL; i++) {
					if (cinfo->mib.digitalmap[i] == get_value.integer) { 
						found++;
						break; 
					}
				}
				if (found) {
					switch (i) {
					case 0:
						working_state.StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
						break;
					case 1:
						working_state.StateUnion.Digital = SAHPI_CTRL_STATE_ON;
						break;
					case 2:
						working_state.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_OFF;
						break;
					case 3:
						working_state.StateUnion.Digital = SAHPI_CTRL_STATE_PULSE_ON;
						break;
					default:
						err("HPI Spec change: Add control digital states");
						snmp_bc_unlock_handler(custom_handle);
						return(SA_ERR_HPI_INTERNAL_ERROR);
					}
				} else {
					err("Control's value not defined");
					snmp_bc_unlock_handler(custom_handle);
					return(SA_ERR_HPI_INTERNAL_ERROR);
				}
			}
			break;
		case SAHPI_CTRL_TYPE_DISCRETE:
			working_state.StateUnion.Discrete = get_value.integer;
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			snmp_bc_unlock_handler(custom_handle);
			err("Analog controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_STREAM:
			snmp_bc_unlock_handler(custom_handle);
			err("Stream controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_TEXT:
			snmp_bc_unlock_handler(custom_handle);
			err("Text controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_OEM:
			snmp_bc_unlock_handler(custom_handle);
			err("Oem controls not supported.");
			return(SA_ERR_HPI_INTERNAL_ERROR);
		default:
			err("%s has invalid control state=%d.", cinfo->mib.oid, working_state.Type);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	if (state) memcpy(state, &working_state, sizeof(SaHpiCtrlStateT));
	if (mode) *mode = cinfo->cur_mode;
	
	snmp_bc_unlock_handler(custom_handle);
	return(SA_OK);
}

/**
 * snmp_bc_set_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Control's operational mode to set.
 * @state: Pointer to control's state to set.
 *
 * Sets a control's operational mode and/or state. 
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_REQUEST - @state contains bad text control data.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 * SA_ERR_HPI_READ_ONLY - Change mode of a read-only mode control.
 *                        Note this is only returned if the specified mode
 *                        is different than the control's default mode.
 **/
SaErrorT snmp_bc_set_control_state(void *hnd,
				   SaHpiResourceIdT rid,
				   SaHpiCtrlNumT cid,
				   SaHpiCtrlModeT mode,
				   SaHpiCtrlStateT *state)
{
	int value;
	SaErrorT err;
	SaHpiRptEntryT *rpt;
        SaHpiRdrT *rdr;
        struct ControlInfo *cinfo;
        struct oh_handler_state *handle;
        struct snmp_bc_hnd *custom_handle;
	struct snmp_value set_value;

	if (!hnd) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}	
		
	handle = (struct oh_handler_state *)hnd;
	custom_handle = (struct snmp_bc_hnd *)handle->data;
	
	if (!custom_handle) {
		err("Invalid parameter.");
		return(SA_ERR_HPI_INVALID_PARAMS);
	}

	snmp_bc_lock_handler(custom_handle);
	/* Check if resource exists and has control capabilities */
	rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INVALID_RESOURCE);
	}
	
        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_CAPABILITY);
	}

	/* Find control and its mapping data - see if it accessable */
        rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
	if (rdr == NULL) {
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_NOT_PRESENT);
	}
	
	cinfo = (struct ControlInfo *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
 	if (cinfo == NULL) {
		err("No control data. Control=%s", rdr->IdString.Data);
		snmp_bc_unlock_handler(custom_handle);
		return(SA_ERR_HPI_INTERNAL_ERROR);
	}       

	/* Validate static control state and mode data */
	err = oh_valid_ctrl_state_mode(&(rdr->RdrTypeUnion.CtrlRec), mode, state);
	if (err) {
		snmp_bc_unlock_handler(custom_handle);
		return(err);
	}

	/* Write control state */
	if (mode != SAHPI_CTRL_MODE_AUTO && state) {
		switch (state->Type) {
		case SAHPI_CTRL_TYPE_DIGITAL:
			/* Code with switch to discover spec changes that need to be reflected
                           in snmp_bc_resources.c digital read/write arrays */
			switch (state->StateUnion.Digital) {
			case SAHPI_CTRL_STATE_OFF:
				value = cinfo->mib.digitalwmap[SAHPI_CTRL_STATE_OFF];
				break;
			case SAHPI_CTRL_STATE_ON:
				value = cinfo->mib.digitalwmap[SAHPI_CTRL_STATE_ON];
				break;	
			case SAHPI_CTRL_STATE_PULSE_OFF:
				value = cinfo->mib.digitalwmap[SAHPI_CTRL_STATE_PULSE_OFF];
				break;
			case SAHPI_CTRL_STATE_PULSE_ON:
				value = cinfo->mib.digitalwmap[SAHPI_CTRL_STATE_PULSE_ON];
				break;
			default:
				err("HPI Spec change: Add control digital states");
				snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INTERNAL_ERROR);
			}

			if (value < 0) {
				err("Invalid hardware control state - %s",
				    oh_lookup_ctrlstatedigital(state->StateUnion.Digital));
				snmp_bc_unlock_handler(custom_handle);
				return(SA_ERR_HPI_INVALID_REQUEST);
			}
			
			set_value.type = ASN_INTEGER;
			set_value.str_len = 1;
			set_value.integer = value;
			
			err = snmp_bc_oid_snmp_set(custom_handle, &(rdr->Entity), cinfo->mib.loc_offset,
						   cinfo->mib.oid, set_value);
			if (err) {
				err("Cannot set SNMP OID=%s; Value=%d.", 
				    cinfo->mib.oid, (int)set_value.integer);
				snmp_bc_unlock_handler(custom_handle);
				return(err);
			}
			break;
		case SAHPI_CTRL_TYPE_DISCRETE:
			set_value.type = ASN_INTEGER;
			set_value.str_len = 1;
			set_value.integer = state->StateUnion.Discrete;
			
			err = snmp_bc_oid_snmp_set(custom_handle, &(rdr->Entity), cinfo->mib.loc_offset,
						   cinfo->mib.oid, set_value);
			if (err) {
				err("Cannot set SNMP OID=%s; Value=%d.", 
				    cinfo->mib.oid, (int)set_value.integer);
				snmp_bc_unlock_handler(custom_handle);
				return(err);
			}
			break;
		case SAHPI_CTRL_TYPE_ANALOG:
			err("Analog controls not supported.");
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_STREAM:
			err("Stream controls not supported.");
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_TEXT:
			err("Text controls not supported.");
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		case SAHPI_CTRL_TYPE_OEM:	
			err("OEM controls not supported.");
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		default:
			err("Invalid control state=%d", state->Type);
			snmp_bc_unlock_handler(custom_handle);
			return(SA_ERR_HPI_INTERNAL_ERROR);
		}
	}

	/* Write control mode, if changed */
	if (mode != cinfo->cur_mode) {
		cinfo->cur_mode = mode;
	}
	
	snmp_bc_unlock_handler(custom_handle);
        return(SA_OK);
}


void * oh_get_control_state (void *, SaHpiResourceIdT, SaHpiCtrlNumT,
                             SaHpiCtrlModeT *, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("snmp_bc_get_control_state")));

void * oh_set_control_state (void *, SaHpiResourceIdT,SaHpiCtrlNumT,
                             SaHpiCtrlModeT, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("snmp_bc_set_control_state")));


