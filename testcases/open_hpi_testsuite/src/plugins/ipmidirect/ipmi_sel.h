/*
 * ipmi_sel.h
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


#ifndef dIpmiSel_h
#define dIpmiSel_h


#ifndef dIpmiEvent_h
#include "ipmi_event.h"
#endif

#ifndef dThread_h
#include "thread.h"
#endif

#include "glib.h"


class cIpmiEntity;
class cIpmiDomain;
class cIpmiSdrs;


#define dMaxSelFetchRetries 3


class cIpmiSel
{
protected:
  cIpmiMc     *m_mc;

  // LUN we are attached with.
  int           m_lun;

  unsigned char m_major_version;
  unsigned char m_minor_version;
  unsigned char m_entries;
  unsigned int  m_last_addition_timestamp;
  unsigned int  m_last_erase_timestamp;
  bool          m_overflow;
  bool          m_supports_delete_sel;
  bool          m_supports_partial_add_sel;
  bool          m_supports_reserve_sel;
  bool          m_supports_get_sel_allocation;

  // entity of SEL
  cIpmiEntity  *m_entity;

  bool          m_fetched;

  // When fetching the data in event-driven mode, these are the
  // variables that track what is going on.
  unsigned int  m_reservation;
  bool          m_sels_changed;

  // SEL
  cThreadLock   m_sel_lock;
  GList        *m_sel;
  unsigned int  m_sel_num;

  // async events
  cThreadLock   m_async_events_lock;
  GList        *m_async_events;
  unsigned int  m_async_events_num;

  int GetInfo();
  int Reserve();
  int ReadSelRecord( cIpmiEvent &event, unsigned int &next_rec_id );
  GList *ReadSel( unsigned int &num, bool &uptodate );
  cIpmiEvent *FindEvent( GList *list, unsigned int record_id );
  bool CheckEvent( GList *&list, cIpmiEvent *event );

public:
  cIpmiSel( cIpmiMc *mc, unsigned int lun );
  ~cIpmiSel();

  // clear a list of cIpmiEvent
  GList *ClearList( GList *list );

  // read SEL and return a list of new events
  GList *GetEvents();

  // add an event to the list of async events
  int AddAsyncEvent( cIpmiEvent *event );

  // get an event
  int GetSelEntry( unsigned short rid, unsigned short &prev, 
                   unsigned short &next, cIpmiEvent &event );

  // delete SEL entry
  int DeleteSelEntry( unsigned short rid );

  // clear the SEL
  int ClearSel();

  // set SEL time
  int SetSelTime( time_t t );

  // get SEL time
  int GetSelTime( time_t &t );

  cIpmiEntity *&Entity() { return m_entity; }
  cIpmiMc      *Mc() { return m_mc; }
  int           Lun() { return m_lun; }

  int           SelNum() { return m_sel_num; }
  unsigned int  AdditionTimestamp() { return m_last_addition_timestamp; }
  unsigned int  EraseTimestamp() { return m_last_erase_timestamp; }
  bool          Overflow() { return m_overflow; }
  bool          SupportsDeleteSel() { return m_supports_delete_sel; }

  void Lock() { m_sel_lock.Lock(); }
  void Unlock() { m_sel_lock.Unlock(); }
};


void IpmiSelHandleSdr( cIpmiDomain *domain, cIpmiMc *mc, cIpmiSdrs *sdrs );


#endif
