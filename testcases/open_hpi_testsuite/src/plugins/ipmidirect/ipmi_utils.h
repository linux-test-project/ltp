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

#ifndef dIpmiUtils_h
#define dIpmiUtils_h

unsigned int IpmiGetUint16( const unsigned char *data );
void         IpmiSetUint16( unsigned char *data, int val );

unsigned int IpmiGetUint32( const unsigned char *data );
void         IpmiSetUint32( unsigned char *data, int val );


#define dDateStringSize 11
void IpmiDateToString( unsigned int time, char *str );

#define dTimeStringSize 9
void IpmiTimeToString( unsigned int time, char *str );

#define dDateTimeStringSize 20
void IpmiDateTimeToString( unsigned int time, char *str );

#endif
