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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <glib.h>
#include <math.h>


void ReadFile( const char *filename, void *p, int size );
void WriteFile( const char *filename, void *p, int size );


static tFloat32 float32_data[] = 
{
  0.0,
  1.0,
  2.0,
  3.0,
  0.5,
  0.25,
  0.75,
  -1.0,
  -2.0,
  -3.0,
  M_PI
};

static int float32_data_num = sizeof( float32_data ) / sizeof( tFloat32 );

static tFloat64 float64_data[] =
{
  0.0,
  1.0,
  2.0,
  3.0,
  0.5,
  0.25,
  0.75,
  -1.0,
  -2.0,
  -3.0,
  M_PI
};

static int float64_data_num = sizeof( float64_data ) / sizeof( tFloat64 );


void
ReadFile( const char *filename, void *p, int size )
{
  int fd = open( filename, O_RDONLY );

  if ( fd == -1 )
     {
       fprintf( stderr, "cannot open %s: %s !\n", filename, 
                strerror( errno ) );

       close( fd );

       return;
     }

  int rv = read( fd, p, size );

  if ( rv != size )
       fprintf( stderr, "cannot read %s: %s !\n", filename, 
                strerror( errno ) );

  close( fd );
}


void 
WriteFile( const char *filename, void *p, int size )
{
  int fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY,
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

  if ( fd == -1 )
     {
       fprintf( stderr, "cannot create %s: %s !\n", filename, 
                strerror( errno ) );

       close( fd );

       return;
     }
  
  int rv = write( fd, p, size );
  
  if ( rv != size )
       fprintf( stderr, "cannot write to %s: %s !\n", filename, 
                strerror( errno ) );

  close( fd );
}


int 
main( int argc, char *argv[] )
{
  int i;
  
  // float 32
  WriteFile( "Float32.bin", float32_data, sizeof( float32_data ) );
  tFloat32 f32[sizeof( float32_data ) / sizeof( tFloat32 )];
  ReadFile( "Float32.bin", f32, sizeof( float32_data ) );
  
  if ( memcmp( float32_data, f32, sizeof( float32_data ) ) )
     {
       fprintf( stderr, "read/write float 32 error !\n" );
       return 1;
     }

  // float 64
  WriteFile( "Float64.bin", float64_data, sizeof( float64_data ) );
  tFloat64 f64[sizeof( float64_data ) / sizeof( tFloat64 )];
  ReadFile( "Float64.bin", f64, sizeof( float64_data ) );

  if ( memcmp( float64_data, f64, sizeof( float64_data ) ) )
     {
       fprintf( stderr, "read/write float 64 error !\n" );
       return 1;
     }

  // conversion
  if ( G_BYTE_ORDER == G_LITTLE_ENDIAN )
       ReadFile( "Float32.bin.ppc", f32, sizeof( float32_data ) );
  else
       ReadFile( "Float32.bin.i386", f32, sizeof( float32_data ) );

  char *p = (char *)f32;

  for( i = 0; i < float32_data_num; i++, p += sizeof( tFloat32 ) )
     {
	/* compile error */
//       unsigned int v = *(unsigned int *)p;
       unsigned int v = *(unsigned int *)(void *)p;
       v = GUINT32_SWAP_LE_BE( v );
	/* compile error */
//       *(unsigned int *)p = v;
       *(unsigned int *)(void *)p = v;
     }

  if ( memcmp( float32_data, f32, sizeof( float32_data ) ) )
     {
       fprintf( stderr, "byteswap float 32 fail !\n" );
       return 1;
     }

  // conversion
  if ( G_BYTE_ORDER == G_LITTLE_ENDIAN )
       ReadFile( "Float64.bin.ppc", f64, sizeof( float64_data ) );
  else
       ReadFile( "Float64.bin.i386", f64, sizeof( float64_data ) );

  p = (char *)f64;

  for( i = 0; i < float64_data_num; i++, p += sizeof( tFloat64 ) )
     {
	/* compile error */
//       unsigned long long v = *(unsigned long long *)p;
       unsigned long long v = *(unsigned long long *)(void *)p;
       v = GUINT64_SWAP_LE_BE( v );
	/* compile error */
//       *(unsigned long long *)p = v;
       *(unsigned long long *)(void *)p = v;
     }

  if ( memcmp( float64_data, f64, sizeof( float64_data ) ) )
     {
       fprintf( stderr, "byteswap float 64 fail !\n" );
       return 1;
     }

  return 0;
}
