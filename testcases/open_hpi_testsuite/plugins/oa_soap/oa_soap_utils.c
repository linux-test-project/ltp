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
 *      Vivek Kumar <vivek.kumar2@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 * This file implements all the utility functions which will be useful of oa
 * soap functioning. Majority of the functions are helper functions for
 * different modules.
 *
 *      get_oa_soap_info()              - Get active and stand by oa information
 *                                        including IP address
 *
 *      get_oa_state()                  - Get the both oa states and initializes
 *                                        the SOAP_CON objects
 *
 *      update_hotswap_event()          - Updates the Hotswap event structure
 *
 *      copy_oa_soap_event()            - Copies the event data from the event
 *                                        received from oa into the
 *                                        allocated event structure
 *
 *      push_event_to_queue()           - Pushes events into the infrastructure
 *                                        event queue
 *
 *      del_rdr_from_event()            - Delete RDRs from rdrlist of the
 *                                        event structure
 *
 *      check_oa_status()               - Check the oa status and update the oa
 *                                        handler with active OA SOAP_CON
 *
 *      check_oa_user_permissions()     - Check the OA with user permissions
 *                                        level and makes sure plug-in can
 *                                        access all the resources using the
 *                                        supplied user credentials in the
 *                                        conf file
 *
 *      check_discovery_failure()       - Checks reason for discovery failure
 *                                        If the failure is due to 'insufficient
 *                                        priveleges' then creates the fresh
 *                                        oa session
 *
 *      lock_oa_soap_handler()          - Tries to lock the oa_handler mutex.
 *                                        If mutex is already locked earlier,
 *                                        returns error
 *
 *      check_config_parameters()       - Checks whether all the parameters are
 *                                        present in the config file
 *
 *      create_event_session()          - Creates a event session with OA
 *
 *      create_oa_connection()          - Create OA connection after closing the
 *                                        earlier soap_con structures
 *
 *      initialize_oa_con()             - Initialize the hpi_con and event_con
 *
 *      delete_all_inventory_info()     - Frees the memory allocated for
 *                                        inventory areas for all the resources
 *
 *      cleanup_plugin_rptable()        - Frees the memory allocated for
 *                                        plugin RPTable
 *
 *      release_oa_soap_resources()     - Frees the memory allocated for
 *                                        resources presence matrix and serial
 *                                        number array
 *
 *      get_oa_fw_version()             - Gets the Active OA firmware version
 *
 *      update_oa_info()                - Updates the RPT entry with OA
 *                                        firmware version. Updates the serial
 *                                        number array with OA serial number.
 *
 *      convert_lower_to_upper          - Converts the lower case to upper case
 *
 **/

#include "oa_soap_utils.h"

/**
 * get_oa_soap_info
 *      @oh_handler: Pointer to the openhpi handler
 *
 * Purpose:
 *      Gets the Active/standby OA hostname/IP address from the config file
 *      and calls the get_oa_info fucntion
 *
 * Detailed Description:
 *      - Will take the active oa ip from configuration file and will check the
 *        oa state. If active oa is not accessible, we would try the standby oa
 *      - Same above operation will be repeated for the standby oa. If standby
 *        oa is also not accessible, then return error
 *      - Checks whether OA hostname/IP address is not empty string
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - if wrong parameters passed
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT get_oa_soap_info(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T len = 0;
        char *server = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Get the Active OA hostname/IP address and check whether it is NULL */
        server = (char *) g_hash_table_lookup(oh_handler->config, "ACTIVE_OA");
        /* Check whether server string is not empty */
        len = strlen(server);
        if (len != 0) {
                /* Get the OA states and initialize the SOAP_CON structures */
                rv = get_oa_state(oh_handler, server);
                if (rv == SA_OK)
                        return SA_OK;
        } else {
                err("ACTIVE_OA is not provided by the user");
        }

        /* May be user has supplied wrong hostname/IP address
         * or OA is not reachable. Ignore and try with standby OA
         * Get the Standby OA hostname/IP address and check whether it is NULL
         */
        server = (char *) g_hash_table_lookup(oh_handler->config, "STANDBY_OA");
        if (server == NULL) {
                err("STANDBY_OA entry is not present in conf file");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether server string is not empty */
        len = strlen(server);
        if (len == 0) {
                err("STANDBY_OA is not provided by the user");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Get the OA states and initialize the SOAP_CON structures */
        rv = get_oa_state(oh_handler, server);
        if (rv != SA_OK) {
                err("Standby OA - %s may not be accessible", server);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * get_oa_state
 *      @oa_handler: Pointer to OA SOAP handler structure
 *      @server:     Pointer to hostname/IP address of the OA
 *
 * Purpose:
 *      Gets the details of active and standby OA
 *
 * Detailed Description:
 *      - Initializes the SOAP_CON structures
 *      - Using the supplied user credentials from config file, soap connection
 *        is made using soap_open api for both hpi and events receiving
 *      - Will check the user permission on OA using check_user_permission api
 *      - Will get the oa information. Using these info, this api will decide
 *        which oa is active and which one is standby
 *
 * Return values:
 *      SA_OK  - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT get_oa_state(struct oh_handler_state *oh_handler,
                      char *server)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct getOaStatus status;
        struct oaStatus status_response;
        struct getOaInfo info;
        struct oaInfo info_response;
        struct getOaNetworkInfo network_info;
        struct oaNetworkInfo network_info_response;
        enum oaRole oa_role;
        char active_ip[MAX_URL_LEN], standby_ip[MAX_URL_LEN], url[MAX_URL_LEN];
        char active_fm[MAX_BUF_SIZE], standby_fm[MAX_BUF_SIZE];
        char firmware[MAX_BUF_SIZE];
        char *user_name = NULL, *password = NULL;
        SaHpiInt32T i, bay = 0, active_bay = 0, standby_bay = 0;
        SOAP_CON *hpi_con = NULL, *event_con = NULL;
        struct oa_info *this_oa = NULL, *other_oa = NULL;

        if (oh_handler == NULL|| server == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Create the OA URL */
        memset(url, 0, MAX_URL_LEN);
        snprintf(url, strlen(server) + strlen(PORT) + 1, "%s" PORT, server);

        /* Get the user_name and password from config file */
        user_name = (char *) g_hash_table_lookup(oh_handler->config,
                                                 "OA_User_Name");
        password = (char *) g_hash_table_lookup(oh_handler->config,
                                                "OA_Password");

        /* Estabish the connection with OA */
        hpi_con = soap_open(url, user_name, password, HPI_CALL_TIMEOUT);
        if (hpi_con == NULL) {
                err("hpi_con intialization for OA - %s has failed", server);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        event_con = soap_open(url, user_name, password, EVENT_CALL_TIMEOUT);
        if (event_con == NULL) {
                err("event_con intialization for OA - %s has failed", server);
                soap_close(hpi_con);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check whether user_name has admin rights */
        rv = check_oa_user_permissions(oa_handler, hpi_con, user_name);
        if (rv != SA_OK) {
                soap_close(hpi_con);
                soap_close(event_con);
                return rv;
        }

        /* Get the 2 OAs information */
        for (i = 1; i <= MAX_OA_BAYS; i++) {
                status.bayNumber = i;
                rv = soap_getOaStatus(hpi_con, &status, &status_response);
                if (rv != SOAP_OK) {
                        err("Get OA status failed");
                        soap_close(hpi_con);
                        soap_close(event_con);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                oa_role = status_response.oaRole;
                /* Sometimes, if the OA is absent, then OA status is shown as
                 * STANDBY in getOaStatus response.  As workaround, if OA
                 * status is STANDBY and oaRedudancy state is set to false,
                 * then, it is considered as ABSENT.
                 *
                 * But, if the OA is recently inserted, then oaRedudancy state
                 * will be set to false.  In this scenario, the OA state will
                 * be wrongly considered as ABSENT.  This is a known limitation.
                 * TODO: Remove this workaround once the fix is available in
                 * OA firmware.
                 */
                if ((oa_role == OA_ABSENT) ||
                    (oa_role == STANDBY &&
                     status_response.oaRedundancy == HPOA_FALSE))
                        continue;

                info.bayNumber = i;
                rv = soap_getOaInfo(hpi_con, &info, &info_response);
                if (rv != SOAP_OK) {
                        err("Get OA info failed");
                        soap_close(hpi_con);
                        soap_close(event_con);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* 'youAreHere' true indicates that we are talking to this OA.
                 * This helps to find the bay number of the active and standby
                 * OA.
                 */
                if (info_response.youAreHere == HPOA_TRUE) {
                        bay = i;
                       /* Find the oa_info structure for this OA (to which we
                        * are talking to) and the other OA.
                        */
                        switch (bay) {
                                case 1:
                                        this_oa = oa_handler->oa_1;
                                        other_oa = oa_handler->oa_2;
                                        break;
                                case 2:
                                        this_oa = oa_handler->oa_2;
                                        other_oa = oa_handler->oa_1;
                                        break;
                        }
                }

                /* Store the firmware version.
                 * Firmware version is not used.
                 * We may require when we support multiple OA firmwares.
                 */
                memset(firmware, 0, MAX_BUF_SIZE);
                strncpy(firmware, info_response.fwVersion,
                        strlen(info_response.fwVersion));

                network_info.bayNumber = i;
                rv = soap_getOaNetworkInfo(hpi_con, &network_info,
                                           &network_info_response);
                if (rv != SOAP_OK) {
                        err("Get OA network info failed");
                        soap_close(hpi_con);
                        soap_close(event_con);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Find the active and standby bay number, IP address
                 * and firmware version */
                switch (oa_role) {
                        case ACTIVE:
                                active_bay = i;
                                memset(active_ip, 0, MAX_URL_LEN);
                                strncpy(active_ip,
                                        network_info_response.ipAddress,
                                        strlen(network_info_response.
                                                       ipAddress));

                                memset(active_fm, 0, MAX_BUF_SIZE);
                                strncpy(active_fm, firmware, strlen(firmware));
                                break;
                        case STANDBY:
                                standby_bay = i;
                                memset(standby_ip, 0, MAX_URL_LEN);
                                strncpy(standby_ip,
                                        network_info_response.ipAddress,
                                        strlen(network_info_response.
                                                       ipAddress));

                                memset(standby_fm, 0, MAX_BUF_SIZE);
                                strncpy(standby_fm, firmware, strlen(firmware));
                                break;
                        default:
                                err("wrong oa state detected for bay %d", i);
                                soap_close(hpi_con);
                                soap_close(event_con);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                }
        }

        if (active_bay != 0) {
                dbg("\tActive OA bay number - %d", active_bay);
                dbg("\tActive OA ip address - %s", active_ip);
                dbg("\tActive OA firmware version - %s", active_fm);
        }
        if (standby_bay != 0) {
                dbg("\tStandby OA bay number - %d", standby_bay);
                dbg("\tStandby OA ip address - %s", standby_ip);
                dbg("\tStandby OA firmware version - %s", standby_fm);
        }

        /* Get the status and firmware version of the OA which we are
         * talking to
         */
        if (bay == active_bay) {
                this_oa->oa_status = ACTIVE;
                this_oa->fm_version = atof(active_fm);
        } else { /* bay == standby_bay */
                this_oa->oa_status = STANDBY;
                this_oa->fm_version = atof(standby_fm);
        }

        /* Initialize the hpi_con and event_con structures */
        this_oa->hpi_con = hpi_con;
        this_oa->event_con = event_con;
        memset(this_oa->server, 0, MAX_URL_LEN);
        strncpy(this_oa->server, server, strlen(server));

        /* Check whether 2 OAs are present or not */
        if (active_bay == 0 || standby_bay == 0)
                return SA_OK;

        memset(url, 0, MAX_URL_LEN);
        memset(other_oa->server, 0, MAX_URL_LEN);
        /* Construct the other OA url and copy the IP address to oa_info
         * structure
         */
        if (bay == standby_bay) {
                other_oa->oa_status = ACTIVE;
                other_oa->fm_version = atof(active_fm);
                strncpy(other_oa->server, active_ip, strlen(active_ip));
                snprintf(url, strlen(active_ip) + strlen(PORT) + 1,
                         "%s" PORT, active_ip);
        } else {
                other_oa->oa_status = STANDBY;
                other_oa->fm_version = atof(standby_fm);
                strncpy(other_oa->server, standby_ip, strlen(standby_ip));
                snprintf(url, strlen(standby_ip) + strlen(PORT) + 1,
                         "%s" PORT, standby_ip);
        }

        /* Initialize the soap_con for hpi and event thread */
        other_oa->hpi_con = soap_open(url, user_name,
                                      password, HPI_CALL_TIMEOUT);
        if (other_oa->hpi_con == NULL) {
                err("Initializing the hpi_con for OA %s failed", url);
                /* If this OA status is ACTIVE, then return error, else ignore
                 * If standby OA is not accessible, then the recovery from
                 * this problem will be done by the event thread.
                 * Since we have access to Active OA, we should ignore this
                 * for time being
                 */
                if (other_oa->oa_status == ACTIVE) {
                        soap_close(this_oa->hpi_con);
                        soap_close(this_oa->event_con);
                        this_oa->hpi_con = NULL;
                        this_oa->event_con = NULL;
                        err("Active OA - %s may not be accessible",
                            other_oa->server);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                } else
                        return SA_OK;
            }

        other_oa->event_con = soap_open(url, user_name,
                                        password, EVENT_CALL_TIMEOUT);
        if (other_oa->event_con == NULL) {
                err("Initializing the event_con for OA %s failed", url);
                /* If this OA status is ACTIVE, then return error, else
                 * ignore
                 */
                if (other_oa->oa_status != ACTIVE) {
                        soap_close(this_oa->hpi_con);
                        soap_close(this_oa->event_con);
                        this_oa->hpi_con = NULL;
                        this_oa->event_con = NULL;
                        soap_close(other_oa->hpi_con);
                        other_oa->hpi_con = NULL;
                        err("Active OA - %s may not be accessible",
                            other_oa->server);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                } else {
                        soap_close(other_oa->hpi_con);
                        other_oa->hpi_con = NULL;
                        return SA_OK;
                }
        }

        return SA_OK;
}

/**
 * update_hotswap_event
 *      @event:      Pointer to openhpi event structure
 *      @oh_handler: Pointer to the openhpi handler
 *
 * Purpose:
 *      this api updates the event structure with hotswap details
 *
 * Detailed Description: NA
 *
 * Return values:
 *      NONE
 **/

void update_hotswap_event(struct oh_handler_state *oh_handler,
                          struct oh_event *event)
{
        if (oh_handler == NULL || event == NULL) {
                err("Invalid parameters");
                return;
        }

        memset(event, 0, sizeof(struct oh_event));
        event->hid = oh_handler->hid;
        event->event.EventType = SAHPI_ET_HOTSWAP;
        /* TODO: map the timestamp of the OA generated event */
        oh_gettimeofday(&(event->event.Timestamp));
        event->event.Severity = SAHPI_CRITICAL;
}

/**
 * copy_oa_soap_event
 *      @event: Pointer to the openhpi event structure
 *
 * Purpose:
 *      makes a copy of the received event
 *
 * Detailed Description:
 *      - Allocates the memory to new event structure.
 *      - Copies the hpi event to the newly created event structure
 *      - Returns the newly created event structure
 *
 * Return values:
 *      Pointer to copied oh_event structure - on success
 *      NULL                                 - if wrong parameters passed
 *                                           - if the memory allocation failed.
 **/

struct oh_event *copy_oa_soap_event(struct oh_event *event)
{
        struct oh_event *e;

        if (event == NULL) {
                err("Invalid parameter");
                return NULL;
        }

        e = (struct oh_event *)g_malloc0(sizeof(struct oh_event));
        if (e == NULL) {
                err("Out of memory!");
                return NULL;
        }
        memcpy(e, event, sizeof(struct oh_event));
        return e;
}

/**
 * del_rdr_from_event
 *      @event: Pointer to the openhpi event structure
 *
 * Purpose:
 *      deletes the RDR from the event.
 *
 * Detailed Description:
 *      - traverse the event.rdrs list and delete the RDRs one by one.
 *      - This function will be called if the discovery fails for a resource
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - if wrong parameters passed
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT del_rdr_from_event(struct oh_event *event)
{
        GSList *node = NULL;
        SaHpiRdrT *rdr = NULL;

        if (event == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (event->rdrs == NULL)
                return SA_OK;

        /* Traverse the RDRs list and delete the RDR */
        node = event->rdrs;
        do {
                rdr = NULL;
                rdr = (SaHpiRdrT *)node->data;
                if (rdr == NULL) {
                        err("Wrong node detected in the GSList");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
                /* Pop out the RDR from the RDRs list  */
                event->rdrs = g_slist_remove(event->rdrs, (gpointer)rdr);
                g_free(rdr);
                /* Get the next RDR */
                node = event->rdrs;
        } while (node != NULL);

        return SA_OK;
}

/**
 * check_oa_status
 *      @oa_handler: Pointer to the OA SOAP plug-in handler
 *      @oa:         Pointer to the OA data structure
 *      @con:        Pointer to the SOAP_CON structure
 *
 * Purpose:
 *      checks the status of the oa
 *
 * Detailed Description:
 *      - gets the status of the OA
 *      - Updates the OA handler with active OA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - if wrong parameters passed
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT check_oa_status(struct oa_soap_handler *oa_handler,
                         struct oa_info *oa,
                         SOAP_CON *con)
{
        SaErrorT rv = SA_OK;
        struct getOaStatus status;
        struct oaStatus status_response;

        if (oa_handler == NULL || oa == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Get the bay number of the OA */
        if (oa == oa_handler->oa_1)
                status.bayNumber = 1;
        else
                status.bayNumber = 2;

        g_mutex_lock(oa->mutex);
        rv = soap_getOaStatus(con, &status, &status_response);
        if (rv != SOAP_OK) {
                err("Get OA status call failed");
                g_mutex_unlock(oa->mutex);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        if (status_response.oaRole == TRANSITION) {
                /* OA is in transitioning state to become active.
                 * This is a very rare scenario.
                 *
                 * Wait till it it becomes Active
                 */
                err("OA is in transition state");
                sleep(OA_STABILIZE_MAX_TIME);
                rv = soap_getOaStatus(con, &status, &status_response);
                if (rv != SOAP_OK) {
                        err("Get OA status call failed");
                        g_mutex_unlock(oa->mutex);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
                /* Check OA is still in TRANSITION state
                 * Ideally, OA should be out of TRANSITION state
                 */
                if (status_response.oaRole == TRANSITION) {
                        err("OA is in TRANSITION for a long time");
                        err("Please correct the OA");
                        g_mutex_unlock(oa->mutex);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                 }
        }

        oa->oa_status = status_response.oaRole;
        if (oa->oa_status == ACTIVE) {
                g_mutex_unlock(oa->mutex);
                /* Always lock the oa_handler mutex and then oa_info mutex
                 * This is to avoid the deadlock
                 */
                g_mutex_lock(oa_handler->mutex);
                g_mutex_lock(oa->mutex);
                /* Point the active_con to Active OA's hpi_con */
                if (oa_handler->active_con != oa->hpi_con) {
                        oa_handler->active_con = oa->hpi_con;
                        err("OA %s has become Active", oa->server);
                }
                g_mutex_unlock(oa->mutex);
                g_mutex_unlock(oa_handler->mutex);
        } else
                g_mutex_unlock(oa->mutex);

        return SA_OK;
}

/**
 * check_oa_user_permissions
 *      @oa_handler: Pointer to the oa_handler structure
 *      @con:        Pointer to the SOAP CON
 *      @user_name:  Pointer to the user name string
 *
 * Purpose:
 *      check oa user permissions, even for the oa administrator.
 *
 * Detailed Description:
 *      - checks whether OA user has the ADMINISTRATOR permission
 *        and can access all the resources in HP BladeSystem c-Class
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - if wrong parameters passed
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT check_oa_user_permissions(struct oa_soap_handler *oa_handler,
                                   SOAP_CON *con,
                                   char *user_name)
{
        SaErrorT rv = SA_OK;
        struct getUserInfo request;
        struct userInfo response;
        struct bayAccess bay_access;

        if (oa_handler == NULL || con == NULL || user_name == NULL) {
                err("Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        memset(&request, 0, sizeof(struct getUserInfo));
        request.username = user_name;

        rv = soap_getUserInfo(con, &request, &response);
        if (rv != SOAP_OK) {
                err("Get user info call failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check user name is enabled or not
         * This should ideally never fail.
         * We can not have a user name which is disabled and session is created
         */
        if (response.isEnabled != HPOA_TRUE) {
                err("User - %s is not enabled for OA %s",
                    user_name, con->server);
                err("Please give full permissions to user - %s", user_name);
                oa_handler->status = PLUGIN_NOT_INITIALIZED;
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check for the ADMINISTRATOR rights */
        if (response.acl != ADMINISTRATOR) {
                err("User - %s does not have Administrator rights for OA %s",
                     user_name, con->server);
                err("Please give full permissions to user - %s", user_name);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check the permissions for accessing OA */
        if (response.bayPermissions.oaAccess != HPOA_TRUE) {
                err("User - %s does not have access rights to OA bay(s) "
                    "for OA %s", user_name, con->server);
                err("Please give full permissions to user - %s", user_name);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check the permissions for accessing server lades */
        while (response.bayPermissions.bladeBays) {
                soap_getBayAccess(response.bayPermissions.bladeBays,
                                  &bay_access);
                if (bay_access.access != HPOA_TRUE) {
                        err("User - %s does not have access rights to "
                            "server bay(s) for OA - %s",
                            user_name, con->server);
                        err("Please give full permissions to user - %s",
                            user_name);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
                response.bayPermissions.bladeBays =
                          soap_next_node(response.bayPermissions.bladeBays);
        }

        /* Check the permissions for accessing interconnect */
        while (response.bayPermissions.interconnectTrayBays) {
                soap_getBayAccess(response.bayPermissions.interconnectTrayBays,
                                   &bay_access);
                if (bay_access.access != HPOA_TRUE) {
                        err("User - %s does not have access rights to "
                            "interconnect bay(s) for OA %s",
                            user_name, con->server);
                        err("Please give full permissions to user - %s",
                            user_name);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
                response.bayPermissions.interconnectTrayBays =
                        soap_next_node(response.bayPermissions.
                                               interconnectTrayBays);
        }

        return SA_OK;
}

/**
 * check_discovery_failure
 *      @oh_handler: Pointer to the openhpi handler
 *
 * Purpose:
 *      checks the oa status after the discovery failure.
 *
 * Detailed Description:
 *      - this api will be called only during the discovery failure.
 *      - checks the reason for discovery failure.
 *      - if the failure is due to 'insufficient priveleges',
 *        then creates the fresh oa session
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - if wrong parameters passed
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT check_discovery_failure(struct oh_handler_state *oh_handler)
{
        SaErrorT oa1_rv, oa2_rv;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        /* Initialize the return values with failure case */
        oa1_rv = SA_ERR_HPI_INTERNAL_ERROR;
        oa2_rv = SA_ERR_HPI_INTERNAL_ERROR;

        /* If the hpi_con is NULL, then OA is not reachable */
        if (oa_handler->oa_1->hpi_con != NULL) {
                /* Get the status of the OA in slot 1 */
                oa1_rv = check_oa_status(oa_handler, oa_handler->oa_1,
                                         oa_handler->oa_1->hpi_con);
                if (oa1_rv != SA_OK)
                        err("check oa_status has failed for - %s",
                             oa_handler->oa_1->server);
        }

        if (oa_handler->oa_2->hpi_con != NULL) {
                /* Get the status of the OA in slot 2 */
                oa2_rv = check_oa_status(oa_handler, oa_handler->oa_2,
                                         oa_handler->oa_2->hpi_con);
                if (oa2_rv != SA_OK)
                        err("check oa_status has failed for OA - %s",
                             oa_handler->oa_2->server);
        }

        /* If the OA is reachable (check_oa_status call succeeded)
         * and OA STATUS is ACTIVE, then return sucess, else return failure.
         */
        if (oa1_rv == SA_OK && oa_handler->oa_1->oa_status == ACTIVE)
                return SA_OK;
        else if (oa2_rv == SA_OK && oa_handler->oa_2->oa_status == ACTIVE)
                return SA_OK;
        else
                return SA_ERR_HPI_INTERNAL_ERROR;
}

/**
 * lock_oa_soap_handler
 *      @oa_handler: Pointer to the oa_handler
 *
 * Purpose:
 *      Tries to lock the oa_handler mutex. If mutex is already locked earlier,
 *      returns error
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on invalid parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT lock_oa_soap_handler(struct oa_soap_handler *oa_handler)
{
        gboolean lock_state = TRUE;

        if (oa_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Try to lock the oa_handler mutex */
        lock_state = g_mutex_trylock(oa_handler->mutex);
        if (lock_state == FALSE) {
                err("OA SOAP Handler is locked.");
                err("No operation is allowed in this state");
                err("Please try after some time");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Unlock the oa_handler mutex */
        g_mutex_unlock(oa_handler->mutex);
        return SA_OK;
}

/**
 * check_config_parameters
 *      @handler_config: Pointer to the config handler
 *
 * Purpose:
 *      Checks whether all the parameters are present in the config file
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on invalid parameters.
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/

SaErrorT check_config_parameters(GHashTable *handler_config)
{
        char *temp = NULL;

        if (handler_config == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Check for entity_root entry */
        temp = (char *)g_hash_table_lookup(handler_config, "entity_root");
        if (temp == NULL) {
                err("entity_root is missing in the config file.");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check for OA user name entry */
        temp = (char *) g_hash_table_lookup(handler_config, "OA_User_Name");
        if (temp == NULL) {
                err("Failed to find attribute OA_User_Name in openhpi.conf ");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check for OA_Password entry */
        temp = (char *) g_hash_table_lookup(handler_config, "OA_Password");
        if (temp == NULL) {
                err("Failed to find attribute OA_Password in openhpi.conf ");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Check for Active OA hostname/IP address entry
         * STANDBY_OA is an optional parameter and hence not checked
         */
        temp = (char *) g_hash_table_lookup(handler_config, "ACTIVE_OA");
        if (temp == NULL) {
                err("Failed to find attribute ACTIVE_OA in openhpi.conf ");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        return SA_OK;
}

/**
 * create_event_session
 *      @oa: Pointer to the oa info structure
 *
 * Purpose:
 *      creates the fresh event session with OA
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on unsupported oa events.
 **/
SaErrorT create_event_session(struct oa_info *oa)
{
        SaErrorT rv = SOAP_OK;
        struct eventPid pid;

        if (oa == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        g_mutex_lock(oa->mutex);
        if (oa->event_con == NULL) {
                dbg("OA may not be accessible");
                g_mutex_unlock(oa->mutex);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = soap_subscribeForEvents(oa->event_con, &pid);
        g_mutex_unlock(oa->mutex);
        if (rv != SOAP_OK) {
                err("Subscribe for events failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Store the event pid in oa_info structure */
        oa->event_pid=pid.pid;

        return SA_OK;
}

/**
 * create_oa_connection
 *      @oa:        Pointer to the oa info structure
 *      @user_name: Pointer the to the OA user name
 *      @password:  Pointer the to the OA password
 *
 * Purpose:
 *      Creates the connection with the OA
 *      This function will not return until the connection is established
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      NONE
 **/
void create_oa_connection(struct oa_soap_handler *oa_handler,
                          struct oa_info *oa,
                          char *user_name,
                          char *password)
{
        SaErrorT rv = SA_OK;
        SaHpiBoolT is_oa_present = SAHPI_FALSE;
        SaHpiBoolT is_oa_accessible = SAHPI_FALSE;

        if (oa == NULL || user_name == NULL || password == NULL) {
                err("Invalid parameters");
                return;
        }

        while (is_oa_accessible == SAHPI_FALSE) {
                OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                /* Check whether the OA is present.
                 * If not, wait till the OA is inserted
                 */
                 is_oa_present = SAHPI_FALSE;
                 while (is_oa_present == SAHPI_FALSE) {
                	OA_SOAP_CHEK_SHUTDOWN_REQ(oa_handler, NULL, NULL, NULL);
                        g_mutex_lock(oa->mutex);
                        if (oa->oa_status != OA_ABSENT) {
                                g_mutex_unlock(oa->mutex);
                                is_oa_present = SAHPI_TRUE;
                        } else {
                                g_mutex_unlock(oa->mutex);
                                /* OA is not present,
                                 * wait for 30 seconds and check again
                                 */
                                sleep(30);
                        }
                }

                g_mutex_lock(oa->mutex);
                /* Close the soap_con strctures */
                if (oa->hpi_con != NULL) {
                        soap_close(oa->hpi_con);
                        oa->hpi_con = NULL;
                }
                if (oa->event_con != NULL) {
                        soap_close(oa->event_con);
                        oa->event_con = NULL;
                }
                g_mutex_unlock(oa->mutex);

                rv = initialize_oa_con(oa, user_name, password);
                if ((rv != SA_OK) && (oa->oa_status != OA_ABSENT)) {
                        /* OA may not be reachable
                         * wait for 2 seconds and check again
                         */
                        sleep(2);
                        continue;
                }

                /* hpi_con and event_con successfully created */
                is_oa_accessible = SAHPI_TRUE;
        }

        return;
}

/**
 * initialize_oa_con
 *      @oa:        Pointer to the oa info structure
 *      @user_name: Pointer the to the OA user name
 *      @password:  Pointer the to the OA password
 *
 * Purpose:
 *      Initializes the hpi_con and event_con
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 *      NONE
 **/
SaErrorT initialize_oa_con(struct oa_info *oa,
                           char *user_name,
                           char *password)
{
        char url[MAX_URL_LEN];

        if (oa == NULL || user_name == NULL || password == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        g_mutex_lock(oa->mutex);
        memset(url, 0, MAX_URL_LEN);
        snprintf(url, strlen(oa->server) + strlen(PORT) + 1,
                 "%s" PORT, oa->server);

        oa->hpi_con = soap_open(url, user_name, password,
                                HPI_CALL_TIMEOUT);
        if (oa->hpi_con == NULL) {
                /* OA may not be reachable */
                g_mutex_unlock(oa->mutex);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Try to create event_con connection
         * Ideally, this call should not fail
         */
        oa->event_con = soap_open(url, user_name, password,
                                  EVENT_CALL_TIMEOUT);
        if (oa->event_con == NULL) {
                /* OA may not be reachable */
                g_mutex_unlock(oa->mutex);
                soap_close(oa->hpi_con);
                oa->hpi_con = NULL;
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        g_mutex_unlock(oa->mutex);

        return SA_OK;

}

/**
 * delete_all_inventory_info
 *      @oh_handler: Pointer to the plugin handler
 *
 * Purpose:
 *      Traverses through all resources and extracts the inventory RDR
 *      Frees up the memory allocated for inventory information
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 **/
SaErrorT delete_all_inventory_info(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT  *rpt = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_next(oh_handler->rptcache, SAHPI_FIRST_ENTRY);
        while (rpt) {
                if (rpt->ResourceCapabilities 
                    & SAHPI_CAPABILITY_INVENTORY_DATA) {
                        /* Free the inventory info from inventory RDR */
                        rv = free_inventory_info(oh_handler, rpt->ResourceId);
                        if (rv != SA_OK) 
                                err("Inventory cleanup failed for resource %d",
                                    rpt->ResourceId);
                }
                /* Get the next resource */
                rpt = oh_get_resource_next(oh_handler->rptcache,
                                           rpt->ResourceId);
        }

        return SA_OK;
}

/**
 * cleanup_plugin_rptable
 *      @oh_handler: Pointer to the plugin handler
 *
 * Purpose:
 *      Frees up the memory allocated for RPT and RDR entries
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      NONE
 **/
void cleanup_plugin_rptable(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        if(oa_handler == NULL) {
                err("Invalid parameter");
                return;
        }

        rv = delete_all_inventory_info(oh_handler);
        if (rv != SA_OK) {
                err("Deleting all inventory information failed");
        }

        release_oa_soap_resources(oa_handler);

        rv = oh_flush_rpt(oh_handler->rptcache);
        if (rv != SA_OK) {
                err("Plugin RPTable flush failed");
        }

        return;
}

/**
 * release_oa_soap_resources
 *      @oa_handler: Pointer to oa soap handler
 *
 * Purpose:
 *      To free the memory allocated for resource presence and serial number
 *      for OA, interconnect, server, fan and power supply
 *
 * Detailed Description: NA
 *
 * Return values:
 *      None
 **/
void release_oa_soap_resources(struct oa_soap_handler *oa_handler)
{
        SaHpiInt32T i;

        /* Release memory of blade presence, resource id and blade
         * serial number arrays
         */
        if (oa_handler->oa_soap_resources.server.presence != NULL) {
                g_free(oa_handler->oa_soap_resources.server.presence);
        }
        if (oa_handler->oa_soap_resources.server.resource_id != NULL) {
                g_free(oa_handler->oa_soap_resources.server.resource_id);
        }
	if(oa_handler->oa_soap_resources.server.serial_number != NULL) {
	    for (i = 0; i < oa_handler->oa_soap_resources.server.max_bays; i++)
	    {
		if (oa_handler->oa_soap_resources.server.serial_number[i] !=
                    NULL) {
                        g_free(oa_handler->oa_soap_resources.server.
                               serial_number[i]);
		}
            }
            g_free(oa_handler->oa_soap_resources.server.serial_number);
	}

        /* Release memory of interconnect presence and serial number array */
        if (oa_handler->oa_soap_resources.interconnect.presence != NULL) {
                g_free(oa_handler->oa_soap_resources.interconnect.presence);
        }
        if (oa_handler->oa_soap_resources.interconnect.resource_id != NULL) {
                g_free(oa_handler->oa_soap_resources.interconnect.resource_id);
        }
	if(oa_handler->oa_soap_resources.interconnect.serial_number != NULL) {
            for (i = 0; i < oa_handler->oa_soap_resources.interconnect.max_bays;
             i++) {
                if (oa_handler->oa_soap_resources.interconnect.
                    serial_number[i] != NULL) {
                        g_free(oa_handler->oa_soap_resources.interconnect.
                               serial_number[i]);
		}
            }
            g_free(oa_handler->oa_soap_resources.interconnect.serial_number);
	}

        /* Release memory of OA presence and serial number array */
        if (oa_handler->oa_soap_resources.oa.presence != NULL) {
                g_free(oa_handler->oa_soap_resources.oa.presence);
        }
        if (oa_handler->oa_soap_resources.oa.resource_id != NULL) {
                g_free(oa_handler->oa_soap_resources.oa.resource_id);
        }
	if(oa_handler->oa_soap_resources.oa.serial_number != NULL) {
            for (i = 0; i < oa_handler->oa_soap_resources.oa.max_bays; i++) {
                if (oa_handler->oa_soap_resources.oa.serial_number[i] != NULL) {
                        g_free(oa_handler->oa_soap_resources.oa.
                               serial_number[i]);
		}
            }
            g_free(oa_handler->oa_soap_resources.oa.serial_number);
	}

        /* Release memory of fan presence.  Since fans do not have serial
         * numbers, a serial numbers array does not need to be released.
         */
        if (oa_handler->oa_soap_resources.fan.presence != NULL) {
                g_free(oa_handler->oa_soap_resources.fan.presence);
        }
        if (oa_handler->oa_soap_resources.fan.resource_id != NULL) {
                g_free(oa_handler->oa_soap_resources.fan.resource_id);
        }

        /* Release memory of fan zone resource id */
        if (oa_handler->oa_soap_resources.fan_zone.resource_id != NULL) {
                g_free(oa_handler->oa_soap_resources.fan_zone.resource_id);
        }

        /* Release memory of power supply presence and serial number array */
        if (oa_handler->oa_soap_resources.ps_unit.presence !=NULL) {
                g_free(oa_handler->oa_soap_resources.ps_unit.presence);
        }
        if (oa_handler->oa_soap_resources.ps_unit.resource_id !=NULL) {
                g_free(oa_handler->oa_soap_resources.ps_unit.resource_id);
        }
	if(oa_handler->oa_soap_resources.ps_unit.serial_number != NULL) {
            for (i = 0; i < oa_handler->oa_soap_resources.ps_unit.max_bays; i++)
	    {
                if (oa_handler->oa_soap_resources.ps_unit.serial_number[i]
                    != NULL) {
                        g_free(oa_handler->oa_soap_resources.
                                ps_unit.serial_number[i]);
		}
            }
            g_free(oa_handler->oa_soap_resources.ps_unit.serial_number);
	}
}

/**
 * get_oa_fw_version
 *      @oh_handler: Pointer to the plugin handler
 *
 * Purpose:
 *      Returns the OA firmware version of the active OA
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      Active OA firmware version - on success
 *      0.0                        - on failure
 **/
SaHpiFloat64T get_oa_fw_version(struct oh_handler_state *oh_handler)
{
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return 0.0;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        if (oa_handler->active_con == oa_handler->oa_1->hpi_con)
                return oa_handler->oa_1->fm_version;
        else if (oa_handler->active_con == oa_handler->oa_2->hpi_con)
                return oa_handler->oa_2->fm_version;
        else
                return 0.0;
}

/**
 * update_oa_info
 *      @oh_handler:  Pointer to the plugin handler
 *      @response:    Pointer to the oaInfo structure
 *      @resource_id: Resource Id
 *
 * Purpose:
 *      Returns the OA firmware version of the active OA
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      SA_HPI_ERR_INAVLID_PARAMS - on invalid parametersfailure
 *      SA_HPI_ERR_INTERNAL_ERROR - on failure
 *      SA_OK                     - on success
 **/
SaErrorT update_oa_info(struct oh_handler_state *oh_handler,
                        struct oaInfo *response,
                        SaHpiResourceIdT resource_id)
{
        SaHpiRptEntryT *rpt = NULL;
        SaHpiFloat64T fm_version;
        SaHpiInt32T major;

        if (oh_handler == NULL || response == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("OA rpt is not present");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        if (strlen(response->fwVersion) == 0) {
                err("Firmware version is null string");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Firmware version is in the format of x.yy. 'x' is the major version
         * 'yy' is the minor version
         */
        fm_version = atof(response->fwVersion);
        rpt->ResourceInfo.FirmwareMajorRev = major = rintf(fm_version);
        rpt->ResourceInfo.FirmwareMinorRev = rintf((fm_version - major) * 100);

        return SA_OK;
}

/**
 * convert_lower_to_upper
 *      @src:      Pointer to the source string handler
 *      @src_len:  String length of the source string
 *      @dest:     Pointer to destination string
 *      @dest_len: Length of the destination string
 *
 * Purpose:
 *      Converts the lower case characters to upper case
 *
 * Detailed Descrption: NA
 *
 * Return values:
 *      SA_HPI_ERR_INAVLID_PARAMS - on invalid parametersfailure
 *      SA_HPI_ERR_INTERNAL_ERROR - on failure
 *      SA_OK                     - on success
 **/

SaErrorT convert_lower_to_upper(char *src,
                                SaHpiInt32T src_len,
                                char *dest,
                                SaHpiInt32T dest_len)
{
        SaHpiInt32T i;

        if (src == NULL || dest == NULL) {
                dbg("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (dest_len < src_len) {
                err("Source string is longer than destination string");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        memset(dest, 0, dest_len);
        for (i = 0; i < src_len; i++)
                dest[i] = toupper(src[i]);

        return SA_OK;
}

/**
 * update_reseource_status()
 *      @res_status     pointer to resource_status_t
 *      @index          index into the resource info fields in res_status
 *      @serial_number  serial_number string to be copied into res_status
 *      @resource_id    resource id to be updated to res_status
 *      @presence       presence status
 *
 *      Description:
 *      This routine updates the resource status entry with passed in
 *      serial_number, resource_id, and presence.  This routine should be
 *      called to set and reset the resource status fields that change
 *      when a a resource gets added and removed.
 *
 *      Return value: none
**/
void oa_soap_update_resource_status(resource_status_t *res_status,
                                    SaHpiInt32T index,
                                    char *serial_number,
                                    SaHpiResourceIdT resource_id,
                                    resource_presence_status_t presence)
{
        if (index <= 0) {
                err("Invalid index value %d - returning without update\n",
                    index);
                return;
        }
        if (serial_number != NULL) {
                size_t len;

                len = strlen(serial_number);
                strncpy(res_status->serial_number[index-1], serial_number, len);
                res_status->serial_number[index-1][len] = '\0';
        }
        res_status->resource_id[index-1] = resource_id;
        res_status->presence[index-1] = presence;

        return;
}

