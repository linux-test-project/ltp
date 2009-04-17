/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
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
 *      Raghavendra M.S. <raghavendra.ms@hp.com>
 *      Shuah Khan <shuah.khan@hp.com>
 *
 * This file implements the discovery functionality. The resources of the
 * HP BladeSystem c-Class are discovered.
 *
 *      oa_soap_discover_resources()    - Checks the plugin initialization
 *                                        completion.  Starts the event threads
 *                                        for active and standby OAs. Starts
 *                                        the discovery.
 *
 *      discover_oa_soap_system()       - Discovers all the resources in HP
 *                                        BladeSystem c-Class
 *
 *      build_enclosure_info()          - Discovers the max bays for all the
 *                                        resources in enclosure
 *
 *      build_enclosure_rpt()           - Builds the enclosure RPT entry
 *
 *      build_enclosure_rdr()           - Builds the enclosure RDRs
 *
 *      discover_enclosure()            - Discovers the enclosure along with
 *                                        its capabilities
 *
 *      build_server_rpt()              - Builds the server RPT entry
 *
 *      build_discovered_server_rpt()   - Builds the server RPT entry
 *
 *      build_server_rdr()              - Builds the server RDRs
 *
 *      discover_server()               - Discovers the server, IO, and
 *                                        storage blades, and their
 *                                        capabilities
 *
 *      discover_interconnect_rpt()     - Builds the interconnect RPT entry
 *
 *      discover_interconnect_rdr()     - Builds the interconnect RDRs
 *
 *      discover_interconnect()         - Discovers the interconnect blades
 *                                        along with their capabilities
 *
 *      oa_soap_discover_fan_rpt()      - Builds the fan RPT entry
 *
 *      oa_soap_discover_fan_rdr()      - Builds the fan RDRs
 *
 *      oa_soap_discover_fan()          - Discovers the fan along with
 *                                        its capabilities
 *
 *      discover_power_subsystem_rpt()  - Builds the power subsystem RPT entry
 *
 *      discover_power_subsystem_rdr()  - Builds the power subsystem RDRs
 *
 *      discover_power_subsystem()      - Discovers the power subsystem
 *                                        along with its capabilities
 *
 *      discover_power_supply_rpt()     - Builds the power supplies RPT entry
 *
 *      discover_power_supply_rdr()     - Builds the power supplies RDRs
 *
 *      discover_power_supply()         - Discovers the power supplies
 *                                        along with its capabilities
 *
 *      discover_oa_rpt()               - Builds the onboard administrator
 *                                        RPT entry
 *
 *      discover_oa_rdr()               - Builds the onboard administrator RDRs
 *
 *      discover_oa()                   - Discovers the onboard administrator
 *                                        along with its capabilities
 *
 *	oa_soap_parse_diag_ex()		- Parses the diagnosticChecksEx
 *					  structure
 *
 *	oa_soap_get_health_val()	- Gets the healthStatus value from
 * 					  extraData structure
 *
 *	oa_soap_build_rpt()		- Generic function to build the RPT
 *					  entry
 *
 *	oa_soap_build_therm_subsys_rdr()- Builds the thermal subsystem RDR
 *
 *	oa_soap_disc_therm_subsys()	- Discovers the thermal subsystem
 *
 *	oa_soap_build_fz_rdr()		- Builds the fan zone RDR
 *
 *	oa_soap_get_fz_arr()		- Gets the fan zone array information
 *					  from OA
 *
 *	oa_soap_disc_fz()		- Discovers the fan zones
 *
 *	oa_soap_build_lcd_rdr()		- Builds the LCD RDR
 *
 *	oa_soap_disc_lcd()		- Discovers the LCD
 *
 *	oa_soap_populate_event()	- Populates the event structure with
 *					  default values
 *
 *	oa_soap_push_disc_res()		- Pushes the discovered resources
 *					  information to openhpi framework
 *
 *	oa_soap_build_blade_thermal_rdr() - Builds or Enables the thermal 
 * 					    sensors of blade resource 
 *
 */

#include "oa_soap_discover.h"

/* Forward declaration for static functions */
static SaErrorT oa_soap_build_enc_info(struct oh_handler_state *oh_handler,
				       struct enclosureInfo *info);
static SaErrorT oa_soap_build_therm_subsys_rdr(struct oh_handler_state
						*oh_handler,
					       SaHpiResourceIdT resource_id);
static SaErrorT oa_soap_build_fz_rdr(struct oh_handler_state *oh_handler,
				     SaHpiResourceIdT resource_id,
				     struct fanZone *fan_zone);
static SaErrorT oa_soap_disc_therm_subsys(struct oh_handler_state *oh_handler);
static SaErrorT oa_soap_disc_fz(struct oh_handler_state *oh_handler);
static SaErrorT oa_soap_disc_fan(struct oh_handler_state *oh_handler);
static SaErrorT oa_soap_build_lcd_rdr(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id);
static SaErrorT oa_soap_disc_lcd(struct oh_handler_state *oh_handler);
static void oa_soap_push_disc_res(struct oh_handler_state *oh_handler);

/**
 * oa_soap_discover_resources
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the resources.
 *      This ABI is called from the OpenHPI framework for
 *      discovering HP BladeSystem cClass resources
 *
 * Detailed Description:
 *      - Checks the plugin initialization completion.
 *        If the plugin initialization has failed, then it tries to do the
 *        plugin initialization and then discovery.
 *      - OpenHPI framework calls this ABI every 3 minutes.
 *        If this function is called after the initial discovery,
 *        then call is ignored and no discovery is done again.
 *      - If the discovery is called for the 1st time (after plugin
 *        initialazation), then, starts the event threads for active and
 *        standby OAs.
 *      - If the discovery is called for the 1st time (after plugin
 *        initialazation), then, starts the discovery
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_discover_resources(void *oh_handler)
{
        struct oh_handler_state *handler;
        struct oa_soap_handler *oa_handler = NULL;
        SaErrorT rv = SA_OK;
        GError **error = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        handler = (struct oh_handler_state *) oh_handler;
        oa_handler = (struct oa_soap_handler *) handler->data;

        /* Check whether the plugin is initialized or not
         * If not, initialize the plugin
         */
        if (oa_handler == NULL) {
                rv = build_oa_soap_custom_handler(handler);
                if (rv != SA_OK) {
                        err("Plugin initialization failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }
        }

        /* Check the status of the plugin */
        g_mutex_lock(oa_handler->mutex);
        switch (oa_handler->status) {
                case PRE_DISCOVERY:
                        /* This is the first call for discovery */
                        g_mutex_unlock(oa_handler->mutex);
                        dbg("First discovery");
                        break;

                case PLUGIN_NOT_INITIALIZED:
                        /* The plugin has encountered a problem while
                         * initializing, the configured OA may not be reachable.
                         * Try to initialize the plugin again.
                         */
                        g_mutex_unlock(oa_handler->mutex);
                        rv = build_oa_soap_custom_handler(handler);
                        if (rv != SA_OK) {
                                err("Plugin initialization failed");
                                return rv;
                        }
                        break;

                case DISCOVERY_FAIL:
                        /* The last discovery has failed.
                         * May be due to active OA is not reachable or
                         * due to OA switchover during discovery.
                         * Try to recover from the problem
                         */
                        g_mutex_unlock(oa_handler->mutex);
                        rv = check_discovery_failure(oh_handler);
                        if (rv != SA_OK) {
                                g_mutex_lock(oa_handler->mutex);
                                oa_handler->status = DISCOVERY_FAIL;
                                g_mutex_unlock(oa_handler->mutex);
                                err("Discovery failed for OA %s",
                                    oa_handler->active_con->server);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                        }
                        break;

                case DISCOVERY_COMPLETED:
                        /* OpenHPI framework calls the discovery every 3 minutes
                         * OA SOAP plugin gets the changes to resource as
                         * part of event handling, hence re-discovery
                         * is not required. If the discovery is already
                         * done once, ignore and return success
                         */
                        g_mutex_unlock(oa_handler->mutex);
                        dbg("Discovery already done");
                        return SA_OK;
                        break;

                default:
                        /* This code should never get executed */
                        g_mutex_unlock(oa_handler->mutex);
                        err("Wrong oa_soap handler state detected");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Create the event thread for the OA in slot 1
         * If the thread_handler is not NULL, then the event threads are
         * already created and skip the event thread creation
         */
        g_mutex_lock(oa_handler->mutex);
        if (oa_handler->oa_1->thread_handler == NULL) {
                /* Subscribe for events, so that we don't lose any changes
                 * to resource states which may happen during discovery
                 */
                rv = create_event_session(oa_handler->oa_1);
                if (rv != SOAP_OK) {
                        /* If the subscription for events fails,
                         * then discovery will fail.
                         * Discovery method has the recovery mechanism.
                         * Hence the error is not handled here
                         */
                        dbg("Subscribe for events failed for OA %s",
                            oa_handler->oa_1->server);
                }

                oa_handler->oa_1->thread_handler =
                        g_thread_create(oa_soap_event_thread,
                                        oa_handler->oa_1,
                                        TRUE, error);
                if (oa_handler->oa_1->thread_handler == NULL) {
                        g_mutex_unlock(oa_handler->mutex);
                        err("g_thread_create failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
               }
        } else
               dbg("OA %s event thread is already started",
                    oa_handler->oa_1->server);

        /* Create the event thread for OA in slot 2 */
        if (oa_handler->oa_2->thread_handler == NULL) {
                rv = create_event_session(oa_handler->oa_2);
                if (rv != SOAP_OK) {
                        dbg("Subscribe for events failed OA %s",
                            oa_handler->oa_2->server);
                }

                oa_handler->oa_2->thread_handler =
                        g_thread_create(oa_soap_event_thread,
                                        oa_handler->oa_2,
                                        TRUE, error);
                if (oa_handler->oa_2->thread_handler == NULL) {
                        g_mutex_unlock(oa_handler->mutex);
                        err("g_thread_create failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
               }
        } else
               dbg("OA %s event thread is already started",
                   oa_handler->oa_2->server);

        /* Plug-in intialization is successfully done.
         * Start the discovery of the cClass resources
         */
        rv = discover_oa_soap_system(handler);
        if (rv != SA_OK) {
                oa_handler->status = DISCOVERY_FAIL;
                g_mutex_unlock(oa_handler->mutex);
                err("Discovery failed for active OA %s",
                    oa_handler->active_con->server);

                /* Cleanup the RPTable which may have partially discovered
                 * resource information.
                 */
                cleanup_plugin_rptable(handler);
                return rv;
        }

        oa_handler->status = DISCOVERY_COMPLETED;
        g_mutex_unlock(oa_handler->mutex);

        dbg("Discovery completed for active OA %s",
            oa_handler->active_con->server);
        return SA_OK;
}

/**
 * discover_oa_soap_system
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the OA SOAP resources.
 *      Discovers all the resources of cClass system
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_oa_soap_system(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        dbg("Discovering HP BladeSystem c-Class");
        dbg(" Discovering Enclosure ......................");
        rv = discover_enclosure(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover Enclosure");
                return rv;
        }

        dbg(" Discovering Blades ...................");
        rv = discover_server(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover Server Blade");
                return rv;
        }

        dbg(" Discovering InterConnect ...................");
        rv = discover_interconnect(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover InterConnect");
                return rv;
        }

	dbg(" Discovering Thermal Subsystem ..............");
	rv = oa_soap_disc_therm_subsys(oh_handler);
	if (rv != SA_OK) {
		err("Failed to discover Thermal Subsystem ");
		return rv;
	}

	dbg(" Discovering Fan Zone .......................");
	rv = oa_soap_disc_fz(oh_handler);
	if (rv != SA_OK) {
		err("Failed to discover Fan Zone ");
		return rv;
	}

        dbg(" Discovering Fan ............................");
        rv = oa_soap_disc_fan(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover Fan ");
                return rv;
        }

        dbg(" Discovering Power Subsystem ................");
        rv = discover_power_subsystem(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover Power Subsystem ");
                return rv;
        }

        dbg(" Discovering Power Supply Unit ..............");
        rv = discover_power_supply(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover Power Supply Unit");
                return rv;
        }

        dbg(" Discovering OA .............................");
        rv = discover_oa(oh_handler);
        if (rv != SA_OK) {
                err("Failed to discover OA");
                return rv;
        }

	dbg(" Discovering LCD .............................");
	rv = oa_soap_disc_lcd(oh_handler);
	if (rv != SA_OK) {
		err("Failed to discover LCD");
		return rv;
	}

        oa_soap_push_disc_res(oh_handler);

        return SA_OK;
}

/**
 * oa_soap_build_enc_info
 *      @oh_handler: Pointer to openhpi handler
 *      @info:       Pointer to enclosure info structure
 *
 * Purpose:
 *      Gets the enclosure info and fills the max bays available for the
 *      enclosure
 *
 * Detailed Description:
 *      - Gets the maximum number bays for server blades, interconnect,
 *        OA, fans and power supply
 *      - Creates the resource id matrix for the server blades, interconnect,
 *        OA, thermal subsystem, fan zones, fans, power subsystem and
 *        power supply
 *      - Creates the presence matrix for the server blades, interconnect,
 *        OA, fans and power supply
 *      - Initialize the presence matrix to ABSENT and resource id matrix to
 *        UNSPECIFIED_ID
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_build_enc_info(struct oh_handler_state *oh_handler,
				       struct enclosureInfo *info)
{
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T i;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        if(oa_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	/* Find the enclosure type and max fan zones */
	switch (info->bladeBays) {
		case OA_SOAP_C7000_MAX_BLADE:
			oa_handler->enc_type = OA_SOAP_ENC_C7000;
			oa_handler->oa_soap_resources.fan_zone.max_bays =
				OA_SOAP_C7000_MAX_FZ;
			break;
		case OA_SOAP_C3000_MAX_BLADE:
			oa_handler->enc_type = OA_SOAP_ENC_C3000;
			oa_handler->oa_soap_resources.fan_zone.max_bays =
				OA_SOAP_C3000_MAX_FZ;
			break;
		default:
			err("Invalid number (%d) of server bays detected",
			    info->bladeBays);
			return SA_ERR_HPI_INTERNAL_ERROR;
	}

        oa_handler->oa_soap_resources.enclosure_rid =
            SAHPI_UNSPECIFIED_RESOURCE_ID;
        oa_handler->oa_soap_resources.power_subsystem_rid =
            SAHPI_UNSPECIFIED_RESOURCE_ID;
        oa_handler->oa_soap_resources.thermal_subsystem_rid =
            SAHPI_UNSPECIFIED_RESOURCE_ID;
	oa_handler->oa_soap_resources.lcd_rid = SAHPI_UNSPECIFIED_RESOURCE_ID;

        /* Create the resource presence matrix for
         * server, interconnect, OA, power supply and fan unit.
         * We need the resource presence matrix for re-discovery to sync
         * with current states of the resources
         */

        /* Build resource presence matrix for servers */
        oa_handler->oa_soap_resources.server.max_bays = info->bladeBays;
        oa_handler->oa_soap_resources.server.presence =
                (enum resource_presence_status *)
                g_malloc0((sizeof(enum resource_presence_status)) *
                          oa_handler->oa_soap_resources.server.max_bays);
        if (oa_handler->oa_soap_resources.server.presence == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* allocate memory for resource_id matrix server blades */
        oa_handler->oa_soap_resources.server.resource_id =
                (SaHpiResourceIdT *) g_malloc0((sizeof(SaHpiResourceIdT ) *
                     oa_handler->oa_soap_resources.server.max_bays));
        if (oa_handler->oa_soap_resources.server.resource_id == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* Create the placeholder for serial number
         * If the gets replaced during the switchover or when OA is not
         * reachable, we can detect this change by comparing the serial
         * numbers of the old and new blade.
         */
        oa_handler->oa_soap_resources.server.serial_number = (char **)
                g_malloc0(sizeof(char **) *
                          oa_handler->oa_soap_resources.server.max_bays);
        if (oa_handler->oa_soap_resources.server.serial_number == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        for (i = 0; i < oa_handler->oa_soap_resources.server.max_bays; i++) {
                oa_handler->oa_soap_resources.server.presence[i] = RES_ABSENT;
                oa_handler->oa_soap_resources.server.resource_id[i] =
                        SAHPI_UNSPECIFIED_RESOURCE_ID;
                oa_handler->oa_soap_resources.server.serial_number[i] = (char *)
                        g_malloc0(sizeof(char *) * MAX_SERIAL_NUM_LENGTH);
                if (oa_handler->oa_soap_resources.server.serial_number[i] ==
                    NULL) {
                        err("Out of memory");
                        release_oa_soap_resources(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
        }

        /* Build resource presence matrix for interconnects */
        oa_handler->oa_soap_resources.interconnect.max_bays =
                info->interconnectTrayBays;
        oa_handler->oa_soap_resources.interconnect.presence =
                (enum resource_presence_status *)
                g_malloc0((sizeof(enum resource_presence_status)) *
                          oa_handler->oa_soap_resources.interconnect.max_bays);
        if (oa_handler->oa_soap_resources.interconnect.presence == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* allocate memory for resource_id matrix interconnects */
        oa_handler->oa_soap_resources.interconnect.resource_id =
                (SaHpiResourceIdT *)
                g_malloc0((sizeof(SaHpiResourceIdT ) *
                          oa_handler->oa_soap_resources.interconnect.max_bays));
        if (oa_handler->oa_soap_resources.interconnect.resource_id == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        oa_handler->oa_soap_resources.interconnect.serial_number = (char **)
                g_malloc0(sizeof(char **) *
                          oa_handler->oa_soap_resources.interconnect.max_bays);
        if (oa_handler->oa_soap_resources.interconnect.serial_number == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        for (i = 0;
             i < oa_handler->oa_soap_resources.interconnect.max_bays;
             i++) {
                oa_handler->oa_soap_resources.interconnect.presence[i] =
                        RES_ABSENT;
                oa_handler->oa_soap_resources.interconnect.resource_id[i] =
                        SAHPI_UNSPECIFIED_RESOURCE_ID;

                oa_handler->oa_soap_resources.interconnect.serial_number[i] =
                        (char *) g_malloc0(sizeof(char *) *
                                          MAX_SERIAL_NUM_LENGTH);
                if (oa_handler->oa_soap_resources.interconnect.serial_number[i]
                    == NULL) {
                        err("Out of memory");
                        release_oa_soap_resources(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
        }

        /* Build resource presence matrix for OAs */
        oa_handler->oa_soap_resources.oa.max_bays = info->oaBays;
        oa_handler->oa_soap_resources.oa.presence =
                (enum resource_presence_status *)
                g_malloc0((sizeof(enum resource_presence_status)) *
                          oa_handler->oa_soap_resources.oa.max_bays);
        if (oa_handler->oa_soap_resources.oa.presence == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        oa_handler->oa_soap_resources.oa.serial_number = (char **)
                g_malloc0(sizeof(char **) *
                          oa_handler->oa_soap_resources.oa.max_bays);
        if (oa_handler->oa_soap_resources.oa.serial_number == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* allocate memory for OAs resource_id array */
        oa_handler->oa_soap_resources.oa.resource_id =
                (SaHpiResourceIdT *)
                g_malloc0((sizeof(SaHpiResourceIdT ) *
                          oa_handler->oa_soap_resources.oa.max_bays));
        if (oa_handler->oa_soap_resources.oa.resource_id == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        for (i = 0; i < oa_handler->oa_soap_resources.oa.max_bays; i++) {
                oa_handler->oa_soap_resources.oa.presence[i] = RES_ABSENT;
                oa_handler->oa_soap_resources.oa.resource_id[i] =
                        SAHPI_UNSPECIFIED_RESOURCE_ID;

                oa_handler->oa_soap_resources.oa.serial_number[i] = (char *)
                        g_malloc0(sizeof(char *) * MAX_SERIAL_NUM_LENGTH);
                if (oa_handler->oa_soap_resources.oa.serial_number[i] == NULL) {
                        err("Out of memory");
                        release_oa_soap_resources(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
        }

        /* Build resource presence matrix for fan zone */
	oa_handler->oa_soap_resources.fan_zone.resource_id =
                (SaHpiResourceIdT *) g_malloc0((sizeof(SaHpiResourceIdT ) *
                     oa_handler->oa_soap_resources.fan_zone.max_bays));
        if (oa_handler->oa_soap_resources.fan_zone.resource_id == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* Fan zones do not have serial number.  Therefore, the serial number
         * array is not constructed.
        */
	for (i = 0; i < oa_handler->oa_soap_resources.fan_zone.max_bays; i++) {
		oa_handler->oa_soap_resources.fan_zone.resource_id[i] =
			SAHPI_UNSPECIFIED_RESOURCE_ID;
	}

        /* Build resource presence matrix for fans */
        oa_handler->oa_soap_resources.fan.max_bays = info->fanBays;
        oa_handler->oa_soap_resources.fan.presence =
                (enum resource_presence_status *)
                g_malloc0((sizeof(enum resource_presence_status)) *
                          oa_handler->oa_soap_resources.fan.max_bays);
        if (oa_handler->oa_soap_resources.fan.presence == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* allocate memory for fans resource_id array */
        oa_handler->oa_soap_resources.fan.resource_id =
                (SaHpiResourceIdT *)
                g_malloc0((sizeof(SaHpiResourceIdT ) *
                          oa_handler->oa_soap_resources.fan.max_bays));
        if (oa_handler->oa_soap_resources.fan.resource_id == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        /* Fans do not have serial number.  Therefore, the serial number
         * array is not constructed.
        */
        for (i = 0; i < oa_handler->oa_soap_resources.fan.max_bays; i++) {
                oa_handler->oa_soap_resources.fan.presence[i] = RES_ABSENT;
                oa_handler->oa_soap_resources.fan.resource_id[i] =
                        SAHPI_UNSPECIFIED_RESOURCE_ID;
        }

        /* Build resource presence matrix for power supply units */
        oa_handler->oa_soap_resources.ps_unit.max_bays = info->powerSupplyBays;
        oa_handler->oa_soap_resources.ps_unit.presence =
                (enum resource_presence_status *)
                g_malloc0((sizeof(enum resource_presence_status)) *
                          oa_handler->oa_soap_resources.ps_unit.max_bays);
        if (oa_handler->oa_soap_resources.ps_unit.presence == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        /* allocate memory for power supplies resource_id array */
        oa_handler->oa_soap_resources.ps_unit.resource_id =
                (SaHpiResourceIdT *)
                g_malloc0((sizeof(SaHpiResourceIdT ) *
                          oa_handler->oa_soap_resources.ps_unit.max_bays));
        if (oa_handler->oa_soap_resources.ps_unit.resource_id == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }
        oa_handler->oa_soap_resources.ps_unit.serial_number = (char **)
                 g_malloc0(sizeof(char **) *
                           oa_handler->oa_soap_resources.ps_unit.max_bays);
        if (oa_handler->oa_soap_resources.ps_unit.serial_number == NULL) {
                err("Out of memory");
                release_oa_soap_resources(oa_handler);
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        for (i = 0; i < oa_handler->oa_soap_resources.ps_unit.max_bays; i++) {
                oa_handler->oa_soap_resources.ps_unit.presence[i] = RES_ABSENT;
                oa_handler->oa_soap_resources.ps_unit.resource_id[i] =
                        SAHPI_UNSPECIFIED_RESOURCE_ID;

                oa_handler->oa_soap_resources.ps_unit.serial_number[i] =
                        (char *) g_malloc0(sizeof(char *) *
                        MAX_SERIAL_NUM_LENGTH);
                if (oa_handler->oa_soap_resources.ps_unit.serial_number[i] ==
                    NULL) {
                        err("Out of memory");
                        release_oa_soap_resources(oa_handler);
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }
        }

        dbg("server bays = %d",oa_handler->oa_soap_resources.server.max_bays);
        dbg("intercennet bays = %d",
            oa_handler->oa_soap_resources.interconnect.max_bays);
        dbg("OA bays = %d",oa_handler->oa_soap_resources.oa.max_bays);
        dbg("fan bays = %d",oa_handler->oa_soap_resources.fan.max_bays);
        dbg("power supply bays = %d",
            oa_handler->oa_soap_resources.ps_unit.max_bays);

        return SA_OK;
}

/**
 * build_enclosure_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @name:        Pointer to the name of the enclosure
 *      @resource_id: Pointer to the resource id
 *
 * Purpose:
 *      Builds the enclosure RPT entry.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_enclosure_rpt(struct oh_handler_state *oh_handler,
                             char *name,
                             SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        struct oa_soap_handler *oa_handler;
        char *entity_root = NULL;
        SaHpiRptEntryT rpt;
        struct rackTopology2 response;
        struct encLink2 enc;

        if (oh_handler == NULL || name == NULL || resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        if(oa_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Fetch and encode the entity path required for the rpt field */
        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        memset(&entity_path, 0, sizeof(SaHpiEntityPathT));
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the enclosure */
        memset(&rpt, 0, sizeof(SaHpiRptEntryT));
        rpt.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                   SAHPI_CAPABILITY_RESOURCE |
                                   SAHPI_CAPABILITY_SENSOR |
                                   SAHPI_CAPABILITY_INVENTORY_DATA |
                                   SAHPI_CAPABILITY_CONTROL ;
        rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_ROOT;
        rpt.ResourceEntity.Entry[0].EntityLocation = 0;
        rv = oh_concat_ep(&(rpt.ResourceEntity), &entity_path);
        if (rv != SA_OK) {
                err("concat of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceFailed = SAHPI_FALSE;
        rpt.HotSwapCapabilities = 0x0;
        rpt.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt.ResourceTag.DataLength = strlen(name) + 1;
        memset(rpt.ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt.ResourceTag.Data),
                 strlen(name) + 1, "%s", name);
        rpt.ResourceId = oh_uid_from_entity_path(&(rpt.ResourceEntity));

        /* getRackTopology2 soap call is supported starting with OA firmware
         * version 2.20
         */
        if (get_oa_fw_version(oh_handler) >= OA_2_20) {
                rv = soap_getRackTopology2(oa_handler->active_con, &response);
                if (rv != SOAP_OK) {
                        err("Get rack topology2 call failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                soap_getEncLink2(response.enclosures, &enc);
                rpt.ResourceInfo.ProductId = enc.productId;

        }

        /* Add the enclosure rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
        if (rv != SA_OK) {
                err("Failed to Add Enclosure Resource");
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_enclosure_rdr
 *      @oh_handler:  Pointer to openhpi handler.
 *      @con:         Pointer to the soap client handler.
 *      @response:    Pointer to enclosure info response structure.
 *      @resource_id: Resource id
 *
 * Purpose:
 *      Populate the enclosure RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the enclosure inventory RDR
 * 	- Creates the temperature, operational status, predictive failure,
 * 	  internal data error, device failure error, device degraded error,
 * 	  redundancy error and device not supported sensor RDR
 * 	- Creates UID control RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on out of memory
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_enclosure_rdr(struct oh_handler_state *oh_handler,
                             SOAP_CON *con,
                             struct enclosureInfo *response,
                             SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
	struct enclosureStatus status_response;
        struct getThermalInfo thermal_request;
        struct thermalInfo thermal_response;
	SaHpiBoolT event_support = SAHPI_FALSE;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL || response == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Build inventory rdr for the enclosure */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = build_enclosure_inv_rdr(oh_handler, response, &rdr, &inventory);
        if (rv != SA_OK) {
                err("Failed to Add enclosure inventory RDR");
                return rv;
        }

        rv = oh_add_rdr(oh_handler->rptcache, resource_id, &rdr, inventory, 0);
        if (rv != SA_OK) {
                err("Failed to add rdr");
                return rv;
        }

	/* Make a soap call to OA requesting for the enclosure thermal status */
	thermal_request.sensorType = SENSOR_TYPE_ENC;
	thermal_request.bayNumber = 1;

	rv = soap_getThermalInfo(con, &thermal_request, &thermal_response);
	if (rv != SOAP_OK) {
		err("Get thermalInfo failed for enclosure");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

        /* Build thermal sensor rdr for the enclosure */
	OA_SOAP_BUILD_THRESHOLD_SENSOR_RDR(OA_SOAP_SEN_TEMP_STATUS,
					   thermal_response)

	/* Make a soap call to OA requesting for the enclosure status */
	rv = soap_getEnclosureStatus(con, &status_response);
	if (rv != SOAP_OK) {
		err("Get enclosure status soap call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status_response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status_response.operationalStatus)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status_response.diagnosticChecks.
						internalDataError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status_response.diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status_response.diagnosticChecks.
						deviceDegraded)

	/* Build redundancy error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_REDUND_ERR,
					status_response.diagnosticChecks.
						redundancy)

	/* Parse the diganosticChecksEx */
	oa_soap_parse_diag_ex(status_response.diagnosticChecksEx,
			      diag_ex_status);

	/* Build device not supported sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT])

        /* Build UID control rdr for Enclosure */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_UID_CNTRL)

        return SA_OK;
}

/**
 * discover_enclosure
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discovers the enclosure.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_enclosure(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct enclosureInfo response;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Make a soap call to OA requesting for the enclosure information */
        rv = soap_getEnclosureInfo(oa_handler->active_con, &response);
        if (rv != SOAP_OK) {
                err("Get enclosure info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Retrieve the enclosure information */
        rv = oa_soap_build_enc_info(oh_handler, &response);
        if (rv != SA_OK) {
                err("build enclosure info failed");
                return rv;
        }

        rv = build_enclosure_rpt(oh_handler, response.name, &resource_id);
        if (rv != SA_OK) {
                err("build enclosure rpt failed");
                return rv;
        }

        /* Save enclosure resource id */
        oa_handler->oa_soap_resources.enclosure_rid = resource_id;

        /* SOAP call has been made while building the rpt, so the response
         * structure is not valid any more.  Get the information again.
         */
        rv = soap_getEnclosureInfo(oa_handler->active_con, &response);
        if (rv != SOAP_OK) {
                err("Get enclosure info failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rv = build_enclosure_rdr(oh_handler, oa_handler->active_con,
                                 &response, resource_id);
        if (rv != SA_OK) {
                err("build enclosure rdr failed");
                return rv;
        }

        return SA_OK;
}

/**
 * build_oa_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @bay_number:  Bay number of the OA
 *      @resource_id: Pointer to the resource Id
 *
 * Purpose:
 *      Populate the OA RPT entry.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_oa_rpt(struct oh_handler_state *oh_handler,
                      SaHpiInt32T bay_number,
                      SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        char *entity_root = NULL;
        SaHpiRptEntryT rpt;

        if (oh_handler == NULL || resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the enclosure */
        memset(&rpt, 0, sizeof(SaHpiRptEntryT));
        rpt.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                   SAHPI_CAPABILITY_RESOURCE |
                                   SAHPI_CAPABILITY_FRU |
                                   SAHPI_CAPABILITY_SENSOR |
                                   SAHPI_CAPABILITY_INVENTORY_DATA |
                                   SAHPI_CAPABILITY_CONTROL ;
        rpt.ResourceEntity.Entry[1].EntityType = SAHPI_ENT_ROOT;
        rpt.ResourceEntity.Entry[1].EntityLocation = 0;
        rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SYS_MGMNT_MODULE;
        rpt.ResourceEntity.Entry[0].EntityLocation = bay_number;
        rv = oh_concat_ep(&(rpt.ResourceEntity), &entity_path);
        if (rv != SA_OK) {
                err("concat of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt.ResourceId = oh_uid_from_entity_path(&(rpt.ResourceEntity));
        rpt.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceFailed = SAHPI_FALSE;
        rpt.HotSwapCapabilities = 0x0;
        rpt.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt.ResourceTag.DataLength = strlen(OA_NAME) + 1;
        memset(rpt.ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt.ResourceTag.Data),
                 strlen(OA_NAME) + 1, OA_NAME);

        /* Add the OA rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
        if (rv != SA_OK) {
                err("Failed to Add OA RPT");
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_oa_rdr
 *      @oh_handler:  Pointer to openhpi handler.
 *      @con:         Pointer to the soap client handler.
 *      @response:    Pointer OA info response structure.
 *      @resource_id: Resource id
 *
 * Purpose:
 *      Populates the OA RDRs
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the inventory RDR
 * 	- Creates the temperature, operational status, predictive failure,
 * 	  internal data error, management processor error, device failure error,
 * 	  device degraded error, redundancy error, device not supported and OA
 * 	  link status sensor RDR
 * 	- Creates UID control RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_oa_rdr(struct oh_handler_state *oh_handler,
                      SOAP_CON *con,
                      SaHpiInt32T bay_number,
                      struct oaInfo *response,
                      SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_sensor_info *sensor_info=NULL;
        struct getThermalInfo thermal_request;
        struct thermalInfo thermal_response;
	SaHpiBoolT event_support = SAHPI_FALSE;
	struct getOaStatus status_request;
	struct oaStatus status_response;
	struct getOaNetworkInfo nw_info_request;
	struct oaNetworkInfo nw_info_response;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL || response == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Build inventory rdr for OA */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = build_oa_inv_rdr(oh_handler, response, &rdr, &inventory);
        if (rv != SA_OK) {
                err("Failed to build OA inventory RDR");
                return rv;
        }
        rv = oh_add_rdr(oh_handler->rptcache, resource_id, &rdr, inventory, 0);
        if (rv != SA_OK) {
                err("Failed to add rdr");
                return rv;
        }

	/* Make a soap call to OA requesting for the OA thermal status */
	thermal_request.sensorType = SENSOR_TYPE_OA;
	thermal_request.bayNumber = bay_number;

	rv = soap_getThermalInfo(con, &thermal_request, &thermal_response);
	if (rv != SOAP_OK) {
		err("Get thermalInfo failed for enclosure");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

        /* Build thermal sensor rdr for the enclosure */
	OA_SOAP_BUILD_THRESHOLD_SENSOR_RDR(OA_SOAP_SEN_TEMP_STATUS,
					thermal_response)

        /* Build UID control rdr for OA */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_UID_CNTRL)

	status_request.bayNumber = response->bayNumber;
	rv = soap_getOaStatus(con, &status_request, &status_response);
	if (rv != SOAP_OK) {
		err("Get OA status failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status_response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status_response.operationalStatus)

	/* Build OA redundancy sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OA_REDUND,
					status_response.oaRedundancy)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status_response.diagnosticChecks.
						internalDataError)

	/* Build management processor error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_MP_ERR,
					status_response.diagnosticChecks.
						managementProcessorError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status_response.diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status_response.diagnosticChecks.
						deviceDegraded)

	/* Build redundancy error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_REDUND_ERR,
					status_response.diagnosticChecks.
						redundancy)

	/* Parse the diganosticChecksEx */
	oa_soap_parse_diag_ex(status_response.diagnosticChecksEx,
			      diag_ex_status);

	/* Build firmware mismatch sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_FW_MISMATCH,
					diag_ex_status[DIAG_EX_FW_MISMATCH])

	/* Build device not supported sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT])

	nw_info_request.bayNumber = response->bayNumber;
	rv = soap_getOaNetworkInfo(con, &nw_info_request, &nw_info_response);
	if (rv != SOAP_OK) {
		err("Get OA network info SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build OA link status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OA_LINK_STATUS,
					nw_info_response.linkActive)

        return SA_OK;
}

/**
 * discover_oa
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the OA.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_oa(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        struct getOaInfo request;
        struct oaInfo response;
        struct getOaStatus status_request;
        struct oaStatus status_response;
        SaHpiInt32T i = 0;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *)oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.oa.max_bays; i++) {
                status_request.bayNumber = i;
                rv = soap_getOaStatus(oa_handler->active_con, &status_request,
                                      &status_response);
                if (rv != SOAP_OK) {
                        err("Get OA status failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Sometimes, if the OA is absent, then OA status is shown as
                 * STANDBY in getOaStatus response.  As workaround, if OA
                 * status is STANDBY and oaRedudancy state is set to false,
                 * Then, it is considered as ABSENT.
                 *
                 * But, if the OA is recently inserted, then oaRedudancy state
                 * will be set to false.  In this scenario, the OA state will
                 * be wrongly considered as ABSENT.  This is a known limitation
                 *
                 * TODO: Remove this workaround once the fix is available in OA
                 *       firmware
                 */
                if ((status_response.oaRole == OA_ABSENT) ||
                    (status_response.oaRole == STANDBY &&
                     status_response.oaRedundancy == HPOA_FALSE)) {
                        /* Update the OA status as absent */
                        switch (i) {
                                case 1:
                                        oa_handler->oa_1->oa_status = OA_ABSENT;
                                        break;
                                case 2:
                                        oa_handler->oa_2->oa_status = OA_ABSENT;
                                        break;
                                default:
                                        err("Wrong OA slot number - %d", i);
                                        return SA_ERR_HPI_INTERNAL_ERROR;
                        }

                        /* If resource not present, continue checking for
                         * next bay
                         */
                        dbg("OA %d is not present", i);
                        continue;
                }

                request.bayNumber = i;
                rv = soap_getOaInfo(oa_handler->active_con, &request,
                                    &response);
                if (rv != SOAP_OK) {
                        err("Get OA Info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* If the OA is not yet stable, then getOaInfo response
                 * structure will not have proper information. Abort the
                 * discovery and let the OA to stabilize. The discovery will be
                 * called by the openhpi framework after 3 minutes
                 */
                if (response.serialNumber == NULL) {
                        err("OA %d is not yet stabilized", i);
                        err("Discovery is aborted");
                        err("Discovery will happen after 3 minutes");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Build rpt entry for OA */
                rv = build_oa_rpt(oh_handler, i, &resource_id);
                if (rv != SA_OK) {
                        err("Failed to build OA RPT");
                        return rv;
                }

                /* Update the OA firmware version to RPT entry */
                rv = update_oa_info(oh_handler, &response, resource_id);
                if (rv != SA_OK) {
                        err("Failed to update OA RPT");
                        return rv;
                }

                /* Update resource_status structure with resource_id,
                   serial_number, and presence status */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.oa, i,
                      response.serialNumber, resource_id, RES_PRESENT);

                /* Build RDRs for OA */
                rv = build_oa_rdr(oh_handler, oa_handler->active_con, i,
                                  &response, resource_id);
                if (rv != SA_OK) {
                        err("Failed to build OA RDR");
                        /* Reset resource_status structure to default values */
                        oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.oa, i,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                        return rv;
                }

        } /* End of for loop */

        return SA_OK;
}

/**
 * build_discovered_server_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @con:         Pointer to the soap client handler
 *      @response:    Pointer to the bladeInfo structure
 *      @resource_id: Pointer to the resource id
 *
 * Purpose:
 *      Populate the server blade RPT with aid of build_server_rpt() and add
 *      hotswap state information to the returned rpt.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_discovered_server_rpt(struct oh_handler_state *oh_handler,
                                     SOAP_CON *con, struct bladeInfo *response,
                                     SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiPowerStateT state;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        SaHpiRptEntryT rpt;

        if (oh_handler == NULL || con == NULL || response == NULL ||
            resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if(build_server_rpt(oh_handler, response, &rpt) != SA_OK) {
                err("Building Server Rpt failed during discovery");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Set power status of discovered blade resource initially as POWER ON*/
	oa_soap_bay_pwr_status[response->bayNumber -1] = SAHPI_POWER_ON;

        /* Get the power state of the server blade to determine the
         * hotswap state.  The hotswap state of the server will be
         * maintained in the private data area of the server RPT.
         */
        if (rpt.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                rv = get_server_power_state(con, response->bayNumber, &state);
                if (rv != SA_OK) {
                        err("Unable to get power status");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                hotswap_state = (struct oa_soap_hotswap_state *)
                        g_malloc0(sizeof(struct oa_soap_hotswap_state));
                if (hotswap_state == NULL) {
                        err("Out of memory");
                        return SA_ERR_HPI_OUT_OF_MEMORY;
                }

                switch (state) {
                        case SAHPI_POWER_ON:
                        case SAHPI_POWER_CYCLE:
                                hotswap_state->currentHsState =
                                        SAHPI_HS_STATE_ACTIVE;
                                break;

                        case SAHPI_POWER_OFF:
                                hotswap_state->currentHsState =
                                        SAHPI_HS_STATE_INACTIVE;

				/* Change the power state to POWER OFF for 
				 * blade entry in oa_soap_bay_pwr_status array
				 */
				oa_soap_bay_pwr_status[response->bayNumber -1] =
								 SAHPI_POWER_OFF;
                                break;

                        default:
                                err("unknown power status");
                                if (hotswap_state != NULL)
                                        g_free(hotswap_state);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                }

        }

        /* Add the server rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, hotswap_state, 0);
        if (rv != SA_OK) {
                err("Failed to add Server rpt");
                if (hotswap_state != NULL)
                        g_free(hotswap_state);
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_server_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @response:    Pointer to the bladeInfo structure
 *      @rpt:         Pointer to rpt to be filled
 *
 * Purpose:
 *      This routine should be called to during discovery/re-discovery phase
 *      and when a new blade gets inserted.  It populates the server blade RPT
 *      information common to discovered and insterted blades.  The caller will
 *      fill in the information specific to the manner in which the blade was
 *      found.  For example, the hotswap state could be different in the case
 *      of discovered vs. inserted blades, because an inserted blade goes
 *      through the pending state, while a discovered blade doesn't.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT build_server_rpt(struct oh_handler_state *oh_handler,
                          struct bladeInfo *response,
                          SaHpiRptEntryT *rpt)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        char *entity_root = NULL;

        if (oh_handler == NULL || response == NULL || rpt == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the server */
        memset(rpt, 0, sizeof(SaHpiRptEntryT));
        rpt->ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                        SAHPI_CAPABILITY_RESET |
                                        SAHPI_CAPABILITY_RESOURCE |
                                        SAHPI_CAPABILITY_POWER |
                                        SAHPI_CAPABILITY_FRU |
                                        SAHPI_CAPABILITY_MANAGED_HOTSWAP |
                                        SAHPI_CAPABILITY_SENSOR |
                                        SAHPI_CAPABILITY_CONTROL |
                                        SAHPI_CAPABILITY_INVENTORY_DATA;
        rpt->ResourceEntity.Entry[1].EntityType = SAHPI_ENT_ROOT;
        rpt->ResourceEntity.Entry[1].EntityLocation = 0;
        switch(response->bladeType) {
                case    BLADE_TYPE_SERVER:
                        rpt->ResourceEntity.Entry[0].EntityType =
                                SAHPI_ENT_SYSTEM_BLADE;
                break;
                case    BLADE_TYPE_IO:
                        rpt->ResourceEntity.Entry[0].EntityType =
                                SAHPI_ENT_IO_BLADE;
                        rpt->ResourceCapabilities &=
                                   ~(SAHPI_CAPABILITY_RESET |
                                     SAHPI_CAPABILITY_POWER |
                                     SAHPI_CAPABILITY_MANAGED_HOTSWAP);
                break;
                case    BLADE_TYPE_STORAGE:
                        rpt->ResourceEntity.Entry[0].EntityType =
                                SAHPI_ENT_DISK_BLADE;
                        rpt->ResourceCapabilities &=
                                   ~(SAHPI_CAPABILITY_RESET |
                                     SAHPI_CAPABILITY_POWER |
                                     SAHPI_CAPABILITY_MANAGED_HOTSWAP);
                break;
                default:
                        err("Invalid blade type: "
                            "expecting server/storage/IO blade");
                        return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt->ResourceEntity.Entry[0].EntityLocation= response->bayNumber;
        rv = oh_concat_ep(&rpt->ResourceEntity, &entity_path);
        if (rv != SA_OK) {
                err("internal error (oh_concat_ep call)");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt->ResourceId = oh_uid_from_entity_path(&(rpt->ResourceEntity));
        rpt->ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
        rpt->ResourceInfo.ProductId = response->productId;
        rpt->ResourceSeverity = SAHPI_OK;
        rpt->ResourceFailed = SAHPI_FALSE;
        rpt->ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt->ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt->ResourceTag.DataLength = strlen(response->name) + 1;
        memset(rpt->ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt->ResourceTag.Data),
                 rpt->ResourceTag.DataLength, "%s", response->name);

        /* set default hotswap capability */
        if (rpt->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                rpt->HotSwapCapabilities =
                        SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY;
        } else {
                rpt->HotSwapCapabilities = 0;
        }
        return SA_OK;
}

/**
 * build_server_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @con:         Pointer to the soap client handler.
 *      @response:    Server blade info response structure
 *      @resource_id: Resource id
 *      @name: Blade resource name
 *
 * Purpose:
 *      Populate the server blade RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the inventory RDR
 * 	- Creates the temperature, power, operational status, predictive
 * 	  failure, internal data error, management processor error, thermal
 * 	  warning, thermal danger, IO configuration error, device power request
 * 	  error, insufficient cooling, device location error, device failure
 * 	  error, device degraded error, device missing, device bonding, device
 * 	  power sequence, network configuration, profile unassigned error,
 * 	  device not supported, too low power request, call HP, storage device
 * 	  missing, power capping error, IML recorded errors, duplicate
 * 	  management IP address sensor RDR
 * 	- Creates UID and power control RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_server_rdr(struct oh_handler_state *oh_handler,
                          SOAP_CON *con,
                          SaHpiInt32T bay_number,
                          SaHpiResourceIdT resource_id,
			  char *name)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        SaHpiRptEntryT *rpt = NULL;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
        struct getBladeThermalInfoArray thermal_request;
        struct bladeThermalInfoArrayResponse thermal_response;
	struct getBladeStatus status_request;
	struct bladeStatus status_response;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id (oh_handler->rptcache, resource_id);
        if (rpt == NULL) {
                err("INVALID RESOURCE");
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        /* Build inventory rdr for server */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = build_server_inv_rdr(oh_handler, con, bay_number,
                                  &rdr, &inventory);
        if (rv != SA_OK) {
                err("Failed to get server inventory RDR");
                return rv;
        }
        rv = oh_add_rdr(oh_handler->rptcache, resource_id, &rdr, inventory, 0);
        if (rv != SA_OK) {
                err("Failed to add rdr");
                return rv;
        }

	/* Make a soap call to OA requesting for the server thermal status */
	thermal_request.bayNumber = bay_number;

	rv = soap_getBladeThermalInfoArray(con, &thermal_request,
					   &thermal_response);
	if (rv != SOAP_OK) {
		err("getBladeThermalInfo failed for blade");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build the thermal sensors based on the blade name*/
	rv = oa_soap_build_blade_thermal_rdr(oh_handler, thermal_response, rpt,
					     name);
	if (rv != SA_OK) {
		err("Failed to build thermal rdr");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

        /* Build power sensor rdr for server */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_PWR_STATUS)

        /* Create power control RDR only for server blades.
           IO and Storage blades don't support power management. */
	if (rpt->ResourceEntity.Entry[0].EntityType == 
		SAHPI_ENT_SYSTEM_BLADE) {
            /* Build power control rdr for server */
	    OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_PWR_CNTRL)
	}
	/* Build UID control rdr for server */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_UID_CNTRL)

	status_request.bayNumber = bay_number;
	rv = soap_getBladeStatus(con, &status_request, &status_response);
	if (rv != SOAP_OK) {
		err("Get thermalInfo failed for enclosure");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status_response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status_response.operationalStatus)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status_response.diagnosticChecks.
						internalDataError)

	/* Build management processor error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_MP_ERR,
					status_response.diagnosticChecks.
						managementProcessorError)

	/* Build thermal waring sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_THERM_WARN,
					status_response.diagnosticChecks.
						thermalWarning)

	/* Build thermal danger sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_THERM_DANGER,
					status_response.diagnosticChecks.
						thermalDanger)

	/* Build IO configuration error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_IO_CONFIG_ERR,
					status_response.diagnosticChecks.
						ioConfigurationError)

	/* Build device power request error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_PWR_REQ,
					status_response.diagnosticChecks.
						devicePowerRequestError)

	/* Build insufficient cooling sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INSUF_COOL,
					status_response.diagnosticChecks.
						insufficientCooling)

	/* Build device location error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_LOC_ERR,
					status_response.diagnosticChecks.
						deviceLocationError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status_response.diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status_response.diagnosticChecks.
						deviceDegraded)

	/* Parse the diganosticChecksEx */
	oa_soap_parse_diag_ex(status_response.diagnosticChecksEx,
			      diag_ex_status);

	/* Build device missing sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_MISS,
					diag_ex_status[DIAG_EX_DEV_MISS])

	/* Build device bonding sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_BOND,
					diag_ex_status[DIAG_EX_DEV_BOND])

	/* Build device power sequence sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_PWR_SEQ,
					diag_ex_status[DIAG_EX_DEV_PWR_SEQ])

	/* Build network configuration sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_NET_CONFIG,
					diag_ex_status[DIAG_EX_NET_CONFIG])

	/* Build profile unassigned error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PROF_UNASSIGN_ERR,
				diag_ex_status[DIAG_EX_PROF_UNASSIGN_ERR])

	/* Build Device not supported sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT])

	/* Build Too low power request sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_TOO_LOW_PWR_REQ,
				diag_ex_status[DIAG_EX_TOO_LOW_PWR_REQ])

	/* Build Call HP sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_CALL_HP,
					diag_ex_status[DIAG_EX_CALL_HP])

	/* Build Storage device missing sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_STORAGE_DEV_MISS,
				diag_ex_status[DIAG_EX_STORAGE_DEV_MISS])

	/* Build Power capping error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_GRPCAP_ERR,
					diag_ex_status[DIAG_EX_GRPCAP_ERR])

	/* Build IML recorded errors sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_IML_ERR,
					diag_ex_status[DIAG_EX_IML_ERR])

	/* Build Duplicate management IP address sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
				diag_ex_status[DIAG_EX_DUP_MGMT_IP_ADDR])

        return SA_OK;
}

/**
 * discover_server
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the server.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_server(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T i;
        struct oa_soap_handler *oa_handler = NULL;
        struct getBladeInfo request;
        struct bladeInfo response;
        SaHpiResourceIdT resource_id;
	char blade_name[MAX_NAME_LEN];

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Discover the blades present in server bays */
        for (i = 1; i <= oa_handler->oa_soap_resources.server.max_bays; i++) {
                request.bayNumber = i;
                /* Make a soap call to get the information of blade in the
                 * current bay
                 */
                rv = soap_getBladeInfo(oa_handler->active_con,
                                       &request, &response);
                if (rv != SOAP_OK) {
                        err("Get blade info failed");
                        return rv;
                }

                if (response.presence != PRESENT) {
                        /* If resource not present, continue checking for
                         * next bay
                         */
                        continue;
                }

		/* Copy the blade name from response for future processing */
        	convert_lower_to_upper(response.name, strlen(response.name),
				       blade_name, MAX_NAME_LEN);

                /* Build rpt entry for server */
                rv = build_discovered_server_rpt(oh_handler,
                          oa_handler->active_con, &response, &resource_id);
                if (rv != SA_OK) {
                        err("Failed to get Server rpt");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Update resource_status structure with resource_id,
                 * serial_number, and presence status
                 */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.server, i,
                      response.serialNumber, resource_id, RES_PRESENT);

                /* Build rdr entry for server */
                rv = build_server_rdr(oh_handler, oa_handler->active_con, i,
                                      resource_id, blade_name);
                if (rv != SA_OK) {
                        err("Failed to add Server rdr");
                        /* Reset resource_status structure to default values */
                        oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.server, i,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

        } /* End of for loop */
        return SA_OK;
}

/**
 * build_interconnect_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @con:         Pointer to the soap client handler.
 *      @name:        Pointer to the name of the interconnect blade
 *      @bay_number:  Bay number of the interconnect blade
 *      @resource_id: Pointer to the resource id
 *      @insetred: flag to indicate if the switch blade is inserted.
 *                 TRUE inserted
 *                 FALSE not inserted (discovered or re-discovered)
 *
 * Purpose:
 *      Populate the interconnect RPT. This routine gets called when a switch
 *      blade is discovered as well as inserted. Hotswap information is
 *      initialized differently for these two cases.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on malloc failure
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_interconnect_rpt(struct oh_handler_state *oh_handler,
                                SOAP_CON *con,
                                char *name,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT *resource_id,
                                int inserted)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        SaHpiPowerStateT state;
        char *entity_root = NULL;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
        SaHpiRptEntryT rpt;
        char temp[MAX_NAME_LEN];
        struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || con == NULL || name == NULL ||
           resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;
        if(oa_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the interconnect */
        memset(&rpt, 0, sizeof(SaHpiRptEntryT));
        rpt.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                   SAHPI_CAPABILITY_RESET |
                                   SAHPI_CAPABILITY_RESOURCE |
                                   SAHPI_CAPABILITY_POWER |
                                   SAHPI_CAPABILITY_FRU |
                                   SAHPI_CAPABILITY_MANAGED_HOTSWAP |
                                   SAHPI_CAPABILITY_SENSOR |
                                   SAHPI_CAPABILITY_CONTROL |
                                   SAHPI_CAPABILITY_INVENTORY_DATA;
        rpt.ResourceEntity.Entry[1].EntityType = SAHPI_ENT_ROOT;
        rpt.ResourceEntity.Entry[1].EntityLocation = 0;
        rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SWITCH_BLADE;
        rpt.ResourceEntity.Entry[0].EntityLocation = bay_number;
        rv = oh_concat_ep(&(rpt.ResourceEntity), &entity_path);
        if (rv != SA_OK) {
                err("concat of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt.ResourceId = oh_uid_from_entity_path(&(rpt.ResourceEntity));

        /* Check whether the interconnect blade is from Cisco Systems
         * TODO: Cisco interconnect blades will have name starting with "Cisco"
         *       If this format gets changed for any reason,
         *       then Cisco interconnect blades will have manufacture id as HP
         *       in ManufacturerId field of rpt entry.
         *       If the interconnect name format changes,
         *       please change the logic accordingly.
         */
        convert_lower_to_upper(name, strlen(name), temp, MAX_NAME_LEN);
        if (strstr(temp, CISCO) != NULL)
                rpt.ResourceInfo.ManufacturerId = CISCO_MANUFACTURING_ID;
        else
                rpt.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;

        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceFailed = SAHPI_FALSE;
        rpt.HotSwapCapabilities = SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY;
        rpt.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt.ResourceTag.DataLength = strlen(name) + 1;
        memset(rpt.ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt.ResourceTag.Data),
                  rpt.ResourceTag.DataLength, "%s", name);

        hotswap_state = (struct oa_soap_hotswap_state *)
                g_malloc0(sizeof(struct oa_soap_hotswap_state));
        if (hotswap_state == NULL) {
                err("Out of memory");
                return SA_ERR_HPI_OUT_OF_MEMORY;
        }

        if (inserted == TRUE) {
                /* The interconnect takes nearly 3 seconds to power on after
                 * insertion.  Intialize the current hotswap state as
                 * change is handled as part of interconnect status events.
                 */
                hotswap_state->currentHsState =
                        SAHPI_HS_STATE_INSERTION_PENDING;

        } else {
                /* Get the power state of the interconnect blade to determine
                 * the hotswap state.  The hotswap state of the interconnect
                 * shall be maintained in a private data area of the
                 * interconnect RPT.
                 */
                rv = get_interconnect_power_state(con, bay_number, &state);
                if (rv != SA_OK) {
                        err("Unable to get power status");
                        return rv;
                }


                switch (state) {
                        case SAHPI_POWER_ON:
                                hotswap_state->currentHsState =
                                        SAHPI_HS_STATE_ACTIVE;
                                break;

                        case SAHPI_POWER_OFF:
                                hotswap_state->currentHsState =
                                        SAHPI_HS_STATE_INACTIVE;
                                break;

                        case SAHPI_POWER_CYCLE:
                        default:
                                err("Wrong power state detected");
                                if (hotswap_state != NULL)
                                        g_free(hotswap_state);
                                return SA_ERR_HPI_INTERNAL_ERROR;
                }
        }

        /* Add the interconnect rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, hotswap_state, 0);
        if (rv != SA_OK) {
                err("Failed to add Interconnect RPT");
                if (hotswap_state != NULL)
                        g_free(hotswap_state);
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_interconnect_rdr
 *      @oh_handler:    Pointer to openhpi handler
 *      @con:           Pointer to the soap client handler.
 *      @info_response: Interconnect info response structure
 *      @resource_id:   Resource id
 *
 * Purpose:
 *      Populate the interconnect blade RDR.
 *      Pushes the RDRs to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the inventory RDR
 * 	- Creates the temperature, operational status, predictive failure,
 * 	  interconnect CPU fault, interconnect health LED, internal data error,
 * 	  management processor error, thermal warning, thermal danger, IO
 * 	  configuration error, device power request error, device failure error,
 * 	  device degraded error, device not supported, device informational,
 * 	  device missing, duplicate management IP address, health operational
 * 	  status and health predictive failure sensor RDR
 * 	- Creates UID and power control RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_interconnect_rdr(struct oh_handler_state *oh_handler,
                                SOAP_CON* con,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
        SaHpiRdrT rdr;
        struct getThermalInfo thermal_request;
        struct thermalInfo thermal_response;
	SaHpiBoolT event_support = SAHPI_FALSE;
	struct getInterconnectTrayStatus status_request;
	struct interconnectTrayStatus status_response;
	enum oa_soap_extra_data_health status;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Build inventory rdr for interconnect */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = build_interconnect_inv_rdr(oh_handler, con, bay_number,
                                        &rdr, &inventory);
        if (rv != SA_OK) {
                err("Failed to get interconnect inventory RDR");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        rv = oh_add_rdr(oh_handler->rptcache, resource_id, &rdr, inventory, 0);
        if (rv != SA_OK) {
                err("Failed to add rdr");
                return rv;
        }

	/* Make a soap call to OA requesting for the interconnect 
	 * thermal status 
	 */
	thermal_request.sensorType = SENSOR_TYPE_INTERCONNECT;
	thermal_request.bayNumber = bay_number;

	rv = soap_getThermalInfo(con, &thermal_request, &thermal_response);
	if (rv != SOAP_OK) {
		err("Get thermalInfo failed for enclosure");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Turn event support ON for thermal sensor. 
	 * Interconnect is the only resource for which the OA supports 
	 * thermal events
	 */
	 event_support = SAHPI_TRUE;

        /* Build thermal sensor rdr for interconnect */
	OA_SOAP_BUILD_THRESHOLD_SENSOR_RDR(OA_SOAP_SEN_TEMP_STATUS,
					   thermal_response)

        /* Build power control rdr for server */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_PWR_CNTRL)

        /* Build UID control rdr for server */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_UID_CNTRL)

	status_request.bayNumber = bay_number;
	rv = soap_getInterconnectTrayStatus(con, &status_request,
					    &status_response);
	if (rv != SOAP_OK) {
		err("Get Interconnect tray status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status_response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status_response.operationalStatus)

	/* Build interconnect CPU fault sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_CPU_FAULT,
					status_response.cpuFault)

	/* Build interconnect health LED sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_HEALTH_LED,
					status_response.healthLed)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status_response.diagnosticChecks.
						internalDataError)

	/* Build management processor error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_MP_ERR,
					status_response.diagnosticChecks.
						managementProcessorError)

	/* Build thermal waring sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_THERM_WARN,
					status_response.diagnosticChecks.
						thermalWarning)

	/* Build thermal danger sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_THERM_DANGER,
					status_response.diagnosticChecks.
						thermalDanger)

	/* Build IO configuration error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_IO_CONFIG_ERR,
					status_response.diagnosticChecks.
						ioConfigurationError)

	/* Build device power request error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_PWR_REQ,
					status_response.diagnosticChecks.
						devicePowerRequestError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status_response.diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status_response.diagnosticChecks.
						deviceDegraded)

	/* Parse the diganosticChecksEx structure */
	oa_soap_parse_diag_ex(status_response.diagnosticChecksEx,
			      diag_ex_status);

	/* Build device not supported sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT])

	/* Build Device informational sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_INFO,
					diag_ex_status[DIAG_EX_DEV_INFO])

	/* Build Storage device missing sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_STORAGE_DEV_MISS,
				diag_ex_status[DIAG_EX_STORAGE_DEV_MISS])

	/* Build Duplicate management IP address sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DUP_MGMT_IP_ADDR,
				diag_ex_status[DIAG_EX_DUP_MGMT_IP_ADDR])

	/* Get the healthStatus enum from extraData structure */
	oa_soap_get_health_val(status_response.extraData, &status);

	/* Build health status operational sensor */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_HEALTH_OPER,
					status)

	/* Build health status predictive failure sensor */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_HEALTH_PRED_FAIL,
					status)

        return SA_OK;
}

/**
 * discover_interconnect
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the interconnect blade.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_interconnect(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        SaHpiInt32T i;
        struct getInterconnectTrayInfo info_request;
        struct interconnectTrayInfo info_response;
        struct getInterconnectTrayStatus status_request;
        struct interconnectTrayStatus status_response;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1;
             i <= oa_handler->oa_soap_resources.interconnect.max_bays;
             i++) {
                status_request.bayNumber = i;
                rv = soap_getInterconnectTrayStatus(oa_handler->active_con,
                                                    &status_request,
                                                    &status_response);
                if (rv != SOAP_OK) {
                        err("Get Interconnect tray status failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* If resource not present, continue checking for next bay */
                if (status_response.presence != PRESENT) {
                        continue;
                }

                info_request.bayNumber = i;
                rv = soap_getInterconnectTrayInfo(oa_handler->active_con,
                                                  &info_request,
                                                  &info_response);
                if (rv != SOAP_OK) {
                        err("Get Interconnect tray info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* Build rpt entry for interconnect */
                rv = build_interconnect_rpt(oh_handler, oa_handler->active_con,
                                            info_response.name, i,
                                            &resource_id, FALSE);
                if (rv != SA_OK) {
                       err("Failed to get interconnect RPT");
                       return rv;
                }

                /* Update resource_status structure with resource_id,
                 * serial_number, and presence status
                 */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.interconnect, i,
                      info_response.serialNumber, resource_id, RES_PRESENT);
                /* Build rdr entry for interconnect */
                rv = build_interconnect_rdr(oh_handler, oa_handler->active_con,
                                            i, resource_id);
                if (rv != SA_OK) {
                       err("Failed to get interconnect RDR");
                        /* Reset resource_status structure to default values */
                        oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.interconnect, i,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                       return rv;
                }

        }
        return SA_OK;
}

/**
 * build_power_subsystem_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @name:        Pointer to the name of the Power subsystem
 *      @resource_id: Pointer to the resource id
 *
 * Purpose:
 *      Populate the power supply RPT.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_power_subsystem_rpt(struct oh_handler_state *oh_handler,
                                   char *name,
                                   SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        char *entity_root = NULL;
        SaHpiRptEntryT rpt;

        if (oh_handler == NULL || name == NULL || resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Fetch and encode the entity path required for the rpt field */
        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the power subsystem */
        memset(&rpt, 0, sizeof(SaHpiRptEntryT));
        rpt.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                   SAHPI_CAPABILITY_RESOURCE |
                                   SAHPI_CAPABILITY_SENSOR;
        rpt.ResourceEntity.Entry[1].EntityType = SAHPI_ENT_ROOT;
        rpt.ResourceEntity.Entry[1].EntityLocation = 0;
        rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_POWER_MGMNT;
        rpt.ResourceEntity.Entry[0].EntityLocation = 1;
        rv = oh_concat_ep(&(rpt.ResourceEntity), &entity_path);
        if (rv != SA_OK) {
                err("concat of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt.ResourceId = oh_uid_from_entity_path(&(rpt.ResourceEntity));
        rpt.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceFailed = SAHPI_FALSE;
        rpt.HotSwapCapabilities = 0x0;
        rpt.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt.ResourceTag.DataLength = strlen(name) + 1;
        memset(rpt.ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt.ResourceTag.Data),
                 rpt.ResourceTag.DataLength, "%s", name);

        /* Add the power subsystem rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
        if (rv != SA_OK) {
                err("Failed to add Power Subsystem RPT");
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_power_subsystem_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *
 * Purpose:
 *      Populate the power supply RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the input power, output power, power status, power capacity,
 * 	  operational status, predictive failure and redundancy sensor RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_power_subsystem_rdr(struct oh_handler_state *oh_handler,
                                   SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        struct oa_soap_sensor_info *sensor_info = NULL;
	struct oa_soap_handler *oa_handler;
	struct powerSubsystemInfo response;
	SaHpiInt32T sensor_status;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Build the input power sensor RDR */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_IN_PWR)

        /* Build the ouput power sensor RDR */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_OUT_PWR)

        /* Build the power consumed sensor RDR */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_PWR_STATUS)

        /* Build the power capacity sensor RDR */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_PWR_CAPACITY)

	rv = soap_getPowerSubsystemInfo(oa_handler->active_con, &response);
	if (rv != SOAP_OK) {
		err("Get power subsystem info SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					response.operationalStatus)

	/* Build redundancy sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_REDUND,
					response.redundancy)

        return SA_OK;
}

/**
 * discover_power_subsystem
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the power supply.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_power_subsystem(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        char power_subsystem[] = POWER_SUBSYSTEM_NAME;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        /* Build rpt entry for power sub system */
        rv = build_power_subsystem_rpt(oh_handler, power_subsystem,
                                       &resource_id);
        if (rv != SA_OK) {
                err("build power subsystem rpt failed");
                return rv;
        }
        /* save power subsystem resource id */
        oa_handler->oa_soap_resources.power_subsystem_rid = resource_id;

        /* Build rdr entry for power sub system*/
        rv = build_power_subsystem_rdr(oh_handler, resource_id);
        if (rv != SA_OK) {
                err("build power subsystem RDR failed");
                return rv;
        }

        return SA_OK;
}

/**
 * build_power_supply_rpt
 *      @oh_handler:  Pointer to openhpi handler
 *      @name:        Pointer to the name of the Power supply
 *      @bay_number:  Bay number of the Power supply
 *      @resource_id: Pointer to resource id
 *
 * Purpose:
 *      Populate the power supply unit RPT.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_power_supply_rpt(struct oh_handler_state *oh_handler,
                                char *name,
                                SaHpiInt32T bay_number,
                                SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        char *entity_root = NULL;
        SaHpiRptEntryT rpt;

        if (oh_handler == NULL || name == NULL || resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the power supply */
        memset(&rpt, 0, sizeof(SaHpiRptEntryT));
        rpt.ResourceCapabilities = SAHPI_CAPABILITY_RDR |
                                   SAHPI_CAPABILITY_RESOURCE |
                                   SAHPI_CAPABILITY_FRU |
                                   SAHPI_CAPABILITY_SENSOR |
                                   SAHPI_CAPABILITY_INVENTORY_DATA;
        rpt.ResourceEntity.Entry[2].EntityType = SAHPI_ENT_ROOT;
        rpt.ResourceEntity.Entry[2].EntityLocation = 0;
        rpt.ResourceEntity.Entry[1].EntityType = SAHPI_ENT_POWER_MGMNT;
        rpt.ResourceEntity.Entry[1].EntityLocation = 1;
        rpt.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_POWER_SUPPLY;
        rpt.ResourceEntity.Entry[0].EntityLocation = bay_number;
        rv = oh_concat_ep(&rpt.ResourceEntity, &entity_path);
        if (rv != SA_OK) {
                err("concat of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        rpt.ResourceId = oh_uid_from_entity_path(&(rpt.ResourceEntity));
        rpt.ResourceInfo.ManufacturerId = HP_MANUFACTURING_ID;
        rpt.ResourceSeverity = SAHPI_OK;
        rpt.ResourceFailed = SAHPI_FALSE;
        rpt.HotSwapCapabilities = 0x0;
        rpt.ResourceTag.DataType = SAHPI_TL_TYPE_TEXT;
        rpt.ResourceTag.Language = SAHPI_LANG_ENGLISH;
        rpt.ResourceTag.DataLength = strlen(name) + 1;
        memset(rpt.ResourceTag.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        snprintf((char *) (rpt.ResourceTag.Data),
                 rpt.ResourceTag.DataLength, "%s", name);

        /* Add the power supply rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
        if (rv != SA_OK) {
                err("Failed to add power supply unit RPT");
                return rv;
        }

        *resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * build_power_supply_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @con:         Pointer to the soap client handler.
 *      @response:    Power supply info response structure
 *      @resource_id: Resource id
 *
 * Purpose:
 *      Populate the power supply unit RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the inventory RDR
 * 	- Creates the power status, operational status, predictive failure,
 * 	  internal data error, device location error, device failure error,
 * 	  device degraded error, AC failure, device not supported and device mix
 * 	  match sensor RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT build_power_supply_rdr(struct oh_handler_state *oh_handler,
                                SOAP_CON *con,
                                struct powerSupplyInfo *response,
                                SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        struct oa_soap_inventory *inventory = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
	struct getPowerSupplyStatus status_request;
	struct powerSupplyStatus status_response;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL || response == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Build inventory rdr for power supply */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = build_power_inv_rdr(oh_handler, response, &rdr, &inventory);
        if (rv != SA_OK) {
                err("Failed to get power supply unit inventory RDR");
                return rv;
        }
        rv = oh_add_rdr(oh_handler->rptcache, resource_id, &rdr, inventory, 0);
        if (rv != SA_OK) {
                err("Failed to add rdr");
                return rv;
        }

        /* Build power sensor rdr for power supply */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_PWR_STATUS)

	status_request.bayNumber = response->bayNumber;
	rv = soap_getPowerSupplyStatus(con, &status_request, &status_response);
	if (rv != SOAP_OK) {
		err("Get power supply status SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status_response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status_response.operationalStatus)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status_response.diagnosticChecks.
						internalDataError)

	/* Build device location error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_LOC_ERR,
					status_response.diagnosticChecks.
						deviceLocationError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status_response.diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status_response.diagnosticChecks.
						deviceDegraded)

	/* Build AC failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_AC_FAIL,
					status_response.diagnosticChecks.
						acFailure)

	/* Parse the diganosticChecksEx */
	oa_soap_parse_diag_ex(status_response.diagnosticChecksEx,
			      diag_ex_status);

	/* Build Device not supported sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_NOT_SUPPORT,
				diag_ex_status[DIAG_EX_DEV_NOT_SUPPORT])

	/* Build Device mix match sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_MIX_MATCH,
					diag_ex_status[DIAG_EX_DEV_MIX_MATCH])

        return SA_OK;
}

/**
 * discover_power_supply
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the power supply.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT discover_power_supply(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T i;
        struct getPowerSupplyInfo request;
        struct powerSupplyInfo response;
        char power_supply[] = POWER_SUPPLY_NAME;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.ps_unit.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getPowerSupplyInfo(oa_handler->active_con,
                                             &request, &response);
                if (rv != SOAP_OK) {
                        err("Get power supply info failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* If resource not present, continue checking for next bay */
                if (response.presence != PRESENT)
                        continue;

                /* If the power supply unit does not have the power cord
                 * plugged in, then power supply unit will be in faulty
                 * condition. In this case, all the information in the
                 * response structure is NULL. Consider the faulty power supply
                 *  unit as ABSENT
                 */
                if (response.serialNumber == NULL)
                        continue;

                /* Build the rpt entry for power supply unit */
                rv = build_power_supply_rpt(oh_handler, power_supply,
                                            i, &resource_id);
                if (rv != SA_OK) {
                        err("build power supply unit rpt failed");
                        return rv;
                }

                /* Update resource_status structure with resource_id,
                 * serial_number, and presence status
                 */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.ps_unit, i,
                      response.serialNumber, resource_id, RES_PRESENT);

                /* Build the rdr entry for power supply */
                rv = build_power_supply_rdr(oh_handler, oa_handler->active_con,
                                            &response, resource_id);
                if (rv != SA_OK) {
                        err("build power supply unit RDR failed");
                        /* Reset resource_status structure to default values */
                        oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.ps_unit, i,
                              "", SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                        return rv;
                }

        }
        return SA_OK;
}

/**
 * oa_soap_parse_diag_ex
 *      @diag_ex	: Pointer to diganosticChecksEx structure
 *	@diag_status_arr: Pointer to array of enum oa_soap_diag_ex
 *
 * Purpose:
 *      Parses the diagnosticChecksEx structure and extracts the value.
 *
 * Detailed Description:
 *	The fields of diagnosticChecksEx structure appear and disappear
 *	depending on the resource configuration and status. If a field is not
 *	available in the diagnosticChecksEx structure, then its value is
 *	considered as NO_ERROR.
 *	This function parses the diagnosticChecksEx structure and gets the value
 *	for the avaiable fields.
 *
 * Return values:
 *	NONE
 **/
void oa_soap_parse_diag_ex(xmlNode *diag_ex,
			   enum diagnosticStatus *diag_status_arr)
{
	struct diagnosticData diag_data;
	SaHpiInt32T i;

	if (diag_status_arr == NULL) {
		err("Invalid parameters");
		return;
	}

	/* Initialize the value array to NO_ERROR.
	 * The diagnosticChecksEx fields will change depending on the
	 * configuration and status. Hence, if one of more fields are not
	 * present, then it is considered not faulty
	 */
	for (i = 0; i < OA_SOAP_MAX_DIAG_EX; i++) {
		diag_status_arr[i] = NO_ERROR;
	}

	while (diag_ex) {
		soap_getDiagnosticChecksEx(diag_ex, &diag_data);
		for (i = 0; i < OA_SOAP_MAX_DIAG_EX; i++) {
			/* Compare the diagnosticChecksEx field name with field
			 * names in the array. */
			if (! strcmp(diag_data.name, oa_soap_diag_ex_arr[i])) {
				diag_status_arr[i] = diag_data.value;
				break;
			}
		}
		diag_ex = soap_next_node(diag_ex);
	}
	return;
}

/**
 * oa_soap_get_health_val
 *      @extra_data: Pointer to extraData structure
 *	@status	   : Pointer to enum oa_soap_extra_data_health
 *
 * Purpose:
 *      Parses the healthStatus field in extraData structure and extracts the
 *	value.
 *
 * Detailed Description:
 *	The fields of extraData structure appear and disappear depending on the
 *	resource configuration and status. If healthStatus field is not
 *	available in the extraData structure, then its value is considered as OK
 *	This function parses the extraData structure and gets the value
 *	of the healthStatus field.
 *
 * Return values:
 *	NONE
 **/
void oa_soap_get_health_val(xmlNode *extra_data,
			    enum oa_soap_extra_data_health *status)
{
	struct extraDataInfo extra_data_info;
	SaHpiInt32T i;

	if (status == NULL) {
		err("Invalid parameters");
		return;
	}

	/* Initialize status to OK */
	*status = HEALTH_OK;

	while (extra_data) {
		soap_getExtraData(extra_data, &extra_data_info);
		/* Check for the healthStatus field */
		if (! (strcmp(extra_data_info.name,
		       OA_SOAP_HEALTH_STATUS_STR))) {
			/* Got the healthStatus field */
			for (i = 0; i < OA_SOAP_MAX_HEALTH_ENUM; i++) {
				if (! strcmp(extra_data_info.value,
					     oa_soap_health_arr[i])) {
					/* Assign the healthStatus enum value */
					*status = i;
					break;
				}
			}
		}
		extra_data = soap_next_node(extra_data);
	}
	return;
}

/**
 * oa_soap_build_rpt
 *      @oh_handler	: Pointer to openhpi handler
 *      @resource_type	: Type of resource
 *      @location	: Device location
 *      @rpt		: Pointer to RPT entry
 *
 * Purpose:
 *	Generic function to build the RPT entry from the global rpt array
 *
 * Detailed Description:
 * 	- Gets the RPT entry from the global rpt array
 *	- Assigns the entity location in the entity path
 *	- Generates the resource id
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_build_rpt(struct oh_handler_state *oh_handler,
			   SaHpiInt32T resource_type,
			   SaHpiInt32T location,
                           SaHpiRptEntryT *rpt)
{
        SaErrorT rv = SA_OK;
        SaHpiEntityPathT entity_path;
        char *entity_root = NULL;

        if (oh_handler == NULL || rpt == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        entity_root = (char *) g_hash_table_lookup(oh_handler->config,
                                                   "entity_root");
        rv = oh_encode_entitypath(entity_root, &entity_path);
        if (rv != SA_OK) {
                err("Encoding entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        /* Populate the rpt with the details of the thermal subsystem */
        memset(rpt, 0, sizeof(SaHpiRptEntryT));
        *rpt = oa_soap_rpt_arr[resource_type];
        rv = oh_concat_ep(&(rpt->ResourceEntity), &entity_path);
        if (rv != SA_OK) {
                err("concatenation of entity path failed");
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

	/* Set the device location in RPT entry */
	if (location) 
		rpt->ResourceEntity.Entry[0].EntityLocation = location;

        rpt->ResourceId = oh_uid_from_entity_path(&(rpt->ResourceEntity));

        return SA_OK;
}

/**
 * oa_soap_build_therm_subsys_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *
 * Purpose:
 *      Populate the thermal subsystem RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 *	Creates the operational status, predictive failure and redundancy
 *	sensor RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_build_therm_subsys_rdr(struct oh_handler_state
						*oh_handler,
					       SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
	struct thermalSubsystemInfo response;
	struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
	SaHpiInt32T sensor_status;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	rv = soap_getThermalSubsystemInfo(oa_handler->active_con, &response);
	if (rv != SOAP_OK) {
		err("Get thermal subsystem info failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					response.operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					response.operationalStatus)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_REDUND,
					response.redundancy)

        return SA_OK;
}

/**
 * oa_soap_disc_therm_subsys
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the thermal subsystem.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_disc_therm_subsys(struct oh_handler_state *oh_handler)
{
	SaErrorT rv = SA_OK;
	struct oa_soap_handler *oa_handler = NULL;
	SaHpiRptEntryT rpt;

	if (oh_handler == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	/* Build the rpt entry for thermal subsystem */
	rv = oa_soap_build_rpt(oh_handler, OA_SOAP_ENT_THERM_SUBSYS, 0, &rpt);
	if (rv != SA_OK) {
		err("Build thermal subsystem rpt failed");
		return rv;
	}

        /* Add the thermal subsystem rpt to the plugin RPTable */
        rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
        if (rv != SA_OK) {
                err("Failed to add thermal subsystem RPT");
                return rv;
        }

	/* Build the rdr entry for thermal subsystem */
	rv = oa_soap_build_therm_subsys_rdr(oh_handler, rpt.ResourceId);
	if (rv != SA_OK) {
		err("Build thermal subsystem RDR failed");
		return rv;
	}

	/* Update resource_status structure with resource_id */
	oa_handler->oa_soap_resources.thermal_subsystem_rid = rpt.ResourceId; 
	return SA_OK;
}

/**
 * oa_soap_build_fz_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @resource_id: Resource id
 *      @fan_zone : Pointer to fanZone structure
 *
 * Purpose:
 *      Populate the fan zone RDR.
 *      Pushes the RDR entry to plugin RPTable
 *
 * Detailed Description:
 * 	Creates the operational status, predictive failure and redundancy
 * 	sensor RDR
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_build_fz_rdr(struct oh_handler_state *oh_handler,
				     SaHpiResourceIdT resource_id,
				     struct fanZone *fan_zone)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
	struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
	SaHpiInt32T sensor_status;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	/* Build the fan zone inventory rdr */
	rv = oa_soap_build_fz_inv(oh_handler, resource_id, fan_zone);
	if (rv != SA_OK) {
		err("Building inventory RDR for Fan Zone failed");
		return rv;
	}

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					fan_zone->operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					fan_zone->operationalStatus)

	/* Build redundancy sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_REDUND,
					fan_zone->redundant)

        return SA_OK;
}

/**
 * oa_soap_get_fz
 *      @oh_handler: Pointer to openhpi handler
 *      @max_fz    : Maximum fan zone supported by the enclosure
 *      @response  : Pointer to getFanZoneArrayResponse structure
 *
 * Purpose:
 *	Gets the fan zone array information
 *
 * Detailed Description:
 *	- Creates the request for getFanZoneArray SOAP call based on the maximum
 *	  fan zones
 *	- Makes the getFanZoneArray SOAP call
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_get_fz_arr(struct oa_soap_handler *oa_handler,
			    SaHpiInt32T max_fz,
			    struct getFanZoneArrayResponse *response)
{
	SaErrorT rv;
	struct getFanZoneArray request;
	struct bayArray bay_zones;
	SaHpiInt32T i;
	byte array[max_fz];

	if (oa_handler == NULL || response == NULL) {
		err("Invalid parameter");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* Create the bayArray for fanZoneArray request */
	for (i = 1; i <= max_fz; i++) {
		array[i - 1] = i;
	}

	bay_zones.array = array;
	bay_zones.size = max_fz;
	request.bayArray = bay_zones;
	rv = soap_getFanZoneArray(oa_handler->active_con, &request,
				  response);
	if (rv != SOAP_OK) {
		err("Get fan zone array SOAP call failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	return SA_OK;
}

/**
 * oa_soap_disc_fz
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the fan zone.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_disc_fz(struct oh_handler_state *oh_handler)
{
	SaErrorT rv = SA_OK;
	struct oa_soap_handler *oa_handler = NULL;
	SaHpiRptEntryT rpt;
	struct fanZone fan_zone;
	struct getFanZoneArrayResponse response;
	SaHpiInt32T max_fz, zone_number;

	if (oh_handler == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;
	max_fz = oa_handler->oa_soap_resources.fan_zone.max_bays;

	/* Get the Fan Zone array information */
	rv = oa_soap_get_fz_arr(oa_handler, max_fz, &response);
	if (rv != SA_OK) {
		err("Failed to get fan zone array");
		return rv;
	}

	while (response.fanZoneArray) {
		soap_fanZone(response.fanZoneArray, &fan_zone);

		zone_number = fan_zone.zoneNumber;
		/* Build the rpt entry for fan zone */
		rv = oa_soap_build_rpt(oh_handler, OA_SOAP_ENT_FZ, zone_number,
				       &rpt);
		if (rv != SA_OK) {
			err("Build fan zone rpt has failed");
			return rv;
		}

		/* Add the fan zone rpt to the plugin RPTable */
		rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
		if (rv != SA_OK) {
			err("Failed to add fan zone RPT");
			return rv;
		}

		/* Build the rdr entry for fan zone */
		rv = oa_soap_build_fz_rdr(oh_handler, rpt.ResourceId,
					  &fan_zone);
		if (rv != SA_OK) {
			err("Build fan zone RDR failed");
			return rv;
		}

		/* Update resource_status structure with resource_id */
		oa_handler->oa_soap_resources.fan_zone.
			resource_id[zone_number - 1] = rpt.ResourceId; 
		response.fanZoneArray = soap_next_node(response.fanZoneArray);
	}
	return SA_OK;
}

/**
 * oa_soap_build_fan_rpt
 *      @oh_handler	: Pointer to openhpi handler
 *      @bay_number	: Bay number of the Fan
 *      @resource_id	: Pointer to the resource id
 *
 * Purpose:
 *      Populate the fan RPT.
 *      Pushes the RPT entry to plugin RPTable
 *
 * Detailed Description:
 * 	- Creates the Fan RPT entry from the global array
 * 	- Puts the primary fan zone number of this fan in entity path
 * 	- Pushes the RPT entry to plugin RPTable
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_build_fan_rpt(struct oh_handler_state *oh_handler,
			       SaHpiInt32T bay_number,
			       SaHpiResourceIdT *resource_id)
{
        SaErrorT rv = SA_OK;
	SaHpiRptEntryT rpt;
	struct oa_soap_handler *oa_handler;

        if (oh_handler == NULL || resource_id == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	rv = oa_soap_build_rpt(oh_handler, OA_SOAP_ENT_FAN, bay_number, &rpt);
	if (rv != SA_OK) {
		err("Build fan rpt has failed");
		return rv;
	}

	/* Set the fan zone location in RPT entry */
	rpt.ResourceEntity.Entry[1].EntityLocation =
		oa_soap_fz_map_arr[oa_handler->enc_type][bay_number].zone;

	/* Add the fan rpt to the plugin RPTable */
	rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
	if (rv != SA_OK) {
		err("Failed to add fan RPT");
		return rv;
	}

	*resource_id = rpt.ResourceId;
        return SA_OK;
}

/**
 * oa_soap_build_fan_rdr
 *      @oh_handler:  Pointer to openhpi handler
 *      @con:         Pointer to the soap client handler.
 *      @response:    Fan info response structure
 *      @resource_id: Resource id
 *
 * Purpose:
 *	- Creates the inventory RDR.
 *	- Creates operational status, predictive failure, internal data error,
 *	  device location error, device failure error, device degraded and
 *	  device missing sensor RDR
 *	- Pushes the RDRs to plugin RPTable
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_build_fan_rdr(struct oh_handler_state *oh_handler,
			       SOAP_CON *con,
			       struct fanInfo *response,
			       SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
        struct oa_soap_sensor_info *sensor_info = NULL;
	SaHpiInt32T sensor_status;
	enum diagnosticStatus diag_ex_status[OA_SOAP_MAX_DIAG_EX];

        if (oh_handler == NULL || con == NULL || response == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Build inventory rdr for fan */
        memset(&rdr, 0, sizeof(SaHpiRdrT));
        rv = oa_soap_build_fan_inv(oh_handler, resource_id, response);
        if (rv != SA_OK) {
                err("Failed to build fan inventory RDR");
                return rv;
        }

        /* Build fan speed sensor rdr for fan */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_FAN_SPEED)

        /* Build power sensor rdr for fan */
	OA_SOAP_BUILD_SENSOR_RDR(OA_SOAP_SEN_PWR_STATUS)

	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					response->operationalStatus)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					response->operationalStatus)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					response->diagnosticChecks.
						internalDataError)

	/* Build device location error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_LOC_ERR,
					response->diagnosticChecks.
						deviceLocationError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					response->diagnosticChecks.
						deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					response->diagnosticChecks.
						deviceDegraded)

	oa_soap_parse_diag_ex(response->diagnosticChecksEx, diag_ex_status);

	/* Build device missing sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_MISS,
					diag_ex_status[DIAG_EX_DEV_MISS])

        return SA_OK;
}

/**
 * oa_soap_disc_fan
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the fan.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_disc_fan(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        struct oa_soap_handler *oa_handler = NULL;
        SaHpiInt32T i;
        struct getFanInfo request;
        struct fanInfo response;
        SaHpiResourceIdT resource_id;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oa_handler = (struct oa_soap_handler *) oh_handler->data;

        for (i = 1; i <= oa_handler->oa_soap_resources.fan.max_bays; i++) {
                request.bayNumber = i;
                rv = soap_getFanInfo(oa_handler->active_con,
                                     &request, &response);
                if (rv != SOAP_OK) {
                        err("Get Fan Info SOAP call failed");
                        return SA_ERR_HPI_INTERNAL_ERROR;
                }

                /* If resource not present, continue checking for next bay */
                if (response.presence != PRESENT)
                        continue;

                rv = oa_soap_build_fan_rpt(oh_handler, i, &resource_id);
                if (rv != SA_OK) {
                        err("Failed to build fan RPT");
			return rv;
		}

                /* Update resource_status structure with resource_id,
                 * serial_number, and presence status.  Fan doesn't have a
                 * serial number, so pass in a null string.
                 */
                oa_soap_update_resource_status(
                      &oa_handler->oa_soap_resources.fan, i,
                      NULL, resource_id, RES_PRESENT);

                rv = oa_soap_build_fan_rdr(oh_handler, oa_handler->active_con,
					   &response, resource_id);
                if (rv != SA_OK) {
                        err("Failed to build fan RDR");
                        /* Reset resource_status structure to default values */
                        oa_soap_update_resource_status(
                              &oa_handler->oa_soap_resources.fan, i,
                              NULL, SAHPI_UNSPECIFIED_RESOURCE_ID, RES_ABSENT);
                        return rv;
                }

        }
        return SA_OK;
}

/**
 * oa_soap_build_lcd_rdr
 *      @oh_handler:	Pointer to openhpi handler
 *      @resource_id:	resource id
 *
 * Purpose:
 * 	Builds the LCD RDR
 *
 * Detailed Description:
 *	- Creates the inventory RDR
 *	- Creates the operational status, predictive failure, internal data
 *	  error, device failure error, device degraded error, enclosure
 *	  aggregate operational status and enclosure aggregate  predictive
 *	  failure sernsor RDR
 *	- Pushes the RDR entry to plugin RPTable
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_build_lcd_rdr(struct oh_handler_state *oh_handler,
				      SaHpiResourceIdT resource_id)
{
        SaErrorT rv = SA_OK;
        SaHpiRdrT rdr;
	struct oa_soap_handler *oa_handler = NULL;
        struct oa_soap_sensor_info *sensor_info = NULL;
	struct lcdStatus status;
	SaHpiInt32T sensor_status;

        if (oh_handler == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	/* Build the LCD inventory RDR */
	rv = oa_soap_build_lcd_inv(oh_handler, resource_id);
	if (rv != SA_OK) {
		err("Building inventory RDR for LCD failed");
		return rv;
	}

	/* Build LCD button lock control rdr */
	OA_SOAP_BUILD_CONTROL_RDR(OA_SOAP_LCD_BUTN_LCK_CNTRL)

	/* Build operational status sensor rdr */
	rv = soap_getLcdStatus(oa_handler->active_con, &status);
	if (rv != SOAP_OK) {
		err("Get LCD status SOAP call has failed");
		return SA_ERR_HPI_INTERNAL_ERROR;
	}
	
	/* Build operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_OPER_STATUS,
					status.status)

	/* Build predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_PRED_FAIL,
					status.status)

	/* Build internal data error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_INT_DATA_ERR,
					status.diagnosticChecks.
						internalDataError)

	/* Build device failure error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_FAIL,
					status.diagnosticChecks.deviceFailure)

	/* Build device degraded error sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_DEV_DEGRAD,
					status.diagnosticChecks.deviceDegraded)

	/* Build enclosure aggregate operational status sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_ENC_AGR_OPER,
					status.lcdSetupHealth)

	/* Build enclosure aggregate predictive failure sensor rdr */
	OA_SOAP_BUILD_ENABLE_SENSOR_RDR(OA_SOAP_SEN_ENC_AGR_PRED_FAIL,
					status.lcdSetupHealth)

        return SA_OK;
}

/**
 * oa_soap_disc_lcd
 *      @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 *      Discover the fan zone.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static SaErrorT oa_soap_disc_lcd(struct oh_handler_state *oh_handler)
{
	SaErrorT rv = SA_OK;
	struct oa_soap_handler *oa_handler = NULL;
	SaHpiRptEntryT rpt;

	if (oh_handler == NULL) {
		err("Invalid parameters");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	oa_handler = (struct oa_soap_handler *) oh_handler->data;

	/* Build the rpt entry for lcd */
	rv = oa_soap_build_rpt(oh_handler, OA_SOAP_ENT_LCD, 0, &rpt);
	if (rv != SA_OK) {
		err("Build LCD rpt has failed");
		return rv;
	}

	/* Add the LCD rpt to the plugin RPTable */
	rv = oh_add_resource(oh_handler->rptcache, &rpt, NULL, 0);
	if (rv != SA_OK) {
		err("Failed to add LCD RPT");
		return rv;
	}

	/* Build the rdr entry for LCD */
	rv = oa_soap_build_lcd_rdr(oh_handler, rpt.ResourceId);
	if (rv != SA_OK) {
		err("Build LCD RDR failed");
		return rv;
	}

	/* Update resource_status structure with resource_id */
	oa_handler->oa_soap_resources.lcd_rid = rpt.ResourceId; 
	return SA_OK;
}

/**
 * oa_soap_populate_event
 *       @oh_handler:  Pointer to openhpi handler
 *       @resource_id: Resource Id
 *       @event:       Pointer to event structure
 *       @assert_sensors: Pointer to GSList
 *
 * Purpose:
 *      Populates the event structure with default values of the resource.
 *	If sensor is in assert state, then pushes the asserted sensor RDR
 *	to list.
 *
 * Detailed Description:
 * 	- Populates the event structure with default values of the resource
 * 	- If sensor is in assert state, then pushes the asserted sensor RDR to
 * 	  assert sensor list. This list is used for generating the sensor assert
 * 	  events
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
SaErrorT oa_soap_populate_event(struct oh_handler_state *oh_handler,
				SaHpiResourceIdT resource_id,
				struct oh_event *event,
				GSList **assert_sensors)
{
        SaHpiRptEntryT *rpt = NULL;
        SaHpiRdrT *rdr = NULL;
	struct oa_soap_sensor_info *sensor_info;
	SaHpiEventStateT state;
	SaHpiEventCategoryT evt_catg;

        if (oh_handler == NULL || event == NULL || assert_sensors == NULL) {
                err("Invalid parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        rpt = oh_get_resource_by_id(oh_handler->rptcache, resource_id);

        memset(event, 0, sizeof(struct oh_event));
        event->event.Source = rpt->ResourceId;
        oh_gettimeofday(&event->event.Timestamp);
        event->event.Severity = rpt->ResourceSeverity;
        event->resource = *rpt;
        event->hid = oh_handler->hid;

        /* Get the first RDR */
        rdr = oh_get_rdr_next(oh_handler->rptcache, rpt->ResourceId,
                                    SAHPI_FIRST_ENTRY);
        while (rdr) {
		/* Push the rdr to event rdrs list */
                event->rdrs =
			g_slist_append(event->rdrs, g_memdup(rdr,
							sizeof(SaHpiRdrT)));

		/* Check whether the RDR is of type sensor */
		if (rdr->RdrType == SAHPI_SENSOR_RDR) {
			sensor_info = (struct oa_soap_sensor_info*)
				oh_get_rdr_data(oh_handler->rptcache,
						resource_id,
					       	rdr->RecordId);

			/* Check whether the event is enabled or not */
			if (sensor_info->event_enable == SAHPI_TRUE) {
				state = sensor_info->current_state;
				evt_catg = rdr->RdrTypeUnion.SensorRec.Category;
				/* Check whether the sensor current state is
				 * asserted or not. Sensor is considered to be
				 * in assert state, if any one of the following
				 * 3 conditions are met:
				 *
				 * 1. event category = ENABLE and state =
				 * DISABLED
				 * 2. event category = PRED_FAIL and state =
				 * PRED_FAILURE_ASSERT
				 * 3. event category = THRESHOLD and state =
				 * UPPER_MAJOR or _UPPER_CRIT
				 */
				if ( (evt_catg == SAHPI_EC_ENABLE &&
				      state == SAHPI_ES_DISABLED) ||
				     (evt_catg == SAHPI_EC_PRED_FAIL &&
				      state == SAHPI_ES_PRED_FAILURE_ASSERT) ||
				     (evt_catg == SAHPI_EC_REDUNDANCY &&
				      state == SAHPI_ES_REDUNDANCY_LOST) ||
				     (evt_catg == SAHPI_EC_THRESHOLD &&
				      (state == SAHPI_ES_UPPER_MAJOR ||
				       state == SAHPI_ES_UPPER_CRIT)) ) {
					/* Push the sensor rdr to assert sensor
					 * list
					 */
					*assert_sensors =
						g_slist_append(*assert_sensors,
						       g_memdup(rdr,
							    sizeof(SaHpiRdrT)));
				}
			}
		}
		/* Get the next RDR */
		rdr = oh_get_rdr_next(oh_handler->rptcache, rpt->ResourceId,
                                      rdr->RecordId);
        }

        return SA_OK;
}

/**
 * oa_soap_push_disc_res
 *       @oh_handler: Pointer to openhpi handler
 *
 * Purpose:
 * 	Pushes the discovered resource entries to openhpi infrastructure
 *
 * Detailed Description:
 *	- Get the rpt entry of the resources one by one.
 *      - Creates the resource or hotswap event depending on the resource
 *        hotswap capability
 *      - Pushes the events to openhpi infrastructure
 *
 * Return values:
 *      SA_OK                     - on success.
 *      SA_ERR_HPI_OUT_OF_MEMORY  - on out of memory
 *      SA_ERR_HPI_INVALID_PARAMS - on wrong parameters
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure.
 **/
static void oa_soap_push_disc_res(struct oh_handler_state *oh_handler)
{
        SaErrorT rv = SA_OK;
        SaHpiRptEntryT *rpt = NULL;
        struct oh_event event;
        struct oa_soap_hotswap_state *hotswap_state = NULL;
	GSList *assert_sensor_list = NULL;

        if (oh_handler == NULL) {
                err("Invalid parameter");
                return;
        }

	/* Get the first resource */
        rpt = oh_get_resource_next(oh_handler->rptcache, SAHPI_FIRST_ENTRY);
        while (rpt) {
		/* Populate the event structure with default values and get the
		 * asserted sensor list 
		 */
                rv = oa_soap_populate_event(oh_handler, rpt->ResourceId, &event,
					    &assert_sensor_list);

                /* Check whether the resource has hotswap capability */
                if (event.resource.ResourceCapabilities &
                        SAHPI_CAPABILITY_MANAGED_HOTSWAP) {
                        /* Get the hotswap state and fill the current
                         * hotswap state
                         */
                        hotswap_state = (struct oa_soap_hotswap_state *)
                                oh_get_resource_data(oh_handler->rptcache,
                                                     event.resource.ResourceId);
                        if (hotswap_state == NULL) {
                                err("Failed to get server hotswap state");
                                return;
                        }
                        event.event.EventType = SAHPI_ET_HOTSWAP;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                hotswap_state->currentHsState;
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_UNKNOWN;
                } else if (event.resource.ResourceCapabilities &
                                SAHPI_CAPABILITY_FRU) {
                        /* The resource is FRU, but does not have hotswap
                         * capability Fill the current hotswap state as ACTIVE.
                         */
                        event.event.EventType = SAHPI_ET_HOTSWAP;
                        event.event.EventDataUnion.HotSwapEvent.HotSwapState =
                                SAHPI_HS_STATE_ACTIVE;
                        event.event.EventDataUnion.HotSwapEvent.
                                CauseOfStateChange = SAHPI_HS_CAUSE_UNKNOWN;
                } else {
                        /* The resource does not have FRU and hotswap
                         * capabilities. Raise the resrouce event.
                         */
                        event.event.EventType = SAHPI_ET_RESOURCE;
                        event.event.EventDataUnion.ResourceEvent.
                                ResourceEventType = SAHPI_RESE_RESOURCE_ADDED;
                }
                /* Push the event to OpenHPI */
                oh_evt_queue_push(oh_handler->eventq,
                                  copy_oa_soap_event(&event));

		/* If the assert sensro list is not empty, raise the assert
		 * sensor events
		 */
		if (assert_sensor_list != NULL) {
			/* Raise the assert sensor events */
			oa_soap_assert_sen_evt(oh_handler, rpt,
					       assert_sensor_list);
			/* Initialize the assert sensor list to NULL */
			assert_sensor_list = NULL;
		}

		/* Get the next resource */
                rpt = oh_get_resource_next(oh_handler->rptcache,
                                           rpt->ResourceId);
        } /* End of while loop */

        return;
}

/**
 * oa_soap_build_blade_thermal_rdr:
 *      @oh_handler		: Pointer to openhpi handler
 *      @thermal_response	: bladeThermalInfoArrayResponse response 
 *				  structure
 *	@rpt			: Pointer to rpt structure
 *	@name			: Blade name
 *
 * Purpose: Builds the thermal sensors of blade resource
 *	
 * Detailed Description: 
 *	- Parses the bladethermalInfoArray responses
 *	- For a particular blade type, then thermal sensor rdrs are built
 *	  based on static information available in 
 *	  "oa_soap_static_thermal_sensor_config" array for the blade type.
 *	- While the sensors are being built, if the soap response is NULL then
 *	  the sensor is left in the disable state, else the response is verified
 *	  to check whether the sensor can be enabled for monitoring and is 
 *	  enabled if monitoring is supported.
 *	- The response contains thermal info for different components, zones
 *	  inside the blade.
 *	- In addition, the bladeThermalInfo sensor of same type in response can
 *	  repeat (Like multiple system zones, cpu_zones, cpu information).
 *	  When there is such multiple instance of the bladeThermalInfo structure
 *	  in the response, sensor numbers are generated as follows:
 *		For each sensor type, a base sensor number is defined
 *		First occurrence of this sensor type in the response structure 
 *		is modeled with 
 *			sensor number = base sensor number
 *		Any later occurrences of the same sensor type in response is
 *		modeled with 
 *			sensor number = sensor number of previous instance of
 *					the same sensor type + 1 
 *	- Currently plugin considers maximum 4 occurrences of thermal info of 
 *	  same sensor type for thermal sensor modeling(For example only 4
 *	  system zone thermal sensors will be modeled even if the blade is able
 *	  to provide more than 4 thermal sensors of system zone type)
 * 	- Finally the bladeThermalInfo structure instance does not contain
 *	  any field identifier to unique distinguish itself into a particular
 *	  sensor type, hence the description field in the bladeThermalInfo 
 *	  structure is used as the key to distinguish into particular sensor
 *	  type category.
 *	- If this module is called during the discovery, then thermal sensors
 * 	  rdr are built for sensors supported by blade
 *
 * Return values:
 *      SA_OK                     - on success
 *      SA_ERR_HPI_INTERNAL_ERROR - on failure
 **/
SaErrorT oa_soap_build_blade_thermal_rdr(struct oh_handler_state *oh_handler,
					 struct bladeThermalInfoArrayResponse 
						response,
					 SaHpiRptEntryT *rpt,
					 char *name)
{
	SaErrorT rv = SA_OK;
	SaHpiBoolT bld_stable = SAHPI_FALSE;
	SaHpiInt32T i, j, sen_num, sen_count;
	enum oa_soap_blade_type bld_index = OTHER_BLADE_TYPE;
	SaHpiRdrT rdr;
	struct oa_soap_sensor_info *sensor_info = NULL;
	struct bladeThermalInfoArrayResponse temp_response;
	struct bladeThermalInfo bld_thrm_info;
	struct extraDataInfo extra_data_info;
	SaHpiSensorRecT *sensor = NULL;

	if (response.bladeThermalInfoArray != NULL)
		bld_stable = SAHPI_TRUE;

	/* Resolve the blade name to blade type enum .
	 * If the blade name did not match any of the existing blade type
	 * string, then it is considered OTHER_BLADE_TYPE
	 */
	for (i=0; i< OA_SOAP_MAX_BLD_TYPE -1 ; i++) {
		if (strstr(name, oa_soap_bld_type_str[i])) {
			bld_index = i;
			break;
		}
	}

	/* Fetch the thermal sensor information from static thermal sensor
	 * meant for blade type under discovery 
	 */
	for (i = 0; i < OA_SOAP_MAX_THRM_SEN; i++) {
		sen_count = oa_soap_static_thrm_sen_config[bld_index]
						[i].sensor_count;
		/* Do not add any sensor rdr if the sensor count is zero */
		if (sen_count == 0) 
			continue;

		for (j = 0; j< sen_count; j++) {
			memset(&rdr, 0, sizeof(SaHpiRdrT));	
			sen_num =
				oa_soap_static_thrm_sen_config[bld_index][i].
					base_sen_num + j;
			rv = oa_soap_build_sen_rdr(oh_handler, rpt->ResourceId,
						   &rdr, &sensor_info, sen_num);
			if (rv != SA_OK) {
				err("Failed to create rdr for sensor %x",
				     sen_num);
				return rv;
			}
			
			/* Initialize the sensor enable state as disabled */	
			sensor_info->sensor_enable = SAHPI_FALSE;
			if (bld_stable == SAHPI_FALSE) {
				dbg("Blade not in stable state, leaving sensor"
				    " in disable state");
			} else {
				/* Call the following function to retrieve 
				 * the correct instance of bladeThermalInfo 
				 * response.
				 */
				temp_response = response;
				rv = oa_soap_get_bld_thrm_sen_data(sen_num,
								temp_response,
								&bld_thrm_info);
								
				if (rv != SA_OK) {
					err("Could not find the matching"
					    " sensors info from blade");	
					return SA_ERR_HPI_INTERNAL_ERROR;
				}

				/* Check for the "SensorPresent" value in 
				 * bladeThermalInfo structure. 
				 * If the value is true, then enable the sensor
				 * built statically in previous step 
				 */
				soap_getExtraData(bld_thrm_info.extraData, 
							&extra_data_info);

				if ((extra_data_info.value != NULL) &&
				    (!(strcasecmp(extra_data_info.value, 
							"true")))) {
					sensor_info->sensor_enable = SAHPI_TRUE;

					sensor = &(rdr.RdrTypeUnion.SensorRec);
					/* Updating the rdr with actual upper 
					 * critical threshold value provided by
					 * OA
					 */
					sensor->DataFormat.Range.Max.Value.
								SensorFloat64 =
					sensor_info->threshold.UpCritical.Value.
								SensorFloat64 =
						bld_thrm_info.criticalThreshold;
			
					/* Updating the rdr with actual upper 
					 * caution threshold value provided by
					 * OA
					 */
					sensor->DataFormat.Range.NormalMax.Value.
								SensorFloat64 =
					sensor_info->threshold.UpMajor.Value.
								SensorFloat64 =
					bld_thrm_info.cautionThreshold;
				} else {
					dbg("Sensor %s not enabled for blade",
					    bld_thrm_info.description);
				}
			}
			
			rv = oh_add_rdr(oh_handler->rptcache, rpt->ResourceId,
					&rdr, sensor_info, 0);
			if (rv != SA_OK) {
				err("Failed to add rdr");
				return rv;
			}
		}
	}
	return SA_OK;
}


void * oh_discover_resources (void *)
                __attribute__ ((weak, alias("oa_soap_discover_resources")));
