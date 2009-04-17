/*
 * ipmi_inventory_parser.cpp
 *
 * Copyright (c) 2004 by FORCE Computers
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

#include "ipmi_inventory_parser.h"
#include "ipmi_utils.h"
#include <string.h>
#include <oh_utils.h>

unsigned char
IpmiChecksum( const unsigned char *data, int size )
{
  unsigned char c = 0;

  while( size-- )
      c += *data++;

  return c;
}

unsigned char
IpmiChecksumMulti( const unsigned char *data, int size, unsigned char csum )
{
  unsigned char c = 0;

  while( size-- )
      c += *data++;

  c += csum;

  return c;
}

static const char *inventory_record_type_map[] =
{
  "Internal",
  "Chassis",
  "Board",
  "Product",
  "MultiRecord"
};


const char *
IpmiInventoryRecordTypeToString( tIpmiInventoryRecordType type )
{
  if ( type >= eIpmiInventoryRecordTypeLast )
       return "Invalid";

  return inventory_record_type_map[type];
}


////////////////////////////////////////////////////////////
//                  cIpmiInventoryField
////////////////////////////////////////////////////////////

cIpmiInventoryField::cIpmiInventoryField(SaHpiEntryIdT area_id,
                                         SaHpiEntryIdT field_id,
                                         SaHpiIdrFieldTypeT field_type)
{
    m_idr_field.AreaId   = area_id;
    m_idr_field.FieldId  = field_id;
    m_idr_field.Type     = field_type;
    m_idr_field.ReadOnly = SAHPI_TRUE;
}


cIpmiInventoryField::~cIpmiInventoryField()
{
}

SaErrorT
cIpmiInventoryField::ReadTextBuffer( const unsigned char *&data, unsigned int &size )
{
  if ( size < 1 )
       return SA_ERR_HPI_INVALID_DATA;

  const unsigned char *d = m_ipmi_text_buffer.SetIpmi( data, true );

  if ( d == 0 )
       return SA_ERR_HPI_INVALID_DATA;

  m_idr_field.Field = m_ipmi_text_buffer;

  size -= d - data;
  data = d;

  return SA_OK;
}

void
cIpmiInventoryField::SetAscii( char *str, int size )
{
  m_idr_field.Field.DataType = SAHPI_TL_TYPE_TEXT;
  m_idr_field.Field.Language = SAHPI_LANG_ENGLISH;
  m_idr_field.Field.DataLength = size;

  memcpy( m_idr_field.Field.Data, str, size );
}

void
cIpmiInventoryField::SetBinary( const unsigned char *data, unsigned int size )
{
  m_idr_field.Field.DataType = SAHPI_TL_TYPE_BINARY;
  m_idr_field.Field.Language = SAHPI_LANG_UNDEF;
  m_idr_field.Field.DataLength = size;

  memcpy( m_idr_field.Field.Data, data, size );
}

////////////////////////////////////////////////////////////
//                  cIpmiInventoryAreaInternal
////////////////////////////////////////////////////////////

cIpmiInventoryAreaInternal::cIpmiInventoryAreaInternal(SaHpiEntryIdT area_id)
  : cIpmiInventoryArea(area_id)
{
    m_area_header.Type = SAHPI_IDR_AREATYPE_INTERNAL_USE;
}


cIpmiInventoryAreaInternal::~cIpmiInventoryAreaInternal()
{
}


SaErrorT
cIpmiInventoryAreaInternal::ParseFruArea( const unsigned char *data, unsigned int size )
{
  return SA_ERR_HPI_INVALID_DATA;
}

////////////////////////////////////////////////////////////
//                  cIpmiInventoryAreaMultiRecord
////////////////////////////////////////////////////////////

cIpmiInventoryAreaMultiRecord::cIpmiInventoryAreaMultiRecord(SaHpiEntryIdT area_id)
  : cIpmiInventoryArea(area_id)
{
    m_area_header.Type = SAHPI_IDR_AREATYPE_OEM;
}


cIpmiInventoryAreaMultiRecord::~cIpmiInventoryAreaMultiRecord()
{
}


SaErrorT
cIpmiInventoryAreaMultiRecord::ParseFruArea( const unsigned char *data, unsigned int size )
{
  cIpmiInventoryField *iif;
  unsigned int record_size;
  bool end_of_list = false;
  unsigned char record_type;
  unsigned char record_csum;

  do {
       if ( size < 5 )
	     return SA_ERR_HPI_INVALID_DATA;

       if ( IpmiChecksum(data, 5 ))
       {
            stdlog << "wrong Multirecord header area checksum !\n";
            return SA_ERR_HPI_INVALID_DATA;
       }

       record_size = *(data+2);
       end_of_list = *(data+1) & 0x80;
       record_type = *data;
       record_csum = *(data+3);

       stdlog << "Multirecord type " << record_type << " size " << record_size << " EOL " << end_of_list << "\n";

       data += 5;
       size -= 5;

       if ( record_size > size )
       {
            stdlog << "wrong Multirecord area checksum !\n";
            return SA_ERR_HPI_INVALID_DATA;
       }

       if ( IpmiChecksumMulti( data, record_size, record_csum ) )
       {
            stdlog << "wrong Multirecord area checksum !\n";
            return SA_ERR_HPI_INVALID_DATA;
       }

       if ( record_type >= eIpmiInventoryMultiRecordTypeOemFirst )
       {
            iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, SAHPI_IDR_FIELDTYPE_CUSTOM);

            m_field_array.Add( iif );

            iif->SetBinary( data, record_size );
       }

       data += record_size;
       size -= record_size;

     } while (end_of_list == false);

  m_area_header.NumFields = m_field_array.Num();

  return SA_OK;
}

////////////////////////////////////////////////////////////
//                  cIpmiInventoryAreaChassis
////////////////////////////////////////////////////////////

cIpmiInventoryAreaChassis::cIpmiInventoryAreaChassis(SaHpiEntryIdT area_id)
  : cIpmiInventoryArea(area_id)
{
    m_area_header.Type = SAHPI_IDR_AREATYPE_CHASSIS_INFO;
}


cIpmiInventoryAreaChassis::~cIpmiInventoryAreaChassis()
{
}


static SaHpiIdrFieldTypeT ChassisInfoAreaFields[] =
{
    SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
};

SaErrorT
cIpmiInventoryAreaChassis::ParseFruArea( const unsigned char *data, unsigned int size )
{
  cIpmiInventoryField *iif;
  SaErrorT rv;
  unsigned int area_size = *(data+1)*8;

  if ( area_size > size )
  {
       stdlog << "wrong chassis area length !\n";
       return SA_ERR_HPI_INVALID_DATA;
  }

  if ( IpmiChecksum( data, *(data+1)*8 ) )
     {
       stdlog << "wrong chassis area checksum !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  data += 2;
  size -= 2;

  if ( size < 1 )
       return SA_ERR_HPI_INVALID_DATA;

  // Skip Chassis type
  data++;
  size--;

  for ( unsigned int i = 0; i < sizeof(ChassisInfoAreaFields)/sizeof(ChassisInfoAreaFields[0]); i++ )
  {
      iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, ChassisInfoAreaFields[i]);

      m_field_array.Add( iif );

      rv = iif->ReadTextBuffer( data, size );

      if ( rv != SA_OK )
          return rv;
  }

  while( true )
     {
       if ( size < 1 )
	    return SA_ERR_HPI_INVALID_DATA;

       if ( *data == 0xc1 )
	    break;

       iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, SAHPI_IDR_FIELDTYPE_CUSTOM);

       m_field_array.Add( iif );

       rv = iif->ReadTextBuffer( data, size );

       if ( rv != SA_OK )
        return rv;
     }

  m_area_header.NumFields = m_field_array.Num();

  return SA_OK;
}

////////////////////////////////////////////////////////////
//                  cIpmiInventoryAreaBoard
////////////////////////////////////////////////////////////

cIpmiInventoryAreaBoard::cIpmiInventoryAreaBoard(SaHpiEntryIdT area_id)
  : cIpmiInventoryArea(area_id)
{
    m_area_header.Type = SAHPI_IDR_AREATYPE_BOARD_INFO;
}


cIpmiInventoryAreaBoard::~cIpmiInventoryAreaBoard()
{
}

static SaHpiIdrFieldTypeT BoardInfoAreaFields[] =
{
    SAHPI_IDR_FIELDTYPE_MANUFACTURER,
    SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
    SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
    SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    SAHPI_IDR_FIELDTYPE_FILE_ID,
};

SaErrorT
cIpmiInventoryAreaBoard::ParseFruArea( const unsigned char *data, unsigned int size )
{
  cIpmiInventoryField *iif;
  SaErrorT rv;
  unsigned int area_size = *(data+1)*8;

  if ( area_size > size )
  {
       stdlog << "wrong board area length !\n";
       return SA_ERR_HPI_INVALID_DATA;
  }

  if ( IpmiChecksum( data, *(data+1)*8 ) )
     {
       stdlog << "wrong board area checksum !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  data += 2;
  size -= 2;

  if ( size < 4 )
       return SA_ERR_HPI_INVALID_DATA;

  // Skip Language
  data++;
  size--;

  time_t mfg_time = (SaHpiTimeT)data[0] + (SaHpiTimeT)data[1] * 256 + (SaHpiTimeT)data[2] * 65536;

  size -= 3;
  data += 3;

  mfg_time *= 60;

  // create date offset
  struct tm tmt;

  tmt.tm_sec  = 0;
  tmt.tm_min  = 0;
  tmt.tm_hour = 0;
  tmt.tm_mday = 1;
  tmt.tm_mon  = 0;
  tmt.tm_year = 96;
  tmt.tm_isdst = 0;

  mfg_time += mktime( &tmt );

  char str[80];
  IpmiDateTimeToString( mfg_time, str );

  iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, SAHPI_IDR_FIELDTYPE_MFG_DATETIME);

  m_field_array.Add( iif );

  iif->SetAscii ( str, strlen( str ) + 1 );

  for ( unsigned int i = 0; i < sizeof(BoardInfoAreaFields)/sizeof(BoardInfoAreaFields[0]); i++ )
  {
      iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, BoardInfoAreaFields[i]);

      m_field_array.Add( iif );

      rv = iif->ReadTextBuffer( data, size );

      if ( rv != SA_OK )
          return rv;
  }

  while( true )
     {
       if ( size < 1 )
	    return SA_ERR_HPI_INVALID_DATA;

       if ( *data == 0xc1 )
	    break;

       iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, SAHPI_IDR_FIELDTYPE_CUSTOM);

       m_field_array.Add( iif );

       rv = iif->ReadTextBuffer( data, size );

       if ( rv != SA_OK )
        return rv;
     }

  m_area_header.NumFields = m_field_array.Num();

  return SA_OK;
}


////////////////////////////////////////////////////////////
//                  cIpmiInventoryAreaProduct
////////////////////////////////////////////////////////////

cIpmiInventoryAreaProduct::cIpmiInventoryAreaProduct(SaHpiEntryIdT area_id)
  : cIpmiInventoryArea(area_id)
{
    m_area_header.Type = SAHPI_IDR_AREATYPE_PRODUCT_INFO;
}


cIpmiInventoryAreaProduct::~cIpmiInventoryAreaProduct()
{
}


static SaHpiIdrFieldTypeT ProductInfoAreaFields[] =
{
    SAHPI_IDR_FIELDTYPE_MANUFACTURER,
    SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
    SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION,
    SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
    SAHPI_IDR_FIELDTYPE_ASSET_TAG,
    SAHPI_IDR_FIELDTYPE_FILE_ID,
};

SaErrorT
cIpmiInventoryAreaProduct::ParseFruArea( const unsigned char *data, unsigned int size )
{
  cIpmiInventoryField *iif;
  SaErrorT rv;
  unsigned int area_size = *(data+1)*8;

  if ( area_size > size )
  {
       stdlog << "wrong product area length !\n";
       return SA_ERR_HPI_INVALID_DATA;
  }

  if ( IpmiChecksum( data, *(data+1)*8 ) )
     {
       stdlog << "wrong product area checksum !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  data += 2;
  size -= 2;

  if ( size < 1 )
       return SA_ERR_HPI_INVALID_DATA;

  // Skip Language
  data++;
  size--;

  for ( unsigned int i = 0; i < sizeof(ProductInfoAreaFields)/sizeof(ProductInfoAreaFields[0]); i++ )
  {
      iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, ProductInfoAreaFields[i]);

      m_field_array.Add( iif );

      rv = iif->ReadTextBuffer( data, size );

      if ( rv != SA_OK )
          return rv;
  }

  while( true )
     {
       if ( size < 1 )
	    return SA_ERR_HPI_INVALID_DATA;

       if ( *data == 0xc1 )
	    break;

       iif = new cIpmiInventoryField( m_area_header.AreaId, m_field_id++, SAHPI_IDR_FIELDTYPE_CUSTOM);

       m_field_array.Add( iif );

       rv = iif->ReadTextBuffer( data, size );

       if ( rv != SA_OK )
        return rv;
     }

  m_area_header.NumFields = m_field_array.Num();

  return SA_OK;
}

////////////////////////////////////////////////////////////
//                  cIpmiInventoryArea
////////////////////////////////////////////////////////////

cIpmiInventoryArea::cIpmiInventoryArea(SaHpiEntryIdT area_id)
{
    m_area_header.AreaId    = area_id;
    m_area_header.Type      = SAHPI_IDR_AREATYPE_UNSPECIFIED;
    m_area_header.ReadOnly  = SAHPI_TRUE;
    m_area_header.NumFields = 0;
    m_field_id              = 1;
}

cIpmiInventoryArea::~cIpmiInventoryArea()
{
}


cIpmiInventoryField *
cIpmiInventoryArea::FindIdrField( SaHpiIdrFieldTypeT fieldtype,
                                  SaHpiEntryIdT fieldid
                                )
{
    cIpmiInventoryField *iif;

    if ( fieldid == SAHPI_FIRST_ENTRY )
    {
        for ( int i = 0; i < m_field_array.Num(); i++ )
        {
            iif = m_field_array[i];

            if (( fieldtype == SAHPI_IDR_FIELDTYPE_UNSPECIFIED ) ||
                ( fieldtype == iif->FieldType() ))
                return iif;
        }
    }
    else
    {
        for( int i = 0; i < m_field_array.Num(); i++ )
        {
            iif = m_field_array[i];

            if ( iif->FieldId() == fieldid )
            {
                if (( fieldtype == SAHPI_IDR_FIELDTYPE_UNSPECIFIED ) ||
                    ( fieldtype == iif->FieldType() ))
                    return iif;
                break;
            }
        }
    }

    return NULL;
}

SaErrorT
cIpmiInventoryArea::GetIdrField( SaHpiIdrFieldTypeT &fieldtype,
                                 SaHpiEntryIdT &fieldid,
			                     SaHpiEntryIdT &nextfieldid,
                                 SaHpiIdrFieldT &field )
{
    cIpmiInventoryField *iif = FindIdrField( fieldtype, fieldid );

    if ( iif == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    field = iif->Field();

    int idx = m_field_array.Find( iif );

    idx++;
    nextfieldid = SAHPI_LAST_ENTRY;

    for ( ; idx < m_field_array.Num(); idx++ )
    {
        iif = m_field_array[idx];
        if (( fieldtype == SAHPI_IDR_FIELDTYPE_UNSPECIFIED ) ||
            ( fieldtype == iif->FieldType() ))
        {
            nextfieldid = iif->FieldId();
            break;
        }
    }

    return SA_OK;
}


////////////////////////////////////////////////////////////
//                  cIpmiInventoryParser
////////////////////////////////////////////////////////////

cIpmiInventoryParser::cIpmiInventoryParser()
{
  m_idr_info.IdrId = 0;
  m_idr_info.UpdateCount = 0;
  m_idr_info.ReadOnly = SAHPI_TRUE;
  m_idr_info.NumAreas = 0;
  m_area_id = 1;
}


cIpmiInventoryParser::~cIpmiInventoryParser()
{
}

cIpmiInventoryArea *
cIpmiInventoryParser::AllocArea( SaHpiEntryIdT area_id, tIpmiInventoryRecordType type )
{
  switch( type )
     {
       case eIpmiInventoryRecordTypeInternal:
	    return new cIpmiInventoryAreaInternal( area_id );

       case eIpmiInventoryRecordTypeChassis:
	    return new cIpmiInventoryAreaChassis( area_id );

       case eIpmiInventoryRecordTypeBoard:
	    return new cIpmiInventoryAreaBoard( area_id );

       case eIpmiInventoryRecordTypeProduct:
	    return new cIpmiInventoryAreaProduct( area_id );

       case eIpmiInventoryRecordTypeMultiRecord:
	    return new cIpmiInventoryAreaMultiRecord( area_id );

       default:
        break;
     }

  return NULL;
}


SaErrorT
cIpmiInventoryParser::ParseFruInfo( const unsigned char *data, unsigned int size, unsigned int idr_id )
{
  cIpmiInventoryArea *ia;
  
  if ( size < 8 )
     {
       stdlog << "Inventory data too short (" << size << " < 8) !\n";
       return SA_ERR_HPI_INVALID_DATA;
     }

  // checksum of common header
  if ( IpmiChecksum( data, 8 ) )
     {
       stdlog << "wrong common header checksum !\n";
       stdlog.Hex( data, 8 );
       stdlog << "\n";

       return SA_ERR_HPI_INVALID_DATA;
     }

  // clear old
  m_area_array.Clear();

  unsigned int pos = size;

  // No Internal support for now
  for( int i = 5; i >= 2; i-- )
     {
       if ( data[i] == 0 )
	    continue;

       tIpmiInventoryRecordType type = (tIpmiInventoryRecordType)(i - 1);
       unsigned int offset = data[i] * 8;
       unsigned int len = pos - offset;
       stdlog << IpmiInventoryRecordTypeToString( type ) << ": offset " << offset << ", len " << len << "\n";
       
       ia = AllocArea( m_area_id, type );

       if ( ia )
       {
           SaErrorT rv = ia->ParseFruArea( data + offset, len );

           // If a FRU area is wrong just ignore it
           if ( rv )
               delete ia;
           else
           {
               m_area_id++;
               m_area_array.Add( ia );
           }
       }

       pos -= len;
     }

  m_idr_info.IdrId = idr_id;
  m_idr_info.UpdateCount++;
  m_idr_info.ReadOnly = SAHPI_TRUE;
  m_idr_info.NumAreas = m_area_array.Num();

  return SA_OK;
}

cIpmiInventoryArea *
cIpmiInventoryParser::FindIdrArea( SaHpiIdrAreaTypeT areatype,
			                       SaHpiEntryIdT areaid
                                 )
{
    cIpmiInventoryArea *ia;

    if ( areaid == SAHPI_FIRST_ENTRY )
    {
        for ( int i = 0; i < m_area_array.Num(); i++ )
        {
            ia = m_area_array[i];

            if (( areatype == SAHPI_IDR_AREATYPE_UNSPECIFIED ) ||
                ( areatype == ia->AreaType() ))
                return ia;
        }
    }
    else
    {
        for( int i = 0; i < m_area_array.Num(); i++ )
        {
            ia = m_area_array[i];

            if ( ia->AreaId() == areaid )
            {
                if (( areatype == SAHPI_IDR_AREATYPE_UNSPECIFIED ) ||
                    ( areatype == ia->AreaType() ))
                    return ia;
                break;
            }
        }
    }

    return NULL;
}


SaErrorT
cIpmiInventoryParser::GetIdrInfo( SaHpiIdrIdT &idrid, SaHpiIdrInfoT &idrinfo )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    idrinfo = m_idr_info;

    return SA_OK;
}

SaErrorT
cIpmiInventoryParser::GetIdrAreaHeader( SaHpiIdrIdT &idrid,
                                        SaHpiIdrAreaTypeT &areatype,
			                            SaHpiEntryIdT &areaid,
                                        SaHpiEntryIdT &nextareaid,
                                        SaHpiIdrAreaHeaderT &header )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;


    cIpmiInventoryArea *ia = FindIdrArea( areatype, areaid );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    header = ia->AreaHeader();

    int idx = m_area_array.Find( ia );

    idx++;
    nextareaid = SAHPI_LAST_ENTRY;

    for ( ; idx < m_area_array.Num(); idx++ )
    {
        ia = m_area_array[idx];
        if (( areatype == SAHPI_IDR_AREATYPE_UNSPECIFIED ) ||
            ( areatype == ia->AreaType() ))
        {
            nextareaid = ia->AreaId();
            break;
        }
    }
    
    return SA_OK;
}

SaErrorT
cIpmiInventoryParser::AddIdrArea( SaHpiIdrIdT &idrid,
                                  SaHpiIdrAreaTypeT &areatype,
			                      SaHpiEntryIdT &areaid )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    // Read only support for the moment
    return SA_ERR_HPI_READ_ONLY;
}

SaErrorT
cIpmiInventoryParser::DelIdrArea( SaHpiIdrIdT &idrid, SaHpiEntryIdT &areaid )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    cIpmiInventoryArea *ia = FindIdrArea( SAHPI_IDR_AREATYPE_UNSPECIFIED, areaid );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    // Read only support for the moment
    return SA_ERR_HPI_READ_ONLY;
}

SaErrorT
cIpmiInventoryParser::GetIdrField( SaHpiIdrIdT &idrid,
                                   SaHpiEntryIdT &areaid,
                                   SaHpiIdrFieldTypeT &fieldtype,
                                   SaHpiEntryIdT &fieldid,
			                       SaHpiEntryIdT &nextfieldid,
                                   SaHpiIdrFieldT &field )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    cIpmiInventoryArea *ia = FindIdrArea( SAHPI_IDR_AREATYPE_UNSPECIFIED, areaid );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    return ia->GetIdrField( fieldtype, fieldid, nextfieldid, field );
}

SaErrorT
cIpmiInventoryParser::AddIdrField( SaHpiIdrIdT &idrid, SaHpiIdrFieldT &field )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    cIpmiInventoryArea *ia = FindIdrArea( SAHPI_IDR_AREATYPE_UNSPECIFIED, field.AreaId );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    // Read only support for the moment
    return SA_ERR_HPI_READ_ONLY;
}

SaErrorT
cIpmiInventoryParser::SetIdrField( SaHpiIdrIdT &idrid, SaHpiIdrFieldT &field )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    cIpmiInventoryArea *ia = FindIdrArea( SAHPI_IDR_AREATYPE_UNSPECIFIED, field.AreaId );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    // Read only support for the moment
    return SA_ERR_HPI_READ_ONLY;
}

SaErrorT
cIpmiInventoryParser::DelIdrField( SaHpiIdrIdT &idrid,
                                   SaHpiEntryIdT &areaid,
                                   SaHpiEntryIdT &fieldid )
{
    if ( m_idr_info.IdrId != idrid )
        return SA_ERR_HPI_NOT_PRESENT;

    cIpmiInventoryArea *ia = FindIdrArea( SAHPI_IDR_AREATYPE_UNSPECIFIED, areaid );

    if ( ia == NULL )
        return SA_ERR_HPI_NOT_PRESENT;

    // Read only support for the moment
    return SA_ERR_HPI_READ_ONLY;
}
