/*
 * ipmi_sdr.cpp
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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
#include "ipmi_type_code.h"


struct cIpmiSdrTypeToName
{
  tIpmiSdrType m_type;
  const char  *m_name;
};


static cIpmiSdrTypeToName type_to_name[] =
{
  { eSdrTypeFullSensorRecord                     , "full sensor" },
  { eSdrTypeCompactSensorRecord                  , "compact sensor" },
  { eSdrTypeEntityAssociationRecord              , "entity association" },
  { eSdrTypeDeviceRelativeEntityAssociationRecord, "device-relative entity association" },
  { eSdrTypeGenericDeviceLocatorRecord           , "generic device locator" },
  { eSdrTypeFruDeviceLocatorRecord               , "FRU device locator" },
  { eSdrTypeMcDeviceLocatorRecord                , "MC device locator" },
  { eSdrTypeMcConfirmationRecord                 , "MC confirmation" },
  { eSdrTypeBmcMessageChannelInfoRecord          , "BMC message channel info" },
  { eSdrTypeOemRecord                            , "OEM" },
  { eSdrTypeUnknown, 0 }
};


const char *
IpmiSdrTypeToName( tIpmiSdrType type )
{
  for( cIpmiSdrTypeToName *t = type_to_name; t->m_name; t++ )
       if ( t->m_type == type )
            return t->m_name;

  return "invalid";
}


bool
cIpmiSdr::Name( char *name, int size )
{
  switch( m_type )
     {
       case eSdrTypeFullSensorRecord:
            IpmiGetDeviceString( m_data + 47, 16, name, size );
            break;

       case eSdrTypeCompactSensorRecord:
            IpmiGetDeviceString( m_data + 31, 16, name, size );
            break;

       case eSdrTypeFruDeviceLocatorRecord:
            IpmiGetDeviceString( m_data + 15, 16, name, size );
            break;

       case eSdrTypeMcDeviceLocatorRecord:
            IpmiGetDeviceString( m_data + 15, 16, name, size );
            break;

       default:
            assert( 0 );
            *name = 0;
            return false;
     }

  return true;
}


static void
ThresholdEventMask( unsigned short em, char *str )
{
  *str = 0;

  if ( em & dIpmiEventLowerNonCriticalLow )
       strcat( str, " lower-non-critical-low" );

  if ( em & dIpmiEventLowerNonCriticalHigh )
       strcat( str, " lower-non-critical-high" );

  if ( em & dIpmiEventLowerCriticalLow )
       strcat( str, " lower-critical-low" );

  if ( em & dIpmiEventLowerCriticalHigh )
       strcat( str, " lower-critical-high" );

  if ( em & dIpmiEventLowerNonRecoverableLow )
       strcat( str, " lower-non-recoverable-low" );

  if ( em & dIpmiEventLowerNonRecoverableHigh )
       strcat( str, " lower-non-recoverable-high" );

  if ( em & dIpmiEventUpperNonCriticalLow )
       strcat( str, " upper-non-critical-low" );

  if ( em & dIpmiEventUpperCriticalHigh )
       strcat( str, " upper-critical-high" );

  if ( em & dIpmiEventUpperNonRecoverableLow )
       strcat( str, " upper-non-recoverable-low" );

  if ( em & dIpmiEventUpperNonRecoverableHigh )
       strcat( str, " upper-non-recoverable-high" );
}


static const char *
Bit( unsigned char byte, int bit )
{
  return ((byte >> bit) & 1) ? "yes" : "no";
}


void
cIpmiSdr::Log()
{
  IpmiLog( "SDR: recordid %d, version %d.%d, type 0x%02x (%s)\n",
           m_record_id, m_major_version, m_minor_version,
           m_type, IpmiSdrTypeToName( m_type ) );

  switch( m_type )
     {
       case eSdrTypeFullSensorRecord:
            {
              char name[80];
              Name( name, 80 );

              IpmiLog( "\tname '%s', owner id 0x%02x, num %d,\n",
                       name, m_data[5] & 0x7f, m_data[7] );
              IpmiLog( "\tentity id 0x%02x, entity instance %d, type 0x%02x, event reading type %s\n",
                       m_data[8], m_data[9], m_data[12], IpmiEventReadingTypeToString( (tIpmiEventReadingType)m_data[13] ) );
              IpmiLog( "\tinit scanning %s, init events %s, init thresholds %s, init hysteresis %s, init sensor type %s,\n",
                       Bit( m_data[10], 6 ), Bit( m_data[10], 5 ), Bit( m_data[10], 4 ), Bit( m_data[10], 3 ), Bit( m_data[10], 2 ) );
              IpmiLog( "\tignore sensor %s, rearm sensor %s, hysteresis support %s, thresholds access %s, event message control %s.\n",
                       Bit( m_data[11], 7 ), Bit( m_data[11], 6 ),
                       IpmiHysteresisSupportToString( (tIpmiHysteresisSupport)((m_data[11] >> 4) & 3 ) ),
                       IpmiThresholdAccessSupportToString( (tIpmiThresholdAccessSuport)((m_data[11] >> 2) & 3 ) ),
                       IpmiEventSupportToString( (tIpmiEventSupport)(m_data[11] & 3) ) );

              if ( m_data[13] == eIpmiEventReadingTypeThreshold )
                 {
                   unsigned short aem = IpmiGetUint16( m_data + 14 );
                   unsigned short dem = IpmiGetUint16( m_data + 16 );
                   char astr[1024];
                   char dstr[1024];

                   ThresholdEventMask( aem, astr );
                   ThresholdEventMask( dem, dstr );

                   IpmiLog( "\t  assertion %s\n\tdeassertion %s\n", astr, dstr );
                 }
            }

            break;

       case eSdrTypeCompactSensorRecord:
            {
              char name[80];
              Name( name, 80 );

              IpmiLog( "\tname '%s', owner id 0x%02x, num %d,\n",
                       name, m_data[5] & 0x7f, m_data[7] );
              IpmiLog( "\tentity id 0x%02x, entity instance %d, type 0x%02x\n",
                       m_data[8], m_data[9], m_data[12] );
            }

            break;

       case eSdrTypeEntityAssociationRecord:
            break;

       case eSdrTypeDeviceRelativeEntityAssociationRecord:
            break;

       case eSdrTypeGenericDeviceLocatorRecord:
            break;

       case eSdrTypeFruDeviceLocatorRecord:
            {
              char name[80];
              Name( name, 80 );

              IpmiLog( "\tname '%s', slave addr 0x%02x, fru id %d,\n",
                       name, m_data[5], m_data[6] );
              IpmiLog( "\tentity id 0x%02x, entity instance %d\n",
                       m_data[12], m_data[13] );
            }
            break;

       case eSdrTypeMcDeviceLocatorRecord:
            {
              char name[80];
              Name( name, 80 );

              IpmiLog( "\tname '%s', slave addr 0x%02x,\n",
                       name, m_data[5] );
              IpmiLog( "\tentity id 0x%02x, entity instance %d\n",
                       m_data[12], m_data[13] );
            }
            break;

       case eSdrTypeMcConfirmationRecord:
            break;

       case eSdrTypeBmcMessageChannelInfoRecord:
            break;

       case eSdrTypeOemRecord:
            break;

       default:
            assert( 0 );
            break;
     }
}


static void
IpmiSdrDestroyRecords( cIpmiSdr **&sdr, unsigned int &n )
{
  if ( sdr == 0 )
       return;

  assert( n > 0 );

  for( unsigned int i = 0; i < n; i++ )
     {
       assert( sdr[i] );
       delete sdr[i];
     }

  delete [] sdr;

  n = 0;
  sdr = 0;
}



cIpmiSdrs::cIpmiSdrs( cIpmiMc *mc, unsigned int lun, bool device_sdr )
  : m_mc( mc ), m_lun( lun ), m_device_sdr( device_sdr ),
    m_fetched( false ), m_major_version( 0 ), m_minor_version( 0 ),
    m_last_addition_timestamp( 0 ), m_last_erase_timestamp( 0 ),
    m_overflow( 0 ), m_update_mode( eIpmiRepositorySdrUpdateUnspecified ),
    m_supports_delete_sdr( false ), m_supports_partial_add_sdr( false ),
    m_supports_reserve_sdr( false ), m_supports_get_sdr_repository_allocation( false ),
    m_dynamic_population( false ),
    m_reservation( 0 ), m_sdr_changed( false ),
    m_num_sdrs( 0 ), m_sdrs( 0 )
{
  m_lun_has_sensors[0] = false;
  m_lun_has_sensors[1] = false;
  m_lun_has_sensors[2] = false;
  m_lun_has_sensors[3] = false;
}


cIpmiSdrs::~cIpmiSdrs()
{
  if ( m_sdrs )
       IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );
}


cIpmiSdr *
cIpmiSdrs::ReadRecord( unsigned short record_id,
                       unsigned short &next_record_id,
                       tReadRecord &err )
{
  cIpmiMsg       msg;
  cIpmiMsg       rsp;
  int            rv;
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

       rv = m_mc->SendCommand( msg, rsp, m_lun );

       if ( rv )
          {
            IpmiLog( "initial_sdr_fetch: Couldn't send GetSdr or GetDeviveSdr fetch: %x !\n", rv );
            err = eReadError;

            return 0;
          }

       if ( rsp.m_data[0] == 0x80 )
          {
            // Data changed during fetch, retry.  Only do this so many
            // times before giving up.
            IpmiLog( "SRD reservation lost.\n" );

            err = eReadReservationLost;

            return 0;
          }

       if ( rsp.m_data[0] == eIpmiCcInvalidReservation )
          {
            IpmiLog( "SRD reservation lost.\n" );

            err = eReadReservationLost;
            return 0;
          }

       if (    record_id == 0 
            && (    (rsp.m_data[0] == eIpmiCcUnknownErr )
	         || (rsp.m_data[0] == eIpmiCcNotPresent ) ) )
          {
            // We got an error fetching the first SDR, so the repository is
            // probably empty.  Just go on.
            IpmiLog( "SRD reservation lost.\n" );

            err = eReadEndOfSdr;
            return 0;
          }
 
       if ( rsp.m_data[0] != eIpmiCcOk )
          {
            IpmiLog( "SDR fetch error getting sdr 0x%x: %x",
                     record_id, rsp.m_data[0] );

            err = eReadError;
            return 0;
          }

       if ( rsp.m_data_len != read_len + 3 )
          {
            IpmiLog( "Got an invalid amount of SDR data: %d, expected %d \n",
		 rsp.m_data_len, read_len + 3 );

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
  sdr->m_length        = record_size;
  memcpy( sdr->m_data, data, record_size );

  err = eReadOk;

  return sdr;
}


int
cIpmiSdrs::Reserve()
{
  cIpmiMsg msg;
  cIpmiMsg rsp;
  int rv;

  assert( m_supports_reserve_sdr );
  
  /* Now get the reservation. */
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
  rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "Couldn't send SDR reservation: %x !\n", rv );
       return EINVAL;
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

            return 0;
	}

       IpmiLog( "Error getting SDR fetch reservation: %x\n",
                rsp.m_data[0] );

       return EINVAL;
     }

  if ( rsp.m_data_len < 3 )
     {
       IpmiLog( "SDR Reservation data not long enough: %d bytes!\n",
                rsp.m_data_len );
       return EINVAL;
     }

  m_reservation = IpmiGetUint16( rsp.m_data + 1 );

  return 0;
}


int
cIpmiSdrs::GetInfo( unsigned short &working_num_sdrs )
{
  int      rv;
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

  rv = m_mc->SendCommand( msg, rsp, m_lun );

  if ( rv )
     {
       IpmiLog( "IpmiSdrsFetch: GetDeviceSdrInfoCmd or GetSdrRepositoryInfoCmd %d, %s !\n", 
                rv, strerror( rv ) );

       m_sdr_changed = true;
       IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

       return rv;
     }

  if ( rsp.m_data[0] != 0 )
     {
  	if ( m_device_sdr )
           {
             /* The device doesn't support the get device SDR info
                command, so just assume some defaults. */
             working_num_sdrs     = 0xfffe;
             m_dynamic_population = false;

             /* Assume it uses reservations, if the reservation returns
               an error, then say that it doesn't. */
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
             IpmiLog( "IPMI Error getting SDR info: %x !\n", rsp.m_data[0]);

             m_sdr_changed = true;
             IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

             return EINVAL;
           }
     }
  else if ( m_device_sdr )
     {
       if ( rsp.m_data_len < 3 ) 
          {
	    IpmiLog( "SDR info is not long enough !\n");

            m_sdr_changed = true;
            IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

	    return EINVAL;
	}

       working_num_sdrs = rsp.m_data[1];
       m_dynamic_population = (rsp.m_data[2] & 0x80) == 0x80;

       // Assume it uses reservations, if the reservation returns
       // an error, then say that it doesn't.
       m_supports_reserve_sdr = true;

       m_lun_has_sensors[0] = (rsp.m_data[2] & 0x01) == 0x01;
       m_lun_has_sensors[1] = (rsp.m_data[2] & 0x01) == 0x02;
       m_lun_has_sensors[2] = (rsp.m_data[2] & 0x01) == 0x04;
       m_lun_has_sensors[3] = (rsp.m_data[2] & 0x01) == 0x08;

       if ( m_dynamic_population )
          {
	    if ( rsp.m_data_len < 7 )
               {
                 IpmiLog( "SDR info is not long enough !\n");

                 m_sdr_changed = 1;
                 IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

                 return EINVAL;
               }

	    add_timestamp = IpmiGetUint32( rsp.m_data + 3 );
          }
       else
	    add_timestamp = 0;

       erase_timestamp = 0;
     }
  else
     {
       if ( rsp.m_data_len < 15 )
          {
	    IpmiLog( "SDR info is not long enough\n" );

            m_sdr_changed = true;
            IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

	    return EINVAL;
          }

       /* Pull pertinant info from the response. */
       m_major_version = rsp.m_data[1] & 0xf;
       m_major_version = (rsp.m_data[1] >> 4) & 0xf;
       working_num_sdrs = IpmiGetUint16( rsp.m_data + 2 );

       m_overflow = (rsp.m_data[14] & 0x80) == 0x80;
       m_update_mode = (tIpmiRepositorySdrUpdate)((rsp.m_data[14] >> 5) & 0x3);
       m_supports_delete_sdr = (rsp.m_data[14] & 0x08) == 0x08;
       m_supports_partial_add_sdr = (rsp.m_data[14] & 0x04) == 0x04;
       m_supports_reserve_sdr = (rsp.m_data[14] & 0x02) == 0x02;
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

  return 0;
}


int
cIpmiSdrs::Fetch()
{
  int      rv;
  int      retry_count;
  cIpmiSdr **records;
  unsigned int num;
  int      finish;
  unsigned short working_num_sdrs;

  m_sdr_changed = false;

  assert( m_mc );

  if ( m_device_sdr )
     {
       if ( !m_mc->ProvidesDeviceSdrs() )
            return ENOSYS;
     }
  else if ( !m_mc->SdrRepositorySupport() )
       return ENOSYS;

  rv = GetInfo( working_num_sdrs );

  // sdr records are up to date
  if ( rv < 0 )
       return 0;

  if ( rv )
       return rv;

  m_sdr_changed = true;
  IpmiSdrDestroyRecords( m_sdrs, m_num_sdrs );

  if ( working_num_sdrs == 0 )
       // No sdrs, so there's nothing to do.
       return 0;

  // read sdr
  records = new cIpmiSdr*[ working_num_sdrs ];
  retry_count = 0;
  finish      = 0;

  while( !finish )
     {
       unsigned short next_record_id = 0;

       if ( retry_count++ == dMaxSdrFetchRetries )
          {
            IpmiLog( "To many retries trying to fetch SDRs\n");
            IpmiSdrDestroyRecords( records, num );

            return EBUSY;
          }

       num = 0;

       rv = Reserve();

       if ( rv )
          {
            IpmiSdrDestroyRecords( records, num );
            return rv;
          }

       // read sdr records
       while( 1 )
          {
            tReadRecord err;
            cIpmiSdr *sdr;
            unsigned short record_id = next_record_id;

            if ( num >= working_num_sdrs )
               {
                 IpmiLog( "To many SDR records !\n", working_num_sdrs );
                 finish = 1;
                 break;
               }

            sdr = ReadRecord( record_id, next_record_id, err );

            if ( sdr == 0 )
               {
                 if ( err == eReadReservationLost )
                      break;

                 if ( err != eReadEndOfSdr )
                    {
                      IpmiSdrDestroyRecords( records, num );
                      return EINVAL;
                    }

                 // SDR is empty
                 finish = 1;

                 break;
               }

            sdr->Log();

            records[num++] = sdr;

            if ( next_record_id == 0xffff )
               {
                 // finish
                 finish = 1;

                 break;
               }
          }
     }

  if ( num == working_num_sdrs )
     {
       m_sdrs = records;
       m_num_sdrs = working_num_sdrs;

       return 0;
     }

  m_sdrs = new cIpmiSdr *[num];
  memcpy( m_sdrs, records, num * sizeof( cIpmiSdr * ) );
  m_num_sdrs = num;

  delete [] records;

  return 0;
}
