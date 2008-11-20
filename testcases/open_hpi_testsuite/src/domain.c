/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2006
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

#include <oh_domain.h>
#include <oh_alarm.h>
#include <oh_config.h>
#include <oh_error.h>
#include <oh_plugin.h>
#include <oh_event.h>
#include <oh_utils.h>
#include <string.h>
#include <sys/time.h>

#define domains_lock() g_static_rec_mutex_lock(&oh_domains.lock)
#define domains_unlock() g_static_rec_mutex_unlock(&oh_domains.lock)

struct oh_domain_table oh_domains = {
        .table = NULL,
        .lock = G_STATIC_REC_MUTEX_INIT,
};


static void __inc_domain_refcount(struct oh_domain *d)
{
        g_static_rec_mutex_lock(&d->refcount_lock);
        d->refcount++;
        g_static_rec_mutex_unlock(&d->refcount_lock);

        return;
}

static void __dec_domain_refcount(struct oh_domain *d)
{
        g_static_rec_mutex_lock(&d->refcount_lock);
        d->refcount--;
        g_static_rec_mutex_unlock(&d->refcount_lock);

        return;
}

static void __free_drt_list(GSList *drt_list)
{
        GSList *node = NULL;

        for (node = drt_list; node; node = node->next) {
                g_free(node->data);
        }
        g_slist_free(drt_list);

        return;
}

static void __delete_domain(struct oh_domain *d)
{
        oh_flush_rpt(&d->rpt);
        oh_el_close(d->del);
        oh_close_alarmtable(d);
        __free_drt_list(d->drt.list);
        g_static_rec_mutex_free(&d->lock);
        g_static_rec_mutex_free(&d->refcount_lock);
        g_free(d);
}
#if 0
static void __query_domains(gpointer key, gpointer value, gpointer user_data)
{
        oh_domain_result dr;
        GList *node = (GList *)value;
        struct oh_domain *domain = (struct oh_domain *)node->data;
        GArray *data = (GArray *)user_data;

        dr.id = domain->id;
        dr.entity_pattern = domain->entity_pattern;
        dr.tag = domain->tag;

        g_array_append_val(data, dr);
}
#endif

static GList *__get_domain(SaHpiDomainIdT did)
{
        GList *node = NULL;
        struct oh_domain *domain = NULL;

        if (did == SAHPI_UNSPECIFIED_DOMAIN_ID) {
                did = OH_DEFAULT_DOMAIN_ID;
        }

        domains_lock();
        node = (GList *)g_hash_table_lookup(oh_domains.table, &did);
        if (!node) {
                domains_unlock();
                return NULL;
        }

        domain = (struct oh_domain *)node->data;

        /* Punch in */
        __inc_domain_refcount(domain);
        /* Unlock domain table */
        domains_unlock();
        /* Wait to get domain lock */
        g_static_rec_mutex_lock(&domain->lock);

        return node;
}

static void gen_domain_event(SaHpiDomainIdT target_id,
                             SaHpiDomainIdT subject_id,
                             SaHpiBoolT addition)
{
        struct oh_event *e = NULL;
        struct timeval tv1;
        SaHpiDomainEventTypeT type =
                (addition) ? SAHPI_DOMAIN_REF_ADDED : SAHPI_DOMAIN_REF_REMOVED;

        e = g_new0(struct oh_event, 1);
        e->resource.ResourceId = SAHPI_UNSPECIFIED_RESOURCE_ID;
        e->event.Source = target_id;
        e->event.EventType = SAHPI_ET_DOMAIN;
        e->event.Severity = SAHPI_INFORMATIONAL;
        e->event.EventDataUnion.DomainEvent.Type = type;
        e->event.EventDataUnion.DomainEvent.DomainId = subject_id;
        gettimeofday(&tv1, NULL);
        e->event.Timestamp = (SaHpiTimeT) tv1.tv_sec *
                        1000000000 + tv1.tv_usec * 1000;
        dbg("domain %d %s domain %d", subject_id,
            type == SAHPI_DOMAIN_REF_ADDED ? "added to" : "removed from",
            target_id);
        oh_evt_queue_push(&oh_process_q, e);
}

static void update_drt(SaHpiDomainIdT target_id,
                       SaHpiDomainIdT subject_id,
                       SaHpiBoolT subject_is_peer,
                       SaHpiBoolT addition)
{
        struct oh_domain *domain = NULL;
        SaHpiDrtEntryT *drtentry = NULL;
        struct timeval tv1;
        int found = 0;

        domain = oh_get_domain(target_id);
        if (!domain) {
                err("Warning. Could not update DRT. Domain %u not found.",
                    target_id);
                return;
        }

        if (addition) {
                drtentry = g_new0(SaHpiDrtEntryT, 1);
                drtentry->DomainId = subject_id;
                drtentry->EntryId = ++(domain->drt.next_id);
                drtentry->IsPeer = subject_is_peer;
                domain->drt.list = g_slist_append(domain->drt.list, drtentry);
                gen_domain_event(target_id, subject_id, SAHPI_TRUE);
        } else {
                GSList *node = NULL, *savenode = NULL;
                int child_count = 0, peer_count = 0, is_peer = 0;
                
                for (node = domain->drt.list;
                     node || savenode;
                     node = (node) ? node->next : savenode) {
                        drtentry = (SaHpiDrtEntryT *)node->data;
                        savenode = NULL;

                        if (drtentry->IsPeer) peer_count++;
                        else child_count++;
                        
                        if (drtentry->DomainId == subject_id && !found) {
                                is_peer = drtentry->IsPeer;
                                savenode = node->next;
                                domain->drt.list =
                                        g_slist_delete_link(domain->drt.list,
                                                            node);
                                g_free(node->data);
                                gen_domain_event(target_id, subject_id, SAHPI_FALSE);
                                found = 1;
                                node = NULL;
                        }
                }

        }

        if (addition || found) {
                gettimeofday(&tv1, NULL);
                domain->drt.update_timestamp = (SaHpiTimeT) tv1.tv_sec *
                                1000000000 + tv1.tv_usec * 1000;
                domain->drt.update_count++;
        }
        oh_release_domain(domain);
}
#define add_drtentry(target_id, subject_id, subject_is_peer) \
        update_drt(target_id, subject_id, subject_is_peer, SAHPI_TRUE);
#define del_drtentry(target_id, subject_id) \
        update_drt(target_id, subject_id, SAHPI_FALSE, SAHPI_FALSE);

static int connect2parent(struct oh_domain *domain, SaHpiDomainIdT parent_id)
{
        struct oh_domain *parent = NULL;

        if (!domain) return -1;
        if (parent_id == SAHPI_UNSPECIFIED_DOMAIN_ID) return -2;

        parent = oh_get_domain(parent_id);
        if (!parent) {
                err("Couldn't get domain %d", parent_id);
                return -3;
        }

        /* Add child drt to peers of parent domain */
#if 0
        if (parent->state & OH_DOMAIN_PEER) {
                GSList *node = NULL;
                for (node = parent->drt.list; node; node = node->next) {
                        SaHpiDrtEntryT *drte = (SaHpiDrtEntryT *)node->data;
                        if (drte->IsPeer) {
                                add_drtentry(drte->DomainId, domain->id,
                                             SAHPI_FALSE);
                        }
                }
        }
#endif 
        oh_release_domain(parent);

        /* Add child drt to parent domain */
        add_drtentry(parent_id, domain->id, SAHPI_FALSE);
        // domain->state |= OH_DOMAIN_CHILD; /* set child state */
        domain->drt.parent_id = parent_id;

        return 0;
}

static int connect2peer(struct oh_domain *domain, SaHpiDomainIdT peer_id)
{
        struct oh_domain *peer = NULL;
        GSList *node = NULL;

        if (!domain) return -1;
        if (peer_id == SAHPI_UNSPECIFIED_DOMAIN_ID) return -2;

        peer = oh_get_domain(peer_id);
        if (!peer) {
                err("Couldn't get domain %d", peer_id);
                return -3;
        }

        /* Copy entitypath pattern. Peers contain the same resources */
        // domain->entity_pattern = peer->entity_pattern;
        /* Copy drt list from target peer.
         * Also, add self drt to peers of target peer. */
        for (node = peer->drt.list; node; node = node->next) {
                SaHpiDrtEntryT *drtentry = (SaHpiDrtEntryT *)node->data;
                add_drtentry(domain->id,
                             drtentry->DomainId,
                             drtentry->IsPeer);
                if (drtentry->IsPeer) {
                        add_drtentry(drtentry->DomainId,
                                     domain->id,
                                     SAHPI_TRUE);
                }
        }
        oh_release_domain(peer);
        /* Add each others drts to domain and domain's peer */
        add_drtentry(domain->id, peer_id, SAHPI_TRUE);
        add_drtentry(peer_id, domain->id, SAHPI_TRUE);
        // domain->state |= OH_DOMAIN_PEER;

        return 0;
}

#if 0
static int disconnect_parent(struct oh_domain *child)
{
        GSList *node = NULL;
        struct oh_domain *parent = NULL;

        if (!child) return -1;
        // if (!(child->state & OH_DOMAIN_CHILD)) return -2;
        
        parent = oh_get_domain(child->drt.parent_id);
        if (!parent) return -3;
        /* Remove child drt from peers of the parent */
        for (node = parent->drt.list; node; node = node->next) {
                SaHpiDrtEntryT *drte = (SaHpiDrtEntryT *)node->data;
                if (drte->IsPeer) {
                        del_drtentry(drte->DomainId, child->id);
                }
        }
        oh_release_domain(parent);
        /* Finally, remove child drt from the parent */
        del_drtentry(child->drt.parent_id, child->id);
        // child->state = child->state & 0x06; /* Unset child state */

        return 0;
}

static int disconnect_peers(struct oh_domain *domain)
{
        GSList *node = NULL;

        if (!domain) return -1;
        // if (!(domain->state & OH_DOMAIN_PEER)) return -2;

        /* Remove drt from peers */
        for (node = domain->drt.list; node; node = node->next) {
                SaHpiDrtEntryT *drte = (SaHpiDrtEntryT *)node->data;
                if (drte->IsPeer) {
                        del_drtentry(drte->DomainId, domain->id);
                }
        }

        // domain->state = domain->state & 0x03; /* Unset peer state */

        return 0;
}
#endif 

/**
 * oh_create_domain
 * @id: Required. 0 or SAHPI_UNSPECIFIED_DOMAIN_ID means default.
 * @entity_pattern: Required.
 * @nametag: Optional.
 * @tier_of: Optional. SAHPI_UNSPECIFIED_DOMAIN_ID means none.
 * @peer_of: Optional. SAHPI_UNSPECIFIED_DOMAIN_ID means none.
 * @capabilities:
 * @ai_timeout:
 *
 * 
 *
 * Returns: SA_OK if domain was created successfully.
 **/
SaErrorT oh_create_domain(SaHpiDomainIdT id,
                          char *tag,
                          SaHpiDomainIdT child_of,
                          SaHpiDomainIdT peer_of,
                          SaHpiDomainCapabilitiesT capabilities,
                          SaHpiTimeoutT ai_timeout)
{
        struct oh_domain *domain = g_new0(struct oh_domain,1);
        struct oh_global_param param = { .type = OPENHPI_DEL_SIZE_LIMIT };
        char filepath[SAHPI_MAX_TEXT_BUFFER_LENGTH*2];

        /* Fix id to int capable value */
        if (id == SAHPI_UNSPECIFIED_DOMAIN_ID)
                id = OH_DEFAULT_DOMAIN_ID;
		
		
	/* Input validation */
        if (peer_of == id || child_of == id ||
                 (child_of == peer_of && child_of != SAHPI_UNSPECIFIED_DOMAIN_ID))
                return SA_ERR_HPI_INVALID_PARAMS;
	
	              
        /* Check to see if domain id is already taken */
        domains_lock();
        if (g_hash_table_lookup(oh_domains.table, &id)) {
                domains_unlock();
                g_free(domain);
                err("Domain %u already exists; not creating twice.", id);
                return SA_ERR_HPI_INVALID_DOMAIN;
        }

        domain->id = id; /* Set domain id */

        if (tag) { /* Set domain tag */
                oh_init_textbuffer(&domain->tag);
                oh_append_textbuffer(&domain->tag, tag);
        }
        domain->capabilities = capabilities;
        domain->ai_timeout = ai_timeout;

        /* Initialize Resource Precense Table */
        oh_init_rpt(&(domain->rpt));

        /* Initialize domain reference table timestamp to a valid value */
        domain->drt.update_timestamp = SAHPI_TIME_UNSPECIFIED;

        oh_get_global_param(&param); /* Get domain event log size limit */
        /* Initialize domain event log */
        domain->del = oh_el_create(param.u.del_size_limit);

        if (!domain->del) {
                domains_unlock();
                g_free(domain->del);
                g_free(domain);
                return SA_ERR_HPI_ERROR;
        }

        g_static_rec_mutex_init(&domain->lock);
        g_static_rec_mutex_init(&domain->refcount_lock);

        /* Get option for saving domain event log or not */
        param.type = OPENHPI_DEL_SAVE;
        oh_get_global_param(&param);

        if (param.u.del_save) {
                param.type = OPENHPI_VARPATH;
                oh_get_global_param(&param);
                snprintf(filepath,
                         SAHPI_MAX_TEXT_BUFFER_LENGTH*2,
                         "%s/del.%u", param.u.varpath, domain->id);
                oh_el_map_from_file(domain->del, filepath);
        }
	param.type = OPENHPI_DAT_SAVE;
	oh_get_global_param(&param);
	if (param.u.dat_save) {
		param.type = OPENHPI_VARPATH;
		oh_get_global_param(&param);
		memset(filepath, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH*2);
		snprintf(filepath,
			 SAHPI_MAX_TEXT_BUFFER_LENGTH*2,
			 "%s/dat.%u", param.u.varpath, domain->id);
		oh_alarms_from_file(domain, filepath);
	}

        /* Need to put new domain in table before relating to other domains. */
        oh_domains.list = g_list_append(oh_domains.list, domain);
        g_hash_table_insert(oh_domains.table,
                            &domain->id,
                            g_list_last(oh_domains.list));

        /* Establish child-parent relationship */
        if (child_of != SAHPI_UNSPECIFIED_DOMAIN_ID &&
            connect2parent(domain, child_of)) {
                err("Error connecting domain %u to parent %u",
                    domain->id, child_of);
        }

        /* Establish peer relationships */
        if (peer_of != SAHPI_UNSPECIFIED_DOMAIN_ID &&
            connect2peer(domain, peer_of)) {
                err("Error connection domain %u to peer %u",
                    domain->id, peer_of);
        }
        
        domains_unlock();

        return SA_OK;
}

#if 0
SaErrorT oh_create_domain_from_table(GHashTable *table)
{
        SaHpiDomainIdT *id = NULL, *child_of = NULL, *peer_of = NULL;
        char *entity_pattern = NULL, *tag = NULL;
        oh_entitypath_pattern epp;
        SaHpiTimeT ai_timeout = SAHPI_TIMEOUT_IMMEDIATE, *ait = NULL;
        unsigned int *ai_readonly = NULL;
        SaHpiDomainCapabilitiesT capabilities =
                SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY;

        if (!table) return SA_ERR_HPI_INVALID_PARAMS;

        id = (SaHpiDomainIdT *)g_hash_table_lookup(table, "id");
        child_of = (SaHpiDomainIdT *)g_hash_table_lookup(table, "child_of");
        peer_of = (SaHpiDomainIdT *)g_hash_table_lookup(table, "peer_of");
        entity_pattern = (char *)g_hash_table_lookup(table, "entity_pattern");
        tag = (char *)g_hash_table_lookup(table, "tag");
        ait = (SaHpiTimeT *)g_hash_table_lookup(table, "ai_timeout");
        ai_readonly = (unsigned int *)g_hash_table_lookup(table, "ai_readonly");

        if (!id) {
                err("Error creating a domain from configuration."
                    " No domain id was given.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (id == 0) { /* ID == 0 is the default domain */
                /* Default domain cannot be a peer or child of anyone */
                child_of = NULL;
                peer_of = NULL;
        }

        if (peer_of && entity_pattern) {
                warn("Warning creating domain %u. Entity pattern will be"
                     " disregarded since a peer was specified.",
                     *id);
        } else if (!peer_of &&
                   oh_compile_entitypath_pattern(entity_pattern, &epp)) {
                err("Error creating domain %u. "
                    "Invalid entity pattern given.", *id);
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (ait) ai_timeout = *ait * 1000000000;

        if (ai_readonly && *ai_readonly == 0)
                capabilities = 0;

        return oh_create_domain(*id, (peer_of) ? NULL : &epp, tag,
                                (child_of) ? *child_of : SAHPI_UNSPECIFIED_DOMAIN_ID,
                                (peer_of) ? *peer_of : SAHPI_UNSPECIFIED_DOMAIN_ID,
                                capabilities, ai_timeout);
}
#endif

/**
 * oh_destroy_domain
 * @did:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_destroy_domain(SaHpiDomainIdT did)
{
	struct oh_domain *domain = NULL;
        GList *node = NULL;
        
        if (did == OH_DEFAULT_DOMAIN_ID ||
            did == SAHPI_UNSPECIFIED_DOMAIN_ID)
                return SA_ERR_HPI_INVALID_PARAMS;

        node = __get_domain(did);
        if (!node) {
                return SA_ERR_HPI_NOT_PRESENT;
        }

        domain = (struct oh_domain *)node->data;

#if 0
        if (domain->state & OH_DOMAIN_CHILD)
                disconnect_parent(domain);

        if (domain->state & OH_DOMAIN_PEER)
                disconnect_peers(domain);
#endif     

        domains_lock();
        g_hash_table_remove(oh_domains.table, &domain->id);
        oh_domains.list = g_list_delete_link(oh_domains.list, node);
        domains_unlock();

        __dec_domain_refcount(domain);
        if (domain->refcount < 1)
                __delete_domain(domain);
        else
                oh_release_domain(domain);

        return SA_OK;
}

/**
 * oh_get_domain
 * @did:
 *
 *
 *
 * Returns:
 **/
struct oh_domain *oh_get_domain(SaHpiDomainIdT did)
{
        GList *node = NULL;
        struct oh_domain *domain = NULL;
        
        node = __get_domain(did);
        if (!node) {
                return NULL;
        }
        
        domain = (struct oh_domain *)node->data;

        return domain;
}

/**
 * oh_release_domain
 * @domain:
 *
 *
 *
 * Returns:
 **/
SaErrorT oh_release_domain(struct oh_domain *domain)
{
        if (!domain) return SA_ERR_HPI_INVALID_PARAMS;

        __dec_domain_refcount(domain); /* Punch out */
        /*
         * If domain was scheduled for destruction before, and
         * no other threads are referring to it, then delete domain.
         */
        if (domain->refcount < 0)
                __delete_domain(domain);
        else
                g_static_rec_mutex_unlock(&domain->lock);

        return SA_OK;
}

#if 0
/**
 * oh_query_domains
 *
 * Fetches information on all present domains
 *
 * Returns: a GArray of oh_domain_result types.
 **/
GArray *oh_query_domains()
{
        GArray *domain_results =
                g_array_new(FALSE, TRUE, sizeof(oh_domain_result));

        domains_lock();
        g_hash_table_foreach(oh_domains.table, __query_domains, domain_results);
        domains_unlock();

        return domain_results;
}
#endif

/**
 * oh_drt_entry_get
 * @did: a domain id
 * @entryid: id of drt entry
 * @nextentryid: id next to @entryid in the drt will be put here.
 * @drtentry: drt entry corresponding to @entryid will be placed here.
 *
 * Fetches a drt entry from the domain identified by @did
 *
 * Returns: SA_OK on success, otherwise an error.
 **/
SaErrorT oh_drt_entry_get(SaHpiDomainIdT     did,
                          SaHpiEntryIdT      entryid,
                          SaHpiEntryIdT      *nextentryid,
                          SaHpiDrtEntryT     *drtentry)
{
        struct oh_domain *domain = NULL;
        GSList *node = NULL;

        if (did < 0 || !nextentryid || !drtentry) {
                err("Error - Invalid parameters passed.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        domain = oh_get_domain(did);
        if (domain == NULL) {
                err("no domain for id %d", did);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        for (node = domain->drt.list; node; node = node->next) {
                SaHpiDrtEntryT *curdrt = (SaHpiDrtEntryT *)node->data;
                if (curdrt->EntryId == entryid || entryid == SAHPI_FIRST_ENTRY) {
                        if (node->next == NULL) { /* last entry */
                                *nextentryid = SAHPI_LAST_ENTRY;
                        } else {
                                SaHpiDrtEntryT *nextdrt =
                                        (SaHpiDrtEntryT *)node->next->data;
                                *nextentryid = nextdrt->EntryId;
                        }
                        memcpy(drtentry, curdrt, sizeof(SaHpiDrtEntryT));
                        oh_release_domain(domain);
                        return SA_OK;
                }
        }
        oh_release_domain(domain);

        return SA_ERR_HPI_NOT_PRESENT;
}
