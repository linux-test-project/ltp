/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Tariq Shureih    <tariq.shureih@intel.com>
 *     Louis Zhuang     <louis.zhuang@intel.com>
 */

 
#include "ipmi.h"

static unsigned char oem_alarm_state = 0xff;

struct ohoi_ctrl_info {
        int done;
	SaErrorT err;
        SaHpiRdrT * rdr;
        struct oh_handler_state *handler;
        SaHpiCtrlModeT mode;
        SaHpiCtrlStateT *state;
};

/* struct for getting power state */
struct ohoi_power_info {
	int done;
	SaErrorT err;
	SaHpiPowerStateT *state;
};

/* struct for getting reset state */
struct ohoi_reset_info {
	int done;
	SaErrorT err;
	SaHpiResetActionT *state;
};

static void __get_control_state(ipmi_control_t *control,
                                int            err,
                                int            *val,
                                void           *cb_data)
{
        struct ohoi_ctrl_info *info = cb_data;

	if (err || !val) {
		err("__get_control_state: err = %d; val = %p", err, val);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
       if (info->state->Type != SAHPI_CTRL_TYPE_OEM) {
                err("IPMI plug-in only support OEM control now");
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
                return;
        }
        
	if (err || !val) {
		err("__get_control_state: err = %d; val = %p", err, val);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
        info->state->StateUnion.Oem.BodyLength 
                = ipmi_control_get_num_vals(control);
        memcpy(&info->state->StateUnion.Oem.Body[0],
               val,
               info->state->StateUnion.Oem.BodyLength);
        info->done = 1;
}

static void __get_control_leds_state(ipmi_control_t *control,
                int err, ipmi_light_setting_t *settings, void *cb_data)
{
	struct ohoi_ctrl_info	*info = cb_data;
	int			len, res ,val;

	if (err) {
		err("__get_control_leds_state: err = %d", err);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
	if (info->state->Type != SAHPI_CTRL_TYPE_OEM) {
		err("IPMI plug-in only support OEM control now");
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
			               
	if (settings == (ipmi_light_setting_t *)NULL) {
		err("__get_control_leds_state: settings = NULL");
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
	len = ipmi_control_get_num_vals(control);
        info->state->StateUnion.Oem.BodyLength = len;
	res = ipmi_light_setting_get_color(settings, 0, &val);
        info->state->StateUnion.Oem.Body[0] = val;
        info->done = 1;
}



static void _get_control_state(ipmi_control_t *control,
                                void           *cb_data)
{
	if (ipmi_control_light_set_with_setting(control))
		ipmi_control_get_light(control, __get_control_leds_state, cb_data);
	else
        	ipmi_control_get_val(control, __get_control_state, cb_data);
}




		/*
		 *     ATCA LEDs
		 */

static void __get_atca_led(ipmi_control_t *control,
                	   int err,
			   ipmi_light_setting_t *st,
			   void *cb_data)
{
	struct ohoi_ctrl_info	*info = cb_data;
	SaHpiCtrlStateT *state = info->state;
	int lc;
	int on_time, off_time;
	int color;
	
	ipmi_light_setting_in_local_control(st, 0, &lc);
	info->mode = lc ? SAHPI_CTRL_MODE_AUTO : SAHPI_CTRL_MODE_MANUAL;
	if (state == NULL) {
		info->done = 1;
		return;
	}
	if (!ipmi_light_setting_get_on_time(st, 0, &on_time) &&
		!ipmi_light_setting_get_off_time(st, 0, &off_time)) {
		if (off_time > 10) {
			state->StateUnion.Oem.Body[0] = off_time / 10;
		} else {
			state->StateUnion.Oem.Body[0] = off_time ? 1 : 0;
		}
		if (on_time > 10) {
			state->StateUnion.Oem.Body[1] = on_time / 10;
		} else {
			state->StateUnion.Oem.Body[1] = on_time ? 1 : 0;
		}
	} else {
		err("couldn't get on/off times");
		info->err = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
	}
	if (ipmi_light_setting_get_color(st, 0, &color) == 0) {
		state->StateUnion.Oem.Body[2] = ohoi_atca_led_to_hpi_color(color);
		state->StateUnion.Oem.Body[3] = state->StateUnion.Oem.Body[2];
	} else {
		err("couldn't get color");
		info->err = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
	}
	state->StateUnion.Oem.Body[4] = 0;
	state->StateUnion.Oem.Body[5] = 0;
	state->StateUnion.Oem.MId = ATCAHPI_PICMG_MID;
	state->StateUnion.Oem.BodyLength = 6;
	state->Type = SAHPI_CTRL_TYPE_OEM;
	info->done = 1;
}

static void _get_atca_led(ipmi_control_t *control,
			  void *cb_data)
{
	struct ohoi_ctrl_info *info = cb_data;

	
	int rv;
	rv = ipmi_control_get_light(control, __get_atca_led, info);
	if (rv) {
		err("ipmi_control_get_light. rv = %d", rv);
		info->err = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
	}
}


SaErrorT orig_get_control_state(struct oh_handler_state *handler,
                                struct ohoi_control_info *c,
				SaHpiRdrT * rdr,
                                SaHpiCtrlModeT *mode,
                                SaHpiCtrlStateT *state)
{
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct ohoi_ctrl_info info;
	SaErrorT         rv;
	ipmi_control_id_t ctrl;
	SaHpiUint8T val, mask, idx, i;
	SaHpiCtrlStateT localstate;
	SaHpiCtrlModeT  localmode;

	ctrl = c->info.orig_ctrl_info.ctrl_id;

	if (state == NULL) {
		state = &localstate;
	}
	if (mode == NULL) {
		mode = &localmode;
	}
	if ((rdr->RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_OEM) &&
            (rdr->RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) &&
            (rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.MId == ATCAHPI_PICMG_MID)) {
	    	/* This is ATCA led */
		info.done = 0;
		info.err = SA_OK;
		info.rdr = rdr;
		info.handler = handler;
		info.mode = 0;
		info.state = state;
		rv = ipmi_control_pointer_cb(ctrl, _get_atca_led, &info);
		if (rv) {
			err("ipmi_control_pointer_cb. rv = %d", rv);
			return SA_ERR_HPI_INVALID_DATA;
		}
		rv = ohoi_loop(&info.done, handler->data);
		if (rv != SA_OK) {
			err("ohoi_loop. rv = %d", rv);
			return rv;
		}
		if (info.err != SA_OK) {
			err("info.err = %d", info.err);
			return info.err;
		}
		*mode = info.mode;
		c->mode = info.mode;
		return SA_OK;
        }

	*mode = c->mode;
	memset(state, 0, sizeof(*state));
	
        info.done  = 0;
        info.state = state;
	info.err = SA_OK;
        info.state->Type = SAHPI_CTRL_TYPE_OEM;
        
        rv = ipmi_control_pointer_cb(ctrl, _get_control_state, &info);
	if (rv) {
		err("Unable to retrieve control state");
		return SA_ERR_HPI_ERROR;
	}

	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}
	if (info.err != SA_OK) {
		return info.err;
	}
	val = info.state->StateUnion.Oem.Body[0];

	if ((rdr->RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_DIGITAL) &&
	    (rdr->RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) &&
	    (rdr->RdrTypeUnion.CtrlRec.Oem >= OEM_ALARM_BASE)) {
		oem_alarm_state = val;
		/* This is a front panel alarm LED. */
		state->Type = SAHPI_CTRL_TYPE_DIGITAL;	
		mask = 0x01;
		idx = rdr->RdrTypeUnion.CtrlRec.Oem - OEM_ALARM_BASE;
		/* bits 0 - 3 = Pwr, Crit, Maj, Min */
		for (i = 0; i < idx; i++) mask = mask << 1;
		if ((val & mask) == 0) 
		  state->StateUnion.Digital = SAHPI_CTRL_STATE_ON;
		else 
		  state->StateUnion.Digital = SAHPI_CTRL_STATE_OFF;
	} 
	return SA_OK;
}

SaErrorT ohoi_get_control_state(void *hnd, SaHpiResourceIdT id,
                                SaHpiCtrlNumT num,
                                SaHpiCtrlModeT *mode,
                                SaHpiCtrlStateT *state)
{
	struct oh_handler_state	*handler = (struct oh_handler_state *)hnd;
	SaErrorT		rv;
	struct ohoi_control_info *ctrl_info;
	SaHpiRdrT		*rdr;

	rdr = oh_get_rdr_by_type(handler->rptcache, id, SAHPI_CTRL_RDR, num);
	if (!rdr) return SA_ERR_HPI_INVALID_RESOURCE;
        rv = ohoi_get_rdr_data(hnd, id, SAHPI_CTRL_RDR, num, (void *)&ctrl_info);
        if (rv!=SA_OK) return rv;
	
	if (ctrl_info->ohoii.get_control_state == NULL) {
		return SA_ERR_HPI_INVALID_CMD;
	}
	
	return ctrl_info->ohoii.get_control_state(handler, ctrl_info, rdr, mode, state);	
}

static void __set_control_state(ipmi_control_t *control,
                                int            err,
                                void           *cb_data)
{
	struct ohoi_ctrl_info *info = cb_data;

	if (err) {
		err("__set_control_state: err = %d", err);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
	}
	info->done = 1;
}

static void _set_control_state(ipmi_control_t *control,
                                void           *cb_data)
{
        struct ohoi_ctrl_info *info = cb_data;
        
        if (info->state->StateUnion.Oem.BodyLength 
                        != ipmi_control_get_num_vals(control)) {
                err("control number is not equal to supplied data");
                info->done = -1;
		info->err = SA_ERR_HPI_INVALID_PARAMS;
                return;
        }
                        
	if (ipmi_control_light_set_with_setting(control)) {
		ipmi_light_setting_t *setting;

		setting = ipmi_alloc_light_settings(1);
		ipmi_light_setting_set_local_control(setting, 0, 1);
		ipmi_light_setting_set_color(setting, 0,
			info->state->StateUnion.Oem.Body[0]);
		ipmi_control_set_light(control, setting,
			__set_control_state, cb_data);
		ipmi_free_light_settings(setting);
	} else {
        	ipmi_control_set_val(control, 
	/* compile error */
//                             (int *)&info->state->StateUnion.Oem.Body[0],
                             (int *)(void *)&info->state->StateUnion.Oem.Body[0],
                             __set_control_state, info);
	}
}

static SaErrorT set_front_panrl_alarm_led(void *hnd,
                                	  struct ohoi_control_info *c,
					  SaHpiRdrT * rdr,
					  ipmi_control_id_t ctrl,
					  SaHpiUint8T idx,
					  SaHpiCtrlModeT mode,
					  SaHpiCtrlStateT *state)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct ohoi_ctrl_info info;
	SaErrorT         rv;
	SaHpiUint8T val, mask;
	SaHpiCtrlStateT my_state;


	
	/* just get the current control states */
	rv = orig_get_control_state(hnd, c, rdr, NULL, &my_state);
	if (rv != SA_OK) {
		return SA_ERR_HPI_NOT_PRESENT;
	}
	if (my_state.Type != state->Type) {
		return SA_ERR_HPI_INVALID_DATA;
	}
	val = oem_alarm_state;
	val |= 0xf0;  /* h.o. nibble always write 1 */
	state->Type = SAHPI_CTRL_TYPE_OEM;
	state->StateUnion.Oem.BodyLength = 1;
	state->StateUnion.Oem.Body[0] = val;
	mask = 0x01;
	/* bits 0 - 3 = Pwr, Crit, Maj, Min */
	mask = 1 << idx;
//	for (i = 0; i < idx; i++) mask = mask << 1;
        info.done  = 0;
        info.state = state;
	info.err = SA_OK;
	switch (state->StateUnion.Digital) {
	case SAHPI_CTRL_STATE_ON :
		if (!(val & mask)) { /* already turned on */
			return SA_OK;
		} 
		state->StateUnion.Oem.Body[0] = val & ~mask;
		break;
	case SAHPI_CTRL_STATE_OFF :
		if (val & mask) { /* already turned off */
			return SA_OK;
		} 
		state->StateUnion.Oem.Body[0] = val |mask;
		break;

	case SAHPI_CTRL_STATE_PULSE_ON :
		if (!(val & mask)) { /* already turned on */
			return SA_ERR_HPI_INVALID_REQUEST;
		}
		/* turn it on */
		state->StateUnion.Oem.Body[0] = val & ~mask;
        	rv = ipmi_control_pointer_cb(ctrl, _set_control_state, &info);
		if (rv) {
			err("Unable to set control state");
			return SA_ERR_HPI_ERROR;
		}
        	rv = ohoi_loop(&info.done, ipmi_handler);
		if (info.err != SA_OK) {
			err("Unable to set control state");
			return info.err;
		}
		if (rv != SA_OK) {
			err("Unable to set control state");
			return rv;
		}
		/* then turn it off. IPMI is slow, it provides us delay */
		state->StateUnion.Oem.Body[0] = val | mask;
		break;
	case SAHPI_CTRL_STATE_PULSE_OFF :
		if (val & mask) { /* already turned off */
			return SA_ERR_HPI_INVALID_REQUEST;
		}
		/* turn it off */
		state->StateUnion.Oem.Body[0] = val | mask;
        	rv = ipmi_control_pointer_cb(ctrl, _set_control_state, &info);
		if (rv) {
			err("Unable to set control state");
			return SA_ERR_HPI_ERROR;
		}
        	rv = ohoi_loop(&info.done, ipmi_handler);
		if (info.err != SA_OK) {
			err("Unable to set control state");
			return info.err;
		}
		if (rv != SA_OK) {
			err("Unable to set control state");
			return rv;
		}
		/* then turn it on. IPMI is slow, it provides us delay */
		state->StateUnion.Oem.Body[0] = val | mask;
		break;
	default :
		return SA_ERR_HPI_INVALID_PARAMS;
	}
	
        rv = ipmi_control_pointer_cb(ctrl, _set_control_state, &info);
	if (rv) {
		err("Unable to set control state");
		return SA_ERR_HPI_ERROR;
	}
        rv = ohoi_loop(&info.done, ipmi_handler);
	if (info.err != SA_OK) {
		err("Unable to set control state");
		return info.err;
	}
	if (rv != SA_OK) {
		err("Unable to set control state");
		return rv;
	}
	
	return SA_OK;
}





	/*
	 *      ATCA LED controls
	 */


static void __set_atca_led(ipmi_control_t *control,
                          int err,
			  ipmi_light_setting_t *st,
			  void *cb_data)
{
	struct ohoi_ctrl_info *info = cb_data;
	SaHpiUint8T *body;
	int lc = 0;
	int color = 0;
	int rv;
	
	ipmi_light_setting_in_local_control(st, 0, &lc);
	if (info->mode == SAHPI_CTRL_MODE_AUTO) {
		/* set MODE only */
		ipmi_light_setting_set_local_control(st, 0, 1);
	} else {
		body = &info->state->StateUnion.Oem.Body[0];
		color = ohoi_atca_led_to_ipmi_color(body[2]);
	
		ipmi_light_setting_set_local_control(st, 0, 0);
	
		rv = ipmi_light_setting_set_color(st, 0, color);
		if (rv) {
			err("ipmi_light_setting_set_color. rv = %d", rv);
		}
		rv = ipmi_light_setting_set_off_time(st, 0, body[0] * 10);
		if (rv) {
			err("ipmi_light_setting_set_off_time. rv = %d", rv);
		}
		rv = ipmi_light_setting_set_on_time(st, 0, body[1] * 10);
		if (rv) {
			err("ipmi_light_setting_set_on_time. rv = %d", rv);
		}
	}
	
	rv = ipmi_control_set_light(control, st,
			__set_control_state, info);
	if (rv) {
		err("ipmi_control_set_light = %d", rv);
		info->err = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
		return;
	}
}





static void _set_atca_led(ipmi_control_t *control,
			  void *cb_data)
{
	struct ohoi_ctrl_info *info = cb_data;

	
	int rv;
	rv = ipmi_control_get_light(control, __set_atca_led, info);
	if (rv) {
		err("ipmi_control_get_light. rv = %d", rv);
		info->err = SA_ERR_HPI_INVALID_DATA;
		info->done = 1;
	}
}

/**
 * is_supported_color
 * @hpi_color: HPI Color to test if supported
 * @rdr: RDR object containing control record for LED.
 *
 * Check if hpi_color is supported by the LED associated with rdr.
 *
 * Return value: 0 if color is supported, 1 otherwise.
 **/
static int is_supported_color(AtcaHpiLedColorT hpi_color, 
			      SaHpiRdrT *rdr)
{
	AtcaHpiColorCapabilitiesT supported_colors = rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.ConfigData[0];

	switch(hpi_color)
	{
		case ATCAHPI_LED_COLOR_BLUE:
			return ((supported_colors & ATCAHPI_LED_BLUE) != 0);
		case ATCAHPI_LED_COLOR_RED:
			return ((supported_colors & ATCAHPI_LED_RED) != 0);
		case ATCAHPI_LED_COLOR_GREEN:
			return ((supported_colors & ATCAHPI_LED_GREEN) != 0);
		case ATCAHPI_LED_COLOR_AMBER:
			return ((supported_colors & ATCAHPI_LED_AMBER) != 0);
		case ATCAHPI_LED_COLOR_ORANGE:
			return ((supported_colors & ATCAHPI_LED_ORANGE) != 0);
		case ATCAHPI_LED_COLOR_WHITE:
			return ((supported_colors & ATCAHPI_LED_WHITE) != 0);
		case ATCAHPI_LED_COLOR_NO_CHANGE:
			return 1;
		case ATCAHPI_LED_COLOR_USE_DEFAULT:
			return 1;
		case ATCAHPI_LED_COLOR_RESERVED:
			return 0;
	}

	return 0;
}

SaErrorT orig_set_control_state(struct oh_handler_state *handler,
                                struct ohoi_control_info *c,
				SaHpiRdrT * rdr,
                                SaHpiCtrlModeT mode,
                                SaHpiCtrlStateT *state)
{
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct ohoi_ctrl_info info;
	SaErrorT         rv;
	ipmi_control_id_t ctrl;

	ctrl = c->info.orig_ctrl_info.ctrl_id;


	if ((rdr->RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_OEM) &&
            (rdr->RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) &&
            (rdr->RdrTypeUnion.CtrlRec.TypeUnion.Oem.MId == ATCAHPI_PICMG_MID)) {
	    	/* This is ATCA led */
		if (state != NULL) {
			if (state->StateUnion.Oem.MId != ATCAHPI_PICMG_MID) {
				err("state->StateUnion.Mid isn't ATCAHPI_PICMG_MID");
				return SA_ERR_HPI_INVALID_DATA;
			}
			if (state->StateUnion.Oem.BodyLength != 6) {
				err("state->StateUnion.Oem.BodyLength(%d) != 6",
					state->StateUnion.Oem.BodyLength);
				return SA_ERR_HPI_INVALID_DATA;
			}
			SaHpiUint8T *body = &state->StateUnion.Oem.Body[0];
			if ((body[2] == 0) || ((body[2] & (body[2] - 1)) != 0)) {
				/* exactly one color must be set */
				return SA_ERR_HPI_INVALID_DATA;
			}
			if (!is_supported_color(body[2], rdr)) {
				/* LED doesn't support this color */
				return SA_ERR_HPI_INVALID_DATA;
			}
		}
		info.done = 0;
		info.err = 0;
		info.rdr = rdr;
		info.handler = handler;
		info.mode = mode;
		info.state = state;
		rv = ipmi_control_pointer_cb(ctrl, _set_atca_led, &info);
		if (rv) {
			err("ipmi_control_pointer_cb. rv = %d", rv);
			return SA_ERR_HPI_INVALID_DATA;
		}
		rv = ohoi_loop(&info.done, handler->data);
		if (rv != SA_OK) {
			err("ohoi_loop. rv = %d", rv);
			return rv;
		}
		if (info.err != SA_OK) {
			err("info.err = %d", info.err);
			return info.err;
		}
		c->mode = mode;
		return SA_OK;
        }

	if (mode == SAHPI_CTRL_MODE_AUTO) {
		c->mode = mode;
		return SA_OK;
	}

	if ((rdr->RdrTypeUnion.CtrlRec.Type == SAHPI_CTRL_TYPE_DIGITAL) &&
            (rdr->RdrTypeUnion.CtrlRec.OutputType == SAHPI_CTRL_LED) &&
            (rdr->RdrTypeUnion.CtrlRec.Oem >= OEM_ALARM_BASE)) {
                /* This is a front panel alarm LED. */
		rv = set_front_panrl_alarm_led(handler, c, rdr, ctrl,
			rdr->RdrTypeUnion.CtrlRec.Oem - OEM_ALARM_BASE,
			mode, state);
		if (rv == SA_OK) {
			c->mode = mode;
		}
		return rv;
        }

	    
        info.done  = 0;
        info.state = state;
	info.err = SA_OK;
        if (info.state->Type != SAHPI_CTRL_TYPE_OEM) {
                err("IPMI only support OEM control");
                return SA_ERR_HPI_INVALID_CMD;
        }
       
        rv = ipmi_control_pointer_cb(ctrl, _set_control_state, &info);
	if (rv) {
		err("Unable to set control state");
		return SA_ERR_HPI_ERROR;
	}

        rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}
	if (info.err != SA_OK) {
		return info.err;
	}
	c->mode = mode;
	return SA_OK;
}	 


	/* interface function */

SaErrorT ohoi_set_control_state(void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiCtrlNumT num,
                                SaHpiCtrlModeT mode,
                                SaHpiCtrlStateT *state)
{
	struct oh_handler_state		*handler =
						(struct oh_handler_state *)hnd;
	struct ohoi_control_info	*ctrl_info;
	SaErrorT			rv;
	SaHpiRdrT			*rdr;

	rdr = oh_get_rdr_by_type(handler->rptcache, id, SAHPI_CTRL_RDR, num);
	if (!rdr) return SA_ERR_HPI_INVALID_RESOURCE;
        rv = ohoi_get_rdr_data(hnd, id, SAHPI_CTRL_RDR, num, (void *)&ctrl_info);
        if (rv != SA_OK) return rv;
		
	if (rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly &&
		rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode != mode) {
		err("Attempt to change mode of RO sensor mode");
		return SA_ERR_HPI_READ_ONLY;
	}

	if (ctrl_info->ohoii.set_control_state == NULL) {
		return SA_ERR_HPI_UNSUPPORTED_API;
	}
	
	rv = ctrl_info->ohoii.set_control_state(handler, ctrl_info, rdr, mode, state);
	
	return rv;

}

static void reset_resource_done (ipmi_control_t *ipmi_control,
				 int err,
				 void *cb_data)
{
	struct ohoi_reset_info *info = cb_data;
	info->done = 1;
	if (err) {
		err("reset_resource_done: err = %d", err);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
	}
}

static void set_resource_reset_state(ipmi_control_t *control,
                            void           *cb_data)
{
        struct ohoi_reset_info *info = cb_data;
	int val = 1;
	int rv;

        /* Just cold reset the entity*/
        rv = ipmi_control_set_val(control, &val, reset_resource_done, cb_data);
	if (rv) {
		err("ipmi_control_set_val returned err = %d", rv);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
	}
}


static void reset_mc_done (ipmi_mc_t *mc,
			int err,
			void *cb_data)
{
	struct ohoi_reset_info *info = cb_data;
	info->done = 1;
	if (err) {
		err("reset_mc_done err = %d", err);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
	}
}

static void set_mc_reset_state(ipmi_mc_t *mc,
                            void           *cb_data)
{
        struct ohoi_reset_info *info = cb_data;
	int rv;
	int act;

	if (*info->state == SAHPI_COLD_RESET) {
		act = IPMI_MC_RESET_COLD;
	} else if (*info->state == SAHPI_WARM_RESET) {
		act = IPMI_MC_RESET_WARM;
	} else {
		info->err = SA_ERR_HPI_INVALID_CMD;
		info->done = 1;
		return;
	}		
        rv = ipmi_mc_reset(mc, act, reset_mc_done, cb_data);
	if (rv) {
		err("ipmi_mc_reset returned err = %d", rv);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
	}
}



SaErrorT ohoi_set_reset_state(void *hnd, SaHpiResourceIdT id, 
		              SaHpiResetActionT act)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
	struct ohoi_reset_info info;	
        struct ohoi_resource_info *ohoi_res_info;
        
	int rv;
        
	info.done = 0;
	info.err = 0;
	if ((act == SAHPI_COLD_RESET) || (act == SAHPI_WARM_RESET)) {
	      	dbg("ResetAction requested: %d", act);
	      	info.state = &act;
	} else {
	      	err("Currently we only support cold and warm reset");
		return SA_ERR_HPI_INVALID_CMD;
	}

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_ENTITY)) {
                rv = ipmi_control_pointer_cb(ohoi_res_info->reset_ctrl, 
                                     set_resource_reset_state, &info);
	} else {
		rv = ipmi_mc_pointer_cb(ohoi_res_info->u.entity.mc_id, 
			set_mc_reset_state, &info);
	}

        if (rv) {
                err("Not support reset in the entity or mc");
                return SA_ERR_HPI_CAPABILITY;
        }

	/* wait until reset_done is called to exit this function */
	rv = ohoi_loop(&info.done, ipmi_handler);
	if ((rv == SA_OK) && (info.err == 0)) {
		return SA_OK;
	} else if (info.err) {
		return info.err;
	} else {
		return rv;
	}
}

static void power_done (ipmi_control_t *ipmi_control,
                        int err,
                        void *cb_data)
{
	
	struct ohoi_power_info *power_info = cb_data;

	power_info->done = 1;
	if (err) {
		err("power_done: err = %d", err);
		power_info->err = SA_ERR_HPI_INTERNAL_ERROR;
	}
}

static void set_power_state_on(ipmi_control_t *control,
                            void           *cb_data)
{
	struct ohoi_power_info *power_info = cb_data;
	int rv;

        rv = ipmi_control_set_val(control, (int *)power_info->state, power_done, cb_data);

	if (rv) {
		err("Failed to set control val (power on). rv = %d", rv);
		OHOI_MAP_ERROR(power_info->err, rv);
		power_info->done = 1;
	} else
	  	power_info->err = SA_OK;
}

static void set_power_state_off(ipmi_control_t *control,
                            void           *cb_data)
{
	struct ohoi_power_info *power_info = cb_data;
	int rv;

        rv = ipmi_control_set_val(control, (int *)power_info->state,
				  power_done, cb_data);
	if (rv) {
	  	err("Failed to set control val (power off). rv = %d", rv);
		OHOI_MAP_ERROR(power_info->err, rv);
		power_info->done = 1;
	} else
	  	power_info->err = SA_OK;
}

SaErrorT ohoi_set_power_state(void *hnd, SaHpiResourceIdT id, 
		              SaHpiPowerStateT state)
{
        struct ohoi_resource_info *ohoi_res_info;

	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
	struct ohoi_power_info power_info;

        int rv;
	power_info.done = 0;
	power_info.state = &state;

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_ENTITY)) {
                err("Not support power in MC");
                return SA_ERR_HPI_INVALID_CMD;
        }
	
	switch (state) {
		case SAHPI_POWER_ON:
			rv = ipmi_control_pointer_cb(ohoi_res_info->power_ctrl, 
						    set_power_state_on, &power_info);
			if (rv) {
				err("set_power_state_on failed");
				return SA_ERR_HPI_INTERNAL_ERROR;
			}
			break;
		case SAHPI_POWER_OFF:
			rv = ipmi_control_pointer_cb(ohoi_res_info->power_ctrl, 
                                                    set_power_state_off, &power_info);
	                if (rv) {
        	            	err("set_power_state_off failed");
                	    	return SA_ERR_HPI_INTERNAL_ERROR;
                	}
			break;
		case SAHPI_POWER_CYCLE:
			dbg("CYCLE power");
			SaHpiPowerStateT	cy_state = 0;
			cy_state = SAHPI_POWER_OFF;
			power_info.state = &cy_state;
			rv = ipmi_control_pointer_cb(ohoi_res_info->power_ctrl, 
						    set_power_state_off, &power_info);
			if (rv) {
				err("set_power_state_off failed");
				return SA_ERR_HPI_INTERNAL_ERROR;
			}
			rv = ohoi_loop(&power_info.done, ipmi_handler);
			if (rv != SA_OK) {
				err("ohopi_loop = 0x%x", rv);
				return rv;
			}
			dbg("CYCLE Stage 1: OK");

			if ((power_info.done) && (power_info.err == SA_OK)) {
			      dbg("CYCLE: done = %d , err = %d",
				  				power_info.done,
								power_info.err);
			      	cy_state = SAHPI_POWER_ON;
				power_info.state = &cy_state;
			      	power_info.done = 0;
			      	rv = ipmi_control_pointer_cb(ohoi_res_info->power_ctrl, 
							    set_power_state_on, &power_info);
				if (rv) {
				      	err("set_power_state_on failed");
					return SA_ERR_HPI_INTERNAL_ERROR;
				}
			}
			break;
		default:
			err("Invalid power state requested");
			return SA_ERR_HPI_INVALID_PARAMS;
	}

        rv = ohoi_loop(&power_info.done, ipmi_handler);
 	if (rv != SA_OK) {
		return rv;
	}       
        return power_info.err;
}

static void get_power_control_val (ipmi_control_t *control,
			     int err,
			     int *val,
			     void *cb_data)
{
	struct ohoi_power_info *info = cb_data;

	if (err || !val) {
		err("get_power_control_val: err = %d; val = %p", err, val);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
	if (*val == 0) {
		err("Power Control Value: %d", *val);
		*info->state = SAHPI_POWER_OFF;
		info->err = SA_OK;
	} else if (*val == 1) {
		err("Power Control Value: %d", *val);
		*info->state = SAHPI_POWER_ON;
		info->err = SA_OK;
	} else {
		err("invalid power state");
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
	}
	info->done = 1;

}

static void get_power_state (ipmi_control_t *ipmi_control,
			     void *cb_data)
{
	struct ohoi_power_info *power_state = cb_data;
	int rv;

	rv = ipmi_control_get_val(ipmi_control, get_power_control_val, cb_data);

	if(rv) {
		err("[power]control_get_val failed. rv = %d", rv);
		power_state->err = SA_ERR_HPI_INTERNAL_ERROR;
		power_state->done = 1;
	}
}

SaErrorT ohoi_get_power_state (void *hnd,
                               SaHpiResourceIdT	id,
                               SaHpiPowerStateT *state)
{
        struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct ohoi_resource_info *ohoi_res_info;
	struct ohoi_power_info power_state;

	int rv;

	power_state.done = 0;
	power_state.err = SA_OK;
	power_state.state = state;
	
        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_ENTITY)) {
                err("MC does not support power!");
                return SA_ERR_HPI_CAPABILITY;
        }

	rv = ipmi_control_pointer_cb(ohoi_res_info->power_ctrl,
					get_power_state, &power_state);
	if (rv) {
		err("get_power_state failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	dbg("waiting for OIPMI to return");
	rv = ohoi_loop(&power_state.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}	     
        return power_state.err;
}

static void get_reset_control_val (ipmi_control_t *control,
			     int err,
			     int *val,
			     void *cb_data)
{
	struct ohoi_reset_info *info = cb_data;

	if (err || !val) {
		err("get_reset_control_val: err = %d; val = %p", err, val);
		info->err = SA_ERR_HPI_INTERNAL_ERROR;
		info->done = 1;
		return;
	}
	if (*val == 0) {
		err("Reset Control Value: %d", *val);
		*info->state = SAHPI_RESET_DEASSERT;
		info->err = SA_OK;
	} else if (*val == 1) {
		err("Power Control Value: %d", *val);
		*info->state = SAHPI_RESET_ASSERT;
		info->err = SA_OK;
	} else {
	  	/* Note: IPMI platfroms don't hold reset state
		 * so we'll always return SAHPI_RESET_DEASSER
		 * this code is here just in case ;-)
		 */
		err("System does not support holding ResetState");
		*info->state = SAHPI_RESET_DEASSERT;
		info->err = SA_OK;
	}
	info->done = 1;
}

static
void get_reset_state(ipmi_control_t *control,
		     void *cb_data)
{
	struct ohoi_reset_info *reset_info = cb_data;

	int rv;

	rv = ipmi_control_get_val(control, get_reset_control_val, cb_data);
	if (rv) {
		//err("[reset] control_get_val failed. IPMI error = %i", rv);
		err("This IPMI system has a pulse reset, state is always DEASSERT");
		/* OpenIPMI will return an error for this call
		   since pulse resets do not support get_state
		   we will return UNSUPPORTED_API
		 */
		*reset_info->state = SAHPI_RESET_DEASSERT;
		reset_info->err = SA_OK;
		reset_info->done = 1;
	}
}
		    
SaErrorT ohoi_get_reset_state(void *hnd,
			      SaHpiResourceIdT id,
			      SaHpiResetActionT *act)
{
	struct oh_handler_state *handler = (struct oh_handler_state *)hnd;
	struct ohoi_handler *ipmi_handler = (struct ohoi_handler *)handler->data;
        struct ohoi_resource_info *ohoi_res_info;
        
	struct ohoi_reset_info reset_state;

	int rv;

	reset_state.done = 0;
	reset_state.err = 0;
	reset_state.state = act;

        ohoi_res_info = oh_get_resource_data(handler->rptcache, id);
        if (!(ohoi_res_info->type & OHOI_RESOURCE_ENTITY)) {
                err("Not support power in MC");
                return SA_ERR_HPI_CAPABILITY;
        }

	rv = ipmi_control_pointer_cb(ohoi_res_info->reset_ctrl,
				     get_reset_state, &reset_state);
	if(rv) {
		err("[reset_state] control pointer callback failed. rv = %d", rv);
		return SA_ERR_HPI_INVALID_CMD;
	}

	rv = ohoi_loop(&reset_state.done, ipmi_handler);
	if (rv != SA_OK) {
		return rv;
	}
	return reset_state.err;
}


void * oh_get_control_state (void *, SaHpiResourceIdT, SaHpiCtrlNumT,
                             SaHpiCtrlModeT *, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("ohoi_get_control_state")));

void * oh_set_control_state (void *, SaHpiResourceIdT,SaHpiCtrlNumT,
                             SaHpiCtrlModeT, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("ohoi_set_control_state")));

void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
                __attribute__ ((weak, alias("ohoi_get_power_state")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
                __attribute__ ((weak, alias("ohoi_set_power_state")));

void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
                __attribute__ ((weak, alias("ohoi_get_reset_state")));

void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
                __attribute__ ((weak, alias("ohoi_set_reset_state")));

