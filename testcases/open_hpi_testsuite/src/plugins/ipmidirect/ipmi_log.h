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

#ifndef dIpmiLog_h
#define dIpmiLog_h


#define dDefaultLogfile "log"

/* log file handling */
#define dIpmiLogPropNone 0
#define dIpmiLogStdOut   1
#define dIpmiLogStdError 2
#define dIpmiLogFile     4 


int  IpmiLogOpen( int lp, const char *log_filename, int max_log_files );
void IpmiLogClose( void );

void IpmiLog( const char *fmt, ... );
void IpmiLogData( const char *fmt, ... );

void IpmiLogHex( const unsigned char *data, int size );


#endif
