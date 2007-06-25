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
 *        Christina Hernandez <hernanc@us.ibm.com>
 *        W. David Ashley <dashley@us.ibm.com>
 */

#include <sim_init.h>


SaErrorT sim_get_control_state(void *hnd,
                                   SaHpiResourceIdT rid,
                                   SaHpiCtrlNumT cid,
                                   SaHpiCtrlModeT *mode,
                                   SaHpiCtrlStateT *state) {
        SaHpiCtrlStateT working_state;
        struct sim_control_info *info;

        if (!hnd) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;

        memset(&working_state, 0, sizeof(SaHpiCtrlStateT));

        /* Check if resource exists and has control capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
                return(SA_ERR_HPI_INVALID_RESOURCE);
        }

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
                return(SA_ERR_HPI_CAPABILITY);
        }

        /* Find control and its mapping data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
        if (rdr == NULL) {
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        info = (struct sim_control_info *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                dbg("No control data. Control=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        if (rdr->RdrTypeUnion.CtrlRec.WriteOnly) {
                return(SA_ERR_HPI_INVALID_CMD);
        }
        if (!mode && !state) {
                return(SA_OK);
        }

        if (state) {
                if (rdr->RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_TEXT) {
                        if (state->StateUnion.Text.Line != SAHPI_TLN_ALL_LINES &&
                            state->StateUnion.Text.Line > rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines) {
                                return(SA_ERR_HPI_INVALID_DATA);
                        }
                }

                /* Find control's state */
                working_state.Type = rdr->RdrTypeUnion.CtrlRec.Type;

                switch (rdr->RdrTypeUnion.CtrlRec.Type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        working_state.StateUnion.Digital =
                                rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default;
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        working_state.StateUnion.Discrete =
                                rdr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default;
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        working_state.StateUnion.Analog =
                                rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default;
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        memcpy(&working_state.StateUnion.Stream,
                               &rdr->RdrTypeUnion.CtrlRec.TypeUnion.Stream,
                               sizeof(SaHpiCtrlStateStreamT));
                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        memcpy(&working_state.StateUnion.Text,
                               &rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default,
                               sizeof(SaHpiCtrlStateTextT));
                        break;
                case SAHPI_CTRL_TYPE_OEM:
                        memcpy(&working_state.StateUnion.Oem,
                               &rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default,
                               sizeof(SaHpiCtrlStateTextT));
                        break;
                default:
//                      dbg("%s has invalid control state=%d.", cinfo->mib.oid, working_state.Type);
                        return(SA_ERR_HPI_INVALID_DATA);
                }
        }

        if (state) memcpy(state, &working_state, sizeof(SaHpiCtrlStateT));
        if (mode)
                *mode = info->mode;

        return(SA_OK);
}

/**
 * sim_set_control_state:
 * @hnd: Handler data pointer.
 * @rid: Resource ID.
 * @cid: Control ID.
 * @mode: Control's operational mode to set.
 * @state: Pointer to control's state to set.
 *
 * Sets a control's operational mode and/or state. Both @mode and @state
 * may be NULL (e.g. check for presence).
 *
 * Return values:
 * SA_OK - Normal case.
 * SA_ERR_HPI_CAPABILITY - Resource doesn't have SAHPI_CAPABILITY_CONTROL.
 * SA_ERR_HPI_INVALID_DATA - @state contains bad text control data.
 * SA_ERR_HPI_INVALID_RESOURCE - Resource doesn't exist.
 * SA_ERR_HPI_NOT_PRESENT - Control doesn't exist.
 **/
SaErrorT sim_set_control_state(void *hnd,
                                   SaHpiResourceIdT rid,
                                   SaHpiCtrlNumT cid,
                                   SaHpiCtrlModeT mode,
                                   SaHpiCtrlStateT *state)
{
        SaErrorT err;
        struct sim_control_info *info;

        struct oh_handler_state *handle = (struct oh_handler_state *)hnd;
        if (!hnd) {
                dbg("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }

        /* Check if resource exists and has control capabilities */
        SaHpiRptEntryT *rpt = oh_get_resource_by_id(handle->rptcache, rid);
        if (!rpt) {
                return(SA_ERR_HPI_INVALID_RESOURCE);
        }

        if (!(rpt->ResourceCapabilities & SAHPI_CAPABILITY_CONTROL)) {
                return(SA_ERR_HPI_CAPABILITY);
        }

        /* Find control and its mapping data - see if it accessable */
        SaHpiRdrT *rdr = oh_get_rdr_by_type(handle->rptcache, rid, SAHPI_CTRL_RDR, cid);
        if (rdr == NULL) {
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /*Note: cinfo must be changed to write to David A's API, not the rptcache*/
        info = (struct sim_control_info *)oh_get_rdr_data(handle->rptcache, rid, rdr->RecordId);
        if (info == NULL) {
                dbg("No control data. Control=%s", rdr->IdString.Data);
                return(SA_ERR_HPI_NOT_PRESENT);
        }

        /* Validate static control state and mode data */
        err = oh_valid_ctrl_state_mode(&(rdr->RdrTypeUnion.CtrlRec), mode, state);
        if (err) {
                return(err);
        }

        /* Write control state */
        if (mode != SAHPI_CTRL_MODE_AUTO && state) {
                switch (state->Type) {
                case SAHPI_CTRL_TYPE_DIGITAL:
                        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default =
                                state->StateUnion.Digital;
                        break;
                case SAHPI_CTRL_TYPE_DISCRETE:
                        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Discrete.Default =
                                state->StateUnion.Discrete;
                        break;
                case SAHPI_CTRL_TYPE_ANALOG:
                        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Analog.Default =
                                state->StateUnion.Analog;
                        break;
                case SAHPI_CTRL_TYPE_STREAM:
                        memcpy(&rdr->RdrTypeUnion.CtrlRec.TypeUnion.Stream.Default,
                               &state->StateUnion.Stream,
                               sizeof(SaHpiCtrlStateStreamT));
                        break;
                case SAHPI_CTRL_TYPE_TEXT:
                        memcpy(&rdr->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default,
                               &state->StateUnion.Text,
                               sizeof(SaHpiCtrlStateTextT));
                        break;
                case SAHPI_CTRL_TYPE_OEM:
                        memcpy(&rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.Default,
                               &state->StateUnion.Oem,
                               sizeof(SaHpiCtrlStateOemT));
                        break;
                default:
                        dbg("Invalid control state=%d", state->Type);
                        return(SA_ERR_HPI_INVALID_DATA);
                }
        }

        /* Write control mode, if changed */
        /*Change to write to David A's API, not the rptcache*/
        if (mode != info->mode) {
                info->mode = mode;
        }

        return(SA_OK);
}


/*
 * Simulator plugin interface
 *
 */

void * oh_get_control_state (void *, SaHpiResourceIdT, SaHpiCtrlNumT,
                             SaHpiCtrlModeT *, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("sim_get_control_state")));

void * oh_set_control_state (void *, SaHpiResourceIdT,SaHpiCtrlNumT,
                             SaHpiCtrlModeT, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("sim_set_control_state")));



