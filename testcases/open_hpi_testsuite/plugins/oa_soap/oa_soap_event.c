/*
 * Copyright (C) 2007-2009, Hewlett-Packard Development Company, LLP
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
 *      Raghavendra P.G. <raghavendra.pg@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 * This file is having the code for handling the events which
 * are coming from OA.
 *
 *      oa_soap_get_event()             - this is not required in case of oa. it
 *                                        is always "PULL" method used for
 *                                        handling any type of signals from oa
 *
 *      oa_soap_event_thread()          - handles the oa events and pushes the
 *                                        same into the framework queue
 *
 *      oa_soap_error_handling()        - handles the oa events based error
 *                                        handling scenario.
 *
 *      process_oa_out_of_access()      - handles the oa, which went out of
 *                                        access
 *
 *      process_oa_events()             - handles the oa events and calls
 *                                        correct handler function for
 *                                        different events
 **/

#include "oa_soap_event.h"

/**
 * oa_soap_get_event
 *      @oh_handler: Pointer to openhpi handler structure
 *
 * Purpose:
 *      Gets the event from the plugin event queue.
 *      Pushes the event to infrastructure
 *
 * Detailed Description: NA
 *
 * Return values:
 *      1 - on pushing the event to infrastructure.
 *      0 - if there are no events in the plugin to push thru this function.
 **/

int oa_soap_get_event(void *oh_handler)
{
        /* Since OA sends the events on any changes to resources
         * Using this function, OA need not to be polled for resource state
         * changes.  This method always returns 0
         *
         * No events for infra-structure to process
         */
        return 0;
}

/**
 * event_thread
 *      @oa_pointer: Pointer to the oa_info structure for this thread.
 *
 * Purpose:
 *      Gets the event from the OA.
 *      Processes the OA event and pushes the event to infrastructure
 *
 * Detailed Description: NA
 *
 * Return values:
 *      (gpointer *) SA_OK                     - on success.
 *      (gpointer *) SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      (gpointer *) SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

gpointer oa_soap_event_thread(gpointer oa_pointer)
{
        SaErrorT rv = SA_OK;
        struct getAllEvents request;
        struct getAllEventsResponse response;
        struct oh_handler_state *handler = NULL;
        struct oa_info *oa = NULL;
        int ret_code = SA_ERR_HPI_INVALID_PARAMS;
        int retry_on_switchover = 0;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiBoolT is_plugin_initialized = SAHPI_FALSE;
        SaHpiBoolT is_discovery_completed = SAHPI_FALSE;
        SaHpiBoolT listen_for_events = SAHPI_TRUE;
        char *user_name, *password, url[MAX_URL_LEN];

        if (oa_pointer == NULL) {
                err("Invalid parameter");
                g_thread_exit(&ret_code);
        }

        /* Extract the oh_handler and oa_info structure from oa_pointer */
	oa = (struct oa_info *)oa_pointer;
	handler = oa->oh_handler;
	oa_handler = handler->data;

        dbg("OA SOAP event thread started for OA %s", oa->server);

        /* Check whether the plugin is initialized.
         * If not, wait till plugin gets initialized
         */
        while (is_plugin_initialized == SAHPI_FALSE) {
        	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                g_mutex_lock(oa_handler->mutex);
                if (oa_handler->status == PRE_DISCOVERY ||
                    oa_handler->status == DISCOVERY_COMPLETED) {
                        g_mutex_unlock(oa_handler->mutex);
                        is_plugin_initialized = SAHPI_TRUE;
                } else {
                        g_mutex_unlock(oa_handler->mutex);
                        dbg("Waiting for the plugin initialization "
                            "to complete.");
                        sleep(2);
                }
        }

        /* Check whether the discovery is over.
         * If not, wait till discovery gets completed
         */
        while (is_discovery_completed == SAHPI_FALSE) {
        	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                g_mutex_lock(oa_handler->mutex);
                if (oa_handler->status == DISCOVERY_COMPLETED) {
                        g_mutex_unlock(oa_handler->mutex);
                        is_discovery_completed = SAHPI_TRUE;
                } else {
                        g_mutex_unlock(oa_handler->mutex);
                        dbg("Waiting for the discovery to complete.");
                        sleep(2);
                }
        }

        /* If the OA server is NULL, do not even try to open the connection
           just get out */
        if (!strcmp(oa->server,"")) { 
                err("oa->server is NULL. Exiting the thread");
                g_thread_exit((gpointer *)NULL);
        } 

        /* The following is an workaround for an OA bug, where the IP is 
           returned as 0.0.0.0 Just quit in that case also */
        if (!strcmp(oa->server,"0.0.0.0")) { 
                err("OA returned IP is 0.0.0.0.");
                g_thread_exit((gpointer *)NULL);
        } 

        /* Check whether OA Status is ABSENT
         * If yes, wait till the OA status becomes ACTIVE or STANDBY
         */
        g_mutex_lock(oa->mutex);
        if (oa->oa_status != OA_ABSENT) {
                g_mutex_unlock(oa->mutex);
        } else {
                g_mutex_unlock(oa->mutex);
                process_oa_out_of_access(handler, oa);
        }

        /* Get the user_name and password from config file */
        user_name = (char *) g_hash_table_lookup(handler->config,
                                                 "OA_User_Name");
        password = (char *) g_hash_table_lookup(handler->config, "OA_Password");

        /* Check whether the OA is accessible or not
         * If the OA is not accessible or OA is not present,
         * then SOAP_CON will be NULL, try to create the OA connection
         */
        if (oa->event_con == NULL) {
                /* This call will not return until the OA connection is
                 * established
                 */
                create_oa_connection(oa_handler, oa, user_name, password);
                rv = create_event_session(oa);
                /* Sleep for a second, let OA stabilize
                 * TODO: Remove this workaround, when OA has the fix
                 */
                sleep(1);
        }

        /* Ideally, the soap_open should pass in 1st try.
         * If not, try until soap_open succeeds
         */
        memset(url, 0, MAX_URL_LEN);
        snprintf(url, strlen(oa->server) + strlen(PORT) + 1,
                 "%s" PORT, oa->server);
        while (oa->event_con2 == NULL) {
        	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                oa->event_con2 = soap_open(url, user_name, password,
                                           HPI_CALL_TIMEOUT);
                if (oa->event_con2 == NULL) 
                        sleep(2);
        }

        /* Intialize the event request structure */
        request.pid = oa->event_pid;
        request.waitTilEventHappens = HPOA_TRUE;
        request.lcdEvents = HPOA_FALSE;

        /* Listen for the events from OA */
        while (listen_for_events == SAHPI_TRUE) {
        	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                rv = soap_getAllEvents(oa->event_con, &request, &response);
                if (rv == SOAP_OK) {
                        retry_on_switchover = 0;
                        /* OA returns empty event response payload for LCD
                         * status change events. Ignore empty event response.
                         */
                        if (response.eventInfoArray == NULL) {
                                dbg("Ignoring empty event response");
                        } else
                                process_oa_events(handler, oa, &response);
                } else {
                        /* On switchover, the standby-turned-active OA stops
                         * responding to SOAP calls to avoid the network loop.
                         * This change is applicable from OA firmware version
                         * 2.21. Re-try the getAllEvents SOAP XML call skipping
                         * the error handling.
                         */
                        if (oa->oa_status == STANDBY &&
                            get_oa_fw_version(handler) >= OA_2_21 &&
                            retry_on_switchover < MAX_RETRY_ON_SWITCHOVER) {
                                sleep(WAIT_ON_SWITCHOVER);
                                dbg("getAllEvents call failed, may be due to "
                                    "OA switchover");
                                dbg("Re-try the getAllEvents SOAP call");
                                retry_on_switchover++;
                        } else {
                                /* Try to recover from the error */
                                err("OA %s may not be accessible", oa->server);
                                oa_soap_error_handling(handler, oa);
                                request.pid = oa->event_pid;

                                /* Re-initialize the con */
                                if (oa->event_con2 != NULL) {
                                        soap_close(oa->event_con2);
                                        oa->event_con2 = NULL;
                                }
                                memset(url, 0, MAX_URL_LEN);
                                snprintf(url, strlen(oa->server) +
                                         strlen(PORT) + 1,
                                        "%s" PORT, oa->server);

                                /* Ideally, the soap_open should pass in
                                 * 1st try. If not, try until soap_open succeeds
                                 */
                                while (oa->event_con2 == NULL) {
        				OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler,
								  NULL, NULL,
								  NULL);
					oa->event_con2 =
						soap_open(url, user_name,
							  password,
							  HPI_CALL_TIMEOUT);
                                        if (oa->event_con2 == NULL) {
                                                if (oa->oa_status == OA_ABSENT)
                                                          sleep(60);
                                                else
                                                          sleep(5);
                                                err("soap_open for oa->event_con2 failed\n");
                                        }
                                }
                        } /* end of else (non-switchover error handling) */
                } /* end of else (SOAP call failure handling) */

        } /* end of 'while(listen_for_events == SAHPI_TRUE)' loop */

        return (gpointer *) SA_OK;
}

/**
 * oa_soap_error_handling
 *      @oa_handler: Pointer to the OA SOAP handler structure
 *      @oa:         Pointer to the oa info structure
 *
 * Purpose:
 *      Process the OA error and establishes the connection with OA
 *      Calls the re-discovery if the OA status is ACTIVE
 *
 * Detailed Description: NA
 *
 * Return values:
 *      NONE - void return, as it is comes out only on recovering from the
 *             problem.
 **/

void oa_soap_error_handling(struct oh_handler_state *oh_handler,
                            struct oa_info *oa)
{
        SaErrorT rv = SA_OK;
        int is_switchover = SAHPI_FALSE;
        SaHpiBoolT is_oa_accessible = SAHPI_FALSE;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T error_code;
        char *user_name = NULL, *password = NULL;

        if (oh_handler == NULL || oa == NULL) {
                err("Invalid parameters");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* If the OA is not PRESENT, then do not even try. Just get out */
        if ( oa->oa_status == OA_ABSENT ) 
                return;

        /* Check whether OA was present. If not, event_con will be NULL */
        g_mutex_lock(oa->mutex);
        if (oa->event_con == NULL) {
                g_mutex_unlock(oa->mutex);
                /* Get the user_name and password from config file */
                user_name = (char *) g_hash_table_lookup(oh_handler->config,
                                                         "OA_User_Name");
                password = (char *) g_hash_table_lookup(oh_handler->config,
                                                        "OA_Password");
                /* Create the OA connection */
                create_oa_connection(oa_handler, oa, user_name, password);
                /* OA session is established. Set the error_code to SOAP_OK
                 * to skip the processing for OA out of access
                 */
                error_code = SOAP_OK;
        } else {
                error_code = soap_error_number(oa->event_con);
                g_mutex_unlock(oa->mutex);
        }

        /* This loop ends when the OA is accessible */
        while (is_oa_accessible == SAHPI_FALSE) {
                OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                /* Check whether the failure is not due to OA event session
                 * expiry
                 */
                if (error_code != SOAP_OK ||
                    error_code != ERR_EVENT_PIPE ||
                    error_code != ERR_EVENT_DAEMON_KILLED) {
                        /* OA may not be reachable, try to establish the
                         * connection
                         */
                        process_oa_out_of_access(oh_handler, oa);
                }

                /* Create a fresh event session */
                rv = create_event_session(oa);
                if (rv != SA_OK) {
                        /* Set the error code to  -1 to make sure
                         * recovery for OA out of access is recovery is done
                         */
                        error_code = -1;
                        continue;
                }

                /* Sleep for a second, let OA stabilize
                 * TODO: Remove this workaround, when OA has the fix
                 */
                sleep(1);

                is_oa_accessible = SAHPI_TRUE;
                if (oa->oa_status == ACTIVE) {
                       /* Always lock the oa_handler mutex and then oa_info
                        * mutex.  This is to avoid the deadlock.
                        */
                        g_mutex_lock(oa_handler->mutex);
                        g_mutex_lock(oa->mutex);
                        /* Re-discover the resources as there is a high chances
                         * that we might have missed some events
                         */
                	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, oa_handler->mutex,
						  oa->mutex, NULL);
                        rv = oa_soap_re_discover_resources(oh_handler, oa, is_switchover);
                        g_mutex_unlock(oa->mutex);
                        g_mutex_unlock(oa_handler->mutex);
                        if (rv != SA_OK) {
                                is_oa_accessible = SAHPI_FALSE;
                                err("Re-discovery failed for OA %s",
                                    oa->server);
                                /* Set the error code to  -1 to make sure
                                 * recovery for OA out of access is recovery
                                 * is done
                                 */
                                error_code = -1;
                        }
                }
        }

        err("OA %s is accessible", oa->server);
        return;
}

/**
 * process_oa_out_of_access
 *      @oa_handler: Pointer to the OA SOAP handler structure
 *      @oa:         Pointer to the oa info structure
 *
 * Purpose:
 *      Try to establish the connection with OA
 *
 * Detailed Description: NA
 *
 * Return values:
 *      NONE - void return, as this function only on OA is reachable
 **/

void process_oa_out_of_access(struct oh_handler_state *oh_handler,
                              struct oa_info *oa)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        GTimer *timer = NULL;
        gulong micro_seconds;
        gdouble time_elapsed = 0.0, timeout = 2.0;
        SaHpiBoolT is_oa_reachable = SAHPI_FALSE;
        SaHpiBoolT is_oa_present = SAHPI_FALSE;
        SaHpiBoolT oa_was_removed = SAHPI_FALSE;
        char *user_name = NULL, *password = NULL;

        if (oh_handler == NULL || oa == NULL) {
                err("Invalid parameters");
                return;
        }

	/* Get the user_name and password from config file */
	user_name =
		(char *) g_hash_table_lookup(oh_handler->config,
					     "OA_User_Name");
	password =
		(char *) g_hash_table_lookup(oh_handler->config, "OA_Password");

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        /* Start a timer */
        timer = g_timer_new();

        /* This loop ends after OA is accessible */
        while (is_oa_reachable == SAHPI_FALSE) {
                OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, timer);
                /* Check whether the OA is present.
                 * If not, wait till the OA is inserted
                 */
                is_oa_present = SAHPI_FALSE;
                while (is_oa_present == SAHPI_FALSE) {
                	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL,
						  timer);
                        g_mutex_lock(oa->mutex);
                        if (oa->oa_status != OA_ABSENT) {
                                g_mutex_unlock(oa->mutex);
                                is_oa_present = SAHPI_TRUE;
                                time_elapsed = 0.0;
                        } else {
                                g_mutex_unlock(oa->mutex);
                                time_elapsed = g_timer_elapsed(timer,
                                                               &micro_seconds);
                                /* Break the loop on reaching timeout value */
                                if (time_elapsed >= timeout)
                                        break;

                                oa_was_removed = SAHPI_TRUE;
                                /* OA is not present,
                                 * wait for 30 seconds and check again
                                 */
                                sleep(30);
                        }
                }

               /* The re-establishing oa connection on timeout for the extracted
                * OA is done for handling the below scenario
                *
                * Say, Active OA is in slot 1 and standby OA is in slot 2.
                * 1. Remove the active OA (slot 1) which results in
                * switchover. OA in slot status is set to ABSENT.
                * 2. After sometime (atleast 10 mins) current Active OA (slot 2)
                *    is extracted. At this stage, there is no OA in the
                *    c-Class enclosure.
                * 3. OA in slot 1 is inserted back.
                * This leads to a hang situation as the event thread for slot 1
                * is not aware of the OA insertion.
                *
                * But, if the OA in slot 1 is put into a different enclosure
                * (with the same IP, user name and password)
                * then OA SOAP plugin becomes unstable and may lead to crash.
                */
               if (time_elapsed >= timeout) {
                        if (oa->event_con == NULL) {
                                rv = initialize_oa_con(oa, user_name, password);
                                if (rv != SA_OK) {
                                        /* OA is not accessible.
                                         * Restart the timer
                                         */
                                        g_timer_start(timer);
                                        /* Double the timeout value until it
                                         * reaches MAX_TIMEOUT
                                         */
                                        if (timeout < MAX_TIMEOUT) {
                                                timeout = timeout * 2;
                                                if (timeout > MAX_TIMEOUT)
                                                        timeout = MAX_TIMEOUT;
                                        }
                                        continue;
                                }
                        }
                        /* Since the OA connection is re-establised, change the
                         * state of oa_was_removed to false
                         */
                       oa_was_removed = SAHPI_FALSE;
                }

                /* Check whether OA got removed and inserted back.
                 * If yes, re-initialize the soap_con structures.
                 * This creates soap_con structure with the
                 * inserted OA IP address
                 */
                if (oa_was_removed == SAHPI_TRUE) {
 			/* Cleanup the timer */
 			g_timer_destroy(timer);
                        /* Create the OA connection */
                        create_oa_connection(oa_handler, oa, user_name,
                                             password);
			/* OA connection is established. Hence break the loop
			 * and return to the calling function 
 			 */
			return;
                } else {
                        rv = check_oa_status(oa_handler, oa, oa->event_con);
                        if (rv == SA_OK) {
                                is_oa_reachable = SAHPI_TRUE;
                        } else {
                                /* If switchover is in progress, then sleep longer */
                                if (( oa_handler->oa_switching == SAHPI_TRUE ) ||
                                    ( oa->oa_status == OA_ABSENT )) 
                                        sleep(30);
                                else
                                        sleep(2);
                                dbg("check_oa_status failed, oa_status is %d\n",oa->oa_status);
                                /* OA is not accessible. Restart the timer */
                                g_timer_start(timer);
                                /* Double the timeout value until it reaches
                                 * MAX_TIMEOUT
                                 */
                                if (time_elapsed >= timeout &&
                                    timeout < MAX_TIMEOUT) {
                                        timeout = timeout * 2;
                                        if (timeout > MAX_TIMEOUT)
                                                timeout = MAX_TIMEOUT;
                                }
                        }
                }
        }

	/* Cleanup the timer */
	g_timer_destroy(timer);
        return;
}

/**
 * process_oa_events
 *      @oh_handler: Pointer to the openhpi handler structure
 *      @oa:         Pointer to the oa_info structure
 *      @con:        Pointer to the SOAP_CON structure
 *      @response:   Pointer to the oa event response
 *
 * Purpose:
 *      Process the oa event and creates the hpi event structure.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      NONE - void return, as this function processes the events
 **/

void process_oa_events(struct oh_handler_state *oh_handler,
                       struct oa_info *oa,
                       struct getAllEventsResponse *response)
{
        SaErrorT rv;
        SaHpiInt32T loc=0;
        struct eventInfo event;
        struct oa_soap_handler *oa_handler = NULL;

        if (response == NULL || oa == NULL || oh_handler == NULL) {
                err("Invalid parameter");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Extract the events from eventInfoArray */
        while (response->eventInfoArray) {
        	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                /* Get the event from eventInfoArray */
                soap_getEventInfo(response->eventInfoArray, &event);
                switch (event.event) {
                        case EVENT_HEARTBEAT:
                                dbg("HEART BEAT EVENT");
                                break;
                        case EVENT_ENC_STATUS:
				dbg("EVENT_ENC_STATUS");
				oa_soap_proc_enc_status(oh_handler,
					&(event.eventData.enclosureStatus));
                                break;
                        case EVENT_ENC_UID:
                                dbg("EVENT_ENC_UID -- Not processed");
                                break;
                        case EVENT_ENC_SHUTDOWN:
				dbg("EVENT_ENC_SHUTDOWN");
				oa_soap_proc_enc_status(oh_handler,
					&(event.eventData.enclosureStatus));
                                break;
                        case EVENT_ENC_INFO:
                                dbg("EVENT_ENC_INFO -- Not processed");
                                break;
                        case EVENT_ENC_NAMES:
                                dbg("EVENT_ENC_NAMES -- Not processed");
                                break;
                        case EVENT_USER_PERMISSION:
                                dbg("EVENT_USER_PERMISSION -- Not processed");
                                break;
                        case EVENT_ADMIN_RIGHTS_CHANGED:
                                dbg("EVENT_ADMIN_RIGHTS_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_ENC_SHUTDOWN_PENDING:
				dbg("EVENT_ENC_SHUTDOWN_PENDING");
				oa_soap_proc_enc_status(oh_handler,
					&(event.eventData.enclosureStatus));
                                break;
                        case EVENT_ENC_TOPOLOGY:
                                dbg("EVENT_ENC_TOPOLOGY -- Not processed");
                                break;
                        case EVENT_FAN_STATUS:
				dbg("EVENT_FAN_STATUS");
				oa_soap_proc_fan_status(oh_handler,
						&(event.eventData.fanInfo));
                                break;

                        case EVENT_FAN_INSERTED:
                                dbg("EVENT_FAN_INSERTED");
                                rv = process_fan_insertion_event(oh_handler,
                                                                 oa->event_con2,
                                                                 &event);
                                break;

                        case EVENT_FAN_REMOVED:
                                dbg("EVENT_FAN_REMOVED");
                                rv = process_fan_extraction_event(oh_handler,
                                                                  &event);
                                break;

                        case EVENT_FAN_GROUP_STATUS:
                                dbg("EVENT_FAN_GROUP_STATUS -- Not processed");
                                break;
                        case EVENT_THERMAL_STATUS:
                                dbg("EVENT_THERMAL_STATUS -- Not processed");
                                break;
                        case EVENT_COOLING_STATUS:
				dbg("EVENT_COOLING_STATUS");
				oa_soap_proc_therm_subsys_info(oh_handler,
				       &(event.eventData.thermalSubsystemInfo));
                                break;
                        case EVENT_FAN_ZONE_STATUS:
				dbg("EVENT_FAN_ZONE_STATUS");
				oa_soap_proc_fz_status(oh_handler,
						&(event.eventData.fanZone));
                                break;
                        case EVENT_PS_STATUS:
				dbg("EVENT_PS_STATUS");
				oa_soap_proc_ps_status(oh_handler,
					&(event.eventData.powerSupplyStatus));
                                break;
                        case EVENT_PS_INSERTED:
                                dbg("EVENT_PS_INSERTED");
                                rv = process_ps_insertion_event(oh_handler,
                                                                oa->event_con2,
                                                                &event);
                                break;

                        case EVENT_PS_REMOVED:
                                dbg("EVENT_PS_REMOVED");
                                rv = process_ps_extraction_event(oh_handler,
                                                                 &event);
                                break;

                        case EVENT_PS_REDUNDANT:
				dbg("EVENT_PS_REDUNDANT");
				oa_soap_proc_ps_subsys_info(oh_handler,
				       &(event.eventData.powerSubsystemInfo));
                                break;
                        case EVENT_PS_OVERLOAD:
				dbg("EVENT_PS_OVERLOAD");
				oa_soap_proc_ps_subsys_info(oh_handler,
				       &(event.eventData.powerSubsystemInfo));
                                break;
                        case EVENT_AC_FAILURE:
                                dbg("EVENT_AC_FAILURE -- Not processed");
                                break;
                        case EVENT_PS_INFO:
                                dbg("EVENT_PS_INFO -- Not processed");
                                break;
                        case EVENT_PS_SUBSYSTEM_STATUS:
				dbg("EVENT_PS_SUBSYSTEM_STATUS");
				oa_soap_proc_ps_subsys_info(oh_handler,
				       &(event.eventData.powerSubsystemInfo));
                                break;
                        case EVENT_SERVER_POWER_REDUCTION_STATUS:
                                dbg("EVENT_SERVER_POWER_REDUCTION_STATUS "
                                    "-- Not processed");
                                break;
                        case EVENT_INTERCONNECT_STATUS:
                                dbg("EVENT_INTERCONNECT_STATUS");
                                oa_soap_proc_interconnect_status(oh_handler,
				     &(event.eventData.interconnectTrayStatus));
                                break;

                        case EVENT_INTERCONNECT_RESET:
                                dbg("EVENT_INTERCONNECT_RESET");
                                rv = process_interconnect_reset_event(
                                        oh_handler, &event);
                                break;
                        case EVENT_INTERCONNECT_UID:
                                dbg("EVENT_INTERCONNECT_UID -- Not processed");
                                break;
                        case EVENT_INTERCONNECT_INSERTED:
                                dbg("EVENT_INTERCONNECT_INSERTED");
                                rv = process_interconnect_insertion_event(
                                        oh_handler, oa->event_con2, &event);
                                break;

                        case EVENT_INTERCONNECT_REMOVED:
                                dbg("EVENT_INTERCONNECT_REMOVED");
                                rv = process_interconnect_extraction_event(
                                        oh_handler, &event);
                                break;

                        case EVENT_INTERCONNECT_INFO:
                                dbg("EVENT_INTERCONNECT_INFO -- Not processed");
                                break;
                        case EVENT_INTERCONNECT_HEALTH_LED:
                                dbg("EVENT_INTERCONNECT_HEALTH_LED "
                                    "-- Not processed");
                                break;
                        case EVENT_INTERCONNECT_THERMAL:
                                dbg("EVENT_INTERCONNECT_THERMAL");
                                oa_soap_proc_interconnect_thermal(oh_handler,
					oa->event_con2, &(event.eventData.
						interconnectTrayStatus));
                                break;
                        case EVENT_INTERCONNECT_CPUFAULT:
                                dbg("EVENT_INTERCONNECT_CPUFAULT "
                                    "-- Not processed");
                                break;
                        case EVENT_INTERCONNECT_POWER:
                                dbg("EVENT_INTERCONNECT_POWER");
                                rv = process_interconnect_power_event(
                                        oh_handler, &event);
                                break;
                        case EVENT_INTERCONNECT_PORTMAP:
                                dbg("EVENT_INTERCONNECT_PORTMAP "
                                    "-- Not processed");
                                break;
                        case EVENT_BLADE_PORTMAP:
                                dbg("EVENT_BLADE_PORTMAP -- Not processed");
                                break;
                        case EVENT_INTERCONNECT_VENDOR_BLOCK:
                                dbg("EVENT_INTERCONNECT_VENDOR_BLOCK "
                                    "-- Not processed");
                                break;
                        case EVENT_INTERCONNECT_HEALTH_STATE:
                                dbg("EVENT_INTERCONNECT_HEALTH_STATE");
                                oa_soap_proc_interconnect_status(oh_handler,
				     &(event.eventData.interconnectTrayStatus));
                                break;
                        case EVENT_DEMO_MODE:
                                dbg("EVENT_DEMO_MODE -- Not processed");
                                break;
                        case EVENT_BLADE_STATUS:
				dbg("EVENT_BLADE_STATUS");
				oa_soap_proc_server_status(oh_handler,
						oa->event_con2,
						&(event.eventData.bladeStatus));
                                break;

                        case EVENT_BLADE_INSERTED:
                                dbg("EVENT_BLADE_INSERTED -- Not processed");
                                break;

                        case EVENT_BLADE_REMOVED:
                                dbg("EVENT_BLADE_REMOVED");
                                rv = process_server_extraction_event(oh_handler,
                                                                     &event);
                                break;

                        case EVENT_BLADE_POWER_STATE:
				dbg("EVENT_BLADE_POWER_STATE");
                                process_server_power_event(oh_handler,
							   oa->event_con2,
							   &event);
                                break;

                        case EVENT_BLADE_POWER_MGMT:
                                dbg("EVENT_BLADE_POWER_MGMT -- Not processed");
                                break;
                        case EVENT_BLADE_UID:
                                dbg("EVENT_BLADE_UID -- Not processed");
                                break;
                        case EVENT_BLADE_SHUTDOWN:
				dbg("EVENT_BLADE_SHUTDOWN");
				oa_soap_proc_server_status(oh_handler,
						oa->event_con2,
						&(event.eventData.bladeStatus));
                                break;
                        case EVENT_BLADE_FAULT:
				dbg("EVENT_BLADE_FAULT");
				oa_soap_proc_server_status(oh_handler,
						oa->event_con2,
						&(event.eventData.bladeStatus));
                                break;
                        case EVENT_BLADE_INFO:
                                dbg("EVENT_BLADE_INFO -- Not processed");
                                break;
                        case EVENT_BLADE_MP_INFO:
                                dbg("EVENT_BLADE_MP_INFO -- Not processed");
                                break;
                        case EVENT_ILO_READY:
                                dbg("EVENT_ILO_READY -- Not processed");
                                break;
                        case EVENT_LCD_BUTTON:
                                dbg("EVENT_LCD_BUTTON -- Not processed");
                                break;
                        case EVENT_KEYING_ERROR:
                                dbg("EVENT_KEYING_ERROR -- Not processed");
                                break;
                        case EVENT_ILO_HAS_IPADDRESS:
                                dbg("EVENT_ILO_HAS_IPADDRESS -- Not processed");
                                break;
                        case EVENT_POWER_INFO:
                                dbg("EVENT_POWER_INFO -- Not processed");
                                break;
                        case EVENT_LCD_STATUS:
				dbg("EVENT_LCD_STATUS");
				oa_soap_proc_lcd_status(oh_handler,
						&(event.eventData.lcdStatus));
                                break;
                        case EVENT_LCD_INFO:
                                dbg("EVENT_LCD_INFO -- Not processed");
                                break;
                        case EVENT_REDUNDANCY:
                                dbg("EVENT_REDUNDANCY -- Not processed");
                                break;
                        case EVENT_ILO_DEAD:
				dbg("EVENT_ILO_DEAD");
				oa_soap_proc_server_status(oh_handler,
						oa->event_con2,
						&(event.eventData.bladeStatus));
                                break;
                        case EVENT_RACK_SERVICE_STARTED:
                                dbg("EVENT_RACK_SERVICE_STARTED "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_SCREEN_REFRESH:
                                dbg("EVENT_LCD_SCREEN_REFRESH "
                                    "-- Not processed");
                                break;
                        case EVENT_ILO_ALIVE:
				dbg("EVENT_ILO_ALIVE");
				oa_soap_proc_server_status(oh_handler,
						oa->event_con2,
						&(event.eventData.bladeStatus));
                                break;
                        case EVENT_PERSONALITY_CHECK:
                                dbg("EVENT_PERSONALITY_CHECK -- Not processed");
                                break;

                        case EVENT_BLADE_POST_COMPLETE:
                                dbg("EVENT_BLADE_POST_COMPLETE");
				oa_soap_serv_post_comp(oh_handler,
						       oa->event_con2, 
						       event.numValue);
                                break;

                        case EVENT_BLADE_SIGNATURE_CHANGED:
                                dbg("EVENT_BLADE_SIGNATURE_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_BLADE_PERSONALITY_CHANGED:
                                dbg("EVENT_BLADE_PERSONALITY_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_BLADE_TOO_LOW_POWER:
                                dbg("EVENT_BLADE_TOO_LOW_POWER "
                                    "-- Not processed");
                                break;
                        case EVENT_VIRTUAL_MEDIA_STATUS:
                                dbg("EVENT_VIRTUAL_MEDIA_STATUS "
                                    "-- Not processed");
                                break;
                        case EVENT_MEDIA_DRIVE_INSERTED:
                                dbg("EVENT_MEDIA_DRIVE_INSERTED "
                                    "-- Not processed");
                                break;
                        case EVENT_MEDIA_DRIVE_REMOVED:
                                dbg("EVENT_MEDIA_DRIVE_REMOVED "
                                    "-- Not processed");
                                break;
                        case EVENT_MEDIA_INSERTED:
                                /* EVENT_OA_INFO that arrives later is processed */
                                dbg("EVENT_MEDIA_INSERTED -- Not processed");
                                break;
                        case EVENT_MEDIA_REMOVED:
                                dbg("EVENT_MEDIA_REMOVED -- Not processed");
                                break;
                        case EVENT_OA_NAMES:
                                dbg("EVENT_OA_NAMES -- Not processed");
                                break;
                        case EVENT_OA_STATUS:
				dbg("EVENT_OA_STATUS");
				oa_soap_proc_oa_status(oh_handler,
						&(event.eventData.oaStatus));
                                break;
                        case EVENT_OA_UID:
                                dbg("EVENT_OA_UID -- Not processed");
                                break;

                        case EVENT_OA_INSERTED:
                                dbg("EVENT_OA_INSERTED -- Not processed");
                                break;

                        case EVENT_OA_REMOVED:
                                dbg("EVENT_OA_REMOVED");
                                rv = process_oa_extraction_event(oh_handler,
                                                                 &event);
                                break;

                        case EVENT_OA_INFO:
                                dbg("EVENT_OA_INFO");
                                rv = process_oa_info_event(oh_handler,
                                                           oa->event_con2,
							   &event);
                                break;
                        case EVENT_OA_FAILOVER:
                                dbg("EVENT_OA_FAILOVER");
                                rv = process_oa_failover_event(oh_handler, oa);
                                /* We have done the re-discovery as part of
                                 * FAILOVER event processing.  Ignore the
                                 * events that are recived along with FAILOVER.
                                 */
                                return;
                                break;

                        case EVENT_OA_TRANSITION_COMPLETE:
                                dbg("EVENT_OA_TRANSITION_COMPLETE "
                                    "-- Not processed");
                                break;
                        case EVENT_OA_VCM:
                                dbg("EVENT_OA_VCM -- Not processed");
                                break;
                        case EVENT_NETWORK_INFO_CHANGED:
                                dbg("EVENT_NETWORK_INFO_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_SNMP_INFO_CHANGED:
                                dbg("EVENT_SNMP_INFO_CHANGED -- Not processed");
                                break;
                        case EVENT_SYSLOG_CLEARED:
                                dbg("EVENT_SYSLOG_CLEARED -- Not processed");
                                break;
                        case EVENT_SESSION_CLEARED:
                                dbg("EVENT_SESSION_CLEARED -- Not processed");
                                break;
                        case EVENT_TIME_CHANGE:
                                dbg("EVENT_TIME_CHANGE -- Not processed");
                                break;
                        case EVENT_SESSION_STARTED:
                                dbg("EVENT_SESSION_STARTED -- Not processed");
                                break;
                        case EVENT_BLADE_CONNECT:
                                dbg("EVENT_BLADE_CONNECT -- Not processed");
                                break;
                        case EVENT_BLADE_DISCONNECT:
                                dbg("EVENT_BLADE_DISCONNECT -- Not processed");
                                break;
                        case EVENT_SWITCH_CONNECT:
                                dbg("EVENT_SWITCH_CONNECT -- Not processed");
                                break;
                        case EVENT_SWITCH_DISCONNECT:
                                dbg("EVENT_SWITCH_DISCONNECT -- Not processed");
                                break;
                        case EVENT_BLADE_CLEARED:
                                dbg("EVENT_BLADE_CLEARED -- Not processed");
                                break;
                        case EVENT_SWITCH_CLEARED:
                                dbg("EVENT_SWITCH_CLEARED -- Not processed");
                                break;
                        case EVENT_ALERTMAIL_INFO_CHANGED:
                                dbg("EVENT_ALERTMAIL_INFO_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_LDAP_INFO_CHANGED:
                                dbg("EVENT_LDAP_INFO_CHANGED -- Not processed");
                                break;
                        case EVENT_EBIPA_INFO_CHANGED:
                                dbg("EVENT_EBIPA_INFO_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_HPSIM_TRUST_MODE_CHANGED:
                                dbg("EVENT_HPSIM_TRUST_MODE_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_HPSIM_CERTIFICATE_ADDED:
                                dbg("EVENT_HPSIM_CERTIFICATE_ADDED "
                                    "-- Not processed");
                                break;
                        case EVENT_HPSIM_CERTIFICATE_REMOVED:
                                dbg("EVENT_HPSIM_CERTIFICATE_REMOVED "
                                    "-- Not processed");
                                break;
                        case EVENT_USER_INFO_CHANGED:
                                dbg("EVENT_USER_INFO_CHANGED -- Not processed");
                                break;
                        case EVENT_BAY_CHANGED:
                                dbg("EVENT_BAY_CHANGED -- Not processed");
                                break;
                        case EVENT_GROUP_CHANGED:
                                dbg("EVENT_GROUP_CHANGED -- Not processed");
                                break;
                        case EVENT_OA_REBOOT:
                                dbg("EVENT_OA_REBOOT -- Not processed");
                                break;
                        case EVENT_OA_LOGOFF_REQUEST:
                                dbg("EVENT_OA_LOGOFF_REQUEST -- Not processed");
                                break;
                        case EVENT_USER_ADDED:
                                dbg("EVENT_USER_ADDED -- Not processed");
                                break;
                        case EVENT_USER_DELETED:
                                dbg("EVENT_USER_DELETED -- Not processed");
                                break;
                        case EVENT_USER_ENABLED:
                                dbg("EVENT_USER_ENABLED -- Not processed");
                                break;
                        case EVENT_USER_DISABLED:
                                dbg("EVENT_USER_DISABLED -- Not processed");
                                break;
                        case EVENT_GROUP_ADDED:
                                dbg("EVENT_GROUP_ADDED -- Not processed");
                                break;
                        case EVENT_GROUP_DELETED:
                                dbg("EVENT_GROUP_DELETED -- Not processed");
                                break;
                        case EVENT_LDAPGROUP_ADDED:
                                dbg("EVENT_LDAPGROUP_ADDED -- Not processed");
                                break;
                        case EVENT_LDAPGROUP_DELETED:
                                dbg("EVENT_LDAPGROUP_DELETED -- Not processed");
                                break;
                        case EVENT_LDAPGROUP_ADMIN_RIGHTS_CHANGED:
                                dbg("EVENT_LDAPGROUP_ADMIN_RIGHTS_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_LDAPGROUP_INFO_CHANGED:
                                dbg("EVENT_LDAPGROUP_INFO_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_LDAPGROUP_PERMISSION:
                                dbg("EVENT_LDAPGROUP_PERMISSION "
                                    "-- Not processed");
                                break;
                        case EVENT_LCDPIN:
                                dbg("EVENT_LCDPIN -- Not processed");
                                break;
                        case EVENT_LCD_USER_NOTES_CHANGED:
                                dbg("EVENT_LCD_USER_NOTES_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_BUTTONS_LOCKED:
                                dbg("EVENT_LCD_BUTTONS_LOCKED "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_SCREEN_CHAT_REQUESTED:
                                dbg("EVENT_LCD_SCREEN_CHAT_REQUESTED "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_SCREEN_CHAT_WITHDRAWN:
                                dbg("EVENT_LCD_SCREEN_CHAT_WITHDRAWN "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_SCREEN_CHAT_ANSWERED:
                                dbg("EVENT_LCD_SCREEN_CHAT_ANSWERED "
                                    "-- Not processed");
                                break;
                        case EVENT_LCD_USER_NOTES_IMAGE_CHANGED:
                                dbg("EVENT_LCD_USER_NOTES_IMAGE_CHANGED "
                                    "-- Not processed");
                                break;
                        case EVENT_ENC_WIZARD_STATUS:
                                dbg("EVENT_ENC_WIZARD_STATUS -- Not processed");
                                break;
                        case EVENT_SSHKEYS_INSTALLED:
                                dbg("EVENT_SSHKEYS_INSTALLED -- Not processed");
                                break;
                        case EVENT_SSHKEYS_CLEARED:
                                dbg("EVENT_SSHKEYS_CLEARED -- Not processed");
                                break;
                        case EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_ADDED:
                                dbg("EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_"
                                    "ADDED -- Not processed");
                                break;
                        case EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_REMOVED:
                                dbg("EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_"
                                    "REMOVED -- Not processed");
                                break;
                        case EVENT_BLADE_BOOT_CONFIG:
                                dbg("EVENT_BLADE_BOOT_CONFIG -- Not processed");
                                break;
                        case EVENT_OA_NETWORK_CONFIG_CHANGED:
				dbg("EVENT_OA_NETWORK_CONFIG_CHANGED");
				oa_soap_proc_oa_network_info(oh_handler,
					 &(event.eventData.oaNetworkInfo));
                                break;
                        case EVENT_HPSIM_XENAME_ADDED:
                                dbg("EVENT_HPSIM_XENAME_ADDED "
                                    "-- Not processed");
                                break;
                        case EVENT_HPSIM_XENAME_REMOVED:
                                dbg("EVENT_HPSIM_XENAME_REMOVED "
                                    "-- Not processed");
                                break;
                        case EVENT_FLASH_PENDING:
                                dbg("EVENT_FLASH_PENDING -- Not processed");
                                break;
                        case EVENT_FLASH_STARTED:
                                dbg("EVENT_FLASH_STARTED -- Not processed");
                                break;
                        case EVENT_FLASH_PROGRESS:
                                dbg("EVENT_FLASH_PROGRESS -- Not processed");
                                break;
                        case EVENT_FLASH_COMPLETE:
                                dbg("EVENT_FLASH_COMPLETE -- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_STARTED:
                                dbg("EVENT_STANDBY_FLASH_STARTED "
                                    "-- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_PROGRESS:
                                dbg("EVENT_STANDBY_FLASH_PROGRESS "
                                    "-- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_COMPLETE:
                                dbg("EVENT_STANDBY_FLASH_COMPLETE "
                                    "-- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_BOOTING:
                                dbg("EVENT_STANDBY_FLASH_BOOTING "
                                    "-- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_BOOTED:
                                dbg("EVENT_STANDBY_FLASH_BOOTED "
                                    "-- Not processed");
                                break;
                        case EVENT_STANDBY_FLASH_FAILED:
                                dbg("EVENT_STANDBY_FLASH_FAILED "
                                    "-- Not processed");
                                break;
                        case EVENT_FLASHSYNC_BUILD:
                                dbg("EVENT_FLASHSYNC_BUILD -- Not processed");
                                break;
                        case EVENT_FLASHSYNC_BUILDDONE:
                                dbg("EVENT_FLASHSYNC_BUILDDONE "
                                    "-- Not processed");
                                break;
                        case EVENT_FLASHSYNC_FAILED:
                                dbg("EVENT_FLASHSYNC_FAILED -- Not processed");
                                break;
                        case EVENT_FLASHSYNC_STANDBY_BUILD:
                                dbg("EVENT_FLASHSYNC_STANDBY_BUILD "
                                    "-- Not processed");
                                break;
                        case EVENT_FLASHSYNC_STANDBY_BUILDDONE:
                                dbg("EVENT_FLASHSYNC_STANDBY_BUILDDONE "
                                    "-- Not processed");
                                break;
                        case EVENT_FLASHSYNC_STANDBY_FAILED:
                                dbg("EVENT_FLASHSYNC_STANDBY_FAILED "
                                    "-- Not processed");
                                break;
                        case EVENT_NONILO_EBIPA:
                                dbg("EVENT_NONILO_EBIPA -- Not processed");
                                break;
                        case EVENT_FACTORY_RESET:
                                dbg("EVENT_FACTORY_RESET -- Not processed");
                                break;
                        case EVENT_BLADE_INSERT_COMPLETED:
                                dbg("EVENT_BLADE_INSERT_COMPLETED");
                                rv = process_server_insertion_event(oh_handler,
                                                                 oa->event_con2,
                                                                 &event, loc);
                                break;
                        case EVENT_EBIPA_INFO_CHANGED_EX:
                                dbg("EVENT_EBIPA_INFO_CHANGED_EX "
                                    "-- Not processed");
                                break;
                        default:
                                dbg("EVENT NOT REGISTERED, Event id %d",
                                    event.event);
                }
                /* Get the next event from the eventInfoArray */
                response->eventInfoArray =
                        soap_next_node(response->eventInfoArray);
        }

        return;
}

void * oh_get_event (void *)
                __attribute__ ((weak, alias("oa_soap_get_event")));
