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

#ifndef dIpmi_h
#define dIpmi_h


#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifndef dIpmiDomain_h
#include "ipmi_domain.h"
#endif


#define dIpmiMagic 0x47110815


class cIpmi : public cIpmiDomain
{
  unsigned int      m_magic;
  oh_handler_state *m_handler;
  const char       *m_entity_root;

  bool GetParams( GHashTable *handler_config );

public:
  bool CheckMagic()
  {
    if ( m_magic == dIpmiMagic )
         return true;

    assert( 0 );
    return false;
  }

  bool CheckHandler( oh_handler_state *handler )
  {
    bool rv = ( handler == m_handler );
    assert( rv );

    return rv;
  }

  cIpmiCon *AllocConnection( GHashTable *handler_config );

  virtual void IfEnter();
  virtual void IfLeave();

  // openhpi abi interface functions
  virtual bool IfOpen( GHashTable *handler_config );
  virtual void IfClose();
  virtual SaErrorT IfGetEvent( oh_event *event, const timeval &timeout );
  virtual SaErrorT IfDiscoverResources();

  virtual SaErrorT IfSetResourceSeverity( cIpmiEntity *ent, SaHpiSeverityT sev );

  virtual SaErrorT IfGetSelInfo( cIpmiSel *sel, SaHpiSelInfoT &info );
  virtual SaErrorT IfGetSelTime( cIpmiSel *sel, SaHpiTimeT &time );
  virtual SaErrorT IfSetSelTime( cIpmiSel *sel, SaHpiTimeT time );
  virtual SaErrorT IfAddSelEntry( cIpmiSel *sel, const SaHpiSelEntryT &Event );
  virtual SaErrorT IfDelSelEntry( cIpmiSel *sel, SaHpiSelEntryIdT sid );
  virtual SaErrorT IfGetSelEntry( cIpmiSel *sel, SaHpiSelEntryIdT current,
                                  SaHpiSelEntryIdT &prev, SaHpiSelEntryIdT &next,
                                  SaHpiSelEntryT &entry );
  virtual SaErrorT IfClearSel( cIpmiSel *sel );

  // sensor
  virtual SaErrorT IfGetSensorData( cIpmiSensor *sensor, SaHpiSensorReadingT &data );
  virtual SaErrorT IfGetSensorThresholds( cIpmiSensor *sensor, SaHpiSensorThresholdsT &thres );
  virtual SaErrorT IfSetSensorThresholds( cIpmiSensor *sensor, const SaHpiSensorThresholdsT &thres );
  virtual SaErrorT IfGetSensorEventEnables( cIpmiSensor *sensor, SaHpiSensorEvtEnablesT &enables );
  virtual SaErrorT IfSetSensorEventEnables( cIpmiSensor *sensor, const SaHpiSensorEvtEnablesT &enables );

  // inventory
  virtual SaErrorT IfGetInventorySize( cIpmiFru *fru, SaHpiUint32T &size );
  virtual SaErrorT IfGetInventoryInfo( cIpmiFru *fru, SaHpiInventoryDataT &data );

  // hot swap
  virtual SaErrorT IfGetHotswapState( cIpmiEntity *ent, SaHpiHsStateT &state );
  virtual SaErrorT IfSetHotswapState( cIpmiEntity *ent, SaHpiHsStateT state );
  virtual SaErrorT IfRequestHotswapAction( cIpmiEntity *ent, SaHpiHsActionT act );

  virtual SaErrorT IfGetPowerState    ( cIpmiEntity *ent, SaHpiHsPowerStateT &state );
  virtual SaErrorT IfSetPowerState    ( cIpmiEntity *ent, SaHpiHsPowerStateT state );
  virtual SaErrorT IfGetIndicatorState( cIpmiEntity *ent, SaHpiHsIndicatorStateT &state );
  virtual SaErrorT IfSetIndicatorState( cIpmiEntity *ent, SaHpiHsIndicatorStateT state );
  virtual SaErrorT IfGetResetState    ( cIpmiEntity *ent, SaHpiResetActionT &state );
  virtual SaErrorT IfSetResetState    ( cIpmiEntity *ent, SaHpiResetActionT state );

  virtual SaErrorT IfControlParm( cIpmiEntity *ent, SaHpiParmActionT act );

  // called from IPMI framework
  virtual void IfHotswapEvent( cIpmiMc *mc, cIpmiSensor *sensor,
                               cIpmiEvent *event );

  void HotswapGenerateEvent( cIpmiSensor *sensor, cIpmiEntity *ent,
                             cIpmiEvent *event, SaHpiHsStateT state );

  virtual void IfEntityAdd( cIpmiEntity *ent );
  virtual void IfEntityRem( cIpmiEntity *ent );

  virtual void IfSensorAdd( cIpmiEntity *ent, cIpmiSensor *sensor );
  virtual void IfSensorRem( cIpmiEntity *ent, cIpmiSensor *sensor );

  virtual void IfSensorDiscreteEvent( cIpmiSensor  *sensor,
                                      tIpmiEventDir dir,
                                      int           offset,
                                      int           severity,
                                      int           prev_severity,
                                      cIpmiEvent   *event );
  virtual void IfSensorThresholdEvent( cIpmiSensor *sensor,
                               tIpmiEventDir      dir,
                               tIpmiThresh        threshold,
                               tIpmiEventValueDir high_low,
                               cIpmiEvent        *event );

  virtual void IfFruAdd( cIpmiEntity *ent, cIpmiFru *fru );

  virtual void IfSelAdd( cIpmiEntity *ent, cIpmiSel *sel );

  virtual void AddHpiEvent( oh_event *event );
  virtual GSList *GetHpiEventList() { return m_handler->eventq; }

  cIpmi();
  ~cIpmi();

  void SetHandler( oh_handler_state *handler );
  oh_handler_state *GetHandler();

  virtual const char *EntityRoot();
  virtual SaHpiRptEntryT *FindResource( SaHpiResourceIdT id );
};


#endif
