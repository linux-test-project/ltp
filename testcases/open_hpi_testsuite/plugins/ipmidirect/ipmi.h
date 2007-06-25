/*
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
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
  cIpmiEntityPath   m_entity_root;

  bool GetParams( GHashTable *handler_config );

public:
  bool CheckMagic()
  {
    if ( m_magic == dIpmiMagic )
         return true;

    return false;
  }

  bool CheckHandler( oh_handler_state *handler )
  {
    if ( handler == m_handler )
        return true;

    return false;
  }

  cIpmiCon *AllocConnection( GHashTable *handler_config );

  virtual void IfEnter();
  virtual void IfLeave();

  // openhpi abi interface functions
  virtual bool IfOpen( GHashTable *handler_config );
  virtual void IfClose();
  virtual SaErrorT IfGetEvent( oh_event *event );
  virtual SaErrorT IfDiscoverResources();

  virtual SaErrorT IfSetResourceTag( cIpmiResource *ent, SaHpiTextBufferT *tag );
  virtual SaErrorT IfSetResourceSeverity( cIpmiResource *res, SaHpiSeverityT sev );

  // hot swap
  virtual SaErrorT IfGetHotswapState( cIpmiResource *res, SaHpiHsStateT &state );
  virtual SaErrorT IfSetHotswapState( cIpmiResource *res, SaHpiHsStateT state );
  virtual SaErrorT IfRequestHotswapAction( cIpmiResource *res, SaHpiHsActionT act );
  virtual SaErrorT IfHotswapPolicyCancel( cIpmiResource *res, SaHpiTimeoutT timeout );
  virtual SaErrorT IfSetAutoInsertTimeout( SaHpiTimeoutT  timeout);
  virtual SaErrorT IfGetAutoExtractTimeout( cIpmiResource *res, SaHpiTimeoutT  &timeout);
  virtual SaErrorT IfSetAutoExtractTimeout( cIpmiResource *res, SaHpiTimeoutT  timeout);

  virtual SaErrorT IfGetPowerState    ( cIpmiResource *res, SaHpiPowerStateT &state );
  virtual SaErrorT IfSetPowerState    ( cIpmiResource *res, SaHpiPowerStateT state );
  virtual SaErrorT IfGetIndicatorState( cIpmiResource *res, SaHpiHsIndicatorStateT &state );
  virtual SaErrorT IfSetIndicatorState( cIpmiResource *res, SaHpiHsIndicatorStateT state );
  virtual SaErrorT IfGetResetState    ( cIpmiResource *res, SaHpiResetActionT &state );
  virtual SaErrorT IfSetResetState    ( cIpmiResource *res, SaHpiResetActionT state );

  virtual SaErrorT IfControlParm( cIpmiResource *res, SaHpiParmActionT act );

  // lock for the hpi event queue
  cThreadLock m_event_lock;

  virtual void AddHpiEvent( oh_event *event );
  virtual oh_evt_queue *GetHpiEventList() { return m_handler->eventq; }

  cIpmi();
  ~cIpmi();

  void SetHandler( oh_handler_state *handler );
  oh_handler_state *GetHandler();

  virtual const cIpmiEntityPath &EntityRoot();
  virtual SaHpiRptEntryT *FindResource( SaHpiResourceIdT id );
};


#endif
