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
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     Kevin Gao <kevin.gao@linux.intel.com>
 *     Rusty Lynch <rusty.lynch@linux.intel.com>
 *     Racing Guo <racing.guo@intel.com>
 */
#ifndef _INC_IPMI_H_
#define _INC_IPMI_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <OpenIPMI/ipmiif.h>
#include <OpenIPMI/ipmi_fru.h>
#include <OpenIPMI/ipmi_smi.h>
#include <OpenIPMI/ipmi_err.h>
#include <OpenIPMI/ipmi_auth.h>
#include <OpenIPMI/ipmi_lan.h>
#include <OpenIPMI/selector.h>
#include <OpenIPMI/os_handler.h>
#include <OpenIPMI/ipmi_posix.h>
#include <OpenIPMI/ipmi_debug.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_handler.h>


#include <SaHpiAtca.h>
#include <OpenIPMI/ipmi_picmg.h>
#include <OpenIPMI/ipmi_msgbits.h>



#define IPMI_DATA_WAIT	10
#define OEM_ALARM_BASE  0x10

#define	IPMI_EVENT_DATA_MAX_LEN 13
#define	MAX_ES_STATE 15 /* max number of possible event state - 1 */


typedef struct ohoi_atca_pwonseq_dsk_s {
	unsigned char body[5];
	SaHpiResourceIdT slotid;
} ohoi_atca_pwonseq_dsk_t;

typedef struct ohoi_atca_pwonseq_rec_s {
	unsigned char head[7];
	unsigned char updated;
	unsigned int rec_num;
} ohoi_atca_pwonseq_rec_t;


struct ohoi_handler {
	GStaticRecMutex ohoih_lock;
	int SDRs_read_done;
	int bus_scan_done;
    	int SELs_read_done;
	int mc_count;		/* to keep track of num of mcs to wait on sdrs */
	int sel_clear_done;	/* we need to wait for mc_sel_reread for clear to succeed */
	int FRU_done;			/* we have to track FRUs */

	ipmi_domain_id_t domain_id;

	/* OpenIPMI connection and os_handler */
	os_handler_t *os_hnd;
	ipmi_con_t *cons[2];
	unsigned int num_cons;

	selector_t *ohoi_sel;

	char *entity_root;
	int connected;
	int islan;
	int fully_up;
	time_t fullup_timeout;
	int updated;
	unsigned int openipmi_scan_time;
	int real_write_fru;
	SaHpiDomainIdT	did;
	enum ipmi_domain_type d_type;
	char domain_name[24];
	/*  ATCA part */
	int shmc_num;
	int shmc_present_num;
	ipmi_mcid_t virt_mcid;
	SaHpiResourceIdT atca_shelf_id;
	SaHpiResourceIdT atca_vshm_id;
	int shelf_fru_corrupted;
	int atca_pwonseq_updated;
	GSList *atca_pwonseq_recs;
	GSList *atca_pwonseq_desk;
};

#define IS_ATCA(type) ((type) == IPMI_DOMAIN_TYPE_ATCA)

struct ohoi_inventory_info;

typedef struct ohoi_slotid_s {
	unsigned char addr;
	unsigned char devid;
	ipmi_entity_id_t entity_id;
} ohoi_slotid_t;



typedef struct ohoi_entity_s {
	ipmi_mcid_t      mc_id;
	ipmi_entity_id_t entity_id;
} ohoi_entity_t;;


#define	OHOI_RESOURCE_ENTITY		0x01
#define	OHOI_RESOURCE_SLOT		0x02 
#define OHOI_RESOURCE_MC		0x04
#define OHOI_FAN_CONTROL_CREATED	0x10
#define OHOI_MC_RESET_CONTROL_CREATED	0x20
#define OHOI_MC_IPMB0_CONTROL_CREATED	0x40



struct ohoi_resource_info {
	  	
  	unsigned char presence;	/* entity presence from OpenIPMI to determine
			   	to push RPT to domain RPTable or not */
	unsigned char updated;	/* refcount of resource add/update from
			   	rptcache to domain RPT */
	unsigned char deleted;	/* entity must be deleled after event of removing
				RPT has been sent to domain */
	unsigned char hs_mark;  /* to handle properly M3->M6->M1 ATCA transition */
	SaHpiTimeoutT hs_inspen_time; /* time of last insertion pending state */
	SaHpiUint8T  sensor_count; 
        SaHpiUint8T  ctrl_count; 

	unsigned int type;
        union {
                ohoi_entity_t entity;
		ohoi_slotid_t    slot;
        } u;
	int max_ipmb0_link;
        ipmi_control_id_t reset_ctrl;
        ipmi_control_id_t power_ctrl;
        SaHpiCtrlNumT hotswapind;
	struct ohoi_inventory_info *fru;
};


typedef struct atca_common_info {
	int		done;
	SaErrorT	rv;
	unsigned char	data[255];
	unsigned int	len;
	unsigned char	addr;
	unsigned char	devid;
	void		*info;
} atca_common_info_t;

		/* Sensor info */


#define OHOI_THS_LMINH	0x0001
#define OHOI_THS_LMINL	0x0002
#define OHOI_THS_LMAJH	0x0004
#define OHOI_THS_LMAJL	0x0008
#define OHOI_THS_LCRTH	0x0010
#define OHOI_THS_LCRTL	0x0020
#define OHOI_THS_UMINH	0x0040
#define OHOI_THS_UMINL	0x0080
#define OHOI_THS_UMAJH	0x0100
#define OHOI_THS_UMAJL	0x0200
#define OHOI_THS_UCRTH	0x0400
#define OHOI_THS_UCRTL	0x0800

typedef enum {
	OHOI_SENSOR_ORIGINAL = 1,
	OHOI_SENSOR_ATCA_MAPPED
} ohoi_sensor_type_t;

typedef struct ohoi_original_sensor_info {
	ipmi_sensor_id_t		sensor_id;
} ohoi_original_sensor_info_t;


typedef struct ohoi_atcamap_sensor_info {
	void			*data;
	int			val;
	SaHpiResourceIdT	rid;
} ohoi_atcamap_sensor_info_t;

typedef union {
	ohoi_original_sensor_info_t	orig_sensor_info;
	ohoi_atcamap_sensor_info_t	atcamap_sensor_info;
} ohoi_sensor_info_union_t;


struct ohoi_sensor_info;

typedef struct ohoi_sensor_interfaces {
	SaErrorT (*get_sensor_event_enable)(struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT   *enable,
					    SaHpiEventStateT  *assert,
					    SaHpiEventStateT  *deassert);
	SaErrorT (*set_sensor_event_enable)(struct oh_handler_state *hnd,
					    struct ohoi_sensor_info *sinfo,
					    SaHpiBoolT enable,
					    SaHpiEventStateT assert,
					    SaHpiEventStateT deassert,
					    unsigned int a_supported,
					    unsigned int d_supported);
	SaErrorT (*get_sensor_reading)(struct oh_handler_state *hnd,
				       struct ohoi_sensor_info *sensor_info,
				       SaHpiSensorReadingT *reading,
				       SaHpiEventStateT *ev_state);
	SaErrorT (*get_sensor_thresholds)(struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  SaHpiSensorThresholdsT *thres);
	SaErrorT (*set_sensor_thresholds)(struct oh_handler_state *hnd,
					  struct ohoi_sensor_info *sinfo,
					  const SaHpiSensorThresholdsT *thres);
} ohoi_sensor_interfaces_t;

struct ohoi_sensor_info {
	ohoi_sensor_type_t		type;
	ohoi_sensor_info_union_t	info;
	int				sen_enabled;
	SaHpiBoolT			enable;
	SaHpiEventStateT		assert;
	SaHpiEventStateT		deassert;
	unsigned int			support_assert;
	unsigned int			support_deassert;
	ohoi_sensor_interfaces_t	ohoii;
};




		/*  Control  info*/

typedef enum {
	OHOI_CTRL_ORIGINAL = 1,
	OHOI_CTRL_ATCA_MAPPED
} ohoi_control_type_t;


typedef struct ohoi_original_ctrl_info {
	ipmi_control_id_t ctrl_id;
} ohoi_original_ctrl_info_t;


typedef struct ohoi_atcamap_ctrl_info {
	void			*data;
	int			val;
	SaHpiResourceIdT	rid;
} ohoi_atcamap_ctrl_info_t;

typedef union {
	ohoi_original_ctrl_info_t	orig_ctrl_info;
	ohoi_atcamap_ctrl_info_t	atcamap_ctrl_info;
} ohoi_control_info_union_t;


struct ohoi_control_info;

typedef struct ohoi_ctrl_interfaces {
	SaErrorT (*get_control_state)(struct oh_handler_state *hnd,
				      struct ohoi_control_info *c,
				      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT *mode,
                                      SaHpiCtrlStateT *state);
	SaErrorT (*set_control_state)(struct oh_handler_state *hnd,
                                      struct ohoi_control_info *c,
                                      SaHpiRdrT * rdr,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);
} ohoi_ctrl_interfaces_t;



struct ohoi_control_info {
	ohoi_control_type_t		type;
	ohoi_control_info_union_t	info;
	SaHpiCtrlModeT 			mode;
	ohoi_ctrl_interfaces_t		ohoii;
};



		/*  Inventory info*/


struct ohoi_inventory_info {
	SaHpiUint32T	update_count;
	// zero if area doesn't exist. If area exists
	// lang for board and product info, 1 for the rest
	unsigned char	iu, ci, bi, pi, oem;
	unsigned int ci_fld_msk;
	unsigned int ci_custom_num;
	unsigned int bi_fld_msk;
	unsigned int bi_custom_num;
	unsigned int pi_fld_msk;
	unsigned int pi_custom_num;
	unsigned int oem_fields_num;
	GSList *oem_areas;
	GMutex *mutex;
};

#define OHOI_AREA_EMPTY_ID		0
#define OHOI_AREA_FIRST_ID		1

#define	OHOI_INTERNAL_USE_AREA_ID	1
#define	OHOI_CHASSIS_INFO_AREA_ID	2
#define	OHOI_BOARD_INFO_AREA_ID		3
#define	OHOI_PRODUCT_INFO_AREA_ID	4
#define	FIRST_OEM_AREA_NUM		5

#define OHOI_FIELD_EMPTY_ID		0
#define OHOI_FIELD_FIRST_ID		1

SaHpiTextTypeT convert_to_hpi_data_type(enum ipmi_str_type_e type);

/* implemented in ipmi_event.c */
void ohoi_setup_done(ipmi_domain_t *domain, void *user_data);
/* implemented in ipmi_close.c */
void ohoi_close_connection(ipmi_domain_id_t domain_id, void *user_data);

/* implemented in ipmi_sensor.c	*/

SaErrorT orig_get_sensor_reading(struct oh_handler_state *handler,
				 struct ohoi_sensor_info *sinfo, 
				 SaHpiSensorReadingT * reading,
				 SaHpiEventStateT * ev_state);

SaErrorT ohoi_get_sensor_reading(void *hnd,
				 struct ohoi_sensor_info *sinfo, 
				 SaHpiSensorReadingT * reading,
				 SaHpiEventStateT * ev_state);

SaErrorT orig_get_sensor_thresholds(struct oh_handler_state *handler,
				    struct ohoi_sensor_info *sinfo,
				    SaHpiSensorThresholdsT *thres);

SaErrorT ohoi_get_sensor_thresholds(void *hnd,
				    struct ohoi_sensor_info *sinfo,
				    SaHpiSensorThresholdsT *thres);

SaErrorT orig_set_sensor_thresholds(struct oh_handler_state *handler,
				    struct ohoi_sensor_info *sinfo, 
				    const SaHpiSensorThresholdsT *thres);

SaErrorT ohoi_set_sensor_thresholds(void *hnd,
				    struct ohoi_sensor_info *sinfo, 
				    const SaHpiSensorThresholdsT *thres);

int ohoi_set_sensor_enable(ipmi_sensor_id_t sensor_id,
			   SaHpiBoolT   enable,
			   void *cb_data);

SaErrorT orig_get_sensor_event_enable(struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT *enable,
				      SaHpiEventStateT *assert,
				      SaHpiEventStateT *deassert);

SaErrorT ohoi_get_sensor_event_enable(void *hnd,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT *enable,
				      SaHpiEventStateT *assert,
				      SaHpiEventStateT *deassert);

SaErrorT orig_set_sensor_event_enable(struct oh_handler_state *handler,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT enable,
				      SaHpiEventStateT assert,
				      SaHpiEventStateT deassert,
				      unsigned int a_supported,
				      unsigned int d_supported);

SaErrorT ohoi_set_sensor_event_enable(void *hnd,
				      struct ohoi_sensor_info *sinfo,
				      SaHpiBoolT enable,
				      SaHpiEventStateT assert,
				      SaHpiEventStateT deassert,
				      unsigned int a_supported,
				      unsigned int d_supported);

void ohoi_get_sel_time(ipmi_mcid_t mc_id, SaHpiTimeT *time, void *cb_data);
void ohoi_set_sel_time(ipmi_mcid_t mc_id, const struct timeval *time, void *cb_data);
void ohoi_get_sel_updatetime(ipmi_mcid_t mc_id, SaHpiTimeT *time);
void ohoi_get_sel_size(ipmi_mcid_t mc_id, int *size);
void ohoi_get_sel_count(ipmi_mcid_t mc_id, int *count);
void ohoi_get_sel_overflow(ipmi_mcid_t mc_id, char *overflow);
void ohoi_get_sel_support_del(ipmi_mcid_t mc_id, char *support_del);
SaErrorT ohoi_clear_sel(ipmi_mcid_t mc_id, void *cb_data);
SaErrorT ohoi_set_sel_state(struct ohoi_handler *ipmi_handler, ipmi_mcid_t mc_id, int enable);
SaErrorT ohoi_get_sel_state(struct ohoi_handler *ipmi_handler, ipmi_mcid_t mc_id, int *enable);
void ohoi_get_sel_first_entry(ipmi_mcid_t mc_id, ipmi_event_t **event);
void ohoi_get_sel_last_entry(ipmi_mcid_t mc_id, ipmi_event_t **event);
void ohoi_get_sel_next_recid(ipmi_mcid_t mc_id, 
                             ipmi_event_t *event,
                             unsigned int *record_id);
void ohoi_get_sel_prev_recid(ipmi_mcid_t mc_id, 
                             ipmi_event_t *event, 
                             unsigned int *record_id);
void ohoi_get_sel_by_recid(ipmi_mcid_t mc_id, SaHpiEventLogEntryIdT entry_id, ipmi_event_t **event);

/* This is used to help plug-in to find resource in rptcache by entity_id and mc_id*/
SaHpiResourceIdT ohoi_get_parent_id(SaHpiRptEntryT *child);

SaHpiRptEntryT *ohoi_get_resource_by_entityid(RPTable                *table,
                                              const ipmi_entity_id_t *entity_id);
SaHpiRptEntryT *ohoi_get_resource_by_mcid(RPTable                *table,
                                          const ipmi_mcid_t *mc_id);
/* This is used to help plug-in to find rdr in rptcache by data*/
SaHpiRdrT *ohoi_get_rdr_by_data(RPTable *table,
                                SaHpiResourceIdT rid,
                                SaHpiRdrTypeT type,
                                void *data);

/* This is used for OpenIPMI to notice sensor change */
void ohoi_sensor_event(enum ipmi_update_e op,
                       ipmi_entity_t      *ent,
                       ipmi_sensor_t      *sensor,
                       void               *cb_data);       
/*
 * This is used to help saHpiEventLogEntryGet()
 * to convert sensor ipmi event to hpi event
 */
int ohoi_sensor_ipmi_event_to_hpi_event(
			struct ohoi_handler *ipmi_handler,
			ipmi_sensor_id_t	sid,
			ipmi_event_t		*event,
			struct oh_event		**e,
			ipmi_entity_id_t	*eid);


/* This is used for OpenIPMI to notice control change */
void ohoi_control_event(enum ipmi_update_e op,
                        ipmi_entity_t      *ent,
                        ipmi_control_t     *control,
                        void		   *cb_data);

/* This is used for OpenIPMI to notice mc change */
void ohoi_mc_event(enum ipmi_update_e op,
                   ipmi_domain_t      *domain,
                   ipmi_mc_t          *mc,
                   void               *cb_data);

/* This is used for OpenIPMI to noice inventroy change */
void ohoi_inventory_event(enum ipmi_update_e    op,
                          ipmi_entity_t         *entity,
                          void                  *cb_data);

/* This is used for OpenIPMI to notice entity change */
void ohoi_entity_event(enum ipmi_update_e       op,
                       ipmi_domain_t            *domain,
                       ipmi_entity_t            *entity,
                       void                     *cb_data);

int ohoi_loop(int *done_flag, struct ohoi_handler *ipmi_handler);
typedef int (*loop_indicator_cb)(const void *cb_data);
int ohoi_loop_until(loop_indicator_cb indicator, const void *cb_data, int timeout, struct ohoi_handler *ipmi_handler); 

SaErrorT ohoi_get_rdr_data(const struct oh_handler_state *handler,
                           SaHpiResourceIdT              id,
                           SaHpiRdrTypeT                 type,
                           SaHpiSensorNumT               num,
                           void                          **pdata);
int ohoi_delete_orig_sensor_rdr(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   ipmi_sensor_id_t *mysid);
int ohoi_delete_orig_control_rdr(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   ipmi_control_id_t *mycid);
typedef int (*rpt_loop_handler_cb)(
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt,
                             struct ohoi_resource_info *res_info,
			     void *cb_data);
void ohoi_iterate_rptcache(struct oh_handler_state *handler,
			   rpt_loop_handler_cb func, void *cb_data);
typedef int (*rdr_loop_handler_cb)(
			     struct oh_handler_state *handler,
			     SaHpiRptEntryT *rpt,
                             SaHpiRdrT      *rdr,
			     void *cb_data);
void ohoi_iterate_rpt_rdrs(struct oh_handler_state *handler,
			   SaHpiRptEntryT *rpt,
			   rdr_loop_handler_cb func, void *cb_data);
int ohoi_rpt_has_controls(struct oh_handler_state *handler,
                         SaHpiResourceIdT rid);
int ohoi_rpt_has_sensors(struct oh_handler_state *handler,
                         SaHpiResourceIdT rid);

SaErrorT ohoi_get_idr_info(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiIdrInfoT *idrinfo);
SaErrorT ohoi_get_idr_area_header(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiIdrAreaTypeT areatype,
                              SaHpiEntryIdT areaid,  SaHpiEntryIdT *nextareaid, SaHpiIdrAreaHeaderT *header);
SaErrorT ohoi_add_idr_area(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiIdrAreaTypeT areatype, SaHpiEntryIdT *areaid);
SaErrorT ohoi_del_idr_area(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiEntryIdT areaid);
SaErrorT ohoi_get_idr_field(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid,
                             SaHpiEntryIdT areaid, SaHpiIdrFieldTypeT fieldtype, SaHpiEntryIdT fieldid,
                             SaHpiEntryIdT *nextfieldid, SaHpiIdrFieldT *field);
SaErrorT ohoi_add_idr_field(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiIdrFieldT *field );
SaErrorT ohoi_set_idr_field(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiIdrFieldT *field );
SaErrorT ohoi_del_idr_field(void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid, SaHpiEntryIdT areaid, SaHpiEntryIdT fieldid);

void ohoi_delete_rpt_fru(struct ohoi_resource_info *res_info);

int ohoi_hot_swap_cb(ipmi_entity_t  *ent,
                     enum ipmi_hot_swap_states last_state,
		     enum ipmi_hot_swap_states curr_state,
		     void   *cb_data,
		     ipmi_event_t  *event);

SaErrorT ohoi_get_hotswap_state(void *hnd, SaHpiResourceIdT id, 
                                SaHpiHsStateT *state);

SaErrorT ohoi_set_hotswap_state(void *hnd, SaHpiResourceIdT id, 
                                SaHpiHsStateT state);

SaErrorT ohoi_request_hotswap_action(void *hnd, SaHpiResourceIdT id, 
                                     SaHpiHsActionT act);

SaErrorT ohoi_get_indicator_state(void *hnd, SaHpiResourceIdT id, 
                                  SaHpiHsIndicatorStateT *state);
SaErrorT ohoi_hotswap_policy_cancel(void *hnd, SaHpiResourceIdT rid,
                                    SaHpiTimeoutT  ins_timeout);

SaErrorT ohoi_set_indicator_state(void *hnd, SaHpiResourceIdT id, 
				  SaHpiHsIndicatorStateT state);

SaErrorT ohoi_set_power_state(void *hnd, SaHpiResourceIdT id, 
                              SaHpiPowerStateT state);

SaErrorT ohoi_get_power_state(void *hnd, SaHpiResourceIdT id,
			      SaHpiPowerStateT *state);
	
SaErrorT ohoi_set_reset_state(void *hnd, SaHpiResourceIdT id, 
		              SaHpiResetActionT act);

SaErrorT ohoi_get_reset_state(void *hnd, SaHpiResourceIdT id, 
		              SaHpiResetActionT *act);


SaErrorT orig_get_control_state(struct oh_handler_state *handler,
				struct ohoi_control_info *c,
				SaHpiRdrT * rdr,
                                SaHpiCtrlModeT *mode,
                                SaHpiCtrlStateT *state);

SaErrorT orig_set_control_state(struct oh_handler_state *handler,
				struct ohoi_control_info *c,
				SaHpiRdrT * rdr,
                                SaHpiCtrlModeT mode,
                                SaHpiCtrlStateT *state);

SaErrorT ohoi_get_control_state(void *hnd, SaHpiResourceIdT id,
                                SaHpiCtrlNumT num,
                                SaHpiCtrlModeT *mode,
                                SaHpiCtrlStateT *state);

SaErrorT ohoi_set_control_state(void *hnd, SaHpiResourceIdT id,
                                SaHpiCtrlNumT num,
                                SaHpiCtrlModeT mode,
                                SaHpiCtrlStateT *state);

void ipmi_connection_handler(ipmi_domain_t	*domain,
			      int		err,
			      unsigned int	conn_num,
			      unsigned int	port_num,
			      int		still_connected,
			      void		*cb_data);	     

struct ohoi_fru_write_s {
	SaErrorT rv;
	int      done;
};
SaErrorT ohoi_fru_write(struct ohoi_handler	*ipmi_handler,
			ipmi_entity_id_t	entid);





		/* ATCA-HPI mapping functions */
		
void ohoi_atca_create_fru_rdrs(struct oh_handler_state *handler);
void ohoi_atca_delete_fru_rdrs(struct oh_handler_state *handler,
                             ipmi_mcid_t mcid);
				
SaHpiUint8T ohoi_atca_led_to_hpi_color(int ipmi_color);
int ohoi_atca_led_to_ipmi_color(SaHpiUint8T c);

void adjust_sensor_to_atcahpi_spec(struct oh_handler_state *handler,
				   SaHpiRptEntryT	*rpt,
				   SaHpiRdrT		*rdr,
				   struct ohoi_sensor_info *sensor_info,
				   ipmi_sensor_t	*sensor);


void ohoi_atca_create_shelf_virtual_rdrs(struct oh_handler_state *hnd);
void create_atca_virt_shmgr_rdrs(struct oh_handler_state *hnd);
void ohoi_send_vshmgr_redundancy_sensor_event(
                                      struct oh_handler_state *handler,
				      int become_present);
	/* ATCA Fan Control */
void ohoi_create_fan_control(struct oh_handler_state *handler,
                             SaHpiResourceIdT rid);
	/* ATCA IPMB-0 Control */
void ohoi_create_ipmb0_controls(struct oh_handler_state *handler,
				ipmi_entity_t *entity,
				unsigned int max_link);

	/* ATCA FRU MC Reset Control */
void ohoi_create_fru_mc_reset_control(struct oh_handler_state *handler,
                             SaHpiResourceIdT rid);

	/* ATCA Ekeying Link State Sensor */
#define OHOI_FIRST_EKEYING_SENSOR_NUM 0x400
void ohoi_create_ekeying_link_state_sensor(
			struct oh_handler_state *handler,
			ipmi_entity_t	*entity,
			unsigned int	s_num,
			unsigned char	*guid,
			unsigned char	link_grouping_id,
			unsigned char	link_type,
			unsigned char	link_type_extension,
			unsigned char	interface_type,
			unsigned char	*channels);

	/* ATCA inventory functions */
unsigned int ohoi_create_atca_oem_idr_areas(
			struct oh_handler_state *handler,
			ipmi_entity_t *entity,
			struct ohoi_resource_info *res_info,
			struct ohoi_inventory_info *i_info,
			unsigned int r_num);
SaHpiUint32T ohoi_atca_oem_area_fields_num(struct ohoi_inventory_info *fru,
			    SaHpiEntryIdT areaid);
SaErrorT ohoi_atca_oem_area_field(struct oh_handler_state  *handler,
				struct ohoi_resource_info   *ohoi_res_info,
				SaHpiEntryIdT *nextfieldid,
				SaHpiIdrFieldT *field);


	/* ATCA slot state specific functions */

void atca_slot_state_sensor_event_send(struct oh_handler_state *handler,
				       SaHpiRptEntryT *dev_entry,
				       int present);
void atca_create_slot_rdrs(struct oh_handler_state *handler,
			   SaHpiResourceIdT rid);



/* misc macros for debug */
#define dump_entity_id(s, x) \
        do { \
                err("%s domain id: %p, entity id: %x, entity instance: %x, channel: %x, address: %x, seq: %lx", \
                     s,                         \
                     (x).domain_id.domain,      \
                     (x).entity_id,             \
                     (x).entity_instance,       \
                     (x).channel,               \
                     (x).address,               \
                     (x).seq);                  \
        } while(0)


/* dump rpttable to make debug easy 
   if you don't like it, feel free to delete it.
   IMO, it is a good idea to place dump_rpttable in rpt_utils.c

*/

static inline void  dump_rpttable(RPTable *table)
{
       SaHpiRptEntryT *rpt;
       rpt = oh_get_resource_next(table, SAHPI_FIRST_ENTRY);

       printf("\n");
       while (rpt) {
               printf("Resource Id:%d", rpt->ResourceId);
	       struct ohoi_resource_info *res_info =
	       		oh_get_resource_data(table, rpt->ResourceId);
		if (res_info->type & OHOI_RESOURCE_ENTITY) {
			ipmi_entity_id_t e = res_info->u.entity.entity_id;
			printf("; entity id: %x, entity instance: %x, channel: %x, address: %x, seq: %lx",
				e.entity_id, e.entity_instance, e.channel, e.address, e.seq);
		}
		printf("\n");
#if 0
               SaHpiRdrT  *rdr;
               rdr = oh_get_rdr_next(table, rpt->ResourceId, SAHPI_FIRST_ENTRY);
               while (rdr) {
                       unsigned char *data;
                       int i;
                       data = oh_get_rdr_data(table, rpt->ResourceId, rdr->RecordId);
			/*FIXME:: causes issue on IA64 */
			/* commenting out for now until fixed */
                       //printf("(Rdr id:%d type:%d) data pointer:%u\n", 
                               //rdr->RecordId,
                               //rdr->RdrType,
                               //(unsigned)(void *)data);
                       //if (data)
                       //        for (i = 0; i < 30; i++)
                       //                printf("%u ", data[i]);
                       //printf("\n");
                       rdr = oh_get_rdr_next(table, rpt->ResourceId, rdr->RecordId);
               }
              // printf("\n");
#endif
               rpt = oh_get_resource_next(table, rpt->ResourceId);
      }
}

#endif

/* called when a resource is removed (swapped?) */
int entity_presence(ipmi_entity_t *entity, int present, void *cb_data, ipmi_event_t *event);
int ipmi_discover_resources(void *hnd);
void entity_rpt_set_updated(struct ohoi_resource_info *res_info,
		struct ohoi_handler *hnd);
void entity_rpt_set_presence(struct ohoi_resource_info *res_info,
		struct ohoi_handler *hnd, int presence);
void ohoi_remove_entity(struct oh_handler_state *handler,
			SaHpiResourceIdT res_id);



#define OHOI_MAP_ERROR(to, err) \
         do {\
		if (err == (IPMI_IPMI_ERR_TOP | IPMI_INVALID_CMD_CC)) {\
			to = SA_ERR_HPI_INVALID_CMD;\
		} else if (err == (IPMI_IPMI_ERR_TOP | IPMI_NODE_BUSY_CC)) {\
			to = SA_ERR_HPI_BUSY;\
		} else if (err == (IPMI_IPMI_ERR_TOP | IPMI_TIMEOUT_CC)) {\
			to = SA_ERR_HPI_NO_RESPONSE;\
		} else if (err == (IPMI_IPMI_ERR_TOP | IPMI_COMMAND_INVALID_FOR_LUN_CC)) {\
			to = SA_ERR_HPI_INVALID_CMD;\
		} else if (err == (IPMI_IPMI_ERR_TOP | IPMI_CANNOT_EXEC_DUPLICATE_REQUEST_CC)) {\
			to = SA_ERR_HPI_BUSY;\
		} else {\
			to = SA_ERR_HPI_INTERNAL_ERROR;\
		}\
	} while (0)


	/*
	 * The following traces are available :
	 *     OHOI_TRACE_ALL - trace all the following traces and trace_ipmi(). Must be "YES".
	 *     OHOI_TRACE_SENSOR -  traces sensors add/change/delete and for events in SEL
	 *     OHOI_TRACE_ENTITY - traces entities add/change/delete
	 *     OHOI_TRACE_MC     - traces MCs  add/change/delete/active/inactive
	 *     OHOI_TRACE_DISCOVERY - prints all existing resources (present and not present)
	 *                            after discovery
	 *
	 * the values of these variables are ignored
	 */
	 
#define IHOI_TRACE_ALL (getenv("OHOI_TRACE_ALL") &&\
                           !strcmp("YES",getenv("OHOI_TRACE_ALL")))
	

#define trace_ipmi(format, ...) \
        do { \
                if (IHOI_TRACE_ALL) { \
                        fprintf(stderr, " %s:%d:%s: ", __FILE__, __LINE__, __func__); \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)

	

#define trace_ipmi_sensors(action, sid) \
        do { \
                if (getenv("OHOI_TRACE_SENSOR") || IHOI_TRACE_ALL) { \
                        fprintf(stderr, "   *** SENSOR %s. sensor_id = {{%p, %d, %d, %ld}, %d, %d}\n", action,\
			sid.mcid.domain_id.domain, sid.mcid.mc_num, sid.mcid.channel, sid.mcid.seq,\
			sid.lun, sid.sensor_num);\
                } \
        } while(0)
	
extern FILE *trace_msg_file; // File to trace all IPMI messages	




