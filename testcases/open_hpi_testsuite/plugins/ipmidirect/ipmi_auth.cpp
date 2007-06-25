/*
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

#include "ipmi_auth.h"
#include <string.h>
#include <errno.h>


cIpmiAuth *
IpmiAuthFactory( tIpmiAuthType type )
{
  switch( type )
     {
       case eIpmiAuthTypeNone:
            return new cIpmiAuthNone;

       case eIpmiAuthTypeMd2:
#ifdef HAVE_OPENSSL_MD2_H
            return new cIpmiAuthMd2;
#else
            break;
#endif

       case eIpmiAuthTypeMd5:
#ifdef HAVE_OPENSSL_MD5_H
            return new cIpmiAuthMd5;
#else
            break;
#endif

       case eIpmiAuthTypeStraight:
            return new cIpmiAuthStraight;

       case eIpmiAuthTypeOem:
            break;
     }

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


#ifdef HAVE_OPENSSL_MD2_H
#include <openssl/md2.h>


int
cIpmiAuthMd2::Init( const unsigned char *password )
{
  memcpy( data, password, 16 );

  return 0;
}


int
cIpmiAuthMd2::Gen( cIpmiAuthSg d[],
                   void       *output )
{
  MD2_CTX ctx;

  MD2_Init( &ctx );
  MD2_Update( &ctx, data, 16 );

  for( int i = 0; d[i].data != 0; i++ )
       MD2_Update( &ctx, (unsigned char *)d[i].data, d[i].len );

  MD2_Update( &ctx, data, 16 );
  MD2_Final( (unsigned char *)output, &ctx );

  return 0;
}


int
cIpmiAuthMd2::Check( cIpmiAuthSg d[],
                     void       *code )
{
  MD2_CTX ctx;
  unsigned char md[16];

  MD2_Init( &ctx );
  MD2_Update( &ctx, data, 16 );

  for( int i = 0; d[i].data != 0; i++ )
       MD2_Update( &ctx, (unsigned char *)d[i].data, d[i].len );

  MD2_Update( &ctx, data, 16 );
  MD2_Final( md, &ctx );

  if ( memcmp( code, md, 16 ) != 0 )
       return EINVAL;

  return 0;
}
#endif


#ifdef HAVE_OPENSSL_MD5_H
#include <openssl/md5.h>


int
cIpmiAuthMd5::Init( const unsigned char *password )
{
  memcpy( data, password, 16 );

  return 0;
}


int
cIpmiAuthMd5::Gen( cIpmiAuthSg d[],
                   void        *output )
{
  MD5_CTX ctx;

  MD5_Init( &ctx );
  MD5_Update( &ctx, data, 16 );

  for( int i = 0; d[i].data != 0; i++ )
       MD5_Update( &ctx, d[i].data, d[i].len );

  MD5_Update( &ctx, data, 16 );
  MD5_Final( (unsigned char *)output, &ctx );

  return 0;
}


int
cIpmiAuthMd5::Check( cIpmiAuthSg d[],
                     void       *code )
{
  MD5_CTX ctx;
  unsigned char md[16];

  MD5_Init( &ctx );
  MD5_Update( &ctx, data, 16 );

  for( int i = 0; d[i].data != 0; i++ )
       MD5_Update( &ctx, d[i].data, d[i].len );

  MD5_Update( &ctx, data, 16 );
  MD5_Final( md, &ctx );

  if ( memcmp( code, md, 16 ) != 0 )
       return EINVAL;

  return 0;
}
#endif


int
cIpmiAuthStraight::Init( const unsigned char *password )
{
  memcpy( data, password, 16 );

  return 0;
}


int
cIpmiAuthStraight::Gen( cIpmiAuthSg /*d*/[], void *output )
{
  memcpy( output, data, 16 );
  return 0;
}


int
cIpmiAuthStraight::Check( cIpmiAuthSg /*d*/[], void *code )
{
  if ( strncmp( (char *)data, (char *)code, 16 ) != 0 )
       return EINVAL;

  return 0;
}
