/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      W. David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include "oh_client_session.h"
extern "C"
{
#include <pthread.h>
#include <oHpi.h>
#include <oh_error.h>
#include <config.h>
#include <oh_domain.h>
#include "oh_client_conf.h"
}

GHashTable *ohd_domains = NULL;
GHashTable *ohd_sessions = NULL;
GStaticRecMutex ohd_sessions_sem = G_STATIC_REC_MUTEX_INIT;
static SaHpiSessionIdT next_client_sid = 1;

static void __destroy_client_connx(gpointer data)
{
        struct oh_client_session *client_session = (struct oh_client_session *)data;

        g_hash_table_destroy(client_session->connxs);
        g_free(client_session);
}

int oh_client_init(void)
{
	char *tmp_env_str = NULL;
        // Initialize GLIB thread engine
	if (!g_thread_supported()) {
        	g_thread_init(NULL);
        }
        
        g_static_rec_mutex_lock(&ohd_sessions_sem);
        // Create session table.
	if (!ohd_sessions) {
		ohd_sessions = g_hash_table_new_full(
			g_int_hash, 
                        g_int_equal,
                        NULL, 
                        __destroy_client_connx
                );
	}

        if (!ohd_domains) { // Create domain table
                struct oh_domain_conf *domain_conf = NULL;
                //  SaHpiDomainIdT default_did = SAHPI_UNSPECIFIED_DOMAIN_ID;
                SaHpiDomainIdT default_did = OH_DEFAULT_DOMAIN_ID;



                ohd_domains = g_hash_table_new_full(
                        g_int_hash, g_int_equal,
                        NULL, g_free
                );                
                /* TODO: Have a default openhpiclient.conf file in /etc */
                if ((tmp_env_str = getenv("OPENHPICLIENT_CONF")) != NULL) {
			oh_load_client_config(tmp_env_str, ohd_domains);
                } else {
			oh_load_client_config(OH_CLIENT_DEFAULT_CONF, ohd_domains);	
		}
		
                
                
                /* Check to see if a default domain exists, if not, add it */
                domain_conf =
                  (struct oh_domain_conf *)g_hash_table_lookup(ohd_domains,
                                                               &default_did);
                if (!domain_conf) {
                        const char *host, *portstr;
                        int port;
                        
                        /* TODO: change these envs to have DEFAULT in the name*/
                        host = getenv("OPENHPI_DAEMON_HOST");
                        if (!host) host = "localhost";
                        
                        portstr = getenv("OPENHPI_DAEMON_PORT");
                        if (!portstr) port = OPENHPI_DEFAULT_DAEMON_PORT;
                        else port = atoi(portstr);
                        
                        domain_conf = g_new0(struct oh_domain_conf, 1);
                        domain_conf->did = default_did;
                        strncpy(domain_conf->host, host,
                                SAHPI_MAX_TEXT_BUFFER_LENGTH);
                        domain_conf->port = port;
                        g_hash_table_insert(ohd_domains,
                                            &domain_conf->did, domain_conf);
                }
        }

        g_static_rec_mutex_unlock(&ohd_sessions_sem);
        return 0;
}

SaErrorT oh_create_connx(SaHpiDomainIdT did, pcstrmsock *pinst)
{
        struct oh_domain_conf *domain_conf = NULL;
        pcstrmsock connx = NULL;
        
        if (!pinst) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        oh_client_init(); /* Initialize library - Will run only once */

        g_static_rec_mutex_lock(&ohd_sessions_sem);
	connx = new cstrmsock;
        if (!connx) {
                g_static_rec_mutex_unlock(&ohd_sessions_sem);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }

        domain_conf = (struct oh_domain_conf *)g_hash_table_lookup(ohd_domains, &did);
        if (!domain_conf) {
                delete connx;
                g_static_rec_mutex_unlock(&ohd_sessions_sem);
                err("Client configuration for domain %u was not found.", did);
                return SA_ERR_HPI_INVALID_DOMAIN;
        }
        g_static_rec_mutex_unlock(&ohd_sessions_sem);
        
	if (connx->Open(domain_conf->host, domain_conf->port)) {
		err("Could not open client socket"
		    "\nPossibly, the OpenHPI daemon has not been started.");
                delete connx;
		return SA_ERR_HPI_NO_RESPONSE;
	}
        
        *pinst = connx;
	dbg("Client instance created");
	return SA_OK;
}

void oh_delete_connx(pcstrmsock pinst)
{
	if (pinst) {
                pinst->Close();
                dbg("Connection closed and deleted");
                delete pinst;
        }
}

SaErrorT oh_close_connx(SaHpiSessionIdT SessionId)
{
        pthread_t thread_id = pthread_self();
        struct oh_client_session *client_session = NULL;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        g_static_rec_mutex_lock(&ohd_sessions_sem);
        client_session = (struct oh_client_session *)g_hash_table_lookup(ohd_sessions, &SessionId);
        if (!client_session) {
                g_static_rec_mutex_unlock(&ohd_sessions_sem);
                err("Did not find connx for sid %d", SessionId);
                return SA_ERR_HPI_INTERNAL_ERROR;
        }
        
        g_hash_table_remove(client_session->connxs, &thread_id);

        g_static_rec_mutex_unlock(&ohd_sessions_sem);

        return SA_OK;
}

SaErrorT oh_get_connx(SaHpiSessionIdT csid, SaHpiSessionIdT *dsid, pcstrmsock *pinst, SaHpiDomainIdT *did)
{
        pthread_t thread_id = pthread_self();
        struct oh_client_session *client_session = NULL;
        pcstrmsock connx = NULL;
        SaErrorT ret = SA_OK;

	if (!csid || !dsid || !pinst || !did)
		return SA_ERR_HPI_INVALID_PARAMS;
		
	oh_client_init(); /* Initialize library - Will run only once */

        // Look up connection table. If it exists, look up connection.
        // if there is not connection, create one on-the-fly.
        g_static_rec_mutex_lock(&ohd_sessions_sem);
        client_session =
          (struct oh_client_session *)g_hash_table_lookup(ohd_sessions, &csid);
        
        if (client_session) {
                connx = (pcstrmsock)g_hash_table_lookup(client_session->connxs,
                                                        &thread_id);

                if (!connx) {
                        ret = oh_create_connx(client_session->did, &connx);
                        if (connx) {
                        	g_hash_table_insert(client_session->connxs, 
                                		    g_memdup(&thread_id,
                                		     sizeof(pthread_t)),
                                		    connx);
				dbg("We are inserting a new connection"
				    " in conns table");
                        }
                }
                *dsid = client_session->dsid;
				*did  = client_session->did;
                *pinst = connx;
        }
        g_static_rec_mutex_unlock(&ohd_sessions_sem);        

	if (client_session) {
                if (connx)
                        return SA_OK;
                else 
                        return ret;
	}
	else
		return SA_ERR_HPI_INVALID_SESSION;
}

static void __delete_connx(gpointer data)
{
        pcstrmsock pinst = (pcstrmsock)data;

        oh_delete_connx(pinst);
}

SaHpiSessionIdT oh_open_session(SaHpiDomainIdT did,
                                SaHpiSessionIdT sid,
                                pcstrmsock pinst)
{
        GHashTable *connxs = NULL;
        pthread_t thread_id;
        struct oh_client_session *client_session;
        
        if (!sid || !pinst)
		return 0;
	
        client_session = g_new0(struct oh_client_session, 1);
        
        g_static_rec_mutex_lock(&ohd_sessions_sem);
        // Create connections table for new session.
        connxs = g_hash_table_new_full(g_int_hash, g_int_equal,
                                       g_free, __delete_connx);
        // Map connection to thread id
        thread_id = pthread_self();
        g_hash_table_insert(connxs, g_memdup(&thread_id, sizeof(pthread_t)),
                            pinst);
        // Map connecitons table to session id
        client_session->did = did;
        client_session->dsid = sid;
        client_session->csid = next_client_sid++;
        client_session->connxs = connxs;
        g_hash_table_insert(ohd_sessions, &client_session->csid, client_session);
        g_static_rec_mutex_unlock(&ohd_sessions_sem);

        return client_session->csid;
}

SaErrorT oh_close_session(SaHpiSessionIdT SessionId)
{        
        if (SessionId == 0)
		return SA_ERR_HPI_INVALID_PARAMS;

        g_static_rec_mutex_lock(&ohd_sessions_sem);
        // Since we used g_hash_table_new_full to create the tables,
        // this will remove the connection hash table and all of its entries also.
        g_hash_table_remove(ohd_sessions, &SessionId);
        
        g_static_rec_mutex_unlock(&ohd_sessions_sem);
        return SA_OK;
}
