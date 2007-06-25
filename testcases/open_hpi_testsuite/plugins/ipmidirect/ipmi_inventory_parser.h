
/*
 * ipmi_inventory_parser.h
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

#ifndef dIpmiInventoryParser_h
#define dIpmiInventoryParser_h


#ifndef dArray_h
#include "array.h"
#endif

#ifndef dIpmiTextBuffer
#include "ipmi_text_buffer.h"
#endif


unsigned char IpmiChecksum( const unsigned char *data, int size );
unsigned char IpmiChecksumMulti( const unsigned char *data, int size, unsigned char csum );

enum tIpmiInventoryRecordType
{
  eIpmiInventoryRecordTypeInternal,
  eIpmiInventoryRecordTypeChassis,
  eIpmiInventoryRecordTypeBoard,
  eIpmiInventoryRecordTypeProduct,
  eIpmiInventoryRecordTypeMultiRecord,
  eIpmiInventoryRecordTypeLast
};

const char *IpmiInventoryRecordTypeToString( tIpmiInventoryRecordType );

#define eIpmiInventoryMultiRecordTypeOemFirst   0xc0
#define eIpmiInventoryMultiRecordTypeOemLast    0xff

class cIpmiInventoryField
{
  cIpmiTextBuffer   m_ipmi_text_buffer;
  SaHpiIdrFieldT    m_idr_field;

public:
  cIpmiInventoryField(SaHpiEntryIdT area_id,
                        SaHpiEntryIdT field_id,
                        SaHpiIdrFieldTypeT field_type);
  virtual ~cIpmiInventoryField();

  void                  SetAscii( char *str, int size );
  void                  SetBinary( const unsigned char *data, unsigned int size );
  SaHpiEntryIdT         FieldId() const { return m_idr_field.FieldId; }
  SaHpiIdrFieldTypeT    FieldType() const { return m_idr_field.Type; }
  SaHpiIdrFieldT        Field() const { return m_idr_field; }

  SaErrorT ReadTextBuffer( const unsigned char *&data, unsigned int &size );
};

class cIpmiInventoryArea
{
protected:
  SaHpiEntryIdT                 m_field_id;
  SaHpiIdrAreaHeaderT           m_area_header;
  cArray<cIpmiInventoryField>   m_field_array;

public:
  cIpmiInventoryArea(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryArea();

  cIpmiInventoryField *FindIdrField( SaHpiIdrFieldTypeT fieldtype,
                                     SaHpiEntryIdT fieldid );

  SaErrorT GetIdrField( SaHpiIdrFieldTypeT &fieldtype,
                        SaHpiEntryIdT &fieldid,
		                SaHpiEntryIdT &nextfieldid,
                        SaHpiIdrFieldT &field );

  SaHpiEntryIdT         AreaId() const { return m_area_header.AreaId; }
  SaHpiIdrAreaTypeT     AreaType() const { return m_area_header.Type; }
  SaHpiIdrAreaHeaderT   AreaHeader() const { return m_area_header; }


  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size ) = 0;


};

class cIpmiInventoryAreaInternal : public cIpmiInventoryArea
{

public:
  cIpmiInventoryAreaInternal(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryAreaInternal();

  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size );
};

class cIpmiInventoryAreaMultiRecord : public cIpmiInventoryArea
{

public:
  cIpmiInventoryAreaMultiRecord(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryAreaMultiRecord();

  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size );
};

class cIpmiInventoryAreaChassis : public cIpmiInventoryArea
{

public:
  cIpmiInventoryAreaChassis(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryAreaChassis();

  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size );
};

class cIpmiInventoryAreaBoard : public cIpmiInventoryArea
{

public:
  cIpmiInventoryAreaBoard(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryAreaBoard();

  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size );
};

class cIpmiInventoryAreaProduct : public cIpmiInventoryArea
{

public:
  cIpmiInventoryAreaProduct(SaHpiEntryIdT area_id);
  virtual ~cIpmiInventoryAreaProduct();

  virtual SaErrorT     ParseFruArea( const unsigned char *data, unsigned int size );
};

class cIpmiInventoryParser
{
  SaHpiIdrInfoT                 m_idr_info;
  SaHpiEntryIdT                 m_area_id;
  cArray<cIpmiInventoryArea>    m_area_array;

public:
  cIpmiInventoryParser();
  virtual ~cIpmiInventoryParser();

  cIpmiInventoryArea *AllocArea( SaHpiEntryIdT area_id, tIpmiInventoryRecordType type );


  virtual SaErrorT     ParseFruInfo( const unsigned char *data, unsigned int size, unsigned int idr_id );

  cIpmiInventoryArea *FindIdrArea( SaHpiIdrAreaTypeT areatype, SaHpiEntryIdT areaid );


  virtual SaErrorT GetIdrInfo( SaHpiIdrIdT &idrid, SaHpiIdrInfoT &idrinfo );

  virtual SaErrorT GetIdrAreaHeader( SaHpiIdrIdT &idrid,
                                     SaHpiIdrAreaTypeT &areatype,
			                         SaHpiEntryIdT &areaid,
                                     SaHpiEntryIdT &nextareaid,
                                     SaHpiIdrAreaHeaderT &header );

  virtual SaErrorT AddIdrArea( SaHpiIdrIdT &idrid,
                               SaHpiIdrAreaTypeT &areatype,
			                   SaHpiEntryIdT &areaid );

  virtual SaErrorT DelIdrArea( SaHpiIdrIdT &idrid, SaHpiEntryIdT &areaid );

  virtual SaErrorT GetIdrField( SaHpiIdrIdT &idrid,
                                SaHpiEntryIdT &areaid,
                                SaHpiIdrFieldTypeT &fieldtype,
                                SaHpiEntryIdT &fieldid,
			                    SaHpiEntryIdT &nextfieldid,
                                SaHpiIdrFieldT &field );

  virtual SaErrorT AddIdrField( SaHpiIdrIdT &idrid, SaHpiIdrFieldT &field );

  virtual SaErrorT SetIdrField( SaHpiIdrIdT &idrid, SaHpiIdrFieldT &field );

  virtual SaErrorT DelIdrField( SaHpiIdrIdT &idrid,
                                SaHpiEntryIdT &areaid,
                                SaHpiEntryIdT &fieldid );

};


#endif
