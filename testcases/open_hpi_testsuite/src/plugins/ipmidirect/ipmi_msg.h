/*
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

#ifndef dIpmiMsg_h
#define dIpmiMsg_h


#ifndef dIpmiCmd_h
#include "ipmi_cmd.h"
#endif


#define dIpmiMaxMsgLength 80


class cIpmiMsg
{
public:
  tIpmiNetfn     m_netfn;
  tIpmiCmd       m_cmd;
  unsigned short m_data_len;
  unsigned char  m_data[dIpmiMaxMsgLength];

public:
  cIpmiMsg();
  cIpmiMsg( tIpmiNetfn netfn, tIpmiCmd cmd,
            unsigned short data_len = 0, unsigned char *data = 0 );

  bool Equal( const cIpmiMsg &msg2 ) const;
};


#endif
