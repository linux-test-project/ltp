/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Sean Dague <sdague@users.sf.net>
 *     Renier Morales <renier@openhpi.org>
 */

#ifndef __SAHPIMACROS_H
#define __SAHPIMACROS_H

/**************************************************************
 *
 *  These macros are defined for clarity of the sahpi.c
 *  source file.  They provide standard mechanisms for
 *  checking for and populating standard types, as well
 *  as providing consistent debug and error functionality.
 *
 *  Yes, macros are evil, but they make this code much
 *  more readable.
 *
 ***********************************************************/

#define OH_CHECK_INIT_STATE(sid) \
        { \
                SaHpiBoolT state; \
                SaErrorT init_error; \
                if ((init_error = oh_get_session_subscription(sid,&state)) != SA_OK) { \
                        err("Init state check failed! (%s, %d)", oh_lookup_error(init_error), sid); \
                        return init_error; \
                } \
        }


/*
 * OH_GET_DID gets the domain id of a session and
 * checks its validity (nonzero). *
 */
#define OH_GET_DID(sid, did) \
        { \
                did = oh_get_session_domain(sid); \
                if (did == SAHPI_UNSPECIFIED_DOMAIN_ID) { \
                        err("No domain for session id %d",sid); \
                        return SA_ERR_HPI_INVALID_SESSION; \
                } \
        }

/*
 * OH_GET_DOMAIN gets the domain object which locks it.
 * Need to call oh_release_domain(domain) after this to unlock it.
 */
#define OH_GET_DOMAIN(did, d) \
        { \
                if (!(d = oh_get_domain(did))) { \
                        err("Domain %d doesn't exist", did); \
                        return SA_ERR_HPI_INVALID_DOMAIN; \
                } \
        }

/*
 * OH_HANDLER_GET gets the hander for the rpt and resource id.  It
 * returns INVALID PARAMS if the handler isn't there
 */
#define OH_HANDLER_GET(d, rid, h) \
        { \
                unsigned int *hid = NULL; \
                hid = oh_get_resource_data(&(d->rpt), rid); \
                if (!hid) { \
                        err("Can't find handler for Resource %d in Domain %d", rid, d->id); \
                        oh_release_domain(d); \
                        return SA_ERR_HPI_INVALID_RESOURCE; \
                } \
                h = oh_get_handler(*hid); \
		if (h && !h->hnd) { \
			oh_release_handler(h); \
			h = NULL; \
		} \
        }

/*
 * OH_RESOURCE_GET gets the resource for an resource id and rpt
 * it returns invalid resource if no resource id is found
 */

#define OH_RESOURCE_GET(d, rid, r) \
        { \
                r = oh_get_resource_by_id(&(d->rpt), rid); \
                if (!r) { \
                        err("Resource %d in Domain %d doesn't exist", rid, d->id); \
                        oh_release_domain(d); \
                        return SA_ERR_HPI_INVALID_RESOURCE; \
                } \
        }

/*
 * OH_RESOURCE_GET_CHECK gets the resource for an resource id and rpt
 * it returns invalid resource if no resource id is found. It will
 * return NO_RESPONSE if the resource is marked as being failed.
 */
#define OH_RESOURCE_GET_CHECK(d, rid, r) \
        { \
                r = oh_get_resource_by_id(&(d->rpt), rid); \
                if (!r) { \
                        err("Resource %d in Domain %d doesn't exist", rid, d->id); \
                        oh_release_domain(d); \
                        return SA_ERR_HPI_INVALID_RESOURCE; \
                } else if (r->ResourceFailed != SAHPI_FALSE) { \
                        err("Resource %d in Domain %d is Failed", rid, d->id); \
                        oh_release_domain(d); \
                        return SA_ERR_HPI_NO_RESPONSE; \
                } \
        }

/*
 * OH_CALL_ABI will check for a valid handler struct and existing plugin abi.
 * If a valid abi or handler is not found, it returns error. Once it passes
 * this validity check, it will call the plugin abi function with the passed
 * parameters. 
 */
#define OH_CALL_ABI(handler, func, err, ret, params...) \
	{ \
		if (!handler || !handler->abi->func) { \
                	oh_release_handler(handler); \
                	return err; \
        	} \
        	ret = handler->abi->func(handler->hnd, params); \
        }

#endif
