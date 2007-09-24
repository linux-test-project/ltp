/*
 * ipmi_fru_info.h
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

#ifndef dIpmiFruInfo_h
#define dIpmiFruInfo_h


#ifndef dIpmiEntity_h
#include "ipmi_entity.h"
#endif


enum tIpmiAtcaSiteType
{
  eIpmiAtcaSiteTypeAtcaBoard = 0,
  eIpmiAtcaSiteTypePowerEntryModule = 1,
  eIpmiAtcaSiteTypeShelfFruInformation = 2,
  eIpmiAtcaSiteTypeDedicatedShMc = 3,
  eIpmiAtcaSiteTypeFanTray = 4,
  eIpmiAtcaSiteTypeFanFilterTray = 5,
  eIpmiAtcaSiteTypeAlarm = 6,
  eIpmiAtcaSiteTypeAdvancedMcModule = 7,
  eIpmiAtcaSiteTypePMC = 8,
  eIpmiAtcaSiteTypeRearTransitionModule = 9,
  eIpmiAtcaSiteTypeMicroTcaCarrierHub = 10,
  eIpmiAtcaSiteTypePowerModule = 11,
  eIpmiAtcaSiteTypeUnknown
};


SaHpiEntityTypeT MapAtcaSiteTypeToEntity( tIpmiAtcaSiteType type );


class cIpmiFruInfo
{
  unsigned int     m_addr;
  unsigned int     m_fru_id;

  unsigned int     m_slot;
  SaHpiEntityTypeT m_entity;

  tIpmiAtcaSiteType m_site;

  // only valid if m_fru_id == 0 
  // dIpmiMcThreadXXXX properties
  unsigned int m_properties; 

public:
  cIpmiFruInfo( unsigned int addr, unsigned int fru_id,
		SaHpiEntityTypeT entity, unsigned int slot,
		tIpmiAtcaSiteType site,
                unsigned int properties );
  virtual ~cIpmiFruInfo();

  unsigned int Address() { return m_addr; }
  unsigned int FruId()   { return m_fru_id; }
  SaHpiEntityTypeT &Entity() { return m_entity; }
  unsigned int &Slot()    { return m_slot; }
  tIpmiAtcaSiteType &Site() { return m_site; }
  unsigned int Properties()
  {
    return m_properties;
  }

  
  virtual cIpmiEntityPath CreateEntityPath( const cIpmiEntityPath &top,
					    const cIpmiEntityPath &bottom );
};


class cIpmiFruInfoContainer
{
  GList *m_fru_info;

public:
  cIpmiFruInfoContainer();
  ~cIpmiFruInfoContainer();

  cIpmiFruInfo *FindFruInfo( unsigned int addr, unsigned int fru_id ) const;
  bool          AddFruInfo( cIpmiFruInfo *fru_info );
  bool          RemFruInfo( cIpmiFruInfo *fru_info );

  cIpmiFruInfo *NewFruInfo( unsigned int addr, unsigned int fru_id,
			    SaHpiEntityTypeT entity, unsigned int slot,
                            tIpmiAtcaSiteType site, unsigned int properties );
  cIpmiFruInfo *NewFruInfo( unsigned int addr, unsigned int fru_id );
  unsigned int GetFreeSlotForOther( unsigned int addr );

  GList *GetFruInfoList() const { return m_fru_info; }
};


#endif
