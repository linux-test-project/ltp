/*
 *
 * Copyright (c) 2004 by FORCE Computers
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

#ifndef dIpmiConSmi_h
#define dIpmiConSmi_h


#ifndef dIpmiCon_h
#include "ipmi_con.h"
#endif


class cIpmiConSmi : public cIpmiCon
{
  int m_if_num;

  int OpenSmiFd( int if_num );

public:
  cIpmiConSmi( unsigned int timeout, unsigned int atca_timeout,
               unsigned int max_outstanding, int if_num );
  virtual ~cIpmiConSmi();

protected:
  virtual int  IfOpen();
  virtual void IfClose();
  virtual int  IfSendCmd( cIpmiRequest *r );
  virtual void IfReadResponse();
};


#endif
