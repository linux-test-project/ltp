/*
 * ipmi_text_buffer.cpp
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
 */

#include <string.h>

#include "ipmi_text_buffer.h"


cIpmiTextBuffer::cIpmiTextBuffer()
{
  Clear();
}


cIpmiTextBuffer::cIpmiTextBuffer( const char *string, SaHpiTextTypeT type, 
				  SaHpiLanguageT l )
{
  m_buffer.DataType = type;
  m_buffer.Language = l;

  SetAscii( string, type, l );
}


cIpmiTextBuffer::cIpmiTextBuffer( const unsigned char *data,
				  SaHpiLanguageT l )
{
  SetIpmi( data, false, l );
}


cIpmiTextBuffer::cIpmiTextBuffer( const SaHpiTextBufferT &buf )
{
  m_buffer = buf;
}


void 
cIpmiTextBuffer::Clear()
{
  m_buffer.DataType   = SAHPI_TL_TYPE_TEXT;
  m_buffer.Language   = SAHPI_LANG_ENGLISH;
  m_buffer.DataLength = 0;
  memset( m_buffer.Data, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH );
}


const unsigned char *
cIpmiTextBuffer::SetIpmi( const unsigned char *data, bool is_fru, SaHpiLanguageT l )
{
  Clear();
  m_buffer.Language = l;

  if ( *data == 0xc1 )
       return 0; // end mark

  m_buffer.DataType   = (SaHpiTextTypeT)((*data >> 6) & 3);
  if (( is_fru == true )
      && ( m_buffer.DataType == SAHPI_TL_TYPE_UNICODE ))
  {
    m_buffer.DataType = SAHPI_TL_TYPE_BINARY;
  }

  m_buffer.DataLength = *data & 0x3f;

  data++;

  memcpy( m_buffer.Data, data, m_buffer.DataLength );

  data += m_buffer.DataLength;

  if (( m_buffer.DataType == SAHPI_TL_TYPE_BCDPLUS )
      || ( m_buffer.DataType == SAHPI_TL_TYPE_ASCII6 ))
    {
      char tmpstr[SAHPI_MAX_TEXT_BUFFER_LENGTH];
      int len;

      len = GetAscii( tmpstr, sizeof (tmpstr) );

      if ( len == -1 )
          return 0;

      m_buffer.DataLength = len;

      memcpy( m_buffer.Data, tmpstr, m_buffer.DataLength );
    }

  return data;
}


// Element will be zero if not present, n-1 if present.
static SaHpiUint8T table_4_bit[256] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0c, 0x0d, 0x00,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


int
cIpmiTextBuffer::AsciiToBcdPlus( const char *s )
{
  m_buffer.DataType = SAHPI_TL_TYPE_BCDPLUS;
  m_buffer.DataLength = 0;

  SaHpiUint8T *p = m_buffer.Data;
  int   bit = 0;

  while( *s )
     {
       if ( m_buffer.DataLength == SAHPI_MAX_TEXT_BUFFER_LENGTH )
	    break;

       switch( bit )
          {
	    case 0:
                 m_buffer.DataLength++;
                 *p = table_4_bit[(unsigned int)*s];
                 bit = 4;
                 break;

	    case 4:
                 *p |= table_4_bit[(unsigned int)*s++] << 4;
                 p++;
                 bit = 0;
                 break;
          }
     }

  return m_buffer.DataLength;
}


// Element will be zero if not present, n-1 if present.
static SaHpiUint8T table_6_bit[256] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x21, 0x08,
  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
  0x00, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
  0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


////////
// 0       1       2       3
// 0123456789012345678901234567890
// 000000111111222222333333444444

int
cIpmiTextBuffer::AsciiToAscii6( const char *s )
{
  m_buffer.DataType   = SAHPI_TL_TYPE_ASCII6;
  m_buffer.DataLength = 0;

  SaHpiUint8T *p = m_buffer.Data;
  int   bit = 0;

  while( *s )
     {
       if ( m_buffer.DataLength == SAHPI_MAX_TEXT_BUFFER_LENGTH )
	    break;

       switch( bit )
          {
	    case 0:
                 *p = table_6_bit[(unsigned int)*s++];
		 m_buffer.DataLength++;
                 bit = 6;
                 break;

	    case 2:
                 *p |= (table_6_bit[(unsigned int)*s] << 2);
                 bit = 0;
                 break;

	    case 4:
                 *p |= table_4_bit[(unsigned int)*s] << 4;
                 p++;
                 *p = (table_4_bit[(unsigned int)*s++] >> 4) & 0x3;
		 m_buffer.DataLength++;
                 bit = 2;
                 break;

	    case 6:
                 *p |= table_4_bit[(unsigned int)*s] << 6;
                 p++;
                 *p = (table_4_bit[(unsigned int)*s++] >> 2) & 0xf;
		 m_buffer.DataLength++;
                 bit = 4;
                 break;
          }
    }

  return m_buffer.DataLength;
}


int
cIpmiTextBuffer::AsciiToLanguage( const char *s )
{
  m_buffer.DataType = SAHPI_TL_TYPE_TEXT;

  int l = strlen( s );

  if ( l > SAHPI_MAX_TEXT_BUFFER_LENGTH )
       l = SAHPI_MAX_TEXT_BUFFER_LENGTH;

  m_buffer.DataLength = l;

  strncpy( ( char *)m_buffer.Data, s, SAHPI_MAX_TEXT_BUFFER_LENGTH );

  return l;
}


bool
cIpmiTextBuffer::SetAscii( const char *string, SaHpiTextTypeT type,
			   SaHpiLanguageT l )
{
  m_buffer.Language = l;
  
  switch( type )
     {
       case SAHPI_TL_TYPE_BCDPLUS:
	    AsciiToBcdPlus( string );
	    return true;

       case SAHPI_TL_TYPE_ASCII6:
	    AsciiToAscii6( string );	    
	    return true;

       case SAHPI_TL_TYPE_TEXT:
	    AsciiToLanguage( string );
	    return true;
	    
       default:
	    break;
     }

  return false;
}


SaHpiTextTypeT
cIpmiTextBuffer::CheckAscii( const char *s )
{
  SaHpiTextTypeT type = SAHPI_TL_TYPE_BCDPLUS;

  while( *s )
     {
       if ( type  == SAHPI_TL_TYPE_BCDPLUS && table_4_bit[(int)*s] == 0 )
	    type = SAHPI_TL_TYPE_ASCII6;

       if ( type == SAHPI_TL_TYPE_ASCII6 && table_6_bit[(int) *s] == 0 )
          {
	    type = SAHPI_TL_TYPE_TEXT;
	    break;
          }
     }
  
  return type;
}


int
cIpmiTextBuffer::BinaryToAscii( char *buffer, unsigned int len ) const
{
  unsigned int l = m_buffer.DataLength;

  if ( l >= len )
       l = len - 1;

  memcpy( buffer, m_buffer.Data, l );
  buffer[l] = 0;

  return len;
}


int
cIpmiTextBuffer::BcdPlusToAscii( char *buffer, unsigned int len ) const
{
  static char table[] = "0123456789 -.:,_";

  unsigned int real_length = 2 * m_buffer.DataLength;

  if ( len > real_length )
       len = real_length;

  bool first = true;
  const unsigned char *d = m_buffer.Data;

  for( unsigned int i = 0; i < len; i++ )
     {
       int val = 0;

       if ( first )
            val = *d & 0xf;
       else
            val = (*d++ >> 4) & 0xf;

       first = !first;
       *buffer++ = table[val];
     }

  *buffer = 0;

  return len;
}


int
cIpmiTextBuffer::Ascii6ToAscii( char *buffer, unsigned int len ) const
{
  static char table[64] =
  {
    ' ', '!', '"', '#', '$', '%', '&', '\'',
    '(', ')', '*', '+', ',', '-', '.', '/', 
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', '<', '=', '>', '?',
    '&', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '[', '\\', ']', '^', '_' 
  };

  unsigned int real_length = (m_buffer.DataLength * 8) / 6;

  if ( len >= real_length )
       len = real_length;

  const unsigned char *d = m_buffer.Data;
  int bo = 0;

  for( unsigned int i =0; i < len; i++ )
     {
       int val = 0;

       switch( bo )
          {
	    case 0:
                 val = *d & 0x3f;
                 bo = 6;
                 break;

	    case 2:
                 val = (*d >> 2) & 0x3f;
                 d++;
                 bo = 0;
                 break;

	    case 4:
                 val = (*d >> 4) & 0xf;
                 d++;
                 val |= (*d & 0x3) << 4;
                 bo = 2;
                 break;
                 
	    case 6:
                 val = (*d >> 6) & 0x3;
                 d++;
                 val |= (*d & 0xf) << 2;
                 bo = 4;
                 break;
          }

       *buffer++ = table[val];
     }

  *buffer = 0;

  return len;
}


int
cIpmiTextBuffer::LanguageToAscii( char *buffer, unsigned int len ) const
{
  if ( m_buffer.Language == SAHPI_LANG_ENGLISH )
       return BinaryToAscii( buffer, len );

  // unicode not supported
  return -1;
}


int
cIpmiTextBuffer::GetAscii( char *buffer, unsigned int len ) const
{
  switch( m_buffer.DataType )
     {
       case SAHPI_TL_TYPE_BINARY:
            return BinaryToAscii( buffer, len );

       case SAHPI_TL_TYPE_BCDPLUS:
            return BcdPlusToAscii( buffer, len );

       case SAHPI_TL_TYPE_ASCII6:
            return Ascii6ToAscii( buffer, len );

       case SAHPI_TL_TYPE_TEXT:
            return LanguageToAscii( buffer, len );

       default:
            return -1;
     }
}


bool
cIpmiTextBuffer::operator==( const cIpmiTextBuffer &tb ) const
{
  if ( m_buffer.DataType != tb.m_buffer.DataType )
       return false;
  
  if ( m_buffer.Language != tb.m_buffer.Language )
       return false;

  if ( m_buffer.DataLength != tb.m_buffer.DataLength )
       return false;

  if ( m_buffer.DataLength )
       return memcmp( m_buffer.Data, m_buffer.Data, tb.m_buffer.DataLength ) == 0;

  return true;
}


bool
cIpmiTextBuffer::operator!=( const cIpmiTextBuffer &tb ) const
{
  return !operator==( tb );
}


cIpmiLog &
operator<<( cIpmiLog &dump, const cIpmiTextBuffer &tb )
{
  char str[2*SAHPI_MAX_TEXT_BUFFER_LENGTH+1] = "";
  tb.GetAscii( str, 2*SAHPI_MAX_TEXT_BUFFER_LENGTH+1 );

  dump << str;

  return dump;
}
