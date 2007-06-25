/*
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
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

#ifndef dIpmiLog_h
#define dIpmiLog_h


#ifndef dThread_h
#include "thread.h"
#endif

#include <stdio.h>
#include <assert.h>


#define dDefaultLogfile "log"

/* log file properties */
#define dIpmiLogPropNone 0
#define dIpmiLogStdOut   1 // use stdout
#define dIpmiLogStdErr   2 // use stderr
#define dIpmiLogLogFile  4 // use a log file
#define dIpmiLogFile     8 // use a file


class cIpmiLog
{
protected:
  cThreadLock m_lock;
  int         m_lock_count;
  int         m_open_count;

  bool  m_hex;  // true => print int in hex
  bool  m_time; // with time
  bool  m_recursive;
  bool  m_std_out;
  bool  m_std_err;

  bool  m_nl;

  FILE *m_fd;

  void Start();
  void Output( const char *str );

public:
  cIpmiLog();
  virtual ~cIpmiLog();

  bool Open( int properties, const char *filename = "", int max_log_files = 1 );
  void Close();

  void Lock()
  {
    m_lock.Lock();
    m_lock_count++;
  }

  void Unlock()
  {
    m_lock_count--;
    assert( m_lock_count >= 0 );
    m_lock.Unlock();
  }

  void Hex( bool hex = true ) { m_hex = hex; }
  bool IsHex()       { return m_hex;  }

  void Time( bool t = true ) { m_time = t; }
  bool WithTime()    { return m_time; }

  void Recursive( bool r ) { m_recursive = true; }
  bool IsRecursive() { return m_recursive; }

  cIpmiLog &operator<<( bool b );
  cIpmiLog &operator<<( unsigned char c );
  cIpmiLog &operator<<( int i );
  cIpmiLog &operator<<( unsigned int i );
  cIpmiLog &operator<<( double d );
  cIpmiLog &operator<<( const char *str );

  void Log( const char *fmt, ... );
  void Hex( const unsigned char *data, int size );

  void Begin( const char *section, const char *name );
  void End();
  cIpmiLog &Entry( const char *entry );
};


extern cIpmiLog stdlog;


#endif
