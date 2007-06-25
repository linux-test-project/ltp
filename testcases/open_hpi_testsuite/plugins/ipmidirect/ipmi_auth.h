/*
 * Copyright (c) 2003,2004 by FORCE Computers
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

#ifndef dIpmiAuth_h
#define dIpmiAuth_h


#include "config.h"


/* Data is provided to the authorization code as an array of these items, a
   "scatter-gather" list.  The algorithm will go through the item in the
   array until "data" is NULL. */
class cIpmiAuthSg
{
public:
  void *data; /* NULL to terminate. */
  int   len;
};


class cIpmiAuth
{
public:
  virtual ~cIpmiAuth() {};

  /* Initialize the authorization engine and return a handle for it.
     You must pass this handle into the other authorization
     calls.  Return 0 on success or an errno on failure. */
  virtual int Init( const unsigned char *password ) = 0;

  /* Generate a 16-byte authorization code and put it into
     "output". Returns 0 on success and an errno on failure.  */
  virtual int Gen( cIpmiAuthSg data[], void *output ) = 0;

  /* Check that the 16-byte authorization code given in "code" is valid.
     This will return 0 if it is valid or EINVAL if not. */
  virtual int Check( cIpmiAuthSg data[], void *code ) = 0;
};


// maximum number of charaters for username
#define dIpmiUsernameMax        16
#define dIpmiPasswordMax        16

// Standard IPMI authentication algorithms.
enum tIpmiAuthType
{
  eIpmiAuthTypeNone     = 0,
  eIpmiAuthTypeMd2      = 1,
  eIpmiAuthTypeMd5      = 2,
  eIpmiAuthTypeStraight = 4,
  eIpmiAuthTypeOem      = 5,
};

cIpmiAuth *IpmiAuthFactory( tIpmiAuthType type );

// This is a table of authentication algorithms.
#define dMaxIpmiAuths           6


// IPMI privilege levels
enum tIpmiPrivilege
{
  eIpmiPrivilegeNone     = 0,
  eIpmiPrivilegeCallback = 1,
  eIpmiPrivilegeUser     = 2,
  eIpmiPrivilegeOperator = 3,
  eIpmiPrivilegeAdmin    = 4,
  eIpmiPrivilegeOem      = 5
};


// Tell if a specific command is permitted for the given priviledge
// level.  Returns one of the following.
enum tIpmiPriv
{
  eIpmiPrivInvalid   = -1,
  eIpmiPrivDenied    =  0,
  eIpmiPrivPermitted =  1,
  eIpmiPrivSend      =  2, // Special send message handling needed.
  eIpmiPrivBoot      =  3, // Special set system boot options handling.
};


class cIpmiAuthNone : public cIpmiAuth
{
  unsigned char data[16];

public:
  virtual int Init( const unsigned char *password );
  virtual int Gen( cIpmiAuthSg   data[], void *output );
  virtual int Check( cIpmiAuthSg data[], void *code );
};


#ifdef HAVE_OPENSSL_MD2_H
class cIpmiAuthMd2 : public cIpmiAuth
{
  unsigned char data[16];

public:
  virtual int Init( const unsigned char *password );
  virtual int Gen( cIpmiAuthSg   data[], void *output );
  virtual int Check( cIpmiAuthSg data[], void *code );
};
#endif


#ifdef HAVE_OPENSSL_MD5_H
class cIpmiAuthMd5 : public cIpmiAuth
{
  unsigned char data[16];

public:
  virtual int Init( const unsigned char *password );
  virtual int Gen( cIpmiAuthSg   data[], void *output );
  virtual int Check( cIpmiAuthSg data[], void *code );
};
#endif


class cIpmiAuthStraight : public cIpmiAuth
{
  unsigned char data[16];

public:
  virtual int Init( const unsigned char *password );
  virtual int Gen( cIpmiAuthSg   data[], void *output );
  virtual int Check( cIpmiAuthSg data[], void *code );
};


#endif
