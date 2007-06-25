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
 *      W. David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <glib.h>
#include <config.h>
extern "C"
{
#include <SaHpi.h>
#include <oHpi.h>
}


#include "marshal_hpi.h"
#include "openhpiclient.h"

#define dClientDebugErr
//#define dClientDebug

#ifndef dClientDebug
#define cdebug_out(cmd, str)
#else
#define cdebug_out(cmd, str)	fprintf(stdout, "%s: %s\n", cmd, str);
#endif

#ifndef dClientDebugErr
#define cdebug_err(cmd, str)
#else
#define cdebug_err(cmd, str)	fprintf(stderr, "%s: %s\n", cmd, str);
#endif

#define SendRecv(sid, cmd) \
	if (pinst->WriteMsg(request)) { \
		cdebug_err(cmd, "WriteMsg failed\n"); \
		if(request) \
			free(request); \
                if (sid) \
                        RemoveOneConnx(sid); \
                else \
                        DeleteConnx(pinst); \
		return SA_ERR_HPI_NO_RESPONSE; \
	} \
	if (pinst->ReadMsg(reply)) { \
		cdebug_err(cmd, "Read failed\n"); \
		if(request) \
			free(request); \
                if (sid) \
                        RemoveOneConnx(sid); \
                else \
                        DeleteConnx(pinst); \
		return SA_ERR_HPI_NO_RESPONSE; \
	}

#define SendRecvNoReturn(cmd) \
	if (pinst->WriteMsg(request)) { \
		cdebug_err(cmd, "WriteMsg failed\n"); \
		if(request) \
			free(request); \
                DeleteConnx(pinst); \
		return; \
	} \
	if (pinst->ReadMsg(reply)) { \
		cdebug_err(cmd, "Read failed\n"); \
		if(request) \
			free(request); \
                DeleteConnx(pinst); \
		return; \
	}


/*----------------------------------------------------------------------------*/
/* Global Vraiables                                                           */
/*----------------------------------------------------------------------------*/

static GHashTable *sessions = NULL;
static GStaticRecMutex sessions_sem = G_STATIC_REC_MUTEX_INIT;


/*----------------------------------------------------------------------------*/
/* Utility routines                                                           */
/*----------------------------------------------------------------------------*/

static pcstrmsock CreateConnx(void);
static void DeleteConnx(pcstrmsock);
static bool InsertConnx(SaHpiSessionIdT, pcstrmsock);
static bool RemoveConnx(SaHpiSessionIdT);
static bool RemoveOneConnx(SaHpiSessionIdT);
static pcstrmsock GetConnx(SaHpiSessionIdT);
static SaErrorT oHpiHandlerCreateInit(void);
static void oHpiHandlerCreateAddTEntry(gpointer key, gpointer value, gpointer data);

static void __destroy_table(gpointer data)
{
        GHashTable *table = (GHashTable *)data;

        g_hash_table_destroy(table);
}

static int init(void)
{
        // Initialize GLIB thread engine
	if (!g_thread_supported()) {
        	g_thread_init(NULL);
        }
        
        // Create session table.
	if (!sessions) {
		sessions = g_hash_table_new_full(g_int_hash, 
                                         	 g_int_equal,
                                        	 g_free, 
                                         	 __destroy_table);
	}

	return 0;
}

static SaErrorT clean_reading(SaHpiSensorReadingT *read_in,
				SaHpiSensorReadingT *read_out)
{
	/* This is a workaround against unknown bugs in the marshal code */
	if (!read_in || !read_out) return SA_ERR_HPI_INVALID_PARAMS;	
	
	memset(read_out, 0, sizeof(SaHpiSensorReadingT));
	
	read_out->IsSupported = read_in->IsSupported;

        if (read_in->IsSupported == SAHPI_TRUE) {
            if (!oh_lookup_sensorreadingtype(read_in->Type)) {
                //printf("Invalid reading type: %d\n", read_in->Type);
                return SA_ERR_HPI_INVALID_DATA;
            }
            read_out->Type = read_in->Type;
        }
        else {
            /* Do we need to set dummy & reading type just to keep marshalling happy? */
            read_out->Type = SAHPI_SENSOR_READING_TYPE_INT64;
            read_out->Value.SensorInt64 = 0;
            return SA_OK;
        }               

	if (read_in->Type == SAHPI_SENSOR_READING_TYPE_INT64) {
		read_out->Value.SensorInt64 = read_in->Value.SensorInt64;
	} else if (read_in->Type == SAHPI_SENSOR_READING_TYPE_UINT64) {
		read_out->Value.SensorUint64 = read_in->Value.SensorUint64;
	} else if (read_in->Type == SAHPI_SENSOR_READING_TYPE_FLOAT64) {
		read_out->Value.SensorFloat64 = read_in->Value.SensorFloat64;
	} else if (read_in->Type == SAHPI_SENSOR_READING_TYPE_BUFFER) {
		memcpy(read_out->Value.SensorBuffer,
		       read_in->Value.SensorBuffer,
		       SAHPI_SENSOR_BUFFER_LENGTH);
	}

	return SA_OK;
}

static SaErrorT clean_thresholds(SaHpiSensorThresholdsT *thrds_in,
				   SaHpiSensorThresholdsT *thrds_out)
{
	/* This is a workaround against unknown bugs in the marshal code */
	SaErrorT err = SA_OK;
	if (!thrds_in || !thrds_out) return SA_ERR_HPI_INVALID_PARAMS;

	err = clean_reading(&thrds_in->LowCritical, &thrds_out->LowCritical);
	if (err) return err;
	err = clean_reading(&thrds_in->LowMajor, &thrds_out->LowMajor);
	if (err) return err;
	err = clean_reading(&thrds_in->LowMinor, &thrds_out->LowMinor);
	if (err) return err;
	err = clean_reading(&thrds_in->UpCritical, &thrds_out->UpCritical);
	if (err) return err;
	err = clean_reading(&thrds_in->UpMajor, &thrds_out->UpMajor);
	if (err) return err;
	err = clean_reading(&thrds_in->UpMinor, &thrds_out->UpMinor);
	if (err) return err;
	err = clean_reading(&thrds_in->PosThdHysteresis,
			    &thrds_out->PosThdHysteresis);
	if (err) return err;
	err = clean_reading(&thrds_in->NegThdHysteresis,
			    &thrds_out->NegThdHysteresis);

	return err;
}

/*----------------------------------------------------------------------------*/
/* CreateConnx                                                                */
/*----------------------------------------------------------------------------*/

static pcstrmsock CreateConnx(void)
{
	const char      *host, *portstr;
	int	       	port;
        pcstrmsock      pinst = NULL;

        host = getenv("OPENHPI_DAEMON_HOST");
        if (host == NULL) {
                host = "localhost";
        }
        portstr = getenv("OPENHPI_DAEMON_PORT");
        if (portstr == NULL) {
                port =  4743;
        }
        else {
                port =  atoi(portstr);
        }
        
        init(); /* Initialize library - Will run only once */

        g_static_rec_mutex_lock(&sessions_sem);
	pinst = new cstrmsock;
        if (pinst == NULL) {
                g_static_rec_mutex_unlock(&sessions_sem);
                return pinst;
        }
        g_static_rec_mutex_unlock(&sessions_sem);
        
	if (pinst->Open(host, port)) {
		cdebug_err("CreateConnx", "Could not open client socket"
			   "\nPossibly, the OpenHPI daemon has not been started.");
                delete pinst;
		return NULL;
	}
	cdebug_out("CreateConnx", "CreateConnx:Client instance created");
	return pinst;
}


/*----------------------------------------------------------------------------*/
/* DeleteConnx                                                                */
/*----------------------------------------------------------------------------*/

static void DeleteConnx(pcstrmsock pinst)
{
	if (pinst == NULL)
		return;
	pinst->Close();
	cdebug_out("DeleteConnx", "Connection closed and deleted");
	delete pinst;
}


/*----------------------------------------------------------------------------*/
/* InsertConnx - with helper functions: __destroy_table, __delete_connx       */
/*----------------------------------------------------------------------------*/
static void __delete_connx(gpointer data)
{
        pcstrmsock pinst = (pcstrmsock)data;

        DeleteConnx(pinst);
}

static bool InsertConnx(SaHpiSessionIdT SessionId, pcstrmsock pinst)
{
        GHashTable *conns = NULL;
        pthread_t thread_id;
        
        if (SessionId == 0)
		return TRUE;
	if (pinst == NULL)
		return TRUE;

        g_static_rec_mutex_lock(&sessions_sem);
        // Create connections table for new session.
        conns = g_hash_table_new_full(g_int_hash, 
                                      g_int_equal,
                                      g_free, 
                                      __delete_connx);
        // Map connection to thread id
        thread_id = pthread_self();
        g_hash_table_insert(conns, g_memdup(&thread_id,
                                            sizeof(pthread_t)),
                                   pinst);
        // Map connecitons table to session id
        g_hash_table_insert(sessions, g_memdup(&SessionId,
                                               sizeof(SaHpiSessionIdT)),
                                      conns);
        g_static_rec_mutex_unlock(&sessions_sem);

        return FALSE;
}


/*----------------------------------------------------------------------------*/
/* RemoveConnx                                                                */
/*----------------------------------------------------------------------------*/

static bool RemoveConnx(SaHpiSessionIdT SessionId)
{
	if (SessionId == 0)
		return TRUE;

        g_static_rec_mutex_lock(&sessions_sem);
        // Since we used g_hash_table_new_full to create the tables,
        // this will remove the connection hash table and all of its entries also.
        g_hash_table_remove(sessions, &SessionId);
        g_static_rec_mutex_unlock(&sessions_sem);

        return FALSE;
}

/*----------------------------------------------------------------------------*/
/* RemoveOneConnx                                                             */
/*----------------------------------------------------------------------------*/

static bool RemoveOneConnx(SaHpiSessionIdT SessionId)
{
        GHashTable *conns = NULL;
        pthread_t thread_id;

        if (SessionId == 0)
                return TRUE;

        g_static_rec_mutex_lock(&sessions_sem);
        thread_id = pthread_self();
        conns = (GHashTable *)g_hash_table_lookup(sessions, &SessionId);
        if (conns)
                g_hash_table_remove(conns, &thread_id);

        g_static_rec_mutex_unlock(&sessions_sem);

        return FALSE;
}

/*----------------------------------------------------------------------------*/
/* GetConnx                                                                   */
/*----------------------------------------------------------------------------*/

static pcstrmsock GetConnx(SaHpiSessionIdT SessionId)
{
        pthread_t thread_id;
        GHashTable *conns = NULL;
        pcstrmsock pinst = NULL;

	if (SessionId == 0)
		return FALSE;
		
	init(); /* Initialize library - Will run only once */

        // Look up connection table. If it exists, look up connection.
        // if there is not connection, create one on-the-fly.
        g_static_rec_mutex_lock(&sessions_sem);
        thread_id = pthread_self();
        conns = (GHashTable *)g_hash_table_lookup(sessions, &SessionId);
        if (conns) {
                pinst = (pcstrmsock)g_hash_table_lookup(conns, &thread_id);

                if (!pinst) {
                        pinst = CreateConnx();
                        if (pinst) {
                        	g_hash_table_insert(conns, 
                                		    g_memdup(&thread_id,
                                		    sizeof(pthread_t)),
                                		    pinst);
				cdebug_out("GetConnx",
					   "we are inserting a new connection"
					   " in conns table");
                        }
                }


        }
        g_static_rec_mutex_unlock(&sessions_sem);

        if (pinst) {
            return pinst;
        } else {
            return FALSE;
        }

}


/*----------------------------------------------------------------------------*/
/* oHpiVersionGet - get OpenHPI version                                       */
/*----------------------------------------------------------------------------*/

SaHpiUint64T oHpiVersionGet()
{
        SaHpiUint64T v = 0;
        char * version = NULL;
        char * start = version;
        char * end = version;
        version = strdup(VERSION);
        if(!version) {
                /* even if we fail, we know we are the client lib */
                return 1;
        }
        start = version;
        end = version;
        v += (strtoull(start, &end, 10) << 48);
        end++;
        start = end;
        v += (strtoull(start, &end, 10) << 32);
        end++;
        start = end;
        v += (strtoull(start, &end, 10) << 16);
        v += 1;
        free(version);
        return v;
}


/*----------------------------------------------------------------------------*/
/* saHpiVersionGet                                                            */
/*----------------------------------------------------------------------------*/

SaHpiVersionT SAHPI_API dOpenHpiClientFunction(VersionGet)
	dOpenHpiClientParam(void)
{
	void *request = NULL;
	char reply[dMaxMessageLength];
	SaHpiVersionT err;
	char cmd[] = "saHpiVersionget";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL) {
                cdebug_err(cmd, "Could not create client connection");
                return SA_ERR_HPI_NO_RESPONSE;
        }

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiVersionGet);

	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiVersionGet, 0);

        SendRecv(0, cmd);

	int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader), &err);

        DeleteConnx(pinst);
	if (mr < 0)
		err = 0;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSessionOpen                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SessionOpen)
	dOpenHpiClientParam(SAHPI_IN SaHpiDomainIdT   DomainId,
			    SAHPI_OUT SaHpiSessionIdT *SessionId,
			    SAHPI_IN  void            *SecurityParams)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err;
	char cmd[] = "saHpiSessionOpen";
        pcstrmsock pinst = CreateConnx();

        if (SessionId == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
	if (SecurityParams != NULL)
		return SA_ERR_HPI_INVALID_PARAMS;
        if (pinst == NULL) {
                cdebug_err(cmd, "Could not create client connection");
                return SA_ERR_HPI_NO_RESPONSE;
        }

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSessionOpen);

	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSessionOpen,
				 hm->m_request_len);
	request = malloc(hm->m_request_len);

	pinst->header.m_len = HpiMarshalRequest1(hm, request, &DomainId);

        SendRecv(0, cmd);

	int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader), &err, SessionId);

	if (request)
		free(request);
	if (err != SA_OK) {
		DeleteConnx(pinst);
                return err;
        }
	if (mr < 0) {
                DeleteConnx(pinst);
		return SA_ERR_HPI_INVALID_PARAMS;
        }
        InsertConnx(*SessionId, pinst);

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSessionClose                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SessionClose)
	dOpenHpiClientParam(SAHPI_IN SaHpiSessionIdT SessionId)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err;
	char cmd[] = "saHpiSessionClose";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSessionClose);
	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSessionClose,
				 hm->m_request_len);
	request = malloc(hm->m_request_len);

	pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

	int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader), &err);

	if (request)
		free(request);
                
        RemoveConnx(SessionId);
        
	if (mr < 0)
		return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiDiscover                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(Discover)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err;
	char cmd[] = "saHpiDiscover";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;

        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDiscover);
	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDiscover,
				 hm->m_request_len);
	request = malloc(hm->m_request_len);

	pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

	int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader), &err);

	if (request)
		free(request);

        if (pinst->header.m_type == eMhError)
		return SA_ERR_HPI_INVALID_PARAMS;

        if (mr < 0)
		return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiDomainInfoGet                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(DomainInfoGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_OUT SaHpiDomainInfoT *DomainInfo)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiDomainInfoGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (DomainInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDomainInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDomainInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, DomainInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiDrtEntryGet                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(DrtEntryGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiEntryIdT   EntryId,
                             SAHPI_OUT SaHpiEntryIdT  *NextEntryId,
                             SAHPI_OUT SaHpiDrtEntryT *DrtEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiDrtEntryGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if((DrtEntry == NULL) ||
           (NextEntryId == NULL) ||
           (EntryId == SAHPI_LAST_ENTRY)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDrtEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDrtEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &EntryId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextEntryId, DrtEntry);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiDomainTagSet                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(DomainTagSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiTextBufferT *DomainTag)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiDomainTagSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!DomainTag)
                return SA_ERR_HPI_INVALID_PARAMS;
	if (!oh_lookup_texttype(DomainTag->DataType))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDomainTagSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDomainTagSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, DomainTag);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiRptEntryGet                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(RptEntryGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiEntryIdT    EntryId,
                             SAHPI_OUT SaHpiEntryIdT   *NextEntryId,
                             SAHPI_OUT SaHpiRptEntryT  *RptEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiRptEntryGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if ((NextEntryId == NULL) || (RptEntry == NULL)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        if (EntryId == SAHPI_LAST_ENTRY) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRptEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRptEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &EntryId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextEntryId, RptEntry);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiRptEntryGetByResourceId                                               */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(RptEntryGetByResourceId)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_OUT SaHpiRptEntryT  *RptEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiRptEntryGetByResourceId";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID ||
            RptEntry == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRptEntryGetByResourceId);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRptEntryGetByResourceId, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, RptEntry);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceSeveritySet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceSeveritySet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSeverityT   Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceSeveritySet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceSeveritySet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceSeveritySet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Severity);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceTagSet                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceTagSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiTextBufferT *ResourceTag)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceTagSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (ResourceTag == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceTagSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceTagSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, ResourceTag);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceIdGet                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceIdGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_OUT SaHpiResourceIdT *ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceIdGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (ResourceId == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceIdGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceIdGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, ResourceId);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogInfoGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogInfoGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT     SessionId,
                             SAHPI_IN SaHpiResourceIdT    ResourceId,
                             SAHPI_OUT SaHpiEventLogInfoT *Info)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogInfoGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Info == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Info);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogEntryGet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogEntryGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT        SessionId,
                             SAHPI_IN SaHpiResourceIdT       ResourceId,
                             SAHPI_IN SaHpiEntryIdT          EntryId,
                             SAHPI_OUT SaHpiEventLogEntryIdT *PrevEntryId,
                             SAHPI_OUT SaHpiEventLogEntryIdT *NextEntryId,
                             SAHPI_OUT SaHpiEventLogEntryT   *EventLogEntry,
                             SAHPI_INOUT SaHpiRdrT           *Rdr,
                             SAHPI_INOUT SaHpiRptEntryT      *RptEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogEntryGet";
        SaHpiRdrT tmp_rdr;
        SaHpiRptEntryT tmp_rpt;
        pcstrmsock pinst;	

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!PrevEntryId || !EventLogEntry || !NextEntryId ||
            EntryId == SAHPI_NO_MORE_ENTRIES) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &EntryId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply5(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, PrevEntryId, NextEntryId, EventLogEntry, &tmp_rdr, &tmp_rpt);

        if (Rdr != NULL) {
                memcpy(Rdr, &tmp_rdr, sizeof(SaHpiRdrT));
        }
        if (RptEntry != NULL) {
                memcpy(RptEntry, &tmp_rpt, sizeof(SaHpiRptEntryT));
        }

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogEntryAdd                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogEntryAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiEventT      *EvtEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogEntryAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (EvtEntry == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (EvtEntry->EventType != SAHPI_ET_USER ||
            EvtEntry->Source != SAHPI_UNSPECIFIED_RESOURCE_ID)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(EvtEntry->Severity))
                return SA_ERR_HPI_INVALID_PARAMS;
	if (!oh_valid_textbuffer(&EvtEntry->EventDataUnion.UserEvent.UserEventData))
		return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogEntryAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogEntryAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, EvtEntry);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogClear                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogClear)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogClear";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogClear);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogClear, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogTimeGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogTimeGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_OUT SaHpiTimeT      *Time)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogTimeGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Time == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogTimeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogTimeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Time);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogTimeSet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogTimeSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiTimeT       Time)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogTimeSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogTimeSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogTimeSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Time);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogStateGet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogStateGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_OUT SaHpiBoolT      *EnableState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogStateGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (EnableState == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, EnableState);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogStateSet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogStateSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiBoolT       EnableState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogStateSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &EnableState);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventLogOverflowReset                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventLogOverflowReset)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventLogOverflowReset";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogOverflowReset);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogOverflowReset, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSubscribe                                                             */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(Subscribe)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSubscribe";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSubscribe);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSubscribe, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiUnsSubscribe                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(Unsubscribe)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiUnsubscribe";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiUnsubscribe);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiUnsubscribe, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventGet                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT         SessionId,
                             SAHPI_IN SaHpiTimeoutT           Timeout,
                             SAHPI_OUT SaHpiEventT            *Event,
                             SAHPI_INOUT SaHpiRdrT            *Rdr,
                             SAHPI_INOUT SaHpiRptEntryT       *RptEntry,
                             SAHPI_INOUT SaHpiEvtQueueStatusT *EventQueueStatus)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventGet";
        SaHpiRdrT tmp_rdr;
        SaHpiRptEntryT tmp_rpt;
        SaHpiEvtQueueStatusT tmp_status;
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Timeout < SAHPI_TIMEOUT_BLOCK || !Event)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &Timeout);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply4(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Event, &tmp_rdr, &tmp_rpt, &tmp_status);

        if (Rdr != NULL) {
                memcpy(Rdr, &tmp_rdr, sizeof(SaHpiRdrT));
        }
        if (RptEntry != NULL) {
                memcpy(RptEntry, &tmp_rpt, sizeof(SaHpiRptEntryT));
        }
        if (EventQueueStatus != NULL) {
                memcpy(EventQueueStatus, &tmp_status, sizeof(SaHpiEvtQueueStatusT));
        }

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiEventAdd                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(EventAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiEventT     *Event)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiEventAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
	err = oh_valid_addevent(Event);
	if (err != SA_OK) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, Event);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAlarmGetNext                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AlarmGetNext)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiSeverityT  Severity,
                             SAHPI_IN SaHpiBoolT      Unack,
                             SAHPI_INOUT SaHpiAlarmT  *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAlarmGetNext";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Alarm)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;
        if (Alarm->AlarmId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_NOT_PRESENT;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &Severity, &Unack, Alarm);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAlarmGet                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AlarmGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiAlarmIdT   AlarmId,
                             SAHPI_OUT SaHpiAlarmT    *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAlarmGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Alarm)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &AlarmId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAlarmAcknowledge                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AlarmAcknowledge)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiAlarmIdT   AlarmId,
                             SAHPI_IN SaHpiSeverityT  Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAlarmAcknowledge";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (AlarmId == SAHPI_ENTRY_UNSPECIFIED &&
            !oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmAcknowledge);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmAcknowledge, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &AlarmId, &Severity);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAlarmAdd                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AlarmAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_INOUT SaHpiAlarmT  *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAlarmAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Alarm ||
            !oh_lookup_severity(Alarm->Severity) ||
            Alarm->AlarmCond.Type != SAHPI_STATUS_COND_TYPE_USER)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, Alarm);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAlarmDelete                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AlarmDelete)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiAlarmIdT   AlarmId,
                             SAHPI_IN SaHpiSeverityT  Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAlarmDelete";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (AlarmId == SAHPI_ENTRY_UNSPECIFIED &&
            !oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &AlarmId, &Severity);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiRdrGet                                                                */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(RdrGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiEntryIdT    EntryId,
                             SAHPI_OUT SaHpiEntryIdT   *NextEntryId,
                             SAHPI_OUT SaHpiRdrT       *Rdr)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiRdrGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (EntryId == SAHPI_LAST_ENTRY || !Rdr || !NextEntryId)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRdrGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRdrGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &EntryId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextEntryId, Rdr);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiRdrGetByInstrumentId                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(RdrGetByInstrumentId)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT    SessionId,
                             SAHPI_IN SaHpiResourceIdT   ResourceId,
                             SAHPI_IN SaHpiRdrTypeT      RdrType,
                             SAHPI_IN SaHpiInstrumentIdT InstrumentId,
                             SAHPI_OUT SaHpiRdrT         *Rdr)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiRdrGetByInstrumentId";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_rdrtype(RdrType) ||
            RdrType == SAHPI_NO_RECORD || !Rdr)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRdrGetByInstrumentId);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRdrGetByInstrumentId, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &RdrType, &InstrumentId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Rdr);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorReadingGet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorReadingGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT        SessionId,
                             SAHPI_IN SaHpiResourceIdT       ResourceId,
                             SAHPI_IN SaHpiSensorNumT        SensorNum,
                             SAHPI_INOUT SaHpiSensorReadingT *Reading,
                             SAHPI_INOUT SaHpiEventStateT    *EventState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorReadingGet";
        SaHpiSensorReadingT tmp_reading;
        SaHpiEventStateT tmp_state;
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorReadingGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorReadingGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &SensorNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, &tmp_reading, &tmp_state);

        if (Reading != NULL) {
                memcpy(Reading, &tmp_reading, sizeof(SaHpiSensorReadingT));
        }
        if (EventState != NULL) {
                memcpy(EventState, &tmp_state, sizeof(SaHpiEventStateT));
        }

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorThresholdsGet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorThresholdsGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT         SessionId,
                             SAHPI_IN SaHpiResourceIdT        ResourceId,
                             SAHPI_IN SaHpiSensorNumT         SensorNum,
                             SAHPI_OUT SaHpiSensorThresholdsT *Thresholds)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorThresholdsGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Thresholds)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorThresholdsGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorThresholdsGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &SensorNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Thresholds);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorThresholdsSet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorThresholdsSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT        SessionId,
                             SAHPI_IN SaHpiResourceIdT       ResourceId,
                             SAHPI_IN SaHpiSensorNumT        SensorNum,
                             SAHPI_IN SaHpiSensorThresholdsT *Thresholds)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorThresholdsSet";
        pcstrmsock pinst;
	SaHpiSensorThresholdsT tmpthrds;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Thresholds)
                return SA_ERR_HPI_INVALID_DATA;

	err = clean_thresholds(Thresholds, &tmpthrds);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorThresholdsSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorThresholdsSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &SensorNum, &tmpthrds);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorTypeGet                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorTypeGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT      SessionId,
                             SAHPI_IN SaHpiResourceIdT     ResourceId,
                             SAHPI_IN SaHpiSensorNumT      SensorNum,
                             SAHPI_OUT SaHpiSensorTypeT    *Type,
                             SAHPI_OUT SaHpiEventCategoryT *Category)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorTypeGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Type || !Category)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorTypeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorTypeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &SensorNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Type, Category);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEnableGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEnableGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSensorNumT  SensorNum,
                             SAHPI_OUT SaHpiBoolT      *Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEnableGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Enabled)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEnableGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEnableGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &SensorNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Enabled);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEnableSet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEnableSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSensorNumT  SensorNum,
                             SAHPI_IN SaHpiBoolT       Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEnableSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEnableSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEnableSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &SensorNum, &Enabled);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEventEnableGet                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEventEnableGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSensorNumT  SensorNum,
                             SAHPI_OUT SaHpiBoolT      *Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEventEnableGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Enabled)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventEnableGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventEnableGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &SensorNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Enabled);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEventEnableSet                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEventEnableSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSensorNumT  SensorNum,
                             SAHPI_IN SaHpiBoolT       Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEventEnableSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventEnableSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventEnableSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &SensorNum, &Enabled);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEventMasksGet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEventMasksGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiSensorNumT  SensorNum,
                             SAHPI_INOUT SaHpiEventStateT *Assert,
                             SAHPI_INOUT SaHpiEventStateT *Deassert)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEventMasksGet";
        pcstrmsock pinst;
        SaHpiEventStateT myassert = 0, mydeassert = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
		
	if (!Assert) Assert = &myassert;
	if (!Deassert) Deassert = &mydeassert;
	
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventMasksGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventMasksGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &SensorNum, Assert, Deassert);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Assert, Deassert);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSensorEventMasksSet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(SensorEventMasksSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT             SessionId,
                             SAHPI_IN SaHpiResourceIdT            ResourceId,
                             SAHPI_IN SaHpiSensorNumT             SensorNum,
                             SAHPI_IN SaHpiSensorEventMaskActionT Action,
                             SAHPI_IN SaHpiEventStateT            Assert,
                             SAHPI_IN SaHpiEventStateT            Deassert)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiSensorEventMasksSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventMasksSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventMasksSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request, &SessionId, &ResourceId, &SensorNum, &Action, &Assert, &Deassert);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiControlTypeGet                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ControlTypeGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiCtrlNumT    CtrlNum,
                             SAHPI_OUT SaHpiCtrlTypeT  *Type)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiControlTypeGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Type)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiControlTypeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiControlTypeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &CtrlNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Type);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiControlGet                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ControlGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT    SessionId,
                             SAHPI_IN SaHpiResourceIdT   ResourceId,
                             SAHPI_IN SaHpiCtrlNumT      CtrlNum,
                             SAHPI_OUT SaHpiCtrlModeT    *Mode,
                             SAHPI_INOUT SaHpiCtrlStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiControlGet";
        SaHpiCtrlModeT tmp_mode = SAHPI_CTRL_MODE_AUTO;
        SaHpiCtrlStateT tmp_state;
        pcstrmsock pinst;

	memset(&tmp_state, 0, sizeof(SaHpiCtrlStateT));

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        if (State != NULL) {
                memcpy(&tmp_state, State, sizeof(SaHpiCtrlStateT));
                if (oh_lookup_ctrltype(tmp_state.Type) == NULL) {
                        tmp_state.Type = SAHPI_CTRL_TYPE_OEM;
                }
        }
        else {
                tmp_state.Type = SAHPI_CTRL_TYPE_OEM;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiControlGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiControlGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &CtrlNum, &tmp_state);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, &tmp_mode, &tmp_state);

        if (Mode != NULL) {
                memcpy(Mode, &tmp_mode, sizeof(SaHpiCtrlModeT));
        }
        if (State != NULL) {
                memcpy(State, &tmp_state, sizeof(SaHpiCtrlStateT));
        }

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiControlSet                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ControlSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiCtrlNumT    CtrlNum,
                             SAHPI_IN SaHpiCtrlModeT   Mode,
                             SAHPI_IN SaHpiCtrlStateT  *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiControlSet";
        pcstrmsock pinst;
        SaHpiCtrlStateT mystate, *pmystate = NULL;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_ctrlmode(Mode) ||
            (Mode != SAHPI_CTRL_MODE_AUTO && !State) ||
            (State && State->Type == SAHPI_CTRL_TYPE_DIGITAL &&
             !oh_lookup_ctrlstatedigital(State->StateUnion.Digital)) ||
            (State && State->Type == SAHPI_CTRL_TYPE_STREAM &&
             State->StateUnion.Stream.StreamLength > SAHPI_CTRL_MAX_STREAM_LENGTH))
                return SA_ERR_HPI_INVALID_PARAMS;

        memset(&mystate, 0, sizeof(SaHpiCtrlStateT));
        if (Mode == SAHPI_CTRL_MODE_AUTO) {
		pmystate = &mystate;        
        } else if (State && !oh_lookup_ctrltype(State->Type)) {
        	return SA_ERR_HPI_INVALID_DATA;        	
        } else {
        	pmystate = State;
        }
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiControlSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiControlSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &CtrlNum, &Mode, pmystate);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrInfoGet                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrInfoGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiIdrIdT      Idrid,
                             SAHPI_OUT SaHpiIdrInfoT   *Info)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrInfoGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Info == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Idrid);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Info);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrAreaHeaderGet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrAreaHeaderGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT      SessionId,
                             SAHPI_IN SaHpiResourceIdT     ResourceId,
                             SAHPI_IN SaHpiIdrIdT          Idrid,
                             SAHPI_IN SaHpiIdrAreaTypeT    AreaType,
                             SAHPI_IN SaHpiEntryIdT        AreaId,
                             SAHPI_OUT SaHpiEntryIdT       *NextAreaId,
                             SAHPI_OUT SaHpiIdrAreaHeaderT *Header)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrAreaHeaderGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if ( ((AreaType < SAHPI_IDR_AREATYPE_INTERNAL_USE) ||
             ((AreaType > SAHPI_IDR_AREATYPE_PRODUCT_INFO) &&
             (AreaType != SAHPI_IDR_AREATYPE_UNSPECIFIED)  &&
             (AreaType != SAHPI_IDR_AREATYPE_OEM)) ||
             (AreaId == SAHPI_LAST_ENTRY)||
             (NextAreaId == NULL) ||
             (Header == NULL)))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaHeaderGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaHeaderGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &Idrid, &AreaType, &AreaId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextAreaId, Header);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrAreaAdd                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrAreaAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiIdrIdT       Idrid,
                             SAHPI_IN SaHpiIdrAreaTypeT AreaType,
                             SAHPI_OUT SaHpiEntryIdT    *AreaId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrAreaAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
        if (!oh_lookup_idrareatype(AreaType) ||
            AreaId == NULL)   {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED) {
                return SA_ERR_HPI_INVALID_DATA;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &Idrid, &AreaType);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, AreaId);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrAreaDelete                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrAreaDelete)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiIdrIdT       Idrid,
                             SAHPI_IN SaHpiEntryIdT     AreaId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrAreaDelete";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (AreaId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &Idrid, &AreaId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrFieldGet                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrFieldGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT    SessionId,
                             SAHPI_IN SaHpiResourceIdT   ResourceId,
                             SAHPI_IN SaHpiIdrIdT        Idrid,
                             SAHPI_IN SaHpiEntryIdT      AreaId,
                             SAHPI_IN SaHpiIdrFieldTypeT FieldType,
                             SAHPI_IN SaHpiEntryIdT      FieldId,
                             SAHPI_OUT SaHpiEntryIdT     *NextId,
                             SAHPI_OUT SaHpiIdrFieldT    *Field)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrFieldGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
        if (!Field ||
            !oh_lookup_idrfieldtype(FieldType) ||
            AreaId == SAHPI_LAST_ENTRY ||
            FieldId == SAHPI_LAST_ENTRY ||
            !NextId)    {
                cdebug_err("saHpiIdrFieldGet", "Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request, &SessionId, &ResourceId, &Idrid, &AreaId, &FieldType, &FieldId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextId, Field);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrFieldAdd                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrFieldAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT    SessionId,
                             SAHPI_IN SaHpiResourceIdT   ResourceId,
                             SAHPI_IN SaHpiIdrIdT        Idrid,
                             SAHPI_INOUT SaHpiIdrFieldT  *Field)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrFieldAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
        if (!Field)   {
        	cdebug_err("saHpiIdrFieldAdd", "Null Field")
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!oh_lookup_idrfieldtype(Field->Type)) {
        	cdebug_err("saHpiIdrFieldAdd", "Bad Field Type")
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (Field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
        	cdebug_err("saHpiIdrFieldAdd", "Unspecified Field Type")
        	return SA_ERR_HPI_INVALID_PARAMS;
        } else if (oh_valid_textbuffer(&Field->Field) != SAHPI_TRUE) {
        	cdebug_err("saHpiIdrFieldAdd", "Bad Text Buffer in Field")
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &Idrid, Field);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Field);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrFieldSet                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrFieldSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiIdrIdT      Idrid,
                             SAHPI_IN SaHpiIdrFieldT   *Field)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrFieldSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Field)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (Field->Type > SAHPI_IDR_FIELDTYPE_CUSTOM)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &Idrid, Field);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiIdrFieldDelete                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(IdrFieldDelete)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiIdrIdT      Idrid,
                             SAHPI_IN SaHpiEntryIdT    AreaId,
                             SAHPI_IN SaHpiEntryIdT    FieldId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiIdrFieldDelete";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (FieldId == SAHPI_LAST_ENTRY || AreaId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &Idrid, &AreaId, &FieldId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiWatchdogTimerGet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(WatchdogTimerGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiWatchdogNumT WatchdogNum,
                             SAHPI_OUT SaHpiWatchdogT   *Watchdog)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiWatchdogTimerGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Watchdog)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiWatchdogTimerGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiWatchdogTimerGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &WatchdogNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Watchdog);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiWatchdogTimerSet                                                      */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(WatchdogTimerSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiWatchdogNumT WatchdogNum,
                             SAHPI_IN SaHpiWatchdogT    *Watchdog)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiWatchdogTimerSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
        if (!Watchdog ||
            !oh_lookup_watchdogtimeruse(Watchdog->TimerUse) ||
            !oh_lookup_watchdogaction(Watchdog->TimerAction) ||
            !oh_lookup_watchdogpretimerinterrupt(Watchdog->PretimerInterrupt)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        if (Watchdog->PreTimeoutInterval > Watchdog->InitialCount) {
        	return SA_ERR_HPI_INVALID_DATA;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiWatchdogTimerSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiWatchdogTimerSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &WatchdogNum, Watchdog);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiWatchdogTimerReset                                                    */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(WatchdogTimerReset)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiWatchdogNumT WatchdogNum)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiWatchdogTimerReset";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiWatchdogTimerReset);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiWatchdogTimerReset, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &WatchdogNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorGetNext                                                    */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorGetNext)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
                             SAHPI_IN SaHpiSeverityT        Severity,
                             SAHPI_IN SaHpiBoolT            Unack,
                             SAHPI_INOUT SaHpiAnnouncementT *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorGetNext";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Announcement == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request, &SessionId, &ResourceId, &AnnNum, &Severity, &Unack, Announcement);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Announcement);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorGet                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT      SessionId,
                             SAHPI_IN SaHpiResourceIdT     ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
                             SAHPI_IN SaHpiEntryIdT        EntryId,
                             SAHPI_OUT SaHpiAnnouncementT  *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
        
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        
        if (Announcement == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &AnnNum, &EntryId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Announcement);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorAcknowledge                                                */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorAcknowledge)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT      SessionId,
                             SAHPI_IN SaHpiResourceIdT     ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
                             SAHPI_IN SaHpiEntryIdT        EntryId,
                             SAHPI_IN SaHpiSeverityT       Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorAcknowledge";
        pcstrmsock pinst;
        SaHpiSeverityT mysev = SAHPI_DEBUG;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
	
	if (EntryId == SAHPI_ENTRY_UNSPECIFIED) {
		if (!oh_lookup_severity(Severity)) {
	                return SA_ERR_HPI_INVALID_PARAMS;
	        } else {
	        	mysev = Severity;
	        }
	}
	
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;        

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorAcknowledge);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorAcknowledge, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &AnnNum, &EntryId, &mysev);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorAdd                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorAdd)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
                             SAHPI_INOUT SaHpiAnnouncementT *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorAdd";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;

        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        if (Announcement == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
                
        if (Announcement->Severity == SAHPI_ALL_SEVERITIES ||
            !oh_lookup_severity(Announcement->Severity) ||
            !oh_valid_textbuffer(&Announcement->StatusCond.Data) ||
            !oh_lookup_statuscondtype(Announcement->StatusCond.Type))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &AnnNum, Announcement);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Announcement);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorDelete                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorDelete)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT      SessionId,
                             SAHPI_IN SaHpiResourceIdT     ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
                             SAHPI_IN SaHpiEntryIdT        EntryId,
                             SAHPI_IN SaHpiSeverityT       Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorDelete";
        pcstrmsock pinst;
        SaHpiSeverityT mysev = SAHPI_DEBUG;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
		
	if (EntryId == SAHPI_ENTRY_UNSPECIFIED) {
		if (!oh_lookup_severity(Severity)) {
			printf("Bad severity %d passed in.\n", Severity);
	                return SA_ERR_HPI_INVALID_PARAMS;
		} else {
			mysev = Severity;
		}
	}

        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;        

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &SessionId, &ResourceId, &AnnNum, &EntryId, &mysev);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorModeGet                                                    */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorModeGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT        SessionId,
                             SAHPI_IN SaHpiResourceIdT       ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT   AnnNum,
                             SAHPI_OUT SaHpiAnnunciatorModeT *Mode)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorModeGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Mode == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorModeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorModeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &AnnNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Mode);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAnnunciatorModeSet                                                    */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AnnunciatorModeSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId,
                             SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
                             SAHPI_IN SaHpiAnnunciatorModeT Mode)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAnnunciatorModeSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_annunciatormode(Mode))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorModeSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorModeSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &SessionId, &ResourceId, &AnnNum, &Mode);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiHotSwapPolicyCancel                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(HotSwapPolicyCancel)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiHotSwapPolicyCancel";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapPolicyCancel);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapPolicyCancel, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceActiveSet                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceActiveSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceActiveSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceActiveSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceActiveSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceInactiveSet                                                   */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceInactiveSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT       SessionId,
                             SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceInactiveSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceInactiveSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceInactiveSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAutoInsertTimeoutGet                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AutoInsertTimeoutGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_OUT SaHpiTimeoutT  *Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAutoInsertTimeoutGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Timeout == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoInsertTimeoutGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoInsertTimeoutGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &SessionId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Timeout);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAutoInsertTimeoutSet                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AutoInsertTimeoutSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT SessionId,
                             SAHPI_IN SaHpiTimeoutT   Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAutoInsertTimeoutSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Timeout != SAHPI_TIMEOUT_IMMEDIATE &&
            Timeout != SAHPI_TIMEOUT_BLOCK &&
            Timeout < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoInsertTimeoutSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoInsertTimeoutSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &Timeout);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAutoExtractGet                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AutoExtractTimeoutGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_OUT SaHpiTimeoutT   *Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAutoExtractTimeoutGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Timeout)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoExtractTimeoutGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoExtractTimeoutGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Timeout);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiAutoExtractTimeoutSet                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(AutoExtractTimeoutSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiTimeoutT    Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiAutoExtractTimeoutSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (Timeout != SAHPI_TIMEOUT_IMMEDIATE &&
            Timeout != SAHPI_TIMEOUT_BLOCK &&
            Timeout < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoExtractTimeoutSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoExtractTimeoutSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Timeout);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiHotSwapStateGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(HotSwapStateGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_OUT SaHpiHsStateT   *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiHotSwapStateGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, State);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiHotSwapActionRequest                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(HotSwapActionRequest)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiHsActionT   Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiHotSwapActionRequest";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_hsaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapActionRequest);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapActionRequest, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Action);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiHotSwapIndicatorStateGet                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(HotSwapIndicatorStateGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT         SessionId,
                             SAHPI_IN SaHpiResourceIdT        ResourceId,
                             SAHPI_OUT SaHpiHsIndicatorStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiHotSwapIndicatorStateGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapIndicatorStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapIndicatorStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, State);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiHotSwapIndicatorStateSet                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(HotSwapIndicatorStateSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT        SessionId,
                             SAHPI_IN SaHpiResourceIdT       ResourceId,
                             SAHPI_IN SaHpiHsIndicatorStateT State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiHotSwapIndicatorStateSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_hsindicatorstate(State))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapIndicatorStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapIndicatorStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &State);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiParmControl                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ParmControl)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiParmActionT Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiParmControl";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_parmaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiParmControl);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiParmControl, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Action);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceResetStateGet                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceResetStateGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT    SessionId,
                             SAHPI_IN SaHpiResourceIdT   ResourceId,
                             SAHPI_OUT SaHpiResetActionT *Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceResetStateGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!Action)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceResetStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceResetStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Action);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceResetStateSet                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourceResetStateSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_IN SaHpiResetActionT Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourceResetStateSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_resetaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceResetStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceResetStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &Action);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourcePowerStateGet                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourcePowerStateGet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT   SessionId,
                             SAHPI_IN SaHpiResourceIdT  ResourceId,
                             SAHPI_OUT SaHpiPowerStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourcePowerStateGet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourcePowerStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourcePowerStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &SessionId, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, State);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourcePowerStateSet                                                 */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API dOpenHpiClientFunction(ResourcePowerStateSet)
	dOpenHpiClientParam (SAHPI_IN SaHpiSessionIdT  SessionId,
                             SAHPI_IN SaHpiResourceIdT ResourceId,
                             SAHPI_IN SaHpiPowerStateT State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "saHpiResourcePowerStateSet";
        pcstrmsock pinst;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        pinst = GetConnx(SessionId);
	if (pinst == NULL )
		return SA_ERR_HPI_INVALID_SESSION;
        if (!oh_lookup_powerstate(State))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourcePowerStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourcePowerStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &SessionId, &ResourceId, &State);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}

/*----------------------------------------------------------------------------*/
/* oHpiHandlerCreate                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiHandlerCreate(GHashTable *config,
                           oHpiHandlerIdT *id)
{
        SaErrorT err;
        void *request;
	char reply[dMaxMessageLength];
	char cmd[] = "oHpiHandlerCreate";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        // initialize the daemon GHashTable
        err = oHpiHandlerCreateInit();
        if (err) {
                return err;
        }

        // add each hash table entry to the daemon hash table
        g_hash_table_foreach(config, oHpiHandlerCreateAddTEntry, NULL);

        // now create the handler
        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerCreate);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerCreate, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &err);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, id);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiHandlerDestroy                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiHandlerDestroy(oHpiHandlerIdT id)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "oHpiHandlerDestroy";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerDestroy);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerDestroy, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiHandlerInfo                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiHandlerInfo(oHpiHandlerIdT id, oHpiHandlerInfoT *info)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "oHpiHandlerInfo";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerInfo);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerInfo, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, info);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiHandlerGetNext                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiHandlerGetNext(oHpiHandlerIdT id, oHpiHandlerIdT *next_id)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "oHpiHandlerGetNext";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, next_id);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}

/*----------------------------------------------------------------------------*/
/* oHpiHandlerFind                                                            */
/*----------------------------------------------------------------------------*/
SaErrorT oHpiHandlerFind(SaHpiSessionIdT sid,
			 SaHpiResourceIdT rid,
			 oHpiHandlerIdT *id)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "oHpiHandlerFind";  
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        if (!id || !sid || !rid) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
	*id = 0; //Initialize output var
	
        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerFind);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerFind, hm->m_request_len);
        request = malloc(hm->m_request_len);
        pinst->header.m_len = HpiMarshalRequest2(hm, request, &sid, &rid);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit,
                                    hm, 
                                    reply + sizeof(cMessageHeader),
                                    &err, 
                                    id);

        DeleteConnx(pinst);

        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

/*----------------------------------------------------------------------------*/
/* oHpiHandlerRetry                                                           */
/*----------------------------------------------------------------------------*/
SaErrorT oHpiHandlerRetry(oHpiHandlerIdT id)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "oHpiHandlerRetry";  
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        if (!id) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerRetry);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerRetry, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit,
                                    hm, 
                                    reply + sizeof(cMessageHeader),
                                    &err);

        DeleteConnx(pinst);

        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiGlobalParamGet                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiGlobalParamGet(oHpiGlobalParamT *param)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "oHpiGlobalParamGet";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiGlobalParamGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiGlobalParamGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, param);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, param);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiGlobalParamSet                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiGlobalParamSet(oHpiGlobalParamT *param)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err;
	char cmd[] = "oHpiGlobalParamSet";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiGlobalParamSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiGlobalParamSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, param);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, param);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiInjectEvent                                                            */
/*----------------------------------------------------------------------------*/
SaErrorT oHpiInjectEvent(oHpiHandlerIdT id,
                         SaHpiEventT *event,
                         SaHpiRptEntryT *rpte,
                         SaHpiRdrT *rdr)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "oHpiInjectEvent";  
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        if (!id) {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!event) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiInjectEvent);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiInjectEvent, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &id, event, rpte, rdr);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply4(pinst->header.m_flags & dMhEndianBit, 
                                    hm, 
                                    reply + sizeof(cMessageHeader),
                                    &err, 
                                    &id, 
                                    event, 
                                    rpte, 
                                    rdr);

        DeleteConnx(pinst);

        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

/*----------------------------------------------------------------------------*/
/* oHpiHandlerCreateInit                                                      */
/*----------------------------------------------------------------------------*/

static SaErrorT oHpiHandlerCreateInit(void)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = 0;
	char cmd[] = "oHpiHandlerCreateInit";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return SA_ERR_HPI_INVALID_SESSION;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerCreateInit);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerCreateInit, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &err);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        DeleteConnx(pinst);
        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* oHpiHandlerCreateAddTEntry                                                 */
/*----------------------------------------------------------------------------*/

static void oHpiHandlerCreateAddTEntry(gpointer key, gpointer value, gpointer data)
{
        oHpiTextBufferT newkey, newvalue;
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = 0;
	char cmd[] = "oHpiHandlerCreateInit";
        pcstrmsock pinst = CreateConnx();

        if (pinst == NULL )
                return;

        newkey.DataLength = strlen((char *)key);
        strcpy((char *)newkey.Data, (char *)key);
        newvalue.DataLength = strlen((char *)value);
        strcpy((char *)newvalue.Data, (char *)value);

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerCreateAddTEntry);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerCreateAddTEntry, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &newkey, &newvalue);

	SendRecvNoReturn(cmd);

        HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        DeleteConnx(pinst);
        if (request)
                free(request);

	return;
}
