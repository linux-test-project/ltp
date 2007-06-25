/*
 * ipmi_sensor_discrete.h
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

#ifndef dIpmiSensorDiscrete_h
#define dIpmiSensorDiscrete_h


#ifndef dIpmiSensor_h
#include "ipmi_sensor.h"
#endif


class cIpmiSensorDiscrete : public cIpmiSensor
{
public:
  cIpmiSensorDiscrete( cIpmiMc *mc );
  virtual ~cIpmiSensorDiscrete();

  // create an hpi event from ipmi event
  virtual SaErrorT CreateEvent( cIpmiEvent *event, SaHpiEventT &h );

  // read sensor parameter from Full Sensor Record
  virtual bool GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr );

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  // get sensor data
  virtual SaErrorT GetSensorReading( SaHpiSensorReadingT &data, SaHpiEventStateT &state );

  virtual SaErrorT GetEventMasksHw( SaHpiEventStateT &AssertEventMask,
                                    SaHpiEventStateT &DeassertEventMask
                                  );
  virtual SaErrorT SetEventMasksHw( const SaHpiEventStateT &AssertEventMask,
                                    const SaHpiEventStateT &DeassertEventMask
                                  );
};


#endif
