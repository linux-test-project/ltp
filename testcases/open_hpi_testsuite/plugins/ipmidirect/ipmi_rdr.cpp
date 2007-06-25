/*
 * ipmi_rdr.cpp
 *
 * Copyright (c) 2004 by FORCE Computers.
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

#include "ipmi_rdr.h"
#include "ipmi_mc.h"
#include "ipmi_entity.h"
#include "ipmi_resource.h"
#include "ipmi_domain.h"


cIpmiRdr::cIpmiRdr( cIpmiMc *mc, SaHpiRdrTypeT type )
  : m_mc( mc ), m_resource( 0 ), m_type( type ),
    m_lun( 0 ), m_populate( false )
{
}


cIpmiRdr::~cIpmiRdr()
{
}


cIpmiDomain *
cIpmiRdr::Domain()
{
  return m_mc->Domain();
}


bool
cIpmiRdr::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  rdr.RecordId = m_record_id;
  rdr.RdrType  = m_type;
  rdr.Entity   = m_entity_path;
  rdr.IdString = m_id_string;

  return true;
}


SaErrorT
cIpmiRdr::SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp,
		       unsigned int lun, int retries )
{
  return m_mc->SendCommand( msg, rsp, lun, retries );
}


bool
cIpmiRdr::Populate(GSList **list)
{
  if ( m_populate )
       return true;

  // find resource
  SaHpiRptEntryT *resource = Domain()->FindResource( Resource()->m_resource_id );

  if ( !resource )
     {
       stdlog << "Resource not found: Can't populate RDR !\n";
       return false;
     }

  // create rdr
  SaHpiRdrT *rdr = (SaHpiRdrT *)g_malloc0(sizeof(SaHpiRdrT));
  CreateRdr( *resource, *rdr );

  int rv = oh_add_rdr( Domain()->GetHandler()->rptcache,
                       resource->ResourceId,
                       rdr, this, 1 );

  if ( rv != 0 )
  {
       stdlog << "Can't add RDR to plugin cache !\n";
       g_free( rdr );
       return false;
  }

  // assign the hpi record id to sensor, so we can find
  // the rdr for a given sensor.
  // the id comes from oh_add_rdr.
  RecordId() = rdr->RecordId;

  stdlog << "cIpmiRdr::Populate RDR for resource " << resource->ResourceId << " RDR " << RecordId() << "\n";
  *list = g_slist_append(*list, rdr);

  m_populate = true;

  return true;
}

