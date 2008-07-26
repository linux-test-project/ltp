/*      -*- linux-c -*-
 *
 * (C) Copright IBM Corp 2004-2006
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

#ifndef __OHPI_H
#define __OHPI_H

#include <SaHpi.h>
#include <glib.h>
#include <oh_utils.h>

#define OPENHPI_DEFAULT_DAEMON_PORT 4743
#define MAX_PLUGIN_NAME_LENGTH 32

typedef SaHpiUint32T oHpiHandlerIdT;

typedef struct {
        oHpiHandlerIdT id;
        char plugin_name[MAX_PLUGIN_NAME_LENGTH];
        SaHpiEntityPathT entity_root;
        int load_failed;
} oHpiHandlerInfoT;

typedef enum {
        OHPI_ON_EP = 1,
        OHPI_LOG_ON_SEV,
        OHPI_EVT_QUEUE_LIMIT,
        OHPI_DEL_SIZE_LIMIT,
        OHPI_DEL_SAVE,
        OHPI_DAT_SIZE_LIMIT,
        OHPI_DAT_USER_LIMIT,
	OHPI_DAT_SAVE,
        //OHPI_DEBUG,
        //OHPI_DEBUG_TRACE,
        //OHPI_DEBUG_LOCK,
        OHPI_PATH,
        OHPI_VARPATH,
        OHPI_CONF
} oHpiGlobalParamTypeT;

typedef union {
        SaHpiEntityPathT OnEP; /* Entity path where this openhpi instance runs */
        /* Log events of severity equal to ore more critical than this */
        SaHpiSeverityT LogOnSev;
        SaHpiUint32T EvtQueueLimit; /* Max events # allowed in session queue */
        SaHpiUint32T DelSizeLimit; /* Maximum size of domain event log */
        SaHpiBoolT DelSave; /* True if domain event log is to be saved to disk */
        SaHpiUint32T DatSizeLimit; /* Max alarms allowed in alarm table */
        SaHpiUint32T DatUserLimit; /* Max number of user alarms allowed */
	SaHpiBoolT DatSave; /* True if domain alarm table is to be saved to disk */
        //unsigned char Debug; /* 1 = YES, 0 = NO */
        //unsigned char DebugTrace; /* !0 = YES, 0 = NO */
        //unsigned char DebugLock; /* !0 = YES, 0 = NO */
        char Path[OH_MAX_TEXT_BUFFER_LENGTH]; /* Dir path to openhpi plugins */
        char VarPath[OH_MAX_TEXT_BUFFER_LENGTH]; /* Dir path for openhpi data */
        char Conf[OH_MAX_TEXT_BUFFER_LENGTH]; /* File path of openhpi configuration */
} oHpiGlobalParamUnionT;

typedef struct {
        oHpiGlobalParamTypeT Type;
        oHpiGlobalParamUnionT u;
} oHpiGlobalParamT;

/* Version function */
SaHpiUint64T oHpiVersionGet(void);

/* Exported OpenHPI handler (plugin instance) calls */
SaErrorT oHpiHandlerCreate(GHashTable *config,
                           oHpiHandlerIdT *id);
SaErrorT oHpiHandlerDestroy(oHpiHandlerIdT id);
SaErrorT oHpiHandlerInfo(oHpiHandlerIdT id, oHpiHandlerInfoT *info);
SaErrorT oHpiHandlerGetNext(oHpiHandlerIdT id, oHpiHandlerIdT *next_id);
SaErrorT oHpiHandlerFind(SaHpiSessionIdT sid,
			 SaHpiResourceIdT rid,
			 oHpiHandlerIdT *id);
SaErrorT oHpiHandlerRetry(oHpiHandlerIdT id);

/* Global parameters */
SaErrorT oHpiGlobalParamGet(oHpiGlobalParamT *param);
SaErrorT oHpiGlobalParamSet(oHpiGlobalParamT *param);

/* Injector */
SaErrorT oHpiInjectEvent(oHpiHandlerIdT id,
                         SaHpiEventT    *event,
                         SaHpiRptEntryT *rpte,
                         SaHpiRdrT *rdr);

#define OHPI_VERSION_GET(v, VER) \
{ \
        char version[] = VER; \
        char *start = version; \
        char *end = version; \
        v += (strtoull(start, &end, 10) << 48); \
        end++; \
        start = end; \
        v += (strtoull(start, &end, 10) << 32); \
        end++; \
        start = end; \
        v += (strtoull(start, &end, 10) << 16); \
}

#endif /*__OHPI_H*/
