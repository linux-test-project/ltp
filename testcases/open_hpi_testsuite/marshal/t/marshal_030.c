/*
 * Copyright (c) 2007 by Sun Microsystems, Inc.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Chris Rinaldo <chris.rinaldo@sun.com>
 */

#include "marshal.h"
#include <glib.h>
#include <string.h>


int
main( int argc, char *argv[] )
{
  tFloat32 value = -1.2113584441986872e-18;
  tFloat32 result;
  tUint32 swap;
  memcpy( &swap, &value, sizeof( tUint32 ) );
  swap = GUINT32_SWAP_LE_BE( swap );

  unsigned int s = Demarshal( MarshalByteOrder() ? 0 : 1,
                              &Marshal_Float32Type, &result, &swap );

  if ( s != sizeof( tFloat32 ) )
       return 1;

  if ( value != result )
       return 1;

  return 0;
}
