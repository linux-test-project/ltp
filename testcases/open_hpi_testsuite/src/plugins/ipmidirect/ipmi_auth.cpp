/*
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

#include <string.h>
#include <errno.h>
#include <assert.h>

#include "ipmi_auth.h"


int
cIpmiAuthPw::Init( const unsigned char *password )
{
  memcpy( data, password, 16 );

  return 0;
}


int
cIpmiAuthPw::Gen( cIpmiAuthSg /*d*/[], void *output )
{
  memcpy( output, data, 16 );
  return 0;
}


int
cIpmiAuthPw::Check( cIpmiAuthSg /*d*/[], void *code )
{
  if ( strncmp( (char *)data, (char *)code, 16 ) != 0 )
       return EINVAL;

  return 0;
}


int
cIpmiAuthNone::Init( const unsigned char * /*password*/ )
{
  return 0;
}


int
cIpmiAuthNone::Gen( cIpmiAuthSg /*d*/[], void *output )
{
  memset( output, 0, 16 );
  return 0;
}


int
cIpmiAuthNone::Check( cIpmiAuthSg /*d*/[], void * /*code*/ )
{
  return 0;
}


cIpmiAuth *
IpmiAuthFactory( tIpmiAuthType type )
{
  switch( type )
     {
       case eIpmiAuthTypeNone:
            return new cIpmiAuthNone;

       case eIpmiAuthTypeMd2:
            return new cIpmiAuthMd2;

       case eIpmiAuthTypeMd5:
            return new cIpmiAuthMd5;

       case eIpmiAuthTypeStraight:
            return new cIpmiAuthPw;

       case eIpmiAuthTypeOem:
            break;
     }

  assert( 0 );
  return 0;
}
