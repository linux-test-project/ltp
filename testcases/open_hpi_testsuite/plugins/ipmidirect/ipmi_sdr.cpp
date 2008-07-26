/*
 * ipmi_sdr.cpp
 *
 * Copyright (c) 2003,2004 by FORCE Computers
 * Copyright (c) 2005-2007 by ESO Technologies.
 *
 * Note that this file is based on parts of OpenIPMI
 * written by Corey Minyard <minyard@mvista.com>
 * of MontaVista Software. Corey's code was helpful
 * and many thanks go to him. He gave the permission
 * to use this code in OpenHPI under BSD license.
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
 *     Andy Cress        <arcress@user.sourceforge.net> 
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "ipmi_mc.h"
#include "ipmi_cmd.h"
#include "ipmi_log.h"
#include "ipmi_utils.h"
#include "ipmi_text_buffer.h"
#include "ipmi_sensor_threshold.h"


static const char *repository_sdr_update_map[] =
{
  "Unspecified",
  "NonModal",
  "Modal",
  "Both"
};

static int repository_sdr_update_num =   sizeof( repository_sdr_update_map )
                                       / sizeof( char * );

const char *
IpmiRepositorySdrUpdateToString( tIpmiRepositorySdrUpdate val )
{
  if ( (int)val >= repository_sdr_update_num )
       return "Invalid";
  
  return repository_sdr_update_map[val];
}


struct cIpmiSdrTypeToName
{
  tIpmiSdrType m_type;
  const char  *m_name;
};


static cIpmiSdrTypeToName type_to_name[] =
{
  { eSdrTypeFullSensorRecord                     , "FullSensor" },
  { eSdrTypeCompactSensorRecord                  , "CompactSensor" },
  { eSdrTypeEntityAssociationRecord              , "EntityAssociation" },
  { eSdrTypeDeviceRelativeEntityAssociationRecord, "DeviceRelativeEntityAssociation" },
  { eSdrTypeGenericDeviceLocatorRecord           , "GenericDeviceLocator" },
  { eSdrTypeFruDeviceLocatorRecord               , "FruDeviceLocator" },
  { eSdrTypeMcDeviceLocatorRecord                , "McDeviceLocator" },
  { eSdrTypeMcConfirmationRecord                 , "McConfirmation" },
  { eSdrTypeBmcMessageChannelInfoRecord          , "BmcMessageChannelInfo" },
  { eSdrTypeOemRecord                            , "Oem" },
  { eSdrTypeUnknown, 0 }
};


const char *
IpmiSdrTypeToName( tIpmiSdrType type )
{
  if ( type == eSdrTypeUnknown )
      return "Unknown";
  for( cIpmiSdrTypeToName *t = type_to_name; t->m_name; t++ )
       if ( t->m_type == type )
            return t->m_name;

  return "Invalid";
}


static bool
Bit( unsigned char v, int bit )
{
  return v & (1<<bit);
}


void 
cIpmiSdr::DumpFullSensor( cIpmiLog &dump ) const
{
  char str[256];

  dump.Entry( "SlaveAddress" ) << m_data[5] << ";\n";    
  dump.Entry( "Channel" ) << (int)(m_data[6] >> 4) << ";\n";
  dump.Entry( "Lun" ) << (int)(m_data[6] & 0x3) << ";\n";
  dump.Entry( "SensorNum" ) << m_data[7] << ";\n";

  tIpmiEntityId id = (tIpmiEntityId)m_data[8];

  if ( !strcmp( IpmiEntityIdToString( id ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", id );
  else
       snprintf( str, sizeof(str), "%s", IpmiEntityIdToString( id ) );

  dump.Entry( "EntityId" ) << str << ";\n";
  dump.Entry( "EntityInstance" ) << (int)m_data[9] << ";\n";

  dump.Entry( "InitScanning" ) << Bit( m_data[10], 6 ) << ";\n";
  dump.Entry( "InitEvents" ) << Bit( m_data[10], 5 ) << ";\n";
  dump.Entry( "InitThresholds" ) << Bit( m_data[10], 4 ) << ";\n";
  dump.Entry( "InitHysteresis" ) << Bit( m_data[10], 3 ) << ";\n";
  dump.Entry( "InitSensorType" ) << Bit( m_data[10], 2 ) << ";\n";
  dump.Entry( "SensorInitPuEvents" ) << Bit( m_data[10], 1 ) << ";\n";
  dump.Entry( "SensorInitPuScanning" ) << Bit( m_data[10], 0 ) << ";\n";
  dump.Entry( "IgnoreIfNoEntity" ) << Bit( m_data[11], 7 ) << ";\n";
  dump.Entry( "SupportsAutoRearm" ) << Bit( m_data[11], 6 ) << ";\n";

  tIpmiHysteresisSupport hs = (tIpmiHysteresisSupport)((m_data[11] >> 4) & 3);
  dump.Entry( "HysteresisSupport" ) << IpmiHysteresisSupportToString( hs ) << ";\n";

  tIpmiThresholdAccessSuport ts = (tIpmiThresholdAccessSuport)((m_data[11] >> 2) & 3);
  dump.Entry( "ThresholdAccess" ) << IpmiThresholdAccessSupportToString( ts ) << ";\n";

  tIpmiEventSupport es = (tIpmiEventSupport)(m_data[11] & 3);
  dump.Entry( "EventSupport" ) << IpmiEventSupportToString( es ) <<  ";\n";

  tIpmiSensorType sensor_type = (tIpmiSensorType)m_data[12];

  if ( !strcmp( IpmiSensorTypeToString( sensor_type ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", sensor_type );
  else
       snprintf( str, sizeof(str), "%s", IpmiSensorTypeToString( sensor_type ) );

  dump.Entry( "SensorType" ) << str << ";\n";

  tIpmiEventReadingType reading_type = (tIpmiEventReadingType)m_data[13];

  if ( !strcmp( IpmiEventReadingTypeToString( reading_type ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", reading_type );
  else
       snprintf( str, sizeof(str), "%s", IpmiEventReadingTypeToString( reading_type ) );

  dump.Entry( "EventReadingType" ) << str << ";\n";

  if ( reading_type == eIpmiEventReadingTypeThreshold )
     {
       // assertion
       unsigned short em = IpmiGetUint16( m_data + 14 );
       IpmiThresholdEventMaskToString( em, str );

       if ( str[0] == 0 )
            strcat( str, "0" );

       dump.Entry( "AssertionEventMask" ) << str << ";\n";

       snprintf( str, sizeof(str), "0x%04x", em >> 12 );
       dump.Entry( "LowerThresholdReadingMask" ) << str << ";\n";

       // deassertion
       em = IpmiGetUint16( m_data + 16 );
       IpmiThresholdEventMaskToString( em, str );

       if ( str[0] == 0 )
            strcat( str, "0" );

       dump.Entry( "DeassertionEventMask" ) << str << ";\n";

       snprintf( str, sizeof(str), "0x%04x", em >> 12 );
       dump.Entry( "UpperThresholdReadingMask" ) << str << ";\n";

       // settable threshold
       em = IpmiGetUint16( m_data + 18 );

       IpmiThresholdMaskToString( em >> 8, str );

       if ( str[0] == 0 )
            strcat( str, "0" );

       dump.Entry( "SettableThresholdsMask" ) << str <<  ";\n";

       IpmiThresholdMaskToString( em & 0xff, str );

       if ( str[0] == 0 )
            strcat( str, "0" );

       dump.Entry( "ReadableThresholdsMask" ) << str << ";\n";

       tIpmiRateUnit ru = (tIpmiRateUnit)((m_data[20] >> 3) & 7);
       dump.Entry( "RateUnit" ) << IpmiRateUnitToString( ru ) << ";\n";

       tIpmiModifierUnit mu = (tIpmiModifierUnit)( (m_data[20] >> 1) & 3);
       dump.Entry( "ModifierUnit" ) << IpmiModifierUnitToString( mu ) << ";\n";
       dump.Entry( "Percentage" ) << ((m_data[20] & 1) == 1) << ";\n";

       dump.Entry( "BaseUnit" ) << IpmiUnitTypeToString( (tIpmiUnitType)m_data[21] ) << ";\n";
       dump.Entry( "ModifierUnit2" ) << IpmiUnitTypeToString( (tIpmiUnitType)m_data[22] ) << ";\n";

       cIpmiSensorFactors sf;
       sf.GetDataFromSdr( this );

       dump.Entry( "AnalogDataFormat" ) << IpmiAnalogeDataFormatToString( sf.AnalogDataFormat() ) << ";\n";

       dump.Entry( "Linearization" ) << IpmiLinearizationToString( sf.Linearization() ) << ";\n";

       dump.Entry( "M" ) << sf.M() << ";\n";
       dump.Entry( "Tolerance" ) << sf.Tolerance() << ";\n";
       dump.Entry( "B" ) << sf.B() << ";\n";
       dump.Entry( "Accuracy" ) << sf.Accuracy() << ";\n";
       dump.Entry( "AccuracyExp" ) << sf.AccuracyExp() << ";\n";
       dump.Entry( "RExp" ) << sf.RExp() << ";\n";
       dump.Entry( "BExp" ) << sf.BExp() << ";\n";

       bool v = m_data[30] & 1;
       dump.Entry( "NominalReadingSpecified" ) << v << ";\n";

       if ( v )
            dump.Entry( "NominalReading" ) << m_data[31] << ";\n";

       v = m_data[30] & 2;
       dump.Entry( "NormalMaxSpecified" ) << v << ";\n";

       if ( v )
            dump.Entry( "NormalMax" ) << m_data[32] << ";\n";

       v = m_data[30] & 4;
       dump.Entry( "NormalMinSpecified" ) << v << ";\n";

       if ( v )
            dump.Entry( "NormalMin" ) << m_data[33] << ";\n";

       dump.Entry( "SensorMax" ) << m_data[34] << ";\n";
       dump.Entry( "SensorMin" ) << m_data[35] << ";\n";
       
       dump.Entry( "UpperNonRecoverableThreshold" ) << m_data[36] << ";\n";
       dump.Entry( "UpperCriticalThreshold" ) << m_data[37] << ";\n";
       dump.Entry( "UpperNonCriticalThreshold" ) << m_data[38] << ";\n";

       dump.Entry( "LowerNonRecoverableThreshold" ) << m_data[39] << ";\n";
       dump.Entry( "LowerCriticalThreshold" ) << m_data[40] << ";\n";
       dump.Entry( "LowerNonCriticalThreshold" ) << m_data[41] << ";\n";
       
       dump.Entry( "PositiveGoingThresholdHysteresis" ) << m_data[42] << ";\n";
       dump.Entry( "NegativeGoingThresholdHysteresis" ) << m_data[43] << ";\n";
     }
  else
     {
       // assertion
       unsigned short em = IpmiGetUint16( m_data + 14 );
       dump.Hex( true );
       dump.Entry( "AssertionEventMask" ) << em << ";\n";

       // deassertion
       em = IpmiGetUint16( m_data + 16 );
       dump.Entry( "DeassertionEventMask" ) << em << ";\n";

       // event mask
       em = IpmiGetUint16( m_data + 18 );
       dump.Entry( "DiscreteReadingMask" ) << em << ";\n";

       dump.Hex( false );
     }

  dump.Entry( "Oem" ) << m_data[46] << ";\n";

  cIpmiTextBuffer tb;
  tb.SetIpmi( m_data + 47 );
  tb.GetAscii( str, 80 );
  dump.Entry( "Id" ) << "\"" << str << "\";\n";
}


void
cIpmiSdr::DumpFruDeviceLocator( cIpmiLog &dump ) const
{
  dump.Entry( "DeviceAccessAddress" ) << m_data[5] << ";\n";

  if ( m_data[7] & 0x80 )
     {
       // logical FRU device
       dump.Entry( "FruDeviceId" ) << (int)m_data[6] << ";\n";
     }
  else
     {
       // device is directly on IPMB
       dump.Entry( "SlaveAddress" ) << m_data[6] << ";\n";
       dump.Entry( "Lun" ) << (int)((m_data[7] >> 3) & 3) << ";\n";
     }

  dump.Entry( "LogicalDevice" ) << Bit(m_data[7], 7 ) << ";\n";
  dump.Entry( "Channel" ) << (int)(m_data[8] >> 4) << ";\n";
  dump.Entry( "DeviceType" ) << m_data[10] << ";\n";
  dump.Entry( "DeviceTypeModifier" ) << m_data[11] << ";\n";

  tIpmiEntityId id = (tIpmiEntityId)m_data[12];
  char str[80];

  if ( !strcmp( IpmiEntityIdToString( id ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", id );
  else
       snprintf( str, sizeof(str), "%s", IpmiEntityIdToString( id ) );

  dump.Entry( "EntityId" ) << str << ";\n";
  dump.Entry( "EntityInstance" ) << (int)m_data[13] << ";\n";

  dump.Entry( "Oem" ) << m_data[14] << ";\n";

  cIpmiTextBuffer tb;
  tb.SetIpmi( m_data + 15 );
  tb.GetAscii( str, 80 );
  dump.Entry( "Id" ) << "\"" << str << "\";\n";
}


void 
cIpmiSdr::DumpMcDeviceLocator( cIpmiLog &dump ) const
{
  dump.Entry( "SlaveAddress" ) << m_data[5] << ";\n";
  dump.Entry( "Channel" ) << (int)(m_data[6] & 0x0f) << ";\n";

  dump.Entry( "AcpiSystemPower" ) << Bit( m_data[7], 7 ) << ";\n";
  dump.Entry( "AcpiDevicePower" ) << Bit( m_data[7], 6 ) << ";\n";

  dump.Entry( "ControllerLogInitAgentErrors" ) << Bit( m_data[7], 3 ) << ";\n";
  dump.Entry( "LogInitializationAgentError" ) << Bit( m_data[7], 2 ) << ";\n";
  dump.Entry( "EventMessageGeneration" ) << ( m_data[7] & 3 ) << ";\n";
  dump.Entry( "ChassisSupport" ) << Bit( m_data[8], 7 ) << ";\n";
  dump.Entry( "BridgeSupport" ) << Bit( m_data[8], 6 ) << ";\n";
  dump.Entry( "IpmbEventGeneratorSupport" ) << Bit( m_data[8], 5 ) << ";\n";
  dump.Entry( "IpmbEventReceiverSupport" ) << Bit( m_data[8], 4 ) << ";\n";
  dump.Entry( "FruInventorySupport" ) << Bit( m_data[8], 3 ) << ";\n";
  dump.Entry( "SelDeviceSupport" ) << Bit( m_data[8], 2 ) << ";\n";
  dump.Entry( "SdrRepositorySupport" ) << Bit( m_data[8], 1 ) << ";\n";
  dump.Entry( "SensorDeviceSupport" ) << Bit( m_data[8], 0 ) << ";\n";

  tIpmiEntityId id = (tIpmiEntityId)m_data[12];
  char str[80];

  if ( !strcmp( IpmiEntityIdToString( id ), "Invalid" ) )
       snprintf( str, sizeof(str), "0x%02x", id );
  else
       snprintf( str, sizeof(str), "%s", IpmiEntityIdToString( id ) );

  dump.Entry( "EntityId" ) << str << ";\n";
  dump.Entry( "EntityInstance" ) << (int)m_data[13] << ";\n";

  dump.Entry( "Oem" ) << m_data[14] << ";\n";

  cIpmiTextBuffer tb;
  tb.SetIpmi( m_data + 15 );
  tb.GetAscii( str, 80 );
  dump.Entry( "Id" ) << "\"" << str << "\";\n";
}


void
cIpmiSdr::Dump( cIpmiLog &dump, const char *name ) const
{
  char str[80];
  snprintf( str, sizeof(str), "%sRecord", IpmiSdrTypeToName( m_type ) );
  dump.Begin( str, name );
  dump.Entry( "Type" ) << IpmiSdrTypeToName( m_type ) << "\n";
  dump.Entry( "RecordId" ) << m_record_id << ";\n";
  dump.Entry( "Version" ) << (int)m_major_version << ", "
			  << (int)m_minor_version << ";\n";

  switch( m_type )
     {
       case eSdrTypeFullSensorRecord:
            DumpFullSensor( dump );
            break;

       case eSdrTypeFruDeviceLocatorRecord:
            DumpFruDeviceLocator( dump );
            break;

       case eSdrTypeMcDeviceLocatorRecord:
            DumpMcDeviceLocator( dump );
            break;

       default:
            dump.Entry( "SDR Type " ) << m_type << ";\n";
            break;
     }

  dump.End();
}


static void
IpmiSdrDestroyRecords( cIpmiSdr **&sdr, unsigned int &n )
{
  if ( sdr == 0 )
       return;

  for( unsigned int i = 0; i < n; i++ )
     {
       assert( sdr[i] );
       delete sdr[i];
     }

  delete [] sdr;

  n = 0;
  sdr = 0;
}


cIpmiSdrs::cIpmiSdrs( cIpmiMc *mc, bool device_sdr )
  : m_mc( mc ), m_device_sdr( device_sdr ),
    m_fetched( false ), m_major_version( 0 ), m_minor_version( 0 ),
    m_last_addition_timestamp( 0 ), m_last_erase_timestamp( 0 ),
    m_overflow( 0 ), m_update_mode( eIpmiRepositorySdrUpdateUnspecified ),
    m_supports_delete_sdr( false ), m_supports_partial_add_sdr( false ),
    m_supports_reserve_sdr( false ), m_supports_get_sdr_repository_allocation( false ),
    m_reservation( 0 ), m_sdr_changed( false ),
    m_num_sdrs( 0 ), m_sdrs( 0 )
{
  for( int i = 0; i < 4; i++ )
       m_lun_has_sensors[i] = false;
}


cIpmiSdrs::~cIpmiSdrs()
{
  if ( m_sdrs )
       IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );
}


cIpmiSdr *
cIpmiSdrs::ReadRecord( unsigned short record_id,
                       unsigned short &next_record_id,
                       tReadRecord &err, unsigned int lun )
{
  cIpmiMsg       msg;
  cIpmiMsg       rsp;
  SaErrorT       rv;
  int            offset = 0;
  unsigned char  data[dMaxSdrData];
  int            record_size = 0;
  int            read_len = 0;
  cIpmiSdr      *sdr;

  memset( data, 0xaa, dMaxSdrData );

  do
     {
       if ( m_device_sdr )
          { 
            msg.m_netfn = eIpmiNetfnSensorEvent;
            msg.m_cmd   = eIpmiCmdGetDeviceSdr;
          }
       else
          {
            msg.m_netfn = eIpmiNetfnStorage;
            msg.m_cmd   = eIpmiCmdGetSdr;
          }

       msg.m_data_len = 6;
       IpmiSetUint16( msg.m_data, m_reservation );
       IpmiSetUint16( msg.m_data + 2, record_id );

       msg.m_data[4] = offset;

       if ( offset == 0 )
            read_len = dSdrHeaderSize;
       else
          {
            read_len = record_size - offset;

            if ( read_len > dMaxSdrFetch )
                 read_len = dMaxSdrFetch;
          }

       msg.m_data[5] = read_len;

       rv = m_mc->SendCommand( msg, rsp, lun );

       if ( rv != SA_OK )
          {
            stdlog << "initial_sdr_fetch: Couldn't send GetSdr or GetDeviveSdr fetch: " << rv << " !\n";
            err = eReadError;

            return 0;
          }

       if ( rsp.m_data[0] == 0x80 )
          {
            // Data changed during fetch, retry.  Only do this so many
            // times before giving up.
            stdlog << "SDR reservation lost 1.\n";

            err = eReadReservationLost;

            return 0;
          }

       if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
          {
            stdlog << "SDR reservation lost 2.\n";

            err = eReadReservationLost;
            return 0;
          }

       if (    record_id == 0 
            && (    (rsp.m_data[0] == eIpmiCcUnknownErr )
	         || (rsp.m_data[0] == eIpmiCcNotPresent ) ) )
          {
            // We got an error fetching the first SDR, so the repository is
            // probably empty.  Just go on.
            stdlog << "SDR reservation lost 3.\n";

            err = eReadEndOfSdr;
            return 0;
          }
 
       if ( rsp.m_data[0] != eIpmiCcOk )
          {
            stdlog << "SDR fetch error getting sdr " << record_id << ": "
                   << rsp.m_data[0] << " !\n";

            err = eReadError;
            return 0;
          }

       if ( rsp.m_data_len != read_len + 3 )
          {
            stdlog << "Got an invalid amount of SDR data: " << rsp.m_data_len 
                   << ", expected " << read_len + 3 << " !\n";

            err = eReadError;
            return 0;
          }

       // copy the data
       memcpy( data + offset, rsp.m_data + 3, read_len );

       // header => get record size
       if ( offset == 0 )
          {
            record_size = rsp.m_data[7] + dSdrHeaderSize;
            next_record_id = IpmiGetUint16( rsp.m_data + 1 );
          }

       offset += read_len;
     }
  while( offset < record_size );

  // create sdr
  sdr = new cIpmiSdr;
  memset( sdr, 0, sizeof( cIpmiSdr ));

  sdr->m_record_id     = IpmiGetUint16( data );
  sdr->m_major_version = data[2] & 0xf;
  sdr->m_minor_version = (data[2] >> 4) & 0xf;
  sdr->m_type          = (tIpmiSdrType)data[3];  

  // Hack to support 1.0 MCs
  if (   (sdr->m_major_version == 1)
      && (sdr->m_minor_version == 0)
      && (sdr->m_type == eSdrTypeMcDeviceLocatorRecord))
  {
      data[8] = data[7];
      data[7] = data[6];
      data[6] = 0;
  }

  sdr->m_length        = record_size;
  memcpy( sdr->m_data, data, record_size );

  err = eReadOk;

  return sdr;
}


SaErrorT
cIpmiSdrs::Reserve(unsigned int lun)
{
  cIpmiMsg msg;
  cIpmiMsg rsp;
  SaErrorT rv;

  if ( !m_supports_reserve_sdr )
  {
    stdlog << "cIpmiSdrs::Reserve: Reserve SDR not supported\n";

    return SA_ERR_HPI_INTERNAL_ERROR;
  }

  // Now get the reservation.
  if ( m_device_sdr )
     {
       msg.m_netfn = eIpmiNetfnSensorEvent;
       msg.m_cmd   = eIpmiCmdReserveDeviceSdrRepository;
     }
  else
     {
       msg.m_netfn = eIpmiNetfnStorage;
       msg.m_cmd   = eIpmiCmdReserveSdrRepository;
     }

  msg.m_data_len = 0;
  rv = m_mc->SendCommand( msg, rsp, lun );

  if ( rv != SA_OK )
     {
       stdlog << "Couldn't send SDR reservation: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] != 0) 
     {
       if (    m_device_sdr
            && rsp.m_data[0] == eIpmiCcInvalidCmd )
          {
	    // This is a special case.  We always attempt a
            // reservation with a device SDR (since there is nothing
            // telling us if this is supported), if it fails then we
            // just go on without the reservation.
	    m_supports_reserve_sdr = false;
	    m_reservation = 0;

            return SA_OK;
	}

       stdlog << "Error getting SDR fetch reservation: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_PARAMS;
     }

  if ( rsp.m_data_len < 3 )
     {
       stdlog << "SDR Reservation data not long enough: " << rsp.m_data_len << " bytes!\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  m_reservation = IpmiGetUint16( rsp.m_data + 1 );

  return SA_OK;
}


SaErrorT
cIpmiSdrs::GetInfo( unsigned short &working_num_sdrs )
{
  SaErrorT rv;
  cIpmiMsg msg;
  cIpmiMsg rsp;
  unsigned int add_timestamp;
  unsigned int erase_timestamp;

  // sdr info
  if ( m_device_sdr )
     {
       msg.m_netfn = eIpmiNetfnSensorEvent;
       msg.m_cmd   = eIpmiCmdGetDeviceSdrInfo;
     }
  else
     {
       msg.m_netfn = eIpmiNetfnStorage;
       msg.m_cmd   = eIpmiCmdGetSdrRepositoryInfo;
     }

  msg.m_data_len = 0;

  rv = m_mc->SendCommand( msg, rsp );

  if ( rv != SA_OK )
     {
       stdlog << "IpmiSdrsFetch: GetDeviceSdrInfoCmd or GetSdrRepositoryInfoCmd "
              << rv << ", " << strerror( rv ) << " !\n";

       m_sdr_changed = true;
       IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
  	if ( m_device_sdr == false )
           {
             // The device doesn't support the get device SDR info
             // command, so just assume some defaults.
             working_num_sdrs     = 0xfffe;
             m_dynamic_population = false;

             // Assume it uses reservations, if the reservation returns
             // an error, then say that it doesn't.
             m_supports_reserve_sdr = true;

             m_lun_has_sensors[0] = true;
             m_lun_has_sensors[1] = false;
             m_lun_has_sensors[2] = false;
             m_lun_has_sensors[3] = false;

             add_timestamp   = 0;
             erase_timestamp = 0;
           }
        else
           {
             stdlog << "IPMI Error getting SDR info: " << rsp.m_data[0] << " !\n";

             m_sdr_changed = true;
             IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

             return SA_ERR_HPI_INVALID_PARAMS;
           }
     }
  else if ( m_device_sdr )
     {
       // device SDR

       if ( rsp.m_data_len < 3 ) 
          {
	    stdlog << "SDR info is not long enough !\n";

            m_sdr_changed = true;
            IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

	    return SA_ERR_HPI_INVALID_DATA;
	}

       working_num_sdrs = rsp.m_data[1];
       m_dynamic_population = (rsp.m_data[2] & 0x80) == 0x80;

       // Assume it uses reservations, if the reservation returns
       // an error, then say that it doesn't.
       m_supports_reserve_sdr = true;

       m_lun_has_sensors[0] = (rsp.m_data[2] & 0x01) == 0x01;
       m_lun_has_sensors[1] = (rsp.m_data[2] & 0x02) == 0x02;
       m_lun_has_sensors[2] = (rsp.m_data[2] & 0x04) == 0x04;
       m_lun_has_sensors[3] = (rsp.m_data[2] & 0x08) == 0x08;

       if ( m_dynamic_population )
          {
	    if ( rsp.m_data_len < 7 )
               {
                 stdlog << "SDR info is not long enough !\n";

                 m_sdr_changed = 1;
                 IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

                 return SA_ERR_HPI_INVALID_DATA;
               }

	    add_timestamp = IpmiGetUint32( rsp.m_data + 3 );
          }
       else
	    add_timestamp = 0;

       erase_timestamp = 0;
     }
  else
     {
       // repository SDR

       if ( rsp.m_data_len < 15 )
          {
	    stdlog << "SDR info is not long enough\n";

            m_sdr_changed = true;
            IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

	    return SA_ERR_HPI_INVALID_DATA;
          }

       /* Pull pertinant info from the response. */
       m_major_version = rsp.m_data[1] & 0xf;
       m_minor_version = (rsp.m_data[1] >> 4) & 0xf;
       working_num_sdrs = IpmiGetUint16( rsp.m_data + 2 );

       m_overflow                 = (rsp.m_data[14] & 0x80) == 0x80;
       m_update_mode              = (tIpmiRepositorySdrUpdate)((rsp.m_data[14] >> 5) & 0x3);
       m_supports_delete_sdr      = (rsp.m_data[14] & 0x08) == 0x08;
       m_supports_partial_add_sdr = (rsp.m_data[14] & 0x04) == 0x04;
       m_supports_reserve_sdr     = (rsp.m_data[14] & 0x02) == 0x02;
       m_supports_get_sdr_repository_allocation
         = (rsp.m_data[14] & 0x01) == 0x01;

       add_timestamp   = IpmiGetUint32( rsp.m_data + 6 );
       erase_timestamp = IpmiGetUint32( rsp.m_data + 10 );
     }

  // If the timestamps still match, no need to re-fetch the repository
  if (      m_fetched
       && ( add_timestamp   == m_last_addition_timestamp )
       && ( erase_timestamp == m_last_erase_timestamp ) )
      return -1; 

  m_last_addition_timestamp = add_timestamp;
  m_last_erase_timestamp    = erase_timestamp;

  return SA_OK;
}


SaErrorT
cIpmiSdrs::ReadRecords( cIpmiSdr **&records, unsigned short &working_num_sdrs,
			unsigned int &num, unsigned int lun )
{
  int retry_count = 0;
  bool loop = true;
  unsigned short save_working = working_num_sdrs;
  unsigned int save_num = num;
  struct timespec ts;

  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  while( loop )
     {
       unsigned short next_record_id = 0;

       working_num_sdrs = save_working;
       num = save_num;

       if ( retry_count++ == dMaxSdrFetchRetries )
          {
            stdlog << "Too many retries trying to fetch SDRs\n";

            return SA_ERR_HPI_BUSY;
          }

       SaErrorT rv = Reserve(lun);

       if ( rv )
           return rv;

       // read sdr records
       while( 1 )
          {
            tReadRecord err;
            unsigned short record_id = next_record_id;

            cIpmiSdr *sdr = ReadRecord( record_id, next_record_id, err, lun );

            if ( sdr == 0 )
               {
                 if ( err == eReadReservationLost )
                 {
                     stdlog << "MC " << (unsigned char)m_mc->GetAddress() << " Lost SDR reservation " << retry_count << " - sleeping\n";
                     // Most likely the Shelf Manager is trying to access the device SDR too
                     // Give him a break and wait till it's done
                     ts.tv_sec = 5+2*retry_count;
                     nanosleep(&ts, NULL);
                     break;
                 }

                 if ( err != eReadEndOfSdr )
                      return SA_ERR_HPI_BUSY;

                 // SDR is empty
                 loop = false;

                 break;
               }

	    GList *list = 0;

	    if ( sdr->m_type == eSdrTypeCompactSensorRecord )
	       {
		 // convert compact sensor record to full sensor records
		 list = CreateFullSensorRecords( sdr );
		 delete sdr;
	       }
	    else
		 list = g_list_append( 0, sdr );

	    while( list )
	       {
		 sdr = (cIpmiSdr *)list->data;
		 list = g_list_remove( list, sdr );

		 sdr->Dump( stdlog, "sdr" );

		 if ( num >= working_num_sdrs )
		    {
		      // resize records
		      cIpmiSdr **rec = new cIpmiSdr*[ working_num_sdrs + 10 ];
		      memcpy( rec, records, sizeof( cIpmiSdr * ) * working_num_sdrs );

		      delete [] records;
		      records = rec;
		      working_num_sdrs += 10;
		    }

		 records[num++] = sdr;
	       }

            if ( next_record_id == 0xffff )
               {
                 // finish
                 loop = false;

                 break;
               }
          }
     }

  return SA_OK;
}


GList *
cIpmiSdrs::CreateFullSensorRecords( cIpmiSdr *sdr )
{
  int n = 1;

  if ( sdr->m_data[23] & 0x0f )
       n = sdr->m_data[23] & 0x0f;

  GList *list = 0;

  for( int i = 0; i < n; i++ )
     {
       cIpmiSdr *s = new cIpmiSdr;
       *s = *sdr;
       s->m_type = eSdrTypeFullSensorRecord;

       memset( s->m_data + 23, 0, dMaxSdrData - 23 );

       // sensor num
       s->m_data[7] = sdr->m_data[7] + i;

       // entity instance
       if ( sdr->m_data[24] & 0x80 )
            s->m_data[9] = sdr->m_data[9] + i;

       // positive-going threshold hysteresis value
       s->m_data[42] = sdr->m_data[25];
       // negativ-going threshold hysteresis value
       s->m_data[43] = sdr->m_data[26];

       // oem
       s->m_data[46] = sdr->m_data[30];

       // id
       int len = sdr->m_data[31] & 0x3f;
       int val = (sdr->m_data[24] & 0x7f) + i;

       memcpy( s->m_data + 47, sdr->m_data + 31, len + 1 );

       if (n > 1)
       {
            int base  = 0;
            int start = 0;
        
            if (( sdr->m_data[23] & 0x30 ) == 0 )
                {
                    // numeric
                    base  = 10;
                    start = '0';
                }
            else if (( sdr->m_data[23] & 0x30 ) == 0x10 )
                {
                    // alpha
                    base  = 26;
                    start = 'A';
                }
        
            if ( base )
                {
                    // add id string postfix
                    if ( val / base > 0 )
                    {
                        s->m_data[48+len] = (val / base) + start;
                        len++;
                    }
        
                    s->m_data[48+len] = (val % base) + start;
                    len++;
                    s->m_data[48+len] = 0;
                    s->m_data[47] = (sdr->m_data[31] & 0xc0) | len;
                }
       }

       list = g_list_append( list, s );
     }

  return list;
}


SaErrorT
cIpmiSdrs::Fetch()
{
  SaErrorT rv;
  cIpmiSdr **records;
  unsigned short working_num_sdrs;

  m_sdr_changed = false;

  assert( m_mc );

  if ( m_device_sdr )
     {
       m_device_sdr = m_mc->ProvidesDeviceSdrs();  /* added ARCress 09/21/06 */
       // if ( !m_mc->ProvidesDeviceSdrs() ) return SA_ERR_HPI_NOT_PRESENT;
     }
  else if ( !m_mc->SdrRepositorySupport() )
       return SA_ERR_HPI_NOT_PRESENT;

  // working num sdrs is just an estimation
  rv = GetInfo( working_num_sdrs );

  // sdr records are up to date
  if ( rv == -1 )
       return SA_OK;

  if ( rv )
       return rv;

  m_sdr_changed = true;
  IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

  // because working_num_sdrs is an estimation
  // read the sdr to get the real number
  if ( working_num_sdrs == 0 )
       working_num_sdrs = 1;

  // read sdr
  unsigned int num = 0;

  records = new cIpmiSdr *[working_num_sdrs];

  rv = SA_OK;

  if ( m_device_sdr )
     {
       for( unsigned int i = 0; rv == SA_OK && i < 4; i++ )
	    if ( m_lun_has_sensors[i] )
		 rv = ReadRecords( records, working_num_sdrs, num, i );
     }
  else
       rv = ReadRecords( records, working_num_sdrs, num, 0 );

  if ( rv != SA_OK )
     {
       IpmiSdrDestroyRecords( records, num );
       return rv;
     }

  if ( num == 0 )
     {
       delete [] records;
       m_sdrs = 0;
       m_num_sdrs = 0;

       return SA_OK;
     }

  if ( num == working_num_sdrs )
     {
       m_sdrs = records;
       m_num_sdrs = working_num_sdrs;

       return SA_OK;
     }

  m_sdrs = new cIpmiSdr *[num];
  memcpy( m_sdrs, records, num * sizeof( cIpmiSdr * ) );
  m_num_sdrs = num;

  delete [] records;

  return SA_OK;
}


void
cIpmiSdrs::Dump( cIpmiLog &dump, const char *name ) const
{
  unsigned int i;
  char str[80];

  if ( dump.IsRecursive() )
     {
       for( i = 0; i < m_num_sdrs; i++ )
	  {
	    snprintf( str, sizeof(str), "Sdr%02x_%d", m_mc->GetAddress(), i );
	    m_sdrs[i]->Dump( dump, str );
	  }
     }

  dump.Begin( "Sdr", name );

  if ( m_device_sdr )
     {
       dump.Entry( "DynamicPopulation" ) << m_dynamic_population << ";\n";
       dump.Entry( "LunHasSensors" )
            << m_lun_has_sensors[0] << ", "
            << m_lun_has_sensors[1] << ", "
            << m_lun_has_sensors[2] << ", "
            << m_lun_has_sensors[3] << ";\n";
     }
  else
     {
       dump.Entry( "Version" ) << m_major_version << ", "
                                                 << m_minor_version << ";\n";
       dump.Entry( "Overflow" ) << m_overflow << ";\n";
       dump.Entry( "UpdateMode" ) << "dMainSdrUpdate"
            << IpmiRepositorySdrUpdateToString( m_update_mode ) << ";\n";
       dump.Entry( "SupportsDeleteSdr" ) << m_supports_delete_sdr << ";\n";
       dump.Entry( "SupportsPartialAddSdr" ) << m_supports_partial_add_sdr << ";\n";
       dump.Entry( "SupportsReserveSdr" ) << m_supports_reserve_sdr << ";\n";
       dump.Entry( "SupportsGetSdrRepositoryAllocation" ) << m_supports_get_sdr_repository_allocation << ";\n";
     }

  if ( dump.IsRecursive() && m_num_sdrs )
     {
       dump.Entry( "Sdr" );

       for( i = 0; i < m_num_sdrs; i++ )
          {
            if ( i != 0 )
                 dump << ", ";

            snprintf( str, sizeof(str), "Sdr%02x_%d", m_mc->GetAddress(), i );
            dump << str;
          }

       dump << ";\n";
     }

  dump.End();
}


cIpmiSdr *
cIpmiSdrs::FindSdr( cIpmiMc *mc )
{
  for( unsigned int i = 0; i < NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = Sdr( i );

       if ( sdr->m_type != eSdrTypeMcDeviceLocatorRecord )
            continue;

       if (    mc->GetAddress() == (unsigned int)sdr->m_data[5] 
            && mc->GetChannel() == (unsigned int)(sdr->m_data[6] & 0x0f) )
            return sdr;
     }

  return 0;
}

// Here we try to find the FRU that is the "parent"
// of the entity this SDR is attached to
// The algorithm here is *not* sophisticated at all
// The assumption is that most of the SDR will be attached
// directly to a FRU so it was optimized for this case
// Also only one level of parenthood is assumed - sorry Grand Pa !
unsigned int
cIpmiSdrs::FindParentFru( SaHpiEntityTypeT type,
                          SaHpiEntityLocationT instance,
                          SaHpiEntityTypeT & parent_type,
                          SaHpiEntityLocationT & parent_instance
                        )
{
  SaHpiEntityTypeT mc_type = SAHPI_ENT_UNSPECIFIED;
  SaHpiEntityLocationT mc_instance = 0;

  parent_type = SAHPI_ENT_UNSPECIFIED;
  parent_instance = 0;

  // First look for FRUs themselves
  for( unsigned int i = 0; i < NumSdrs(); i++ )
    {
       cIpmiSdr *sdr = Sdr( i );

       if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
        {
            mc_type = (SaHpiEntityTypeT)sdr->m_data[12];
            mc_instance = (SaHpiEntityLocationT)sdr->m_data[13];

            if ( ( type != (SaHpiEntityTypeT)sdr->m_data[12] )
                || ( instance != (SaHpiEntityLocationT)sdr->m_data[13] ) )

                continue;

            parent_type = type;
            parent_instance = instance;

            return 0;
        }
       else if ( sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
        {
            if ( (( sdr->m_data[7] & 0x80) == 0 )
                || ( type != (SaHpiEntityTypeT)sdr->m_data[12] )
                || ( instance != (SaHpiEntityLocationT)sdr->m_data[13] ) )

                continue;

            parent_type = type;
            parent_instance = instance;

            return sdr->m_data[6];
       }
    }

  stdlog << "Entity ID " << type << ", Instance " << instance << " is not a FRU\n";

  // SDR entity is not a FRU: look for association records
  for( unsigned int i = 0; i < NumSdrs(); i++ )
    {
       cIpmiSdr *sdr = Sdr( i );

       if ( sdr->m_type == eSdrTypeEntityAssociationRecord )
       {
           // Entity range
           if ( sdr->m_data[7] & 0x80 )
           {
               if (( type == (SaHpiEntityTypeT)sdr->m_data[8] )
                   && ( type == (SaHpiEntityTypeT)sdr->m_data[10] )
                   && ( ( instance >= (SaHpiEntityLocationT)sdr->m_data[9]  )
                     && ( instance <= (SaHpiEntityLocationT)sdr->m_data[11] )))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
               if (( type == (SaHpiEntityTypeT)sdr->m_data[12] )
                   && ( type == (SaHpiEntityTypeT)sdr->m_data[14] )
                   && ( ( instance >= (SaHpiEntityLocationT)sdr->m_data[13]  )
                     && ( instance <= (SaHpiEntityLocationT)sdr->m_data[15] )))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
           }
           // Entity list
           else
           {
               if (( type == (SaHpiEntityTypeT)sdr->m_data[8] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[9] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[10] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[11] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[12] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[13] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[14] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[15] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
           }
       }
       else if ( sdr->m_type == eSdrTypeDeviceRelativeEntityAssociationRecord )
       {
           // Entity range
           if ( sdr->m_data[9] & 0x80 )
           {
               if (( type == (SaHpiEntityTypeT)sdr->m_data[12] )
                   && ( type == (SaHpiEntityTypeT)sdr->m_data[16] )
                   && ( ( instance >= (SaHpiEntityLocationT)sdr->m_data[13]  )
                     && ( instance <= (SaHpiEntityLocationT)sdr->m_data[17] )))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
               if (( type == (SaHpiEntityTypeT)sdr->m_data[20] )
                   && ( type == (SaHpiEntityTypeT)sdr->m_data[24] )
                   && ( ( instance >= (SaHpiEntityLocationT)sdr->m_data[21]  )
                     && ( instance <= (SaHpiEntityLocationT)sdr->m_data[25] )))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
           }
           // Entity list
           else
           {
               if (( type == (SaHpiEntityTypeT)sdr->m_data[12] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[13] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[16] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[17] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[20] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[21] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }

               if (( type == (SaHpiEntityTypeT)sdr->m_data[24] )
                   && ( instance == (SaHpiEntityLocationT)sdr->m_data[25] ))
               {
                   parent_type = (SaHpiEntityTypeT)sdr->m_data[5];
                   parent_instance = (SaHpiEntityLocationT)sdr->m_data[6];
                   break;
               }
           }
       }
    }

  // Didn't find proper association record
  if ( parent_type == SAHPI_ENT_UNSPECIFIED )
  {
      stdlog << "WARNING : Entity ID " << type << ", Instance " << instance << " did not find parent FRU\n";
      stdlog << "WARNING : Defaulting to FRU 0, Entity ID " << mc_type << ", Instance " << mc_instance << "\n";

      // We didn't find an exact match
      // Since a lot of ATCA boards have wrong SDRs
      // we default the parent to the MC itself
      parent_type = mc_type;
      parent_instance = mc_instance;

      return 0;
  }

  stdlog << "Entity ID " << type << ", Instance " << instance << " parent ID " << parent_type << ", Instance " << parent_instance << "\n";

  // We found the parent now we want its FRU number
  // We already have MC == FRU #0 data
  if (( parent_type == mc_type )
      && ( parent_instance == mc_instance ))
      return 0;

  // Now look for FRUs other than #0
  for( unsigned int i = 0; i < NumSdrs(); i++ )
    {
       cIpmiSdr *sdr = Sdr( i );

       if ( sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
        {
            if ( (( sdr->m_data[7] & 0x80) == 0 )
                || ( parent_type != (SaHpiEntityTypeT)sdr->m_data[12] )
                || ( parent_instance != (SaHpiEntityLocationT)sdr->m_data[13] ) )

                continue;

            return sdr->m_data[6];
       }
    }

  stdlog << "WARNING : Entity ID " << type << ", Instance " << instance << " did not find parent FRU\n";
  stdlog << "WARNING : Defaulting to FRU 0, Entity ID " << mc_type << ", Instance " << mc_instance << "\n";

  // We didn't find an exact match
  // Since a lot of ATCA boards have wrong SDRs
  // we default the parent to the MC itself
  parent_type = mc_type;
  parent_instance = mc_instance;

  return 0;
}
