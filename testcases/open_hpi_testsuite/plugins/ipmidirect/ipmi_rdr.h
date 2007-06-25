/*
 * ipmi_rdr.h base class for cIpmiSensor, cIpmiControl
 *
 * Copyright (c) 2004 by FORCE Computers.
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

#ifndef dIpmiRdr_h
#define dIpmiRdr_h


#ifndef dIpmiTextBuffer_h
#include "ipmi_text_buffer.h"
#endif

#include <glib.h>

extern "C" {
#include "SaHpi.h"
}


class cIpmiMc;
class cIpmiResource;


#ifndef dIpmiEntity_h
#include "ipmi_entity.h"
#endif

#ifndef dIpmiMsg_h
#include "ipmi_msg.h"
#endif

class cIpmiDomain;


class cIpmiRdr
{
protected:
  cIpmiMc        *m_mc;
  cIpmiResource  *m_resource;
  SaHpiEntryIdT   m_record_id;
  SaHpiRdrTypeT   m_type;
  cIpmiTextBuffer m_id_string;
  unsigned int    m_lun;
  cIpmiEntityPath m_entity_path;

public:
  cIpmiRdr( cIpmiMc *mc, SaHpiRdrTypeT type );
  virtual ~cIpmiRdr();

  cIpmiMc         *Mc() const { return m_mc; }
  cIpmiResource  *&Resource() { return m_resource; }
  SaHpiEntryIdT   &RecordId() { return m_record_id; }
  SaHpiRdrTypeT    Type() const { return m_type; }
  cIpmiTextBuffer &IdString() { return m_id_string; }
  const cIpmiTextBuffer &IdString() const { return m_id_string; }
  cIpmiEntityPath &EntityPath() { return m_entity_path; }
  cIpmiDomain *Domain();

  // create an RDR sensor record
  virtual bool CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr );

  // sensor num, control num, fru device id
  virtual unsigned int Num() const = 0;
  virtual unsigned int Lun() const { return m_lun; }

  //virtual void Dump( cIpmiLog &dump ) = 0;
  SaErrorT SendCommand( const cIpmiMsg &msg, cIpmiMsg &rsp,
                        unsigned int lun = 0, int retries = 3 );

  // populate rdrs
private:
  bool m_populate;

public:
  virtual bool Populate(GSList **);
};


#endif
