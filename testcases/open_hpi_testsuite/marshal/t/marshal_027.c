/*
 * Copyright (c) 2004 by FORCE Computers.
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

#include "marshal.h"
#include <stdlib.h>


static unsigned int my_user_data = 42;


static int
MyMarshalFunction( const cMarshalType *type, const void *data,
		   void *buffer, void *user_data )
{
  unsigned int *ud = (unsigned int *)user_data;

  if ( ud != &my_user_data )
       exit( 1 );

  if ( *ud != my_user_data )
       exit( 1 );

  const tUint8 *d = (const tUint8 *)data;
  tUint8 *b = (tUint8 *)buffer;

  tUint8 v1 = *d++;
  tUint8 v2 = *d++;
  tUint8 v3 = *d++;

  *b++ = v2;
  *b++ = v3;
  *b++ = v1;

  return 3;
}


static int
MyDemarshalFunction( int byte_order, const cMarshalType *type, void *data,
		     const void *buffer, void *user_data )
{
  unsigned int *ud = (unsigned int *)user_data;

  if ( ud != &my_user_data )
       exit( 1 );

  if ( *ud != my_user_data )
       exit( 1 );

  tUint8 *d = (tUint8 *)data;
  const tUint8 *b = (const tUint8 *)buffer;

  tUint8 v2 = *b++;
  tUint8 v3 = *b++;
  tUint8 v1 = *b++;
  
  *d++ = v1;
  *d++ = v2;
  *d++ = v3;

  return 3;
}


static cMarshalType UserDefinedType = dUserDefined( MyMarshalFunction,
						    MyDemarshalFunction,
						    &my_user_data );


int
main( int argc, char *argv[] )
{
  tUint8         value[4]  = { 0x42, 0X43, 0X44, 0X45 };
  tUint8         result[4] = { 0x11, 0x12, 0x13, 0x14 };
  unsigned char  buffer[256];

  unsigned int s1 = Marshal( &UserDefinedType, value, buffer );

  if ( s1 != 3 )
       return 1;

  unsigned int s2 = Demarshal( MarshalByteOrder(),
                               &UserDefinedType, result, buffer );

  if ( s2 != 3 )
       return 1;

  if ( value[0] != result[0] )
       return 1;

  if ( value[1] != result[1] )
       return 1;

  if ( value[2] != result[2] )
       return 1;

  if ( result[3] != 0x14 )
       return 1;

  return 0;
}
