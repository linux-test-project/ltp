/*
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
 *     Andy Cress        <arcress@user.sourceforge.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "ipmi_mc.h"
#include "ipmi_inventory.h"
#include "ipmi_sensor.h"
#include "ipmi_utils.h"
#include "ipmi_domain.h"


cIpmiMc::cIpmiMc( cIpmiDomain *domain, const cIpmiAddr &addr )
  : m_addr( addr ), m_active( true ),
    m_domain( domain ),
    m_sensors_in_my_sdr( 0 ),
    m_sel( 0 ),
    m_device_id( 0 ), m_device_revision( 0 ), 
    m_provides_device_sdrs( false ), m_device_available( false ),
    m_device_support ( 0 ),
    m_chassis_support( false ), m_bridge_support( false ),
    m_ipmb_event_generator_support( false ), m_ipmb_event_receiver_support( false ),
    m_fru_inventory_support( false ), m_sel_device_support( false ),
    m_sdr_repository_support( false ), m_sensor_device_support( false ),
    m_major_fw_revision( 0 ), m_minor_fw_revision( 0 ),
    m_major_version( 0 ), m_minor_version( 0 ),
    m_manufacturer_id( 0 ), m_product_id( 0 ), m_is_tca_mc( false ), m_is_rms_board( false )
{
  stdlog << "adding MC: " << addr.m_channel << " " << addr.m_slave_addr << "\n";

  // use default as long as the manufactorer
  // and product id is unknown
  m_vendor = cIpmiMcVendorFactory::GetFactory()->Default();

  m_aux_fw_revision[0] = 0;
  m_aux_fw_revision[1] = 0;
  m_aux_fw_revision[2] = 0;
  m_aux_fw_revision[3] = 0;

  m_sdrs = new cIpmiSdrs( this, true );

  m_sel = new cIpmiSel( this, 0 );
}


cIpmiMc::~cIpmiMc()
{
  assert( !m_active );

  if ( m_sdrs )
     {
       delete m_sdrs;
       m_sdrs = 0;
     }

  if ( m_sel )
     {
       delete m_sel;
       m_sel = 0;
     }

  assert( Num() == 0 );
}


cIpmiResource *
cIpmiMc::FindResource( cIpmiResource *res )
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *r = operator[]( i );

       if ( r == res )
	    return res;
     }

  return 0;
}

cIpmiResource *
cIpmiMc::GetResource( int i )
{
  if ( i >= Num() )
      return 0;

  cIpmiResource *res = operator[]( i );

  return res;
}


cIpmiResource *
cIpmiMc::FindResource( const cIpmiEntityPath &ep )
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       if ( res->EntityPath() == ep )
	    return res;
     }

  return 0;
}


void
cIpmiMc::AddResource( cIpmiResource *res )
{
  if ( FindResource( res ) )
     {
       assert( 0 );
       return;
     }

  Add( res );
}


void
cIpmiMc::RemResource( cIpmiResource *res )
{
  int idx = Find( res );

  if ( idx == -1 )
     {
       assert( 0 );
       return;
     }

  Rem( idx );
}


bool
cIpmiMc::Cleanup()
{
  m_vendor->CleanupMc( this );

  // First the device SDR sensors, since they can be there for any MC.
  while( m_sensors_in_my_sdr )
     {
       cIpmiSensor *sensor = (cIpmiSensor *)m_sensors_in_my_sdr->data;
       m_sensors_in_my_sdr = g_list_remove( m_sensors_in_my_sdr, sensor );
       sensor->Resource()->RemRdr( sensor );
       delete sensor;
     }

  while( Num() )
     {
       cIpmiResource *res = operator[]( 0 );
       res->Destroy();
     }

  m_active = false;

  stdlog << "removing MC: " << m_addr.m_channel << " " << m_addr.m_slave_addr << "\n";

  return true;
}


SaErrorT
cIpmiMc::SendSetEventRcvr( unsigned int addr )
{
  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdSetEventReceiver );
  cIpmiMsg rsp;
  SaErrorT rv;

  stdlog << "Send set event receiver: " << addr << ".\n";

  msg.m_data_len = 2;
  msg.m_data[0]  = addr;
  msg.m_data[1]  = 0; // LUN is 0 per the spec (section 7.2 of 1.5 spec).

  stdlog << "SendSetEventRcvr: " << GetChannel() << " " << (unsigned char)GetAddress()
         << " -> 0 " << (unsigned char)addr << "\n";

  rv = SendCommand( msg, rsp );

  if ( rv != SA_OK )
       return rv;

  if ( rsp.m_data[0] != 0 )
     {
       // Error setting the event receiver, report it.
       stdlog << "Could not set event receiver for MC at " << m_addr.m_slave_addr << " !\n";

       // Intel ShMc does not like being told where to send events !
       if ( rsp.m_data[0] == eIpmiCcInsufficientPrivilege )
           return SA_OK;

       return SA_ERR_HPI_INVALID_DATA;
     }

  return SA_OK;
}


SaErrorT
cIpmiMc::HandleNew()
{
  SaErrorT rv;

  m_active = true;

  /* fix for SdrRep also - ARCress 09/21/06 */
  if ( m_provides_device_sdrs ||  m_sdr_repository_support  )
     {
       rv = m_sdrs->Fetch();

       if ( rv )
            return rv;

       if ( m_sdrs->NumSdrs() == 0 )
       {
            stdlog << "WARNING: MC " << m_addr.m_slave_addr << " SDR is empty !!!\n";
            return SA_ERR_HPI_INVALID_PARAMS;
       }

       if ( m_vendor->ProcessSdr( Domain(), this, m_sdrs ) == false )
            return SA_ERR_HPI_INVALID_PARAMS;

       if ( m_vendor->CreateRdrs( Domain(), this, m_sdrs ) == false )
            return SA_ERR_HPI_INVALID_PARAMS;
     }

  // read the sel first
  if ( m_sel_device_support )
     {
       rv = m_sel->GetInfo();

       if ( rv != SA_OK )
       {
            m_sel_device_support = false;
       }
       else
       {
           SaHpiTimeT sel_time;

           oh_gettimeofday( &sel_time );

           m_sel->SetSelTime( sel_time );

           m_sel->m_fetched = false;

           if (IsTcaMc()) {
               rv = m_sel->ClearSel();
               if ( rv != SA_OK )
                   m_sel_device_support = false;
           }

           if ( m_sel_device_support )
           {
                // read old events
                GList *list = m_sel->GetEvents();
                m_sel->ClearList( list );
           }
       }
     }

  // We set the event receiver here, so that we know all the SDRs
  // are installed.  That way any incoming events from the device
  // will have the proper sensor set.
  unsigned int event_rcvr = 0;

  if ( m_ipmb_event_generator_support )
     {
       cIpmiMc *er = m_domain->GetEventRcvr();

       if ( er )
            event_rcvr = er->GetAddress();
     }
  else if ( m_sel_device_support && m_provides_device_sdrs) {  
       // If it is an SEL device and not an event receiver, then
       // may want to set its event receiver to itself.
       event_rcvr = GetAddress();   
       stdlog << "New mc, event_rcvr " << GetAddress() << "\n";
  }

  if ( event_rcvr && IsTcaMc())
     {
       // This is a re-arm of all sensors of the MC
       // => each sensor sends the pending events again.
       // because we now which events are old,
       // we can get the current state.
       rv = SendSetEventRcvr( event_rcvr );

       if ( rv )
            return rv;
     }

  return 0;
}


bool
cIpmiMc::DeviceDataCompares( const cIpmiMsg &rsp ) const
{
  const unsigned char *rsp_data = rsp.m_data;

  if ( rsp.m_data_len < 12 )
       return false;

  if ( m_device_id != rsp_data[1] )
       return false;

  if ( m_device_revision != (rsp_data[2] & 0xf) )
       return false;
    
  if ( m_provides_device_sdrs != ((rsp_data[2] & 0x80) == 0x80) )
       ;   /* dont  return false;   ARCress */

  if ( m_device_available != ((rsp_data[3] & 0x80) == 0x80) )
       return false;

  if ( m_major_fw_revision != (rsp_data[3] & 0x7f) )
       return false;

  if ( m_minor_fw_revision != (rsp_data[4]) )
       return false;

  if ( m_major_version != (rsp_data[5] & 0xf) )
       return false;

  if ( m_minor_version != ((rsp_data[5] >> 4) & 0xf) )
       return false;

  if ( m_chassis_support != ((rsp_data[6] & 0x80) == 0x80) )
       return false;

  if ( m_bridge_support != ((rsp_data[6] & 0x40) == 0x40) )
       return false;

  if ( m_ipmb_event_generator_support != ((rsp_data[6] & 0x20)==0x20) )
       return false;

  if ( m_ipmb_event_receiver_support != ((rsp_data[6] & 0x10) == 0x10) )
       return false;

  if ( m_fru_inventory_support != ((rsp_data[6] & 0x08) == 0x08) )
       return false;

  if ( m_sel_device_support != ((rsp_data[6] & 0x04) == 0x04) )
       return false;

  if ( m_sdr_repository_support != ((rsp_data[6] & 0x02) == 0x02) )
       return false;

  if ( m_sensor_device_support != ((rsp_data[6] & 0x01) == 0x01) )
       return false;

  if ( m_manufacturer_id != (unsigned int)(   (rsp_data[7]
						 | (rsp_data[8] << 8)
						 | (rsp_data[9] << 16))) )
       return false;

  if ( m_product_id != (rsp_data[10] | (rsp_data[11] << 8)) )
       return false;

  if ( rsp.m_data_len < 16 )
     {
       // no aux revision, it should be all zeros.
       if (    ( m_aux_fw_revision[0] != 0 )
            || ( m_aux_fw_revision[1] != 0 )
            || ( m_aux_fw_revision[2] != 0 )
            || ( m_aux_fw_revision[3] != 0 ) )
            return false;
     }
  else 
       if ( memcmp( m_aux_fw_revision, rsp_data + 12, 4 ) != 0 )
            return false;

  // Everything's the same.
  return true;
}

void
cIpmiMc::CheckTca()
{
    cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetPicMgProperties );
    msg.m_data_len = 1;
    msg.m_data[0]  = dIpmiPicMgId;

    cIpmiMsg rsp;
    SaErrorT rv;

    m_is_tca_mc = false;
    m_picmg_major = 0;
    m_picmg_minor = 0;

    rv = SendCommand( msg, rsp );

    if ( rv != SA_OK || rsp.m_data[0] || rsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "WARNING: MC " << m_addr.m_slave_addr << " is not a TCA MC !!!\n";
        return;
    }

    m_picmg_minor = (rsp.m_data[2] >> 4) & 0x0f;
    m_picmg_major = rsp.m_data[2] & 0x0f;

    if ( m_picmg_major == 2 )
    {
        stdlog << "MC " << m_addr.m_slave_addr << " is an ATCA MC, PICMG Extension version " << (int)m_picmg_major << "." << (int)m_picmg_minor << "\n";
        m_is_tca_mc = true;
        return;
    }
    else if ( m_picmg_major == 5 )
    {
        stdlog << "MC " << m_addr.m_slave_addr << " is a MicroTCA MC, PICMG Extension version " << (int)m_picmg_major << "." << (int)m_picmg_minor << "\n";
        m_is_tca_mc = true;
        return;
    }

    stdlog << "WARNING: MC " << m_addr.m_slave_addr << " is not an ATCA MC !!!\n";
    return;
}

int
cIpmiMc::GetDeviceIdDataFromRsp( const cIpmiMsg &rsp )
{
  const unsigned char *rsp_data = rsp.m_data;

  if ( rsp_data[0] != 0 )
       return EINVAL;

  if ( rsp.m_data_len < 12 )
       return EINVAL;

  m_device_id                    = rsp_data[1];
  m_device_revision              = rsp_data[2] & 0xf;
  m_provides_device_sdrs         = (rsp_data[2] & 0x80) == 0x80;
  m_device_available             = (rsp_data[3] & 0x80) == 0x80;
  m_major_fw_revision            = rsp_data[3] & 0x7f;
  m_minor_fw_revision            = (rsp_data[4] & 0x0f) + 10 * ((rsp_data[4] >> 4) & 0x0f);
  m_major_version                = rsp_data[5] & 0xf;
  m_minor_version                = (rsp_data[5] >> 4) & 0xf;
  m_device_support               = rsp_data[6];
  m_chassis_support              = (rsp_data[6] & 0x80) == 0x80;
  m_bridge_support               = (rsp_data[6] & 0x40) == 0x40;
  m_ipmb_event_generator_support = (rsp_data[6] & 0x20) == 0x20;
  m_ipmb_event_receiver_support  = (rsp_data[6] & 0x10) == 0x10;
  m_fru_inventory_support        = (rsp_data[6] & 0x08) == 0x08;
  m_sel_device_support           = (rsp_data[6] & 0x04) == 0x04;
  m_sdr_repository_support       = (rsp_data[6] & 0x02) == 0x02;
  m_sensor_device_support        = (rsp_data[6] & 0x01) == 0x01;
  m_manufacturer_id              =    (rsp_data[7]
                                    | (rsp_data[8] << 8)
                                    | (rsp_data[9] << 16));
  m_product_id                   = rsp_data[10] | (rsp_data[11] << 8);

  if ( rsp.m_data_len < 16 )
       // no aux revision.
       memset( m_aux_fw_revision, 0, 4 );
  else
       memcpy( m_aux_fw_revision, rsp_data + 12, 4 );

  return 0;
}


void
cIpmiMc::CheckEventRcvr()
{
  SaErrorT rv;

  if ( m_ipmb_event_generator_support )
       return;

  // We have an MC that is live (or still live) and generates
  // events, make sure the event receiver is set properly.
  cIpmiMc *er = m_domain->GetEventRcvr();
  
  if ( !er )
       return;

  unsigned int event_rcvr = er->GetAddress();

  // Don't bother if we have no possible event receivers.
  if ( !event_rcvr )
       return;

  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdGetEventReceiver );
  cIpmiMsg rsp;

  rv = SendCommand( msg, rsp );

  if ( rv != SA_OK )
       // No care about return values, if this fails it will be done
       //   again later.
       return;

  if ( rsp.m_data[0] != 0 )
     {
       // Error getting the event receiver, report it.
       stdlog << "Could not get event receiver for MC at " << m_addr.m_slave_addr << " !\n";
       return;
     } 

  if ( rsp.m_data_len < 2 )
     {
       stdlog << "Get event receiver length invalid for MC at " << m_addr.m_slave_addr << " !\n";
       return;
     }

  cIpmiDomain  *domain = m_domain;
  cIpmiMc      *destmc;
  cIpmiAddr    ipmb( eIpmiAddrTypeIpmb, GetChannel(), 0, rsp.m_data[1] );

  destmc = domain->FindMcByAddr( ipmb );

  if ( !destmc || destmc->m_ipmb_event_receiver_support == 0 )
     {
       // The current event receiver doesn't exist or cannot
       // receive events, change it.
       er = m_domain->GetEventRcvr();

       if ( er )
            SendSetEventRcvr( er->GetAddress() );
     }
}


SaErrorT
cIpmiMc::SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp_msg,
                      unsigned int lun, int retries )
{
  cIpmiAddr addr = m_addr;

  addr.m_lun = lun;

  return m_domain->SendCommand( addr, msg, rsp_msg, retries );
}


unsigned int
cIpmiMc::GetChannel() const
{
  if ( m_addr.m_type == eIpmiAddrTypeSystemInterface )
       return dIpmiBmcChannel;

  return m_addr.m_channel;
}


unsigned int
cIpmiMc::GetAddress() const
{
  if ( m_addr.m_type == eIpmiAddrTypeIpmb )
       return m_addr.m_slave_addr;
   
  if ( m_addr.m_type == eIpmiAddrTypeSystemInterface )
       return m_addr.m_channel;

  // Address is ignore for other types.
  return 0;
}


cIpmiRdr *
cIpmiMc::FindRdr( cIpmiRdr *r )
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       int idx = res->FindRdr( r );

       if ( idx != -1 )
	    return r;
     }

  return 0;
}


cIpmiSensor *
cIpmiMc::FindSensor( unsigned int lun, unsigned int sensor_id )
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       cIpmiRdr *r = res->FindRdr( this, SAHPI_SENSOR_RDR,
				   sensor_id, lun );

       if ( r )
	    return (cIpmiSensor *)r;
     }

  return 0;
}


cIpmiSensorHotswap *
cIpmiMc::FindHotswapSensor()
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       cIpmiSensorHotswap *hs = res->GetHotswapSensor();

       if ( hs )
	    return hs;
     }

  return 0;
}

bool
cIpmiMc::DumpControls( cIpmiLog &dump, const char *name ) const
{
  int i;

  // create a list of controls
  cArray<cIpmiControl> controls;

  for( i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       for( int j = 0; j < res->NumRdr(); j++ )
	  {
	    cIpmiRdr *rdr = res->GetRdr( j );

	    cIpmiControl *control = dynamic_cast<cIpmiControl *>( rdr );

	    if ( control )
		 controls.Add( control );
	  }
     }

  if ( controls.Num() == 0 )
       return false;

  char control_device_name[80];
  snprintf( control_device_name, sizeof(control_device_name), "ControlDevice%02x_", GetAddress() );

  // dump controls
  for( i = 0; i < controls.Num(); i++ )
     {
       cIpmiControl *control = controls[i];

       char str[80];
       snprintf( str, sizeof(str), "%s%d", control_device_name, control->Num() );
       control->Dump( dump, str );
     }

  // dump control device
  dump.Begin( "Control", name );
  dump.Entry( "ControlDevices" );

  bool first = true;

  while( controls.Num() )
     {
       cIpmiControl *control = controls.Rem( 0 );

       if ( first )
            first = false;
       else
            dump << ", ";

       dump << control_device_name << control->Num();
     }

  dump << ";\n";

  dump.End();

  return true;
}


void
cIpmiMc::Dump( cIpmiLog &dump, const char *name ) const
{
  char sel_name[80];
  snprintf( sel_name, sizeof(sel_name), "Sel%02x", GetAddress() );

  char fru_name[80];
  snprintf( fru_name, sizeof(fru_name), "Fru%02x", GetAddress() );
  bool fru_inventory = false;

  char control_name[80];
  snprintf( control_name, sizeof(control_name), "Control%02x", GetAddress() );
  bool control = false;

  char sdr_name[80];
  snprintf( sdr_name, sizeof(sdr_name), "Sdr%02x", GetAddress() );

  if ( dump.IsRecursive() )
     {
       if ( m_provides_device_sdrs && m_sdrs )
	    m_sdrs->Dump( dump, sdr_name );

       if ( m_sel && m_sel_device_support )
	    m_sel->Dump( dump, sel_name );

       control = DumpControls( dump, control_name );
     }

  dump.Begin( "Mc", name );

  if ( dump.IsRecursive() )
     {
       if ( m_provides_device_sdrs && m_sdrs )
		      dump.Entry( "Sdr" ) << sdr_name << ";\n";

       if ( m_sel && m_sel_device_support )
	    dump.Entry( "Sel" ) << sel_name << ";\n";

       if ( fru_inventory )
            dump.Entry( "Fru" ) << fru_name << "\n";

       if ( control )
            dump.Entry( "Control" ) << control_name << "\n";
     }

  dump.Entry( "DeviceId" ) << (int)m_device_id << ";\n";
  dump.Entry( "DeviceRevision" ) << (int)m_device_revision << ";\n";
  dump.Entry( "ProvidesDeviceSdr" ) << m_provides_device_sdrs << ";\n";
  dump.Entry( "DeviceAvailable" ) << (m_device_available ? "UpdateInProgress" 
                                         : "NormalOperation" ) << ";\n";
  dump.Entry( "ChassisSupport" ) << m_chassis_support << ";\n";
  dump.Entry( "BridgeSupport" ) << m_bridge_support << ";\n";
  dump.Entry( "IpmbEventGeneratorSupport" ) << m_ipmb_event_generator_support << ";\n";
  dump.Entry( "IpmbEventReceiverSupport" ) << m_ipmb_event_receiver_support << ";\n";
  dump.Entry( "FruInventorySupport" ) << m_fru_inventory_support << ";\n";
  dump.Entry( "SelDeviceSupport" ) << m_sel_device_support << ";\n";
  dump.Entry( "SdrRepositorySupport" ) << m_sdr_repository_support << ";\n";
  dump.Entry( "SensorDeviceSupport" ) << m_sensor_device_support << ";\n";
  dump.Entry( "FwVersion" ) << (int)m_major_fw_revision
			    << ", " << (int)m_minor_fw_revision << ";\n";
  dump.Entry( "Version" ) << (int)m_major_version << ", "
			  << (int)m_minor_version << ";\n";
  dump.Hex( true );
  dump.Entry( "ManufacturerId" ) << m_manufacturer_id << ";\n";
  dump.Entry( "ProductId" ) << m_product_id << ";\n";
  dump.Hex( false );

  dump.End();
}


bool
cIpmiMc::Populate()
{
  for( int i = 0; i < Num(); i++ )
     {
       cIpmiResource *res = operator[]( i );

       if ( res->Populate() == false )
	    return false;
     }

  return true;
}
