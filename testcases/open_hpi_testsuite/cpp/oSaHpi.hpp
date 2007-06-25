/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *    W. David Ashley <dashley@us.ibm.com>
 */


#ifndef Included_oSaHpi
#define Included_oSaHpi

#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}


#include "oSaHpiAlarm.hpp"
#include "oSaHpiAnnouncement.hpp"
#include "oSaHpiAnnunciatorRec.hpp"
#include "oSaHpiCondition.hpp"
#include "oSaHpiCtrlDefaultMode.hpp"
#include "oSaHpiCtrlRec.hpp"
#include "oSaHpiCtrlRecAnalog.hpp"
#include "oSaHpiCtrlRecDigital.hpp"
#include "oSaHpiCtrlRecDiscrete.hpp"
#include "oSaHpiCtrlRecOem.hpp"
#include "oSaHpiCtrlRecStream.hpp"
#include "oSaHpiCtrlRecText.hpp"
#include "oSaHpiCtrlState.hpp"
#include "oSaHpiCtrlStateOem.hpp"
#include "oSaHpiCtrlStateStream.hpp"
#include "oSaHpiCtrlStateText.hpp"
#include "oSaHpiDomainEvent.hpp"
#include "oSaHpiDomainInfo.hpp"
#include "oSaHpiDrtEntry.hpp"
#include "oSaHpiEntity.hpp"
#include "oSaHpiEntityPath.hpp"
#include "oSaHpiEvent.hpp"
#include "oSaHpiEventLogEntry.hpp"
#include "oSaHpiEventLogInfo.hpp"
#include "oSaHpiHotSwapEvent.hpp"
#include "oSaHpiHpiSwEvent.hpp"
#include "oSaHpiIdrAreaHeader.hpp"
#include "oSaHpiIdrField.hpp"
#include "oSaHpiIdrInfo.hpp"
#include "oSaHpiInventoryRec.hpp"
#include "oSaHpiName.hpp"
#include "oSaHpiOemEvent.hpp"
#include "oSaHpiRdr.hpp"
#include "oSaHpiResourceEvent.hpp"
#include "oSaHpiResourceInfo.hpp"
#include "oSaHpiRptEntry.hpp"
#include "oSaHpiSensorDataFormat.hpp"
#include "oSaHpiSensorEnableChangeEvent.hpp"
#include "oSaHpiSensorEvent.hpp"
#include "oSaHpiSensorRange.hpp"
#include "oSaHpiSensorReading.hpp"
#include "oSaHpiSensorRec.hpp"
#include "oSaHpiSensorThdDefn.hpp"
#include "oSaHpiSensorThresholds.hpp"
#include "oSaHpiTextBuffer.hpp"
#include "oSaHpiTypesEnums.hpp"
#include "oSaHpiUserEvent.hpp"
#include "oSaHpiWatchdog.hpp"
#include "oSaHpiWatchdogEvent.hpp"
#include "oSaHpiWatchdogRec.hpp"

#endif

