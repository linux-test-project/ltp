/*
 * ipmi_utils.cpp
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

#ifndef dIpmiUtils_h
#define dIpmiUtils_h

#include <sys/time.h>


enum tIpmiFruState
{
  eIpmiFruStateNotInstalled           = 0,
  eIpmiFruStateInactive               = 1,
  eIpmiFruStateActivationRequest      = 2,
  eIpmiFruStateActivationInProgress   = 3,
  eIpmiFruStateActive                 = 4,
  eIpmiFruStateDeactivationRequest    = 5,
  eIpmiFruStateDeactivationInProgress = 6,
  eIpmiFruStateCommunicationLost      = 7
};

const char *IpmiFruStateToString( tIpmiFruState state );


unsigned int IpmiGetUint16( const unsigned char *data );
void         IpmiSetUint16( unsigned char *data, int val );

unsigned int IpmiGetUint32( const unsigned char *data );
void         IpmiSetUint32( unsigned char *data, int val );


#define dDateStringSize 11
void IpmiDateToString( unsigned int time, char *str );

#define dTimeStringSize 9
void IpmiTimeToString( unsigned int time, char *str );

#define dDateTimeStringSize 20
void IpmiDateTimeToString( unsigned int time, char *str );


class cTime
{
public:
  timeval m_time;

  cTime()
  {
    m_time.tv_sec  = 0;
    m_time.tv_usec = 0;    
  }
  cTime( const struct timeval &tv )
  {
    m_time.tv_sec  = tv.tv_sec;
    m_time.tv_usec = tv.tv_usec;
  }
  cTime( const cTime &t )
  {
    m_time.tv_sec  = t.m_time.tv_sec;
    m_time.tv_usec = t.m_time.tv_usec;
  }
  cTime( unsigned int s, unsigned int u )
  {
    m_time.tv_sec = s;
    m_time.tv_usec = u;
  }

  void Normalize()
  {
    while( m_time.tv_usec > 1000000 )
       {
         m_time.tv_usec -= 1000000;
         m_time.tv_sec++;
       }

    while( m_time.tv_usec < 0 )
       {
         m_time.tv_usec += 1000000;
         m_time.tv_sec--;
       }
  }

  int Cmp( const cTime &t )
  {
    if ( m_time.tv_sec < t.m_time.tv_sec )
         return -1;

    if ( m_time.tv_sec > t.m_time.tv_sec )
         return 1;

    if ( m_time.tv_usec < t.m_time.tv_usec )
         return -1;

    if ( m_time.tv_usec > t.m_time.tv_usec )
         return 1;

    return 0;
  }

  bool operator<( const cTime &t )
  {
    return Cmp( t ) < 0;
  }

  bool operator<=( const cTime &t )
  {
    return Cmp( t ) < 0;
  }

  bool operator>( const cTime &t )
  {
    return Cmp( t ) > 0;
  }

  bool operator>=( const cTime &t )
  {
    return Cmp( t ) >= 0;
  }

  bool operator==( const cTime &t )
  {
    return Cmp( t ) == 0;
  }

  bool operator!=( const cTime &t )
  {
    return Cmp( t ) == 0;
  }

  cTime &operator+=( const cTime &t )
  {
    m_time.tv_sec += t.m_time.tv_sec;
    m_time.tv_usec += t.m_time.tv_usec;

    Normalize();

    return *this;
  }

  cTime &operator+=( int ms )
  {
    m_time.tv_sec  += ms / 1000;
    m_time.tv_usec += (ms % 1000) * 1000;

    Normalize();

    return *this;
  }

  cTime &operator-=( int ms )
  {
    m_time.tv_sec  -= ms / 1000;
    m_time.tv_usec -= (ms % 1000) * 1000;

    Normalize();

    return *this;
  }

  static cTime Now()
  {
    cTime t;
    gettimeofday( &t.m_time, 0 );

    return t;
  }
};


#endif
