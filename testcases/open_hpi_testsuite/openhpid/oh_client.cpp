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
 *      Anton Pak <anton.pak@pigeonpoint.com>
 *
 */

extern "C"
{
#include <string.h>
#include <glib.h>
#include <SaHpi.h>
#include <oHpi.h>
#include <oh_error.h>
#include <oh_domain.h>
#include <config.h>
}

#include <marshal_hpi.h>

#include "oh_client.h"
#include "oh_client_session.h"

#define client_dbg(cmd, str) dbg("%s: %s\n", cmd, str)
#define client_err(cmd, str) err("%s: %s\n", cmd, str)

/*----------------------------------------------------------------------------*/
/* Utility functions                                                          */
/*----------------------------------------------------------------------------*/

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

static void __dehash_config(gpointer key, gpointer value, gpointer data)
{
        oHpiHandlerConfigT *handler_config = (oHpiHandlerConfigT *)data;        
        
        strncpy((char *)handler_config->Params[handler_config->NumberOfParams].Name,
        	(const char *)key,
        	SAHPI_MAX_TEXT_BUFFER_LENGTH);
        strncpy((char *)handler_config->Params[handler_config->NumberOfParams].Value,
        	(const char *)value,
        	SAHPI_MAX_TEXT_BUFFER_LENGTH);

        handler_config->NumberOfParams = handler_config->NumberOfParams + 1;
        
	return;
}

/******************************************************************************/
/* HPI Client Layer                                                           */
/******************************************************************************/

/*----------------------------------------------------------------------------*/
/* saHpiVersionGet                                                            */
/*----------------------------------------------------------------------------*/

SaHpiVersionT SAHPI_API saHpiVersionGet (void)
{
        return SAHPI_INTERFACE_VERSION;
}

/*----------------------------------------------------------------------------*/
/* saHpiInitialize                                                            */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiInitialize(
        SAHPI_IN    SaHpiVersionT    RequestedVersion,
        SAHPI_IN    SaHpiUint32T     NumOptions,
        SAHPI_INOUT SaHpiInitOptionT *Options,
        SAHPI_OUT   SaHpiUint32T     *FailedOption,
        SAHPI_OUT   SaErrorT         *OptionError)
{
        if ( RequestedVersion != SAHPI_INTERFACE_VERSION ) {
            return SA_ERR_HPI_UNSUPPORTED_API;
        }
        if ( ( NumOptions != 0 ) && ( Options == NULL ) ) {
            return SA_ERR_HPI_INVALID_PARAMS;
        }

        // TODO implement more checks from section 5.2.1 of B.03.01 spec
        // Current implementation does not cover options check

        // TODO implement any library initialization code here
        // Current implementation does not utilize this function
        // 

        return SA_OK;
}

/*----------------------------------------------------------------------------*/
/* saHpiFinalize                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiFinalize()
{
        // TODO implement
        // TODO implement any library finalization code here
        // Current implementation does not utilize this function
        return SA_OK;
}

/*----------------------------------------------------------------------------*/
/* saHpiSessionOpen                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiSessionOpen(
	SAHPI_IN SaHpiDomainIdT   DomainId,
	SAHPI_OUT SaHpiSessionIdT *SessionId,
	SAHPI_IN  void            *SecurityParams)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err = SA_OK;
        SaHpiSessionIdT domain_sid = 0;
	char cmd[] = "saHpiSessionOpen";
        pcstrmsock pinst = NULL;
        SaHpiDomainIdT default_did = SAHPI_UNSPECIFIED_DOMAIN_ID;
	

        if (!SessionId || SecurityParams)
                return SA_ERR_HPI_INVALID_PARAMS;

        if (DomainId == SAHPI_UNSPECIFIED_DOMAIN_ID) (DomainId = OH_DEFAULT_DOMAIN_ID);

        err = oh_create_connx(DomainId, &pinst);
        if (err) {
                client_err(cmd, "Could not create client connection");
                return err;
        }

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSessionOpen);
	pinst->MessageHeaderInit(
                eMhMsg, 0, eFsaHpiSessionOpen, hm->m_request_len);
	request = malloc(hm->m_request_len);
        /* TODO: Does daemon need domain id param in this call*/
	pinst->header.m_len = HpiMarshalRequest1(hm, request, &default_did);
        SendRecv(0, cmd);

	int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader),
                                    &err, &domain_sid);

	if (request)
		free(request);
	if (err != SA_OK) {
		oh_delete_connx(pinst);
                return err;
        }
	if (mr < 0) {
                oh_delete_connx(pinst);
		return SA_ERR_HPI_INVALID_PARAMS;
        }
	
        *SessionId = oh_open_session(DomainId, domain_sid, pinst);

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiSessionClose                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiSessionClose(
	SAHPI_IN SaHpiSessionIdT SessionId)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err = SA_OK;
	char cmd[] = "saHpiSessionClose";
        pcstrmsock pinst;
        SaHpiSessionIdT dsid = 0;
		SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSessionClose);
	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSessionClose,
				 hm->m_request_len);
	request = malloc(hm->m_request_len);

	pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

	SendRecv(SessionId, cmd);

	int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit,
				    hm, reply + sizeof(cMessageHeader), &err);

	if (request)
		free(request);
                
        oh_close_session(SessionId);
        
	if (mr < 0)
		return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiDiscover                                                              */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiDiscover(
	SAHPI_IN SaHpiSessionIdT SessionId)
{
	void *request;
	char reply[dMaxMessageLength];
	SaErrorT err = SA_OK;
	char cmd[] = "saHpiDiscover";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
		

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;

        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

	cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDiscover);
	pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDiscover,
				 hm->m_request_len);
	request = malloc(hm->m_request_len);

	pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

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

SaErrorT SAHPI_API saHpiDomainInfoGet(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_OUT SaHpiDomainInfoT *DomainInfo)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiDomainInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
	
	/* oh_get_connx() places the real Domain Id to dsid */
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (DomainInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDomainInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDomainInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, DomainInfo);

	/* Set Domain Id to real Domain Id */
	DomainInfo->DomainId = did;
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

SaErrorT SAHPI_API saHpiDrtEntryGet(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiEntryIdT   EntryId,
	SAHPI_OUT SaHpiEntryIdT  *NextEntryId,
	SAHPI_OUT SaHpiDrtEntryT *DrtEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiDrtEntryGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
	
	/* oh_get_connx() places the real Domain Id to dsid */
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if((DrtEntry == NULL) ||
           (NextEntryId == NULL) ||
           (EntryId == SAHPI_LAST_ENTRY)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDrtEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDrtEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &EntryId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, NextEntryId, DrtEntry);

	/* Set Domain Id to real Domain Id */
	DrtEntry->DomainId = did;
	
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

SaErrorT SAHPI_API saHpiDomainTagSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiTextBufferT *DomainTag)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiDomainTagSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!DomainTag)
                return SA_ERR_HPI_INVALID_PARAMS;
	if (!oh_lookup_texttype(DomainTag->DataType))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDomainTagSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDomainTagSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, DomainTag);

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

SaErrorT SAHPI_API saHpiRptEntryGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiEntryIdT    EntryId,
	SAHPI_OUT SaHpiEntryIdT   *NextEntryId,
	SAHPI_OUT SaHpiRptEntryT  *RptEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiRptEntryGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if ((NextEntryId == NULL) || (RptEntry == NULL)) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        if (EntryId == SAHPI_LAST_ENTRY) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRptEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRptEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &EntryId);

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

SaErrorT SAHPI_API saHpiRptEntryGetByResourceId(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiRptEntryT  *RptEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiRptEntryGetByResourceId";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID ||
            RptEntry == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRptEntryGetByResourceId);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRptEntryGetByResourceId, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiResourceSeveritySet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSeverityT   Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceSeveritySet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceSeveritySet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceSeveritySet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Severity);

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

SaErrorT SAHPI_API saHpiResourceTagSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiTextBufferT *ResourceTag)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceTagSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (ResourceTag == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceTagSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceTagSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, ResourceTag);

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
/* saHpiMyEntityPathGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiMyEntityPathGet(
        SAHPI_IN  SaHpiSessionIdT  SessionId,
        SAHPI_OUT SaHpiEntityPathT *EntityPath)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiMyEntityPathGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (EntityPath == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiMyEntityPathGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiMyEntityPathGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, EntityPath);

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

SaErrorT SAHPI_API saHpiResourceIdGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_OUT SaHpiResourceIdT *ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceIdGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (ResourceId == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceIdGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceIdGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

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
/* saHpiGetIdByEntityPath                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiGetIdByEntityPath(
	SAHPI_IN    SaHpiSessionIdT    SessionId,
	SAHPI_IN    SaHpiEntityPathT   EntityPath,
	SAHPI_IN    SaHpiRdrTypeT      InstrumentType,
	SAHPI_INOUT SaHpiUint32T       *InstanceId,
	SAHPI_OUT   SaHpiResourceIdT   *ResourceId,
	SAHPI_OUT   SaHpiInstrumentIdT *InstrumentId,
	SAHPI_OUT   SaHpiUint32T       *RptUpdateCount)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiGetIdByEntityPath";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        SaHpiInstrumentIdT instrument_id;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (ResourceId == NULL || InstanceId == NULL ||
            *InstanceId == SAHPI_LAST_ENTRY || RptUpdateCount == NULL ||
            (InstrumentId == NULL && InstrumentType != SAHPI_NO_RECORD))
                return SA_ERR_HPI_INVALID_PARAMS;
        if (InstrumentId == NULL)
                InstrumentId = &instrument_id;
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiGetIdByEntityPath);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiGetIdByEntityPath, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid,
                                                 &EntityPath, &InstrumentType,
                                                 InstanceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply4(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader), &err,
                                    InstanceId, ResourceId, InstrumentId, RptUpdateCount);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiGetChildEntityPath                                                    */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiGetChildEntityPath(
	SAHPI_IN    SaHpiSessionIdT  SessionId,
	SAHPI_IN    SaHpiEntityPathT ParentEntityPath,
	SAHPI_INOUT SaHpiUint32T     *InstanceId,
	SAHPI_OUT   SaHpiEntityPathT *ChildEntityPath,
	SAHPI_OUT   SaHpiUint32T     *RptUpdateCount)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiGetChildEntityPath";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (InstanceId == NULL || *InstanceId == SAHPI_LAST_ENTRY ||
            RptUpdateCount == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiGetChildEntityPath);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiGetChildEntityPath, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid,
                                                 &ParentEntityPath,
                                                 InstanceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply3(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader), &err,
                                    InstanceId, ChildEntityPath, RptUpdateCount);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceFailedRemove                                                  */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiResourceFailedRemove(
	SAHPI_IN    SaHpiSessionIdT        SessionId,
	SAHPI_IN    SaHpiResourceIdT       ResourceId)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiResourceFailedRemove";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceFailedRemove);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceFailedRemove, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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
/* saHpiEventLogInfoGet                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiEventLogInfoGet(
	SAHPI_IN SaHpiSessionIdT     SessionId,
	SAHPI_IN SaHpiResourceIdT    ResourceId,
	SAHPI_OUT SaHpiEventLogInfoT *Info)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Info == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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
/* saHpiEventLogCapabilitiesGet                                               */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiEventLogCapabilitiesGet(
	SAHPI_IN SaHpiSessionIdT     SessionId,
	SAHPI_IN SaHpiResourceIdT    ResourceId,
	SAHPI_OUT SaHpiEventLogCapabilitiesT  *EventLogCapabilities)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiEventLogCapabilitiesGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (EventLogCapabilities == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogCapabilitiesGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogCapabilitiesGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader),
                                    &err, EventLogCapabilities);

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

SaErrorT SAHPI_API saHpiEventLogEntryGet(
	SAHPI_IN SaHpiSessionIdT        SessionId,
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
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogEntryGet";
        SaHpiRdrT tmp_rdr;
        SaHpiRptEntryT tmp_rpt;
        pcstrmsock pinst;
		SaHpiSessionIdT dsid =0;
        SaHpiDomainIdT did = 0;	

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!PrevEntryId || !EventLogEntry || !NextEntryId ||
            EntryId == SAHPI_NO_MORE_ENTRIES) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogEntryGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogEntryGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &EntryId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply5(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, PrevEntryId, NextEntryId, EventLogEntry, &tmp_rdr, &tmp_rpt);

        if (Rdr != NULL) {
                memcpy(Rdr, &tmp_rdr, sizeof(SaHpiRdrT));
        }
        if (RptEntry != NULL) {
                memcpy(RptEntry, &tmp_rpt, sizeof(SaHpiRptEntryT));
        }

	/* If this event is Domain Event, then adjust DomainId */
	if ((ResourceId == SAHPI_UNSPECIFIED_RESOURCE_ID) && 
		(EventLogEntry->Event.EventType == SAHPI_ET_DOMAIN)) {
		EventLogEntry->Event.EventDataUnion.DomainEvent.DomainId = did;
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

SaErrorT SAHPI_API saHpiEventLogEntryAdd(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiEventT      *EvtEntry)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogEntryAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
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

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, EvtEntry);

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

SaErrorT SAHPI_API saHpiEventLogClear(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogClear";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogClear);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogClear, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiEventLogTimeGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiTimeT      *Time)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogTimeGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Time == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogTimeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogTimeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiEventLogTimeSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiTimeT       Time)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogTimeSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogTimeSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogTimeSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Time);

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

SaErrorT SAHPI_API saHpiEventLogStateGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiBoolT      *EnableState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogStateGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (EnableState == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiEventLogStateSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiBoolT       EnableState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogStateSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &EnableState);

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

SaErrorT SAHPI_API saHpiEventLogOverflowReset(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventLogOverflowReset";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventLogOverflowReset);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventLogOverflowReset, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiSubscribe(
	SAHPI_IN SaHpiSessionIdT  SessionId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSubscribe";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSubscribe);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSubscribe, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

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

SaErrorT SAHPI_API saHpiUnsubscribe(
	SAHPI_IN SaHpiSessionIdT  SessionId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiUnsubscribe";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiUnsubscribe);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiUnsubscribe, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

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

SaErrorT SAHPI_API saHpiEventGet(
	SAHPI_IN SaHpiSessionIdT         SessionId,
	SAHPI_IN SaHpiTimeoutT           Timeout,
	SAHPI_OUT SaHpiEventT            *Event,
	SAHPI_INOUT SaHpiRdrT            *Rdr,
	SAHPI_INOUT SaHpiRptEntryT       *RptEntry,
	SAHPI_INOUT SaHpiEvtQueueStatusT *EventQueueStatus)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventGet";
        SaHpiRdrT tmp_rdr;
        SaHpiRptEntryT tmp_rpt;
        SaHpiEvtQueueStatusT tmp_status;
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Timeout < SAHPI_TIMEOUT_BLOCK || !Event)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &Timeout);

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

SaErrorT SAHPI_API saHpiEventAdd(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiEventT     *Event)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiEventAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        
	err = oh_valid_addevent(Event);
	if (err != SA_OK) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiEventAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiEventAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, Event);

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

SaErrorT SAHPI_API saHpiAlarmGetNext(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiSeverityT  Severity,
	SAHPI_IN SaHpiBoolT      Unack,
	SAHPI_INOUT SaHpiAlarmT  *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAlarmGetNext";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Alarm)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;
        if (Alarm->AlarmId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_NOT_PRESENT;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &Severity, &Unack, Alarm);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        /* Set Alarm DomainId to actual DomainID since openhpi core lib has DEFAULT_DOMAIN_ID */
        Alarm->AlarmCond.DomainId = did;
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

SaErrorT SAHPI_API saHpiAlarmGet(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiAlarmIdT   AlarmId,
	SAHPI_OUT SaHpiAlarmT    *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAlarmGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Alarm)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &AlarmId);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        /* Set Alarm DomainId to actual DomainID since openhpi core lib has DEFAULT_DOMAIN_ID */
        Alarm->AlarmCond.DomainId = did;
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

SaErrorT SAHPI_API saHpiAlarmAcknowledge(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiAlarmIdT   AlarmId,
	SAHPI_IN SaHpiSeverityT  Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAlarmAcknowledge";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (AlarmId == SAHPI_ENTRY_UNSPECIFIED &&
            !oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmAcknowledge);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmAcknowledge, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &AlarmId, &Severity);

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

SaErrorT SAHPI_API saHpiAlarmAdd(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_INOUT SaHpiAlarmT  *Alarm)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAlarmAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Alarm ||
            !oh_lookup_severity(Alarm->Severity) ||
            Alarm->AlarmCond.Type != SAHPI_STATUS_COND_TYPE_USER)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, Alarm);

	SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, Alarm);

        /* Set Alarm DomainId to actual DomainID since openhpi core lib has DEFAULT_DOMAIN_ID */
		Alarm->AlarmCond.DomainId = did;
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

SaErrorT SAHPI_API saHpiAlarmDelete(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiAlarmIdT   AlarmId,
	SAHPI_IN SaHpiSeverityT  Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAlarmDelete";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (AlarmId == SAHPI_ENTRY_UNSPECIFIED &&
            !oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAlarmDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAlarmDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &AlarmId, &Severity);

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

SaErrorT SAHPI_API saHpiRdrGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiEntryIdT    EntryId,
	SAHPI_OUT SaHpiEntryIdT   *NextEntryId,
	SAHPI_OUT SaHpiRdrT       *Rdr)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiRdrGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (EntryId == SAHPI_LAST_ENTRY || !Rdr || !NextEntryId)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRdrGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRdrGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &EntryId);

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

SaErrorT SAHPI_API saHpiRdrGetByInstrumentId(
	SAHPI_IN SaHpiSessionIdT    SessionId,
	SAHPI_IN SaHpiResourceIdT   ResourceId,
	SAHPI_IN SaHpiRdrTypeT      RdrType,
	SAHPI_IN SaHpiInstrumentIdT InstrumentId,
	SAHPI_OUT SaHpiRdrT         *Rdr)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiRdrGetByInstrumentId";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_rdrtype(RdrType) ||
            RdrType == SAHPI_NO_RECORD || !Rdr)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRdrGetByInstrumentId);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRdrGetByInstrumentId, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &RdrType, &InstrumentId);

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
/* saHpiRdrUpdateCountGet                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiRdrUpdateCountGet(
        SAHPI_IN  SaHpiSessionIdT  SessionId,
        SAHPI_IN  SaHpiResourceIdT ResourceId,
        SAHPI_OUT SaHpiUint32T     *UpdateCount)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiRdrUpdateCountGet";
        pcstrmsock pinst;
        SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (UpdateCount == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiRdrUpdateCountGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiRdrUpdateCountGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, UpdateCount);

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

SaErrorT SAHPI_API saHpiSensorReadingGet(
	SAHPI_IN SaHpiSessionIdT        SessionId,
	SAHPI_IN SaHpiResourceIdT       ResourceId,
	SAHPI_IN SaHpiSensorNumT        SensorNum,
	SAHPI_INOUT SaHpiSensorReadingT *Reading,
	SAHPI_INOUT SaHpiEventStateT    *EventState)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorReadingGet";
        SaHpiSensorReadingT tmp_reading;
        SaHpiEventStateT tmp_state;
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorReadingGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorReadingGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &SensorNum);

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

SaErrorT SAHPI_API saHpiSensorThresholdsGet(
	SAHPI_IN SaHpiSessionIdT         SessionId,
	SAHPI_IN SaHpiResourceIdT        ResourceId,
	SAHPI_IN SaHpiSensorNumT         SensorNum,
	SAHPI_OUT SaHpiSensorThresholdsT *Thresholds)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorThresholdsGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Thresholds)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorThresholdsGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorThresholdsGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &SensorNum);

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

SaErrorT SAHPI_API saHpiSensorThresholdsSet(
	SAHPI_IN SaHpiSessionIdT        SessionId,
	SAHPI_IN SaHpiResourceIdT       ResourceId,
	SAHPI_IN SaHpiSensorNumT        SensorNum,
	SAHPI_IN SaHpiSensorThresholdsT *Thresholds)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorThresholdsSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
	SaHpiSensorThresholdsT tmpthrds;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Thresholds)
                return SA_ERR_HPI_INVALID_PARAMS;

	err = clean_thresholds(Thresholds, &tmpthrds);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorThresholdsSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorThresholdsSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &SensorNum, &tmpthrds);

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

SaErrorT SAHPI_API saHpiSensorTypeGet(
	SAHPI_IN SaHpiSessionIdT      SessionId,
	SAHPI_IN SaHpiResourceIdT     ResourceId,
	SAHPI_IN SaHpiSensorNumT      SensorNum,
	SAHPI_OUT SaHpiSensorTypeT    *Type,
	SAHPI_OUT SaHpiEventCategoryT *Category)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorTypeGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Type || !Category)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorTypeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorTypeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &SensorNum);

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

SaErrorT SAHPI_API saHpiSensorEnableGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSensorNumT  SensorNum,
	SAHPI_OUT SaHpiBoolT      *Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEnableGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Enabled)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEnableGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEnableGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &SensorNum);

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

SaErrorT SAHPI_API saHpiSensorEnableSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSensorNumT  SensorNum,
	SAHPI_IN SaHpiBoolT       Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEnableSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEnableSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEnableSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &SensorNum, &Enabled);

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

SaErrorT SAHPI_API saHpiSensorEventEnableGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSensorNumT  SensorNum,
	SAHPI_OUT SaHpiBoolT      *Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEventEnableGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Enabled)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventEnableGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventEnableGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &SensorNum);

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

SaErrorT SAHPI_API saHpiSensorEventEnableSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSensorNumT  SensorNum,
	SAHPI_IN SaHpiBoolT       Enabled)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEventEnableSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventEnableSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventEnableSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &SensorNum, &Enabled);

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

SaErrorT SAHPI_API saHpiSensorEventMasksGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiSensorNumT  SensorNum,
	SAHPI_INOUT SaHpiEventStateT *Assert,
	SAHPI_INOUT SaHpiEventStateT *Deassert)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEventMasksGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        SaHpiEventStateT myassert = 0, mydeassert = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
		
	if (!Assert) Assert = &myassert;
	if (!Deassert) Deassert = &mydeassert;
	
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventMasksGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventMasksGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &SensorNum, Assert, Deassert);

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

SaErrorT SAHPI_API saHpiSensorEventMasksSet(
	SAHPI_IN SaHpiSessionIdT             SessionId,
	SAHPI_IN SaHpiResourceIdT            ResourceId,
	SAHPI_IN SaHpiSensorNumT             SensorNum,
	SAHPI_IN SaHpiSensorEventMaskActionT Action,
	SAHPI_IN SaHpiEventStateT            Assert,
	SAHPI_IN SaHpiEventStateT            Deassert)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiSensorEventMasksSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiSensorEventMasksSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiSensorEventMasksSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request,
              &dsid, &ResourceId, &SensorNum, &Action, &Assert, &Deassert);

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

SaErrorT SAHPI_API saHpiControlTypeGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiCtrlNumT    CtrlNum,
	SAHPI_OUT SaHpiCtrlTypeT  *Type)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiControlTypeGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Type)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiControlTypeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiControlTypeGet,
                                 hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid,
                                                 &ResourceId, &CtrlNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader), &err, Type);

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

SaErrorT SAHPI_API saHpiControlGet(
	SAHPI_IN SaHpiSessionIdT    SessionId,
	SAHPI_IN SaHpiResourceIdT   ResourceId,
	SAHPI_IN SaHpiCtrlNumT      CtrlNum,
	SAHPI_OUT SaHpiCtrlModeT    *Mode,
	SAHPI_INOUT SaHpiCtrlStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiControlGet";
        SaHpiCtrlModeT tmp_mode = SAHPI_CTRL_MODE_AUTO;
        SaHpiCtrlStateT tmp_state;
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	memset(&tmp_state, 0, sizeof(SaHpiCtrlStateT));

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        if (State != NULL) {
                memcpy(&tmp_state, State, sizeof(SaHpiCtrlStateT));
                if (oh_lookup_ctrltype(tmp_state.Type) == NULL) {
                        tmp_state.Type = SAHPI_CTRL_TYPE_TEXT;
                }
        }
        else {
                tmp_state.Type = SAHPI_CTRL_TYPE_TEXT;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiControlGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiControlGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid,
                                                 &ResourceId, &CtrlNum,
                                                 &tmp_state);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, &tmp_mode, &tmp_state);

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

SaErrorT SAHPI_API saHpiControlSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiCtrlNumT    CtrlNum,
	SAHPI_IN SaHpiCtrlModeT   Mode,
	SAHPI_IN SaHpiCtrlStateT  *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiControlSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        SaHpiCtrlStateT mystate, *pmystate = NULL;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
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

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &CtrlNum, &Mode, pmystate);

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

SaErrorT SAHPI_API saHpiIdrInfoGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiIdrIdT      Idrid,
	SAHPI_OUT SaHpiIdrInfoT   *Info)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Info == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Idrid);

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

SaErrorT SAHPI_API saHpiIdrAreaHeaderGet(
	SAHPI_IN SaHpiSessionIdT      SessionId,
	SAHPI_IN SaHpiResourceIdT     ResourceId,
	SAHPI_IN SaHpiIdrIdT          Idrid,
	SAHPI_IN SaHpiIdrAreaTypeT    AreaType,
	SAHPI_IN SaHpiEntryIdT        AreaId,
	SAHPI_OUT SaHpiEntryIdT       *NextAreaId,
	SAHPI_OUT SaHpiIdrAreaHeaderT *Header)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrAreaHeaderGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
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

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &Idrid, &AreaType, &AreaId);

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

SaErrorT SAHPI_API saHpiIdrAreaAdd(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiIdrIdT       Idrid,
	SAHPI_IN SaHpiIdrAreaTypeT AreaType,
	SAHPI_OUT SaHpiEntryIdT    *AreaId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrAreaAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        
        if (!oh_lookup_idrareatype(AreaType) ||
            AreaId == NULL)   {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED) {
                return SA_ERR_HPI_INVALID_DATA;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &Idrid, &AreaType);

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
/* saHpiIdrAreaAddById                                                        */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiIdrAreaAddById(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiIdrIdT       Idrid,
	SAHPI_IN SaHpiIdrAreaTypeT AreaType,
	SAHPI_IN SaHpiEntryIdT     AreaId)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiIdrAreaAddById";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        
        if (!oh_lookup_idrareatype(AreaType))   {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (AreaType == SAHPI_IDR_AREATYPE_UNSPECIFIED) {
                return SA_ERR_HPI_INVALID_DATA;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaAddById);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaAddById, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &Idrid, &AreaType, &AreaId);

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
/* saHpiIdrAreaDelete                                                         */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiIdrAreaDelete(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiIdrIdT       Idrid,
	SAHPI_IN SaHpiEntryIdT     AreaId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrAreaDelete";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (AreaId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrAreaDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrAreaDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &Idrid, &AreaId);

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

SaErrorT SAHPI_API saHpiIdrFieldGet(
	SAHPI_IN SaHpiSessionIdT    SessionId,
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
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrFieldGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        
        if (!Field ||
            !oh_lookup_idrfieldtype(FieldType) ||
            AreaId == SAHPI_LAST_ENTRY ||
            FieldId == SAHPI_LAST_ENTRY ||
            !NextId)    {
                client_err(cmd, "Invalid Parameters");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request, &dsid, &ResourceId, &Idrid, &AreaId, &FieldType, &FieldId);

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

SaErrorT SAHPI_API saHpiIdrFieldAdd(
	SAHPI_IN SaHpiSessionIdT    SessionId,
	SAHPI_IN SaHpiResourceIdT   ResourceId,
	SAHPI_IN SaHpiIdrIdT        Idrid,
	SAHPI_INOUT SaHpiIdrFieldT  *Field)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrFieldAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        
        if (!Field)   {
        	client_err(cmd, "Null Field");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!oh_lookup_idrfieldtype(Field->Type)) {
        	client_err(cmd, "Bad Field Type");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (Field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
        	client_err(cmd, "Unspecified Field Type");
        	return SA_ERR_HPI_INVALID_PARAMS;
        } else if (oh_valid_textbuffer(&Field->Field) != SAHPI_TRUE) {
        	client_err(cmd, "Bad Text Buffer in Field");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldAdd);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldAdd, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &Idrid, Field);

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
/* saHpiIdrFieldAddById                                                       */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiIdrFieldAddById(
	SAHPI_IN SaHpiSessionIdT    SessionId,
	SAHPI_IN SaHpiResourceIdT   ResourceId,
	SAHPI_IN SaHpiIdrIdT        Idrid,
	SAHPI_INOUT SaHpiIdrFieldT  *Field)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiIdrFieldAddById";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        
        if (!Field)   {
                client_err(cmd, "Null Field");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (!oh_lookup_idrfieldtype(Field->Type)) {
                client_err(cmd, "Bad Field Type");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (Field->Type == SAHPI_IDR_FIELDTYPE_UNSPECIFIED) {
                client_err(cmd, "Unspecified Field Type");
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (oh_valid_textbuffer(&Field->Field) != SAHPI_TRUE) {
                client_err(cmd, "Bad Text Buffer in Field");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldAddById);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldAddById, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &Idrid, Field);

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

SaErrorT SAHPI_API saHpiIdrFieldSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiIdrIdT      Idrid,
	SAHPI_IN SaHpiIdrFieldT   *Field)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrFieldSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Field)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (Field->Type > SAHPI_IDR_FIELDTYPE_CUSTOM)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &Idrid, Field);

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

SaErrorT SAHPI_API saHpiIdrFieldDelete(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiIdrIdT      Idrid,
	SAHPI_IN SaHpiEntryIdT    AreaId,
	SAHPI_IN SaHpiEntryIdT    FieldId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiIdrFieldDelete";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (FieldId == SAHPI_LAST_ENTRY || AreaId == SAHPI_LAST_ENTRY)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiIdrFieldDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiIdrFieldDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &Idrid, &AreaId, &FieldId);

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

SaErrorT SAHPI_API saHpiWatchdogTimerGet(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiWatchdogNumT WatchdogNum,
	SAHPI_OUT SaHpiWatchdogT   *Watchdog)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiWatchdogTimerGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Watchdog)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiWatchdogTimerGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiWatchdogTimerGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &WatchdogNum);

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

SaErrorT SAHPI_API saHpiWatchdogTimerSet(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiWatchdogNumT WatchdogNum,
	SAHPI_IN SaHpiWatchdogT    *Watchdog)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiWatchdogTimerSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        
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

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &WatchdogNum, Watchdog);

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

SaErrorT SAHPI_API saHpiWatchdogTimerReset(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiWatchdogNumT WatchdogNum)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiWatchdogTimerReset";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiWatchdogTimerReset);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiWatchdogTimerReset, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &WatchdogNum);

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

SaErrorT SAHPI_API saHpiAnnunciatorGetNext(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
	SAHPI_IN SaHpiSeverityT        Severity,
	SAHPI_IN SaHpiBoolT            Unack,
	SAHPI_INOUT SaHpiAnnouncementT *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorGetNext";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Announcement == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (!oh_lookup_severity(Severity))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest6(hm, request, &dsid, &ResourceId, &AnnNum, &Severity, &Unack, Announcement);

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

SaErrorT SAHPI_API saHpiAnnunciatorGet(
	SAHPI_IN SaHpiSessionIdT      SessionId,
	SAHPI_IN SaHpiResourceIdT     ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
	SAHPI_IN SaHpiEntryIdT        EntryId,
	SAHPI_OUT SaHpiAnnouncementT  *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        
	if (err) return err;
        
        if (Announcement == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &AnnNum, &EntryId);

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

SaErrorT SAHPI_API saHpiAnnunciatorAcknowledge(
	SAHPI_IN SaHpiSessionIdT      SessionId,
	SAHPI_IN SaHpiResourceIdT     ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
	SAHPI_IN SaHpiEntryIdT        EntryId,
	SAHPI_IN SaHpiSeverityT       Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorAcknowledge";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
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
	
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;        

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorAcknowledge);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorAcknowledge, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &AnnNum, &EntryId, &mysev);

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

SaErrorT SAHPI_API saHpiAnnunciatorAdd(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
	SAHPI_INOUT SaHpiAnnouncementT *Announcement)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorAdd";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;

        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

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

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &AnnNum, Announcement);

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

SaErrorT SAHPI_API saHpiAnnunciatorDelete(
	SAHPI_IN SaHpiSessionIdT      SessionId,
	SAHPI_IN SaHpiResourceIdT     ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT AnnNum,
	SAHPI_IN SaHpiEntryIdT        EntryId,
	SAHPI_IN SaHpiSeverityT       Severity)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorDelete";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
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

        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;        

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorDelete);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorDelete, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request, &dsid, &ResourceId, &AnnNum, &EntryId, &mysev);

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

SaErrorT SAHPI_API saHpiAnnunciatorModeGet(
	SAHPI_IN SaHpiSessionIdT        SessionId,
	SAHPI_IN SaHpiResourceIdT       ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT   AnnNum,
	SAHPI_OUT SaHpiAnnunciatorModeT *Mode)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorModeGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Mode == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorModeGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorModeGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &AnnNum);

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

SaErrorT SAHPI_API saHpiAnnunciatorModeSet(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT  AnnNum,
	SAHPI_IN SaHpiAnnunciatorModeT Mode)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAnnunciatorModeSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_annunciatormode(Mode))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAnnunciatorModeSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAnnunciatorModeSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request, &dsid, &ResourceId, &AnnNum, &Mode);

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
/* saHpiDimiInfoGet                                                           */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiDimiInfoGet(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiDimiNumT  	DimiNum,
	SAHPI_OUT SaHpiDimiInfoT   *DimiInfo)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiDimiInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!DimiInfo)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &DimiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, DimiInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

	return err;
}

SaErrorT SAHPI_API saHpiDimiTestInfoGet(
	SAHPI_IN    SaHpiSessionIdT      SessionId,
	SAHPI_IN    SaHpiResourceIdT     ResourceId,
	SAHPI_IN    SaHpiDimiNumT        DimiNum,
	SAHPI_IN    SaHpiDimiTestNumT    TestNum,
	SAHPI_OUT   SaHpiDimiTestT       *DimiTest)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (!DimiTest)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, DimiTest);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiDimiTestReadinessGet(
	SAHPI_IN    SaHpiSessionIdT      SessionId,
	SAHPI_IN    SaHpiResourceIdT     ResourceId,
	SAHPI_IN    SaHpiDimiNumT        DimiNum,
	SAHPI_IN    SaHpiDimiTestNumT    TestNum,
	SAHPI_OUT   SaHpiDimiReadyT      *DimiReady)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestReadinessGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (!DimiReady)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestReadinessGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestReadinessGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, DimiReady);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiDimiTestStart(
	SAHPI_IN  SaHpiSessionIdT                SessionId,
	SAHPI_IN  SaHpiResourceIdT               ResourceId,
	SAHPI_IN  SaHpiDimiNumT                  DimiNum,
	SAHPI_IN  SaHpiDimiTestNumT              TestNum,
	SAHPI_IN  SaHpiUint8T                    NumberOfParams,
	SAHPI_IN  SaHpiDimiTestVariableParamsT   *ParamsList)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        SaHpiDimiTestVariableParamsListT params_list;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (ParamsList == NULL && NumberOfParams != 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        params_list.NumberOfParams = NumberOfParams;
        params_list.ParamsList = ParamsList;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum,
                                &params_list);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiDimiTestCancel(
	SAHPI_IN    SaHpiSessionIdT      SessionId,
	SAHPI_IN    SaHpiResourceIdT     ResourceId,
	SAHPI_IN    SaHpiDimiNumT        DimiNum,
	SAHPI_IN    SaHpiDimiTestNumT    TestNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestCancel";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestCancel);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestCancel, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiDimiTestStatusGet(
	SAHPI_IN    SaHpiSessionIdT                   SessionId,
	SAHPI_IN    SaHpiResourceIdT                  ResourceId,
	SAHPI_IN    SaHpiDimiNumT                     DimiNum,
	SAHPI_IN    SaHpiDimiTestNumT                 TestNum,
	SAHPI_OUT   SaHpiDimiTestPercentCompletedT    *PercentCompleted,
	SAHPI_OUT   SaHpiDimiTestRunStatusT           *RunStatus)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestStatusGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        SaHpiDimiTestPercentCompletedT percent;
        SaHpiDimiTestPercentCompletedT *ppercent = &percent;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (RunStatus == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (PercentCompleted != NULL)
                ppercent = PercentCompleted;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestStatusGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestStatusGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, ppercent, RunStatus);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiDimiTestResultsGet(
	SAHPI_IN    SaHpiSessionIdT          SessionId,
	SAHPI_IN    SaHpiResourceIdT         ResourceId,
	SAHPI_IN    SaHpiDimiNumT            DimiNum,
	SAHPI_IN    SaHpiDimiTestNumT        TestNum,
	SAHPI_OUT   SaHpiDimiTestResultsT    *TestResults)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiDimiTestResultsGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (TestResults == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiDimiTestResultsGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiDimiTestResultsGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &DimiNum, &TestNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, TestResults);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

/*******************************************************************************
 *
 * FUMI Functions
 *
 ******************************************************************************/

SaErrorT SAHPI_API saHpiFumiSpecInfoGet(
        SAHPI_IN    SaHpiSessionIdT    SessionId,
        SAHPI_IN    SaHpiResourceIdT   ResourceId,
        SAHPI_IN    SaHpiFumiNumT      FumiNum,
        SAHPI_OUT   SaHpiFumiSpecInfoT *SpecInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiSpecInfoGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (SpecInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiSpecInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiSpecInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, SpecInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiServiceImpactGet(
        SAHPI_IN   SaHpiSessionIdT              SessionId,
        SAHPI_IN   SaHpiResourceIdT             ResourceId,
        SAHPI_IN   SaHpiFumiNumT                FumiNum,
        SAHPI_OUT  SaHpiFumiServiceImpactDataT  *ServiceImpact)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiServiceImpactGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (ServiceImpact == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiServiceImpactGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiServiceImpactGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, ServiceImpact);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}



SaErrorT SAHPI_API saHpiFumiSourceSet (
	SAHPI_IN    SaHpiSessionIdT         SessionId,
	SAHPI_IN    SaHpiResourceIdT        ResourceId,
	SAHPI_IN    SaHpiFumiNumT           FumiNum,
	SAHPI_IN    SaHpiBankNumT           BankNum,
	SAHPI_IN    SaHpiTextBufferT        *SourceUri)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiSourceSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (SourceUri == NULL || SourceUri->DataType != SAHPI_TL_TYPE_TEXT)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiSourceSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiSourceSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum,
                                SourceUri);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiSourceInfoValidateStart (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiSourceInfoValidateStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiSourceInfoValidateStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiSourceInfoValidateStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiSourceInfoGet (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum,
	SAHPI_OUT   SaHpiFumiSourceInfoT  *SourceInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiSourceInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (SourceInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiSourceInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiSourceInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, SourceInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiSourceComponentInfoGet(
        SAHPI_IN    SaHpiSessionIdT         SessionId,
        SAHPI_IN    SaHpiResourceIdT        ResourceId,
        SAHPI_IN    SaHpiFumiNumT           FumiNum,
        SAHPI_IN    SaHpiBankNumT           BankNum,
        SAHPI_IN    SaHpiEntryIdT           ComponentEntryId,
        SAHPI_OUT   SaHpiEntryIdT           *NextComponentEntryId,
        SAHPI_OUT   SaHpiFumiComponentInfoT *ComponentInfo)
{               
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiSourceComponentInfoGet"; 
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        
        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (NextComponentEntryId == NULL || ComponentInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (ComponentEntryId == SAHPI_LAST_ENTRY )
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiSourceComponentInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiSourceComponentInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum, &ComponentEntryId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, NextComponentEntryId, ComponentInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiTargetInfoGet (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum,
	SAHPI_OUT   SaHpiFumiBankInfoT    *BankInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiTargetInfoGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (BankInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiTargetInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiTargetInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, BankInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiTargetComponentInfoGet(
        SAHPI_IN    SaHpiSessionIdT         SessionId,
        SAHPI_IN    SaHpiResourceIdT        ResourceId,
        SAHPI_IN    SaHpiFumiNumT           FumiNum,
        SAHPI_IN    SaHpiBankNumT           BankNum,
        SAHPI_IN    SaHpiEntryIdT           ComponentEntryId,
        SAHPI_OUT   SaHpiEntryIdT           *NextComponentEntryId,
        SAHPI_OUT   SaHpiFumiComponentInfoT *ComponentInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiTargetComponentInfoGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (NextComponentEntryId == NULL || ComponentInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (ComponentEntryId == SAHPI_LAST_ENTRY )
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiTargetComponentInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiTargetComponentInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &FumiNum, &BankNum, &ComponentEntryId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, NextComponentEntryId, ComponentInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiLogicalTargetInfoGet(
        SAHPI_IN    SaHpiSessionIdT             SessionId,
        SAHPI_IN    SaHpiResourceIdT            ResourceId,
        SAHPI_IN    SaHpiFumiNumT               FumiNum,
        SAHPI_OUT   SaHpiFumiLogicalBankInfoT   *BankInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiLogicalTargetInfoGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (BankInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiLogicalTargetInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiLogicalTargetInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, BankInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiLogicalTargetComponentInfoGet(
        SAHPI_IN    SaHpiSessionIdT                SessionId,
        SAHPI_IN    SaHpiResourceIdT               ResourceId,
        SAHPI_IN    SaHpiFumiNumT                  FumiNum,
        SAHPI_IN    SaHpiEntryIdT                  ComponentEntryId,
        SAHPI_OUT   SaHpiEntryIdT                  *NextComponentEntryId,
        SAHPI_OUT   SaHpiFumiLogicalComponentInfoT *ComponentInfo)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiLogicalTargetComponentInfoGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (NextComponentEntryId == NULL || ComponentInfo == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (ComponentEntryId == SAHPI_LAST_ENTRY )
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiLogicalTargetComponentInfoGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiLogicalTargetComponentInfoGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &ComponentEntryId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply2(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, NextComponentEntryId, ComponentInfo);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiBackupStart(
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiBackupStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiBackupStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiBackupStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiBankBootOrderSet (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum,
	SAHPI_IN    SaHpiUint32T          Position)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiBankBootOrderSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiBankBootOrderSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiBankBootOrderSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum, &Position);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiBankCopyStart(
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         SourceBankNum,
	SAHPI_IN    SaHpiBankNumT         TargetBankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiBankCopyStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiBankCopyStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiBankCopyStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest5(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &SourceBankNum, &TargetBankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiInstallStart (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiInstallStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiInstallStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiInstallStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiUpgradeStatusGet (
	SAHPI_IN    SaHpiSessionIdT         SessionId,
	SAHPI_IN    SaHpiResourceIdT        ResourceId,
	SAHPI_IN    SaHpiFumiNumT           FumiNum,
	SAHPI_IN    SaHpiBankNumT           BankNum,
	SAHPI_OUT   SaHpiFumiUpgradeStatusT *UpgradeStatus)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiUpgradeStatusGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (UpgradeStatus == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiUpgradeStatusGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiUpgradeStatusGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, UpgradeStatus);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiTargetVerifyStart (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiTargetVerifyStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiTargetVerifyStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiTargetVerifyStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiTargetVerifyMainStart(
        SAHPI_IN    SaHpiSessionIdT  SessionId,
        SAHPI_IN    SaHpiResourceIdT ResourceId,
        SAHPI_IN    SaHpiFumiNumT    FumiNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiTargetVerifyMainStart";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;
        
        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        
        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiTargetVerifyMainStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiTargetVerifyMainStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum );
                
        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request) 
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiUpgradeCancel (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum,
	SAHPI_IN    SaHpiBankNumT         BankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiUpgradeCancel";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiUpgradeCancel);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiUpgradeCancel, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiAutoRollbackDisableGet(
        SAHPI_IN    SaHpiSessionIdT  SessionId,
        SAHPI_IN    SaHpiResourceIdT ResourceId,
        SAHPI_IN    SaHpiFumiNumT    FumiNum,
        SAHPI_OUT   SaHpiBoolT       *Disable)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiAutoRollbackDisableGet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (Disable == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiAutoRollbackDisableGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiAutoRollbackDisableGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum );

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, Disable);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiAutoRollbackDisableSet(
        SAHPI_IN    SaHpiSessionIdT  SessionId,
        SAHPI_IN    SaHpiResourceIdT ResourceId,
        SAHPI_IN    SaHpiFumiNumT    FumiNum,
        SAHPI_IN    SaHpiBoolT       Disable)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiAutoRollbackDisableSet";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiAutoRollbackDisableSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiAutoRollbackDisableSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &Disable );

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiRollbackStart (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiRollbackStart";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiRollbackStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiRollbackStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiActivate (
	SAHPI_IN    SaHpiSessionIdT       SessionId,
	SAHPI_IN    SaHpiResourceIdT      ResourceId,
	SAHPI_IN    SaHpiFumiNumT         FumiNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiActivate";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiActivate);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiActivate, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request,
                                &dsid, &ResourceId, &FumiNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiActivateStart(
        SAHPI_IN    SaHpiSessionIdT  SessionId,
        SAHPI_IN    SaHpiResourceIdT ResourceId,
        SAHPI_IN    SaHpiFumiNumT    FumiNum,
        SAHPI_IN    SaHpiBoolT       Logical)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiActivateStart";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiActivateStart);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiActivateStart, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum, &Logical);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}

SaErrorT SAHPI_API saHpiFumiCleanup(
        SAHPI_IN    SaHpiSessionIdT  SessionId,
        SAHPI_IN    SaHpiResourceIdT ResourceId,
        SAHPI_IN    SaHpiFumiNumT    FumiNum,
        SAHPI_IN    SaHpiBankNumT    BankNum)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiFumiCleanup";
        pcstrmsock pinst;
        SaHpiDomainIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiFumiCleanup);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiFumiCleanup, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest4(hm, request,
                                &dsid, &ResourceId, &FumiNum,
                                &BankNum);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

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

SaErrorT SAHPI_API saHpiHotSwapPolicyCancel(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiHotSwapPolicyCancel";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapPolicyCancel);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapPolicyCancel, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiResourceActiveSet(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceActiveSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceActiveSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceActiveSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiResourceInactiveSet(
	SAHPI_IN SaHpiSessionIdT       SessionId,
	SAHPI_IN SaHpiResourceIdT      ResourceId)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceInactiveSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceInactiveSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceInactiveSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiAutoInsertTimeoutGet(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_OUT SaHpiTimeoutT  *Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAutoInsertTimeoutGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Timeout == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoInsertTimeoutGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoInsertTimeoutGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &dsid);

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

SaErrorT SAHPI_API saHpiAutoInsertTimeoutSet(
	SAHPI_IN SaHpiSessionIdT SessionId,
	SAHPI_IN SaHpiTimeoutT   Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAutoInsertTimeoutSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Timeout != SAHPI_TIMEOUT_IMMEDIATE &&
            Timeout != SAHPI_TIMEOUT_BLOCK &&
            Timeout < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoInsertTimeoutSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoInsertTimeoutSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &Timeout);

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

SaErrorT SAHPI_API saHpiAutoExtractTimeoutGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiTimeoutT   *Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAutoExtractTimeoutGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Timeout)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoExtractTimeoutGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoExtractTimeoutGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiAutoExtractTimeoutSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiTimeoutT    Timeout)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiAutoExtractTimeoutSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (Timeout != SAHPI_TIMEOUT_IMMEDIATE &&
            Timeout != SAHPI_TIMEOUT_BLOCK &&
            Timeout < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiAutoExtractTimeoutSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiAutoExtractTimeoutSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Timeout);

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

SaErrorT SAHPI_API saHpiHotSwapStateGet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiHsStateT   *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiHotSwapStateGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiHotSwapActionRequest(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiHsActionT   Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiHotSwapActionRequest";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_hsaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapActionRequest);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapActionRequest, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Action);

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

SaErrorT SAHPI_API saHpiHotSwapIndicatorStateGet(
	SAHPI_IN SaHpiSessionIdT         SessionId,
	SAHPI_IN SaHpiResourceIdT        ResourceId,
	SAHPI_OUT SaHpiHsIndicatorStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiHotSwapIndicatorStateGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapIndicatorStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapIndicatorStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiHotSwapIndicatorStateSet(
	SAHPI_IN SaHpiSessionIdT        SessionId,
	SAHPI_IN SaHpiResourceIdT       ResourceId,
	SAHPI_IN SaHpiHsIndicatorStateT State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiHotSwapIndicatorStateSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_hsindicatorstate(State))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiHotSwapIndicatorStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiHotSwapIndicatorStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &State);

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

SaErrorT SAHPI_API saHpiParmControl(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiParmActionT Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiParmControl";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_parmaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiParmControl);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiParmControl, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Action);

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
/* saHpiResourceLoadIdGet                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiResourceLoadIdGet(
	SAHPI_IN  SaHpiSessionIdT  SessionId,
	SAHPI_IN  SaHpiResourceIdT ResourceId,
	SAHPI_OUT SaHpiLoadIdT *LoadId)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiResourceLoadIdGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceLoadIdGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceLoadIdGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err, LoadId);

        if (request)
                free(request);
        if (pinst->header.m_type == eMhError)
                return SA_ERR_HPI_INVALID_PARAMS;
        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}


/*----------------------------------------------------------------------------*/
/* saHpiResourceLoadIdSet                                                     */
/*----------------------------------------------------------------------------*/

SaErrorT SAHPI_API saHpiResourceLoadIdSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiLoadIdT *LoadId)
{
        void *request;
        char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
        char cmd[] = "saHpiResourceLoadIdSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

        if (SessionId == 0)
                return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
        if (err) return err;
        if (LoadId == NULL)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceLoadIdSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceLoadIdSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, LoadId);

        SendRecv(SessionId, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm,
                                    reply + sizeof(cMessageHeader),
                                    &err);

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

SaErrorT SAHPI_API saHpiResourceResetStateGet(
	SAHPI_IN SaHpiSessionIdT    SessionId,
	SAHPI_IN SaHpiResourceIdT   ResourceId,
	SAHPI_OUT SaHpiResetActionT *Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceResetStateGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!Action)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceResetStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceResetStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiResourceResetStateSet(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_IN SaHpiResetActionT Action)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourceResetStateSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_resetaction(Action))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourceResetStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourceResetStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &Action);

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

SaErrorT SAHPI_API saHpiResourcePowerStateGet(
	SAHPI_IN SaHpiSessionIdT   SessionId,
	SAHPI_IN SaHpiResourceIdT  ResourceId,
	SAHPI_OUT SaHpiPowerStateT *State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourcePowerStateGet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!State)
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourcePowerStateGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourcePowerStateGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest2(hm, request, &dsid, &ResourceId);

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

SaErrorT SAHPI_API saHpiResourcePowerStateSet(
	SAHPI_IN SaHpiSessionIdT  SessionId,
	SAHPI_IN SaHpiResourceIdT ResourceId,
	SAHPI_IN SaHpiPowerStateT State)
{
        void *request;
	char reply[dMaxMessageLength];
        SaErrorT err = SA_OK;
	char cmd[] = "saHpiResourcePowerStateSet";
        pcstrmsock pinst;
		SaHpiSessionIdT dsid = 0;
        SaHpiDomainIdT did = 0;

	if (SessionId == 0)
		return SA_ERR_HPI_INVALID_SESSION;
        err = oh_get_connx(SessionId, &dsid, &pinst, &did);
	if (err) return err;
        if (!oh_lookup_powerstate(State))
                return SA_ERR_HPI_INVALID_PARAMS;

        cHpiMarshal *hm = HpiMarshalFind(eFsaHpiResourcePowerStateSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFsaHpiResourcePowerStateSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest3(hm, request, &dsid, &ResourceId, &State);

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
/* oHpiVersionGet                                                             */
/*----------------------------------------------------------------------------*/

SaHpiUint64T oHpiVersionGet(void)
{
        SaHpiUint64T v = 0;

        OHPI_VERSION_GET(v, VERSION);

        return v;
}

/*----------------------------------------------------------------------------*/
/* oHpiHandlerCreate                                                          */
/*----------------------------------------------------------------------------*/

SaErrorT oHpiHandlerCreate(GHashTable *config,
                           oHpiHandlerIdT *id)
{
        SaErrorT err = SA_OK;
        void *request;
	char reply[dMaxMessageLength];
	char cmd[] = "oHpiHandlerCreate";
        /* TODO: Change oHpi apis to send a domain id */
        pcstrmsock pinst = NULL;
        oHpiHandlerConfigT handler_config;

        if (!config || !id)
        	return SA_ERR_HPI_INVALID_PARAMS;
        
        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        handler_config.NumberOfParams = 0;
        handler_config.Params = (oHpiHandlerConfigParamT *)g_malloc0(sizeof(oHpiHandlerConfigParamT)*g_hash_table_size(config));
        // add each hash table entry to the marshable handler_config
        g_hash_table_foreach(config, __dehash_config, &handler_config);

        // now create the handler
        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerCreate);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerCreate, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &handler_config);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, id);

        oh_delete_connx(pinst);
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
        SaErrorT err = SA_OK;
	char cmd[] = "oHpiHandlerDestroy";
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerDestroy);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerDestroy, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply0(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err);

        oh_delete_connx(pinst);
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
        SaErrorT err = SA_OK;
	char cmd[] = "oHpiHandlerInfo";
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerInfo);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerInfo, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, info);

        oh_delete_connx(pinst);
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
        SaErrorT err = SA_OK;
	char cmd[] = "oHpiHandlerGetNext";
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiHandlerGetNext);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiHandlerGetNext, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, &id);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, next_id);

        oh_delete_connx(pinst);
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
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

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

        oh_delete_connx(pinst);

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
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

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

        oh_delete_connx(pinst);

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
        SaErrorT err = SA_OK;
	char cmd[] = "oHpiGlobalParamGet";
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiGlobalParamGet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiGlobalParamGet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, param);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, param);

        oh_delete_connx(pinst);
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
        SaErrorT err = SA_OK;
	char cmd[] = "oHpiGlobalParamSet";
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

        cHpiMarshal *hm = HpiMarshalFind(eFoHpiGlobalParamSet);
        pinst->MessageHeaderInit(eMhMsg, 0, eFoHpiGlobalParamSet, hm->m_request_len);
        request = malloc(hm->m_request_len);

        pinst->header.m_len = HpiMarshalRequest1(hm, request, param);

        SendRecv(0, cmd);

        int mr = HpiDemarshalReply1(pinst->header.m_flags & dMhEndianBit, hm, reply + sizeof(cMessageHeader), &err, param);

        oh_delete_connx(pinst);
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
        pcstrmsock pinst = NULL;

        err = oh_create_connx(SAHPI_UNSPECIFIED_DOMAIN_ID, &pinst);
        if (err) return err;

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
                                    hm, reply + sizeof(cMessageHeader),
                                    &err, &id, event, rpte, rdr);

        oh_delete_connx(pinst);

        if (request)
                free(request);

        if (mr < 0)
                return SA_ERR_HPI_INVALID_PARAMS;

        return err;
}
