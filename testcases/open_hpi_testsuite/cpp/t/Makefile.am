# (C) Copyright IBM Corp 2005
# All rights reserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under a BSD style license.  See
# the Copying file included with the OpenHPI distribution for
# full licensing terms.

MOSTLYCLEANFILES 	= @TEST_CLEAN@

INCLUDES                = @OPENHPI_INCLUDES@ -I$(top_builddir)/cpp

MAINTAINERCLEANFILES 	= Makefile.in

TDEPLIB			= $(top_builddir)/cpp/libosahpi.la

TESTS = tSaHpiTextBuffer \
	tSaHpiEntityPath \
	tSaHpiSensorReading \
	tSaHpiSensorThresholds \
	tSaHpiSensorRec \
	tSaHpiCtrlRec \
	tSaHpiIdrField \
	tSaHpiIdrAreaHeader \
	tSaHpiIdrInfo \
	tSaHpiInventoryRec \
	tSaHpiWatchdog \
	tSaHpiWatchdogRec \
	tSaHpiAnnouncement \
	tSaHpiAnnunciatorRec \
	tSaHpiRdr \
	tSaHpiSensorEvent \
	tSaHpiSensorEnableChangeEvent \
	tSaHpiHotSwapEvent \
	tSaHpiWatchdogEvent \
	tSaHpiHpiSwEvent \
	tSaHpiOemEvent \
	tSaHpiUserEvent \
	tSaHpiEvent \
	tSaHpiResourceInfo \
	tSaHpiRptEntry \
	tSaHpiDomainInfo \
	tSaHpiDrtEntry \
	tSaHpiAlarm \
	tSaHpiEventLogInfo \
	tSaHpiEventLogEntry

check_PROGRAMS = $(TESTS)

tSaHpiTextBuffer_SOURCES = tSaHpiTextBuffer.cpp
tSaHpiTextBuffer_LDADD = $(TDEPLIB)
tSaHpiEntityPath_SOURCES = tSaHpiEntityPath.cpp
tSaHpiEntityPath_LDADD = $(TDEPLIB)
tSaHpiSensorReading_SOURCES = tSaHpiSensorReading.cpp
tSaHpiSensorReading_LDADD = $(TDEPLIB)
tSaHpiSensorThresholds_SOURCES = tSaHpiSensorThresholds.cpp
tSaHpiSensorThresholds_LDADD = $(TDEPLIB)
tSaHpiSensorRec_SOURCES = tSaHpiSensorRec.cpp
tSaHpiSensorRec_LDADD = $(TDEPLIB)
tSaHpiCtrlRec_SOURCES = tSaHpiCtrlRec.cpp
tSaHpiCtrlRec_LDADD = $(TDEPLIB)
tSaHpiIdrField_SOURCES = tSaHpiIdrField.cpp
tSaHpiIdrField_LDADD = $(TDEPLIB)
tSaHpiIdrAreaHeader_SOURCES = tSaHpiIdrAreaHeader.cpp
tSaHpiIdrAreaHeader_LDADD = $(TDEPLIB)
tSaHpiIdrInfo_SOURCES = tSaHpiIdrInfo.cpp
tSaHpiIdrInfo_LDADD = $(TDEPLIB)
tSaHpiInventoryRec_SOURCES = tSaHpiInventoryRec.cpp
tSaHpiInventoryRec_LDADD = $(TDEPLIB)
tSaHpiWatchdog_SOURCES = tSaHpiWatchdog.cpp
tSaHpiWatchdog_LDADD = $(TDEPLIB)
tSaHpiWatchdogRec_SOURCES = tSaHpiWatchdogRec.cpp
tSaHpiWatchdogRec_LDADD = $(TDEPLIB)
tSaHpiAnnouncement_SOURCES = tSaHpiAnnouncement.cpp
tSaHpiAnnouncement_LDADD = $(TDEPLIB)
tSaHpiAnnunciatorRec_SOURCES = tSaHpiAnnunciatorRec.cpp
tSaHpiAnnunciatorRec_LDADD = $(TDEPLIB)
tSaHpiRdr_SOURCES = tSaHpiRdr.cpp
tSaHpiRdr_LDADD = $(TDEPLIB)
tSaHpiSensorEvent_SOURCES = tSaHpiSensorEvent.cpp
tSaHpiSensorEvent_LDADD = $(TDEPLIB)
tSaHpiSensorEnableChangeEvent_SOURCES = tSaHpiSensorEnableChangeEvent.cpp
tSaHpiSensorEnableChangeEvent_LDADD = $(TDEPLIB)
tSaHpiHotSwapEvent_SOURCES = tSaHpiHotSwapEvent.cpp
tSaHpiHotSwapEvent_LDADD = $(TDEPLIB)
tSaHpiWatchdogEvent_SOURCES = tSaHpiWatchdogEvent.cpp
tSaHpiWatchdogEvent_LDADD = $(TDEPLIB)
tSaHpiHpiSwEvent_SOURCES = tSaHpiHpiSwEvent.cpp
tSaHpiHpiSwEvent_LDADD = $(TDEPLIB)
tSaHpiOemEvent_SOURCES = tSaHpiOemEvent.cpp
tSaHpiOemEvent_LDADD = $(TDEPLIB)
tSaHpiUserEvent_SOURCES = tSaHpiUserEvent.cpp
tSaHpiUserEvent_LDADD = $(TDEPLIB)
tSaHpiEvent_SOURCES = tSaHpiEvent.cpp
tSaHpiEvent_LDADD = $(TDEPLIB)
tSaHpiResourceInfo_SOURCES = tSaHpiResourceInfo.cpp
tSaHpiResourceInfo_LDADD = $(TDEPLIB)
tSaHpiRptEntry_SOURCES = tSaHpiRptEntry.cpp
tSaHpiRptEntry_LDADD = $(TDEPLIB)
tSaHpiDomainInfo_SOURCES = tSaHpiDomainInfo.cpp
tSaHpiDomainInfo_LDADD = $(TDEPLIB)
tSaHpiDrtEntry_SOURCES = tSaHpiDrtEntry.cpp
tSaHpiDrtEntry_LDADD = $(TDEPLIB)
tSaHpiAlarm_SOURCES = tSaHpiAlarm.cpp
tSaHpiAlarm_LDADD = $(TDEPLIB)
tSaHpiEventLogInfo_SOURCES = tSaHpiEventLogInfo.cpp
tSaHpiEventLogInfo_LDADD = $(TDEPLIB)
tSaHpiEventLogEntry_SOURCES = tSaHpiEventLogEntry.cpp
tSaHpiEventLogEntry_LDADD = $(TDEPLIB)

