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


#include <string.h>
#include <assert.h>
#include <errno.h>

#include "ipmi_domain.h"
#include "ipmi_utils.h"
#include "ipmi_fru.h"


static tIpmiFruItemDesc chassis_desc[] =
{
  { dIpmiFruItemVersion     , eIpmiFruItemTypeInt },
  { ""                      , eIpmiFruItemTypePad }, // ignore length
  { dIpmiFruItemChassisType , eIpmiFruItemTypeInt },
  { dIpmiFruItemPartNumber  , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemSerialNumber, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemCustom      , eIpmiFruItemTypeCustom },
  { 0, eIpmiFruItemTypeUnknown }
};


static tIpmiFruItemDesc board_desc[] =
{
  { dIpmiFruItemVersion     , eIpmiFruItemTypeInt },
  { ""                      , eIpmiFruItemTypePad }, // ignore length
  { dIpmiFruItemLanguage    , eIpmiFruItemTypeInt },
  { dIpmiFruItemMfgDate     , eIpmiFruItemTypeDate3 },
  { dIpmiFruItemManufacturer, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemProductName , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemSerialNumber, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemPartNumber  , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemFruFileId   , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemCustom      , eIpmiFruItemTypeCustom },
  { 0, eIpmiFruItemTypeUnknown }
};

static tIpmiFruItemDesc product_desc[] =
{
  { dIpmiFruItemVersion     , eIpmiFruItemTypeInt },
  { ""                      , eIpmiFruItemTypePad }, // ignore length
  { dIpmiFruItemLanguage    , eIpmiFruItemTypeInt },
  { dIpmiFruItemManufacturer, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemProductName , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemPartNumber  , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemProductVersion, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemSerialNumber, eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemAssetTag    , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemFruFileId   , eIpmiFruItemTypeTextBuffer },
  { dIpmiFruItemCustom      , eIpmiFruItemTypeCustom },
  { 0, eIpmiFruItemTypeUnknown }
};


//////////////////////////////////////////////////
//                  cIpmiFruItem
//////////////////////////////////////////////////

cIpmiFruItem::cIpmiFruItem( const char *name, tIpmiFruItemType type )
  : m_type( type )
{
  assert( name );
  strcpy( m_name, name );

  m_u.m_data.m_data = 0;
}


cIpmiFruItem::~cIpmiFruItem()
{
  if ( m_type == eIpmiFruItemTypeData && m_u.m_data.m_data )
       delete [] m_u.m_data.m_data;
  else if ( m_type == eIpmiFruItemTypeRecord && m_u.m_record )
       delete m_u.m_record;
}


void
cIpmiFruItem::Log()
{
  switch( m_type )
     {
       case eIpmiFruItemTypeTextBuffer:
            {
              char str[256] = "";
              m_u.m_text_buffer.GetAscii( str, 256 );
              str[255] = 0;

              IpmiLog( "\t%s\t\t%s\n", m_name, str );
            }
            break;

       case eIpmiFruItemTypeInt:
            IpmiLog( "\t%s\t\t%d 0x%x\n", m_name, m_u.m_int, m_u.m_int );
            break;

       default:
            IpmiLog( "\t%s\t\tunknown\n", m_name );
            break;
     }
}


//////////////////////////////////////////////////
//                  cIpmiFruRecord
//////////////////////////////////////////////////

cIpmiFruRecord::cIpmiFruRecord( const char *name )
  : m_array( 0 ), m_num( 0 )
{
  strcpy( m_name, name );
}


cIpmiFruRecord::~cIpmiFruRecord()
{
  Clear();
}


void
cIpmiFruRecord::Add( cIpmiFruItem *item )
{
  cIpmiFruItem **array = new cIpmiFruItem *[m_num+1];

  if ( m_array )
     {
       memcpy( array, m_array, sizeof( cIpmiFruItem * ) * m_num );
       delete [] m_array;
     }

  m_array = array;
  m_array[m_num++] = item;
}


void
cIpmiFruRecord::Clear()
{
  if ( m_array )
     {
       for( int i = 0; i < m_num; i++ )
            delete m_array[i];

       delete [] m_array;
       m_array = 0;
       m_num = 0;
     }
}


cIpmiFruItem *
cIpmiFruRecord::Find( const char *name )
{
  for( int i = 0; i < m_num; i++ )
       if ( !strcmp( m_array[i]->m_name, name ) )
            return m_array[i];

  return 0;
}


void
cIpmiFruRecord::Log()
{
  for( int i = 0; i < m_num; i++ )
       m_array[i]->Log();
}


//////////////////////////////////////////////////
//                  cIpmiFru
//////////////////////////////////////////////////

cIpmiFru::cIpmiFru( cIpmiEntity *ent, unsigned int fru_device_id )
  : m_entity( ent ), m_fru_device_id( fru_device_id ),
    m_size( 0 ), m_access( eFruAccessModeByte ),
    m_array( 0 ), m_num( 0 ),
    m_inventory_size( 0 )
{
}


cIpmiFru::~cIpmiFru()
{
  Clear();
}


void
cIpmiFru::Add( cIpmiFruRecord *record )
{
  cIpmiFruRecord **array = new cIpmiFruRecord *[m_num+1];

  if ( m_array )
     {
       memcpy( array, m_array, sizeof( cIpmiFruRecord * ) * m_num );
       delete [] m_array;
     }

  m_array = array;
  m_array[m_num++] = record;
}


void
cIpmiFru::Clear()
{
  if ( m_array )
     {
       for( int i = 0; i < m_num; i++ )
            delete m_array[i];

       delete [] m_array;
       m_array = 0;
       m_num = 0;
     }

  m_inventory_size = 0;
}


cIpmiFruRecord *
cIpmiFru::Find( const char *name )
{
  for( int i = 0; i < m_num; i++ )
       if ( !strcmp( m_array[i]->m_name, name ) )
            return m_array[i];

  return 0;
}


int
cIpmiFru::GetFruInventoryAreaInfo( unsigned int &size,
                                   tFruAccessMode &byte_access )
{
  cIpmiAddr addr( eIpmiAddrTypeIpmb, m_entity->Channel(),
                  m_entity->Lun(), m_entity->AccessAddress() );
  cIpmiMsg  msg( eIpmiNetfnStorage, eIpmiCmdGetFruInventoryAreaInfo );
  cIpmiMsg  rsp;

  msg.m_data[0] = m_fru_device_id;
  msg.m_data_len = 1;

  int rv = m_entity->Domain()->SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot GetFruInventoryAreaInfo: %d !\n", rv );
       return rv;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       IpmiLog( "cannot GetFruInventoryAreaInfo: %s !\n",
                IpmiCompletionCodeToString( (tIpmiCompletionCode)rsp.m_data[0] ) );

       return EINVAL;
     }

  byte_access = (rsp.m_data[3] & 1) ? eFruAccessModeWord : eFruAccessModeByte;
  size = IpmiGetUint16( rsp.m_data + 1 ) >> byte_access;

  return 0;
}


int
cIpmiFru::ReadFruData( unsigned short offset, unsigned int num, unsigned int &n, unsigned char *data )
{
  cIpmiAddr addr( eIpmiAddrTypeIpmb, m_entity->Channel(),
                  m_entity->Lun(), m_entity->AccessAddress() );
  cIpmiMsg  msg( eIpmiNetfnStorage, eIpmiCmdReadFruData );
  cIpmiMsg  rsp;

  msg.m_data[0] = m_fru_device_id;
  IpmiSetUint16( msg.m_data + 1, offset >> m_access );
  msg.m_data[3] = num >> m_access;
  msg.m_data_len = 4;

  int rv = m_entity->Domain()->SendCommand( addr, msg, rsp );

  if ( rv )
     {
       IpmiLog( "cannot ReadFruData: %d !\n", rv );
       return rv;
     }

  if ( rsp.m_data[0] != eIpmiCcOk )
     {
       IpmiLog( "cannot ReadFruData: %s !\n",
                IpmiCompletionCodeToString( (tIpmiCompletionCode)rsp.m_data[0] ) );

       return EINVAL;
     }

  n = rsp.m_data[1] << m_access;

  if ( n < 1 )
     {
       IpmiLog( "ReadFruData: read 0 bytes !\n" );

       return EINVAL;
     }

  memcpy( data, rsp.m_data + 2, n );

  return 0;
}


int
cIpmiFru::Fetch()
{
  m_fetched = false;
  Clear();

  int rv = GetFruInventoryAreaInfo( m_size,
                                    m_access );

  if ( rv || m_size == 0 )
       return rv;

  unsigned short offset = 0;
  unsigned char *data = new unsigned char[m_size];

  while( offset < m_size )
     {
       unsigned int num = m_size - offset;

       if ( num > dMaxFruFetchBytes )
            num = dMaxFruFetchBytes;

       unsigned int n;

       rv = ReadFruData( offset, num, n, data + offset );

       if ( rv )
          {
            delete [] data;
            return rv;
          }

       offset += n;
     }

  rv = CreateInventory( data );

  delete [] data;

  m_fetched = rv ? false : true;

  return rv;
}


static unsigned char
checksum( unsigned char *data, int size )
{
  unsigned char c = 0;

  while( size-- )
      c += *data++;

  return c;
}


int
cIpmiFru::CreateInventory( unsigned char *data )
{
  IpmiLog( "FRU Inventory:\n" );

  if ( m_size < 8 )
     {
       IpmiLog( "FRU data too short (%d < 8) !\n", m_size );
       return EINVAL;
     }

  if ( checksum( data, 8 ) )
     {
       IpmiLog( "wrong FRU header checksum !\n" );
       return EINVAL;
     }

  unsigned int pos = m_size;
  unsigned int offset;
  unsigned int len;

  if ( data[5] )
     {
       offset = data[5] * 8;
       len = pos - offset;

       IpmiLog( "MultiRecord: offset %d, len %d\n", offset, len );

       CreateMultiRecord( data + offset, len );
       pos -= len;
     }

  if ( data[4] )
     {
       offset = data[4] * 8;
       len = pos - offset;

       IpmiLog( "Product: offset %d, len %d\n", offset, len );

       CreateProduct( data + offset, len );

       pos -= len;
     }

  if ( data[3] )
     {
       offset = data[3] * 8;
       len = pos - offset;

       IpmiLog( "Board: offset %d, len %d\n", offset, len );

       CreateBoard( data + offset, len );
       pos -= len;
     }

  if ( data[2] )
     {
       offset = data[2] * 8;
       len = pos - offset;

       IpmiLog( "Chassis: offset %d, len %d\n", offset, len );

       CreateChassis( data + offset, len );
       pos -= len;
     }

  if ( data[1] )
     {
       offset = data[1] * 8;
       len = pos - offset;

       IpmiLog( "Internal: offset %d, len %d\n", offset, len );

       CreateInternalUse( data + offset, len );
       pos -= len;
     }

  return 0;
}


int
cIpmiFru::CreateRecord( const char *name, tIpmiFruItemDesc *desc, 
                        unsigned char *data, unsigned int /*len*/ )
{
  cIpmiFruRecord *r = new cIpmiFruRecord( name );
  cIpmiFruItem *item;
  int i;

  while( desc->m_name )
     {
       item = 0;

       switch( desc->m_type )
          {
            case eIpmiFruItemTypeTextBuffer:
                 item = new cIpmiFruItem( desc->m_name, eIpmiFruItemTypeTextBuffer );
                 data = item->m_u.m_text_buffer.Set( data );
                 assert( data );

                 break;

            case eIpmiFruItemTypeInt:
                 item = new cIpmiFruItem( desc->m_name, eIpmiFruItemTypeInt );
                 item->m_u.m_int = *data++;

                 break;
  
            case eIpmiFruItemTypeDate3:
                 {
                   item = new cIpmiFruItem( desc->m_name, eIpmiFruItemTypeInt );

                   time_t t;
                   struct tm tm;

                   // minutes since 1996.01.01 00:00:00
                   t = *data++ << 16;
                   t += *data++ << 8;
                   t += *data++;
                   t *= 60;

                   // create date offset
                   tm.tm_sec  = 0;
                   tm.tm_min  = 0;
                   tm.tm_hour = 0;
                   tm.tm_mday = 1;
                   tm.tm_mon  = 0;
                   tm.tm_year = 96;
                   tm.tm_isdst = 0;

                   item->m_u.m_int = t;
                   item->m_u.m_int += mktime( &tm );
                 }

                 break;

            case eIpmiFruItemTypeCustom:
                 for( i = 0; i < 20; i++ )
                    {
                      char str[dIpmiFruItemNameLen];
                      sprintf( str, "%s%d", desc->m_name, i + 1 );

                      item = new cIpmiFruItem( str, eIpmiFruItemTypeTextBuffer );
                      data = item->m_u.m_text_buffer.Set( data );

                      if ( data )
                           r->Add( item );
                      else
                         {
                           delete item;

                           Add( r );
                           r->Log();

                           return 0;
                         }
                    }

                 // something goes wrong
                 assert( 0 );
                 return EINVAL;

            case eIpmiFruItemTypePad:
                 data++;
                 break;

            default:
                 assert( 0 );
                 break;
          }

       if ( item )
            r->Add( item );

       desc++;
     }

  Add( r );
  r->Log();

  return 0;
}


int
cIpmiFru::CreateInternalUse( unsigned char *data, unsigned int len )
{
  cIpmiFruRecord *r = new cIpmiFruRecord( dIpmiFruRecordInternalUseArea );
  
  // create version item
  cIpmiFruItem *i = new cIpmiFruItem( dIpmiFruItemVersion, eIpmiFruItemTypeInt );
  i->m_u.m_int = *data++;
  len--;

  r->Add( i );

  // create data item
  unsigned char *d = new unsigned char[len];
  memcpy( d, data, len );

  i = new cIpmiFruItem( dIpmiFruItemData, eIpmiFruItemTypeData );
  i->m_u.m_data.m_data = d;
  i->m_u.m_data.m_size = len;

  Add( r );

  r->Log();

  return 0;
}


int
cIpmiFru::CreateChassis( unsigned char *data, unsigned int len )
{
  return CreateRecord( dIpmiFruRecordChassisInfoArea, 
                       chassis_desc, data, len );
}


int
cIpmiFru::CreateBoard( unsigned char *data, unsigned int len )
{
  return CreateRecord( dIpmiFruRecordBoardInfoArea,
                       board_desc, data, len );
}


int
cIpmiFru::CreateProduct( unsigned char *data, unsigned int len )
{
  return CreateRecord( dIpmiFruRecordProductInfoArea, 
                       product_desc, data, len );
}


int
cIpmiFru::CreateMultiRecord( unsigned char * /*data*/, unsigned int /*len*/ )
{
  return 0;
}


void
IpmiFruHandleSdr( cIpmiDomain *domain, cIpmiMc * /*mc*/, cIpmiSdrs *sdrs )
{
  for( unsigned int i = 0; i < sdrs->NumSdrs(); i++ )
     {
       cIpmiSdr *sdr = sdrs->Sdr( i );

       tIpmiEntityId id;
       unsigned int instance;
       unsigned int fru_id;
       unsigned int addr;
       unsigned int channel;
       unsigned int lun;

       if ( sdr->m_type == eSdrTypeMcDeviceLocatorRecord )
          {
            // fru inventory device ?
            if ( (sdr->m_data[8] & 8) == 0 )
                 continue;

            id       = (tIpmiEntityId)sdr->m_data[12];
            instance = sdr->m_data[13];
            fru_id   = 0;
            addr     = sdr->m_data[5];
            channel  = sdr->m_data[6] & 0xf;
            lun      = 0;
          }
       else if ( sdr->m_type == eSdrTypeFruDeviceLocatorRecord )
          {
            id       = (tIpmiEntityId)sdr->m_data[12];
            instance = sdr->m_data[13];
            fru_id   = sdr->m_data[6];
            addr     = sdr->m_data[5];
            channel  = (sdr->m_data[8] >> 4) & 0xf;
            lun      = (sdr->m_data[7] >> 3) & 3;
          }
       else
            continue;

       // create mc
       cIpmiMc *m = domain->FindOrCreateMcBySlaveAddr( addr );
       cIpmiEntity *ent = domain->Entities().Add( m, lun, id, instance, "" );
       assert( ent );

       cIpmiFru *fru = ent->FindFru( fru_id );

       if ( fru == 0 )
          {
            fru = new cIpmiFru( ent, fru_id );
            ent->AddFru( fru );
          }

       // read fru
       if ( !fru->Fetch() )
            domain->IfFruAdd( ent, fru );
     }
}
