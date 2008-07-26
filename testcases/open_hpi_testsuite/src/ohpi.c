/*      -*- linux-c -*-
 *
 * (C) Copright IBM Corp 2004,2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Renier Morales <renier@openhpi.org>
 */

#include <oHpi.h>
#include <oh_config.h>
#include <oh_init.h>
#include <oh_plugin.h>
#include <oh_event.h>
#include <oh_domain.h>
#include <oh_session.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_lock.h>
#include <sahpimacros.h>
#include <config.h>


/**
 * oHpiVersionGet
 *
 * Returns the version of the library as a SaHpiUint64T type.
 * The version consists of 3 16-bit integers: MAJOR, MINOR, and PATCH.
 */

SaHpiUint64T oHpiVersionGet()
{
        SaHpiUint64T v = 0;

	OHPI_VERSION_GET(v, VERSION);

        return v;
}

/* Handler operations */

/**
 * oHpiHandlerCreate
 * @config: IN. Hash table. Holds configuration information used by handler.
 * @id: IN/OUT. The id of the newly created handler is returned here.
 *
 * Creates a new handler (instance of a plugin). Plugin handlers are what
 * respond to most API calls.
 * @config needs to have an entry for "plugin" in order to know for which
 * plugin the handler is being created.
 *
 * Returns: SA_OK on success. SA_ERR_HPI_INTERNAL_ERROR if a handler is
 * created, but failed to open. oHpiHandlerRetry can be used to retry
 * opening the handler.
 **/
SaErrorT oHpiHandlerCreate(GHashTable *config,
                           oHpiHandlerIdT *id)
{
	SaErrorT error = SA_OK;

        if (!config || !id) {
                err("Invalid parameters.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        if (oh_init()) return SA_ERR_HPI_ERROR;

	error = oh_create_handler(config, id);

	return error;
}

/**
 * oHpiHandlerDestroy
 * @id: IN. The id of the handler to destroy
 *
 * Destroys a handler. Calls the plugin's abi close function.
 *
 * Returns: SA_OK on success. Minus SA_OK on error.
 **/
SaErrorT oHpiHandlerDestroy(oHpiHandlerIdT id)
{
        if (!id)
                return SA_ERR_HPI_INVALID_PARAMS;
                
        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        if (oh_destroy_handler(id))
                return SA_ERR_HPI_ERROR;

        return SA_OK;
}

/**
 * oHpiHandlerInfo
 * @id: IN. The id of the handler to query
 * @info: IN/OUT. Pointer to struct for holding handler information
 *
 * Queries a handler for the information associated with it.
 *
 * Returns: SA_OK on success. Minus SA_OK on error.
 **/
SaErrorT oHpiHandlerInfo(oHpiHandlerIdT id, oHpiHandlerInfoT *info)
{
        struct oh_handler *h = NULL;

        if (!id || !info)
               return SA_ERR_HPI_INVALID_PARAMS;

	if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        h = oh_get_handler(id);
        if (!h) {
                err("Handler %d not found.", id);
                return SA_ERR_HPI_NOT_PRESENT;
        }

	info->id = id;
        strncpy(info->plugin_name, h->plugin_name, MAX_PLUGIN_NAME_LENGTH);
	oh_encode_entitypath((const char *)g_hash_table_lookup(h->config, "entity_root"),
			     &info->entity_root);
	
	if (!h->hnd) info->load_failed = 1;
	else info->load_failed = 0;

        oh_release_handler(h);

        return SA_OK;
}

/**
 * oHpiHandlerGetNext
 * @id: IN. Id of handler to search for.
 * @next_id: IN/OUT. The id of the handler next to the handler being searched for
 * will be returned here.
 *
 * Used for iterating through all loaded handlers. If you pass
 * 0 (SAHPI_FIRST_ENTRY), you will get the id of the first handler returned
 * in next_id.
 *
 * Returns: SA_OK on success. Minus SA_OK on error.
 **/
SaErrorT oHpiHandlerGetNext(oHpiHandlerIdT id, oHpiHandlerIdT *next_id)
{
        if (!next_id) {
                err("Invalid parameters.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        if (oh_getnext_handler_id(id, next_id))
                return SA_ERR_HPI_NOT_PRESENT;

        return SA_OK;
}

/**
 * oHpiHandlerFind
 * @sid: a valid session id
 * @rid: resource id
 * @id: pointer where handler id found will be placed.
 *
 * Inputs are the @sid and @rid. @rid corresponds to some resource available
 * in that session. The function then will return the handler that served such
 * resource.
 *
 * Returns: SA_OK if handler was found.
 **/
SaErrorT oHpiHandlerFind(SaHpiSessionIdT sid,
			 SaHpiResourceIdT rid,
			 oHpiHandlerIdT *id)
{
	SaHpiDomainIdT did;
        struct oh_domain *d = NULL;
        unsigned int *hid = NULL;

        OH_CHECK_INIT_STATE(sid);
        OH_GET_DID(sid, did);

        if (sid == 0 || rid == 0 || !id) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        OH_GET_DOMAIN(did, d); /* Lock domain */

        hid = (unsigned int *)oh_get_resource_data(&d->rpt, rid);

        if (hid == NULL) {
                err("No such Resource Id %d in Domain %d", rid, did);
                oh_release_domain(d); /* Unlock domain */
                return SA_ERR_HPI_INVALID_RESOURCE;
        }

        *id = *hid;
        oh_release_domain(d); /* Unlock domain */

        return SA_OK;
}

/**
 * oHpiHandlerRetry
 * @id: handler id
 *
 * Returns: SA_OK if handler opens successfully.
 **/
SaErrorT oHpiHandlerRetry(oHpiHandlerIdT id)
{
	struct oh_handler *h = NULL;
	SaErrorT error = SA_OK;

	if (id == 0) return SA_ERR_HPI_INVALID_PARAMS;

        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

	h = oh_get_handler(id);
	if (!h) return SA_ERR_HPI_NOT_PRESENT;

	if (h->hnd != NULL) {
		oh_release_handler(h);
		return SA_OK;
	}

	h->hnd = h->abi->open(h->config, h->id, &oh_process_q);
	if (h->hnd == NULL) error = SA_ERR_HPI_INTERNAL_ERROR;
	else error = SA_OK;

	oh_release_handler(h);

	return error;
}

/* Global parameters */

/**
 * oHpiGlobalParamGet
 * @param: param->type needs to be set to know what parameter to fetch.
 *
 * Gets the value of the specified global parameter.
 *
 * Returns: SA_OK on success. Minus SA_OK on error.
 **/
SaErrorT oHpiGlobalParamGet(oHpiGlobalParamT *param)
{
        struct oh_global_param p;

        if (!param || !param->Type) {
                err("Invalid parameters. oHpiGlobalParamGet()");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        p.type = param->Type;

        if (oh_get_global_param(&p))
                return SA_ERR_HPI_UNKNOWN;

        memcpy(&param->u, &p.u, sizeof(oh_global_param_union));

        return SA_OK;
}

/**
 * oHpiGlobalParamSet
 * @param: param->type needs to be set to know what parameter to set.
 * Also, the appropiate value in param->u needs to be filled in.
 *
 * Sets a global parameter.
 *
 * Returns: SA_OK on success. Minus SA_OK on error.
 **/
SaErrorT oHpiGlobalParamSet(oHpiGlobalParamT *param)
{
        struct oh_global_param p;

        if (!param || !param->Type) {
                err("Invalid parameters. oHpiGlobalParamSet()");
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

        p.type = param->Type;
        memcpy(&p.u, &param->u, sizeof(oh_global_param_union));

        if (oh_set_global_param(&p))
                return SA_ERR_HPI_ERROR;

        return SA_OK;
}

/**
 * oHpiInjectEvent
 * @id: id of handler into which the event will be injected.
 * @event: pointer to the event to be injected.
 * @rpte: pointer to the resource to be injected.
 * @rdrs: pointer to the list of RDRs to be injected along with @resoruce
 *
 * @id and @event are required parameters. @rpte is only required if the event
 * is of RESOURCE type or HOTSWAP type. @rdrs is an optional argument in all
 * cases and can be NULL. If @rdrs is passed, it will be copied. It is the
 * responsibility of the caller to clean up the RDRs list once it is used here.
 *
 * Returns: SA_OK on success. This call will set the event.Source, rpte.ResourceId,
 * rpte.ResourceEntity so that the caller knows what the final assigned values were.
 * For rpte.ResourceEntity, the entity_root configuration parameter for the plugin
 * is used to complete it. In addition, for each rdr in @rdrs, a Num, RecordId,
 * and Entity will be assigned. This will also be reflected in the passed @rdrs
 * list so that the caller can know what the assigned values were.
 **/
SaErrorT oHpiInjectEvent(oHpiHandlerIdT id,
                         SaHpiEventT *event,
                         SaHpiRptEntryT *rpte,
                         SaHpiRdrT *rdr)
{
	SaErrorT (*inject_event)(void *hnd,
                            	 SaHpiEventT *evt,
                            	 SaHpiRptEntryT *rpte,
                                 SaHpiRdrT *rdr); /* TODO: Allow for an array/list of RDRs */

	struct oh_handler *h = NULL;
	SaErrorT error = SA_OK;

	if (!id) {
		err("Invalid handler id %d passed",id);
		return SA_ERR_HPI_INVALID_PARAMS;
	} else if (!event) {
		err("Invalid NULL event passed");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	if (oh_init()) return SA_ERR_HPI_INTERNAL_ERROR;

	h = oh_get_handler(id);
	inject_event = h ? h->abi->inject_event : NULL;
        if (!inject_event) {
                oh_release_handler(h);
                return SA_ERR_HPI_INVALID_CMD;
        }

	error = inject_event(h->hnd, event, rpte, rdr);
        if (error) {
                err("Event injection into handler %d failed", id);
        }

        oh_release_handler(h);
        return error;
}

