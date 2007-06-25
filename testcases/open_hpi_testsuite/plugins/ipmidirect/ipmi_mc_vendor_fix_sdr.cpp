/*
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
 *     Pierre Sangouard <psangouard@eso-tech.com>
 */

#include "ipmi_mc_vendor_fix_sdr.h"
#include "ipmi_utils.h"
#include "ipmi_log.h"
#include "ipmi_mc.h"
#include <errno.h>
#include <assert.h>

mc_sdr_patch_t sdr_patch_fboard[] =
{
    // ATCA Front Board
    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        eIpmiEntityIdPicMgFrontBoard, 0x60, false},
    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        ENTITY_DONT_CARE, ENTITY_DONT_CARE, true},
};

mc_sdr_patch_t sdr_patch_IShMC[] =
{
    // ShMC itself
    {eIpmiEntityIdPicmgShelfManager, ENTITY_DONT_CARE,
        eIpmiEntityIdPicmgShelfManager, 0x60, false},
    {eIpmiEntityIdBios, ENTITY_DONT_CARE,
        eIpmiEntityIdPicmgShelfManager, 0x60, false},
    {eIpmiEntityIdSystemManagementSoftware, ENTITY_DONT_CARE,
        eIpmiEntityIdPicmgShelfManager, 0x60, false},

    // Air Filter FRU
    {eIpmiEntityIdPicmgFiltrationUnit, ENTITY_DONT_CARE,
        eIpmiEntityIdPicmgFiltrationUnit, 0x60, false},

    // Chassis Data Module 1
    {eIpmiEntityIdPicmgShelfFruInformation, 0,
        eIpmiEntityIdPicmgShelfFruInformation, 0x60, false},

    // Chassis Data Module 2
    {eIpmiEntityIdPicmgShelfFruInformation, 1,
        eIpmiEntityIdPicmgShelfFruInformation, 0x61, false},

    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        ENTITY_DONT_CARE, ENTITY_DONT_CARE, true},
};

mc_sdr_patch_t sdr_patch_IPEM[] =
{
    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        eIpmiEntityIdPowerSupply, 0x60, false},

    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        ENTITY_DONT_CARE, ENTITY_DONT_CARE, true},
};

mc_sdr_patch_t sdr_patch_IFanTray[] =
{
    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        eIpmiEntityIdCoolingUnit, 0x60, false},

    {ENTITY_DONT_CARE, ENTITY_DONT_CARE,
        ENTITY_DONT_CARE, ENTITY_DONT_CARE, true},
};

// Entries in this table are for boards
// with badly formed SDRs that have been tested
// with the plugin. As soon as a fixed firmware
// is available from the vendor, the corresponding
// entry will be removed from the table
mc_patch_t mc_patch[] =
{
    {0x157, 0x80A,  sdr_patch_fboard},   // Tested with firmware <= 1.01
    {0x157, 0x808,  sdr_patch_fboard},   // Tested with firmware <= 1.10
    {0x157, 0x841,  sdr_patch_IShMC},    // Tested with firmware = 5.01
    {0x157, 0x850,  sdr_patch_IPEM},     // Tested with firmware = 1.04
    {0x157, 0x870,  sdr_patch_IFanTray}, // Tested with firmware = 1.02
    {0,     0,      NULL},
};


cIpmiMcVendorFixSdr::cIpmiMcVendorFixSdr( unsigned int manufacturer_id, unsigned int product_id )
  : cIpmiMcVendor( manufacturer_id, product_id, "Some MC" )
{
}


cIpmiMcVendorFixSdr::~cIpmiMcVendorFixSdr()
{
}


bool
cIpmiMcVendorFixSdr::InitMc( cIpmiMc *mc, const cIpmiMsg &devid )
{
  stdlog << "InitMc : Found Mc with SDR to fix.\n";

  m_sdr_patch = NULL;

  stdlog << "Manuf " << m_manufacturer_id << " Product " << m_product_id << ".\n";

  for ( int i = 0; mc_patch[i].sdr_patch != NULL; i++ )
  {
      if (( mc_patch[i].manufacturer_id == m_manufacturer_id )
          && ( mc_patch[i].product_id == m_product_id ))
       {
          m_sdr_patch = mc_patch[i].sdr_patch;
          break;
      }
  }

  assert( m_sdr_patch != NULL ); 

  return true;
}

bool
cIpmiMcVendorFixSdr::ProcessSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs )
{
  unsigned char *pEntityId, *pEntityInstance;
  stdlog << "ProcessSdr : Special Mc found.\n";

  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       if ( (sdr->m_type == eSdrTypeMcDeviceLocatorRecord)
           || (sdr->m_type == eSdrTypeFruDeviceLocatorRecord) )
          {
                pEntityId = &sdr->m_data[12];
                pEntityInstance = &sdr->m_data[13];
          }
       else if ( (sdr->m_type == eSdrTypeFullSensorRecord)
                || (sdr->m_type == eSdrTypeCompactSensorRecord) )
          {
                pEntityId = &sdr->m_data[8];
                pEntityInstance = &sdr->m_data[9];
          }
       else
          {
                stdlog << "Type is " << sdr->m_type << "\n";
                continue;
          }

       stdlog << "Old Type " << sdr->m_type << " Ent ID " << *pEntityId << " Inst " << *pEntityInstance << "\n";

       for ( int j = 0; m_sdr_patch[j].last_entry != true; j++ )
       {
           if (( m_sdr_patch[j].old_entity_id == ENTITY_DONT_CARE )
               || ( m_sdr_patch[j].old_entity_id == *pEntityId ))
           {
               if (( m_sdr_patch[j].old_entity_instance == ENTITY_DONT_CARE )
                   || ( m_sdr_patch[j].old_entity_instance == *pEntityInstance ))
               {
                   *pEntityId       = m_sdr_patch[j].new_entity_id;
                   *pEntityInstance = m_sdr_patch[j].new_entity_instance;
                   break;
               }
           }
       }

       stdlog << "New Type " << sdr->m_type << " Ent ID " << *pEntityId << " Inst " << *pEntityInstance << "\n";
     }

  return true;
}
