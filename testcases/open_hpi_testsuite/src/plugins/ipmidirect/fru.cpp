/*
 * fru.cpp
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ipmi.h"


static SaHpiTextBufferT *
SetItem( cIpmiFruRecord *fr, unsigned char *&p, int &s, const char *name )
{
  SaHpiTextBufferT *t = (SaHpiTextBufferT *)p;
  memset( t->Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH );

  cIpmiFruItem *i = fr->Find( name );

  if ( i )
     {
       assert( i->m_type == eIpmiFruItemTypeTextBuffer );

       const cIpmiTextBuffer *b = &i->m_u.m_text_buffer;

       t->DataType = (SaHpiTextTypeT)b->m_type;
       t->Language = (SaHpiLanguageT)b->m_language;
       t->DataLength = b->m_len;
       memcpy( t->Data, b->m_data, b->m_len );
     }
  else
     {
       t->DataType = SAHPI_TL_TYPE_BINARY;
       t->Language = SAHPI_LANG_UNDEF;
       t->DataLength = 0;
     }

  s += sizeof( SaHpiTextBufferT );
  p += sizeof( SaHpiTextBufferT );

  return t;
}


static int
GeneralDataRecord( SaHpiInventGeneralDataT *r, cIpmiFruRecord *fr )
{
  int size = 0;
  unsigned char *p = (unsigned char *)r + sizeof( SaHpiInventGeneralDataT );

  // calculate number of custom fields
  unsigned int idx = 0;

  while( true )
     {
       char str[dIpmiFruItemNameLen];
       sprintf( str, "%s%d", dIpmiFruItemCustom, idx + 1 );

       if ( fr->Find( str ) == 0 )
            break;

       idx++;
      }

  size = idx * sizeof( SaHpiTextBufferT * );
  p += size;

  // mfg date
  r->MfgDateTime = SAHPI_TIME_UNSPECIFIED;

  cIpmiFruItem *i = fr->Find( dIpmiFruItemMfgDate );

  if ( i )
     {
       r->MfgDateTime = (SaHpiTimeT)i->m_u.m_int;
       r->MfgDateTime *= 1000000000;
     }

  r->Manufacturer   = SetItem( fr, p, size, dIpmiFruItemManufacturer );
  r->ProductName    = SetItem( fr, p, size, dIpmiFruItemProductName );
  r->ProductVersion = SetItem( fr, p, size, dIpmiFruItemProductVersion );
  r->ModelNumber    = SetItem( fr, p, size, "ModelNumer" ); // just to create an empty text buffer
  r->SerialNumber   = SetItem( fr, p, size, dIpmiFruItemSerialNumber );
  r->PartNumber	    = SetItem( fr, p, size, dIpmiFruItemPartNumber );
  r->FileId 	    = SetItem( fr, p, size, dIpmiFruItemFruFileId );
  r->AssetTag       = SetItem( fr, p, size,  dIpmiFruItemAssetTag);

  // custom fields
  idx = 0;

  while( true )
     {
       char str[dIpmiFruItemNameLen];
       sprintf( str, "%s%d", dIpmiFruItemCustom, idx + 1 );

       if ( !fr->Find( str ) )
            break;

       r->CustomField[idx] = SetItem( fr, p, size, str ); 
       idx++;
     }

  r->CustomField[idx] = 0;

  return size;
}


static int
InternalUseRecord( SaHpiInventInternalUseDataT *r, cIpmiFruRecord *fr )
{
  int s = 0;

  cIpmiFruItem *i = fr->Find( dIpmiFruItemData );

  if ( i )
     {
       memcpy( r->Data, i->m_u.m_data.m_data, i->m_u.m_data.m_size );
       s += i->m_u.m_data.m_size;
     }

  return s;
}


static int
ChassisInfoAreaRecord( SaHpiInventChassisDataT *r, cIpmiFruRecord *fr )
{
  int s = sizeof( SaHpiInventChassisDataT );

  r->Type = SAHPI_INVENT_CTYP_UNKNOWN;

  cIpmiFruItem *i = fr->Find( dIpmiFruItemChassisType );

  if ( i )
       r->Type = (SaHpiInventChassisTypeT)i->m_u.m_int;

  return s + GeneralDataRecord( &r->GeneralData, fr );
}


static int
BoradInfoAreaRecord( SaHpiInventGeneralDataT *r, cIpmiFruRecord *fr )
{
  int s = sizeof( SaHpiInventGeneralDataT );

  return s + GeneralDataRecord( r, fr );
}


static int
ProductInfoAreaRecord( SaHpiInventGeneralDataT *r, cIpmiFruRecord *fr )
{
  int s = sizeof( SaHpiInventGeneralDataT );

  return s + GeneralDataRecord( r, fr );
}


static unsigned int
GetInventoryInfo( cIpmiFru *fru, SaHpiInventoryDataT &data )
{
  int size = sizeof( SaHpiInventoryDataT )
    + fru->NumRecords() * sizeof( SaHpiInventDataRecordT * );

  data.Validity = SAHPI_INVENT_DATA_VALID;

  // position of the first record
  unsigned char *p = (unsigned char *)&data + size;

  int idx = 0;

  for( int i = fru->NumRecords() - 1; i >= 0; i-- )
     {
       cIpmiFruRecord *fr = fru->GetRecord( i );
       SaHpiInventDataRecordT *r = (SaHpiInventDataRecordT *)p;
       int s;

       if ( !strcmp( fr->m_name, dIpmiFruRecordInternalUseArea ) )
	  {
	    s = InternalUseRecord( &r->RecordData.InternalUse, fr );
	    r->RecordType = SAHPI_INVENT_RECTYPE_INTERNAL_USE;
	  }
       else if ( !strcmp( fr->m_name, dIpmiFruRecordChassisInfoArea ) )
	  {
	    s = ChassisInfoAreaRecord( &r->RecordData.ChassisInfo, fr );
	    r->RecordType = SAHPI_INVENT_RECTYPE_CHASSIS_INFO;
	  }
       else if ( !strcmp( fr->m_name, dIpmiFruRecordBoardInfoArea ) )
	  {
	    s = BoradInfoAreaRecord( &r->RecordData.BoardInfo, fr );
	    r->RecordType = SAHPI_INVENT_RECTYPE_BOARD_INFO;
	  }
       else if ( !strcmp( fr->m_name, dIpmiFruRecordProductInfoArea ) )
	  {
	    s = ProductInfoAreaRecord( &r->RecordData.ProductInfo, fr );
	    r->RecordType = SAHPI_INVENT_RECTYPE_PRODUCT_INFO;
	  }
       else if ( !strcmp( fr->m_name, dIpmiFruRecordMultiRecord ) )
	    continue;
       else
	  {
	    assert( 0 );
	    continue;
          }

       data.DataRecords[idx++] = (SaHpiInventDataRecordT *)p;
       r->DataLength = s;
       s += sizeof( SaHpiInventDataRecordT ) - sizeof( SaHpiInventDataUnionT );
       p += s;
       size += s;
     }

  // end mark
  data.DataRecords[fru->NumRecords()] = 0;

  return size;
}


static void
AddFruEventInventoryRec( cIpmiFru *fru,  SaHpiInventoryRecT &rec )
{
  rec.EirId = fru->FruId();
  rec.Oem = 0;
}


static void
AddFruEventRdr( cIpmiFru *fru, SaHpiRdrT   &rdr,
                   const SaHpiRptEntryT &resource )
{
  rdr.RecordId = 0;
  rdr.RdrType = SAHPI_INVENTORY_RDR;
  rdr.Entity = resource.ResourceEntity;

  AddFruEventInventoryRec( fru, rdr.RdrTypeUnion.InventoryRec );

  char	name[32];
  memset( name,'\0',32 );

  sprintf( name, "FRU%d", fru->FruId() );
  rdr.IdString.DataType = SAHPI_TL_TYPE_ASCII6;
  rdr.IdString.Language = SAHPI_LANG_ENGLISH;
  rdr.IdString.DataLength = 32;

  memcpy( rdr.IdString.Data,name, strlen( name ) + 1 );
}


static void
AddInventoryDataEvent( cIpmiEntity *ent, cIpmiFru *fru, const SaHpiRptEntryT &resource )
{
  struct oh_event *e;

  e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );   
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );

  e->type                   = oh_event::OH_ET_RDR;

  AddFruEventRdr( fru, e->u.rdr_event.rdr, resource );

  //  SaHpiResourceIdT rid = oh_uid_lookup( &e->u.rdr_event.rdr.Entity );
  oh_add_rdr( ent->Domain()->GetHandler()->rptcache,
              resource.ResourceId,
              &e->u.rdr_event.rdr, fru, 1);

  // assign the hpi record id to fru, so we can find
  // the rdr for a given fru.
  // the id comes from oh_add_rdr.
  fru->m_record_id = e->u.rdr_event.rdr.RecordId;

  ent->Domain()->AddHpiEvent( e );
}


void
cIpmi::IfFruAdd( cIpmiEntity *ent, cIpmiFru *fru )
{
  IpmiLog( "adding inventory data\n" );

  dbg( "adding inventory data %d.%d (%s): id %02x",
       ent->EntityId(), ent->EntityInstance(),
       ent->EntityIdString(), fru->FruId() );

  // calulate fru inventory size
  unsigned char *buffer = new unsigned char[1024*128];
  SaHpiInventoryDataT *d = (SaHpiInventoryDataT *)buffer;
  fru->m_inventory_size = GetInventoryInfo( fru, *d );
  delete [] buffer;

  // check for overflow
  {
    unsigned int n = 256;

    buffer = new unsigned char[fru->m_inventory_size+n];

    unsigned char *b = buffer + fru->m_inventory_size;
    unsigned int i;

    for( i = 0; i < n; i++ )
         *b++ = (unsigned char)i;

    d = (SaHpiInventoryDataT *)buffer;
    unsigned int s = GetInventoryInfo( fru, *d );

    assert( s == fru->m_inventory_size );

    b = buffer + fru->m_inventory_size;

    for( i = 0; i < n; i++, b++ )
       {
         //printf( "%d = 0x%02x\n", i, *b );
         assert( *b == i );
       }

    delete [] buffer;
  }

  // find resource
  SaHpiRptEntryT *resource = ent->Domain()->FindResource( ent->m_resource_id );

  if ( !resource )
     {
       assert( 0 );
       return;
     }

  if ( !(resource->ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA ) )
     {
       // update resource
       resource->ResourceCapabilities |= SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_INVENTORY_DATA;

       struct oh_event *e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

       if ( !e )
          {
            IpmiLog( "Out of space !\n" );
            return;
          }

       memset( e, 0, sizeof( struct oh_event ) );
       e->type               = oh_event::OH_ET_RESOURCE;
       e->u.res_event.entry = *resource;

       AddHpiEvent( e );
     }

  // create rdr
  AddInventoryDataEvent( ent, fru, *resource );
}


SaErrorT
cIpmi::IfGetInventorySize( cIpmiFru *fru, SaHpiUint32T &size )
{
  assert( fru->m_inventory_size );

  size = fru->m_inventory_size;

  return SA_OK;
}


SaErrorT
cIpmi::IfGetInventoryInfo( cIpmiFru *fru, SaHpiInventoryDataT &data )
{
  assert( fru->m_inventory_size );

  GetInventoryInfo( fru, data );

  return SA_OK;
}
