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

#ifndef dMd5_h
#define dMd5_h


#ifndef dIpmiAuth_h
#include "ipmi_auth.h"
#endif


class cIpmiAuthMd5 : public cIpmiAuth
{
  unsigned char data[16];

public:
  virtual int Init( const unsigned char *password );
  virtual int Gen( cIpmiAuthSg   data[], void *output );
  virtual int Check( cIpmiAuthSg data[], void *code );
};


#endif
