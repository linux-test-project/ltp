/*
 * ipmi_sensor_hotswap.h
 *
 * Copyright (c) 2004 by FORCE Computers
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

#ifndef dIpmiSensorHotswap_h
#define dIpmiSensorHotswap_h


#ifndef dIpmiSensorDiscrete_h
#include "ipmi_sensor_discrete.h"
#endif

#ifndef dIpmiUtils_h
#include "ipmi_utils.h"
#endif


class cIpmiSensorHotswap : public cIpmiSensorDiscrete
{
public:
  static SaHpiHsStateT ConvertIpmiToHpiHotswapState( tIpmiFruState h );

public:
  cIpmiSensorHotswap( cIpmiMc *mc );
  virtual ~cIpmiSensorHotswap();

  // handle hotswap sensor events
  virtual void HandleEvent( cIpmiEvent *event );

  // read sensor parameter from Full Sensor Record
  virtual bool GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr );

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  // read hotswap state
  SaErrorT GetPicmgState( tIpmiFruState &state );
  SaErrorT GetHpiState( SaHpiHsStateT &state );
};


#endif
