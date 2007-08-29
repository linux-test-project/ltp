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


#ifndef Included_oSaHpiTypesEnums
#define Included_oSaHpiTypesEnums

extern "C"
{
#include <SaHpi.h>
}


class oSaHpiTypesEnums {
    public:
        // all the real methods in this class are static
        // so they can be used from any other class
        static SaHpiBoolT str2torf(const char *str);
        static const char * torf2str(SaHpiBoolT f);
        static SaHpiLanguageT str2language(const char *strtype);
        static const char * language2str(SaHpiLanguageT value);
        static SaHpiTextTypeT str2texttype(const char *type);
        static const char * texttype2str(SaHpiTextTypeT value);
        static SaHpiEntityTypeT str2entitytype(const char *strtype);
        static const char * entitytype2str(SaHpiEntityTypeT value);
        static SaHpiSensorReadingTypeT str2sensorreadingtype(const char *strtype);
        static const char * sensorreadingtype2str(SaHpiSensorReadingTypeT value);
        static SaHpiSensorUnitsT str2sensorunits(const char *strtype);
        static const char * sensorunits2str(SaHpiSensorUnitsT value);
        static SaHpiSensorModUnitUseT str2sensoruse(const char *strtype);
        static const char * sensoruse2str(SaHpiSensorModUnitUseT value);
        static SaHpiSensorThdMaskT str2sensorthdmask(const char *strtype);
        static const char * sensorthdmask2str(SaHpiSensorThdMaskT value);
        static SaHpiSensorEventCtrlT str2sensoreventctrl(const char *strtype);
        static const char * sensoreventctrl2str(SaHpiSensorEventCtrlT value);
        static SaHpiSensorTypeT str2sensortype(const char *strtype);
        static const char * sensortype2str(SaHpiSensorTypeT value);
        static SaHpiEventCategoryT str2eventcategory(const char *strtype);
        static const char * eventcategory2str(SaHpiEventCategoryT value);
        static SaHpiEventStateT str2eventstate(const char *strtype);
        static const char * eventstate2str(SaHpiEventStateT value);
        static SaHpiCtrlTypeT str2ctrltype(const char *strtype);
        static const char * ctrltype2str(SaHpiCtrlTypeT value);
        static SaHpiCtrlStateDigitalT str2ctrlstatedigital(const char *strtype);
        static const char * ctrlstatedigital2str(SaHpiCtrlStateDigitalT value);
        static SaHpiUint32T str2aggregatestatus(const char *strtype);
        static const char * aggregatestatus2str(SaHpiUint32T value);
        static SaHpiCtrlOutputTypeT str2ctrloutputtype(const char *strtype);
        static const char * ctrloutputtype2str(SaHpiCtrlOutputTypeT value);
        static SaHpiCtrlModeT str2ctrlmode(const char *strtype);
        static const char * ctrlmode2str(SaHpiCtrlModeT value);
        static SaHpiIdrAreaTypeT str2idrareatype(const char *strtype);
        static const char * idrareatype2str(SaHpiIdrAreaTypeT value);
        static SaHpiIdrFieldTypeT str2idrfieldtype(const char *strtype);
        static const char * idrfieldtype2str(SaHpiIdrFieldTypeT value);
        static SaHpiWatchdogActionT str2watchdogaction(const char *strtype);
        static const char * watchdogaction2str(SaHpiWatchdogActionT value);
        static SaHpiWatchdogActionEventT str2watchdogactionevent(const char *strtype);
        static const char * watchdogactionevent2str(SaHpiWatchdogActionEventT value);
        static SaHpiWatchdogPretimerInterruptT str2watchdogpretimerinterrupt(const char *strtype);
        static const char * watchdogpretimerinterrupt2str(SaHpiWatchdogPretimerInterruptT value);
        static SaHpiWatchdogTimerUseT str2watchdogtimeruse(const char *strtype);
        static const char * watchdogtimeruse2str(SaHpiWatchdogTimerUseT value);
        static SaHpiWatchdogExpFlagsT str2watchdogexpflags(const char *strtype);
        static const char * watchdogexpflags2str(SaHpiWatchdogExpFlagsT value);
        static SaHpiStatusCondTypeT str2statuscondtype(const char *strtype);
        static const char * statuscondtype2str(SaHpiStatusCondTypeT value);
        static SaHpiAnnunciatorModeT str2annunciatormode(const char *strtype);
        static const char * annunciatormode2str(SaHpiAnnunciatorModeT value);
        static SaHpiSeverityT str2severity(const char *strtype);
        static const char * severity2str(SaHpiSeverityT value);
        static SaHpiAnnunciatorTypeT str2annunciatortype(const char *strtype);
        static const char * annunciatortype2str(SaHpiAnnunciatorTypeT value);
        static SaHpiRdrTypeT str2rdrtype(const char *strtype);
        static const char * rdrtype2str(SaHpiRdrTypeT value);
        static SaHpiHsIndicatorStateT str2hsindicatorstate(const char *strtype);
        static const char * hsindicatorstate2str(SaHpiHsIndicatorStateT value);
        static SaHpiHsActionT str2hsaction(const char *strtype);
        static const char * hsaction2str(SaHpiHsActionT value);
        static SaHpiHsStateT str2hsstate(const char *strtype);
        static const char * hsstate2str(SaHpiHsStateT value);
        static SaHpiResourceEventTypeT str2resourceeventtype(const char *strtype);
        static const char * resourceeventtype2str(SaHpiResourceEventTypeT value);
        static SaHpiDomainEventTypeT str2domaineventtype(const char *strtype);
        static const char * domaineventtype2str(SaHpiDomainEventTypeT value);
        static SaHpiSensorOptionalDataT str2sensoroptionaldata(const char *strtype);
        static const char * sensoroptionaldata2str(SaHpiSensorOptionalDataT value);
        static SaHpiSwEventTypeT str2sweventtype(const char *strtype);
        static const char * sweventtype2str(SaHpiSwEventTypeT value);
        static SaHpiEventTypeT str2eventtype(const char *strtype);
        static const char * eventtype2str(SaHpiEventTypeT value);
        static SaHpiParmActionT str2parmaction(const char *strtype);
        static const char * parmaction2str(SaHpiParmActionT value);
        static SaHpiResetActionT str2resetaction(const char *strtype);
        static const char * resetaction2str(SaHpiResetActionT value);
        static SaHpiPowerStateT str2powerstate(const char *strtype);
        static const char * powerstate2str(SaHpiPowerStateT value);
        static SaHpiCapabilitiesT str2capabilities(const char *strtype);
        static const char * capabilities2str(SaHpiCapabilitiesT value);
        static SaHpiHsCapabilitiesT str2hscapabilities(const char *strtype);
        static const char * hscapabilities2str(SaHpiHsCapabilitiesT value);
        static SaHpiEventLogOverflowActionT str2eventlogoverflowaction(const char *strtype);
        static const char * eventlogoverflowaction2str(SaHpiEventLogOverflowActionT value);
        static SaHpiEventLogEntryIdT str2eventlogentryid(const char *strtype);
        static const char * eventlogentryid2str(SaHpiEventLogEntryIdT value);
};

#endif

