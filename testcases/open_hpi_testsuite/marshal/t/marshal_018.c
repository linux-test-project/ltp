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
#include <glib.h>


int
main( int argc, char *argv[] )
{
  tInt64 value = 0x1234567890abcdefLL;
  tInt64 swap = GUINT64_SWAP_LE_BE( value );
  tInt64 result;

  unsigned int s = Demarshal( MarshalByteOrder() ? 0 : 1,
                              &Marshal_Int64Type, &result, &swap );

  if ( s != sizeof( tInt64 ) )
       return 1;

  if ( value != result )
       return 1;

  return 0;
}
