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

#ifndef dIpmiEvent_h
#define dIpmiEvent_h


// Event types and directions for sensors.
enum tIpmiThresh
{
  eIpmiLowerNonCritical = 0,
  eIpmiLowerCritical,
  eIpmiLowerNonRecoverable,
  eIpmiUpperNonCritical,
  eIpmiUpperCritical,
  eIpmiUpperNonRecoverable
};


//char *ipmi_get_threshold_string(unsigned int val);

enum tIpmiEventValueDir
{
  eIpmiGoingLow = 0,
  eIpmiGoingHigh
};


// char *ipmi_get_value_dir_string(unsigned int val);

enum tIpmiEventDir
{
  eIpmiAssertion = 0,
  eIpmiDeassertion
};


// The event state is which events are set and cleared for the given
// sensor.  Events are enumerated for threshold events and numbered
// for discrete events.  Use the provided functions to initialize,
// read, and modify an event state.


#define dIpmiSensorEventsEnabled    0x80
#define dIpmiSensorScanningEnabled  0x40
#define dIpmiSensorBusy             0x20

// The event state data structure.
class cIpmiEventState
{
public:
  unsigned int m_status; // dIpmiSensorEventsEnabled, dIpmiSensorScanningEnabled, dIpmiSensorBusy
  unsigned int m_assertion_events;
  unsigned int m_deassertion_events;

  cIpmiEventState();

  void SetEventsEnabled( bool val );
  bool GetEventsEnabled() const;
  void SetScanningEnabled( bool val );
  bool GetScanningEnabled() const;
  void SetBusy( bool val );
  bool GetBusy() const;
  void SetEnableEvents( int val );

  void ThresholdEventClear( tIpmiThresh type, tIpmiEventValueDir value_dir,
                            tIpmiEventDir dir );
  void ThresholdEventSet( tIpmiThresh type,
                          tIpmiEventValueDir value_dir,
                          tIpmiEventDir dir);
  bool IsThresholdEventSet( tIpmiThresh type,
                            tIpmiEventValueDir value_dir,
                            tIpmiEventDir dir ) const;
  void DiscreteEventClear( int event_offset,
                            tIpmiEventDir dir );
  void DiscreteEventSet( int event_offset,
                         tIpmiEventDir dir );
  bool IsDiscreteEventSet( int event_offset,
                           tIpmiEventDir dir ) const;
};


// Maximum amount of data allowed in a SEL.
#define dIpmiMaxSelData 13

class cIpmiMc;


// An entry from the system event log.
class cIpmiEvent
{
public:
  cIpmiMc      *m_mc; // The MC this event is stored in.

  unsigned int  m_record_id;
  unsigned int  m_type;
  unsigned char m_data[dIpmiMaxSelData];

public:
  cIpmiEvent();

  int Cmp( const cIpmiEvent &event ) const;
  void Log() const;
};


#endif
