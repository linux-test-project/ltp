/*
 * ipmi_type_code.h
 *
 * Copyright (c) 2003 by FORCE Computers
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


#ifndef dIpmiTypeCode_h
#define dIpmiTypeCode_h


void IpmiGetDeviceString( unsigned char *input,
                          int           in_len,
                          char          *output,
                          int           max_out_len );

void IpmiSetDeviceString( char          *input,
                          unsigned char *output,
                          int           *out_len );

enum tIpmiTextType
{
  eIpmiTextTypeBinary = 0,
  eIpmiTextTypeBcdplus,
  eIpmiTextTypeAscii6,
  eIpmiTextTypeLanguage
};

enum tIpmiLanguge
{
  eIpmiLanguageEnglish
};


#define dMaxTextBufferLength  64


class cIpmiTextBuffer
{
protected:
  int Binary2Ascii( char *buffer, unsigned int len );
  int BcdPlus2Ascii( char *buffer, unsigned int len );
  int Ascii62Ascii( char *buffer, unsigned int len );
  int Language2Ascii( char *buffer, unsigned int len );

public:
  tIpmiLanguge  m_language;
  tIpmiTextType m_type;
  unsigned int  m_len;
  unsigned char m_data[dMaxTextBufferLength];

  // there is no ct because cIpmiTextBuffer is used in unions !
  unsigned char *Set( unsigned char *data, tIpmiLanguge l = eIpmiLanguageEnglish );

  // returns length of string or -1 on error
  int GetAscii( char *buffer, unsigned int len );
};


#endif
