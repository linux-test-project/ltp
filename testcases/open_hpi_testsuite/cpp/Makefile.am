#      -*- linux-c -*-
#
# (C) Copyright IBM Corp. 2005
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.
#
# Author(s):
#        W. David Ashley <dashley@us.ibm.com>
#

SUBDIRS                 = t
DIST_SUBDIRS            = t

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= @OPENHPI_INCLUDES@

includedir=$(base_includedir)/openhpi
include_HEADERS		= oSaHpiAlarm.hpp \
                  	  oSaHpiAnnouncement.hpp \
                  	  oSaHpiAnnunciatorRec.hpp \
                  	  oSaHpiCondition.hpp \
                  	  oSaHpiCtrlDefaultMode.hpp \
                  	  oSaHpiCtrlRec.hpp \
                  	  oSaHpiCtrlRecAnalog.hpp \
                  	  oSaHpiCtrlRecDigital.hpp \
                  	  oSaHpiCtrlRecDiscrete.hpp \
                  	  oSaHpiCtrlRecOem.hpp \
                  	  oSaHpiCtrlRecStream.hpp \
                  	  oSaHpiCtrlRecText.hpp \
                  	  oSaHpiCtrlState.hpp \
                  	  oSaHpiCtrlStateOem.hpp \
                  	  oSaHpiCtrlStateStream.hpp \
                  	  oSaHpiCtrlStateText.hpp \
                  	  oSaHpiDomainEvent.hpp \
                  	  oSaHpiDomainInfo.hpp \
                  	  oSaHpiDrtEntry.hpp \
                  	  oSaHpiEntity.hpp \
                  	  oSaHpiEntityPath.hpp \
                  	  oSaHpiEvent.hpp \
                  	  oSaHpiEventLogEntry.hpp \
                  	  oSaHpiEventLogInfo.hpp \
                  	  oSaHpiHotSwapEvent.hpp \
                  	  oSaHpiHpiSwEvent.hpp \
                  	  oSaHpiIdrAreaHeader.hpp \
                  	  oSaHpiIdrField.hpp \
                  	  oSaHpiIdrInfo.hpp \
                  	  oSaHpiInventoryRec.hpp \
                  	  oSaHpiName.hpp \
                  	  oSaHpiOemEvent.hpp \
                  	  oSaHpiRdr.hpp \
                  	  oSaHpiResourceEvent.hpp \
                  	  oSaHpiResourceInfo.hpp \
                  	  oSaHpiRptEntry.hpp \
                  	  oSaHpiSensorDataFormat.hpp \
                  	  oSaHpiSensorEnableChangeEvent.hpp \
                  	  oSaHpiSensorEvent.hpp \
                  	  oSaHpiSensorRange.hpp \
                  	  oSaHpiSensorReading.hpp \
                  	  oSaHpiSensorRec.hpp \
                  	  oSaHpiSensorThdDefn.hpp \
                  	  oSaHpiSensorThresholds.hpp \
                  	  oSaHpiTextBuffer.hpp \
                  	  oSaHpiTypesEnums.hpp \
                  	  oSaHpiUserEvent.hpp \
                  	  oSaHpiWatchdog.hpp \
                  	  oSaHpiWatchdogEvent.hpp \
                  	  oSaHpiWatchdogRec.hpp \
                  	  oSaHpi.hpp

pkglib_LTLIBRARIES	= libosahpi.la

libosahpi_la_LIBADD	= -luuid
libosahpi_la_SOURCES	= oSaHpiAlarm.cpp oSaHpiAlarm.hpp \
                          oSaHpiAnnouncement.cpp oSaHpiAnnouncement.hpp \
                          oSaHpiAnnunciatorRec.cpp oSaHpiAnnunciatorRec.hpp \
                          oSaHpiCondition.cpp oSaHpiCondition.hpp \
                          oSaHpiCtrlDefaultMode.cpp oSaHpiCtrlDefaultMode.hpp \
                          oSaHpiCtrlRec.cpp oSaHpiCtrlRec.hpp \
                          oSaHpiCtrlRecAnalog.cpp oSaHpiCtrlRecAnalog.hpp \
                          oSaHpiCtrlRecDigital.cpp oSaHpiCtrlRecDigital.hpp \
                          oSaHpiCtrlRecDiscrete.cpp oSaHpiCtrlRecDiscrete.hpp \
                          oSaHpiCtrlRecOem.cpp oSaHpiCtrlRecOem.hpp \
                          oSaHpiCtrlRecStream.cpp oSaHpiCtrlRecStream.hpp \
                          oSaHpiCtrlRecText.cpp oSaHpiCtrlRecText.hpp \
                          oSaHpiCtrlState.cpp oSaHpiCtrlState.hpp \
                          oSaHpiCtrlStateOem.cpp oSaHpiCtrlStateOem.hpp \
                          oSaHpiCtrlStateStream.cpp oSaHpiCtrlStateStream.hpp \
                          oSaHpiCtrlStateText.cpp oSaHpiCtrlStateText.hpp \
                          oSaHpiDomainEvent.cpp oSaHpiDomainEvent.hpp \
                          oSaHpiDomainInfo.cpp oSaHpiDomainInfo.hpp \
                          oSaHpiDrtEntry.cpp oSaHpiDrtEntry.hpp \
                          oSaHpiEntity.cpp oSaHpiEntity.hpp \
                          oSaHpiEntityPath.cpp oSaHpiEntityPath.hpp \
                          oSaHpiEvent.cpp oSaHpiEvent.hpp \
                          oSaHpiEventLogEntry.cpp oSaHpiEventLogEntry.hpp \
                          oSaHpiEventLogInfo.cpp oSaHpiEventLogInfo.hpp \
                          oSaHpiHotSwapEvent.cpp oSaHpiHotSwapEvent.hpp \
                          oSaHpiHpiSwEvent.cpp oSaHpiHpiSwEvent.hpp \
                          oSaHpiIdrAreaHeader.cpp oSaHpiIdrAreaHeader.hpp \
                          oSaHpiIdrField.cpp oSaHpiIdrField.hpp \
                          oSaHpiIdrInfo.cpp oSaHpiIdrInfo.hpp \
                          oSaHpiInventoryRec.cpp oSaHpiInventoryRec.hpp \
                          oSaHpiName.cpp oSaHpiName.hpp \
                          oSaHpiOemEvent.cpp oSaHpiOemEvent.hpp \
                          oSaHpiRdr.cpp oSaHpiRdr.hpp \
                          oSaHpiResourceEvent.cpp oSaHpiResourceEvent.hpp \
                          oSaHpiResourceInfo.cpp oSaHpiResourceInfo.hpp \
                          oSaHpiRptEntry.cpp oSaHpiRptEntry.hpp \
                          oSaHpiSensorDataFormat.cpp oSaHpiSensorDataFormat.hpp \
                          oSaHpiSensorEnableChangeEvent.cpp
			  oSaHpiSensorEnableChangeEvent.hpp \
                          oSaHpiSensorEvent.cpp oSaHpiSensorEvent.hpp \
                          oSaHpiSensorRange.cpp oSaHpiSensorRange.hpp \
                          oSaHpiSensorReading.cpp oSaHpiSensorReading.hpp \
                          oSaHpiSensorRec.cpp oSaHpiSensorRec.hpp \
                          oSaHpiSensorThdDefn.cpp oSaHpiSensorThdDefn.hpp \
                          oSaHpiSensorThresholds.cpp oSaHpiSensorThresholds.hpp \
                          oSaHpiTextBuffer.cpp oSaHpiTextBuffer.hpp \
                          oSaHpiTypesEnums.cpp oSaHpiTypesEnums.hpp \
                          oSaHpiUserEvent.cpp oSaHpiUserEvent.hpp \
                          oSaHpiWatchdog.cpp oSaHpiWatchdog.hpp \
                          oSaHpiWatchdogEvent.cpp oSaHpiWatchdogEvent.hpp \
                          oSaHpiWatchdogRec.cpp oSaHpiWatchdogRec.hpp
libosahpi_la_LDFLAGS	= -version-info @HPI_LIB_VERSION@

