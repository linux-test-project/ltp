/*
 * ipmi_resource.cpp
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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>

#include "ipmi_domain.h"


cIpmiResource::cIpmiResource( cIpmiMc *mc, unsigned int fru_id )
  : m_sel( false ), m_mc( mc ), m_fru_id( fru_id ),
    m_is_fru( false ),
    m_hotswap_sensor( 0 ),
    m_picmg_fru_state( eIpmiFruStateNotInstalled ),
    m_policy_canceled( true ),
    m_prev_prev_fru_state( eIpmiFruStateNotInstalled ),
    m_oem( 0 ), m_current_control_id( 0 ),
    m_populate( false )
{
  m_extract_timeout = Domain()->ExtractTimeout();

  for( int i = 0; i < 256; i++ )
       m_sensor_num[i] = -1;
}


cIpmiResource::~cIpmiResource()
{
}


cIpmiDomain *
cIpmiResource::Domain() const
{
  return m_mc->Domain();
}


SaErrorT
cIpmiResource::SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp,
			    unsigned int lun, int retries )
{
  return m_mc->SendCommand( msg, rsp, lun, retries );
}


SaErrorT
cIpmiResource::SendCommandReadLock( const cIpmiMsg &msg, cIpmiMsg &rsp,
                                    unsigned int lun, int retries )
{
  cIpmiResource *resource = this;
  cIpmiDomain *domain = Domain();
  domain->ReadUnlock();

  SaErrorT rv = SendCommand( msg, rsp, lun, retries );

  domain->ReadLock();

  if ( domain->VerifyResource( resource ) == false )
       return SA_ERR_HPI_NOT_PRESENT;

  return rv;
}


SaErrorT
cIpmiResource::SendCommandReadLock( cIpmiRdr *rdr, const cIpmiMsg &msg, cIpmiMsg &rsp,
                                    unsigned int lun, int retries )
{
  cIpmiDomain *domain = Domain();
  domain->ReadUnlock();
  
  SaErrorT rv = SendCommand( msg, rsp, lun, retries );
  
  domain->ReadLock();
  
  if ( domain->VerifyRdr( rdr ) == false )
       return SA_ERR_HPI_NOT_PRESENT;

  return rv;
}


int
cIpmiResource::CreateSensorNum( SaHpiSensorNumT num )
{
  int v = num;

  if ( m_sensor_num[v] != -1 )
     {
       for( int i = 0xff; i >= 0; i-- )
            if ( m_sensor_num[i] == -1 )
               {
                 v = i;
                 break;
               }

       if ( m_sensor_num[v] != -1 )
          {
            assert( 0 );
            return -1;
          }
     }

  m_sensor_num[v] = num;

  return v;
}


bool
cIpmiResource::Create( SaHpiRptEntryT &entry )
{
  stdlog << "add resource: " << m_entity_path << ".\n";

  entry.EntryId = 0;

  // resource info
  SaHpiResourceInfoT &info = entry.ResourceInfo;

  memset( &info, 0, sizeof( SaHpiResourceInfoT ) );

  entry.ResourceEntity = m_entity_path;
  entry.ResourceId     = oh_uid_from_entity_path( &entry.ResourceEntity );

  entry.ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE;

  if ( m_sel )
    entry.ResourceCapabilities |= SAHPI_CAPABILITY_EVENT_LOG;

  if ( m_is_fru == true )
    {
        entry.ResourceCapabilities |= SAHPI_CAPABILITY_FRU;

        if ( m_fru_id == 0 )
            {
                info.ResourceRev      = (SaHpiUint8T)m_mc->DeviceRevision();
                info.DeviceSupport    = (SaHpiUint8T)m_mc->DeviceSupport();
                info.ManufacturerId   = (SaHpiManufacturerIdT)m_mc->ManufacturerId();
                info.ProductId        = (SaHpiUint16T)m_mc->ProductId();
                info.FirmwareMajorRev = (SaHpiUint8T)m_mc->MajorFwRevision();
                info.FirmwareMinorRev = (SaHpiUint8T)m_mc->MinorFwRevision();
                info.AuxFirmwareRev   = (SaHpiUint8T)m_mc->AuxFwRevision( 0 );
            }

        // Reset supported on ATCA FRUs - Don't allow it on fru #0 of the active ShMC
        if ( m_mc->IsTcaMc() )
           {
                if (!( (m_mc->GetAddress() == dIpmiBmcSlaveAddr) && (m_fru_id == 0) ))
                {
                    entry.ResourceCapabilities |= SAHPI_CAPABILITY_RESET;
                }
           }
        else if ( m_mc->IsRmsBoard() ) { /*Rms enabling - 11/09/06 ARCress */
            SaHpiEntityTypeT type = cIpmiEntityPath(m_entity_path).GetEntryType(0);
            if (type == SAHPI_ENT_SYSTEM_BOARD)  {
               stdlog << "Enabling Reset on RMS type " << type << "\n";
                entry.ResourceCapabilities |= SAHPI_CAPABILITY_RESET;
                entry.ResourceCapabilities |= SAHPI_CAPABILITY_POWER;
           }
        }
    }

  entry.HotSwapCapabilities = 0;
  entry.ResourceSeverity = SAHPI_OK;
  entry.ResourceFailed = SAHPI_FALSE;
  entry.ResourceTag = ResourceTag();

  return true;
}


void
cIpmiResource::Destroy()
{
  SaHpiRptEntryT *rptentry;
  stdlog << "removing resource: " << m_entity_path << ").\n";

  while( Num() )
  {
      cIpmiRdr *rdr = GetRdr( 0 );
      RemRdr( rdr );
      delete rdr;
  }
  
  rptentry = oh_get_resource_by_id( Domain()->GetHandler()->rptcache, m_resource_id );
  if ( !rptentry )
  {
       stdlog << "Can't find resource in plugin cache !\n";
  }
  else
  {
      // create remove event
      oh_event *e = (oh_event *)g_malloc0( sizeof( oh_event ) );
  
      // remove sensors only if resource is FRU
      if ( rptentry->ResourceCapabilities & SAHPI_CAPABILITY_FRU )
      {
           e->event.EventType = SAHPI_ET_HOTSWAP;
           if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)
           {
               e->event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
               e->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
           }
           else
           {
               e->event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
               e->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
           }
      }
      else
      {
           e->event.EventType = SAHPI_ET_RESOURCE;
           e->event.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_FAILURE;
           rptentry->ResourceFailed = SAHPI_TRUE;
      }
  
      e->event.Source = rptentry->ResourceId;
      oh_gettimeofday(&e->event.Timestamp);
      e->event.Severity = rptentry->ResourceSeverity;
      e->resource = *rptentry;
      stdlog << "cIpmiResource::Destroy OH_ET_RESOURCE_DEL Event resource " << m_resource_id << "\n";
      Domain()->AddHpiEvent( e );

      // remove resource from local cache
      int rv = oh_remove_resource( Domain()->GetHandler()->rptcache, m_resource_id );

      if ( rv != 0 )
      {
          stdlog << "Can't remove resource from plugin cache !\n";
      }
  }
  
  m_mc->RemResource( this );

  delete this;
}


cIpmiRdr *
cIpmiResource::FindRdr( cIpmiMc *mc, SaHpiRdrTypeT type,
			unsigned int num, unsigned int lun )
{
  for( int i = 0; i < NumRdr(); i++ )
     {
       cIpmiRdr *r = GetRdr( i );

       if (    r->Mc()   == mc 
            && r->Type() == type
            && r->Num()  == num
            && r->Lun()  == lun )
	    return r;
     }

  return 0;
}


bool
cIpmiResource::AddRdr( cIpmiRdr *rdr )
{
  stdlog << "adding rdr: " << rdr->EntityPath();
  stdlog << " " << rdr->Num();
  stdlog << " " << rdr->IdString() << "\n";

  // set resource
  rdr->Resource() = this;

  // add rdr to resource
  Add( rdr );

  // check for hotswap sensor
  cIpmiSensorHotswap *hs = dynamic_cast<cIpmiSensorHotswap *>( rdr );

  if ( hs )
     {
       if (hs->EntityPath() == EntityPath())
       {
           if ( m_hotswap_sensor )
                stdlog << "WARNING: found a second hotswap sensor, discard it !\n";
           else
                m_hotswap_sensor = hs;
       }
       else
       {
           stdlog << "WARNING: hotswap sensor ep " << hs->EntityPath() << "!= resource ep " << EntityPath() <<", discard it \n";
       }
     }

  return true;
}


bool
cIpmiResource::RemRdr( cIpmiRdr *rdr )
{
  int idx = Find( rdr );
  
  if ( idx == -1 )
     {
       stdlog << "user requested removal of a control"
                " from a resource, but the control was not there !\n";
       return false;
     }

  if ( rdr == m_hotswap_sensor )
       m_hotswap_sensor = 0;

  Rem( idx );

  return true;
}


bool
cIpmiResource::Populate()
{
  if ( m_populate == false )
     {
       // create rpt entry
       stdlog << "populate resource: " << EntityPath() << ".\n";

       struct oh_event *e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

       if ( Create( e->resource ) == false )
          {
            g_free( e );
            return false;
          }

       // assign the hpi resource id to ent, so we can find
       // the resource for a given entity
       m_resource_id = e->resource.ResourceId;

       // add the resource to the resource cache
       int rv = oh_add_resource( Domain()->GetHandler()->rptcache,
                                 &(e->resource), this, 1 );

       if ( rv != 0 )
       {
            stdlog << "Can't add resource to plugin cache !\n";
            g_free( e );
            return false;
       }

       for( int i = 0; i < NumRdr(); i++ )
       {
           cIpmiRdr *rdr = GetRdr( i );

           if ( rdr->Populate(&e->rdrs) == false )
	       return false;
       }

       // find updated resource in plugin cache
       SaHpiRptEntryT *resource = oh_get_resource_by_id( Domain()->GetHandler()->rptcache,
                                                            m_resource_id );

       if (!resource)
            return false;

       // Update resource in event accordingly
       memcpy(&e->resource, resource, sizeof (SaHpiRptEntryT));

       if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_FRU)
       {
           e->event.EventType = SAHPI_ET_HOTSWAP;

           if (e->resource.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP)
           {
               SaHpiHsStateT hpi_state = GetHpiState();
               e->event.EventDataUnion.HotSwapEvent.HotSwapState = hpi_state;
               e->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = hpi_state;
               stdlog << "cIpmiResource::Populate SAHPI_ET_HOTSWAP Managed FRU Event resource " << m_resource_id << " State " << hpi_state << "\n";
           }
           else
           {
               e->event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_ACTIVE;
               e->event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
               stdlog << "cIpmiResource::Populate SAHPI_ET_HOTSWAP FRU Event resource " << m_resource_id << "\n";
           }
       }
       else
       {
           e->event.EventType = SAHPI_ET_RESOURCE;
           e->event.EventDataUnion.ResourceEvent.ResourceEventType =
               SAHPI_RESE_RESOURCE_ADDED;
           stdlog << "cIpmiResource::Populate SAHPI_ET_RESOURCE Event resource " << m_resource_id << "\n";
       }

       e->event.Source = e->resource.ResourceId;
       e->event.Severity = e->resource.ResourceSeverity;
       oh_gettimeofday(&e->event.Timestamp);

       Domain()->AddHpiEvent( e );
  
       m_populate = true;
     }

  return true;
}

void
cIpmiResource::Activate()
{
    cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruActivation );
    msg.m_data_len = 3;
    msg.m_data[0]  = dIpmiPicMgId;
    msg.m_data[1]  = FruId();
    msg.m_data[2] = dIpmiActivateFru;

    cIpmiMsg rsp;

    SaErrorT rv = SendCommand( msg, rsp );

    if ( rv != SA_OK )
    {
        stdlog << "Activate: could not send set FRU Activation: " << rv << " !\n";
    }
    else if ( rsp.m_data_len < 2
            || rsp.m_data[0] != eIpmiCcOk
            || rsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "Activate: IPMI error set FRU Activation: "
                    << rsp.m_data[0] << " !\n";
    }
}

void
cIpmiResource::Deactivate()
{
    cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdSetFruActivation );
    msg.m_data_len = 3;
    msg.m_data[0]  = dIpmiPicMgId;
    msg.m_data[1]  = FruId();
    msg.m_data[2] = dIpmiDeactivateFru;

    cIpmiMsg rsp;

    SaErrorT rv = SendCommand( msg, rsp );

    if ( rv != SA_OK )
    {
        stdlog << "Deactivate: could not send set FRU deactivation: " << rv << " !\n";
    }
    else if ( rsp.m_data_len < 2
            || rsp.m_data[0] != eIpmiCcOk
            || rsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "Deactivate: IPMI error set FRU deactivation: "
                    << rsp.m_data[0] << " !\n";
    }
}

SaHpiHsStateT
cIpmiResource::GetHpiState()
{
    cIpmiSensorHotswap *hs = GetHotswapSensor();

    if ( hs == 0 )
        return SAHPI_HS_STATE_NOT_PRESENT;

    tIpmiFruState picmg_state;

    SaErrorT rv = hs->GetPicmgState( picmg_state );

    if ( rv != SA_OK )
        return SAHPI_HS_STATE_NOT_PRESENT;

    PicmgFruState() = picmg_state;

    SaHpiHsStateT hpi_state;

    rv = hs->GetHpiState( hpi_state );

    if ( rv != SA_OK )
        return SAHPI_HS_STATE_NOT_PRESENT;

    return hpi_state;
}
