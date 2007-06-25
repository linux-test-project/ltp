/*
 * ipmi_sdr.h
 *
 * Copyright (c) 2003,2004 by FORCE Computers
 * Copyright (c) 2005 by ESO Technologies.
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
 */

#ifndef dIpmiSdr_h
#define dIpmiSdr_h


#include <assert.h>

extern "C" {
#include "SaHpi.h"
}

#include "glib.h"

#ifndef dIpmiLog_h
#include "ipmi_log.h"
#endif


#define dMaxSdrData 255

#define dSdrHeaderSize 5

// Max bytes to try to get at a time.
#define dMaxSdrFetch 20

// Do up to this many retries when the reservation is lost.
#define dMaxSdrFetchRetries 10


enum tIpmiSdrType
{
  eSdrTypeUnknown = 0,
  eSdrTypeFullSensorRecord = 0x01,
  eSdrTypeCompactSensorRecord = 0x02,
  eSdrTypeEntityAssociationRecord = 0x08,
  eSdrTypeDeviceRelativeEntityAssociationRecord = 0x09,
  eSdrTypeGenericDeviceLocatorRecord = 0x10,
  eSdrTypeFruDeviceLocatorRecord = 0x11,
  eSdrTypeMcDeviceLocatorRecord = 0x12,
  eSdrTypeMcConfirmationRecord = 0x13,
  eSdrTypeBmcMessageChannelInfoRecord = 0x14,
  eSdrTypeOemRecord = 0xc0
};

const char *IpmiSdrTypeToName( tIpmiSdrType type );


class cIpmiSdr
{
public:
  unsigned short m_record_id;
  unsigned char  m_major_version;
  unsigned char  m_minor_version;
  tIpmiSdrType   m_type;
  unsigned char  m_length;
  unsigned char  m_data[dMaxSdrData];

  void Dump( cIpmiLog &dump, const char *name ) const;

protected:
  void DumpFullSensor( cIpmiLog &dump ) const;
  void DumpFruDeviceLocator( cIpmiLog &dump ) const;
  void DumpMcDeviceLocator( cIpmiLog &dump ) const;
};


enum tIpmiRepositorySdrUpdate
{
  eIpmiRepositorySdrUpdateUnspecified = 0,
  eIpmiRepositorySdrUpdateNonModal    = 1,
  eIpmiRepositorySdrUpdateModal       = 2,
  eIpmiRepositorySdrUpdatBoth         = 3
};

const char *IpmiRepositorySdrUpdateToString( tIpmiRepositorySdrUpdate val );


class cIpmiMc;


class cIpmiSdrs
{
protected:
  cIpmiMc       *m_mc;
  bool           m_device_sdr;

  // Have the SDRs previously been fetched?
  bool           m_fetched;

  // Information from the SDR Repository Info command, non-sensor
  // mode only.
  unsigned char  m_major_version;
  unsigned char  m_minor_version;
  unsigned int   m_last_addition_timestamp;
  unsigned int   m_last_erase_timestamp;
  bool           m_overflow;
  tIpmiRepositorySdrUpdate m_update_mode;
  bool           m_supports_delete_sdr;
  bool           m_supports_partial_add_sdr;
  bool           m_supports_reserve_sdr;
  bool           m_supports_get_sdr_repository_allocation;

  // device SDR
  bool           m_dynamic_population;
  bool           m_lun_has_sensors[4];

  unsigned int   m_reservation;
  bool           m_sdr_changed;

  // The actual current copy of the SDR repository.
  unsigned int   m_num_sdrs;
  cIpmiSdr     **m_sdrs;

private:
  enum tReadRecord
  {
    eReadOk,
    eReadEndOfSdr,
    eReadReservationLost,
    eReadError
  };

  SaErrorT ReadRecords( cIpmiSdr **&records, unsigned short &working_num_sdrs,
			unsigned int &num, unsigned int lun );
  cIpmiSdr *ReadRecord( unsigned short record_id,
                        unsigned short &next_record_id,
                        tReadRecord &err, unsigned int lun );
  GList *CreateFullSensorRecords( cIpmiSdr *sdr );

  SaErrorT Reserve(unsigned int lun);
  int GetInfo( unsigned short &working_num_sdrs );

public:
  cIpmiSdrs( cIpmiMc *mc, bool device_sdr );
  ~cIpmiSdrs();

  unsigned int NumSdrs() const { return m_num_sdrs; }
  cIpmiSdr     *Sdr( unsigned int i )
  {
    assert( i < m_num_sdrs );
    return m_sdrs[i];
  }

  SaErrorT Fetch();

  void Dump( cIpmiLog &dump, const char *name ) const;

  cIpmiSdr *FindSdr( cIpmiMc *mc );

  unsigned int FindParentFru( SaHpiEntityTypeT type,
                              SaHpiEntityLocationT instance,
                              SaHpiEntityTypeT & parent_type,
                              SaHpiEntityLocationT & parent_instance
                            );
};


#endif
