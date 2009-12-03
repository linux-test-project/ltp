/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copright IBM Corp 2003-2006
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
 *     Sean Dague <http://dague.net/sean>
 *     Renier Morales <renier@openhpi.org>
 *     Racing Guo <racing.guo@intel.com>
 *     Anton Pak <anton.pak@pigeonpoint.com>
 */

#ifndef __OH_HANDLER_H
#define __OH_HANDLER_H

#include <uuid/uuid.h>
#include <glib.h>

#include <sys/time.h>
#include <SaHpi.h>
#include <oh_event.h>
#include <oh_utils.h>
#include <oHpi.h>

#ifdef __cplusplus
extern "C" {
#endif

void oh_wake_event_thread(SaHpiBoolT);

/*
 * Common OpenHPI implementation specific definitions
 * --------------------------------------------------
 *
 * plugin - software component contained in a shared library that exports
 *          a function named 'get_interface'.  Loading a plugin entails
 *          performing a dlopen on the library, finding 'get_interface' with
 *          dl_sym, and calling 'get_interface' to get an interface pointer
 *          (referred to as the 'abi'.)
 *
 * abi - pointer to a structure of type oh_abi_XX where XX represents a
 *       version of the structure.  This structure is a bundle of function
 *       pointers that represents the interface between a given plug-in
 *       instance (known as a handler), and the OpenHPI infrastructure.
 *
 * handler - an instance of a plugin represented by a structure of type
 *           oh_handler which contains an abi and a pointer to an instance
 *           specific data structure that is private to the plug-in.
 */

/*
 * How plugins are instantiated
 * ----------------------------
 *
 * When an HPI application initializes OpenHPI by calling saHpiInitialize(),
 * the OpenHPI infrastructure will seek out all configured plug-ins
 * (see oh_config.h for details on how a plug-in is configured), and:
 * 1. load the plug-in into memory
 * 2. extract an abi from the plug-in
 * 3. create a new oh_plugin containing the name of the plugin and
 *    the abi extracted from the plugin
 * 4. add the oh_plugin to the global list of plugins
 *
 * The first time the HPI application creates a new session by calling
 * saHpiSessionOpen(), the OpenHPI infrastructure will once again examine
 * the implementation configuration and create new plug-in instances
 * (i.e. a handler) as the configuration dictates.
 *
 * Each handler configuration item will specify:
 * 1. name of plugin in question
 * 2. additional arguments understood by the plug-in
 *
 * Each new handler is created by:
 * 1. finding the oh_plugin containing the same plugin name as the
 *    configuration item
 * 2. using the abi found in the oh_plugin to call abi->open(), passing
 *    the additional plug-in specific arguments to the open function.
 *    The open call will return a void* pointer (known as hnd) that is
 *    required to be passed back to the plug-in for all further abi
 *    function calls.
 * 3. creating a new oh_handler that contains a pointer to the associated
 *    abi, and the hnd returned by the open function.
 */

/*
 * How plugins can have multiple instances open at the same time
 * -------------------------------------------------------------
 *
 * The key to a plugin being able to support multiple instances
 * is in the 'void *hnd' passed to all abi functions (except open().)
 * The intent is that hnd is used as a private pointer to an instance specific
 * data structure.
 *
 * For example, if a plug-in were created to allow an HPI implementation
 * running a remote server to inter-operate with the local OpenHPI
 * implementation, then the plug-in could be written such that:
 * 1. the plugin defines a new structure containing an event queue and tcp
 *    socket to the remote machine
 * 2. the plugin requires that handler configuration entries for this
 *    plugin to contain the IP address of the remote machine to connect
 * 3. when open() is called, the plugin
 *    - opens a socket to the new machine
 *    - allocates a new event queue
 *    - allocates a new instance structure
 *    - stores the event queue and socket in the instance structure
 *    - returns a pointer to the structure as 'hnd'.
 * 4. as other abi functions are called, the 'hnd' passed in with those
 *    functions is cast back to a pointer to the instance data, and then
 *    communicates over the socket in that structure to service the given
 *    request.
 *
 */

struct oh_handler_state {
        unsigned int    hid;
        oh_evt_queue    *eventq;
        GHashTable      *config;
        RPTable         *rptcache;
        oh_el           *elcache;
        GThread         *thread_handle;
        void            *data;
};

/* Current abi is version 2. Version 1 is out-of-date and nobody
 * should use it
 */

/* UUID_OH_ABI_V1 is out-of-date, keep here just for reference
 * ee778a5f-32cf-453b-a650-518814dc956c
 */
/* static const uuid_t UUID_OH_ABI_V1 = {
        0xee, 0x77, 0x8a, 0x5f, 0x32, 0xcf, 0x45, 0x3b,
        0xa6, 0x50, 0x51, 0x88, 0x14, 0xdc, 0x95, 0x6c
};
*/

/* 13adfcc7-d788-4aaf-b966-5cd30bdcd808 */
/* regen this with via
 *
 * perl -e 'use POSIX qw(strftime); my $str = strftime("%Y%m%d%H%M%S",gmtime(time)); $str .= "00"; for my $c (split(//, $str)) {print "0x0$c, "} '
 *
 * any time you make a change */

/*
static const uuid_t UUID_OH_ABI_V2 = {
        0x02, 0x00, 0x00, 0x04, 0x01, 0x02, 0x01, 0x05,
        0x01, 0x06, 0x01, 0x04, 0x01, 0x00, 0x00, 0x00,
};
*/


static const uuid_t UUID_OH_ABI_V2 = {
        0x02, 0x00, 0x00, 0x05, 0x00, 0x04, 0x01, 0x03,
        0x01, 0x02, 0x05, 0x04, 0x05, 0x00, 0x00, 0x00,
};


struct oh_abi_v2 {
        /***
         * open
         * handler_config: instance's configuration data.
         * hid: id of this intance.
         * eventq: pointer to queue to place events in.
         *
         * The function creates a pluing instance.
         *
         * Returns: pointer to the handler of the instance
         **/
        void *(*open)(GHashTable *handler_config,
                      unsigned int hid,
                      oh_evt_queue *eventq);

        void (*close)(void *hnd);

	/********************************************************************
	 * PLUGIN HPI ABIs - These plugin functions implement a large part of
	 * the HPI APIs from the specification.
	 ********************************************************************/

        /***
         * saHpiEventGet - passed down to plugin
         *
         * @remark at the start-up, plugins must send out res/rdr event for all
         * resources and rdrs so as to OpenHPI can build up RPT/RDR.
         * @return >0 if an event is returned; 0 if timeout; otherwise an error
         * occur.
         **/
        SaErrorT (*get_event)(void *hnd);

        /***
         * saHpiDiscover, passed down to plugin
         **/
        SaErrorT (*discover_resources)(void *hnd);

        /***
         * saHpiResourceTagSet, this is passed down so the device has
         * a chance to set it in nv storage if it likes
         **/
        SaErrorT (*set_resource_tag)(void *hnd,
				     SaHpiResourceIdT id,
				     SaHpiTextBufferT *tag);

        /***
         * saHpiResourceSeveritySet is pushed down so the device has
         * a chance to set it in nv storage
         **/
        SaErrorT (*set_resource_severity)(void *hnd,
					  SaHpiResourceIdT id,
					  SaHpiSeverityT sev);
        /***
         * saHpiResourceFailedRemove, passed down to plugin
         **/
	SaErrorT (*resource_failed_remove)(void *hnd,
					   SaHpiResourceIdT rid);

        /*****************
         * EVENT LOG ABIs
         *****************/

        /***
         * saHpiEventLogInfoGet
         **/
        SaErrorT (*get_el_info)(void *hnd,
				SaHpiResourceIdT id,
				SaHpiEventLogInfoT *info);

        /***
         * saHpiEventLogCapabilitiesGet
         **/
        SaErrorT (*get_el_caps)(void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiEventLogCapabilitiesT *caps);

        /***
         * saHpiEventLogTimeSet
         **/
        SaErrorT (*set_el_time)(void *hnd,
				SaHpiResourceIdT id,
				SaHpiTimeT time);

        /***
         * saHpiEventLogEntryAdd
         **/
        SaErrorT (*add_el_entry)(void *hnd,
				 SaHpiResourceIdT id,
				 const SaHpiEventT *Event);

        /***
         * saHpiEventLogEntryGet
         **/
        SaErrorT (*get_el_entry)(void *hnd,
				 SaHpiResourceIdT id,
				 SaHpiEventLogEntryIdT current,
				 SaHpiEventLogEntryIdT *prev,
				 SaHpiEventLogEntryIdT *next,
				 SaHpiEventLogEntryT *entry,
				 SaHpiRdrT *rdr,
				 SaHpiRptEntryT *rptentry);

        /***
         * saHpiEventLogClear
         **/
        SaErrorT (*clear_el)(void *hnd, SaHpiResourceIdT id);

        /***
         * saHpiEventLogStateSet
         **/
        SaErrorT (*set_el_state)(void *hnd, SaHpiResourceIdT id, SaHpiBoolT e);

        /***
         * saHpiEventLogOverflowReset
         **/
        SaErrorT (*reset_el_overflow)(void *hnd, SaHpiResourceIdT id);

	/**************
	 * SENSOR ABIs
	 **************/

        /***
         * saHpiSensorReadingGet
         **/
        SaErrorT (*get_sensor_reading)(void *hnd,
				       SaHpiResourceIdT id,
                                       SaHpiSensorNumT num,
                                       SaHpiSensorReadingT *reading,
                                       SaHpiEventStateT *state);
        /***
         * saHpiSensorThresholdsGet
         **/
        SaErrorT (*get_sensor_thresholds)(void *hnd,
					  SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  SaHpiSensorThresholdsT *thres);

        /***
         * saHpiSensorThresholdsSet
         **/
        SaErrorT (*set_sensor_thresholds)(void *hnd,
					  SaHpiResourceIdT id,
					  SaHpiSensorNumT num,
					  const SaHpiSensorThresholdsT *thres);

        /***
         * saHpiSensorEnableGet
         **/
        SaErrorT (*get_sensor_enable)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiSensorNumT num,
                                      SaHpiBoolT *enable);

        /***
         * saHpiSensorEnableSet
         **/
        SaErrorT (*set_sensor_enable)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiSensorNumT num,
                                      SaHpiBoolT enable);

        /***
         * saHpiSensorEventEnableGet
         **/
        SaErrorT (*get_sensor_event_enables)(void *hnd,
					     SaHpiResourceIdT id,
					     SaHpiSensorNumT num,
					     SaHpiBoolT *enables);

        /***
         * saHpiSensorEventEnableSet
         **/
        SaErrorT (*set_sensor_event_enables)(void *hnd,
					     SaHpiResourceIdT id,
                                    	     SaHpiSensorNumT num,
                                    	     const SaHpiBoolT enables);

        /***
         * saHpiSensorEventMasksGet
         **/
        SaErrorT (*get_sensor_event_masks)(void *hnd,
					   SaHpiResourceIdT id,
                                           SaHpiSensorNumT num,
                                           SaHpiEventStateT *AssertEventMask,
                                           SaHpiEventStateT *DeassertEventMask);

	/***
         * saHpiSensorEventMasksSet
         **/
        SaErrorT (*set_sensor_event_masks)(void *hnd, SaHpiResourceIdT id,
                                           SaHpiSensorNumT num,
                                           SaHpiSensorEventMaskActionT act,
                                           SaHpiEventStateT AssertEventMask,
                                           SaHpiEventStateT DeassertEventMask);

	/***************
	 * CONTROL ABIs
	 ***************/

        /***
         * saHpiControlGet
         **/
        SaErrorT (*get_control_state)(void *hnd,
				      SaHpiResourceIdT id,
				      SaHpiCtrlNumT num,
				      SaHpiCtrlModeT *mode,
				      SaHpiCtrlStateT *state);

        /***
         * saHpiControlSet
         **/
        SaErrorT (*set_control_state)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiCtrlNumT num,
                                      SaHpiCtrlModeT mode,
                                      SaHpiCtrlStateT *state);

	/*****************
	 * INVENTORY ABIs
	 *****************/

	/***
         * saHpiIdrInfoGet
         **/
        SaErrorT (*get_idr_info)(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiIdrIdT idrid,
				 SaHpiIdrInfoT *idrinfo);

        /***
         * saHpiIdrAreaHeaderGet
         **/
        SaErrorT (*get_idr_area_header)(void *hnd,
					SaHpiResourceIdT rid,
					SaHpiIdrIdT idrid,
					SaHpiIdrAreaTypeT areatype,
                              		SaHpiEntryIdT areaid,
					SaHpiEntryIdT *nextareaid,
					SaHpiIdrAreaHeaderT *header);

        /***
         * saHpiIdrAreaAdd
         **/
        SaErrorT (*add_idr_area)(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiIdrIdT idrid,
				 SaHpiIdrAreaTypeT areatype,
				 SaHpiEntryIdT *areaid);
                                 
        /***
         * saHpiIdrAreaAddById
         **/
        SaErrorT (*add_idr_area_id)(void *,
                                    SaHpiResourceIdT,
                                    SaHpiIdrIdT,
                                    SaHpiIdrAreaTypeT,
                                    SaHpiEntryIdT);

        /***
         * saHpiIdrAreaDelete
         **/
        SaErrorT (*del_idr_area)(void *hnd,
				 SaHpiResourceIdT rid,
				 SaHpiIdrIdT idrid,
				 SaHpiEntryIdT areaid);

        /***
         * saHpiIdrFieldGet
         **/
        SaErrorT (*get_idr_field)(void *hnd,
				  SaHpiResourceIdT rid,
				  SaHpiIdrIdT idrid,
				  SaHpiEntryIdT areaid,
				  SaHpiIdrFieldTypeT fieldtype,
				  SaHpiEntryIdT fieldid,
                             	  SaHpiEntryIdT *nextfieldid,
				  SaHpiIdrFieldT *field);

        /***
         * saHpiIdrFieldAdd
         **/
        SaErrorT (*add_idr_field)(void *hnd,
				  SaHpiResourceIdT rid,
				  SaHpiIdrIdT idrid,
				  SaHpiIdrFieldT *field);

        /***
         * saHpiIdrFieldAddById
         **/
        SaErrorT (*add_idr_field_id)(void *hnd,
                                     SaHpiResourceIdT rid,
                                     SaHpiIdrIdT idrid,
                                     SaHpiIdrFieldT *field);

        /***
         * saHpiIdrFieldSet
         **/
        SaErrorT (*set_idr_field)(void *hnd,
				  SaHpiResourceIdT rid,
				  SaHpiIdrIdT idrid,
				  SaHpiIdrFieldT *field);

        /***
         * saHpiIdrFieldDelete
         **/
        SaErrorT (*del_idr_field)(void *hnd,
				  SaHpiResourceIdT rid,
				  SaHpiIdrIdT idrid,
				  SaHpiEntryIdT areaid,
				  SaHpiEntryIdT fieldid);

        /***
         * saHpiWatchdogTimerGet
         **/
        SaErrorT (*get_watchdog_info)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiWatchdogNumT num,
                                      SaHpiWatchdogT *wdt);

        /***
         * saHpiWatchdogTimerSet
         **/
        SaErrorT (*set_watchdog_info)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiWatchdogNumT num,
                                      SaHpiWatchdogT *wdt);

        /***
         * saHpiWatchdogTimerReset
         **/
        SaErrorT (*reset_watchdog)(void *hnd,
				   SaHpiResourceIdT id,
                              	   SaHpiWatchdogNumT num);

        /******************
         * ANNUCIATOR ABIs
         ******************/

        /* the first 5 Annunciator functions are really operating on
           a single Annunciator and doing things to the announcements
           that it contains.  For this reason the functions are named
           _announce */
	/***
	 * saHpiAnnunciatorGetNext
	 **/
        SaErrorT (*get_next_announce)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiAnnunciatorNumT num,
				      SaHpiSeverityT sev,
                                      SaHpiBoolT ack,
				      SaHpiAnnouncementT *ann);

	/***
	 * saHpiAnnunciatorGet
	 **/
        SaErrorT (*get_announce)(void *hnd,
				 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
				 SaHpiEntryIdT annid,
				 SaHpiAnnouncementT *ann);

	/***
	 * saHpiAnnunciatorAcknowledge
	 **/
	SaErrorT (*ack_announce)(void *hnd,
				 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
				 SaHpiEntryIdT annid,
				 SaHpiSeverityT sev);

	/***
	 * saHpiAnnunciatorAdd
	 **/
        SaErrorT (*add_announce)(void *hnd,
				 SaHpiResourceIdT id,
                                 SaHpiAnnunciatorNumT num,
				 SaHpiAnnouncementT *ann);

	/***
	 * saHpiAnnunciatorDelete
	 **/
        SaErrorT (*del_announce)(void *hnd,
				 SaHpiResourceIdT id,
                             	 SaHpiAnnunciatorNumT num,
				 SaHpiEntryIdT annid,
				 SaHpiSeverityT sev);

        /* the last 2 functions deal with Annunciator mode setting */

	/***
	 * saHpiAnnunciatorModeGet
	 **/
        SaErrorT (*get_annunc_mode)(void *hnd,
				    SaHpiResourceIdT id,
                                    SaHpiAnnunciatorNumT num,
				    SaHpiAnnunciatorModeT *mode);

	/***
	 * saHpiAnnunciatorModeSet
	 **/
	SaErrorT (*set_annunc_mode)(void *hnd,
				    SaHpiResourceIdT id,
                                    SaHpiAnnunciatorNumT num,
				    SaHpiAnnunciatorModeT mode);
				    
	/***************
	 * DIMI ABIs
	 ***************/
	 
	/***
	 * saHpiDimiInfoGet
	 **/
	SaErrorT (*get_dimi_info)(void *hnd,
				  SaHpiResourceIdT id,
				  SaHpiDimiNumT num,
				  SaHpiDimiInfoT *info);

        /***
         * saHpiDimiTestInfoGet
         **/
        SaErrorT (*get_dimi_test)(void *hnd,
                                  SaHpiResourceIdT id,
                                  SaHpiDimiNumT num,
                                  SaHpiDimiTestNumT testnum,
                                  SaHpiDimiTestT *test);

        /***
         * saHpiDimiTestReadinessGet
         **/
        SaErrorT (*get_dimi_test_ready)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiDimiNumT num,
                                SaHpiDimiTestNumT testnum,
                                SaHpiDimiReadyT *ready);

        /***
         * saHpiDimiTestStart
         **/
        SaErrorT (*start_dimi_test)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiDimiNumT num,
                                SaHpiDimiTestNumT testnum,
                                SaHpiUint8T numparams,
                                SaHpiDimiTestVariableParamsT *paramslist);

        /***
         * saHpiDimiTestCancel
         **/
        SaErrorT (*cancel_dimi_test)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiDimiNumT num,
                                SaHpiDimiTestNumT testnum);

        /***
         * saHpiDimiTestStatusGet
         **/
        SaErrorT (*get_dimi_test_status)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiDimiNumT num,
                                SaHpiDimiTestNumT testnum,
                                SaHpiDimiTestPercentCompletedT *percentcompleted,
                                SaHpiDimiTestRunStatusT *runstatus);

        /***
         * saHpiDimiTestResultsGet
         **/
        SaErrorT (*get_dimi_test_results)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiDimiNumT num,
                                SaHpiDimiTestNumT testnum,
                                SaHpiDimiTestResultsT *testresults);

        /***************
         * FUMI ABIs
         ***************/

        /***
         * saHpiFumiSpecInfoGet 
         **/
        SaErrorT (*get_fumi_spec)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiFumiSpecInfoT *specinfo );

        /***
         * saHpiFumiServiceImpactGet
         **/
        SaErrorT (*get_fumi_service_impact)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiFumiServiceImpactDataT *serviceimpact );

        /***
         * saHpiFumiSourceSet
         **/
        SaErrorT (*set_fumi_source)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiTextBufferT *sourceuri);

        /***
         * saHpiFumiSourceInfoValidateStart
         **/
        SaErrorT (*validate_fumi_source)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

        /***
         * saHpiFumiSourceInfoGet
         **/
        SaErrorT (*get_fumi_source)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiSourceInfoT *sourceinfo);

        /***
         * saHpiFumiSourceComponentInfoGet
         **/
        SaErrorT (*get_fumi_source_component)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiEntryIdT compid,
                                SaHpiEntryIdT *nextcompid,
                                SaHpiFumiComponentInfoT *compinfo);

        /***
         * saHpiFumiTargetInfoGet
         **/
        SaErrorT (*get_fumi_target)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiBankInfoT *bankinfo);

        /***
         * saHpiFumiTargetComponentInfoGet
         **/
        SaErrorT (*get_fumi_target_component)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiEntryIdT compid,
                                SaHpiEntryIdT *nextcompid,
                                SaHpiFumiComponentInfoT *compinfo);

        /***
         * saHpiFumiLogicalTargetInfoGet
         **/
        SaErrorT (*get_fumi_logical_target)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiFumiLogicalBankInfoT *bankinfo);

        /***
         * saHpiFumiLogicalTargetComponentInfoGet
         **/
        SaErrorT (*get_fumi_logical_target_component)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiEntryIdT compid,
                                SaHpiEntryIdT *nextcompid,
                                SaHpiFumiLogicalComponentInfoT *compinfo);

        /***
         * saHpiFumiBackupStart
         **/
        SaErrorT (*start_fumi_backup)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num);

        /***
         * saHpiFumiBankBootOrderSet
         **/
        SaErrorT (*set_fumi_bank_order)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiUint32T position);

        /***
         * saHpiFumiBankBootOrderSet
         **/
        SaErrorT (*start_fumi_bank_copy)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT sourcebanknum,
                                SaHpiBankNumT targetbanknum);

        /***
         * saHpiFumiInstallStart
         **/
        SaErrorT (*start_fumi_install)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

        /***
         * saHpiFumiUpgradeStatusGet
         **/
        SaErrorT (*get_fumi_status)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiUpgradeStatusT *status);

        /***
         * saHpiFumiTargetVerifyStart
         **/
        SaErrorT (*start_fumi_verify)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

        /***
         * saHpiFumiTargetVerifyMainStart
         **/
        SaErrorT (*start_fumi_verify_main)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num );

        /***
         * saHpiFumiUpgradeCancel
         **/
        SaErrorT (*cancel_fumi_upgrade)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);

        /***
         * saHpiFumiAutoRollbackDisableGet
         **/
        SaErrorT (*get_fumi_autorollback_disable)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBoolT *disable);

        /***
         * saHpiFumiAutoRollbackDisableSet
         **/
        SaErrorT (*set_fumi_autorollback_disable)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBoolT disable);

        /***
         * saHpiFumiRollbackStart
         **/
        SaErrorT (*start_fumi_rollback)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num);

        /***
         * saHpiFumiActivate
         **/
        SaErrorT (*activate_fumi)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num);
        /***
         * saHpiFumiActivateStart
         **/
        SaErrorT (*start_fumi_activate)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBoolT logical);

        /***
         * saHpiFumiCleanup
         **/
        SaErrorT (*cleanup_fumi)(
                                void *hnd,
                                SaHpiResourceIdT id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum);
        
	/***************
	 * HOTSWAP ABIs
	 ***************/

	/***
         * saHpiHotSwapPolycyCancel
         **/
        SaErrorT (*hotswap_policy_cancel)(void *hnd,
					  SaHpiResourceIdT id,
                                 	  SaHpiTimeoutT timeout);

        /***
         * saHpiAutoInsertTimeoutSet
         **/
        SaErrorT (*set_autoinsert_timeout)(void *hnd,
					   SaHpiTimeoutT timeout);

	/***
	 * saHpiAutoExtractTimeoutGet
	 **/
	SaErrorT (*get_autoextract_timeout)(void *hnd,
					    SaHpiResourceIdT id,
					    SaHpiTimeoutT *timeout);

	/***
	 * saHpiAutoExtractTimeoutSet
	 **/
	SaErrorT (*set_autoextract_timeout)(void *hnd,
					    SaHpiResourceIdT id,
					    SaHpiTimeoutT timeout);

        /***
         * saHpiHotSwapStateGet
         **/
        SaErrorT (*get_hotswap_state)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiHsStateT *state);

        /***
         * saHpiHotSwapStateSet
         **/
        SaErrorT (*set_hotswap_state)(void *hnd,
				      SaHpiResourceIdT id,
                                      SaHpiHsStateT state);

        /***
	 * saHpiHotSwapActionRequest
         **/
        SaErrorT (*request_hotswap_action)(void *hnd,
					   SaHpiResourceIdT id,
                                           SaHpiHsActionT act);

	/***
	 * saHpiHotSwapIndicatorStateGet
         **/
        SaErrorT (*get_indicator_state)(void *hnd,
					SaHpiResourceIdT id,
                                        SaHpiHsIndicatorStateT *state);

        /***
	 * saHpiHotSwapIndicatorStateSet
         **/
        SaErrorT (*set_indicator_state)(void *hnd,
					SaHpiResourceIdT id,
                                        SaHpiHsIndicatorStateT state);

	/*************
	 * POWER ABIs
	 *************/

        /***
         * saHpiResourcePowerStateGet
         **/
        SaErrorT (*get_power_state)(void *hnd,
				    SaHpiResourceIdT id,
                               	    SaHpiPowerStateT *state);

        /***
         * saHpiResourcePowerStateSet
         **/
        SaErrorT (*set_power_state)(void *hnd,
				    SaHpiResourceIdT id,
                               	    SaHpiPowerStateT state);

	/*****************
	 * PARAMETER ABIs
	 *****************/

        /***
         * saHpiParmControl
         **/
        SaErrorT (*control_parm)(void *hnd,
				 SaHpiResourceIdT id,
				 SaHpiParmActionT act);

        /***********************
         * Load Management ABIs
         ***********************/
     
        /***
         * saHpiResourceLoadIdGet
         **/
        SaErrorT (*load_id_get)(void *hnd,
                                SaHpiResourceIdT rid,
                                SaHpiLoadIdT *load_id);

        /***
         * saHpiResourceLoadIdSet
         **/
        SaErrorT (*load_id_set)(void *hnd,
                                SaHpiResourceIdT rid,
                                SaHpiLoadIdT *load_id);
                                
	/*************
	 * RESET ABIs
	 *************/

        /***
         * saHpiResourceResetStateGet
         **/
        SaErrorT (*get_reset_state)(void *hnd,
				    SaHpiResourceIdT id,
                               	    SaHpiResetActionT *act);

        /***
         * saHpiResourceResetStateSet
         **/
        SaErrorT (*set_reset_state)(void *hnd,
				    SaHpiResourceIdT id,
                               	    SaHpiResetActionT act);

	/**********************************
	 * INJECTOR ABIs - OpenHPI specific
	 **********************************/

	 /***
	  * oHpiInjectEvent
	  **/
	  SaErrorT (*inject_event)(void *hnd,
                            	   SaHpiEventT *event,
                            	   SaHpiRptEntryT *rpte,
                            	   SaHpiRdrT *rdr);

};

/*The function is used for plugin loader to get interface*/
int get_interface(void **pp, const uuid_t uuid) __attribute__ ((weak));

/* Structure for static plugins */
typedef int (*get_interface_t)( void **pp, const uuid_t uuid );

struct oh_static_plugin
{
        char           *name;
        get_interface_t get_interface;
};

#ifdef __cplusplus
}
#endif

/* Macors for use in handlers to walk a single linked list such as
 * eventq
 */
#define g_slist_for_each(pos, head) \
        for (pos = head; pos != NULL; pos = g_slist_next(pos))

#define g_slist_for_each_safe(pos, pos1, head) \
        for (pos = head, pos1 = g_slist_next(pos); pos; pos = pos1, pos1 = g_slist_next(pos1))

#endif/*__OH_HANDLER_H*/
