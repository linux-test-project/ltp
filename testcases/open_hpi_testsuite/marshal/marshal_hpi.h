/*
 * marshaling/demarshaling of hpi functions
 *
 * Copyright (c) 2004 by FORCE Computers.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 *     W. David Ashley <dashley@us.ibm.com.com>
 *     Anton Pak <anton.pak@pigeonpoint.com>
 */

#ifndef dMarshalHpi_h
#define dMarshalHpi_h


#ifndef dMarshalHpiTypes_h
#include "marshal_hpi_types.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define dDefaultDaemonPort 4743


typedef enum
{
  eFsaHpiNull,
  eFsaHpiSessionOpen,
  eFsaHpiSessionClose,
  eFsaHpiDiscover,
  eFsaHpiDomainInfoGet,
  eFsaHpiDrtEntryGet,
  eFsaHpiDomainTagSet,
  eFsaHpiRptEntryGet,
  eFsaHpiRptEntryGetByResourceId,
  eFsaHpiResourceSeveritySet,
  eFsaHpiResourceTagSet,
  eFsaHpiResourceIdGet,
  eFsaHpiGetIdByEntityPath,
  eFsaHpiGetChildEntityPath,
  eFsaHpiResourceFailedRemove,
  eFsaHpiEventLogInfoGet,
  eFsaHpiEventLogCapabilitiesGet,
  eFsaHpiEventLogEntryGet,
  eFsaHpiEventLogEntryAdd,
  eFsaHpiEventLogClear,
  eFsaHpiEventLogTimeGet,
  eFsaHpiEventLogTimeSet,
  eFsaHpiEventLogStateGet,
  eFsaHpiEventLogStateSet,
  eFsaHpiEventLogOverflowReset,
  eFsaHpiSubscribe,
  eFsaHpiUnsubscribe,
  eFsaHpiEventGet,
  eFsaHpiEventAdd,
  eFsaHpiAlarmGetNext,
  eFsaHpiAlarmGet,
  eFsaHpiAlarmAcknowledge,
  eFsaHpiAlarmAdd,
  eFsaHpiAlarmDelete,
  eFsaHpiRdrGet,
  eFsaHpiRdrGetByInstrumentId,
  eFsaHpiSensorReadingGet,
  eFsaHpiSensorThresholdsGet,
  eFsaHpiSensorThresholdsSet,
  eFsaHpiSensorTypeGet,
  eFsaHpiSensorEnableGet,
  eFsaHpiSensorEnableSet,
  eFsaHpiSensorEventEnableGet,
  eFsaHpiSensorEventEnableSet,
  eFsaHpiSensorEventMasksGet,
  eFsaHpiSensorEventMasksSet,
  eFsaHpiControlTypeGet,
  eFsaHpiControlGet,
  eFsaHpiControlSet,
  eFsaHpiIdrInfoGet,
  eFsaHpiIdrAreaHeaderGet,
  eFsaHpiIdrAreaAdd,
  eFsaHpiIdrAreaAddById,
  eFsaHpiIdrAreaDelete,
  eFsaHpiIdrFieldGet,
  eFsaHpiIdrFieldAdd,
  eFsaHpiIdrFieldAddById,
  eFsaHpiIdrFieldSet,
  eFsaHpiIdrFieldDelete,
  eFsaHpiWatchdogTimerGet,
  eFsaHpiWatchdogTimerSet,
  eFsaHpiWatchdogTimerReset,
  eFsaHpiAnnunciatorGetNext,
  eFsaHpiAnnunciatorGet,
  eFsaHpiAnnunciatorAcknowledge,
  eFsaHpiAnnunciatorAdd,
  eFsaHpiAnnunciatorDelete,
  eFsaHpiAnnunciatorModeGet,
  eFsaHpiAnnunciatorModeSet,
  eFsaHpiDimiInfoGet,
  eFsaHpiDimiTestInfoGet,
  eFsaHpiDimiTestReadinessGet,
  eFsaHpiDimiTestStart,
  eFsaHpiDimiTestCancel,
  eFsaHpiDimiTestStatusGet,
  eFsaHpiDimiTestResultsGet,
  eFsaHpiFumiSourceSet,
  eFsaHpiFumiSourceInfoValidateStart,
  eFsaHpiFumiSourceInfoGet,
  eFsaHpiFumiTargetInfoGet,
  eFsaHpiFumiBackupStart,
  eFsaHpiFumiBankBootOrderSet,
  eFsaHpiFumiBankCopyStart,
  eFsaHpiFumiInstallStart,
  eFsaHpiFumiUpgradeStatusGet,
  eFsaHpiFumiTargetVerifyStart,
  eFsaHpiFumiUpgradeCancel,
  eFsaHpiFumiRollbackStart,
  eFsaHpiFumiActivate,
  eFsaHpiHotSwapPolicyCancel,
  eFsaHpiResourceActiveSet,
  eFsaHpiResourceInactiveSet,
  eFsaHpiAutoInsertTimeoutGet,
  eFsaHpiAutoInsertTimeoutSet,
  eFsaHpiAutoExtractTimeoutGet,
  eFsaHpiAutoExtractTimeoutSet,
  eFsaHpiHotSwapStateGet,
  eFsaHpiHotSwapActionRequest,
  eFsaHpiHotSwapIndicatorStateGet,
  eFsaHpiHotSwapIndicatorStateSet,
  eFsaHpiParmControl,
  eFsaHpiResourceLoadIdGet,
  eFsaHpiResourceLoadIdSet,
  eFsaHpiResourceResetStateGet,
  eFsaHpiResourceResetStateSet,
  eFsaHpiResourcePowerStateGet,
  eFsaHpiResourcePowerStateSet,
  eFoHpiHandlerCreate,
  eFoHpiHandlerDestroy,
  eFoHpiHandlerInfo,
  eFoHpiHandlerGetNext,
  eFoHpiHandlerFind,
  eFoHpiHandlerRetry,
  eFoHpiGlobalParamGet,
  eFoHpiGlobalParamSet,
  eFoHpiInjectEvent,

  // New B.03.01 functions
  // They are added to the end of enum in order to keep ABI compatibility
  eFsaHpiMyEntityPathGet,
  eFsaHpiRdrUpdateCountGet,
  eFsaHpiFumiSpecInfoGet,
  eFsaHpiFumiServiceImpactGet,
  eFsaHpiFumiSourceComponentInfoGet,
  eFsaHpiFumiTargetComponentInfoGet,
  eFsaHpiFumiLogicalTargetInfoGet,
  eFsaHpiFumiLogicalTargetComponentInfoGet,
  eFsaHpiFumiTargetVerifyMainStart,
  eFsaHpiFumiAutoRollbackDisableGet,
  eFsaHpiFumiAutoRollbackDisableSet,
  eFsaHpiFumiActivateStart,
  eFsaHpiFumiCleanup,

} tHpiFucntionId;


typedef struct
{
  int                  m_id;
  const cMarshalType **m_request;
  const cMarshalType **m_reply; // the first param is the result
  unsigned int         m_request_len;
  unsigned int         m_reply_len;
} cHpiMarshal;


#define dHpiMarshalEntry(name) \
{                              \
  eF ## name,                  \
  name ## In,                  \
  name ## Out,                 \
  0, 0                         \
}


cHpiMarshal *HpiMarshalFind( int id );

int HpiMarshalRequest ( cHpiMarshal *m, void *buffer, const void **params );
int HpiMarshalRequest1( cHpiMarshal *m, void *buffer, const void *p1 );
int HpiMarshalRequest2( cHpiMarshal *m, void *buffer, const void *p1, const void *p2 );
int HpiMarshalRequest3( cHpiMarshal *m, void *buffer, const void *p1, const void *p2, const void *p3 );
int HpiMarshalRequest4( cHpiMarshal *m, void *buffer, const void *p1, const void *p2, const void *p3, const void *p4 );
int HpiMarshalRequest5( cHpiMarshal *m, void *buffer, const void *p1, const void *p2, const void *p3, const void *p4, const void *p5 );
int HpiMarshalRequest6( cHpiMarshal *m, void *buffer, const void *p1, const void *p2, const void *p3, const void *p4, const void *p5, const void * p6 );

int HpiDemarshalRequest ( int byte_order, cHpiMarshal *m, const void *buffer, void **params );
int HpiDemarshalRequest1( int byte_order, cHpiMarshal *m, const void *buffer, void *p1 );
int HpiDemarshalRequest2( int byte_order, cHpiMarshal *m, const void *buffer, void *p1, void *p2 );
int HpiDemarshalRequest3( int byte_order, cHpiMarshal *m, const void *buffer, void *p1, void *p2, void *p3 );
int HpiDemarshalRequest4( int byte_order, cHpiMarshal *m, const void *buffer, void *p1, void *p2, void *p3, void *p4 );
int HpiDemarshalRequest5( int byte_order, cHpiMarshal *m, const void *buffer, void *p1, void *p2, void *p3, void *p4, void *p5 );
int HpiDemarshalRequest6( int byte_order, cHpiMarshal *m, const void *buffer, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6 );

int HpiMarshalReply ( cHpiMarshal *m, void *buffer, const void **params );
int HpiMarshalReply0( cHpiMarshal *m, void *buffer, const void *result );
int HpiMarshalReply1( cHpiMarshal *m, void *buffer, const void *result, const void *p1  );
int HpiMarshalReply2( cHpiMarshal *m, void *buffer, const void *result, const void *p1, const void *p2 );
int HpiMarshalReply3( cHpiMarshal *m, void *buffer, const void *result, const void *p1, const void *p2, const void *p3 );
int HpiMarshalReply4( cHpiMarshal *m, void *buffer, const void *result, const void *p1, const void *p2, const void *p3, const void *p4 );
int HpiMarshalReply5( cHpiMarshal *m, void *buffer, const void *result, const void *p1, const void *p2, const void *p3, const void *p4, const void *p5 );

int HpiDemarshalReply ( int byte_order, cHpiMarshal *m, const void *buffer, void **params );
int HpiDemarshalReply0( int byte_order, cHpiMarshal *m, const void *buffer, void *result );
int HpiDemarshalReply1( int byte_order, cHpiMarshal *m, const void *buffer, void *result, void *p1 );
int HpiDemarshalReply2( int byte_order, cHpiMarshal *m, const void *buffer, void *result, void *p1, void *p2 );
int HpiDemarshalReply3( int byte_order, cHpiMarshal *m, const void *buffer, void *result, void *p1, void *p2, void *p3 );
int HpiDemarshalReply4( int byte_order, cHpiMarshal *m, const void *buffer, void *result, void *p1, void *p2, void *p3, void *p4 );
int HpiDemarshalReply5( int byte_order, cHpiMarshal *m, const void *buffer, void *result, void *p1, void *p2, void *p3, void *p4, void *p5 );

#ifdef __cplusplus
}
#endif

#endif
