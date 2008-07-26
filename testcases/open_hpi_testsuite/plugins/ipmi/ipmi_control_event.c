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
 *     Tariq Shureih <tariq.shureih@intel.com>
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 */

#include "ipmi.h"
#include <oh_utils.h>
#include <string.h>


//static void add_control_event_data_format(ipmi_control_t	*control,
					   //SaHpiCtrlRecT	*rec)
//{


//}

static void trace_ipmi_control(char *str,
                               ipmi_control_t *control,
			       SaHpiRptEntryT *rpt)
{
	if (!getenv("OHOI_TRACE_CONTROL") && !IHOI_TRACE_ALL) {
		return;
	}
	char strtype[24];
	char name[24];

	ipmi_control_get_id(control, name, 24);	
	switch (ipmi_control_get_type(control)) {
	case IPMI_CONTROL_ONE_SHOT_RESET:
		strncpy(strtype,"reset", 24);
		break;
	case IPMI_CONTROL_POWER:
		strncpy(strtype,"power", 24);
		break;
	case IPMI_CONTROL_ALARM:
		strncpy(strtype,"alarm", 24);
		break;
	case IPMI_CONTROL_IDENTIFIER:
		strncpy(strtype,"address", 24);
		break;
	case IPMI_CONTROL_LIGHT:
		strncpy(strtype,"LED", 24);
		break;
	default:
		snprintf(strtype, 24, "unknown(%d)",
			ipmi_control_get_type(control));
		break;
	}
		
	fprintf(stderr, "   *** CONTROL %s. type %s (%s). RPT %d(%s)\n",
		str, strtype, name, rpt->ResourceId, rpt->ResourceTag.Data);
}

SaHpiUint8T ohoi_atca_led_to_hpi_color(int ipmi_color)
{
	switch (ipmi_color) {
	case IPMI_CONTROL_COLOR_WHITE:
		return ATCAHPI_LED_COLOR_WHITE;
	case IPMI_CONTROL_COLOR_RED:
		return ATCAHPI_LED_COLOR_RED;
	case IPMI_CONTROL_COLOR_GREEN:
		return ATCAHPI_LED_COLOR_GREEN;
	case IPMI_CONTROL_COLOR_BLUE:
		return ATCAHPI_LED_COLOR_BLUE;
	case IPMI_CONTROL_COLOR_YELLOW:
		return ATCAHPI_LED_COLOR_AMBER;
	case IPMI_CONTROL_COLOR_ORANGE:
		return ATCAHPI_LED_COLOR_ORANGE;
	default:
		err("strange color %d, return WHITE", ipmi_color);
		return ATCAHPI_LED_COLOR_WHITE;
	}
}

int ohoi_atca_led_to_ipmi_color(SaHpiUint8T c)
{
	switch (c) {
	case ATCAHPI_LED_COLOR_WHITE:
		return IPMI_CONTROL_COLOR_WHITE;
	case ATCAHPI_LED_COLOR_ORANGE:
		return IPMI_CONTROL_COLOR_ORANGE;
	case ATCAHPI_LED_COLOR_AMBER:
		return IPMI_CONTROL_COLOR_YELLOW;
	case ATCAHPI_LED_COLOR_GREEN:
		return IPMI_CONTROL_COLOR_GREEN;
	case ATCAHPI_LED_COLOR_RED:
		return IPMI_CONTROL_COLOR_RED;
	case ATCAHPI_LED_COLOR_BLUE:
		return IPMI_CONTROL_COLOR_BLUE;
	default:
		return 0;
	}
}

static SaHpiCtrlOutputTypeT _control_type_from_ipmi_to_hpi(
				ipmi_control_t *control)
{
        switch (ipmi_control_get_type(control)) {
		case IPMI_CONTROL_ALARM:
                        return SAHPI_CTRL_FRONT_PANEL_LOCKOUT;
                        
		case IPMI_CONTROL_DISPLAY:
                        return SAHPI_CTRL_LCD_DISPLAY;

                case IPMI_CONTROL_LIGHT:
                        return SAHPI_CTRL_LED;
                        
		case IPMI_CONTROL_FAN_SPEED:
                        return SAHPI_CTRL_FAN_SPEED;

                case IPMI_CONTROL_IDENTIFIER:
		case IPMI_CONTROL_RELAY:
                default:
                        return SAHPI_CTRL_OEM;                       
        } 
}

static void set_idstring(ipmi_control_t *control,
			 SaHpiRdrT		*rdr)
{
	char		name[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	SaHpiTextTypeT	data_type;
	int		name_len;
	
	memset(name, '\0' ,SAHPI_MAX_TEXT_BUFFER_LENGTH);	
	ipmi_control_get_id(control, name, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	name_len = ipmi_control_get_id_length(control);
	if (name_len >= SAHPI_MAX_TEXT_BUFFER_LENGTH)
		name_len = SAHPI_MAX_TEXT_BUFFER_LENGTH - 1;
	data_type = convert_to_hpi_data_type(ipmi_control_get_id_type(control));
	rdr->IdString.DataType = data_type;
	rdr->IdString.Language = SAHPI_LANG_ENGLISH;
	rdr->IdString.DataLength = name_len;

	memset(rdr->IdString.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
	memcpy(rdr->IdString.Data, name, name_len);
}


static int add_control_event(ipmi_entity_t	*ent,
			     ipmi_control_t	*control,
			     struct oh_handler_state *handler,
			     SaHpiEntityPathT	parent_ep,
			     SaHpiResourceIdT	rid)
{
        struct ohoi_resource_info *info;
	struct ohoi_control_info  *ctrl_info;
	SaHpiRdrT		rdr;

        ctrl_info = malloc(sizeof(struct ohoi_control_info));
        if (!ctrl_info) {
                err("Out of memory");
                return 1;
        }
	memset(&rdr, 0, sizeof(rdr));
	ctrl_info->type = OHOI_CTRL_ORIGINAL;
        ctrl_info->info.orig_ctrl_info.ctrl_id =
				ipmi_control_convert_to_id(control);
	ctrl_info->mode = SAHPI_CTRL_MODE_AUTO;
	ctrl_info->ohoii.get_control_state = orig_get_control_state;
	ctrl_info->ohoii.set_control_state = orig_set_control_state;

	rdr.RecordId = 0;
	rdr.Entity = parent_ep;
	rdr.RdrTypeUnion.CtrlRec.OutputType = _control_type_from_ipmi_to_hpi(control);
	rdr.RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_OEM;

	set_idstring(control, &rdr);

	info = oh_get_resource_data(handler->rptcache, rid);
        if (!info) {
		free(ctrl_info);
                err("No info in resource(%d)\n", rid);
                return 1;
        }
        rdr.RdrTypeUnion.CtrlRec.Num = ++info->ctrl_count;

        rid = oh_uid_lookup(&rdr.Entity);
        
	if (oh_add_rdr(handler->rptcache, rid,
			&rdr, ctrl_info, 1) != SA_OK) {
		err("couldn't add control rdr");
		free(ctrl_info);
		return 1;
	}
	return 0;
}



static void set_possible_colors(ipmi_control_t	*control,
				unsigned char *clrs)
{
	unsigned char c = 0;
	int num = 0;

	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_WHITE)) {
		c |= ATCAHPI_LED_WHITE;
		num++;
	}
	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_RED)) {
		c |= ATCAHPI_LED_RED;
		num++;
	}
	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_GREEN)) {
		c |= ATCAHPI_LED_GREEN;
		num++;
	}
	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_BLUE)) {
		c |= ATCAHPI_LED_BLUE;
		num++;
	}
	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_YELLOW)) {
		c |= ATCAHPI_LED_AMBER;
		num++;
	}
	if (ipmi_control_light_is_color_supported(control,
				IPMI_CONTROL_COLOR_ORANGE)) {
		c |= ATCAHPI_LED_ORANGE;
		num++;
	}
	*clrs = c;
}


typedef struct {
	SaHpiCtrlRecOemT	*oem;
	SaHpiCtrlDefaultModeT	*dm;
	int err;
	int done;
} ohoi_led_info_t;


static void set_led_oem_cb(ipmi_control_t	*control,
			   int			err,
			   ipmi_light_setting_t	*st,
			   void			*cb_data)
{
	ohoi_led_info_t *info = cb_data;
	SaHpiCtrlRecOemT	*oem = info->oem;
	SaHpiCtrlDefaultModeT	*dm = info->dm;
	int lc = 0;
	int color;
	int on_time, off_time;
	
	if (err) {
		info->err = err;
		info->done = 1;
		err("led_default_mode_settings_cb = %d", err);
		return;
	}	
	oem->Default.MId = ATCAHPI_PICMG_MID;
	oem->MId = ATCAHPI_PICMG_MID;
	
	// set possible colors
	set_possible_colors(control, &oem->ConfigData[0]);
	
	//set default auto color
	if (ipmi_light_setting_get_color(st, 0, &color) == 0) {
		oem->ConfigData[1] = ohoi_atca_led_to_hpi_color(color);
	} else {
		oem->ConfigData[1] = 0;
	}
	//set default manual color
	if (ipmi_light_setting_get_color(st, 0, &color) == 0) {
		oem->ConfigData[2] = ohoi_atca_led_to_hpi_color(color);
	} else {
		oem->ConfigData[2] = 0;
	}
	
	if (!ipmi_light_setting_get_on_time(st, 0, &on_time) &&
		!ipmi_light_setting_get_off_time(st, 0, &off_time)) {
		oem->ConfigData[3] = ATCAHPI_LED_BR_SUPPORTED;
		if (off_time > 10) {
			oem->Default.Body[0] = off_time / 10;
		} else {
			oem->Default.Body[0] = off_time ? 1 : 0;
		}
		if (on_time > 10) {
			oem->Default.Body[1] = on_time / 10;
		} else {
			oem->Default.Body[1] = on_time ? 1 : 0;
		}
	} else {
		oem->ConfigData[3] = ATCAHPI_LED_BR_NOT_SUPPORTED;
	}
	oem->Default.Body[2] = oem->ConfigData[1];
	oem->Default.Body[3] = oem->ConfigData[2];
	oem->Default.Body[4] = SAHPI_FALSE;
	oem->Default.BodyLength = 6;
	
	if (!ipmi_control_light_has_local_control(control)) {
		dm->Mode = SAHPI_CTRL_MODE_AUTO;
		dm->ReadOnly = SAHPI_TRUE;
	} else {
		ipmi_light_setting_in_local_control(st, 0, &lc);
		dm->Mode = lc ? SAHPI_CTRL_MODE_AUTO : SAHPI_CTRL_MODE_MANUAL;
		dm->ReadOnly = SAHPI_FALSE;
	}
	info->done = 1;
}





static int add_led_control_event(ipmi_entity_t	*ent,
			     ipmi_control_t	*control,
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt)
{
	SaHpiEntityPathT parent_ep = rpt->ResourceEntity;
	SaHpiResourceIdT	rid = rpt->ResourceId;
        struct ohoi_resource_info *info;
	struct ohoi_control_info  *ctrl_info;
	SaHpiRdrT		rdr;
	int rv;

        ctrl_info = malloc(sizeof(struct ohoi_control_info));
        if (!ctrl_info) {
                err("Out of memory");
                return 1;
        }
	memset(&rdr, 0, sizeof(rdr));
	ctrl_info->type = OHOI_CTRL_ORIGINAL;
        ctrl_info->info.orig_ctrl_info.ctrl_id =
				ipmi_control_convert_to_id(control);
	ctrl_info->ohoii.get_control_state = orig_get_control_state;
	ctrl_info->ohoii.set_control_state = orig_set_control_state;
        
	rdr.RecordId = 0;
	rdr.RdrType = SAHPI_CTRL_RDR;
	rdr.Entity = parent_ep;
	rdr.RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_LED;
	rdr.RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_OEM;
	rdr.RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
	rdr.RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = SAHPI_TRUE;
	set_idstring(control, &rdr);
	if (ipmi_control_light_set_with_setting(control)) {
		ohoi_led_info_t info;
		info.done = 0;
		info.err = 0;
		info.oem = &rdr.RdrTypeUnion.CtrlRec.TypeUnion.Oem;
		info.dm = &rdr.RdrTypeUnion.CtrlRec.DefaultMode;
		rv = ipmi_control_get_light(control,
			set_led_oem_cb, &info);
		if (rv) {
			err("ipmi_control_get_light = 0x%x", rv);
		} else {
			ohoi_loop(&info.done, handler->data);
		}

	} else {
		err("ipmi_control_light_set_with_setting == 0");
	} 
	ctrl_info->mode = rdr.RdrTypeUnion.CtrlRec.DefaultMode.Mode;
	rdr.RdrTypeUnion.CtrlRec.Oem = ATCAHPI_PICMG_CT_ATCA_LED;
	


	info = oh_get_resource_data(handler->rptcache, rid);
        if (!info) {
		free(ctrl_info);
                err("No info in resource(%d)\n", rid);
                return 1;
        }
	if (strcasecmp((char *)rdr.IdString.Data, "blue led") == 0) {
		rdr.RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_BLUE_LED;
	} else if (strcasecmp((char *)rdr.IdString.Data, "led 1") == 0) {
		rdr.RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_LED1;
	} else if (strcasecmp((char *)rdr.IdString.Data, "led 2") == 0) {
		rdr.RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_LED2;
	} else if (strcasecmp((char *)rdr.IdString.Data, "led 3") == 0) {
		rdr.RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_LED3;
	} else if (strcasestr((char *)rdr.IdString.Data, "application-specific led ") ==
		   (char *)rdr.IdString.Data) {
		char *appnum = 
			(char *)&rdr.IdString.Data[strlen("application-specific led ")];
		if (strlen(appnum) <= 3) {
			rdr.RdrTypeUnion.CtrlRec.Num = ATCAHPI_CTRL_NUM_APP_LED +
				strtoul(appnum, NULL, 0);
		} else {
			err("Invalid data in LED Control\n");
			return 1;
		}
	} else {
		err("Invalid data in LED Control\n");
		return 1;
	}

        rid = oh_uid_lookup(&rdr.Entity);
        
	rv = oh_add_rdr(handler->rptcache, rid, &rdr, ctrl_info, 1);
	if (rv != SA_OK) {
		err("couldn't add control rdr. rv = %d", rv);
		free(ctrl_info);
		return 1;
	}

#if 0	
		/* May be it's hot swap indicator? */	
	if (strcmp(rdr->IdString.Data, "blue led")) {
		return;
	}
	if (rpt->HotSwapCapabilities &
			SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED) {
				// already set?
		err("Resource %d. hot swap indicator already set 0x%x !?",
					rid, rpt->HotSwapCapabilities);
	}
	trace_ipmi("Attach hot swap indicator into entity %d(%s)",
		rpt->ResourceId, rpt->ResourceTag.Data);
	info->hotswapind = e->u.rdr_event.rdr.RdrTypeUnion.CtrlRec.Num;
	rpt->HotSwapCapabilities |=
			SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED;
#endif
	return 0;
}
/*
 * add_alarm_rdr
 */
static void add_alarm_rdr(char 				*name,
			  int 				num,
			  SaHpiResourceIdT 		rptid,
			  SaHpiEntityPathT 		parent_ent,
			  SaHpiCtrlDefaultModeT		*def_mode,
			  SaHpiBoolT			wo,
			  ipmi_control_id_t 		*control_id,
			  struct oh_handler_state 	*handler)
{
	SaHpiRdrT               rdr_temp;
	SaHpiRdrT               *rdr;
	int			name_len;
	struct ohoi_control_info *ctrl_info;
        struct ohoi_resource_info *info;
	

	info = oh_get_resource_data(handler->rptcache, rptid);
        if (!info) {
                err("No info in resource(%d)\n", rptid);
                return;
        }
        ctrl_info = malloc(sizeof(struct ohoi_control_info));
        if (!ctrl_info) {
                err("Out of memory");
                return;
        }
	ctrl_info->type = OHOI_CTRL_ORIGINAL;
        ctrl_info->info.orig_ctrl_info.ctrl_id = *control_id;
	ctrl_info->mode = SAHPI_CTRL_MODE_AUTO;
	ctrl_info->ohoii.get_control_state = orig_get_control_state;
	ctrl_info->ohoii.set_control_state = orig_set_control_state;
	 
	rdr = &rdr_temp;
        rdr->RecordId = 0;
        rdr->RdrType = SAHPI_CTRL_RDR;
        rdr->Entity = parent_ent;
 
	name_len = strlen(name);
	if (name_len >= SAHPI_MAX_TEXT_BUFFER_LENGTH)
		name_len = SAHPI_MAX_TEXT_BUFFER_LENGTH - 1;
        rdr->IdString.DataType = SAHPI_TL_TYPE_TEXT;
        rdr->IdString.Language = SAHPI_LANG_ENGLISH;
        rdr->IdString.DataLength = strlen(name);
        memset(rdr->IdString.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        memcpy(rdr->IdString.Data, name, strlen(name));

        rdr->RdrTypeUnion.CtrlRec.Num = ++info->ctrl_count;
        rdr->RdrTypeUnion.CtrlRec.Type         = SAHPI_CTRL_TYPE_DIGITAL;
        rdr->RdrTypeUnion.CtrlRec.TypeUnion.Digital.Default =
						SAHPI_CTRL_STATE_OFF;
        rdr->RdrTypeUnion.CtrlRec.OutputType   = SAHPI_CTRL_LED; 
        rdr->RdrTypeUnion.CtrlRec.Oem          = OEM_ALARM_BASE + num;
	/* FIXME: OpenIPMI does not provide a reading */
        rdr->RdrTypeUnion.CtrlRec.WriteOnly    = wo;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.Mode = def_mode->Mode;
	rdr->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = def_mode->ReadOnly;
        if(oh_add_rdr(handler->rptcache, rptid, rdr, ctrl_info, 1)) {
		err("couldn't add alarm control");
		free(ctrl_info);
		return;
	}
	trace_ipmi("add_alarm_rdr: %s\n",name); 
}

/*
 * add_alarm_rdrs
 */
static int add_alarm_rdrs(
		struct oh_handler_state *handler,
		SaHpiRptEntryT *rpt,
		ipmi_control_t     *control)
{
	SaHpiResourceIdT   	rid;
        SaHpiEntityPathT        ent;
	SaHpiCtrlDefaultModeT default_mode;
	SaHpiBoolT wo;
	static ipmi_control_id_t   alarm_control_id;  /*save this */
	static int alarms_done = 0;
	

	if (alarms_done) return 0;  /* only do alarms the first time */
	rid = rpt->ResourceId;
	ent = rpt->ResourceEntity;

	alarm_control_id = ipmi_control_convert_to_id(control);
	wo = (ipmi_control_is_readable(control) == 0);
	default_mode.ReadOnly = (ipmi_control_is_settable(control) != 0);
	default_mode.Mode = SAHPI_CTRL_MODE_AUTO;
	rpt->ResourceCapabilities |=  SAHPI_CAPABILITY_RDR;
	rpt->ResourceCapabilities |=  SAHPI_CAPABILITY_CONTROL;

	add_alarm_rdr("Power Alarm LED", 0, rid, ent,
		&default_mode, wo, &alarm_control_id, handler);
	add_alarm_rdr("Critical Alarm LED", 1, rid, ent,
		&default_mode, wo, &alarm_control_id, handler);
	add_alarm_rdr("Major Alarm LED", 2, rid, ent,
		&default_mode, wo, &alarm_control_id, handler);
	add_alarm_rdr("Minor Alarm LED",   3, rid, ent,
		&default_mode, wo, &alarm_control_id, handler);
	alarms_done = 1;
	return 0;
}


#if 0
static void
address_control(ipmi_control_t *control,
		int		err,
		unsigned char	*val,
		int		length,
		void		*cb_data)
{
	int i;
	int *location = cb_data;
	
	if (control == NULL) {
		err("Invalid control?");
		return;
	}
	

	for (i=0; i<length; i++) {
		//dbg("Address control: 0x%2.2x", val[i]);
		dbg("Address control: %d", val[i]);
	}
	*location = val[1];
	dbg("Location %d", *location);

}

	
static int
address_control_get(ipmi_control_t			*control,
			 struct oh_handler_state	*handler,
			 ipmi_entity_t			*entity,
			SaHpiRptEntryT	*rpt)
{
	int rv;
	int location;
	//SaHpiEntityPathT	entity_ep;
	struct ohoi_handler *ipmi_handler = handler->data;


	rv = ipmi_control_identifier_get_val(control, address_control, &location);

	if(rv) {
		err("Error getting identifier control val");
		return -1;
	}

	ohoi_loop(&location, ipmi_handler);

	//rpt->ResourceEntity.Entry[0].EntityLocation = 
		//ipmi_entity_get_entity_instance(entity) - 96 ;
	//rpt->ResourceEntity.Entry[1].EntityLocation = location;

				//rpt.ResourceId =
					//oh_uid_from_entity_path(&rpt.ResourceEntity);
				//dbg("Control New ResourceId: %d", rpt.ResourceId);
	//rv = oh_add_resource(handler->rptcache, *rpt, NULL, 1);
	//if (rv) {
	      	//err("oh_add_resource failed for %d = %s\n", rpt->ResourceId, oh_lookup_error(rv));
	//}

	return 0;
}
#endif

void ohoi_control_event(enum ipmi_update_e op,
		        ipmi_entity_t      *ent,
			ipmi_control_t     *control,
			void               *cb_data)
{
	struct oh_handler_state *handler = cb_data;
	struct ohoi_handler *ipmi_handler = handler->data;
        struct ohoi_resource_info *ohoi_res_info;
	int ctrl_type = ipmi_control_get_type(control);
	ipmi_control_id_t cid = ipmi_control_convert_to_id(control);
	
	
	ipmi_entity_id_t	entity_id;	
	SaHpiRptEntryT		*rpt_entry;
	char str[24];
	int rv;
        
	entity_id = ipmi_entity_convert_to_id(ent);
	rpt_entry = ohoi_get_resource_by_entityid(handler->rptcache,
						  &entity_id);   
	
	if (!rpt_entry) {
			dump_entity_id("Control with RPT Entry?!", entity_id);
			return;
	}
	
	g_static_rec_mutex_lock(&ipmi_handler->ohoih_lock);	
	ohoi_res_info = oh_get_resource_data(handler->rptcache,
                                             rpt_entry->ResourceId);
        
	if (op == IPMI_ADDED) {
		trace_ipmi_control("ADD", control, rpt_entry);                
		/* attach power and reset to chassis entity since
		   IPMI provides them as such */
		switch (ctrl_type) {
		case IPMI_CONTROL_ONE_SHOT_RESET:
			ohoi_res_info->reset_ctrl = cid;
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_RESET;
			break;
		case IPMI_CONTROL_POWER:
			if ((ipmi_handler->d_type == IPMI_DOMAIN_TYPE_ATCA) &&
				(ipmi_entity_get_entity_id(ent) ==
				SAHPI_ENT_SYSTEM_CHASSIS)) {
				// never power off ATCA chassis
				break;
			}
			ohoi_res_info->power_ctrl = cid;
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_POWER;
			break;
		case IPMI_CONTROL_ALARM:
			rv = add_alarm_rdrs(handler,rpt_entry,control);
			if (rv) {
				err("add_alarms_rdrs failed");
				break;
			}
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_CONTROL |
					SAHPI_CAPABILITY_RDR;
			break;
		case IPMI_CONTROL_IDENTIFIER:
#if 0
			rv = address_control_get(control,handler, ent, rpt_entry);
			if (rv) {
				err("address_control_get failed");
				break;
			}
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_CONTROL |
					SAHPI_CAPABILITY_RDR;
#endif
			break;
		case IPMI_CONTROL_LIGHT:
			ipmi_control_get_id(control, str, 24);
			if (add_led_control_event(ent, control, handler, rpt_entry)) {
				break;
			}
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_CONTROL |
     					SAHPI_CAPABILITY_RDR;
			break;
		default:
			if (SA_OK != add_control_event(ent, control, handler,
					rpt_entry->ResourceEntity,
					rpt_entry->ResourceId)) {
				break;
			}
			rpt_entry->ResourceCapabilities |=
					SAHPI_CAPABILITY_CONTROL |
     					SAHPI_CAPABILITY_RDR;
			break;
		}            
	} else if (op == IPMI_DELETED) {
		trace_ipmi_control("DELETE", control, rpt_entry);
		switch (ctrl_type) {
		case IPMI_CONTROL_ONE_SHOT_RESET:
			ipmi_control_id_set_invalid(
				&ohoi_res_info->reset_ctrl);
			rpt_entry->ResourceCapabilities &=
					~SAHPI_CAPABILITY_RESET;
			break;
		case IPMI_CONTROL_POWER:
			ipmi_control_id_set_invalid(
				&ohoi_res_info->power_ctrl);
			rpt_entry->ResourceCapabilities &=
					~SAHPI_CAPABILITY_POWER;
			break;
		default:
			if (ohoi_delete_orig_control_rdr(handler,
							rpt_entry, &cid)) {
				// no more controlss for rpt
				rpt_entry->ResourceCapabilities &=
				 ~SAHPI_CAPABILITY_CONTROL;
			ohoi_res_info->ctrl_count = 0;
			}
			break;
		}
		if ((oh_get_rdr_next(handler->rptcache, rpt_entry->ResourceId,
					 SAHPI_FIRST_ENTRY) == NULL) &&
					 (ohoi_res_info->fru == NULL)) {
				// no more rdrs for rpt
			rpt_entry->ResourceCapabilities &=
							~SAHPI_CAPABILITY_RDR; 
		}
	}
	trace_ipmi("Set updated for res_info %p(%d). Control", ohoi_res_info, rpt_entry->ResourceId);
	entity_rpt_set_updated(ohoi_res_info, ipmi_handler);
	g_static_rec_mutex_unlock(&ipmi_handler->ohoih_lock);	
}
	
