/*
 *
 * Copyright (c) 2003 by FORCE Computers.
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
 */

#include "ipmi_cmd.h"


struct cConvMap
{
  const char *m_name;
  int         m_value;
};


const char *
ConvIntToString( int flags, const cConvMap *map, const char *def )
{
  for( int i = 0; map->m_name; i++, map++ )
       if ( flags == map->m_value )
	    return map->m_name;

  return def;
}


static cConvMap netfn_map[] =
{
  { "Chassis"    , eIpmiNetfnChassis     },
  { "Bridge"     , eIpmiNetfnBridge      },
  { "SensorEvent", eIpmiNetfnSensorEvent },
  { "App"        , eIpmiNetfnApp         },
  { "Firmware"   , eIpmiNetfnFirmware    },
  { "Storage "   , eIpmiNetfnStorage     },
  { "Transport"  , eIpmiNetfnTransport   },
  { "Picmg "     , eIpmiNetfnPicmg       },
  { 0, 0 }
};
 

const char *
IpmiNetfnToString( tIpmiNetfn netfn )
{
  return ConvIntToString( netfn, netfn_map, "Invalid" );	
}


static cConvMap completion_code_map[] =
{
  { "Ok"                         , eIpmiCcOk                          },
  { "NodeBusy"                   , eIpmiCcNodeBusy                    },
  { "InvalidCmd"                 , eIpmiCcInvalidCmd                  },
  { "CommandInvalidForLun"       , eIpmiCcCommandInvalidForLun        },
  { "Timeout"                    , eIpmiCcTimeout                     },
  { "OutOfSpace"                 , eIpmiCcOutOfSpace                  },
  { "InvalidReservation"         , eIpmiCcInvalidReservation          },
  { "RequestDataTruncated"       , eIpmiCcRequestDataTruncated        },
  { "RequestDataLengthInvalid"   , eIpmiCcRequestDataLengthInvalid    },
  { "RequestedDataLengthExceeded", eIpmiCcRequestedDataLengthExceeded },
  { "ParameterOutOfRange"        , eIpmiCcParameterOutOfRange         },
  { "CannotReturnReqLength"      , eIpmiCcCannotReturnReqLength       },
  { "NotPresent"                 , eIpmiCcNotPresent                  },
  { "InvalidDataField"           , eIpmiCcInvalidDataField            },
  { "CommandIllegalForSensor"    , eIpmiCcCommandIllegalForSensor     },
  { "CouldNotProvideResponse"    , eIpmiCcCouldNotProvideResponse     },
  { "CannotExecDuplicateRequest" , eIpmiCcCannotExecDuplicateRequest  },
  { "RepositoryInUpdateMode"     , eIpmiCcRepositoryInUpdateMode      },
  { "DeviceInFirmwareUpdate"     , eIpmiCcDeviceInFirmwareUpdate      },
  { "BmcInitInProgress"          , eIpmiCcBmcInitInProgress           },
  { "DestinationUnavailable"     , eIpmiCcDestinationUnavailable      },
  { "InsufficientPrivilege"      , eIpmiCcInsufficientPrivilege       },
  { "NotSupportedInPresentState" , eIpmiCcNotSupportedInPresentState  },
  { "UnknownErr"                 , eIpmiCcUnknownErr                  },
  { 0, 0 }
};


const char *
IpmiCompletionCodeToString( tIpmiCompletionCode cc )
{
  return ConvIntToString( cc, completion_code_map, "Invalid" );	
}


struct cIpmiCmdToClass
{
  const char *m_name;
  tIpmiNetfn  m_netfn;
  tIpmiCmd    m_cmd;
};


static cIpmiCmdToClass cmd_class_map[] =
{
  // Chassis netfn
  { "GetChassisCapabilities"    , eIpmiNetfnChassis    , eIpmiCmdGetChassisCapabilities     },
  { "GetChassisStatus"          , eIpmiNetfnChassis    , eIpmiCmdGetChassisStatus           },
  { "ChassisControl"            , eIpmiNetfnChassis    , eIpmiCmdChassisControl             },
  { "ChassisReset"              , eIpmiNetfnChassis    , eIpmiCmdChassisReset               },
  { "ChassisIdentify"           , eIpmiNetfnChassis    , eIpmiCmdChassisIdentify            },
  { "SetChassisCapabilities"    , eIpmiNetfnChassis    , eIpmiCmdSetChassisCapabilities     },
  { "SetPowerRestorePolicy"     , eIpmiNetfnChassis    , eIpmiCmdSetPowerRestorePolicy      },
  { "GetSystemRestartCause"     , eIpmiNetfnChassis    , eIpmiCmdGetSystemRestartCause      },
  { "SetSystemBootOptions"      , eIpmiNetfnChassis    , eIpmiCmdSetSystemBootOptions       },
  { "GetSystemBootOptions"      , eIpmiNetfnChassis    , eIpmiCmdGetSystemBootOptions       },
  { "GetPohCounter"             , eIpmiNetfnChassis    , eIpmiCmdGetPohCounter              },

  // Bridge netfn
  { "GetBridgeState"            , eIpmiNetfnBridge     , eIpmiCmdGetBridgeState             },
  { "SetBridgeState"            , eIpmiNetfnBridge     , eIpmiCmdSetBridgeState             },
  { "GetIcmbAddress"            , eIpmiNetfnBridge     , eIpmiCmdGetIcmbAddress             },
  { "SetIcmbAddress"            , eIpmiNetfnBridge     , eIpmiCmdSetIcmbAddress             },
  { "SetBridgeProxyAddress"     , eIpmiNetfnBridge     , eIpmiCmdSetBridgeProxyAddress      },
  { "GetBridgeStatistics"       , eIpmiNetfnBridge     , eIpmiCmdGetBridgeStatistics        },
  { "GetIcmbCapabilities"       , eIpmiNetfnBridge     , eIpmiCmdGetIcmbCapabilities        },

  { "ClearBridgeStatistics"     , eIpmiNetfnBridge     , eIpmiCmdClearBridgeStatistics      },
  { "GetBridgeProxyAddress"     , eIpmiNetfnBridge     , eIpmiCmdGetBridgeProxyAddress      },
  { "GetIcmbConnectorInfo"      , eIpmiNetfnBridge     , eIpmiCmdGetIcmbConnectorInfo       },
  { "SetIcmbConnectorInfo"      , eIpmiNetfnBridge     , eIpmiCmdSetIcmbConnectorInfo       },
  { "SendIcmbConnectionId"      , eIpmiNetfnBridge     , eIpmiCmdSendIcmbConnectionId       },

  { "PrepareForDiscovery"       , eIpmiNetfnBridge     , eIpmiCmdPrepareForDiscovery        },
  { "GetAddresses"              , eIpmiNetfnBridge     , eIpmiCmdGetAddresses               },
  { "SetDiscovered"             , eIpmiNetfnBridge     , eIpmiCmdSetDiscovered              },
  { "GetChassisDeviceId"        , eIpmiNetfnBridge     , eIpmiCmdGetChassisDeviceId         },
  { "SetChassisDeviceId"        , eIpmiNetfnBridge     , eIpmiCmdSetChassisDeviceId         },

  { "BridgeRequest"             , eIpmiNetfnBridge     , eIpmiCmdBridgeRequest              },
  { "BridgeMessage"             , eIpmiNetfnBridge     , eIpmiCmdBridgeMessage              },

  { "GetEventCount"             , eIpmiNetfnBridge     , eIpmiCmdGetEventCount              },
  { "SetEventdestination"       , eIpmiNetfnBridge     , eIpmiCmdSetEventdestination        },
  { "SetEventReceptionState"    , eIpmiNetfnBridge     , eIpmiCmdSetEventReceptionState     },
  { "SendIcmbEventMessage"      , eIpmiNetfnBridge     , eIpmiCmdSendIcmbEventMessage       },
  { "GetEventDestiation"        , eIpmiNetfnBridge     , eIpmiCmdGetEventDestiation         },
  { "GetEventReceptionState"    , eIpmiNetfnBridge     , eIpmiCmdGetEventReceptionState     },

  { "ErrorReport"               , eIpmiNetfnBridge     , eIpmiCmdErrorReport                },

  // Sensor/Event netfn
  { "SetEventReceiver"          , eIpmiNetfnSensorEvent, eIpmiCmdSetEventReceiver           },
  { "GetEventReceiver"          , eIpmiNetfnSensorEvent, eIpmiCmdGetEventReceiver           },
  { "PlatformEvent"             , eIpmiNetfnSensorEvent, eIpmiCmdPlatformEvent              },

  { "GetPefCapabilities"        , eIpmiNetfnSensorEvent, eIpmiCmdGetPefCapabilities         },
  { "ArmPefPostponeTimer"       , eIpmiNetfnSensorEvent, eIpmiCmdArmPefPostponeTimer        },
  { "SetPefConfigParms"         , eIpmiNetfnSensorEvent, eIpmiCmdSetPefConfigParms          },
  { "GetPefConfigParms"         , eIpmiNetfnSensorEvent, eIpmiCmdGetPefConfigParms          },
  { "SetLastProcessedEventId"   , eIpmiNetfnSensorEvent, eIpmiCmdSetLastProcessedEventId    },
  { "GetLastProcessedEventId"   , eIpmiNetfnSensorEvent, eIpmiCmdGetLastProcessedEventId    },
  { "AlertImmediate"            , eIpmiNetfnSensorEvent, eIpmiCmdAlertImmediate             },
  { "PetAcknowledge"            , eIpmiNetfnSensorEvent, eIpmiCmdPetAcknowledge             },

  { "GetDeviceSdrInfo"          , eIpmiNetfnSensorEvent, eIpmiCmdGetDeviceSdrInfo           },
  { "GetDeviceSdr"              , eIpmiNetfnSensorEvent, eIpmiCmdGetDeviceSdr               },
  { "ReserveDeviceSdrRepository", eIpmiNetfnSensorEvent, eIpmiCmdReserveDeviceSdrRepository },
  { "GetSensorReadingFactors"   , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorReadingFactors    },
  { "SetSensorHysteresis"       , eIpmiNetfnSensorEvent, eIpmiCmdSetSensorHysteresis        },
  { "GetSensorHysteresis"       , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorHysteresis        },
  { "SetSensorThreshold"        , eIpmiNetfnSensorEvent, eIpmiCmdSetSensorThreshold         },
  { "GetSensorThreshold"        , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorThreshold         },
  { "SetSensorEventEnable"      , eIpmiNetfnSensorEvent, eIpmiCmdSetSensorEventEnable       },
  { "GetSensorEventEnable"      , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorEventEnable       },
  { "RearmSensorEvents"         , eIpmiNetfnSensorEvent, eIpmiCmdRearmSensorEvents          },
  { "GetSensorEventStatus"      , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorEventStatus       },
  { "GetSensorReading"          , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorReading           },
  { "SetSensorType"             , eIpmiNetfnSensorEvent, eIpmiCmdSetSensorType              },
  { "GetSensorType"             , eIpmiNetfnSensorEvent, eIpmiCmdGetSensorType              },

  // App netfn
  { "GetDeviceId"               , eIpmiNetfnApp        , eIpmiCmdGetDeviceId                },
  { "BroadcastGetDeviceId"      , eIpmiNetfnApp        , eIpmiCmdBroadcastGetDeviceId       },
  { "ColdReset"                 , eIpmiNetfnApp        , eIpmiCmdColdReset                  },
  { "WarmReset"                 , eIpmiNetfnApp        , eIpmiCmdWarmReset                  },
  { "GetSelfTestResults"        , eIpmiNetfnApp        , eIpmiCmdGetSelfTestResults         },
  { "ManufacturingTestOn"       , eIpmiNetfnApp        , eIpmiCmdManufacturingTestOn        },
  { "SetAcpiPowerState"         , eIpmiNetfnApp        , eIpmiCmdSetAcpiPowerState          },
  { "GetAcpiPowerState"         , eIpmiNetfnApp        , eIpmiCmdGetAcpiPowerState          },
  { "GetDeviceGuid"             , eIpmiNetfnApp        , eIpmiCmdGetDeviceGuid              },
  { "ResetWatchdogTimer"        , eIpmiNetfnApp        , eIpmiCmdResetWatchdogTimer         },
  { "SetWatchdogTimer"          , eIpmiNetfnApp        , eIpmiCmdSetWatchdogTimer           },
  { "GetWatchdogTimer"          , eIpmiNetfnApp        , eIpmiCmdGetWatchdogTimer           },
  { "SetBmcGlobalEnables"       , eIpmiNetfnApp        , eIpmiCmdSetBmcGlobalEnables        },
  { "GetBmcGlobalEnables"       , eIpmiNetfnApp        , eIpmiCmdGetBmcGlobalEnables        },
  { "ClearMsgFlags"             , eIpmiNetfnApp        , eIpmiCmdClearMsgFlags              },
  { "GetMsgFlags"               , eIpmiNetfnApp        , eIpmiCmdGetMsgFlags                },
  { "EnableMessageChannelRcv"   , eIpmiNetfnApp        , eIpmiCmdEnableMessageChannelRcv    },
  { "GetMsg"                    , eIpmiNetfnApp        , eIpmiCmdGetMsg                     },
  { "SendMsg"                   , eIpmiNetfnApp        , eIpmiCmdSendMsg                    },
  { "ReadEventMsgBuffer"        , eIpmiNetfnApp        , eIpmiCmdReadEventMsgBuffer         },
  { "GetBtInterfaceCapabilities", eIpmiNetfnApp        , eIpmiCmdGetBtInterfaceCapabilities },
  { "GetSystemGuid"             , eIpmiNetfnApp        , eIpmiCmdGetSystemGuid              },
  { "GetChannelAuthCapabilities", eIpmiNetfnApp        , eIpmiCmdGetChannelAuthCapabilities },
  { "GetSessionChallenge"       , eIpmiNetfnApp        , eIpmiCmdGetSessionChallenge        },
  { "ActivateSession"           , eIpmiNetfnApp        , eIpmiCmdActivateSession            },
  { "SetSessionPrivilege"       , eIpmiNetfnApp        , eIpmiCmdSetSessionPrivilege        },
  { "CloseSession"              , eIpmiNetfnApp        , eIpmiCmdCloseSession               },
  { "GetSessionInfo"            , eIpmiNetfnApp        , eIpmiCmdGetSessionInfo             },

  { "GetAuthcode"               , eIpmiNetfnApp        , eIpmiCmdGetAuthcode                },
  { "SetChannelAccess"          , eIpmiNetfnApp        , eIpmiCmdSetChannelAccess           },
  { "GetChannelAccess"          , eIpmiNetfnApp        , eIpmiCmdGetChannelAccess           },
  { "GetChannelInfo"            , eIpmiNetfnApp        , eIpmiCmdGetChannelInfo             },
  { "SetUserAccess"             , eIpmiNetfnApp        , eIpmiCmdSetUserAccess              },
  { "GetUserAccess"             , eIpmiNetfnApp        , eIpmiCmdGetUserAccess              },
  { "SetUserName"               , eIpmiNetfnApp        , eIpmiCmdSetUserName                },
  { "GetUserName"               , eIpmiNetfnApp        , eIpmiCmdGetUserName                },
  { "SetUserPassword"           , eIpmiNetfnApp        , eIpmiCmdSetUserPassword            },

  { "MasterReadWrite"           , eIpmiNetfnApp        , eIpmiCmdMasterReadWrite            },

  // Storage netfn
  { "GetFruInventoryAreaInfo"   , eIpmiNetfnStorage    , eIpmiCmdGetFruInventoryAreaInfo    },
  { "ReadFruData"               , eIpmiNetfnStorage    , eIpmiCmdReadFruData                },
  { "WriteFruData"              , eIpmiNetfnStorage    , eIpmiCmdWriteFruData               },

  { "GetSdrRepositoryInfo"      , eIpmiNetfnStorage    , eIpmiCmdGetSdrRepositoryInfo       },
  { "GetSdrRepositoryAllocInfo" , eIpmiNetfnStorage    , eIpmiCmdGetSdrRepositoryAllocInfo  },
  { "ReserveSdrRepository"      , eIpmiNetfnStorage    , eIpmiCmdReserveSdrRepository       },
  { "GetSdr"                    , eIpmiNetfnStorage    , eIpmiCmdGetSdr                     },
  { "AddSdr"                    , eIpmiNetfnStorage    , eIpmiCmdAddSdr                     },
  { "PartialAddSdr"             , eIpmiNetfnStorage    , eIpmiCmdPartialAddSdr              },
  { "DeleteSdr"                 , eIpmiNetfnStorage    , eIpmiCmdDeleteSdr                  },
  { "ClearSdrRepository"        , eIpmiNetfnStorage    , eIpmiCmdClearSdrRepository         },
  { "GetSdrRepositoryTime"      , eIpmiNetfnStorage    , eIpmiCmdGetSdrRepositoryTime       },
  { "SetSdrRepositoryTime"      , eIpmiNetfnStorage    , eIpmiCmdSetSdrRepositoryTime       },
  { "EnterSdrRepositoryUpdate"  , eIpmiNetfnStorage    , eIpmiCmdEnterSdrRepositoryUpdate   },
  { "ExitSdrRepositoryUpdate"   , eIpmiNetfnStorage    , eIpmiCmdExitSdrRepositoryUpdate    },
  { "RunInitializationAgent"    , eIpmiNetfnStorage    , eIpmiCmdRunInitializationAgent     },

  { "GetSelInfo"                , eIpmiNetfnStorage    , eIpmiCmdGetSelInfo                 },
  { "GetSelAllocationInfo"      , eIpmiNetfnStorage    , eIpmiCmdGetSelAllocationInfo       },
  { "ReserveSel"                , eIpmiNetfnStorage    , eIpmiCmdReserveSel                 },
  { "GetSelEntry"               , eIpmiNetfnStorage    , eIpmiCmdGetSelEntry                },
  { "AddSelEntry"               , eIpmiNetfnStorage    , eIpmiCmdAddSelEntry                },
  { "PartialAddSelEntry"        , eIpmiNetfnStorage    , eIpmiCmdPartialAddSelEntry         },
  { "DeleteSelEntry"            , eIpmiNetfnStorage    , eIpmiCmdDeleteSelEntry             },
  { "ClearSel"                  , eIpmiNetfnStorage    , eIpmiCmdClearSel                   },
  { "GetSelTime"                , eIpmiNetfnStorage    , eIpmiCmdGetSelTime                 },
  { "SetSelTime"                , eIpmiNetfnStorage    , eIpmiCmdSetSelTime                 },
  { "GetAuxiliaryLogStatus"     , eIpmiNetfnStorage    , eIpmiCmdGetAuxiliaryLogStatus      },
  { "SetAuxiliaryLogStatus"     , eIpmiNetfnStorage    , eIpmiCmdSetAuxiliaryLogStatus      },

  // Transport netfn
  { "SetLanConfigParms"         , eIpmiNetfnTransport  , eIpmiCmdSetLanConfigParms          },
  { "GetLanConfigParms"         , eIpmiNetfnTransport  , eIpmiCmdGetLanConfigParms          },
  { "SuspendBmcArps"            , eIpmiNetfnTransport  , eIpmiCmdSuspendBmcArps             },
  { "GetIpUdpRmcpStats"         , eIpmiNetfnTransport  , eIpmiCmdGetIpUdpRmcpStats          },

  { "SetSerialModemConfig"      , eIpmiNetfnTransport  , eIpmiCmdSetSerialModemConfig       },
  { "GetSerialModemConfig"      , eIpmiNetfnTransport  , eIpmiCmdGetSerialModemConfig       },
  { "SetSerialModemMux"         , eIpmiNetfnTransport  , eIpmiCmdSetSerialModemMux          },
  { "GetTapResponseCodes"       , eIpmiNetfnTransport  , eIpmiCmdGetTapResponseCodes        },
  { "SetPppUdpProxyXmitData"    , eIpmiNetfnTransport  , eIpmiCmdSetPppUdpProxyXmitData     },
  { "GetPppUdpProxyXmitData"    , eIpmiNetfnTransport  , eIpmiCmdGetPppUdpProxyXmitData     },
  { "SendPppUdpProxyPacket"     , eIpmiNetfnTransport  , eIpmiCmdSendPppUdpProxyPacket      },
  { "GetPppUdpProxyRecvData"    , eIpmiNetfnTransport  , eIpmiCmdGetPppUdpProxyRecvData     },
  { "SerialModemConnActive"     , eIpmiNetfnTransport  , eIpmiCmdSerialModemConnActive      },
  { "Callback"                  , eIpmiNetfnTransport  , eIpmiCmdCallback                   },
  { "SetUserCallbackOptions"    , eIpmiNetfnTransport  , eIpmiCmdSetUserCallbackOptions     },
  { "GetUserCallbackOptions"    , eIpmiNetfnTransport  , eIpmiCmdGetUserCallbackOptions     },

  // PIGMG netfn
  { "GetPicMgProperties"        , eIpmiNetfnPicmg      , eIpmiCmdGetPicMgProperties         },
  { "GetAddressInfo"            , eIpmiNetfnPicmg      , eIpmiCmdGetAddressInfo             },
  { "GetShelfAddressInfo"       , eIpmiNetfnPicmg      , eIpmiCmdGetShelfAddressInfo        },
  { "SetShelfAddressInfo"       , eIpmiNetfnPicmg      , eIpmiCmdSetShelfAddressInfo        },
  { "FruControl"                , eIpmiNetfnPicmg      , eIpmiCmdFruControl                 },
  { "GetFruLedProperties"       , eIpmiNetfnPicmg      , eIpmiCmdGetFruLedProperties        },
  { "GetLedColorCapabilities"   , eIpmiNetfnPicmg      , eIpmiCmdGetLedColorCapabilities    },
  { "SetFruLedState"            , eIpmiNetfnPicmg      , eIpmiCmdSetFruLedState             },
  { "GetFruLedState"            , eIpmiNetfnPicmg      , eIpmiCmdGetFruLedState             },
  { "SetIpmbState"              , eIpmiNetfnPicmg      , eIpmiCmdSetIpmbState               },
  { "SetFruActivationPolicy"    , eIpmiNetfnPicmg      , eIpmiCmdSetFruActivationPolicy     },
  { "GetFruActivationPolicy"    , eIpmiNetfnPicmg      , eIpmiCmdGetFruActivationPolicy     },
  { "SetFruActivation"          , eIpmiNetfnPicmg      , eIpmiCmdSetFruActivation           },
  { "GetDeviceLocatorRecordId"  , eIpmiNetfnPicmg      , eIpmiCmdGetDeviceLocatorRecordId   },
  { "SetPortState"              , eIpmiNetfnPicmg      , eIpmiCmdSetPortState               },
  { "GetPortState"              , eIpmiNetfnPicmg      , eIpmiCmdGetPortState               },
  { "ComputePowerProperties"    , eIpmiNetfnPicmg      , eIpmiCmdComputePowerProperties     },
  { "SetPowerLevel"             , eIpmiNetfnPicmg      , eIpmiCmdSetPowerLevel              },
  { "GetPowerLevel"             , eIpmiNetfnPicmg      , eIpmiCmdGetPowerLevel              },
  { "RenegotiatePower"          , eIpmiNetfnPicmg      , eIpmiCmdRenegotiatePower           },
  { "GetFanSpeedProperties"     , eIpmiNetfnPicmg      , eIpmiCmdGetFanSpeedProperties      },
  { "SetFanLevel"               , eIpmiNetfnPicmg      , eIpmiCmdSetFanLevel                },
  { "GetFanLevel"               , eIpmiNetfnPicmg      , eIpmiCmdGetFanLevel                },
  { "BusedResource"             , eIpmiNetfnPicmg      , eIpmiCmdBusedResource              },
  { 0                           , eIpmiNetfnChassis    , eIpmiCmdGetChassisCapabilities     }
};


const char *
IpmiCmdToString( tIpmiNetfn netfn, tIpmiCmd cmd )
{
  for( int i = 0; cmd_class_map[i].m_name; i++ )
     {
       cIpmiCmdToClass *cc = &cmd_class_map[i];

       if ( cc->m_netfn == netfn && cc->m_cmd == cmd )
	    return cc->m_name;
     }

  return "Invalid";
}

