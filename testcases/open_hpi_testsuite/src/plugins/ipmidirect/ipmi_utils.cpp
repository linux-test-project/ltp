/*
 * ipmi_utils.cpp
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

#include <time.h>

#include "ipmi_utils.h"


unsigned int
IpmiGetUint32( const unsigned char *data )
{
    return (data[0]
	    | (data[1] << 8)
	    | (data[2] << 16)
	    | (data[3] << 24));
}


/* Extract a 16-bit integer from the data, IPMI (little-endian) style. */
unsigned int
IpmiGetUint16( const unsigned char *data )
{
    return data[0] | (data[1] << 8);
}


/* Add a 32-bit integer to the data, IPMI (little-endian) style. */
void
IpmiSetUint32( unsigned char *data, int val )
{
    data[0] =  val & 0xff;
    data[1] = (val >> 8) & 0xff;
    data[2] = (val >> 16) & 0xff;
    data[3] = (val >> 24) & 0xff;
}


/* Add a 16-bit integer to the data, IPMI (little-endian) style. */
void
IpmiSetUint16( unsigned char *data, int val )
{
    data[0] = val & 0xff;
    data[1] = (val >> 8) & 0xff;
}


void
IpmiDateToString( unsigned int t, char *str )
{
  struct tm tm;
  time_t dummy = t;

  localtime_r( &dummy, &tm );

  // 2003.10.30
  strftime( str, dDateStringSize, "%Y.%m.%d", &tm );
}


void
IpmiTimeToString( unsigned int t, char *str )
{
  struct tm tm;
  time_t dummy = t;

  localtime_r( &dummy, &tm );

  // 11:11:11
  strftime( str, dTimeStringSize, "%H:%M:%S", &tm );
}


void
IpmiDateTimeToString( unsigned int t, char *str )
{
  struct tm tm;
  time_t dummy = t;
  localtime_r( &dummy, &tm );

  // 2003.10.30 11:11:11
  strftime( str, dDateTimeStringSize, "%Y.%m.%d %H:%M:%S", &tm );
}
