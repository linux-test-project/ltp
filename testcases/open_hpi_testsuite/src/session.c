/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include <string.h>
#include <oh_utils.h>
#include <oh_error.h>
#include <oh_lock.h>
#include <oh_session.h>
#include <oh_domain.h>
#include <oh_config.h>

struct oh_session_table oh_sessions = {
        .table = NULL,
        .lock = G_STATIC_REC_MUTEX_INIT
};

static struct oh_event *oh_generate_hpi_event(void)
{
        struct oh_event *e = NULL;
        char *msg = "This session is being destroyed now!";

        e = g_new0(struct oh_event, 1);
        e->event.EventType = SAHPI_ET_HPI_SW;
        e->event.Source = SAHPI_UNSPECIFIED_RESOURCE_ID;
        e->event.Timestamp = SAHPI_TIME_UNSPECIFIED;
        e->event.Severity = SAHPI_CRITICAL;
        e->event.EventDataUnion.HpiSwEvent.MId =
		SAHPI_MANUFACTURER_ID_UNSPECIFIED;
        e->event.EventDataUnion.HpiSwEvent.Type =
		SAHPI_HPIE_OTHER;
        e->event.EventDataUnion.HpiSwEvent.EventData.DataType =
        	SAHPI_TL_TYPE_TEXT;
        e->event.EventDataUnion.HpiSwEvent.EventData.Language =
        	SAHPI_TL_TYPE_TEXT;
        strcpy((char *)e->event.EventDataUnion.HpiSwEvent.EventData.Data, msg);
        e->event.EventDataUnion.HpiSwEvent.EventData.DataLength	= strlen(msg);

        return e;
}

/**
 * oh_create_session
 * @did:
 *
 *
 *
 * Returns:
 **/
SaHpiSessionIdT oh_create_session(SaHpiDomainIdT did)
{
        struct oh_session *session = NULL;
        struct oh_domain *domain = NULL;
        static SaHpiSessionIdT id = 1;        /* Session ids will start at 1 */

        if (did == SAHPI_UNSPECIFIED_DOMAIN_ID)
                did = OH_DEFAULT_DOMAIN_ID;

        session = g_new0(struct oh_session, 1);
        if (!session)
                return 0;

        session->did = did;
        session->eventq = g_async_queue_new();
        session->subscribed = SAHPI_FALSE;

        domain = oh_get_domain(did);
        if (!domain) {
                g_async_queue_unref(session->eventq);
                g_free(session);
                return 0;
        }
        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session->id = id++;
        g_hash_table_insert(oh_sessions.table, &(session->id), session);
        oh_sessions.list = g_slist_append(oh_sessions.list, session);
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */
        oh_release_domain(domain);

        return session->id;
}

/**
 * oh_get_session_domain
 * @sid:
 *
 *
 *
 * Returns: SAHPI_UNSPECIFIED_DOMAIN_ID if domain id was not found.
 **/
SaHpiDomainIdT oh_get_session_domain(SaHpiSessionIdT sid)
{
        struct oh_session *session = NULL;
        SaHpiDomainIdT did;

        if (sid < 1)
                return SAHPI_UNSPECIFIED_DOMAIN_ID;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                return SAHPI_UNSPECIFIED_DOMAIN_ID;
        }

        did = session->did;
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */


        return did;
}

/**
 * oh_list_sessions
 * @did:
 *
 *
 *
 * Returns: A dynamically allocated array of session ids.
 * The caller needs to free this array when he is done with it.
 **/
GArray *oh_list_sessions(SaHpiDomainIdT did)
{
        struct oh_domain *domain = NULL;
        GArray *session_ids = NULL;
        GSList *node = NULL;

        if (did == SAHPI_UNSPECIFIED_DOMAIN_ID)
                did = OH_DEFAULT_DOMAIN_ID;

        domain = oh_get_domain(did);
        if (!domain)
                return NULL;

        session_ids = g_array_new(FALSE, TRUE, sizeof(SaHpiSessionIdT));

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        for (node = oh_sessions.list; node; node = node->next) {
                struct oh_session *s = node->data;
                if (s->did != did) continue;
                g_array_append_val(session_ids, s->id);
        }
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */
        oh_release_domain(domain);

        return session_ids;
}

/**
 * oh_get_session_state
 * @sid:
 * @state:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_get_session_subscription(SaHpiSessionIdT sid,
                                     SaHpiBoolT * state)
{
        struct oh_session *session = NULL;        
                
        if (sid < 1)
        	return SA_ERR_HPI_INVALID_SESSION;
        	
	if (state == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;       
                
        if (oh_sessions.table == NULL)
        	return SA_ERR_HPI_INTERNAL_ERROR;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                return SA_ERR_HPI_INVALID_SESSION;
        }
        *state = session->subscribed;
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */

        return SA_OK;
}

/**
 * oh_set_session_state
 * @sid:
 * @state:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_set_session_subscription(SaHpiSessionIdT sid, SaHpiBoolT state)
{
        struct oh_session *session = NULL;
        struct oh_event e;

        if (sid < 1)
                return SA_ERR_HPI_INVALID_PARAMS;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                return SA_ERR_HPI_INVALID_SESSION;
        }
        session->subscribed = state;

        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */
        /* Flush session's event queue
         */
        if (state == SAHPI_FALSE) {
                while (oh_dequeue_session_event(sid,
                                                SAHPI_TIMEOUT_IMMEDIATE,
                                                &e, NULL) == SA_OK) {
			oh_event_free(&e, TRUE);
		}
        }
        return SA_OK;
}

/**
 * oh_queue_session_event
 * @sid:
 * @event:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_queue_session_event(SaHpiSessionIdT sid,
                                struct oh_event * event)
{
        struct oh_session *session = NULL;
        struct oh_event *qevent = NULL;
        struct oh_global_param param = {.type = OPENHPI_EVT_QUEUE_LIMIT };

        if (sid < 1 || !event)
                return SA_ERR_HPI_INVALID_PARAMS;

        qevent = oh_dup_event(event);
        if (!qevent)
                return SA_ERR_HPI_OUT_OF_MEMORY;

        if (oh_get_global_param(&param))
                param.u.evt_queue_limit = OH_MAX_EVT_QUEUE_LIMIT;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                oh_event_free(qevent, FALSE);
                return SA_ERR_HPI_INVALID_SESSION;
        }

        if (param.u.evt_queue_limit != OH_MAX_EVT_QUEUE_LIMIT) {
                SaHpiSessionIdT tmp_sid = session->id;
                gint qlength = g_async_queue_length(session->eventq);
                if (qlength > 0 && qlength >= param.u.evt_queue_limit) {
                        /* Don't proceed with event push if queue is overflowed */
                        session->eventq_status = SAHPI_EVT_QUEUE_OVERFLOW;
                        g_static_rec_mutex_unlock(&oh_sessions.lock);
                        oh_event_free(qevent, FALSE);
                        err("Session %d's queue is out of space; "
                            "# of events is %d; Max is %d",
                            tmp_sid, qlength, param.u.evt_queue_limit);
                        return SA_ERR_HPI_OUT_OF_SPACE;
                }
        }

        g_async_queue_push(session->eventq, qevent);
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */

        return SA_OK;
}

/**
 * oh_dequeue_session_event
 * @sid:
 * @event:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_dequeue_session_event(SaHpiSessionIdT sid,
                                  SaHpiTimeoutT timeout,
                                  struct oh_event * event,
                                  SaHpiEvtQueueStatusT * eventq_status)
{
        struct oh_session *session = NULL;
        struct oh_event *devent = NULL;
        GTimeVal gfinaltime;
        GAsyncQueue *eventq = NULL;
        SaHpiBoolT subscribed;
        SaErrorT invalid;

        if (sid < 1 || (event == NULL))
                return SA_ERR_HPI_INVALID_PARAMS;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                return SA_ERR_HPI_INVALID_SESSION;
        }

        if (eventq_status) {
                *eventq_status = session->eventq_status;
        }
        session->eventq_status = 0;
        eventq = session->eventq;
        g_async_queue_ref(eventq);
        g_static_rec_mutex_unlock(&oh_sessions.lock);

        if (timeout == SAHPI_TIMEOUT_IMMEDIATE) {
                devent = g_async_queue_try_pop(eventq);
        } else if (timeout == SAHPI_TIMEOUT_BLOCK) {
                while (devent == NULL) {
                        g_get_current_time(&gfinaltime);
                        g_time_val_add(&gfinaltime, 5000000L);
                        devent =
                            g_async_queue_timed_pop(eventq, &gfinaltime);
                        /* compliance with spec page 63 */
                        invalid =
                            oh_get_session_subscription(sid, &subscribed);
                        /* Is the session still open? or still subscribed? */
                        if (invalid || !subscribed) {
                                g_async_queue_unref(eventq);
                                oh_event_free(devent, FALSE);
                                return invalid ? SA_ERR_HPI_INVALID_SESSION
                                    : SA_ERR_HPI_INVALID_REQUEST;
                        }
                }
        } else {
                g_get_current_time(&gfinaltime);
                g_time_val_add(&gfinaltime, (glong) (timeout / 1000));
                devent = g_async_queue_timed_pop(eventq, &gfinaltime);
                invalid = oh_get_session_subscription(sid, &subscribed);
                if (invalid || !subscribed) {
                        g_async_queue_unref(eventq);
                        oh_event_free(devent, FALSE);
                        return invalid ? SA_ERR_HPI_INVALID_SESSION :
                            SA_ERR_HPI_INVALID_REQUEST;
                }
        }
        g_async_queue_unref(eventq);

        if (devent) {
                memcpy(event, devent, sizeof(struct oh_event));
                g_free(devent);
                return SA_OK;
        } else {
                memset(event, 0, sizeof(struct oh_event));
                return SA_ERR_HPI_TIMEOUT;
        }
}

/**
 * oh_destroy_session
 * @sid:
 * @update_domain:
 *
 *
 * Returns:
 **/
SaErrorT oh_destroy_session(SaHpiSessionIdT sid)
{
        struct oh_session *session = NULL;
        SaHpiDomainIdT did;
        gpointer event = NULL;
        int i, len;

        if (sid < 1)
                return SA_ERR_HPI_INVALID_PARAMS;

        g_static_rec_mutex_lock(&oh_sessions.lock); /* Locked session table */
        session = g_hash_table_lookup(oh_sessions.table, &sid);
        if (!session) {
                g_static_rec_mutex_unlock(&oh_sessions.lock);
                return SA_ERR_HPI_INVALID_SESSION;
        }
        oh_sessions.list = g_slist_remove(oh_sessions.list, session);
        g_hash_table_remove(oh_sessions.table, &(session->id));
        g_static_rec_mutex_unlock(&oh_sessions.lock); /* Unlocked session table */
        did = session->did;

        /* Finalize session */
        len = g_async_queue_length(session->eventq);
        if (len > 0) {
                for (i = 0; i < len; i++) {
                        event = g_async_queue_try_pop(session->eventq);
                        if (event)
                                oh_event_free(event, FALSE);
                        event = NULL;
                }
        }
        g_async_queue_unref(session->eventq);
        g_free(session);

        return SA_OK;
}

static void __delete_by_did(gpointer key, gpointer value,
                            gpointer user_data)
{
        SaHpiDomainIdT did = *((SaHpiDomainIdT *) user_data);
        struct oh_session *session = (struct oh_session *) value;
        int i, len;
        gpointer event = NULL;

        if (!session) {
                err("Session does not exist!");
                return;
        }

        if (session->did != did) {
                return;
        }

        oh_sessions.list = g_slist_remove(oh_sessions.list, session);
        g_hash_table_remove(oh_sessions.table, &(session->id));

        /* Neutralize event queue */
        len = g_async_queue_length(session->eventq);
        if (len < 0) {
                for (i = 0; i > len; i--) {
                        g_async_queue_push(session->eventq,
                                           oh_generate_hpi_event());
                }
        } else if (len > 0) {
                for (i = 0; i < len; i++) {
                        event = g_async_queue_try_pop(session->eventq);
                        if (event)
                                oh_event_free(event, FALSE);
                        event = NULL;
                }
        }
        g_async_queue_unref(session->eventq);
        g_free(session);
}

SaErrorT oh_destroy_domain_sessions(SaHpiDomainIdT did)
{
        if (did == SAHPI_UNSPECIFIED_DOMAIN_ID)
                did = OH_DEFAULT_DOMAIN_ID;

        g_static_rec_mutex_lock(&oh_sessions.lock);
        g_hash_table_foreach(oh_sessions.table, __delete_by_did, &did);
        g_static_rec_mutex_unlock(&oh_sessions.lock);

        return SA_OK;
}

