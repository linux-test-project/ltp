/*
 * ipmi_type_code.cpp
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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
 */

#include <string.h>
#include <assert.h>

#include "ipmi_type_code.h"


static void IpmiGetUnicode  ( int len, unsigned char *d, int in_len, char *out, int out_len );
static void IpmiGetBcdPlus  ( int len, unsigned char *d, int in_len, char *out, int out_len );
static void IpmiGet6BitAscii( int len, unsigned char *d, int in_len, char *out, int out_len );
static void IpmiGet8BitAscii( int len, unsigned char *d, int in_len, char *out, int out_len );

static void IpmiSetBcdPlus  ( char *input, unsigned char *output, int *out_len );
static void IpmiSet6BitAscii( char *input, unsigned char *output, int *out_len );
static void IpmiSet8BitAscii( char *input, unsigned char *output, int *out_len );


static void
IpmiGetUnicode(int /*len*/,
               unsigned char * /*d*/, int /*in_len*/,
               char *out, int /*out_len*/ )
{
  /* FIXME - no unicode handling. */
  *out = '\0';
}


static void
IpmiGetBcdPlus(int len,
               unsigned char *d, int in_len,
               char *out, int out_len)
{
  static char table[16] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ' ', '-', '.', ':', ',', '_'
  };
  
  int pos;
  int bo;
  int val = 0;
  int i;
  int real_length;
  
  real_length = (in_len * 8) / 6;
  if ( len > real_length )
       len = real_length;
    
  if ( len > out_len )
       len = out_len;

  bo = 0;
  pos = 0;
  for( i=0; i<len; i++ )
     {
       switch( bo )
          {
	    case 0:
                 val = *d & 0xf;
                 bo = 4;
                 break;

	    case 4:
                 val = (*d >> 4) & 0xf;
                 d++;
                 bo = 0;
                 break;
          }

       *out = table[val];
       out++;
     }

  *out = '\0';
}


static void
IpmiGet6BitAscii(int len,
                 unsigned char *d, int in_len,
                 char *out, int out_len )
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
  
  int pos;
  int bo;
  int val = 0;
  int i;
  int real_length;

  real_length = (in_len * 8) / 6;
  if (len > real_length)
       len = real_length;
    
  if ( len > out_len )
       len = out_len;
  
  bo = 0;
  pos = 0;

  for( i=0; i<len; i++ )
     {
       switch( bo )
          {
	    case 0:
                 val = *d & 0x3f;
                 bo = 6;
                 break;

	    case 2:
                 val = (*d >> 2) & 3;
                 d++;
                 val |= (*d & 0xf) << 2;
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

       *out = table[val];
       out++;
     }

  *out = '\0';
}


static void
IpmiGet8BitAscii( int len,
                  unsigned char *d, int in_len,
                  char *out, int out_len)
{
  int j;
    
  if ( len > in_len )
       len = in_len;

  if ( len > out_len )
       len = out_len;

  for( j=0; j<len; j++ )
     {
       *out = *d;
       out++;
       d++;
     }

  *out = '\0';
}


void
IpmiGetDeviceString( unsigned char *input,
                     int           in_len,
                     char          *output,
                     int           max_out_len )
{
  int type;
  int len;
  
  if ( max_out_len <= 0 )
       return;

  if ( in_len < 2 )
     {
       *output = '\0';
       return;
     }

  /* Remove the nil from the length. */
  max_out_len--;

  type = (*input >> 6) & 3;
  len = *input & 0x3f;
  input++;
  in_len--;

  switch( type )
     {
       case 0: /* Unicode */
	    IpmiGetUnicode( len, input, in_len, output, max_out_len);
	    break;

       case 1: /* BCD Plus */
	    IpmiGetBcdPlus( len, input, in_len, output, max_out_len);
	    break;

       case 2: /* 6-bit ASCII */
	    IpmiGet6BitAscii( len, input, in_len, output, max_out_len);
	    break;

       case 3: /* 8-bit ASCII */
	    IpmiGet8BitAscii(len, input, in_len, output, max_out_len);
	    break;
     }
}


/* Element will be zero if not present, n-1 if present. */
static char table_4_bit[256] =
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


/* Element will be zero if not present, n-1 if present. */
static char table_6_bit[256] =
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


static void
IpmiSetBcdPlus( char          *input,
                unsigned char *output,
                int           *out_len )
{
  int  len = *out_len;
  char *s = input;
  int  pos = 0;
  int  bit = 0;
  int  count = 0;
  
  while( *s != '\0' )
     {
       if ( pos >= len )
          {
	    output[0] = (0x01 << 6) | count;
	    return;
          }

       switch( bit )
          {
	    case 0:
                 pos++;
                 output[pos] = table_4_bit[(int) *s];
                 bit = 4;
                 break;

	    case 4:
                 output[pos] |= table_4_bit[(int) *s] << 4;
                 bit = 0;
                 break;
          }

       count++;
     }

  output[0] = (0x01 << 6) | count;
  *out_len = pos+1;
}


static void
IpmiSet6BitAscii( char          *input,
                  unsigned char *output,
                  int           *out_len )
{
  int  len = *out_len;
  char *s = input+1;
  int  pos = 0;
  int  bit = 0;
  int  count = 0;
  int  cval;
  
  while( *s != '\0' )
     {
       if ( pos >= len )
          {
	    output[0] = (0x02 << 6) | count;
	    return;
          }

       cval = *s;

       switch( bit )
          {
	    case 0:
                 pos++;
                 output[pos] = table_6_bit[cval];
                 bit = 6;
                 break;

	    case 2:
                 output[pos] |= (table_6_bit[cval] << 2);
                 bit = 0;
                 break;

	    case 4:
                 output[pos] |= table_4_bit[cval] << 4;
                 pos++;
                 output[pos] = (table_4_bit[cval] >> 4) & 0x3;
                 bit = 2;
                 break;
                 
	    case 6:
                 output[pos] |= table_4_bit[cval] << 6;
                 pos++;
                 output[pos] = (table_4_bit[cval] >> 2) & 0xf;
                 bit = 4;
                 break;
          }

       count++;
    }

  output[0] = (0x02 << 6) | count;
  *out_len = pos+1;
}


static void
IpmiSet8BitAscii( char          *input,
                  unsigned char *output,
                  int           *out_len )
{
  int len = strlen(input+1);
  if ( len > (*out_len - 1))
       len = *out_len - 1;
  else
       *out_len = len + 1;

  strncpy( input, (char *)output+1, len );
  output[0] = (0x02 << 6) | len;
}


void
IpmiSetDeviceString( char          *input,
                     unsigned char *output,
                     int           *out_len )
{
  char *s = input+1;
  int  bsize = 0; /* Start with 4-bit. */

  /* Max size is 30. */
  if ( *out_len > 30 )
       *out_len = 30;

  while( *s != '\0' )
     {
       if ( (bsize == 0) && (table_4_bit[(int) *s] == 0) )
	    bsize = 1;

       if ((bsize == 1) && (table_6_bit[(int) *s] == 0))
          {
	    bsize = 2;
	    break;
          }
     }

  if ( bsize == 0 )
     {
       /* We can encode it in 4-bit BCD+ */
       IpmiSetBcdPlus( input, output, out_len );
     } 
  else if ( bsize == 1 )
     {
       /* We can encode it in 6-bit ASCII. */
       IpmiSet6BitAscii( input, output, out_len );
     }
  else
     {
       /* 8-bit ASCII is required. */
       IpmiSet8BitAscii( input, output, out_len );
    }
}


unsigned char *
cIpmiTextBuffer::Set(  unsigned char *data, tIpmiLanguge l )
{
  m_language = l;
  m_type     = eIpmiTextTypeBinary;
  m_len      = 0;
  memset( m_data, 0, dMaxTextBufferLength );

  if ( *data == 0xc1 )
       return 0; // end mark

  m_type = (tIpmiTextType)((*data >> 6) & 3);
  m_len = *data & 0x3f;

  data++;

  memcpy( m_data, data, m_len );

  data += m_len;

  return data;
}


int
cIpmiTextBuffer::Binary2Ascii( char *buffer, unsigned int len )
{
  unsigned int l = m_len;

  if ( l >= len )
       l = len - 1;

  memcpy( buffer, m_data, l );
  buffer[len-1] = 0;

  return len;
}


int
cIpmiTextBuffer::BcdPlus2Ascii( char *buffer, unsigned int len )
{
  static char table[] = "0123456789 -.:,_";

  unsigned int real_length = 2 * m_len;

  if ( len > real_length )
       len = real_length;

  bool first = true;
  unsigned char *d = m_data;

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
cIpmiTextBuffer::Ascii62Ascii( char *buffer, unsigned int len )
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

  unsigned int real_length = (m_len * 8) / 6;

  if ( len >= real_length )
       len = real_length;

  unsigned char *d = m_data;
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
                 val = (*d >> 2) & 3;
                 d++;
                 val |= (*d & 0xf) << 2;
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
cIpmiTextBuffer::Language2Ascii( char *buffer, unsigned int len )
{
  if ( m_language == eIpmiLanguageEnglish )
       return Binary2Ascii( buffer, len );

  // unicode not supported
  return -1;
}


int
cIpmiTextBuffer::GetAscii( char *buffer, unsigned int len )
{
  switch( m_type )
     {
       case eIpmiTextTypeBinary:
            return Binary2Ascii( buffer, len );

       case eIpmiTextTypeBcdplus:
            return BcdPlus2Ascii( buffer, len );

       case eIpmiTextTypeAscii6:
            return Ascii62Ascii( buffer, len );

       case eIpmiTextTypeLanguage:
            return Language2Ascii( buffer, len );

       default:
            assert( 0 );
            return -1;
     }

  // not reached
  return -1;
}
