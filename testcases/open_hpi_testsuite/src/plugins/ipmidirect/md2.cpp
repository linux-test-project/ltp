/*
 *
 * Copyright (c) 2003 by FORCE Computers.
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "md2.h"

#ifdef HAVE_OPENSSL_MD2_H
#include <openssl/md2.h>
#endif


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
#ifdef HAVE_OPENSSL_MD2_H
  MD2_CTX ctx;

  MD2_Init( &ctx );
  MD2_Update( &ctx, data, 16 );

  for( int i = 0; d[i].data != 0; i++ )
       MD2_Update( &ctx, (unsigned char *)d[i].data, d[i].len );

  MD2_Update( &ctx, data, 16 );
  MD2_Final( (unsigned char *)output, &ctx );

  return 0;
#else
  return EINVAL;
#endif
}


int
cIpmiAuthMd2::Check( cIpmiAuthSg d[],
                     void       *code )
{
#ifdef HAVE_OPENSSL_MD2_H
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
#else
  return EINVAL;
#endif
}
