/*
 * ipmi_mc_vendor.cpp
 *
 * Copyright (c) 2004 by FORCE Computers
 * Copyright (c) 2005-2006 by ESO Technologies.
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

#include <assert.h>

extern "C" {
#include "SaHpiAtca.h"
}

#include "ipmi_mc_vendor.h"
#include "ipmi_mc_vendor_force.h"
#include "ipmi_mc_vendor_intel.h"
#include "ipmi_mc_vendor_sun.h"
#include "ipmi_mc_vendor_fix_sdr.h"
#include "ipmi_domain.h"
#include "ipmi_control_fan.h"
#include "ipmi_control_atca_led.h"
#include "ipmi_watchdog.h"


cIpmiMcVendorFactory *cIpmiMcVendorFactory::m_factory = 0;
static int use_count = 0;
static cThreadLock lock;


cIpmiMcVendorFactory::cIpmiMcVendorFactory()
  : m_mc_vendors( 0 ), m_default( new cIpmiMcVendor( 0, 0, "default MC vendor" ) )
{
}


cIpmiMcVendorFactory::~cIpmiMcVendorFactory()
{
  while( m_mc_vendors )
     {
       cIpmiMcVendor *mv = (cIpmiMcVendor *)m_mc_vendors->data;
       m_mc_vendors = g_list_remove( m_mc_vendors, mv );
 
       delete mv;
     }

  if ( m_default )
       delete m_default;
}


void
cIpmiMcVendorFactory::InitFactory()
{
  lock.Lock();

  if ( m_factory == 0 )
     {
       m_factory = new cIpmiMcVendorFactory;

       // Force ShMC specific stuff
       m_factory->Register( new cIpmiMcVendorForceShMc( 0x1011 ) );
       m_factory->Register( new cIpmiMcVendorForceShMc( 0x1080 ) );
       // Intel BMC specific stuff
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x000C ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x001B ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0022 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0026 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0028 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0100 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x4311 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0811 ) );
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0900 ) ); /*HSC*/
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0911 ) ); /*HSC*/
       m_factory->Register( new cIpmiMcVendorIntelBmc( 0x0A0C ) ); /*HSC*/
       // Sun BMC specific stuff
       m_factory->Register( new cIpmiMcVendorSunBmc( 0x4701 ) );

       // Enabling this code will fix badly formed SDR
       // found on various boards tested with the plugin
       // It would be much better if vendors fixed those
       // This is why this code is not enabled by default
#ifdef FIX_BAD_SDR
       for ( int i = 0; mc_patch[i].sdr_patch != NULL; i++ )
       {
           m_factory->Register( new cIpmiMcVendorFixSdr( mc_patch[i].manufacturer_id,
                                                            mc_patch[i].product_id ) );
       }
#endif
     }

  use_count++;

  lock.Unlock();
}


void
cIpmiMcVendorFactory::CleanupFactory()
{
  lock.Lock();

  use_count--;
  assert( use_count >= 0 );

  if ( use_count == 0 )
     {
       delete m_factory;
       m_factory = 0;
     }

  lock.Unlock();
}


bool
cIpmiMcVendorFactory::Register( cIpmiMcVendor *mv )
{
  if ( Find( mv->m_manufacturer_id, mv->m_product_id ) )
     {
       assert( 0 );
       return false;
     }

  m_mc_vendors = g_list_append( m_mc_vendors, mv );

  return true;
}


bool
cIpmiMcVendorFactory::Unregister( unsigned int manufacturer_id,
                                  unsigned int product_id )
{
  cIpmiMcVendor *mv = Find( manufacturer_id, product_id );

  if ( !mv )
       return false;

  m_mc_vendors = g_list_remove( m_mc_vendors, mv );

  return true;
}


cIpmiMcVendor *
cIpmiMcVendorFactory::Find( unsigned int manufacturer_id,
                            unsigned int product_id )
{
  GList *list = m_mc_vendors;

  while( list )
     {
       cIpmiMcVendor *mv = (cIpmiMcVendor *)list->data;

       if (    mv->m_manufacturer_id == manufacturer_id
            && mv->m_product_id == product_id )
            return mv;

       list = g_list_next( list );
     }

  return 0;
}


cIpmiMcVendor *
cIpmiMcVendorFactory::Get( unsigned int manufacturer_id,
                           unsigned int product_id )
{
  cIpmiMcVendor *mv = Find( manufacturer_id, product_id );

  if ( mv )
       return mv;

  return m_default;
}


SaHpiEntityLocationT cIpmiMcVendor::m_unique_instance = 256;


cIpmiMcVendor::cIpmiMcVendor( unsigned int manufacturer_id,
                              unsigned int product_id,
                              const char *desc )
  : m_manufacturer_id( manufacturer_id ),
    m_product_id( product_id )
{
  strncpy( m_description, desc, sizeof(m_description) - 1 );
  m_description[sizeof(m_description) - 1] = 0;
}


cIpmiMcVendor::~cIpmiMcVendor()
{
}


bool
cIpmiMcVendor::InitMc( cIpmiMc * /*mc*/, const cIpmiMsg & /*devid*/ )
{
  return true;
}


void
cIpmiMcVendor::CleanupMc( cIpmiMc * /*mc*/ )
{
}


bool
cIpmiMcVendor::ProcessSdr( cIpmiDomain * /*domain*/, cIpmiMc * /*mc*/, cIpmiSdrs * /*sdrs*/ )
{
  return true;
}

bool
cIpmiMcVendor::ProcessFru( cIpmiInventory * /*inv*/, cIpmiMc * /*mc*/,
                       unsigned int /*sa*/, SaHpiEntityTypeT /*type*/)
{
  return true;
}


bool
cIpmiMcVendor::CreateRdrs( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  if ( CreateResources( domain, source_mc, sdrs ) == false )
       return false;

  if ( CreateSensors( domain, source_mc, sdrs ) == false )
       return false;

  if ( CreateControls( domain, source_mc, sdrs ) == false )
       return false;

  if ( CreateSels( domain, source_mc, sdrs ) == false )
       return false;

  if ( CreateInvs( domain, source_mc, sdrs ) == false )
       return false;

  if ( CreateWatchdogs( domain, source_mc ) == false )
       return false;

  return true;
}


bool
cIpmiMcVendor::CreateResources( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  assert( source_mc );

  bool found = false;

  // Make sure that there is a mcdlr
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
  {
      cIpmiSdr *sdr = sdrs->Sdr( i );
      if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
	  {
	    found = true;
        break;
	  }
  }

  // there must be a mcdlr !
  if ( found == false )
  {
      stdlog << "WARNING : MC " << source_mc->GetAddress() << " NO MC Device Locator Record\n";
      return false;
  }

  // create one resource per mcdlr and fdlr
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       cIpmiAddr addr;
       unsigned int fru_id;

       if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
	  {
	    addr.m_slave_addr = sdr->m_data[5];
	    addr.m_channel    = sdr->m_data[6] & 0xf;
        fru_id = 0;
	  }
       else if ( sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
	  {
        // We care only about logical FRUs
	    if ( (sdr->m_data[7] & 0x80) == 0 )
            continue;
	    addr.m_slave_addr = sdr->m_data[5];
	    addr.m_channel    = (sdr->m_data[8]  >> 4) & 0xf;
	    fru_id = sdr->m_data[6];
	  }
       else
	    continue;

      stdlog << "CreateResources FRU " << fru_id << "\n";

      if ( addr.m_slave_addr != source_mc->GetAddress() )
          stdlog << "WARNING : SDR slave address " << addr.m_slave_addr << " NOT equal to MC slave address " << (unsigned char)source_mc->GetAddress() << "\n";

      if ( addr.m_channel != source_mc->GetChannel() )
          stdlog << "WARNING : SDR channel " << addr.m_channel << " NOT equal to MC channel " << source_mc->GetChannel() << "\n";

       if ( FindOrCreateResource( domain, source_mc, fru_id, sdr, sdrs ) == false )
	    return false;
     }

  return true;
}


cIpmiResource *
cIpmiMcVendor::FindResource( cIpmiDomain *domain, cIpmiMc *mc,
                             unsigned int fru_id,
                             SaHpiEntityTypeT type,
                             SaHpiEntityLocationT instance,
                             cIpmiSdrs *sdrs )
{
  assert( mc );

  stdlog << "FindResource mc " << mc->GetAddress() << " FRU " << fru_id << " type " << type << " instance " << instance << "\n";

  cIpmiEntityPath ep = CreateEntityPath( domain, mc->GetAddress(), fru_id,
					 type, instance, sdrs );

  stdlog << "Looking for resource: " << ep << ".\n";

  cIpmiResource *res = mc->FindResource( ep );

  return res;
}

cIpmiResource *
cIpmiMcVendor::FindResource( cIpmiDomain *domain, cIpmiMc *mc,
				     unsigned int fru_id,
				     cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  assert( mc );

  SaHpiEntityTypeT     type = SAHPI_ENT_UNKNOWN;
  SaHpiEntityLocationT instance = GetUniqueInstance();

  if ( sdr )
     {
       if (    sdr->m_type == eSdrTypeMcDeviceLocatorRecord 
	    || sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[12];
	    instance = (SaHpiEntityLocationT)sdr->m_data[13];
	  }
       else if ( sdr->m_type == eSdrTypeFullSensorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[8];
	    instance = (SaHpiEntityLocationT)sdr->m_data[9];	    
	  }
       else
	    assert( 0 );
     }
  stdlog << "FindResource mc " << mc->GetAddress() << " FRU " << fru_id << " type " << type << " instance " << instance << "\n";

  cIpmiEntityPath ep = CreateEntityPath( domain, mc->GetAddress(), fru_id,
					 type, instance, sdrs );

  stdlog << "Looking for resource: " << ep << ".\n";

  cIpmiResource *res = mc->FindResource( ep );

  return res;
}

cIpmiResource *
cIpmiMcVendor::FindOrCreateResource( cIpmiDomain *domain, cIpmiMc *mc,
				     unsigned int fru_id,
				     cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  assert( mc );

  SaHpiEntityTypeT     type = SAHPI_ENT_UNKNOWN;
  SaHpiEntityLocationT instance = GetUniqueInstance();

  if ( sdr )
     {
       if (    sdr->m_type == eSdrTypeMcDeviceLocatorRecord 
	    || sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[12];
	    instance = (SaHpiEntityLocationT)sdr->m_data[13];
	  }
       else if ( sdr->m_type == eSdrTypeFullSensorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[8];
	    instance = (SaHpiEntityLocationT)sdr->m_data[9];	    
	  }
       else
	    assert( 0 );
     }
  stdlog << "FindOrCreateResource mc " << mc->GetAddress() << " FRU " << fru_id << " type " << type << " instance " << instance << "\n";

  cIpmiEntityPath ep = CreateEntityPath( domain, mc->GetAddress(), fru_id,
					 type, instance, sdrs );

  stdlog << "Looking for resource: " << ep << ".\n";

  cIpmiResource *res = mc->FindResource( ep );

  if ( res )
       return res;

  return CreateResource( domain, mc, fru_id, sdr, sdrs );
}


cIpmiResource *
cIpmiMcVendor::CreateResource( cIpmiDomain *domain, cIpmiMc *mc,
			       unsigned int fru_id, cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  // create a new resource
  cIpmiResource *res = new cIpmiResource( mc, fru_id );

  SaHpiEntityTypeT     type = SAHPI_ENT_UNKNOWN;
  SaHpiEntityLocationT instance = GetUniqueInstance();

  if ( sdr )
     {
       if (    sdr->m_type == eSdrTypeMcDeviceLocatorRecord 
	    || sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[12];
	    instance = (SaHpiEntityLocationT)sdr->m_data[13];
	  }
       else if ( sdr->m_type == eSdrTypeFullSensorRecord )
	  {
	    type     = (SaHpiEntityTypeT)sdr->m_data[8];
	    instance = (SaHpiEntityLocationT)sdr->m_data[9];	    
	  }
       else
	    assert( 0 );
     }

  res->EntityPath() = CreateEntityPath( domain, mc->GetAddress(), fru_id,
                                        type, instance, sdrs );

  if (    sdr
       && (    sdr->m_type == eSdrTypeMcDeviceLocatorRecord 
            || sdr->m_type == eSdrTypeFruDeviceLocatorRecord ) )
     {
       stdlog << "Adding FRU " << fru_id << " ";
       // set mc id string
       res->ResourceTag().SetIpmi( sdr->m_data + 15 );
       res->Oem() = sdr->m_data[14];
       res->IsFru() = true;
     }

  // create rpt entry
  stdlog << "adding resource: " << res->EntityPath() << ".\n";

  // add res to mc
  mc->AddResource( res );

  return res;
}


// TODO:
//    Handling of entity association records.
//    This is the reason for the parameter sdrs.
cIpmiEntityPath
cIpmiMcVendor::CreateEntityPath( cIpmiDomain *domain, unsigned int mc_addr, unsigned int fru_id, 
                                 SaHpiEntityTypeT type,
                                 SaHpiEntityLocationT instance, cIpmiSdrs * /*sdrs*/ )
{
  // find fru info
  cIpmiFruInfo *fi = domain->FindFruInfo( mc_addr, fru_id );

  if ( ( fi == NULL ) && ( fru_id != 0) )
  {
      fi = domain->NewFruInfo( mc_addr, fru_id );
  }

  cIpmiEntityPath bottom;

  // clear bit 7
  instance &= 0x7f;

  if ( instance >= 0x60 )
       instance -= 0x60;

  switch (type)
  {
    default:
        break;

    case eIpmiEntityIdPicMgFrontBoard:
        type = SaHpiEntityTypeT(ATCAHPI_ENT_PICMG_FRONT_BLADE);
        break;

    case eIpmiEntityIdPicMgRearTransitionModule:
        type = SAHPI_ENT_BACK_PANEL_BOARD;
        break;

    case eIpmiEntityIdPicMgAdvancedMcModule:
        type = SaHpiEntityTypeT(ATCAHPI_ENT_AMC);
        break;

    case eIpmiEntityIdPicMgMicroTcaCarrierHub:
        type = SAHPI_ENT_SWITCH_BLADE;
        break;

    case eIpmiEntityIdPicmgShelfManager:
        type = SAHPI_ENT_SHELF_MANAGER;
        break;

    case eIpmiEntityIdPicmgFiltrationUnit:
        type = SaHpiEntityTypeT(ATCAHPI_ENT_FILTRATION_UNIT);
        break;

    case eIpmiEntityIdPicmgShelfFruInformation:
        type = SaHpiEntityTypeT(ATCAHPI_ENT_SHELF_FRU_DEVICE);
        break;

    case eIpmiEntityIdPicmgAlarmPanel:
        type = SAHPI_ENT_ALARM_MANAGER;
        break;
  }

  bottom.SetEntry( 0, type, instance );
  bottom.AppendRoot( 1 );

  cIpmiEntityPath top = domain->EntityRoot();

  if ( fi )
       return fi->CreateEntityPath( top, bottom );

  // fru info not found => use default entity path
  cIpmiEntityPath ep = bottom;
  ep += top;

  return ep;
}


static cIpmiSensor *
FindSensor( GList *list, unsigned int num, unsigned char lun )
{
  for( ; list; list = g_list_next( list ) )
     {
       cIpmiSensor *sensor = (cIpmiSensor *)list->data;

       if (    sensor->Num() == num
            && sensor->Lun() == lun )
            return sensor;
     }

  return 0;
}


bool
cIpmiMcVendor::CreateSensors( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  GList *old_sensors = domain->GetSdrSensors( source_mc );
  GList *new_sensors = 0;
  GList *sensors     = GetSensorsFromSdrs( domain, source_mc, sdrs );
  unsigned int parent_fru_id;
  SaHpiEntityTypeT     type, parent_type;
  SaHpiEntityLocationT instance, parent_instance;

  while( sensors )
     {
       cIpmiSensor *sensor = (cIpmiSensor *)sensors->data;
       sensors = g_list_remove( sensors, sensor );

       cIpmiSensor *old_sensor = FindSensor( old_sensors, sensor->Num(), sensor->Lun() );

       if ( old_sensor && sensor->Cmp( *old_sensor ) )
          {
            // sensor already there, use old one
            delete sensor;
            old_sensor->HandleNew( domain );
            old_sensors = g_list_remove( old_sensors, old_sensor );
            new_sensors = g_list_append( new_sensors, old_sensor );
            continue;
          }

       if ( old_sensor )
          {
            // remove the old sensor
            old_sensors = g_list_remove( old_sensors, old_sensor );
            old_sensor->Resource()->RemRdr( old_sensor );
            delete old_sensor;
          }

       // check if the sensor is defined twice
       if ( FindSensor( new_sensors, sensor->Num(), sensor->Lun() ) )
          {
            stdlog << "sensor " << sensor->IdString() << " defined twice in SDR !\n";
            delete sensor;
            continue;
          }

       cIpmiSdr *sdr = sensor->GetSdr();
       
       if ( sdr == 0 )
       {
           sdr = sdrs->FindSdr( sensor->Mc() );

           if (!sdr)
           {
               delete sensor;
               continue;
           }
       }

       type     = (SaHpiEntityTypeT)sdr->m_data[8];
       instance = (SaHpiEntityLocationT)sdr->m_data[9];

       parent_fru_id = sdrs->FindParentFru( type,
                                            instance,
                                            parent_type,
                                            parent_instance
                                          );

       cIpmiResource *res = FindResource( domain, sensor->Mc(),
                                          parent_fru_id, parent_type, parent_instance,
                                          sdrs );
       if (!res )
       {
           delete sensor;
           continue;
       }

       new_sensors = g_list_append( new_sensors, sensor );
       sensor->HandleNew( domain );
       res->AddRdr( sensor );
     }

  // destry old sensors
  while( old_sensors )
     {
       cIpmiSensor *sensor = (cIpmiSensor *)old_sensors->data;
       old_sensors = g_list_remove( old_sensors, sensor );
       sensor->Resource()->RemRdr( sensor );
       delete sensor;
     }

  // set new sdr sensors
  domain->SetSdrSensors( source_mc, new_sensors );

  return true;
}


GList *
cIpmiMcVendor::GetSensorsFromSdrs( cIpmiDomain *domain, cIpmiMc *source_mc,
                                   cIpmiSdrs *sdrs )
{
  GList *sensors = 0;

  // create a list of full sensor records
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       if ( sdr->m_type != eSdrTypeFullSensorRecord )
	    continue;

       GList *l = CreateSensorFromFullSensorRecord( domain, source_mc, sdr, sdrs );

       if ( l )
	    sensors = g_list_concat( sensors, l );
     }

  return sensors;
}


GList *
cIpmiMcVendor::CreateSensorFromFullSensorRecord( cIpmiDomain *domain, cIpmiMc *source_mc,
                                                 cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  GList *list = 0;

  tIpmiSensorType sensor_type = (tIpmiSensorType)sdr->m_data[12];

  if ( sensor_type == eIpmiSensorTypeAtcaHotSwap )
       list = CreateSensorHotswap( domain, source_mc, sdr, sdrs );
  else
     {
       tIpmiEventReadingType reading_type = (tIpmiEventReadingType)sdr->m_data[13];

       if ( reading_type == eIpmiEventReadingTypeThreshold )
            list = CreateSensorThreshold( domain, source_mc, sdr, sdrs );
       else
            list = CreateSensorDefault( domain, source_mc, sdr, sdrs );
     }

  for( GList *l = list; l; l = g_list_next( l ) )
     {
       cIpmiSensor *s = (cIpmiSensor *)l->data;

       if ( s->GetSdr() == 0 )
	    s->SetSdr( sdr );
     }

  return list;
}


GList *
cIpmiMcVendor::CreateSensorHotswap( cIpmiDomain *domain, cIpmiMc *source_mc,
                                    cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  cIpmiMc *mc = source_mc; // FindMcBySdr( domain, sdr );
  assert( mc );

  cIpmiSensorHotswap *hs = new cIpmiSensorHotswap( mc );
  hs->SourceMc() = source_mc;

  if ( !hs->GetDataFromSdr( mc, sdr ) )
     {
       delete hs;
       return 0;
     }

  CreateSensorEntityPath( domain, hs, source_mc, sdr, sdrs );

  return g_list_append( 0, hs );
}


GList *
cIpmiMcVendor::CreateSensorThreshold( cIpmiDomain *domain, cIpmiMc *source_mc,
                                      cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  cIpmiMc *mc = source_mc; // FindMcBySdr( domain, sdr );
  assert( mc );

  cIpmiSensorThreshold *ts = new cIpmiSensorThreshold( mc );
  ts->SourceMc() = source_mc;

  if ( !ts->GetDataFromSdr( mc, sdr ) )
     {
       delete ts;

       return 0;
     }

  CreateSensorEntityPath( domain, ts, source_mc, sdr, sdrs );

  return g_list_append( 0, ts );
}


GList *
cIpmiMcVendor::CreateSensorDiscrete( cIpmiDomain *domain, cIpmiMc *source_mc,
                                     cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  cIpmiMc *mc = source_mc; //FindMcBySdr( domain, sdr );
  assert( mc );

  cIpmiSensorDiscrete *ds = new cIpmiSensorDiscrete( mc );
  ds->SourceMc() = source_mc;

  if ( !ds->GetDataFromSdr( mc, sdr ) )
     {
       delete ds;

       return 0;
     }

  CreateSensorEntityPath( domain, ds, source_mc, sdr, sdrs );

  return g_list_append( 0, ds );
}


GList *
cIpmiMcVendor::CreateSensorDefault( cIpmiDomain *domain, cIpmiMc *source_mc,
                                    cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  return CreateSensorDiscrete( domain, source_mc, sdr, sdrs );
}


void
cIpmiMcVendor::CreateSensorEntityPath( cIpmiDomain *domain, cIpmiSensor *s, 
                                       cIpmiMc *source_mc,
                                       cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  SaHpiEntityTypeT     type, parent_type;
  SaHpiEntityLocationT instance, parent_instance;
  unsigned int parent_fru_id;

  if ( sdr )
     {
       type     = (SaHpiEntityTypeT)sdr->m_data[8];
       instance = (SaHpiEntityLocationT)sdr->m_data[9];
     }
  else
     {
       type     = SAHPI_ENT_UNKNOWN;
       instance = GetUniqueInstance();
     }

  parent_fru_id = sdrs->FindParentFru( type,
                                       instance,
                                       parent_type,
                                       parent_instance
                                     );

  stdlog << "CreateSensorEntityPath mc " << source_mc->GetAddress() << " FRU " << parent_fru_id << " type " << type << " instance " << instance << "\n";

  cIpmiEntityPath parent_ep = CreateEntityPath( domain, source_mc->GetAddress(), parent_fru_id,
                                          parent_type, parent_instance, sdrs );

  if ((type != parent_type)
      || (instance != parent_instance))
  {
    // clear bit 7
    instance &= 0x7f;

    if ( instance >= 0x60 )
       instance -= 0x60;

    cIpmiEntityPath child_ep;

    child_ep.SetEntry( 0, type, instance );
    child_ep.AppendRoot( 1 );

    child_ep += parent_ep;

    s->EntityPath() = child_ep;
  }
  else
  {
    s->EntityPath() = parent_ep;
  }
}


cIpmiMc *
cIpmiMcVendor::FindMcBySdr( cIpmiDomain *domain, cIpmiSdr *sdr )
{
  switch( sdr->m_type )
     {
       case eSdrTypeCompactSensorRecord:
       case eSdrTypeFullSensorRecord:
       case eSdrTypeMcDeviceLocatorRecord:
       case eSdrTypeFruDeviceLocatorRecord:
            {
              cIpmiAddr addr( eIpmiAddrTypeIpmb, 0, 0, sdr->m_data[5] );

              return domain->FindMcByAddr( addr );
            }

       default:
            break;
     }

  return 0;
}


bool
cIpmiMcVendor::CreateControls( cIpmiDomain *domain, cIpmiMc *source_mc, 
                               cIpmiSdrs *sdrs )
{
  // controls only for device SDR
  if ( source_mc == 0 )
       return true;

  if ( source_mc->IsTcaMc() )
     {
       return CreateControlsAtca( domain, source_mc, sdrs );
     }

  return true;
}


bool
cIpmiMcVendor::CreateWatchdogs( cIpmiDomain *domain, cIpmiMc *mc )
{
  cIpmiResource *res;

  for ( int i = 0; i < mc->NumResources(); i++ )
  {
      res = mc->GetResource ( i );

      if ( res == 0 )
          continue;

      stdlog << "CreateWatchdogs: addr " << mc->GetAddress() << " FruId " <<
                  res->FruId() << "\n";

      if (res->FruId() == 0) 
      {
          cIpmiMsg  msg( eIpmiNetfnApp, eIpmiCmdGetWatchdogTimer );
          cIpmiMsg  rsp;

          if (mc->IsRmsBoard() && 
              res->EntityPath().GetEntryType(0) != SAHPI_ENT_SYSTEM_BOARD) 
              continue;

          /* Do an IPMI GetWatchdogTimer command to verify this feature. */
          msg.m_data_len = 0;
          SaErrorT rv = res->SendCommand( msg, rsp );
          if (rv != 0 ||  rsp.m_data[0] != 0) {
              stdlog << "CreateWatchdogs: IPMI error " << rv << " ccode " <<
                         rsp.m_data[0] << "\n";
              continue;
          }

          /* Everything is valid, create the Watchdog RDR */
          stdlog << "CreateWatchdogs Resource type " << res->EntityPath().GetEntryType(0) << " instance " << res->EntityPath().GetEntryInstance(0) << "\n";
        
          cIpmiRdr *wd = new cIpmiWatchdog::cIpmiWatchdog( mc, SAHPI_DEFAULT_WATCHDOG_NUM, 0 );
        
          wd->EntityPath() = res->EntityPath();
        
          wd->IdString().SetAscii( "Watchdog", SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH );
        
          res->AddRdr( wd );
      }
  }

  return true;
}


bool
cIpmiMcVendor::CreateControlsAtca( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs )
{
  cIpmiResource *res;

  for ( int i = 0; i < mc->NumResources(); i++ )
  {
      res = mc->GetResource ( i );

      if ( res == 0 )
          continue;

      if ( res->IsFru() )
      {
        stdlog << "CreateControlsAtca Resource type " << res->EntityPath().GetEntryType(0) << " instance " << res->EntityPath().GetEntryInstance(0) << " FRU " << res->FruId() << "\n";
        CreateControlAtcaLed( domain, res, sdrs );
        CreateControlAtcaFan( domain, res, sdrs );
      }
  }

  return true;
}

bool
cIpmiMcVendor::CreateControlAtcaLed( cIpmiDomain *domain, cIpmiResource *res,
                                     cIpmiSdrs *sdrs )
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetFruLedProperties );
  msg.m_data[0] = dIpmiPicMgId;
  msg.m_data[1] = res->FruId();
  msg.m_data_len = 2;

  cIpmiMsg rsp;

  SaErrorT rv = res->SendCommand( msg, rsp );

  if (    rv != SA_OK
       || rsp.m_data_len < 4
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "cannot get FRU Led properties !\n";
       return true;
     }

  unsigned char num_app_leds = rsp.m_data[3];

  if ( num_app_leds > 0xFB )
    num_app_leds = 0;

  for (int i = 0; i <= (3+num_app_leds); i++)
  {
    if ( ( i <= 3 ) && ( rsp.m_data[2] & (1<<i) ) == 0 )
            continue;

    cIpmiMsg ledmsg( eIpmiNetfnPicmg, eIpmiCmdGetLedColorCapabilities );
    ledmsg.m_data[0] = dIpmiPicMgId;
    ledmsg.m_data[1] = res->FruId();
    ledmsg.m_data[2] = i;
    ledmsg.m_data_len = 3;

    cIpmiMsg ledrsp;

    rv = res->SendCommand( ledmsg, ledrsp );

    if (   rv != SA_OK
        || ledrsp.m_data_len < 5
        || ledrsp.m_data[0] != eIpmiCcOk
        || ledrsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "cannot get Led color capabilities !\n";
        continue;
    }

    unsigned char led_color_capabilities = ledrsp.m_data[2] & 0x7E;
    unsigned char led_default_local_color = ledrsp.m_data[3];
    unsigned char led_default_override_color = ledrsp.m_data[4];

    ledmsg.m_cmd = eIpmiCmdGetFruLedState;

    rv = res->SendCommand( ledmsg, ledrsp );

    if (   rv != SA_OK
        || ledrsp.m_data_len < 6
        || ledrsp.m_data[0] != eIpmiCcOk
        || ledrsp.m_data[1] != dIpmiPicMgId )
    {
        continue;
    }

    if ( (ledrsp.m_data[2] & 0x01) == 0 )
        led_default_local_color = 0;

    cIpmiControlAtcaLed *l = new cIpmiControlAtcaLed( res->Mc(), ATCAHPI_CTRL_NUM_BLUE_LED+i,
                                                      led_color_capabilities,
                                                      led_default_local_color,
                                                      led_default_override_color );
    l->EntityPath() = res->EntityPath();

    char ledname[32];
    if (i == 0)
        snprintf (ledname, sizeof(ledname), "Blue LED");
    else
        snprintf (ledname, sizeof(ledname), "LED %d", i);

    l->IdString().SetAscii( ledname, SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH );

    res->AddRdr( l );
  }

  return true;
}

bool
cIpmiMcVendor::CreateControlAtcaFan( cIpmiDomain *domain, cIpmiResource *res,
				     cIpmiSdrs *sdrs )
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetFanSpeedProperties );
  msg.m_data[0] = dIpmiPicMgId;
  msg.m_data[1] = res->FruId();
  msg.m_data_len = 2;

  cIpmiMsg rsp;

  SaErrorT rv = res->SendCommand( msg, rsp );

  if (    rv != SA_OK
       || rsp.m_data_len < 6
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
     {
       stdlog << "cannot get fan speed properties !\n";
       return true;
     }

  unsigned int min      = rsp.m_data[2];
  unsigned int max      = rsp.m_data[3];
  unsigned int def      = rsp.m_data[4];
  bool         auto_adj = rsp.m_data[5] & 0x80;

  cIpmiControlFan *f = new cIpmiControlFan( res->Mc(), ATCAHPI_CTRL_NUM_FAN_SPEED,
                                            min, max, def,
                                            auto_adj );
  f->EntityPath() = res->EntityPath();

  f->IdString().SetAscii( "Fan Control", SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH );

  res->AddRdr( f );

  return true;
}

bool
cIpmiMcVendor::CreateInvs( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
          {
            if ( (sdr->m_data[8] & 8) == 0 )
                      continue;
          }
       else if ( sdr->m_type != eSdrTypeFruDeviceLocatorRecord )
            continue;

       if ( CreateInv( domain, source_mc, sdr, sdrs ) == false )
            return false;
     }

  return true;
}


bool
cIpmiMcVendor::CreateInv( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdr *sdr, cIpmiSdrs *sdrs )
{
  unsigned int fru_id;
  unsigned int lun;
  unsigned int sa = mc->GetAddress();
  SaHpiEntityTypeT     type;

  if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
     {
       fru_id = 0;
       lun    = 0;
       sa     = sdr->m_data[5];
       type   = (SaHpiEntityTypeT)sdr->m_data[12];
     }
  else
     {
       fru_id = sdr->m_data[6];
       lun    = (sdr->m_data[7] >> 3) & 3;
       type     = SAHPI_ENT_UNKNOWN;
     }

  cIpmiMc *m = mc;
  assert( m );

  cIpmiResource *res = FindResource( domain, m, fru_id, sdr, sdrs );

  if ( !res )
      return true;

  cIpmiInventory *inv = (cIpmiInventory *)res->FindRdr( m, SAHPI_INVENTORY_RDR, fru_id );
  bool need_add = false;

  if ( inv == 0 )
     {
       inv = new cIpmiInventory( m, fru_id );

       inv->IdString().SetIpmi( sdr->m_data + 15 );
       inv->Oem() = sdr->m_data[14];

       inv->Resource() = res;
       ProcessFru(inv, m, sa, type); 
       need_add = true;
     }

  SaErrorT rv = inv->Fetch();

  if ( rv != SA_OK )
     {
       if ( need_add )
            delete inv;

       // Ignore FRU errors
       return true;
     }

  inv->EntityPath() = res->EntityPath();

  if ( !need_add )
       return true;

  res->AddRdr( inv );
  return true;
}


bool
cIpmiMcVendor::CreateSels( cIpmiDomain *domain, cIpmiMc *source_mc, cIpmiSdrs *sdrs )
{
  if ( source_mc == 0 )
       return false;

  if ( !source_mc->SelDeviceSupport() )
       return true;

  cIpmiSdr *mcdlr = sdrs->FindSdr( source_mc );

  if ( mcdlr == 0 )
       return true;

  cIpmiResource *res = FindResource( domain, source_mc, 0, mcdlr, sdrs );

  if ( !res )
      return true;

  // create hpi sel
  stdlog << "adding SEL " << res->EntityPath() << "\n";

  // sel capabilities
  res->m_sel = true;

  return true;
}
