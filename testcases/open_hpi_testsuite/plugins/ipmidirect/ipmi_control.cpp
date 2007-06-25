/*
 * ipmi_control.cpp
 *
 * Copyright (c) 2004 by FORCE Computers.
 * Copyright (c) 2005 by ESO Technologies.
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

#include "ipmi_control.h"
#include "ipmi_domain.h"


cIpmiControl::cIpmiControl( cIpmiMc *mc,
			    unsigned int num,
                            SaHpiCtrlOutputTypeT output_type,
                            SaHpiCtrlTypeT type )
  : cIpmiRdr( mc, SAHPI_CTRL_RDR ),
    m_num( num ),
    m_output_type( output_type ),
    m_type( type )
{
}


cIpmiControl::~cIpmiControl()
{
}

bool
cIpmiControl::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  if ( cIpmiRdr::CreateRdr( resource, rdr ) == false )
       return false;

  // update resource
  resource.ResourceCapabilities |= SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_CONTROL;

  // control record
  SaHpiCtrlRecT &rec = rdr.RdrTypeUnion.CtrlRec;

  rec.Num        = (SaHpiCtrlNumT)m_num;
  rec.OutputType = m_output_type;
  rec.Type       = m_type;
  rec.Oem        = m_oem;

  return true;
}
