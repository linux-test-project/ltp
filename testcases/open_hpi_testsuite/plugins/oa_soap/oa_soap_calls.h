/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Bryan Sutula <Bryan.Sutula@hp.com>
 *     Raghavendra PG <raghavendra.pg@hp.com>
 *     Raghavendra MS <raghavendra.ms@hp.com>
 *     Anand S <S.Anand@hp.com>
 */

#ifndef _INC_OA_SOAP_CALLS_H_
#define _INC_OA_SOAP_CALLS_H_


/* Include files */
#include <oa_soap_callsupport.h>


/* Data types used to help us be more consistent with the WSDL description */
typedef unsigned char byte;


/* These define the SOAP commands used to talk to the OA */
#define GET_BLADE_INFO \
                        "<hpoa:getBladeInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getBladeInfo>\n"

#define SUBSCRIBE_FOR_EVENTS \
                        "<hpoa:subscribeForEvents>" \
                        "</hpoa:subscribeForEvents>\n"

#define UN_SUBSCRIBE_FOR_EVENTS \
                        "<hpoa:unSubscribeForEvents>" \
                        "<hpoa:pid>%d</hpoa:pid>" \
                        "</hpoa:unSubscribeForEvents>\n"

#define GET_EVENT \
                        "<hpoa:getEvent>" \
                        "<hpoa:pid>%d</hpoa:pid>" \
                        "<hpoa:waitTilEventHappens>" \
                                "%d</hpoa:waitTilEventHappens>" \
                        "<hpoa:lcdEvents>%d</hpoa:lcdEvents>" \
                        "</hpoa:getEvent>\n"

#define GET_ALL_EVENTS \
                        "<hpoa:getAllEvents>" \
                        "<hpoa:pid>%d</hpoa:pid>" \
                        "<hpoa:waitTilEventHappens>" \
                                "%d</hpoa:waitTilEventHappens>" \
                        "<hpoa:lcdEvents>%d</hpoa:lcdEvents>" \
                        "</hpoa:getAllEvents>\n"

#define SET_BLADE_POWER \
                        "<hpoa:setBladePower>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "<hpoa:power>%s</hpoa:power>" \
                        "</hpoa:setBladePower>\n"

#define SET_INTERCONNECT_TRAY_POWER \
                        "<hpoa:setInterconnectTrayPower>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "<hpoa:on>%d</hpoa:on>" \
                        "</hpoa:setInterconnectTrayPower>\n"

#define RESET_INTERCONNECT_TRAY \
                        "<hpoa:resetInterconnectTray>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:resetInterconnectTray>\n"

#define GET_ENCLOSURE_INFO \
                        "<hpoa:getEnclosureInfo>" \
                        "</hpoa:getEnclosureInfo>\n"

#define GET_OA_STATUS \
                        "<hpoa:getOaStatus>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getOaStatus>\n"

#define GET_OA_INFO \
                        "<hpoa:getOaInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getOaInfo>\n"

#define GET_INTERCONNECT_TRAY_STATUS \
                        "<hpoa:getInterconnectTrayStatus>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getInterconnectTrayStatus>\n"

#define GET_INTERCONNECT_TRAY_INFO \
                        "<hpoa:getInterconnectTrayInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getInterconnectTrayInfo>\n"

#define GET_FAN_INFO \
                        "<hpoa:getFanInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getFanInfo>\n"

#define GET_POWER_SUBSYSTEM_INFO \
                        "<hpoa:getPowerSubsystemInfo>" \
                        "</hpoa:getPowerSubsystemInfo>\n"

#define GET_POWER_SUPPLY_INFO \
                        "<hpoa:getPowerSupplyInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getPowerSupplyInfo>\n"

#define GET_OA_NETWORK_INFO \
                        "<hpoa:getOaNetworkInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getOaNetworkInfo>\n"

#define GET_BLADE_STATUS \
                        "<hpoa:getBladeStatus>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getBladeStatus>\n"

#define GET_THERMAL_INFO \
                        "<hpoa:getThermalInfo>" \
                        "<hpoa:sensorType>%s</hpoa:sensorType>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getThermalInfo>\n"

#define GET_USER_INFO \
                        "<hpoa:getUserInfo>" \
                        "<hpoa:username>%s</hpoa:username>" \
                        "</hpoa:getUserInfo>\n"

#define IS_VALID_SESSION \
                        "<hpoa:isValidSession>" \
                        "</hpoa:isValidSession>\n"

#define GET_RACK_TOPOLOGY2 \
                        "<hpoa:getRackTopology2>" \
                        "</hpoa:getRackTopology2>\n"

#define GET_BLADE_MP_INFO \
                        "<hpoa:getBladeMpInfo>" \
                        "<hpoa:bayNumber>%d</hpoa:bayNumber>" \
                        "</hpoa:getBladeMpInfo>\n"

/* Enumerated types used for specific SOAP commands */
OA_SOAP_ENUM(hpoa_boolean,
        HPOA_FALSE,
        HPOA_TRUE)

OA_SOAP_ENUM(presence,
        PRESENCE_NO_OP,
        PRESENCE_UNKNOWN,
        ABSENT,
        PRESENT,
        SUBSUMED)

OA_SOAP_ENUM(bladeType,
        BLADE_TYPE_NO_OP,
        BLADE_TYPE_UNKNOWN,
        BLADE_TYPE_SERVER,
        BLADE_TYPE_STORAGE,
        BLADE_TYPE_WORKSTATION,
        BLADE_TYPE_IO)

OA_SOAP_ENUM(power,
        POWER_NO_OP,
        POWER_UNKNOWN,
        POWER_ON,
        POWER_OFF,
        POWER_STAGED_OFF,
        POWER_REBOOT)

OA_SOAP_ENUM(powerState,
        PS_NO_OP,
        PS_UNKNOWN,
        PS_OFF,
        PS_LOW,
        PS_AUTOMATIC,
        PS_MAXIMUM)

OA_SOAP_ENUM(shutdown,
        SHUTDOWN_NO_OP,
        SHUTDOWN_UNKNOWN,
        SHUTDOWN_OK,
        SHUTDOWN_SHUTDOWN,
        SHUTDOWN_THERMAL,
        SHUTDOWN_FAN,
        SHUTDOWN_RESTART)

OA_SOAP_ENUM(uidStatus,
        UID_NO_OP,
        UID_UNKNOWN,
        UID_ON,
        UID_OFF,
        UID_BLINK,
        UID_DEMONSTRATION)

/* Thank-you, OA team!  One of the enum names here includes a '-' character,
 * which is illegal in an enum name.  We need to keep the enum name legal in
 * C, yet parse a '-' character in the ASCII representations of opStatus.
 * If any changes are made to these values, make sure to keep both version
 * the same!
 */
enum opStatus {
        OP_STATUS_UNKNOWN,
        OP_STATUS_OTHER,
        OP_STATUS_OK,
        OP_STATUS_DEGRADED,
        OP_STATUS_STRESSED,
        OP_STATUS_PREDICTIVE_FAILURE,
        OP_STATUS_ERROR,
        OP_STATUS_NON_RECOVERABLE_ERROR,
        OP_STATUS_STARTING,
        OP_STATUS_STOPPING,
        OP_STATUS_STOPPED,
        OP_STATUS_IN_SERVICE,
        OP_STATUS_NO_CONTACT,
        OP_STATUS_LOST_COMMUNICATION,
        OP_STATUS_ABORTED,
        OP_STATUS_DORMANT,
        OP_STATUS_SUPPORTING_ENTITY_IN_ERROR,
        OP_STATUS_COMPLETED,
        OP_STATUS_POWER_MODE,
        OP_STATUS_DMTF_RESERVED,
        OP_STATUS_VENDER_RESERVED};
OA_SOAP_ENUM_STRING(opStatus,
        OP_STATUS_UNKNOWN,
        OP_STATUS_OTHER,
        OP_STATUS_OK,
        OP_STATUS_DEGRADED,
        OP_STATUS_STRESSED,
        OP_STATUS_PREDICTIVE_FAILURE,
        OP_STATUS_ERROR,
        OP_STATUS_NON-RECOVERABLE_ERROR,
        OP_STATUS_STARTING,
        OP_STATUS_STOPPING,
        OP_STATUS_STOPPED,
        OP_STATUS_IN_SERVICE,
        OP_STATUS_NO_CONTACT,
        OP_STATUS_LOST_COMMUNICATION,
        OP_STATUS_ABORTED,
        OP_STATUS_DORMANT,
        OP_STATUS_SUPPORTING_ENTITY_IN_ERROR,
        OP_STATUS_COMPLETED,
        OP_STATUS_POWER_MODE,
        OP_STATUS_DMTF_RESERVED,
        OP_STATUS_VENDER_RESERVED)

OA_SOAP_ENUM(sensorStatus,
        SENSOR_STATUS_NO_OP,
        SENSOR_STATUS_UNKNOWN,
        SENSOR_STATUS_OK,
        SENSOR_STATUS_WARM,
        SENSOR_STATUS_CAUTION,
        SENSOR_STATUS_CRITICAL)

OA_SOAP_ENUM(diagnosticStatus,
        NOT_RELEVANT,
        DIAGNOSTIC_CHECK_NOT_PERFORMED,
        NO_ERROR,
        ERROR)

OA_SOAP_ENUM(oaRole,
        OA_ABSENT,
        STANDBY,
        TRANSITION,
        ACTIVE)

OA_SOAP_ENUM(wizardStatus,
        WIZARD_NOT_COMPLETED,
        LCD_WIZARD_COMPLETE,
        WIZARD_SETUP_COMPLETE)

OA_SOAP_ENUM(portMapStatus,
        UNKNOWN,
        OK,
        MISMATCH)

OA_SOAP_ENUM(bladeSizeType,
        BLADE_SIZE_TYPE_MT,
        BLADE_SIZE_TYPE_1X1,
        BLADE_SIZE_TYPE_1X2)

OA_SOAP_ENUM(bladeMezzSlotType,
        MEZZ_SLOT_TYPE_MT,
        MEZZ_SLOT_TYPE_ONE,
        MEZZ_SLOT_TYPE_TWO,
        MEZZ_SLOT_TYPE_FIXED)

OA_SOAP_ENUM(bladeMezzDevType,
        MEZZ_DEV_TYPE_MT,
        MEZZ_DEV_TYPE_ONE,
        MEZZ_DEV_TYPE_TWO,
        MEZZ_DEV_TYPE_FIXED)

OA_SOAP_ENUM(bladeMezzDevStatus,
        MEZZ_DEV_STATUS_UNKNOWN,
        MEZZ_DEV_STATUS_OK,
        MEZZ_DEV_STATUS_MISMATCH)

OA_SOAP_ENUM(fabricType,
        FABRIC_TYPE_MT,
        FABRIC_TYPE_ETH,
        FABRIC_TYPE_FIB,
        FABRIC_TYPE_10GETH,
        FABRIC_TYPE_IFB,
        FABRIC_TYPE_PCI,
        FABRIC_TYPE_SAS,
        FABRIC_TYPE_MAX)

OA_SOAP_ENUM(fabricStatus,
        FABRIC_STATUS_UNKNOWN,
        FABRIC_STATUS_OK,
        FABRIC_STATUS_MISMATCH)

OA_SOAP_ENUM(interconnectTrayType,
        INTERCONNECT_TRAY_TYPE_NO_CONNECTION,
        INTERCONNECT_TRAY_TYPE_NIC,
        INTERCONNECT_TRAY_TYPE_FC,
        INTERCONNECT_TRAY_TYPE_10GETH,
        INTERCONNECT_TRAY_TYPE_IB,
        INTERCONNECT_TRAY_TYPE_PCIE,
        INTERCONNECT_TRAY_TYPE_SAS,
        INTERCONNECT_TRAY_TYPE_MAX)

OA_SOAP_ENUM(interconnectTraySizeType,
        INTERCONNECT_TRAY_SIZE_TYPE_MT,
        INTERCONNECT_TRAY_SIZE_TYPE_1X1,
        INTERCONNECT_TRAY_SIZE_TYPE_2x1)

OA_SOAP_ENUM(interconnectTrayPassThroughEnabled,
        INTERCONNECT_TRAY_PASSTHROUGH_UNKNOWN,
        INTERCONNECT_TRAY_PASSTHROUGH_DISABLED,
        INTERCONNECT_TRAY_PASSTHROUGH_ENABLED)

OA_SOAP_ENUM(interconnectTrayPortStatus,
        INTERCONNECT_TRAY_PORT_STATUS_UNKNOWN,
        INTERCONNECT_TRAY_PORT_STATUS_OK,
        INTERCONNECT_TRAY_PORT_STATUS_MISMATCH)

OA_SOAP_ENUM(interconnectTrayPortEnabled,
        INTERCONNECT_TRAY_PORT_ENABLED_UNKNOWN,
        INTERCONNECT_TRAY_PORT_DISABLED,
        INTERCONNECT_TRAY_PORT_ENABLED)

OA_SOAP_ENUM(interconnectTrayPortUidStatus,
        INTERCONNECT_TRAY_UID_UNKNOWN,
        INTERCONNECT_TRAY_UID_OFF,
        INTERCONNECT_TRAY_UID_ON)

OA_SOAP_ENUM(interconnectTrayPortLinkLedStatus,
        INTERCONNECT_TRAY_LINK_LED_UNKNOWN,
        INTERCONNECT_TRAY_LINK_LED_OFF,
        INTERCONNECT_TRAY_LINK_LED_ON)

OA_SOAP_ENUM(powerSystemType,
        SUBSYSTEM_NO_OP,
        SUBSYSTEM_UNKNOWN,
        INTERNAL_AC,
        INTERNAL_DC,
        EXTERNAL_DC)

OA_SOAP_ENUM(redundancy,
        REDUNDANCY_NO_OP,
        REDUNDANCY_UNKNOWN,
        NOT_REDUNDANT,
        REDUNDANT)

OA_SOAP_ENUM(powerRedundancy,
        REDUNDANT_UNKNOWN,
        NON_REDUNDANT,
        AC_REDUNDANT,
        POWER_SUPPLY_REDUNDANT,
        AC_REDUNDANT_WITH_POWER_CEILING,
        POWER_SUPPLY_REDUNDANT_WITH_POWER_CEILING,
        NON_REDUNDANT_WITH_POWER_CEILING)

#define SENSOR_TYPE_LENGTH      25      /* Max length of these enums + 1 */
OA_SOAP_ENUM(sensorType,
        SENSOR_TYPE_BLADE,
        SENSOR_TYPE_INTERCONNECT,
        SENSOR_TYPE_OA,
        SENSOR_TYPE_ENC)

OA_SOAP_ENUM(userAcl,
        ADMINISTRATOR,
        OPERATOR,
        USER,
        ANONYMOUS)

OA_SOAP_ENUM(lcdButton,
        LCD_OK,
        LCD_UP,
        LCD_DOWN,
        LCD_RIGHT,
        LCD_LEFT,
        LCD_USERNOTES)

OA_SOAP_ENUM(lcdButtonState,
        CLICKED,
        PRESSED,
        RELEASED)

OA_SOAP_ENUM(lcdSetupHealth,
        LCD_SETUP_HEALTH_UNKNOWN,
        LCD_SETUP_HEALTH_OK,
        LCD_SETUP_HEALTH_INFORMATIONAL,
        LCD_SETUP_HEALTH_DEGRADED,
        LCD_SETUP_HEALTH_FAILED)

OA_SOAP_ENUM(lcdChatMessageType,
        STATEMENT,
        QUESTION,
        ANSWER,
        QUESTION_DISMISSED)

OA_SOAP_ENUM(hpSimTrustMode,
        HPSIM_DISABLED,
        TRUST_BY_NAME,
        TRUST_BY_CERTIFICATE,
        TRUST_ALL)

OA_SOAP_ENUM(iplDevice,
        IPL_NO_OP,
        CD,
        FLOPPY,
        HDD,
        USB,
        PXE_NIC1,
        PXE_NIC2,
        PXE_NIC3,
        PXE_NIC4)

OA_SOAP_ENUM(oneTimeBootDevice,
        ONE_TIME_BOOT_NO_CHANGE,
        ONE_TIME_BOOT_FLOPPY,
        ONE_TIME_BOOT_CD,
        ONE_TIME_BOOT_HARD_DRIVE,
        ONE_TIME_BOOT_TAPE)

OA_SOAP_ENUM(oneTimeBootAgent,
        NORMAL_BOOT_OS,
        SYS_PART,
        QUICK_DIAGS,
        RBSU,
        PXE)

OA_SOAP_ENUM(powerReductionState,
        SPRS_NO_OP,
        SPRS_UNKNOWN,
        SPRS_FIRED,
        SPRS_RESTORED)

OA_SOAP_ENUM(powerReductionArmedState,
        SPRAS_NO_OP,
        SPRAS_UNKNOWN,
        SPRAS_DISARMED,
        SPRAS_ARMED)

OA_SOAP_ENUM(virtualMediaSupport,
        VM_SUPPORT_UNKNOWN,
        VM_DEV_ABSENT,
        VM_BAY_SUBSUMED,
        VM_SUPPORTED,
        VM_NOT_SUPPORTED,
        VM_FIRMWARE_UPDATE_NEEDED)

OA_SOAP_ENUM(virtualMediaDeviceStatus,
        VM_DEV_STATUS_UNKNOWN,
        VM_DEV_STATUS_DISCONNECTED,
        VM_DEV_STATUS_CONNECTED,
        VM_DEV_STATUS_DISCONNECTING,
        VM_DEV_STATUS_CONNECTING)

#define POWER_CONTROL_LENGTH    16      /* Max length of these enums + 1 */
OA_SOAP_ENUM(powerControl,
        MOMENTARY_PRESS,
        PRESS_AND_HOLD,
        COLD_BOOT,
        RESET)

OA_SOAP_ENUM(eventType,
        EVENT_HEARTBEAT,
        EVENT_ENC_STATUS,
        EVENT_ENC_UID,
        EVENT_ENC_SHUTDOWN,
        EVENT_ENC_INFO,
        EVENT_ENC_NAMES,
        EVENT_USER_PERMISSION,
        EVENT_ADMIN_RIGHTS_CHANGED,
        EVENT_ENC_SHUTDOWN_PENDING,
        EVENT_ENC_TOPOLOGY,
        EVENT_FAN_STATUS,
        EVENT_FAN_INSERTED,
        EVENT_FAN_REMOVED,
        EVENT_FAN_GROUP_STATUS,
        EVENT_THERMAL_STATUS,
        EVENT_COOLING_STATUS,
        EVENT_FAN_ZONE_STATUS,
        EVENT_PS_STATUS,
        EVENT_PS_INSERTED,
        EVENT_PS_REMOVED,
        EVENT_PS_REDUNDANT,
        EVENT_PS_OVERLOAD,
        EVENT_AC_FAILURE,
        EVENT_PS_INFO,
        EVENT_PS_SUBSYSTEM_STATUS,
        EVENT_SERVER_POWER_REDUCTION_STATUS,
        EVENT_INTERCONNECT_STATUS,
        EVENT_INTERCONNECT_RESET,
        EVENT_INTERCONNECT_UID,
        EVENT_INTERCONNECT_INSERTED,
        EVENT_INTERCONNECT_REMOVED,
        EVENT_INTERCONNECT_INFO,
        EVENT_INTERCONNECT_HEALTH_LED,
        EVENT_INTERCONNECT_THERMAL,
        EVENT_INTERCONNECT_CPUFAULT,
        EVENT_INTERCONNECT_POWER,
        EVENT_INTERCONNECT_PORTMAP,
        EVENT_BLADE_PORTMAP,
        EVENT_INTERCONNECT_VENDOR_BLOCK,
        EVENT_INTERCONNECT_HEALTH_STATE,
        EVENT_DEMO_MODE,
        EVENT_BLADE_STATUS,
        EVENT_BLADE_INSERTED,
        EVENT_BLADE_REMOVED,
        EVENT_BLADE_POWER_STATE,
        EVENT_BLADE_POWER_MGMT,
        EVENT_BLADE_UID,
        EVENT_BLADE_SHUTDOWN,
        EVENT_BLADE_FAULT,
        EVENT_BLADE_THERMAL,
        EVENT_BLADE_INFO,
        EVENT_BLADE_MP_INFO,
        EVENT_ILO_READY,
        EVENT_LCD_BUTTON,
        EVENT_KEYING_ERROR,
        EVENT_ILO_HAS_IPADDRESS,
        EVENT_POWER_INFO,
        EVENT_LCD_STATUS,
        EVENT_LCD_INFO,
        EVENT_REDUNDANCY,
        EVENT_ILO_DEAD,
        EVENT_RACK_SERVICE_STARTED,
        EVENT_LCD_SCREEN_REFRESH,
        EVENT_ILO_ALIVE,
        EVENT_PERSONALITY_CHECK,
        EVENT_BLADE_POST_COMPLETE,
        EVENT_BLADE_SIGNATURE_CHANGED,
        EVENT_BLADE_PERSONALITY_CHANGED,
        EVENT_BLADE_TOO_LOW_POWER,
        EVENT_VIRTUAL_MEDIA_STATUS,
        EVENT_MEDIA_DRIVE_INSERTED,
        EVENT_MEDIA_DRIVE_REMOVED,
        EVENT_MEDIA_INSERTED,
        EVENT_MEDIA_REMOVED,
        EVENT_OA_NAMES,
        EVENT_OA_STATUS,
        EVENT_OA_UID,
        EVENT_OA_INSERTED,
        EVENT_OA_REMOVED,
        EVENT_OA_INFO,
        EVENT_OA_FAILOVER,
        EVENT_OA_TRANSITION_COMPLETE,
        EVENT_OA_VCM,
        EVENT_NETWORK_INFO_CHANGED,
        EVENT_SNMP_INFO_CHANGED,
        EVENT_SYSLOG_CLEARED,
        EVENT_SESSION_CLEARED,
        EVENT_TIME_CHANGE,
        EVENT_SESSION_STARTED,
        EVENT_BLADE_CONNECT,
        EVENT_BLADE_DISCONNECT,
        EVENT_SWITCH_CONNECT,
        EVENT_SWITCH_DISCONNECT,
        EVENT_BLADE_CLEARED,
        EVENT_SWITCH_CLEARED,
        EVENT_ALERTMAIL_INFO_CHANGED,
        EVENT_LDAP_INFO_CHANGED,
        EVENT_EBIPA_INFO_CHANGED,
        EVENT_HPSIM_TRUST_MODE_CHANGED,
        EVENT_HPSIM_CERTIFICATE_ADDED,
        EVENT_HPSIM_CERTIFICATE_REMOVED,
        EVENT_USER_INFO_CHANGED,
        EVENT_BAY_CHANGED,
        EVENT_GROUP_CHANGED,
        EVENT_OA_REBOOT,
        EVENT_OA_LOGOFF_REQUEST,
        EVENT_USER_ADDED,
        EVENT_USER_DELETED,
        EVENT_USER_ENABLED,
        EVENT_USER_DISABLED,
        EVENT_GROUP_ADDED,
        EVENT_GROUP_DELETED,
        EVENT_LDAPGROUP_ADDED,
        EVENT_LDAPGROUP_DELETED,
        EVENT_LDAPGROUP_ADMIN_RIGHTS_CHANGED,
        EVENT_LDAPGROUP_INFO_CHANGED,
        EVENT_LDAPGROUP_PERMISSION,
        EVENT_LCDPIN,
        EVENT_LCD_USER_NOTES_CHANGED,
        EVENT_LCD_BUTTONS_LOCKED,
        EVENT_LCD_SCREEN_CHAT_REQUESTED,
        EVENT_LCD_SCREEN_CHAT_WITHDRAWN,
        EVENT_LCD_SCREEN_CHAT_ANSWERED,
        EVENT_LCD_USER_NOTES_IMAGE_CHANGED,
        EVENT_ENC_WIZARD_STATUS,
        EVENT_SSHKEYS_INSTALLED,
        EVENT_SSHKEYS_CLEARED,
        EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_ADDED,
        EVENT_LDAP_DIRECTORY_SERVER_CERTIFICATE_REMOVED,
        EVENT_BLADE_BOOT_CONFIG,
        EVENT_OA_NETWORK_CONFIG_CHANGED,
        EVENT_HPSIM_XENAME_ADDED,
        EVENT_HPSIM_XENAME_REMOVED,
        EVENT_FLASH_PENDING,
        EVENT_FLASH_STARTED,
        EVENT_FLASH_PROGRESS,
        EVENT_FLASH_COMPLETE,
        EVENT_STANDBY_FLASH_STARTED,
        EVENT_STANDBY_FLASH_PROGRESS,
        EVENT_STANDBY_FLASH_COMPLETE,
        EVENT_STANDBY_FLASH_BOOTING,
        EVENT_STANDBY_FLASH_BOOTED,
        EVENT_STANDBY_FLASH_FAILED,
        EVENT_FLASHSYNC_BUILD,
        EVENT_FLASHSYNC_BUILDDONE,
        EVENT_FLASHSYNC_FAILED,
        EVENT_FLASHSYNC_STANDBY_BUILD,
        EVENT_FLASHSYNC_STANDBY_BUILDDONE,
        EVENT_FLASHSYNC_STANDBY_FAILED,
        EVENT_NONILO_EBIPA,
        EVENT_FACTORY_RESET,
        EVENT_BLADE_INSERT_COMPLETED,
        EVENT_EBIPA_INFO_CHANGED_EX,
        EVENT_BLADE_ESI_CHANGED,
        EVENT_ENC_TOPOLOGY_2,
        EVENT_TFA_CA_CERT_ADDED,
        EVENT_TFA_CA_CERT_REMOVED,
        EVENT_USER_CERT_ADDED,
        EVENT_USER_CERT_REMOVED,
        EVENT_PW_SETTINGS_CHANGED,
        EVENT_SYSLOG_SETTINGS_CHANGED,
        EVENT_POWERDELAY_SETTINGS_CHANGED,
        EVENT_USB_OA_FW_FILES,
        EVENT_USB_OA_CONFIG_SCRIPTS,
        EVENT_MEDIA_DRIVE_INSERTED2,
        EVENT_MEDIA_DRIVE_REMOVED2,
        EVENT_MEDIA_INSERTED2,
        EVENT_MEDIA_REMOVED2)

/* This is not part of the SOAP response data from the OA, but is useful
 * for identifying the type of data that comes back from getAllEvents().
 */
OA_SOAP_ENUM(enum_eventInfo,
        SYSLOG,
        RACKTOPOLOGY,
        ENCLOSURESTATUS,
        ENCLOSUREINFO,
        OASTATUS,
        OAINFO,
        BLADEINFO,
        BLADEMPINFO,
        BLADESTATUS,
        BLADEPORTMAP,
        FANINFO,
        INTERCONNECTTRAYSTATUS,
        INTERCONNECTTRAYINFO,
        INTERCONNECTTRAYPORTMAP,
        POWERSUPPLYINFO,
        POWERSUPPLYSTATUS,
        POWERSUBSYSTEMINFO,
        POWERCONFIGINFO,
        THERMALINFO,
        USERINFOARRAY,
        USERINFO,
        LDAPINFO,
        LDAPGROUPINFO,
        SNMPINFO,
        ENCLOSURENETWORKINFO,
        OANETWORKINFO,
        ENCLOSURETIME,
        ALERTMAILINFO,
        PASSWORDSETTINGS,
        EBIPAINFO,
        LCDCHATMESSAGE,
        LCDUSERNOTES,
        LCDBUTTONEVENT,
        LCDSTATUS,
        LCDINFO,
        HPSIMINFO,
        THERMALSUBSYSTEMINFO,
        BLADEBOOTINFO,
        OAVCMMODE,
        POWERREDUCTIONSTATUS,
        VIRTUALMEDIASTATUS,
        OAMEDIADEVICE,
        FANZONE,
        EBIPAINFOEX,
        CACERTSINFO,
        RACKTOPOLOGY2,
        USERCERTIFICATEINFO,
        SYSLOGSETTINGS,
        POWERDELAYSETTINGS,
        USBMEDIAFIRMWAREIMAGES,
        CONFIGSCRIPTS,
        NUMVALUE,
        STRING,
        MESSAGE,
        NOPAYLOAD)

OA_SOAP_ENUM(enum_usbMode,
        USB_KVM_ENABLED,
        USB_DVD_ENABLED)

OA_SOAP_ENUM(enum_networkProtocol,
        NET_PROTO_SNMP,
        NET_PROTO_SSH,
        NET_PROTO_TELNET,
        NET_PROTO_HTTP,
        NET_PROTO_NTP,
        NET_PROTO_IPSECURITY,
        NET_PROTO_ALERTMAIL,
        NET_PROTO_EBIPA_SVB,
        NET_PROTO_EBIPA_SWM,
        NET_PROTO_XMLREPLY,
        NET_PROTO_DYNDNS,
        NET_PROTO_LLF,
        NET_PROTO_IPSWAP)

OA_SOAP_ENUM(enum_nicSpeed,
        NIC_SPEED_10,
        NIC_SPEED_100,
        NIC_SPEED_1000,
        NIC_SPEED_10000)

OA_SOAP_ENUM(enum_nicDuplex,
        NIC_DUPLEX_HALF,
        NIC_DUPLEX_FULL)

OA_SOAP_ENUM(enum_fileType,
        FIRMWARE_IMAGE,
        LCD_IMAGE,
        CONFIG_SCRIPT,
        SSH_KEYS_FILE,
        SSL_CERTIFICATE,
        LDAP_DIRECTORY_SERVER_CERTIFICATE,
        HPSIM_CERTIFICATE,
        FIRMWARE_INTERNAL_IMAGE,
        PROLIANT_MP_IMAGE)

/* Structures that return information from OA SOAP calls */
struct bladeCpuInfo
{
        char *cpuType;
        int cpuSpeed;
};

struct bladeNicInfo
{
        char *port;
        char *macAddress;
};

#if 0                                   /* TODO: This doesn't work yet */
struct extraData
{
        char *item;
        char *name;
        char *mixed;
};
#endif

struct bladeInfo
{
        byte bayNumber;
        enum presence presence;
        enum bladeType bladeType;
        byte width;
        byte height;
        char *name;
        char *manufacturer;
        char *partNumber;
        char *sparePartNumber;
        char *serialNumber;
        char *serverName;
        char *uuid;
        char *rbsuOsName;
        char *assetTag;
        char *romVersion;
        byte numberOfCpus;
        xmlNode *cpus;
        int memory;
        byte numberOfNics;
        xmlNode *nics;
        short mmHeight;
        short mmWidth;
        short mmDepth;
        int deviceId;
        int productId;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

/* Structures that supply information to OA SOAP calls */
struct getBladeInfo
{
        int bayNumber;
};

struct diagnosticChecks
{
        enum diagnosticStatus internalDataError;
        enum diagnosticStatus managementProcessorError;
        enum diagnosticStatus thermalWarning;
        enum diagnosticStatus thermalDanger;
        enum diagnosticStatus ioConfigurationError;
        enum diagnosticStatus devicePowerRequestError;
        enum diagnosticStatus insufficientCooling;
        enum diagnosticStatus deviceLocationError;
        enum diagnosticStatus deviceFailure;
        enum diagnosticStatus deviceDegraded;
        enum diagnosticStatus acFailure;
        enum diagnosticStatus i2cBuses;
        enum diagnosticStatus redundancy;
};

struct diagnosticData
{
        enum diagnosticStatus item;
        char *name;
};

struct bladeStatus
{
        byte bayNumber;
        enum presence presence;
        enum opStatus operationalStatus;
        enum sensorStatus thermal;
        enum power powered;
        enum powerState powerState;
        enum shutdown shutdown;
        enum uidStatus uid;
        int powerConsumed;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getBladeStatus
{
        int bayNumber;
};

struct syslog
{
        byte bayNumber;
        int syslogStrlen;
        char *logContents;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct encLink
{
        byte enclosureNumber;
        char *oaName;
        char *uuid;
        char *rackName;
        char *enclosureName;
        char *url;
        enum hpoa_boolean local;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct rackTopology
{
        char *ruid;
        xmlNode *enclosures;            /* Items are struct encLink */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct enclosureStatus
{
        enum opStatus operationalStatus;
        enum uidStatus uid;
        enum wizardStatus wizardStatus;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct enclosureInfo
{
        char *rackName;
        char *enclosureName;
        char *hwVersion;
        byte bladeBays;
        byte fanBays;
        byte powerSupplyBays;
        byte thermalSensors;
        byte interconnectTrayBays;
        byte oaBays;
        char *name;
        char *partNumber;
        char *serialNumber;
        char *uuid;
        char *assetTag;
        char *manufacturer;
        char *chassisSparePartNumber;
        char *interposerManufacturer;
        char *interposerName;
        char *interposerPartNumber;
        char *interposerSerialNumber;
        char *pduType;
        short mmHeight;
        short mmWidth;
        short mmDepth;
        char *pduPartNumber;
        char *pduSparePartNumber;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct oaStatus
{
        byte bayNumber;
        char *oaName;
        enum oaRole oaRole;
        enum opStatus operationalStatus;
        enum uidStatus uid;
        byte restartCause;
        enum hpoa_boolean oaRedundancy;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getOaStatus
{
        int bayNumber;
};

struct oaInfo
{
        byte bayNumber;
        enum hpoa_boolean youAreHere;
        char *name;
        char *partNumber;
        char *sparePartNumber;
        char *serialNumber;
        char *uuid;
        char *assetTag;
        char *manufacturer;
        char *hwVersion;
        char *fwVersion;
        short mmHeight;
        short mmWidth;
        short mmDepth;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getOaInfo
{
        int bayNumber;
};

struct getBladeMpInfo
{
        int bayNumber;
};

struct bladeMpInfo
{
        byte bayNumber;
        char *ipAddress;
        char *macAddress;
        char *dnsName;
        char *modelName;
        char *fwVersion;
        char *remoteConsoleUrl;
        char *webUrl;
        char *ircUrl;
        char *loginUrl;
        char *ircFullUrl;
        char *remoteSerialUrl;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeMezzSlotPort
{
        byte slotNumber;
        byte interconnectTrayBayNumber;
        byte interconnectTrayPortNumber;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeMezzSlotInfo
{
        enum bladeMezzSlotType type;
        int sizeslot;
        struct bladeMezzSlotPort slot;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeMezzDevPort
{
        byte portNumber;
        char *wwpn;
        enum fabricType fabric;
        enum fabricStatus status;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeMezzDevInfo
{
        char *name;
        enum bladeMezzDevType type;
        enum bladeMezzDevStatus status;
        int sizeport;
        struct bladeMezzDevPort port;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeMezzInfo
{
        byte mezzNumber;
        struct bladeMezzSlotInfo mezzSlots;
        struct bladeMezzDevInfo mezzDevices;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladePortMap
{
        byte bladeBayNumber;
        enum portMapStatus status;
        enum bladeSizeType bladeSizeType;
        byte numberOfMezzes;
        int sizemezz;
        struct bladeMezzInfo mezz;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct fanInfo
{
        byte bayNumber;
        enum presence presence;
        char *name;
        char *partNumber;
        char *sparePartNumber;
        char *serialNumber;
        int powerConsumed;
        int fanSpeed;
        int maxFanSpeed;
        int lowLimitFanSpeed;
        enum opStatus operationalStatus;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getFanInfo
{
        int bayNumber;
};

struct portEnabled
{
        byte portNumber;
        enum hpoa_boolean enabled;
};

struct interconnectTrayStatus
{
        byte bayNumber;
        enum opStatus operationalStatus;
        enum presence presence;
        enum sensorStatus thermal;
        enum hpoa_boolean cpuFault;
        enum hpoa_boolean healthLed;
        enum uidStatus uid;
        enum power powered;
        xmlNode *ports;                 /* Items are struct portEnabled */
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getInterconnectTrayStatus
{
        int bayNumber;
};

struct interconnectTrayInfo
{
        byte bayNumber;
        enum interconnectTrayType interconnectTrayType;
        enum hpoa_boolean passThroughSupport;
        enum hpoa_boolean portDisableSupport;
        enum hpoa_boolean temperatureSensorSupport;
        byte width;
        char *manufacturer;
        char *name;
        char *partNumber;
        char *serialNumber;
        char *sparePartNumber;
        enum hpoa_boolean rs232PortRoute;
        enum hpoa_boolean ethernetPortRoute;
        char *userAssignedName;
        char *inBandIpAddress;
        char *urlToMgmt;
        int powerOnWatts;
        int powerOffWatts;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getInterconnectTrayInfo
{
        int bayNumber;
};

struct interconnectTrayPortInfo
{
        byte interconnectTraySlotPortNumber;
        byte bladeBayNumber;
        byte bladeMezzNumber;
        byte bladeMezzPortNumber;
        enum interconnectTrayPortStatus portStatus;
        enum interconnectTrayPortEnabled portEnabled;
        enum interconnectTrayPortUidStatus portUidStatus;
        enum interconnectTrayPortLinkLedStatus portLinkLedStatus;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct interconnectTraySlotInfo
{
        byte interconnectTraySlotNumber;
        enum interconnectTrayType type;
        int sizeport;
        struct interconnectTrayPortInfo port;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct interconnectTrayPortMap
{
        byte interconnectTrayBayNumber;
        enum portMapStatus status;
        enum interconnectTraySizeType sizeType;
        enum interconnectTrayPassThroughEnabled passThroughModeEnabled;
        byte numberOfSlots;
        int sizeslot;
        struct interconnectTraySlotInfo slot;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct powerSupplyInfo
{
        byte bayNumber;
        enum presence presence;
        char *modelNumber;
        char *sparePartNumber;
        char *serialNumber;
        int capacity;
        int actualOutput;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getPowerSupplyInfo
{
        int bayNumber;
};

struct powerSupplyStatus
{
        byte bayNumber;
        enum presence presence;
        enum opStatus operationalStatus;
        enum opStatus inputStatus;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct powerSubsystemInfo
{
        enum powerSystemType subsystemType;
        enum opStatus operationalStatus;
        enum redundancy redundancy;
        enum powerRedundancy redundancyMode;
        int capacity;
        int redundantCapacity;
        int outputPower;
        int powerConsumed;
        float inputPowerVa;
        float inputPowerCapacityVa;
        float inputPower;
        float inputPowerCapacity;
        byte goodPowerSupplies;
        byte wantedPowerSupplies;
        byte neededPowerSupplies;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct powerConfigInfo
{
        int powerCeiling;
        enum powerRedundancy redundancyMode;
        enum hpoa_boolean dynamicPowerSaverEnabled;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct thermalInfo
{
        enum sensorType sensorType;
        byte bayNumber;
        enum sensorStatus sensorStatus;
        enum opStatus operationalStatus;
        byte temperatureC;
        byte cautionThreshold;
        byte criticalThreshold;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getThermalInfo
{
        enum sensorType sensorType;
        int bayNumber;
};

struct bayAccess
{
        byte bayNumber;
        enum hpoa_boolean access;
};

struct enclosureBaysSelection
{
        enum hpoa_boolean oaAccess;
        xmlNode *bladeBays;             /* Items are struct bayAccess */
        xmlNode *interconnectTrayBays;  /* Items are struct bayAccess */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct userInfo
{
        char *username;
        char *fullname;
        char *contactInfo;
        enum hpoa_boolean isEnabled;
        enum userAcl acl;
        struct enclosureBaysSelection bayPermissions;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getUserInfo
{
        char *username;
};

struct x509CertificateInfo
{
        byte certificateVersion;
        char *issuerOrganization;
        char *issuerOrganizationalUnit;
        char *issuerCommonName;
        char *subjectOrganization;
        char *subjectOrganizationalUnit;
        char *subjectCommonName;
        time_t validFrom;
        time_t validTo;
        char *serialNumber;
        int extensionCount;
        char *md5Fingerprint;
        char *sha1Fingerprint;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct certificates
{
        int numberOfCertificates;
        int sizecertificate;
        xmlNode *certificate;   /* Items are struct x509CertificateInfo */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct ldapInfo
{
        enum hpoa_boolean ldapEnabled;
        enum hpoa_boolean localUsersEnabled;
        char *directoryServerAddress;
        short directoryServerSslPort;
        char *searchContext1;
        char *searchContext2;
        char *searchContext3;
        enum hpoa_boolean userNtAccountNameMapping;
        struct certificates certificates;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct ldapGroupInfo
{
        char *ldapGroupName;
        char *description;
        enum userAcl acl;
        struct enclosureBaysSelection bayPermissions;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct trapInfo
{
        char *ipAddress;
        char *community;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct snmpInfo
{
        char *sysName;
        char *sysLocation;
        char *sysContact;
        char *roCommunity;
        char *rwCommunity;
        int numTraps;
        xmlNode *traps;                 /* Items are struct trapInfo */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct enclosureNetworkInfo
{
        xmlNode *ntpServers;            /* Items are char *ipAddress */
        int ntpPoll;
        xmlNode *ipAllow;               /* Items are char *ipAddress */
        enum hpoa_boolean httpsEnabled;
        enum hpoa_boolean snmpEnabled;
        enum hpoa_boolean sshEnabled;
        enum hpoa_boolean telnetEnabled;
        enum hpoa_boolean ntpEnabled;
        enum hpoa_boolean ipSecurityEnabled;
        enum hpoa_boolean alertmailEnabled;
        enum hpoa_boolean ebipaSvbEnabled;
        enum hpoa_boolean ebipaSwmEnabled;
        enum hpoa_boolean xmlReplyEnabled;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct oaNetworkInfo
{
        byte bayNumber;
        enum hpoa_boolean dhcpEnabled;
        enum hpoa_boolean dynDnsEnabled;
        char *macAddress;
        char *ipAddress;
        char *netmask;
        char *gateway;
        xmlNode *dns;                   /* Items are char *ipAddress */
        char *elinkIpAddress;
        enum hpoa_boolean linkActive;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getOaNetworkInfo
{
        int bayNumber;
};

struct enclosureTime
{
        time_t dateTime;
        char *timeZone;
        time_t universalDateTime;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct alertmailInfo
{
        char *server;
        char *receiver;
        char *domain;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct ebipaInfo
{
        char *svbIpAddress;
        char *svbNetmask;
        char *svbGateway;
        char *svbDomain;
        xmlNode *svbDns;                /* Items are char *ipAddress */
        xmlNode *svbNtpServer;          /* Items are char *ipAddress */
        char *swmIpAddress;
        char *swmNetmask;
        char *swmGateway;
        char *swmDomain;
        xmlNode *swmDns;                /* Items are char *ipAddress */
        xmlNode *swmNtpServer;          /* Items are char *ipAddress */
        enum hpoa_boolean isConfiguredSvb;
        enum hpoa_boolean isConfiguredSwm;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct lcdChatMessage
{
        enum lcdChatMessageType lcdChatMessageType;
        char *screenName;
        char *questionText;
        char *answerChoiceList;
        char *selectedAnswerText;
        char *customAnswerText;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct lcdUserNotes
{
        char *lcdUserNotesLine1;
        char *lcdUserNotesLine2;
        char *lcdUserNotesLine3;
        char *lcdUserNotesLine4;
        char *lcdUserNotesLine5;
        char *lcdUserNotesLine6;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct lcdButtonEvent
{
        enum lcdButton button;
        enum lcdButtonState buttonState;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct lcdStatus
{
        enum opStatus status;
        enum uidStatus display;
        enum hpoa_boolean lcdPin;
        enum hpoa_boolean buttonLock;
        enum lcdSetupHealth lcdSetupHealth;
        struct diagnosticChecks diagnosticChecks;
#if 0
        xmlNode *diagnosticChecksEx;    /* Items are struct diagnosticData */
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct lcdInfo
{
        char *name;
        char *partNumber;
        char *manufacturer;
        char *fwVersion;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct hpSimInfo
{
        enum hpSimTrustMode trustMode;
        struct certificates certificates;
        xmlNode *xeNameList;            /* Items are char *xeName */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct thermalSubsystemInfo
{
        enum opStatus operationalStatus;
        enum redundancy redundancy;
        byte goodFans;
        byte wantedFans;
        byte neededFans;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct bladeIpl
{
        enum iplDevice iplDevice;
        byte bootPriority;
};

struct bladeBootInfo
{
        byte numberOfIpls;
        xmlNode *ipls;                  /* Items are struct bladeIpl */
        byte lastIplDeviceBooted;
        enum oneTimeBootDevice oneTimeBootDevice;
        enum oneTimeBootAgent oneTimeBootAgent;
        enum hpoa_boolean oneTimeBypassF1F2Messages;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct oaVcmMode
{
        enum hpoa_boolean isVcmMode;
        char *vcmUrl;
        char *vcmDomainId;
        char *vcmDomainName;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct powerReductionStatus
{
        enum powerReductionState powerReductionState;
        enum powerReductionArmedState powerReductionArmedState;
        enum powerReductionState powerReductionFiredState;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct virtualMediaStatus
{
        byte bayNumber;
        enum virtualMediaSupport support;
        enum virtualMediaDeviceStatus cdromStatus;
        char *cdromUrl;
        enum virtualMediaDeviceStatus floppyStatus;
        char *floppyUrl;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct fanZone
{
        byte zoneNumber;
        enum redundancy redundant;
        enum opStatus operationalStatus;
        int targetRpm;
        int targetPwm;
        xmlNode *deviceBayArray;                /* Items are short bay */
        xmlNode *fanInfoArray;                  /* Items are struct fanInfo */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct ebipaBay
{
        int bayNumber;
        enum hpoa_boolean enabled;
        char *ipAddress;
};

struct ebipaInfoEx
{
        struct ebipaInfo info;
        xmlNode *deviceBays;                    /* Items are struct ebipaBay */
        xmlNode *interconnectBays;              /* Items are struct ebipaBay */
};

struct eventPid
{
        int pid;
};

struct unSubscribeForEvents
{
        int pid;
};

struct syslogSettings
{
        char *syslogServer;
        int syslogPort;
        enum hpoa_boolean remoteEnabled;
};

struct encLinkOa
{
        enum hpoa_boolean activeOa;
        int bayNumber;
        char *oaName;
        char *ipAddress;
        char *macAddress;
        char *fwVersion;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct encLink2
{
        int enclosureNumber;
        int productId;
        int mfgId;
        char *enclosureUuid;
        char *enclosureSerialNumber;
        char *enclosureName;
        char *enclosureProductName;
        enum opStatus enclosureStatus;
        char *enclosureRackIpAddress;
        char *enclosureUrl;
        char *rackName;
        enum hpoa_boolean primaryEnclosure;
        xmlNode *encLinkOa;                     /* Items are struct encLinkOa */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct rackTopology2
{
        char *ruid;
        xmlNode *enclosures;                    /* Items are struct encLink2 */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct passwordSettings
{
        enum hpoa_boolean strictPasswordsEnabled;
        int minPasswordLength;
};

struct oaMediaDevice
{
        int bayNumber;
        int deviceIndex;
        int deviceType;
        enum presence devicePresence;
        enum presence mediaPresence;
        char *volumeLabel;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct firmwareImage
{
        char *fileName;
        char *fwVersion;
};

struct usbMediaFirmwareImages
{
        xmlNode *image;                 /* Items are struct firmwareImage */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct usbMediaConfigScript
{
        char *fileName;
};

struct usbMediaConfigScripts
{
        xmlNode *usbMediaConfigScript;          /* Items are struct
                                                 * usbMediaConfigScript
                                                 */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct tfaSettings
{
        enum hpoa_boolean enableTwoFactor;
        enum hpoa_boolean enableCrl;
        enum hpoa_boolean subjectAltName;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct userCertificateInfo
{
        char *fingerprint;
};

struct caCertsInfo
{
        xmlNode *certificates;
};

struct powerdelayBay
{
        int bayNumber;
        int delay;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct powerdelaySettings
{
        enum hpoa_boolean isPowerdelayInProgress;
        xmlNode *interconnects;         /* Items are struct powerdelayBay */
        xmlNode *servers;               /* Items are struct powerdelayBay */
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
};

struct getEvent
{
        int pid;
        enum hpoa_boolean waitTilEventHappens;
        enum hpoa_boolean lcdEvents;
};

struct getAllEvents
{
        int pid;
        enum hpoa_boolean waitTilEventHappens;
        enum hpoa_boolean lcdEvents;
};

union _hpoa__data
{
        struct syslog syslog;
        struct rackTopology rackTopology;
        struct enclosureStatus enclosureStatus;
        struct enclosureInfo enclosureInfo;
        struct oaStatus oaStatus;
        struct oaInfo oaInfo;
        struct bladeInfo bladeInfo;
        struct bladeMpInfo bladeMpInfo;
        struct bladeStatus bladeStatus;
        struct bladePortMap bladePortMap;
        struct fanInfo fanInfo;
        struct interconnectTrayStatus interconnectTrayStatus;
        struct interconnectTrayInfo interconnectTrayInfo;
        struct interconnectTrayPortMap interconnectTrayPortMap;
        struct powerSupplyInfo powerSupplyInfo;
        struct powerSupplyStatus powerSupplyStatus;
        struct powerSubsystemInfo powerSubsystemInfo;
        struct powerConfigInfo powerConfigInfo;
        struct thermalInfo thermalInfo;
        xmlNode *userInfoArray;         /* Items are struct userInfo */
        struct userInfo userInfo;
        struct ldapInfo ldapInfo;
        struct ldapGroupInfo ldapGroupInfo;
        struct snmpInfo snmpInfo;
        struct enclosureNetworkInfo enclosureNetworkInfo;
        struct oaNetworkInfo oaNetworkInfo;
        struct enclosureTime enclosureTime;
        struct alertmailInfo alertmailInfo;
        struct passwordSettings passwordSettings;
        struct ebipaInfo ebipaInfo;
        struct lcdChatMessage lcdChatMessage;
        struct lcdUserNotes lcdUserNotes;
        struct lcdButtonEvent lcdButtonEvent;
        struct lcdStatus lcdStatus;
        struct lcdInfo lcdInfo;
        struct hpSimInfo hpSimInfo;
        struct thermalSubsystemInfo thermalSubsystemInfo;
        struct bladeBootInfo bladeBootInfo;
        struct oaVcmMode oaVcmMode;
        struct powerReductionStatus powerReductionStatus;
        struct virtualMediaStatus virtualMediaStatus;
        struct oaMediaDevice oaMediaDevice;
        struct fanZone fanZone;
        struct ebipaInfoEx ebipaInfoEx;
        struct caCertsInfo caCertsInfo;
        struct rackTopology2 rackTopology2;
        struct userCertificateInfo userCertificateInfo;
        struct syslogSettings syslogSettings;
        struct powerdelaySettings powerdelaySettings;
        struct usbMediaFirmwareImages usbMediaFirmwareImages;
        struct usbMediaConfigScripts configScripts;
        int numValue;
        char *string;
        char *message;
        char *noPayload;
};

struct eventInfo
{
        enum eventType event;
        time_t eventTimeStamp;
        int queueSize;
        union _hpoa__data eventData;
#if 0
        int sizeextraData;
        xmlNode *extraData;
#endif
        enum enum_eventInfo enum_eventInfo;
};

struct getAllEventsResponse
{
        xmlNode *eventInfoArray;
};

struct setBladePower
{
        int bayNumber;
        enum powerControl power;
};

struct setInterconnectTrayPower
{
        int bayNumber;
        enum hpoa_boolean on;
};

struct resetInterconnectTray
{
        int bayNumber;
};


/* Main OA SOAP Function prototypes */
int soap_subscribeForEvents(SOAP_CON *connection,
                            struct eventPid *response);

int soap_unSubscribeForEvents(SOAP_CON *connection,
                              struct unSubscribeForEvents *request);

int soap_getEvent(SOAP_CON *connection,
                  struct getEvent *request,
                  struct eventInfo *response);

int soap_getAllEvents(SOAP_CON *connection,
                      struct getAllEvents *request,
                      struct getAllEventsResponse *response);

int soap_getBladeInfo(SOAP_CON *connection,
                      struct getBladeInfo *request,
                      struct bladeInfo *response);

int soap_getBladeMpInfo(SOAP_CON *connection,
                        struct getBladeMpInfo *request,
                        struct bladeMpInfo *response);

int soap_getEnclosureInfo(SOAP_CON *connection,
                          struct enclosureInfo *response);

int soap_getOaStatus(SOAP_CON *connection,
                     struct getOaStatus *request,
                     struct oaStatus *response);

int soap_getOaInfo(SOAP_CON *connection,
                   struct getOaInfo *request,
                   struct oaInfo *response);

int soap_getInterconnectTrayStatus(SOAP_CON *connection,
                                   struct getInterconnectTrayStatus *request,
                                   struct interconnectTrayStatus *response);

int soap_getInterconnectTrayInfo(SOAP_CON *connection,
                                 struct getInterconnectTrayInfo *request,
                                 struct interconnectTrayInfo *response);

int soap_getFanInfo(SOAP_CON *connection,
                    struct getFanInfo *request,
                    struct fanInfo *response);

int soap_getPowerSubsystemInfo(SOAP_CON *connection,
                               struct powerSubsystemInfo *response);

int soap_getPowerSupplyInfo(SOAP_CON *connection,
                            struct getPowerSupplyInfo *request,
                            struct powerSupplyInfo *response);

int soap_getOaNetworkInfo(SOAP_CON *connection,
                          struct getOaNetworkInfo *request,
                          struct oaNetworkInfo *response);

int soap_getBladeStatus(SOAP_CON *connection,
                        struct getBladeStatus *request,
                        struct bladeStatus *response);

int soap_setBladePower(SOAP_CON *connection,
                       struct setBladePower *request);

int soap_setInterconnectTrayPower(SOAP_CON *connection,
                                  struct setInterconnectTrayPower *request);

int soap_resetInterconnectTray(SOAP_CON *connection,
                               struct resetInterconnectTray *request);

int soap_getThermalInfo(SOAP_CON *connection,
                        struct getThermalInfo *request,
                        struct thermalInfo *response);

int soap_getUserInfo(SOAP_CON *connection,
                     struct getUserInfo *request,
                     struct userInfo *response);

int soap_isValidSession(SOAP_CON *connection);

int soap_getRackTopology2(SOAP_CON *con,
                          struct rackTopology2 *response);


/* Function prototypes for OA SOAP helper functions */
void    soap_getBladeCpuInfo(xmlNode *cpus, struct bladeCpuInfo *result);
void    soap_getBladeNicInfo(xmlNode *nics, struct bladeNicInfo *result);
void    soap_getDiagnosticData(xmlNode *data, struct diagnosticData *result);
void    soap_getBayAccess(xmlNode *bay, struct bayAccess *result);
void    soap_getEncLink(xmlNode *data, struct encLink *result);
void    soap_getPortEnabled(xmlNode *data, struct portEnabled *result);
void    soap_getIpAddress(xmlNode *ips, char **result);
void    soap_getEventInfo(xmlNode *events, struct eventInfo *result);

void    soap_getEncLinkOa(xmlNode *data, struct encLinkOa *result);
void    soap_getEncLink2(xmlNode *data, struct encLink2 *result);

#endif  /* _INC_OASOAP_CALLS_H_ */
