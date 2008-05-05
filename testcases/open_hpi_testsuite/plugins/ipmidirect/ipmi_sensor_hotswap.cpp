/*
 * ipmi_sensor_hotswap.cpp
 *
 * Copyright (c) 2004 by FORCE Computers
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

#include "ipmi_sensor_hotswap.h"
#include "ipmi_domain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <math.h>
#include <oh_utils.h>

cIpmiSensorHotswap::cIpmiSensorHotswap( cIpmiMc *mc )
  : cIpmiSensorDiscrete( mc )
{
}


cIpmiSensorHotswap::~cIpmiSensorHotswap()
{
}


bool
cIpmiSensorHotswap::GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr )
{
  if ( !cIpmiSensorDiscrete::GetDataFromSdr( mc, sdr ) )
       return false;

  return true;
}


bool
cIpmiSensorHotswap::CreateRdr( SaHpiRptEntryT &resource,
                               SaHpiRdrT &rdr )
{
  if ( cIpmiSensorDiscrete::CreateRdr( resource, rdr ) == false )
       return false;

  cIpmiResource *res = Resource();

  if( !res )
       return false;

  if (EntityPath() == res->EntityPath())
  {
    // update resource capabilities
    resource.ResourceCapabilities |= SAHPI_CAPABILITY_MANAGED_HOTSWAP;
    resource.HotSwapCapabilities  |= SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED;
  }

  return true;
}


SaHpiHsStateT
cIpmiSensorHotswap::ConvertIpmiToHpiHotswapState( tIpmiFruState h )
{
  switch( h )
     {
       case eIpmiFruStateNotInstalled:
            return SAHPI_HS_STATE_NOT_PRESENT;

       case eIpmiFruStateInactive:
            return SAHPI_HS_STATE_INACTIVE;

       case eIpmiFruStateActivationRequest:
            return SAHPI_HS_STATE_INSERTION_PENDING;

       case eIpmiFruStateActivationInProgress:
            return SAHPI_HS_STATE_INSERTION_PENDING;

       case eIpmiFruStateActive:
            return SAHPI_HS_STATE_ACTIVE;

       case eIpmiFruStateDeactivationRequest:
            return SAHPI_HS_STATE_EXTRACTION_PENDING;

       case eIpmiFruStateDeactivationInProgress:
            return SAHPI_HS_STATE_EXTRACTION_PENDING;

       case eIpmiFruStateCommunicationLost:
       default:
            return SAHPI_HS_STATE_NOT_PRESENT;
     }
}

void
cIpmiSensorHotswap::HandleEvent( cIpmiEvent *event )
{
  cIpmiResource *res = Resource();

  if( !res )
     {
       stdlog << "cIpmiSensorHotswap::HandleEvent: No resource !\n";
       return;
     }

  tIpmiFruState state = (tIpmiFruState)(event->m_data[10] & 0x07);
  tIpmiFruState previous_state = (tIpmiFruState)(event->m_data[11] & 0x07);

  switch (state)
  {
  case eIpmiFruStateActivationInProgress:
    if (previous_state == eIpmiFruStateActivationRequest)
    {
        stdlog << "cIpmiSensorHotswap::HandleEvent: M2->M3 ignore\n";
        res->PreviousPrevFruState() = previous_state;
        return;
    }
    break;
  case eIpmiFruStateDeactivationInProgress:
    if (previous_state == eIpmiFruStateDeactivationRequest)
    {
        stdlog << "cIpmiSensorHotswap::HandleEvent: M5->M6 ignore\n";
        res->PreviousPrevFruState() = previous_state;
        return;
    }
    if (previous_state == eIpmiFruStateActivationInProgress)
    {
        stdlog << "cIpmiSensorHotswap::HandleEvent: M3->M6 ignore\n";
        res->PreviousPrevFruState() = previous_state;
        return;
    }
    break;
  default:
    break;
  }

  oh_event *e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  SaHpiRptEntryT *rptentry = oh_get_resource_by_id( res->Domain()->GetHandler()->rptcache, res->m_resource_id );

  if ( rptentry )
      e->resource = *rptentry;
  else
      e->resource.ResourceCapabilities = 0;

  // hpi event
  SaHpiEventT &h = e->event;

  h.Source    = res->m_resource_id;
  h.EventType = SAHPI_ET_HOTSWAP;
  // Default severity
  h.Severity = SAHPI_INFORMATIONAL;

  // Hot swap events must be dated here otherwise
  // hot swap policy won't work properly !
  oh_gettimeofday(&h.Timestamp);

  SaHpiHotSwapEventT &he = h.EventDataUnion.HotSwapEvent;

  he.HotSwapState = ConvertIpmiToHpiHotswapState( state );
  he.PreviousHotSwapState = ConvertIpmiToHpiHotswapState( previous_state );

  switch (state)
  {
  case eIpmiFruStateNotInstalled:
    if (previous_state == eIpmiFruStateCommunicationLost)
    {
        he.PreviousHotSwapState = ConvertIpmiToHpiHotswapState( res->PreviousPrevFruState() );
    }
    else if (previous_state != eIpmiFruStateInactive)
    {
      // get severity from plugin cache
      if ( rptentry )
        h.Severity = rptentry->ResourceSeverity;
    }
    break;

  case eIpmiFruStateCommunicationLost:

    h.EventType = SAHPI_ET_RESOURCE;
    h.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_FAILURE;

    stdlog << "cIpmiSensorHotswap::HandleEvent SAHPI_RESE_RESOURCE_FAILURE Event resource " << res->m_resource_id << "\n";

    if ( rptentry )
    {
      rptentry->ResourceFailed = SAHPI_TRUE;
      oh_add_resource(res->Domain()->GetHandler()->rptcache,
                      rptentry, res, 1);
      h.Severity = rptentry->ResourceSeverity;
    }
    break;

  case eIpmiFruStateInactive:

    // M3->M6->M1 special case
    if ((previous_state == eIpmiFruStateDeactivationInProgress)
        && (res->PreviousPrevFruState() == eIpmiFruStateActivationInProgress))
    {
        he.PreviousHotSwapState = SAHPI_HS_STATE_INSERTION_PENDING;
        if ( rptentry )
        {
            h.Severity = rptentry->ResourceSeverity;
        }
    }

  default:
    if (previous_state == eIpmiFruStateCommunicationLost)
    {
      h.EventType = SAHPI_ET_RESOURCE;
      h.EventDataUnion.ResourceEvent.ResourceEventType = SAHPI_RESE_RESOURCE_RESTORED;

      stdlog << "cIpmiSensorHotswap::HandleEvent SAHPI_RESE_RESOURCE_RESTORED Event resource " << res->m_resource_id << "\n";

      if ( rptentry )
      {
        rptentry->ResourceFailed = SAHPI_FALSE;
        oh_add_resource(res->Domain()->GetHandler()->rptcache,
                        rptentry, res, 1);
        h.Severity = rptentry->ResourceSeverity;
      }
    }
    break;
  }

  res->PreviousPrevFruState() = previous_state;

  if (h.EventType == SAHPI_ET_HOTSWAP)
    stdlog << "cIpmiSensorHotswap::HandleEvent SAHPI_ET_HOTSWAP Event resource " << res->m_resource_id << "\n";
  m_mc->Domain()->AddHpiEvent( e );

  if (h.EventType != SAHPI_ET_HOTSWAP)
    return;

  oh_event *oem_e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( rptentry )
      oem_e->resource = *rptentry;
  else
      oem_e->resource.ResourceCapabilities = 0;

  // hpi event
  SaHpiEventT &oem_h = oem_e->event;

  oem_h.Source    = h.Source;
  oem_h.Timestamp = h.Timestamp;
  oem_h.EventType = SAHPI_ET_OEM;
  oem_h.Severity = SAHPI_INFORMATIONAL;

  SaHpiOemEventT &oem_he = oem_h.EventDataUnion.OemEvent;

  oem_he.MId = ATCAHPI_PICMG_MID;

  oem_he.OemEventData.DataType =   SAHPI_TL_TYPE_TEXT;
  oem_he.OemEventData.Language =   SAHPI_LANG_UNDEF;
  oem_he.OemEventData.DataLength = 3;
  oem_he.OemEventData.Data[0] =    he.HotSwapState;
  oem_he.OemEventData.Data[1] =    he.PreviousHotSwapState;

  switch ((event->m_data[11] >> 4) & 0x0F)
  {
  case 0x0:
  case 0x1:
    oem_he.OemEventData.Data[2] = 0;
    break;
  case 0x3:
    oem_he.OemEventData.Data[2] = 1;
    break;
  case 0x2:
    oem_he.OemEventData.Data[2] = 2;
    break;
  case 0x7:
    oem_he.OemEventData.Data[2] = 3;
    break;
  case 0x9:
    oem_he.OemEventData.Data[2] = 4;
    break;
  case 0x6:
    oem_he.OemEventData.Data[2] = 5;
    break;
  case 0x8:
    oem_he.OemEventData.Data[2] = 6;
    break;
  case 0xF:
  default:
    oem_he.OemEventData.Data[2] = 7;
    break;
  }

  stdlog << "cIpmiSensorHotswap::HandleEvent SAHPI_ET_OEM Event resource " << res->m_resource_id << "\n";
  m_mc->Domain()->AddHpiEvent( oem_e );
}

SaErrorT
cIpmiSensorHotswap::GetPicmgState( tIpmiFruState &state )
{
  // read hotswap state
  cIpmiMsg rsp;

  // Default value just in case
  state = eIpmiFruStateCommunicationLost;

  SaErrorT rv = GetSensorData( rsp );

  if ( rv != SA_OK )
     {
       stdlog << "cannot get hotswap state !\n";
       return rv;
     }

  // Reading should be 0 according to PICMG 3.0 specification
  // However if it's not this is not a fatal error so just flag it
  if ( rsp.m_data[1] != 0 )
     {
       stdlog << "WARNING: hotswap sensor reading not 0 : " << rsp.m_data[1] << " !\n";
     }

  unsigned int value = rsp.m_data[3];

  for( int i = 0; i < 8; i++ )
       if ( value & ( 1 << i ) )
          {
            state = (tIpmiFruState)i;
            return SA_OK;
          }

  stdlog << "WRONG Hot Swap State " << value << "\n";

  return SA_ERR_HPI_INVALID_DATA;
}


SaErrorT
cIpmiSensorHotswap::GetHpiState( SaHpiHsStateT &state )
{
  tIpmiFruState fs;
  SaErrorT rv = GetPicmgState( fs );

  if ( rv != SA_OK )
       return rv;

  state = ConvertIpmiToHpiHotswapState( fs );

  return SA_OK;
}
