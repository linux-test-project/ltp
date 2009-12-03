/*
 * ipmi_cmd.h
 *
 * Interface for IPMI file connection
 *
 * Copyright (c) 2003 by FORCE Computers.
 * Copyright (c) 2005 by ESO Technologies.
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
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#ifndef dIpmiCmd_h
#define dIpmiCmd_h


#define dMaxIpmiDataSize 36


//
// IPMI commands
//

enum tIpmiCmd
{
  // Chassis netfn (0x00)
  eIpmiCmdGetChassisCapabilities     = 0x00,
  eIpmiCmdGetChassisStatus           = 0x01,
  eIpmiCmdChassisControl             = 0x02,
  eIpmiCmdChassisReset               = 0x03,
  eIpmiCmdChassisIdentify            = 0x04,
  eIpmiCmdSetChassisCapabilities     = 0x05,
  eIpmiCmdSetPowerRestorePolicy      = 0x06,
  eIpmiCmdGetSystemRestartCause      = 0x07,
  eIpmiCmdSetSystemBootOptions       = 0x08,
  eIpmiCmdGetSystemBootOptions       = 0x09,

  eIpmiCmdGetPohCounter              = 0x0f,

  // Bridge netfn (0x00)
  eIpmiCmdGetBridgeState             = 0x00,
  eIpmiCmdSetBridgeState             = 0x01,
  eIpmiCmdGetIcmbAddress             = 0x02,
  eIpmiCmdSetIcmbAddress             = 0x03,
  eIpmiCmdSetBridgeProxyAddress      = 0x04,
  eIpmiCmdGetBridgeStatistics        = 0x05,
  eIpmiCmdGetIcmbCapabilities        = 0x06,

  eIpmiCmdClearBridgeStatistics      = 0x08,
  eIpmiCmdGetBridgeProxyAddress      = 0x09,
  eIpmiCmdGetIcmbConnectorInfo       = 0x0a,
  eIpmiCmdSetIcmbConnectorInfo       = 0x0b,
  eIpmiCmdSendIcmbConnectionId       = 0x0c,

  eIpmiCmdPrepareForDiscovery        = 0x10,
  eIpmiCmdGetAddresses               = 0x11,
  eIpmiCmdSetDiscovered              = 0x12,
  eIpmiCmdGetChassisDeviceId         = 0x13,
  eIpmiCmdSetChassisDeviceId         = 0x14,

  eIpmiCmdBridgeRequest              = 0x20,
  eIpmiCmdBridgeMessage              = 0x21,

  eIpmiCmdGetEventCount              = 0x30,
  eIpmiCmdSetEventdestination        = 0x31,
  eIpmiCmdSetEventReceptionState     = 0x32,
  eIpmiCmdSendIcmbEventMessage       = 0x33,
  eIpmiCmdGetEventDestiation         = 0x34,
  eIpmiCmdGetEventReceptionState     = 0x35,

  eIpmiCmdErrorReport                = 0xff,

  // Sensor/Event netfn (0x04)
  eIpmiCmdSetEventReceiver           = 0x00,
  eIpmiCmdGetEventReceiver           = 0x01,
  eIpmiCmdPlatformEvent              = 0x02,

  eIpmiCmdGetPefCapabilities         = 0x10,
  eIpmiCmdArmPefPostponeTimer        = 0x11,
  eIpmiCmdSetPefConfigParms          = 0x12,
  eIpmiCmdGetPefConfigParms          = 0x13,
  eIpmiCmdSetLastProcessedEventId    = 0x14,
  eIpmiCmdGetLastProcessedEventId    = 0x15,
  eIpmiCmdAlertImmediate             = 0x16,
  eIpmiCmdPetAcknowledge             = 0x17,

  eIpmiCmdGetDeviceSdrInfo           = 0x20,
  eIpmiCmdGetDeviceSdr               = 0x21,
  eIpmiCmdReserveDeviceSdrRepository = 0x22,
  eIpmiCmdGetSensorReadingFactors    = 0x23,
  eIpmiCmdSetSensorHysteresis        = 0x24,
  eIpmiCmdGetSensorHysteresis        = 0x25,
  eIpmiCmdSetSensorThreshold         = 0x26,
  eIpmiCmdGetSensorThreshold         = 0x27,
  eIpmiCmdSetSensorEventEnable       = 0x28,
  eIpmiCmdGetSensorEventEnable       = 0x29,
  eIpmiCmdRearmSensorEvents          = 0x2a,
  eIpmiCmdGetSensorEventStatus       = 0x2b,
  eIpmiCmdGetSensorReading           = 0x2d,
  eIpmiCmdSetSensorType              = 0x2e,
  eIpmiCmdGetSensorType              = 0x2f,

  // App netfn (0x06)
  eIpmiCmdGetDeviceId                = 0x01,
  eIpmiCmdBroadcastGetDeviceId       = 0x01,
  eIpmiCmdColdReset                  = 0x02,
  eIpmiCmdWarmReset                  = 0x03,
  eIpmiCmdGetSelfTestResults         = 0x04,
  eIpmiCmdManufacturingTestOn        = 0x05,
  eIpmiCmdSetAcpiPowerState          = 0x06,
  eIpmiCmdGetAcpiPowerState          = 0x07,
  eIpmiCmdGetDeviceGuid              = 0x08,
  eIpmiCmdResetWatchdogTimer         = 0x22,
  eIpmiCmdSetWatchdogTimer           = 0x24,
  eIpmiCmdGetWatchdogTimer           = 0x25,
  eIpmiCmdSetBmcGlobalEnables        = 0x2e,
  eIpmiCmdGetBmcGlobalEnables        = 0x2f,
  eIpmiCmdClearMsgFlags              = 0x30,
  eIpmiCmdGetMsgFlags                = 0x31,
  eIpmiCmdEnableMessageChannelRcv    = 0x32,
  eIpmiCmdGetMsg                     = 0x33,
  eIpmiCmdSendMsg                    = 0x34,
  eIpmiCmdReadEventMsgBuffer         = 0x35,
  eIpmiCmdGetBtInterfaceCapabilities = 0x36,
  eIpmiCmdGetSystemGuid              = 0x37,
  eIpmiCmdGetChannelAuthCapabilities = 0x38,
  eIpmiCmdGetSessionChallenge        = 0x39,
  eIpmiCmdActivateSession            = 0x3a,
  eIpmiCmdSetSessionPrivilege        = 0x3b,
  eIpmiCmdCloseSession               = 0x3c,
  eIpmiCmdGetSessionInfo             = 0x3d,

  eIpmiCmdGetAuthcode                = 0x3f,
  eIpmiCmdSetChannelAccess           = 0x40,
  eIpmiCmdGetChannelAccess           = 0x41,
  eIpmiCmdGetChannelInfo             = 0x42,
  eIpmiCmdSetUserAccess              = 0x43,
  eIpmiCmdGetUserAccess              = 0x44,
  eIpmiCmdSetUserName                = 0x45,
  eIpmiCmdGetUserName                = 0x46,
  eIpmiCmdSetUserPassword            = 0x47,

  eIpmiCmdMasterReadWrite            = 0x52,

  // Storage netfn (0x0a)
  eIpmiCmdGetFruInventoryAreaInfo    = 0x10,
  eIpmiCmdReadFruData                = 0x11,
  eIpmiCmdWriteFruData               = 0x12,

  eIpmiCmdGetSdrRepositoryInfo       = 0x20,
  eIpmiCmdGetSdrRepositoryAllocInfo  = 0x21,
  eIpmiCmdReserveSdrRepository       = 0x22,
  eIpmiCmdGetSdr                     = 0x23,
  eIpmiCmdAddSdr                     = 0x24,
  eIpmiCmdPartialAddSdr              = 0x25,
  eIpmiCmdDeleteSdr                  = 0x26,
  eIpmiCmdClearSdrRepository         = 0x27,
  eIpmiCmdGetSdrRepositoryTime       = 0x28,
  eIpmiCmdSetSdrRepositoryTime       = 0x29,
  eIpmiCmdEnterSdrRepositoryUpdate   = 0x2a,
  eIpmiCmdExitSdrRepositoryUpdate    = 0x2b,
  eIpmiCmdRunInitializationAgent     = 0x2c,

  eIpmiCmdGetSelInfo                 = 0x40,
  eIpmiCmdGetSelAllocationInfo       = 0x41,
  eIpmiCmdReserveSel                 = 0x42,
  eIpmiCmdGetSelEntry                = 0x43,
  eIpmiCmdAddSelEntry                = 0x44,
  eIpmiCmdPartialAddSelEntry         = 0x45,
  eIpmiCmdDeleteSelEntry             = 0x46,
  eIpmiCmdClearSel                   = 0x47,
  eIpmiCmdGetSelTime                 = 0x48,
  eIpmiCmdSetSelTime                 = 0x49,
  eIpmiCmdGetAuxiliaryLogStatus      = 0x5a,
  eIpmiCmdSetAuxiliaryLogStatus      = 0x5b,

  // Transport netfn (0x0c)
  eIpmiCmdSetLanConfigParms          = 0x01,
  eIpmiCmdGetLanConfigParms          = 0x02,
  eIpmiCmdSuspendBmcArps             = 0x03,
  eIpmiCmdGetIpUdpRmcpStats          = 0x04,

  eIpmiCmdSetSerialModemConfig       = 0x10,
  eIpmiCmdGetSerialModemConfig       = 0x11,
  eIpmiCmdSetSerialModemMux          = 0x12,
  eIpmiCmdGetTapResponseCodes        = 0x13,
  eIpmiCmdSetPppUdpProxyXmitData     = 0x14,
  eIpmiCmdGetPppUdpProxyXmitData     = 0x15,
  eIpmiCmdSendPppUdpProxyPacket      = 0x16,
  eIpmiCmdGetPppUdpProxyRecvData     = 0x17,
  eIpmiCmdSerialModemConnActive      = 0x18,
  eIpmiCmdCallback                   = 0x19,
  eIpmiCmdSetUserCallbackOptions     = 0x1a,
  eIpmiCmdGetUserCallbackOptions     = 0x1b,

  // PICMG netfn (0x2c)
  eIpmiCmdGetPicMgProperties         = 0x00,
  eIpmiCmdGetAddressInfo             = 0x01,
  eIpmiCmdGetShelfAddressInfo        = 0x02,
  eIpmiCmdSetShelfAddressInfo        = 0x03,
  eIpmiCmdFruControl                 = 0x04,
  eIpmiCmdGetFruLedProperties        = 0x05,
  eIpmiCmdGetLedColorCapabilities    = 0x06,
  eIpmiCmdSetFruLedState             = 0x07,
  eIpmiCmdGetFruLedState             = 0x08,
  eIpmiCmdSetIpmbState               = 0x09,
  eIpmiCmdSetFruActivationPolicy     = 0x0a,
  eIpmiCmdGetFruActivationPolicy     = 0x0b,
  eIpmiCmdSetFruActivation           = 0x0c,
  eIpmiCmdGetDeviceLocatorRecordId   = 0x0d,
  eIpmiCmdSetPortState               = 0x0e,
  eIpmiCmdGetPortState               = 0x0f,
  eIpmiCmdComputePowerProperties     = 0x10,
  eIpmiCmdSetPowerLevel              = 0x11,
  eIpmiCmdGetPowerLevel              = 0x12,
  eIpmiCmdRenegotiatePower           = 0x13,
  eIpmiCmdGetFanSpeedProperties      = 0x14,
  eIpmiCmdSetFanLevel                = 0x15,
  eIpmiCmdGetFanLevel                = 0x16,
  eIpmiCmdBusedResource              = 0x17,
  eIpmiCmdGetIpmbLinkInfo            = 0x18,
};


// PICMG Identifier. Indicates that this is a PICMG-defined
// group extension command.
#define dIpmiPicMgId 0


#define dIpmiDeactivateFru 0
#define dIpmiActivateFru   1


//
// NetFNs
//
enum tIpmiNetfn
{
  eIpmiNetfnChassis        = 0x00,
  eIpmiNetfnChassisRsp     = 0x01,
  eIpmiNetfnBridge         = 0x02,
  eIpmiNetfnBridgeRsp      = 0x03,
  eIpmiNetfnSensorEvent    = 0x04,
  eIpmiNetfnSensorEventRsp = 0x05,
  eIpmiNetfnApp            = 0x06,
  eIpmiNetfnAppRsp         = 0x07,
  eIpmiNetfnFirmware       = 0x08,
  eIpmiNetfnFirmwareRsp    = 0x09,
  eIpmiNetfnStorage        = 0x0a,
  eIpmiNetfnStorageRsp     = 0x0b,
  eIpmiNetfnTransport      = 0x0c,
  eIpmiNetfnTransportRsp   = 0x0d,
  eIpmiNetfnPicmg          = 0x2c,
  eIpmiNetfnPicmgRsp       = 0x2d,
  eIpmiNetfnOem            = 0x2e,
  eIpmiNetfnOemRsp         = 0x2f
};

const char *IpmiNetfnToString( tIpmiNetfn netfn );


//
// Completion codes for IPMI.
//

enum tIpmiCompletionCode
{
  eIpmiCcOk                          = 0x00,
  eIpmiCcNodeBusy                    = 0xC0,
  eIpmiCcInvalidCmd                  = 0xC1,
  eIpmiCcCommandInvalidForLun        = 0xC2,
  eIpmiCcTimeout                     = 0xC3,
  eIpmiCcOutOfSpace                  = 0xC4,
  eIpmiCcInvalidReservation          = 0xC5,
  eIpmiCcRequestDataTruncated        = 0xC6,
  eIpmiCcRequestDataLengthInvalid    = 0xC7,
  eIpmiCcRequestedDataLengthExceeded = 0xC8,
  eIpmiCcParameterOutOfRange         = 0xC9,
  eIpmiCcCannotReturnReqLength       = 0xCA,
  eIpmiCcNotPresent                  = 0xCB,
  eIpmiCcInvalidDataField            = 0xCC,
  eIpmiCcCommandIllegalForSensor     = 0xCD,
  eIpmiCcCouldNotProvideResponse     = 0xCE,
  eIpmiCcCannotExecDuplicateRequest  = 0xCF,
  eIpmiCcRepositoryInUpdateMode	     = 0xD0,
  eIpmiCcDeviceInFirmwareUpdate	     = 0xD1,
  eIpmiCcBmcInitInProgress           = 0xD2,
  eIpmiCcDestinationUnavailable      = 0xD3,
  eIpmiCcInsufficientPrivilege       = 0xD4,
  eIpmiCcNotSupportedInPresentState  = 0xD5,
  eIpmiCcUnknownErr                  = 0xff
};

const char *IpmiCompletionCodeToString( tIpmiCompletionCode cc );


const char *IpmiCmdToString( tIpmiNetfn netfn, tIpmiCmd cmd );


#endif
