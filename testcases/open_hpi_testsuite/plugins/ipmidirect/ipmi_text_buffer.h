/*
 * ipmi_text_buffer.h
 *
 * Copyright (c) 2003 by FORCE Computers
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


#ifndef dIpmiTextBuffer_h
#define dIpmiTextBuffer_h


#ifndef dIpmiLog_h
#include "ipmi_log.h"
#endif

extern "C" {
#include "SaHpi.h"
}


// wrapper class for SaHpiTextBufferT
class cIpmiTextBuffer
{
protected:
  int BinaryToAscii( char *buffer, unsigned int len ) const;
  int BcdPlusToAscii( char *buffer, unsigned int len ) const;
  int Ascii6ToAscii( char *buffer, unsigned int len ) const;
  int LanguageToAscii( char *buffer, unsigned int len ) const;

  int AsciiToBcdPlus ( const char *input );
  int AsciiToAscii6  ( const char *input );
  int AsciiToLanguage( const char *input );

  SaHpiTextBufferT m_buffer;

public:
  cIpmiTextBuffer();
  cIpmiTextBuffer( const char *string, SaHpiTextTypeT type, 
		   SaHpiLanguageT l = SAHPI_LANG_ENGLISH );
  cIpmiTextBuffer( const unsigned char *data,
		   SaHpiLanguageT l = SAHPI_LANG_ENGLISH );
  cIpmiTextBuffer( const SaHpiTextBufferT &buf );

  void Clear();

  operator SaHpiTextBufferT () const
  {
    return m_buffer;
  }

  SaHpiUint8T DataLength() const { return m_buffer.DataLength; }
  
  const unsigned char *SetIpmi( const unsigned char *data, bool is_fru = false,
				SaHpiLanguageT l = SAHPI_LANG_ENGLISH );

  SaHpiTextTypeT CheckAscii( const char *s );

  // convert ascii string to text buffer
  bool SetAscii( const char *string, SaHpiTextTypeT type,
		 SaHpiLanguageT l = SAHPI_LANG_ENGLISH );

  // returns length of string or -1 on error
  int GetAscii( char *buffer, unsigned int len ) const;

  bool operator==( const cIpmiTextBuffer &tb ) const;
  bool operator!=( const cIpmiTextBuffer &tb ) const;
};


cIpmiLog &operator<<( cIpmiLog &dump, const cIpmiTextBuffer &tb );


#endif
