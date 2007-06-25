/*
 * ipmi_watchdog.h
 *
 * Copyright (c) 2006 by ESO Technologies.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */

#ifndef dIpmiWatchdog_h
#define dIpmiWatchdog_h


#ifndef dIpmiRdr_h
#include "ipmi_rdr.h"
#endif

extern "C" {
#include "SaHpi.h"
}

class cIpmiWatchdog : public cIpmiRdr
{
protected:
  unsigned int         m_num; // control num
  unsigned int         m_oem;


public:
  cIpmiWatchdog( cIpmiMc *mc,
                 unsigned int num,
                 unsigned int oem );
  ~cIpmiWatchdog();

  unsigned int Num() const { return m_num; }
  unsigned int Oem() const { return m_oem; }

  // create an RDR sensor record
  bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  SaErrorT GetWatchdogInfo( SaHpiWatchdogT &watchdog);
  SaErrorT SetWatchdogInfo( SaHpiWatchdogT &watchdog);
  SaErrorT ResetWatchdog();
};


#endif
