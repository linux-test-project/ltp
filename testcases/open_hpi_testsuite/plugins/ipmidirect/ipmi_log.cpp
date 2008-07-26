/*
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
 * Copyright (c) 2007 by ESO Technologies.
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
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>

#include "ipmi_log.h"
#include "ipmi_utils.h"


cIpmiLog stdlog;


cIpmiLog::cIpmiLog()
  : m_lock_count( 0 ),
    m_open_count( 0 ),
    m_hex( false ),
    m_time( false ),
    m_recursive( false ),
    m_std_out( false ),
    m_std_err( false )
{
}


cIpmiLog::~cIpmiLog()
{
}


bool
cIpmiLog::Open( int properties, const char *filename, int max_log_files )
{
  m_open_count++;

  if ( m_open_count > 1 )
       // already open
       return true;

  assert( m_lock_count == 0 );

  if ( properties & dIpmiLogStdOut )
       m_std_out = true;

  if ( properties & dIpmiLogStdErr )
       m_std_err = true;

  char file[1024] = "";

  if ( properties & dIpmiLogLogFile )
     {
       char tf[1024];
       int i;
       struct stat st1, st2;

       if ( filename == 0 || *filename == 0 )
	  {
	    fprintf( stderr, "not filename for logfile !\n" );
            return false;
	  }

        // max numbers of logfiles
       if ( max_log_files < 1 )
            max_log_files = 1;

       // find a new one or the oldes logfile
       for( i = 0; i < max_log_files; i++ )
          {
            snprintf( tf, sizeof(tf), "%s%02d.log", filename, i );

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
     }

  if ( properties & dIpmiLogFile )
     {
       if ( filename == 0 || *filename == 0 )
	  {
	    fprintf( stderr, "not filename for logfile !\n" );
            return false;
	  }
  
       strcpy( file, filename );
     }
  
  if ( file[0] )
     {
       m_fd = fopen( file, "w" );

       if ( m_fd == 0 )
	  {
            fprintf( stderr, "can not open logfile %s\n", file );
	    return false;
	  }
     }

  m_nl = true;

  return true;
}


void
cIpmiLog::Close()
{
  m_open_count--;
  assert( m_open_count >= 0 );

  if ( m_open_count > 0 )
       return;

  assert( m_lock_count == 0 );
  assert( m_nl );

  if ( m_fd )
     {
       fclose( m_fd );
       m_fd = 0;
     }

  m_std_out = false;
  m_std_err = false;
}


void
cIpmiLog::Output( const char *str )
{
  int l = strlen( str );
  size_t fwrote;

  if ( m_fd )
       fwrote = fwrite( str, l, 1, m_fd );

  if ( m_std_out )
       fwrote = fwrite( str, l, 1, stdout );

  if ( m_std_err )
       fwrote = fwrite( str, l, 1, stderr );
}


void
cIpmiLog::Start()
{
  //Lock();

  if ( m_nl )
     {
       if ( m_time )
	  {
            struct timeval tv;
            gettimeofday( &tv, 0 );

	    char b[dDateTimeStringSize+5];
            IpmiDateTimeToString( tv.tv_sec, b );
#if defined(__sparc) || defined(__sparc__)
	    snprintf( b + dDateTimeStringSize - 1, 6, ".%03ld ", (long)tv.tv_usec / 1000 );
#else
	    snprintf( b + dDateTimeStringSize - 1, 6, ".%03ld ", tv.tv_usec / 1000 );
#endif

	    Output( b );
	  }
     }
}


cIpmiLog &
cIpmiLog::operator<<( bool b )
{
  Start();

  Output( b ? "true" : "false" );

  return *this;
}


cIpmiLog &
cIpmiLog::operator<<( unsigned char c )
{
  Start();

  char b[5];
  snprintf( b, sizeof(b), "0x%02x", c );

  Output( b );

  return *this;
}


cIpmiLog &
cIpmiLog::operator<<( int i )
{
  Start();

  char b[20];
  snprintf( b, sizeof(b), "%d", i );
  Output( b );

  return *this;
}


cIpmiLog &
cIpmiLog::operator<<( unsigned int i )
{
  Start();

  char b[20];

  if ( m_hex )
       snprintf( b, sizeof(b), "0x%08x", i );
  else
       snprintf( b, sizeof(b), "%u", i );

  Output( b );

  return *this;
}


cIpmiLog &
cIpmiLog::operator<<( double d )
{
  Start();
  
  char b[20];
  snprintf( b, sizeof(b), "%f", d );
  Output( b );

  return *this;  
}


cIpmiLog &
cIpmiLog::operator<<( const char *str )
{
  Log( "%s", str );

  return *this;
}


void
cIpmiLog::Log( const char *fmt, ... )
{
  Start();

  va_list ap;
  va_start( ap, fmt );

  char b[10240];
  vsnprintf( b, 10240, fmt, ap );  

  va_end( ap );

  char buf[10230] = "";

  char *p = b;
  char *q = buf;

  m_nl = false;

  while( *p )
     {
       if ( *p == '\n' )
	  {
	    m_nl = true;

	    *q++ = *p++;

	    *q = 0;

	    Output( buf );
	    q = buf;

	    continue;
	  }

       m_nl = false;
       *q++ = *p++;
     }

  if ( q != b )
     {
       *q = 0;
       Output( buf );
     }

  if ( m_nl )
     {
       if ( m_fd )
            fflush( m_fd );

       if ( m_std_out )
            fflush( stdout );

       if ( m_std_err )
            fflush( stderr );
 
       //while( m_lock_count > 0 )
       //    Unlock();
     }
}


void
cIpmiLog::Hex( const unsigned char *data, int size )
{
  char str[256];
  char *s = str;
  int i, remaining;

  for( i = 0; i < size; i++ )
     {
       if ( i != 0 && (i % 16) == 0 )
          {
            Log( "%s\n", str );
            s = str;
          }
       remaining = sizeof(str) - (s - str);
       if (remaining > 0)
          s += snprintf( s, remaining, " %02x", *data++ );
     }

  if ( s != str )
       Log( "%s\n", str );
}


void
cIpmiLog::Begin( const char *section, const char *name )
{
  if ( IsRecursive() )
       *this << section << " \"" << name << "\"\n{\n";
}


void
cIpmiLog::End()
{
  if ( IsRecursive() )
       *this << "}\n\n\n";
}


cIpmiLog &
cIpmiLog::Entry( const char *entry )
{
  char str[256];
  strcpy( str, entry );

  int l = 30 - strlen( entry );

  if ( l > 0 )
     {
       char *p = str + strlen( entry );
  
       while( l-- > 0 )
	    *p++ = ' ';

       *p = 0;
     }

  *this << "        " << str << " = ";

  return *this;
}
