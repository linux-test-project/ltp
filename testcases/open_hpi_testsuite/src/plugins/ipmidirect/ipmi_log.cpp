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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

#include "ipmi_log.h"
#include "ipmi_utils.h"


static int  log_open = 0;
static int  log_prop = dIpmiLogPropNone;
static char log_file[1024];
static int  log_fd = -1;


int
IpmiLogOpen( int lp, const char *filename, int max_log_files )
{
  time_t t;
  
  if ( log_open )
       return 0;

  log_prop = dIpmiLogPropNone;

  if ( lp & dIpmiLogStdOut )
       log_prop |= dIpmiLogStdOut;

  if ( lp & dIpmiLogStdError )
       log_prop |= dIpmiLogStdError;

  if ( lp & dIpmiLogFile )
     {
       char tf[1024];
       char file[1024] = "";
       int i;
       struct stat st1, st2;

       if ( filename == 0 || *filename == 0 )
            return EINVAL;

        // max numbers of logfiles
       if ( max_log_files < 1 )
            max_log_files = 1;

       // find a new one or the oldes logfile
       for( i = 0; i < max_log_files; i++ )
          {
            sprintf( tf, "%s%02d.log", filename, i );

            if ( file[0] == 0 )
                 strcpy( file, tf );
                
            if (    !stat( tf, &st1 )
                 && S_ISREG( st1. st_mode ) )
               {
                 if (    !stat( file, &st2 )
                      && S_ISREG( st1. st_mode )
                      && st2.st_mtime > st1.st_mtime )
                      strcpy( file, tf );
               }
            else
               {
                 strcpy( file, tf );
                 break;
               }
          }

       strcpy( log_file, file );

       log_fd = open( log_file, O_WRONLY|O_CREAT|O_TRUNC,
                      S_IRUSR|S_IWUSR|S_IRWXG|S_IWGRP|S_IRWXO );

       if ( log_fd >= 0 )
            log_prop |= dIpmiLogFile;
       else
            fprintf( stderr, "can not open logfile %s\n", log_file );
     }

  log_open = 1;

  t = time( 0 );

  if ( log_prop & dIpmiLogFile )
       IpmiLog( "Logfile %s, %s", log_file,
                ctime( &t ) );

  return 0;
}


void
IpmiLogClose()
{
  if ( !log_open )
       return;

  if ( log_fd >= 0 )
     {
       IpmiLog( "closing logfile.\n" );
       log_fd = -1;
     }

  log_file[0] = 0;
  log_prop = dIpmiLogPropNone;
  log_open = 0;
}


void
IpmiLog( const char *fmt, ... )
{
  char buf[10240] = "# ";
  va_list ap;

  if ( !log_open )
       return;

  va_start( ap, fmt );
  vsnprintf( buf+2, 10237, fmt, ap );
  va_end( ap );

  if ( log_fd >= 0 )
       write( log_fd, buf, strlen( buf ) );

  if ( log_prop & dIpmiLogStdError )
     {
       fprintf( stderr, "%s", buf + 2 );
       fflush( stderr );
     }

  if ( log_prop & dIpmiLogStdOut )
     {
       fprintf( stdout, "%s", buf + 2 );
       fflush( stdout );
     }
}


void
IpmiLogData( const char *fmt, ... )
{
  static int new_line = 1;
  char buf[10240];
  va_list ap;

  if ( !log_open )
       return;

  va_start( ap, fmt );
  vsnprintf( buf, 10237, fmt, ap );
  va_end( ap );

  if ( log_fd >= 0 )
     {
       if ( new_line )
          {
            char b[10240];
            struct timeval tv;

            gettimeofday( &tv, 0 );
            IpmiTimeToString( tv.tv_sec, b );
            sprintf( b + dTimeStringSize - 1, ".%03ld ", tv.tv_usec / 1000 );
            strcat( b, buf );
            write( log_fd, b, strlen( b ) );
          }
       else
            write( log_fd, buf, strlen( buf ) );

       new_line = strchr( buf, '\n' ) ? 1 : 0;
     }

  if ( log_prop & dIpmiLogStdError )
     {
       fprintf( stderr, "%s", buf );
       fflush( stderr );
     }

  if ( log_prop & dIpmiLogStdOut )
     {
       fprintf( stdout, "%s", buf );
       fflush( stdout );
     }
}


void
IpmiLogHex( const unsigned char *data, int size )
{
  char str[256];
  char *s = str;
  int i;

  for( i = 0; i < size; i++ )
     {
       if ( i != 0 && (i % 16) == 0 )
          {
            IpmiLog( "%s\n", str );
            s = str;
          }

       s += sprintf( s, " %02x", *data++ );
     }

  if ( s != str )
       IpmiLog( "%s\n", str );
}
