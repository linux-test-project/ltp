/*
 * ipmi_fru.h
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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

#ifndef dIpmiFru_h
#define dIpmiFru_h


#ifndef dIpmiTypeCode_h
#include "ipmi_type_code.h"
#endif


class cIpmiEntity;

enum tIpmiChassisType
{
  eIpmiChassisTypeOther = 1
};


enum tIpmiFruItemType
{
  eIpmiFruItemTypeUnknown,
  eIpmiFruItemTypeData,
  eIpmiFruItemTypeTextBuffer,
  eIpmiFruItemTypeInt,
  eIpmiFruItemTypeRecord,

  // cmds
  eIpmiFruItemTypeDate3,
  eIpmiFruItemTypeCustom,
  eIpmiFruItemTypePad,
};


// item names
#define dIpmiFruItemAssetTag       "AssetTag"
#define dIpmiFruItemChassisType    "ChassisType"
#define dIpmiFruItemCustom         "Custom"
#define dIpmiFruItemData           "Data"
#define dIpmiFruItemFruFileId      "FruFileId"
#define dIpmiFruItemLanguage       "Language"
#define dIpmiFruItemManufacturer   "Manufacturer"
#define dIpmiFruItemMfgDate        "MfgDate"
#define dIpmiFruItemPartNumber     "PartNumber"
#define dIpmiFruItemProductName    "ProductName"
#define dIpmiFruItemProductVersion "ProductVersion"
#define dIpmiFruItemSerialNumber   "SerialNumber"
#define dIpmiFruItemVersion        "Version"


struct tIpmiFruItemDesc
{
  const char       *m_name;
  tIpmiFruItemType  m_type;
};


#define dIpmiFruItemNameLen 32

class cIpmiFruRecord;


class cIpmiFruItem
{
public:
  char             m_name[dIpmiFruItemNameLen];
  tIpmiFruItemType m_type;

  cIpmiFruItem( const char *name, tIpmiFruItemType type );
  ~cIpmiFruItem();

  union
  {
    struct
    {
      unsigned char *m_data;
      unsigned int   m_size;
    } m_data;

    unsigned int      m_int;
    cIpmiTextBuffer   m_text_buffer;
    cIpmiFruRecord   *m_record;
  } m_u;

  void Log();
};


// record names
#define dIpmiFruRecordInternalUseArea "InternalUseArea"
#define dIpmiFruRecordChassisInfoArea "ChassisInfoArea"
#define dIpmiFruRecordBoardInfoArea   "BoardInfoArea"
#define dIpmiFruRecordProductInfoArea "ProductInfoArea"
#define dIpmiFruRecordMultiRecord     "MultiRecord"


#define dIpmiFruRecordNameLen 64

class cIpmiFruRecord
{
public:
  char           m_name[dIpmiFruRecordNameLen];
  cIpmiFruItem **m_array;
  int            m_num;

  cIpmiFruRecord( const char *name );
  ~cIpmiFruRecord();

  void Add( cIpmiFruItem *item );
  void Clear();

  cIpmiFruItem *Find( const char *name );

  void Log();
};


enum tFruAccessMode
{
  eFruAccessModeByte = 0,
  eFruAccessModeWord = 1
};


#define dMaxFruFetchBytes 20


class cIpmiFru
{
  cIpmiEntity   *m_entity;
  unsigned char  m_fru_device_id; // fru device id
  unsigned int   m_size;
  tFruAccessMode m_access;
  bool           m_fetched;

  int GetFruInventoryAreaInfo( unsigned int &size, tFruAccessMode &byte_access );
  int ReadFruData( unsigned short offset, unsigned int num, unsigned int &n, unsigned char *data );

  int CreateRecord( const char *name, tIpmiFruItemDesc *desc, 
                    unsigned char *data, unsigned int len );

  int CreateInventory( unsigned char *data );
  int CreateInternalUse( unsigned char *data, unsigned int len );
  int CreateChassis( unsigned char *data, unsigned int len );
  int CreateBoard( unsigned char *data, unsigned int len );
  int CreateProduct( unsigned char *data, unsigned int len );
  int CreateMultiRecord( unsigned char *data, unsigned int len );

  cIpmiFruRecord **m_array;
  int              m_num;

  void Add( cIpmiFruRecord *record );
  void Clear();

public:
  cIpmiFru( cIpmiEntity *ent, unsigned int fru_device_id );
  ~cIpmiFru();

  int Fetch();

  unsigned int FruId() { return m_fru_device_id; }

  cIpmiFruRecord *Find( const char *name );

  int NumRecords() { return m_num; }
  cIpmiFruRecord *GetRecord( int idx )
  {
    assert( idx >= 0 && idx < m_num );
    return m_array[idx]; 
  }

  // HPI record id to find the rdr
  SaHpiEntryIdT m_record_id;

  unsigned int  m_inventory_size;
};


class cIpmiDomain;
class cIpmiSdrs;
class cIpmiMc;

void IpmiFruHandleSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );


#endif
