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
 *     Andy Cress        <arcress@users.sourceforge.net>
 */

#include <netdb.h>
#include <errno.h>

#include "ipmi.h"
#include "ipmi_con_lan.h"
#include "ipmi_con_smi.h"
#include "ipmi_utils.h"


static cIpmi *
VerifyIpmi( void *hnd )
{
  if (!hnd)
    return 0;

  oh_handler_state *handler = (oh_handler_state *)hnd;
  cIpmi *ipmi = (cIpmi *)handler->data;

  if ( !ipmi )
     {
       return 0;
     }

  if ( !ipmi->CheckMagic() )
     {
       return 0;
     }

  if ( !ipmi->CheckHandler( handler ) )
     {
       return 0;
     }

  return ipmi;
}


static cIpmiSensor *
VerifySensorAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiSensorNumT num,
                      cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_SENSOR_RDR, num );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiSensor *sensor = (cIpmiSensor *)oh_get_rdr_data( ipmi->GetHandler()->rptcache,
                                                        rid, rdr->RecordId );
  if ( !sensor )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifySensor( sensor ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return sensor;
}


static cIpmiControl *
VerifyControlAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiCtrlNumT num,
                      cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_CTRL_RDR, num );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiControl *control = (cIpmiControl *)oh_get_rdr_data( ipmi->GetHandler()->rptcache,
                                                           rid, rdr->RecordId );
  if ( !control )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifyControl( control ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return control;
}

static cIpmiWatchdog *
VerifyWatchdogAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiWatchdogNumT num,
                        cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_WATCHDOG_RDR, num );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiWatchdog *watchdog = (cIpmiWatchdog *)oh_get_rdr_data( ipmi->GetHandler()->rptcache,
                                                              rid, rdr->RecordId );
  if ( !watchdog )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifyWatchdog( watchdog ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return watchdog;
}

static cIpmiInventory *
VerifyInventoryAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiIdrIdT idrid,
                         cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_INVENTORY_RDR, idrid );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiInventory *inv = (cIpmiInventory *)oh_get_rdr_data( ipmi->GetHandler()->rptcache,
                                                           rid, rdr->RecordId );
  if ( !inv )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifyInventory( inv ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return inv;
}


static cIpmiResource *
VerifyResourceAndEnter( void *hnd, SaHpiResourceIdT rid, cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  cIpmiResource *res = (cIpmiResource *)oh_get_resource_data( ipmi->GetHandler()->rptcache, rid );

  if ( !res )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifyResource( res ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return res;
}


static cIpmiSel *
VerifySelAndEnter( void *hnd, SaHpiResourceIdT rid, cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return 0;
     }

  ipmi->IfEnter();

  cIpmiResource *res = (cIpmiResource *)oh_get_resource_data( ipmi->GetHandler()->rptcache, rid );

  if ( !res )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( !ipmi->VerifyResource( res ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  if ( res->FruId() || !res->Mc()->SelDeviceSupport() )
     {
       ipmi->IfLeave();
       return 0;
     }

  return res->Mc()->Sel();
}


// new plugin_loader
extern "C" {

// ABI Interface functions

static void *
IpmiOpen( GHashTable *, unsigned int, oh_evt_queue * ) __attribute__((used));

static void *
IpmiOpen( GHashTable *handler_config, unsigned int hid, oh_evt_queue *eventq )
{
  // open log
  const char *logfile = 0;
  int   max_logfiles = 10;
  char *tmp;
  int   lp = dIpmiLogPropNone;

  dbg( "IpmiOpen" );

  if ( !handler_config )
     {
       err( "No config file provided.....ooops!" );
       return 0;
     }

  logfile = (char *)g_hash_table_lookup( handler_config, "logfile" );
  tmp = (char *)g_hash_table_lookup( handler_config, "logfile_max" );

  if ( tmp )
       max_logfiles = atoi( tmp );

  tmp = (char *)g_hash_table_lookup( handler_config, "logflags" );

  if ( tmp )
     {
       if (    strstr( tmp, "StdOut" )
            || strstr( tmp, "stdout" ) )
            lp |= dIpmiLogStdOut;

       if (    strstr( tmp, "StdError" )
            || strstr( tmp, "stderr" ) )
            lp |= dIpmiLogStdErr;

       if (    strstr( tmp, "File" )
            || strstr( tmp, "file" ) )
          {
            lp |= dIpmiLogLogFile;

            if ( logfile == 0 )
                 logfile = dDefaultLogfile;
          }
     }

  stdlog.Open( lp, logfile, max_logfiles );
  stdlog.Time( true );

  // create domain
  cIpmi *ipmi = new cIpmi;

  // allocate handler
  oh_handler_state *handler = (oh_handler_state *)g_malloc0(
                                  sizeof( oh_handler_state ) );

  if ( !handler )
     {
       err("cannot allocate handler");

       delete ipmi;

       stdlog.Close();

       return 0;
     }

  handler->data     = ipmi;
  handler->rptcache = (RPTable *)g_malloc0( sizeof( RPTable ) );
  if ( !handler->rptcache )
     {
       err("cannot allocate RPT cache");

       g_free( handler );

       delete ipmi;

       stdlog.Close();

       return 0;
     }

  handler->config   = handler_config;
  handler->hid = hid;
  handler->eventq = eventq;

  ipmi->SetHandler( handler );

  if ( !ipmi->IfOpen( handler_config ) )
     {
       ipmi->IfClose();

       delete ipmi;

       oh_flush_rpt( handler->rptcache );
       g_free( handler->rptcache );
       g_free( handler );

       stdlog.Close();

       return 0;
     }

  return handler;
}


static void
IpmiClose( void * ) __attribute__((used));

static void
IpmiClose( void *hnd )
{
  dbg( "IpmiClose" );

  cIpmi *ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return;
     }
/** Commenting this code block due to the multi-domain changes
 ** in the infrastructure.
 ** (Renier Morales 11/21/06)
  if ( ipmi->DomainId() != oh_get_default_domain_id() )
  {
      stdlog << "Releasing domain id " << ipmi->DomainId() << "\n";

      SaErrorT rv = oh_request_domain_delete( ipmi->HandlerId(), ipmi->DomainId() );

      if ( rv != SA_OK )
          stdlog << "oh_request_domain_delete error " << rv << "\n";
  }*/

  ipmi->IfClose();

  ipmi->CheckLock();

  delete ipmi;

  oh_handler_state *handler = (oh_handler_state *)hnd;

  if ( handler->rptcache )
  {
      oh_flush_rpt( handler->rptcache );
      g_free( handler->rptcache );
  }

  g_free( handler );

  // close logfile
  stdlog.Close();
}


static SaErrorT
IpmiGetEvent( void * ) __attribute__((used));

static SaErrorT
IpmiGetEvent( void *hnd )
{
  cIpmi *ipmi = VerifyIpmi( hnd );
  struct oh_event event;

  if ( !ipmi )
     {
       return SA_ERR_HPI_INTERNAL_ERROR;
     }

  // there is no need to get a lock because
  // the event queue has its own lock
  SaErrorT rv = ipmi->IfGetEvent( &event );

  return rv;
}


static SaErrorT
IpmiDiscoverResources( void * ) __attribute__((used));

static SaErrorT
IpmiDiscoverResources( void *hnd )
{
  cIpmi *ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return SA_ERR_HPI_INTERNAL_ERROR;
     }

  stdlog << "Simple discovery let's go " << hnd << "\n";

  SaErrorT rv = ipmi->IfDiscoverResources();

  return rv;
}


static SaErrorT
IpmiSetResourceTag( void *, SaHpiResourceIdT, SaHpiTextBufferT * ) __attribute__((used));

static SaErrorT
IpmiSetResourceTag( void *hnd, SaHpiResourceIdT id, SaHpiTextBufferT *tag )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetResourceTag( res, tag );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetResourceSeverity( void *, SaHpiResourceIdT, SaHpiSeverityT ) __attribute__((used));

static SaErrorT
IpmiSetResourceSeverity( void *hnd, SaHpiResourceIdT id, SaHpiSeverityT sev )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetResourceSeverity( res, sev );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorReading( void *,
                   SaHpiResourceIdT id,
                   SaHpiSensorNumT num,
                   SaHpiSensorReadingT *data,
                   SaHpiEventStateT *state ) __attribute__((used));

static SaErrorT
IpmiGetSensorReading( void *hnd,
                   SaHpiResourceIdT id,
                   SaHpiSensorNumT num,
                   SaHpiSensorReadingT *data,
                   SaHpiEventStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->GetSensorReading( *data, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorThresholds( void *hnd,
                         SaHpiResourceIdT,
                         SaHpiSensorNumT,
                         SaHpiSensorThresholdsT * ) __attribute__((used));

static SaErrorT
IpmiGetSensorThresholds( void                   *hnd,
                         SaHpiResourceIdT        id,
                         SaHpiSensorNumT         num,
                         SaHpiSensorThresholdsT *thres )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = SA_ERR_HPI_INVALID_PARAMS;

  cIpmiSensorThreshold *t = dynamic_cast<cIpmiSensorThreshold *>( sensor );

  if ( t )
       rv = t->GetThresholdsAndHysteresis( *thres );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorThresholds( void *,
                         SaHpiResourceIdT,
                         SaHpiSensorNumT,
                         const SaHpiSensorThresholdsT * ) __attribute__((used));

static SaErrorT
IpmiSetSensorThresholds( void *hnd,
                         SaHpiResourceIdT id,
                         SaHpiSensorNumT  num,
                         const SaHpiSensorThresholdsT *thres )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = SA_ERR_HPI_INVALID_PARAMS;

  cIpmiSensorThreshold *t = dynamic_cast<cIpmiSensorThreshold *>( sensor );

  if ( t )
       rv = t->SetThresholdsAndHysteresis( *thres );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorEnable( void *,
                     SaHpiResourceIdT,
                     SaHpiSensorNumT,
                     SaHpiBoolT * ) __attribute__((used));

static SaErrorT
IpmiGetSensorEnable( void *hnd,
                     SaHpiResourceIdT id,
                     SaHpiSensorNumT  num,
                     SaHpiBoolT       *enable )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->GetEnable( *enable );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorEnable( void *,
                     SaHpiResourceIdT,
                     SaHpiSensorNumT,
                     SaHpiBoolT ) __attribute__((used));

static SaErrorT
IpmiSetSensorEnable( void *hnd,
                     SaHpiResourceIdT id,
                     SaHpiSensorNumT  num,
                     SaHpiBoolT       enable )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->SetEnable( enable );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorEventEnables( void *,
                           SaHpiResourceIdT,
                           SaHpiSensorNumT,
                           SaHpiBoolT * ) __attribute__((used));

static SaErrorT
IpmiGetSensorEventEnables( void *hnd,
                           SaHpiResourceIdT id,
                           SaHpiSensorNumT  num,
                           SaHpiBoolT       *enables )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->GetEventEnables( *enables );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorEventEnables( void *,
                           SaHpiResourceIdT,
                           SaHpiSensorNumT,
                           SaHpiBoolT ) __attribute__((used));

static SaErrorT
IpmiSetSensorEventEnables( void *hnd,
                           SaHpiResourceIdT id,
                           SaHpiSensorNumT  num,
                           SaHpiBoolT       enables )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->SetEventEnables( enables );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorEventMasks( void *,
                           SaHpiResourceIdT,
                           SaHpiSensorNumT,
                           SaHpiEventStateT *,
                           SaHpiEventStateT * ) __attribute__((used));

static SaErrorT
IpmiGetSensorEventMasks( void *hnd,
                           SaHpiResourceIdT id,
                           SaHpiSensorNumT  num,
                           SaHpiEventStateT *AssertEventMask,
                           SaHpiEventStateT *DeassertEventMask
                       )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->GetEventMasks( *AssertEventMask, *DeassertEventMask );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorEventMasks( void *,
                           SaHpiResourceIdT,
                           SaHpiSensorNumT,
                           SaHpiSensorEventMaskActionT,
                           SaHpiEventStateT,
                           SaHpiEventStateT ) __attribute__((used));

static SaErrorT
IpmiSetSensorEventMasks( void *hnd,
                           SaHpiResourceIdT id,
                           SaHpiSensorNumT  num,
                           SaHpiSensorEventMaskActionT act,
                           SaHpiEventStateT            AssertEventMask,
                           SaHpiEventStateT            DeassertEventMask
                       )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sensor->SetEventMasks( act, AssertEventMask, DeassertEventMask );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetControlState( void *, SaHpiResourceIdT,
                     SaHpiCtrlNumT,
                     SaHpiCtrlModeT *,
                     SaHpiCtrlStateT * ) __attribute__((used));

static SaErrorT
IpmiGetControlState( void *hnd, SaHpiResourceIdT id,
                     SaHpiCtrlNumT num,
                     SaHpiCtrlModeT *mode,
                     SaHpiCtrlStateT *state )
{
  cIpmi *ipmi;
  cIpmiControl *control = VerifyControlAndEnter( hnd, id, num, ipmi );

  if ( !control )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = control->GetState( *mode, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetControlState( void *, SaHpiResourceIdT,
                     SaHpiCtrlNumT,
                     SaHpiCtrlModeT,
                     SaHpiCtrlStateT * ) __attribute__((used));

static SaErrorT
IpmiSetControlState( void *hnd, SaHpiResourceIdT id,
                     SaHpiCtrlNumT num,
                     SaHpiCtrlModeT mode,
                     SaHpiCtrlStateT *state )
{
  cIpmi *ipmi;
  cIpmiControl *control = VerifyControlAndEnter( hnd, id, num, ipmi );

  if ( !control )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = control->SetState( mode, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetIdrInfo( void *,
                SaHpiResourceIdT,
                SaHpiIdrIdT,
                SaHpiIdrInfoT * ) __attribute__((used));

static SaErrorT
IpmiGetIdrInfo( void *hnd,
                SaHpiResourceIdT id,
                SaHpiIdrIdT idrid,
                SaHpiIdrInfoT *idrinfo )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->GetIdrInfo( idrid, *idrinfo );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetIdrAreaHeader( void *,
                      SaHpiResourceIdT,
                      SaHpiIdrIdT,
                      SaHpiIdrAreaTypeT,
                                  SaHpiEntryIdT,
                      SaHpiEntryIdT *,
                      SaHpiIdrAreaHeaderT * ) __attribute__((used));

static SaErrorT
IpmiGetIdrAreaHeader( void *hnd,
                      SaHpiResourceIdT id,
                      SaHpiIdrIdT idrid,
                      SaHpiIdrAreaTypeT areatype,
                                  SaHpiEntryIdT areaid,
                      SaHpiEntryIdT *nextareaid,
                      SaHpiIdrAreaHeaderT *header )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->GetIdrAreaHeader( idrid, areatype, areaid, *nextareaid, *header );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiAddIdrArea( void *,
                SaHpiResourceIdT,
                SaHpiIdrIdT,
                SaHpiIdrAreaTypeT,
                            SaHpiEntryIdT * ) __attribute__((used));

static SaErrorT
IpmiAddIdrArea( void *hnd,
                SaHpiResourceIdT id,
                SaHpiIdrIdT idrid,
                SaHpiIdrAreaTypeT areatype,
                            SaHpiEntryIdT *areaid )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->AddIdrArea( idrid, areatype, *areaid );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiDelIdrArea( void *,
                SaHpiResourceIdT,
                SaHpiIdrIdT,
                            SaHpiEntryIdT ) __attribute__((used));

static SaErrorT
IpmiDelIdrArea( void *hnd,
                SaHpiResourceIdT id,
                SaHpiIdrIdT idrid,
                            SaHpiEntryIdT areaid )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->DelIdrArea( idrid, areaid );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetIdrField( void *,
                 SaHpiResourceIdT,
                 SaHpiIdrIdT,
                 SaHpiEntryIdT,
                 SaHpiIdrFieldTypeT,
                 SaHpiEntryIdT,
                             SaHpiEntryIdT *,
                 SaHpiIdrFieldT * ) __attribute__((used));

static SaErrorT
IpmiGetIdrField( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiIdrIdT idrid,
                 SaHpiEntryIdT areaid,
                 SaHpiIdrFieldTypeT fieldtype,
                 SaHpiEntryIdT fieldid,
                             SaHpiEntryIdT *nextfieldid,
                 SaHpiIdrFieldT *field )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->GetIdrField( idrid, areaid, fieldtype, fieldid, *nextfieldid, *field );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiAddIdrField( void *,
                 SaHpiResourceIdT,
                 SaHpiIdrIdT,
                 SaHpiIdrFieldT * ) __attribute__((used));

static SaErrorT
IpmiAddIdrField( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiIdrIdT idrid,
                 SaHpiIdrFieldT *field )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->AddIdrField( idrid, *field );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetIdrField( void *,
                 SaHpiResourceIdT,
                 SaHpiIdrIdT,
                 SaHpiIdrFieldT * ) __attribute__((used));

static SaErrorT
IpmiSetIdrField( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiIdrIdT idrid,
                 SaHpiIdrFieldT *field )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->SetIdrField( idrid, *field );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiDelIdrField( void *,
                 SaHpiResourceIdT,
                 SaHpiIdrIdT,
                 SaHpiEntryIdT,
                 SaHpiEntryIdT ) __attribute__((used));

static SaErrorT
IpmiDelIdrField( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiIdrIdT idrid,
                 SaHpiEntryIdT areaid,
                 SaHpiEntryIdT fieldid )
{
  cIpmi *ipmi = 0;
  cIpmiInventory *inv = VerifyInventoryAndEnter( hnd, id, idrid, ipmi );

  if ( !inv )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = inv->DelIdrField( idrid, areaid, fieldid );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSelInfo( void *,
                SaHpiResourceIdT,
                SaHpiEventLogInfoT * ) __attribute__((used));

static SaErrorT
IpmiGetSelInfo( void *hnd,
                SaHpiResourceIdT id,
                SaHpiEventLogInfoT   *info )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->GetSelInfo( *info );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSelTime( void *, SaHpiResourceIdT, SaHpiTimeT ) __attribute__((used));

static SaErrorT
IpmiSetSelTime( void *hnd, SaHpiResourceIdT id, SaHpiTimeT t )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->SetSelTime( t );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiAddSelEntry( void *, SaHpiResourceIdT,
                 const SaHpiEventT * ) __attribute__((used));

static SaErrorT
IpmiAddSelEntry( void *hnd, SaHpiResourceIdT id,
                 const SaHpiEventT *Event )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->AddSelEntry( *Event );

  ipmi->IfLeave();

  return rv;
}

#ifdef NOTUSED
static SaErrorT
IpmiDelSelEntry( void *, SaHpiResourceIdT,
                 SaHpiEventLogEntryIdT ) __attribute__((used));

static SaErrorT
IpmiDelSelEntry( void *hnd, SaHpiResourceIdT id,
                 SaHpiEventLogEntryIdT sid )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->DeleteSelEntry( sid );

  ipmi->IfLeave();

  return rv;
}
#endif


static SaErrorT
IpmiGetSelEntry( void *hnd, SaHpiResourceIdT,
                 SaHpiEventLogEntryIdT,
                 SaHpiEventLogEntryIdT *, SaHpiEventLogEntryIdT *,
                 SaHpiEventLogEntryT *,
                 SaHpiRdrT *,
                 SaHpiRptEntryT * ) __attribute__((used));

static SaErrorT
IpmiGetSelEntry( void *hnd, SaHpiResourceIdT id,
                 SaHpiEventLogEntryIdT current,
                 SaHpiEventLogEntryIdT *prev, SaHpiEventLogEntryIdT *next,
                 SaHpiEventLogEntryT *entry,
                 SaHpiRdrT *rdr,
                 SaHpiRptEntryT *rptentry )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->GetSelEntry( current, *prev, *next, *entry, *rdr, *rptentry );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiClearSel( void *, SaHpiResourceIdT ) __attribute__((used));

static SaErrorT
IpmiClearSel( void *hnd, SaHpiResourceIdT id )
{
  cIpmi *ipmi = 0;
  cIpmiSel *sel = VerifySelAndEnter( hnd, id, ipmi );

  if ( !sel )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = sel->ClearSel();

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiHotswapPolicyCancel( void *, SaHpiResourceIdT,
                         SaHpiTimeoutT ) __attribute__((used));

static SaErrorT
IpmiHotswapPolicyCancel( void *hnd, SaHpiResourceIdT id,
                         SaHpiTimeoutT timeout)
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfHotswapPolicyCancel( res, timeout );

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiSetAutoInsertTimeout( void *, SaHpiTimeoutT ) __attribute__((used));

static SaErrorT
IpmiSetAutoInsertTimeout( void *hnd, SaHpiTimeoutT  timeout)
{
  cIpmi *ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       return SA_ERR_HPI_INTERNAL_ERROR;
     }

  SaErrorT rv = ipmi->IfSetAutoInsertTimeout( timeout );

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiGetAutoExtractTimeout( void *, SaHpiResourceIdT,
                           SaHpiTimeoutT * ) __attribute__((used));

static SaErrorT
IpmiGetAutoExtractTimeout( void *hnd, SaHpiResourceIdT id,
                           SaHpiTimeoutT *timeout )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetAutoExtractTimeout( res, *timeout );

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiSetAutoExtractTimeout( void *, SaHpiResourceIdT,
                           SaHpiTimeoutT ) __attribute__((used));

static SaErrorT
IpmiSetAutoExtractTimeout( void *hnd, SaHpiResourceIdT id,
                           SaHpiTimeoutT timeout )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetAutoExtractTimeout( res, timeout );

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiGetHotswapState( void *, SaHpiResourceIdT ,
                     SaHpiHsStateT * ) __attribute__((used));

static SaErrorT
IpmiGetHotswapState( void *hnd, SaHpiResourceIdT id,
                     SaHpiHsStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetHotswapState( res, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetHotswapState( void *, SaHpiResourceIdT,
                     SaHpiHsStateT ) __attribute__((used));

static SaErrorT
IpmiSetHotswapState( void *hnd, SaHpiResourceIdT id,
                     SaHpiHsStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetHotswapState( res, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiRequestHotswapAction( void *, SaHpiResourceIdT,
                          SaHpiHsActionT ) __attribute__((used));

static SaErrorT
IpmiRequestHotswapAction( void *hnd, SaHpiResourceIdT id,
                          SaHpiHsActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfRequestHotswapAction( res, act );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetPowerState( void *, SaHpiResourceIdT,
                   SaHpiPowerStateT * ) __attribute__((used));

static SaErrorT
IpmiGetPowerState( void *hnd, SaHpiResourceIdT id,
                   SaHpiPowerStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetPowerState( res, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetPowerState( void *, SaHpiResourceIdT,
                   SaHpiPowerStateT ) __attribute__((used));

static SaErrorT
IpmiSetPowerState( void *hnd, SaHpiResourceIdT id,
                   SaHpiPowerStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetPowerState( res, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetIndicatorState( void *, SaHpiResourceIdT,
                       SaHpiHsIndicatorStateT * ) __attribute__((used));

static SaErrorT
IpmiGetIndicatorState( void *hnd, SaHpiResourceIdT id,
                       SaHpiHsIndicatorStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetIndicatorState( res, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetIndicatorState( void *, SaHpiResourceIdT,
                       SaHpiHsIndicatorStateT ) __attribute__((used));

static SaErrorT
IpmiSetIndicatorState( void *hnd, SaHpiResourceIdT id,
                       SaHpiHsIndicatorStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetIndicatorState( res, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiControlParm( void *,
                 SaHpiResourceIdT,
                 SaHpiParmActionT ) __attribute__((used));

static SaErrorT
IpmiControlParm( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiParmActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfControlParm( res, act );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetResetState( void *, SaHpiResourceIdT,
                   SaHpiResetActionT * ) __attribute__((used));

static SaErrorT
IpmiGetResetState( void *hnd, SaHpiResourceIdT id,
                   SaHpiResetActionT *act )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetResetState( res, *act );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetResetState( void *,
                   SaHpiResourceIdT,
                   SaHpiResetActionT ) __attribute__((used));

static SaErrorT
IpmiSetResetState( void *hnd,
                   SaHpiResourceIdT id,
                   SaHpiResetActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiResource *res = VerifyResourceAndEnter( hnd, id, ipmi );

  if ( !res )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetResetState( res, act );

  ipmi->IfLeave();

  return rv;
}

static SaErrorT
IpmiGetWatchdogInfo(void *,
                    SaHpiResourceIdT,
                    SaHpiWatchdogNumT,
                    SaHpiWatchdogT *) __attribute__((used));

static SaErrorT
IpmiGetWatchdogInfo(void *hnd,
                    SaHpiResourceIdT  id,
                    SaHpiWatchdogNumT num,
                    SaHpiWatchdogT    *watchdog)
{
  cIpmi *ipmi = 0;
  cIpmiWatchdog *wd = VerifyWatchdogAndEnter( hnd, id, num, ipmi );
  if ( !wd ) return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = wd->GetWatchdogInfo( *watchdog );
  ipmi->IfLeave();
  return rv; 
}

static SaErrorT
IpmiSetWatchdogInfo(void *,
                    SaHpiResourceIdT,
                    SaHpiWatchdogNumT,
                    SaHpiWatchdogT *) __attribute__((used));

static SaErrorT
IpmiSetWatchdogInfo(void *hnd,
                    SaHpiResourceIdT  id,
                    SaHpiWatchdogNumT num,
                    SaHpiWatchdogT    *watchdog)
{
  cIpmi *ipmi = 0;
  cIpmiWatchdog *wd = VerifyWatchdogAndEnter( hnd, id, num, ipmi );
  if ( !wd ) return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = wd->SetWatchdogInfo( *watchdog );
  ipmi->IfLeave();
  return rv; 
}

static SaErrorT
IpmiResetWatchdog(void *,
                  SaHpiResourceIdT,
                  SaHpiWatchdogNumT) __attribute__((used));

static SaErrorT
IpmiResetWatchdog(void *hnd,
                  SaHpiResourceIdT  id,
                  SaHpiWatchdogNumT num)
{
  cIpmi *ipmi = 0;
  cIpmiWatchdog *wd = VerifyWatchdogAndEnter( hnd, id, num, ipmi );
  if ( !wd ) return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = wd->ResetWatchdog();
  ipmi->IfLeave();
  return rv; 
}


} // new plugin_loader

extern "C" {

void * oh_open (GHashTable *, unsigned int, oh_evt_queue *) __attribute__ ((weak, alias("IpmiOpen")));

void * oh_close (void *) __attribute__ ((weak, alias("IpmiClose")));

void * oh_get_event (void *)
                __attribute__ ((weak, alias("IpmiGetEvent")));

void * oh_discover_resources (void *)
                __attribute__ ((weak, alias("IpmiDiscoverResources")));

void * oh_set_resource_tag (void *, SaHpiResourceIdT, SaHpiTextBufferT *)
                __attribute__ ((weak, alias("IpmiSetResourceTag")));

void * oh_set_resource_severity (void *, SaHpiResourceIdT, SaHpiSeverityT)
                __attribute__ ((weak, alias("IpmiSetResourceSeverity")));

void * oh_get_el_info (void *, SaHpiResourceIdT, SaHpiEventLogInfoT *)
                __attribute__ ((weak, alias("IpmiGetSelInfo")));

void * oh_set_el_time (void *, SaHpiResourceIdT, const SaHpiEventT *)
                __attribute__ ((weak, alias("IpmiSetSelTime")));

void * oh_add_el_entry (void *, SaHpiResourceIdT, const SaHpiEventT *)
                __attribute__ ((weak, alias("IpmiAddSelEntry")));

void * oh_get_el_entry (void *, SaHpiResourceIdT, SaHpiEventLogEntryIdT,
                       SaHpiEventLogEntryIdT *, SaHpiEventLogEntryIdT *,
                       SaHpiEventLogEntryT *, SaHpiRdrT *, SaHpiRptEntryT  *)
                __attribute__ ((weak, alias("IpmiGetSelEntry")));

void * oh_clear_el (void *, SaHpiResourceIdT)
                __attribute__ ((weak, alias("IpmiClearSel")));

void * oh_get_sensor_reading (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiSensorReadingT *,
                             SaHpiEventStateT    *)
                __attribute__ ((weak, alias("IpmiGetSensorReading")));

void * oh_get_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("IpmiGetSensorThresholds")));

void * oh_set_sensor_thresholds (void *, SaHpiResourceIdT,
                                 SaHpiSensorNumT,
                                 const SaHpiSensorThresholdsT *)
                __attribute__ ((weak, alias("IpmiSetSensorThresholds")));

void * oh_get_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT *)
                __attribute__ ((weak, alias("IpmiGetSensorEnable")));

void * oh_set_sensor_enable (void *, SaHpiResourceIdT,
                             SaHpiSensorNumT,
                             SaHpiBoolT)
                __attribute__ ((weak, alias("IpmiSetSensorEnable")));

void * oh_get_sensor_event_enables (void *, SaHpiResourceIdT,
                                    SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("IpmiGetSensorEventEnables")));

void * oh_set_sensor_event_enables (void *, SaHpiResourceIdT id, SaHpiSensorNumT,
                                    SaHpiBoolT *)
                __attribute__ ((weak, alias("IpmiSetSensorEventEnables")));

void * oh_get_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiEventStateT *, SaHpiEventStateT *)
                __attribute__ ((weak, alias("IpmiGetSensorEventMasks")));

void * oh_set_sensor_event_masks (void *, SaHpiResourceIdT, SaHpiSensorNumT,
                                  SaHpiSensorEventMaskActionT,
                                  SaHpiEventStateT,
                                  SaHpiEventStateT)
                __attribute__ ((weak, alias("IpmiSetSensorEventMasks")));

void * oh_get_control_state (void *, SaHpiResourceIdT, SaHpiCtrlNumT,
                             SaHpiCtrlModeT *, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("IpmiGetControlState")));

void * oh_set_control_state (void *, SaHpiResourceIdT,SaHpiCtrlNumT,
                             SaHpiCtrlModeT, SaHpiCtrlStateT *)
                __attribute__ ((weak, alias("IpmiSetControlState")));

void * oh_get_idr_info (void *hnd, SaHpiResourceIdT, SaHpiIdrIdT,SaHpiIdrInfoT)
                __attribute__ ((weak, alias("IpmiGetIdrInfo")));

void * oh_get_idr_area_header (void *, SaHpiResourceIdT, SaHpiIdrIdT,
                                SaHpiIdrAreaTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                                SaHpiIdrAreaHeaderT)
                __attribute__ ((weak, alias("IpmiGetIdrAreaHeader")));

void * oh_add_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrAreaTypeT,
                        SaHpiEntryIdT)
                __attribute__ ((weak, alias("IpmiAddIdrArea")));

void * oh_del_idr_area (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT)
                __attribute__ ((weak, alias("IpmiDelIdrArea")));

void * oh_get_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldTypeT, SaHpiEntryIdT, SaHpiEntryIdT,
                         SaHpiIdrFieldT)
                __attribute__ ((weak, alias("IpmiGetIdrField")));

void * oh_add_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
                __attribute__ ((weak, alias("IpmiAddIdrField")));

void * oh_set_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiIdrFieldT)
                __attribute__ ((weak, alias("IpmiSetIdrField")));

void * oh_del_idr_field (void *, SaHpiResourceIdT, SaHpiIdrIdT, SaHpiEntryIdT,
                         SaHpiEntryIdT)
                __attribute__ ((weak, alias("IpmiDelIdrField")));

void * oh_hotswap_policy_cancel (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak, alias("IpmiHotswapPolicyCancel")));

void * oh_set_autoinsert_timeout (void *, SaHpiTimeoutT)
                __attribute__ ((weak, alias("IpmiSetAutoInsertTimeout")));

void * oh_get_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT *)
                __attribute__ ((weak, alias("IpmiGetAutoExtractTimeout")));

void * oh_set_autoextract_timeout (void *, SaHpiResourceIdT, SaHpiTimeoutT)
                __attribute__ ((weak, alias("IpmiSetAutoExtractTimeout")));

void * oh_get_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT *)
                __attribute__ ((weak, alias("IpmiGetHotswapState")));

void * oh_set_hotswap_state (void *, SaHpiResourceIdT, SaHpiHsStateT)
                __attribute__ ((weak, alias("IpmiSetHotswapState")));

void * oh_request_hotswap_action (void *, SaHpiResourceIdT, SaHpiHsActionT)
                __attribute__ ((weak, alias("IpmiRequestHotswapAction")));

void * oh_get_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT *)
                __attribute__ ((weak, alias("IpmiGetPowerState")));

void * oh_set_power_state (void *, SaHpiResourceIdT, SaHpiPowerStateT)
                __attribute__ ((weak, alias("IpmiSetPowerState")));

void * oh_get_indicator_state (void *, SaHpiResourceIdT,
                               SaHpiHsIndicatorStateT *)
                __attribute__ ((weak, alias("IpmiGetIndicatorState")));

void * oh_set_indicator_state (void *, SaHpiResourceIdT,
                               SaHpiHsIndicatorStateT)
                __attribute__ ((weak, alias("IpmiSetIndicatorState")));

void * oh_control_parm (void *, SaHpiResourceIdT, SaHpiParmActionT)
                __attribute__ ((weak, alias("IpmiControlParm")));

void * oh_get_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT *)
                __attribute__ ((weak, alias("IpmiGetResetState")));

void * oh_set_reset_state (void *, SaHpiResourceIdT, SaHpiResetActionT)
                __attribute__ ((weak, alias("IpmiSetResetState")));

void * oh_get_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
                __attribute__ ((weak, alias("IpmiGetWatchdogInfo")));
void * oh_set_watchdog_info (void *, SaHpiResourceIdT, SaHpiWatchdogNumT,
                             SaHpiWatchdogT *)
                __attribute__ ((weak, alias("IpmiSetWatchdogInfo")));
void * oh_reset_watchdog (void *, SaHpiResourceIdT , SaHpiWatchdogNumT )
                __attribute__ ((weak, alias("IpmiResetWatchdog")));
}


static unsigned int
GetIntNotNull( GHashTable *handler_config, const char *str, unsigned int def = 0 )
{
  const char *value = (const char *)g_hash_table_lookup(handler_config, str );

  if ( !value )
       return def;

  unsigned int v = strtol( value, 0, 0 );

  if ( v == 0 )
       return def;

  return v;
}

static SaHpiTimeoutT
GetTimeout( GHashTable *handler_config, const char *str, SaHpiTimeoutT def )
{
  const char *value = (const char *)g_hash_table_lookup(handler_config, str );

  if ( !value )
       return def;

  int v = strtol( value, 0, 0 );

  if ( v == 0 )
       return SAHPI_TIMEOUT_IMMEDIATE;

  if ( v == -1 )
       return SAHPI_TIMEOUT_BLOCK;

  SaHpiTimeoutT timeout = v * 1000000000;

  return timeout;
}


cIpmi::cIpmi()
  : m_magic( dIpmiMagic ),
    m_handler( 0 )
{
}


cIpmi::~cIpmi()
{
}


void
cIpmi::SetHandler( oh_handler_state *handler )
{
  m_handler = handler;
}


// wrapper class move async events to domain
class cIpmiConLanDomain : public cIpmiConLan
{
  cIpmiDomain *m_domain;

public:
  cIpmiConLanDomain( cIpmiDomain *domain,
                     unsigned int timeout, int log_level,
                     struct in_addr addr, int port,
                     tIpmiAuthType auth, tIpmiPrivilege priv,
                     char *user, char *passwd )
    : cIpmiConLan( timeout, log_level, addr, port, auth, priv,
                   user, passwd ),
      m_domain( domain )
  {
  }

  virtual ~cIpmiConLanDomain()
  {
  }

  virtual void HandleAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg )
  {
    m_domain->HandleAsyncEvent( addr, msg );
  }
};


// wrapper class move async events to domain
class cIpmiConSmiDomain : public cIpmiConSmi
{
  cIpmiDomain *m_domain;

public:
  cIpmiConSmiDomain( cIpmiDomain *domain,
                     unsigned int timeout, int log_level, int if_num )
    : cIpmiConSmi( timeout, log_level, if_num ), m_domain( domain )
  {
  }

  virtual ~cIpmiConSmiDomain()
  {
  }

  virtual void HandleAsyncEvent( const cIpmiAddr &addr, const cIpmiMsg &msg )
  {
    m_domain->HandleAsyncEvent( addr, msg );
  }
};


cIpmiCon *
cIpmi::AllocConnection( GHashTable *handler_config )
{
  // default is 5s for IPMI
  m_con_ipmi_timeout = GetIntNotNull( handler_config, "IpmiConnectionTimeout", 5000 );
  stdlog << "AllocConnection: IPMITimeout " << m_con_ipmi_timeout << " ms.\n";

   // default is 1s for ATCA systems
  m_con_atca_timeout = GetIntNotNull( handler_config, "AtcaConnectionTimeout", 1000 );
  stdlog << "AllocConnection: AtcaTimeout " << m_con_atca_timeout << " ms.\n";

  unsigned int enable_sel_on_all = GetIntNotNull( handler_config, "EnableSelOnAll", 0 );
  if ( enable_sel_on_all == 1 )
     {
        m_enable_sel_on_all = true;
        stdlog << "AllocConnection: Enable SEL on all MCs.\n";
     }
  else
     {
        m_enable_sel_on_all = false;
        stdlog << "AllocConnection: Enable SEL only on BMC.\n";
     }

  // outstanding messages 0 => read from BMC/ShMc
  m_max_outstanding = GetIntNotNull( handler_config, "MaxOutstanding", 0 );

  if ( m_max_outstanding > 256 )
       m_max_outstanding = 256;

  stdlog << "AllocConnection: Max Outstanding IPMI messages "
         << m_max_outstanding << ".\n";

  unsigned int poll_alive = GetIntNotNull( handler_config, "AtcaPollAliveMCs", 0 );
  if ( poll_alive == 1 )
     {
        m_atca_poll_alive_mcs = true;
        stdlog << "AllocConnection: Poll alive MCs.\n";
     }
  else
     {
        m_atca_poll_alive_mcs = false;
        stdlog << "AllocConnection: Don't poll alive MCs.\n";
     }

  m_own_domain = false;
  /** This code block has been commented out due to the
   ** multi-domain changes in the infrastructure.
   ** (Renier Morales 11/21/06)
  const char *create_own_domain = (const char *)g_hash_table_lookup(handler_config, "MultipleDomains");
  if ((create_own_domain != (char *)NULL)
      && ((strcmp(create_own_domain, "YES") == 0)
            || (strcmp(create_own_domain, "yes") == 0)))
  {
      int *hid = (int *)g_hash_table_lookup(handler_config, "handler-id");
      if (hid)
      {
          m_own_domain = true;
          m_handler_id = *hid;
          stdlog << "AllocConnection: Multi domain handler " << *hid << "\n";

          const char *domain_tag = (const char *)g_hash_table_lookup(handler_config, "DomainTag");

          if (domain_tag != NULL)
          {
              m_domain_tag.SetAscii(domain_tag, SAHPI_TL_TYPE_TEXT, SAHPI_LANG_ENGLISH);
          }
      }
  }*/

  m_insert_timeout = GetTimeout( handler_config, "InsertTimeout", SAHPI_TIMEOUT_IMMEDIATE );
  m_extract_timeout = GetTimeout( handler_config, "ExtractTimeout", SAHPI_TIMEOUT_IMMEDIATE );

  const char *name = (const char *)g_hash_table_lookup(handler_config, "name");

  if ( !name )
     {
       stdlog << "Empty parameter !\n";
       return 0;
     }

  stdlog << "IpmiAllocConnection: connection name = '" << name << "'.\n";

  if ( !strcmp( name, "lan" ) || !strcmp( name, "rmcp" ) )
     {
       const char     *addr;
       struct in_addr  lan_addr;
       int             lan_port   = dIpmiConLanStdPort;
       tIpmiAuthType   auth       = eIpmiAuthTypeNone;
       tIpmiPrivilege  priv       = eIpmiPrivilegeAdmin;
       char            user[32]   = "";
       char            passwd[32] = "";
       char           *value;
       struct hostent *ent;

       // Address
       addr = (const char *)g_hash_table_lookup(handler_config, "addr");

       if ( !addr )
          {
            stdlog << "TCP/IP address missing in config file !\n";
            return 0;
          }

       stdlog << "AllocConnection: addr = '" << addr << "'.\n";
       ent = gethostbyname( addr );

       if ( !ent )
          {
            stdlog << "Unable to resolve IPMI LAN address: " << addr << " !\n";
            return 0;
          }

       memcpy( &lan_addr, ent->h_addr_list[0], ent->h_length );
       unsigned int a = *(unsigned int *)(void *)ent->h_addr_list[0];

       stdlog << "Using host at "
              << (int)(a & 0xff) << "."
              << (int)((a >> 8 ) & 0xff) << "."
              << (int)((a >> 16) & 0xff) << "."
              << (int)((a >> 24) & 0xff) << ".\n";

       // Port
       lan_port = GetIntNotNull( handler_config, "port", 623 );

       stdlog << "AllocConnection: port = " << lan_port << ".\n";

       // Authentication type
       value = (char *)g_hash_table_lookup( handler_config, "auth_type" );

       if ( value )
          {
            if ( !strcmp( value, "none" ) )
                 auth = eIpmiAuthTypeNone;
            else if ( !strcmp( value, "straight" ) )
                 auth = eIpmiAuthTypeStraight;
            else if ( !strcmp( value, "md2" ) )
#ifdef HAVE_OPENSSL_MD2_H
                 auth = eIpmiAuthTypeMd2;
#else
               {
                 stdlog << "MD2 is not supported. Please install SSL and recompile.\n";
                 return 0;
               }
#endif
            else if ( !strcmp( value, "md5" ) )
#ifdef HAVE_OPENSSL_MD5_H
                 auth = eIpmiAuthTypeMd5;
#else
               {
                 stdlog << "MD5 is not supported. Please install SSL and recompile.\n";
                 return 0;
               }
#endif
            else
               {
                 stdlog << "Invalid IPMI LAN authentication method '" << value << "' !\n";
                 return 0;
               }
          }

       stdlog << "AllocConnection: authority: " << value << "(" << auth << ").\n";

       // Priviledge
       value = (char *)g_hash_table_lookup(handler_config, "auth_level" );

       if ( value )
          {
            if ( !strcmp( value, "operator" ) )
                 priv = eIpmiPrivilegeOperator;
            else if ( !strcmp( value, "admin" ) )
                 priv = eIpmiPrivilegeAdmin;
            else
               {
                 stdlog << "Invalid authentication method '" << value << "' !\n";
                 stdlog << "Only operator and admin are supported !\n";
                 return 0;
               }
          }

       stdlog << "AllocConnection: priviledge = " << value << "(" << priv << ").\n";

       // User Name
       value = (char *)g_hash_table_lookup( handler_config, "username" );

       if ( value )
            strncpy( user, value, 32);

       stdlog << "AllocConnection: user = " << user << ".\n";

       // Password
       value = (char *)g_hash_table_lookup( handler_config, "password" );

       if ( value )
            strncpy( passwd, value, 32 );

       return new cIpmiConLanDomain( this, m_con_ipmi_timeout, dIpmiConLogAll,
                                     lan_addr, lan_port, auth, priv,
                                     user, passwd );
     }
  else if ( !strcmp( name, "smi" ) )
     {
       const char *addr = (const char *)g_hash_table_lookup(handler_config, "addr");

       int if_num = 0;

       if ( addr )
            if_num = strtol( addr, 0, 10 );

       stdlog << "AllocConnection: interface number = " << if_num << ".\n";

       return new cIpmiConSmiDomain( this, m_con_ipmi_timeout, dIpmiConLogAll, if_num );
     }

  stdlog << "Unknown connection type: " << name << " !\n";

  return 0;
}


void
cIpmi::AddHpiEvent( oh_event *event )
{
  m_event_lock.Lock();

  if ( m_handler )
  {
    event->hid = m_handler->hid;
    oh_evt_queue_push(m_handler->eventq, event);
  }

  m_event_lock.Unlock();
}


const cIpmiEntityPath &
cIpmi::EntityRoot()
{
  return m_entity_root;
}


oh_handler_state *
cIpmi::GetHandler()
{
  return m_handler;
}


SaHpiRptEntryT *
cIpmi::FindResource( SaHpiResourceIdT rid )
{
  if ( m_handler )
  {
      return oh_get_resource_by_id( m_handler->rptcache, rid);
  }
  else
  {
      return 0;
  }
}


void
cIpmi::IfEnter()
{
  ReadLock();
}


void
cIpmi::IfLeave()
{
  ReadUnlock();
}


bool
cIpmi::GetParams( GHashTable *handler_config )
{
  // get mcs to scan
  char str[100];

  for( unsigned int i = 1; i < 0xf1; i++ )
     {
       snprintf( str, sizeof(str), "MC%02x", i );

       char *value = (char *)g_hash_table_lookup( handler_config,
                                                  str );

       if ( value == 0 )
          {
            snprintf( str, sizeof(str), "MC%02X", i );

            value = (char *)g_hash_table_lookup( handler_config, str );
          }

       if ( value == 0 )
            continue;

       unsigned int properties = 0;

       char *tokptr;
       char *tok = strtok_r( value, " \t\n", &tokptr );

       while( tok )
          {
            if ( !strcmp( tok, "initial_discover" ) )
                 properties |= dIpmiMcThreadInitialDiscover;
            else if ( !strcmp( tok, "poll_alive" ) )
                 properties |= dIpmiMcThreadPollAliveMc;
            else if ( !strcmp(tok, "poll_dead" ) )
                 properties |= dIpmiMcThreadPollDeadMc;
            else
                 stdlog << "unknown propertiy for MC " << (unsigned char)i
                        << ": " << tok << " !\n";

            tok = strtok_r( 0, " \t\n", &tokptr );
          }

       if ( properties == 0 )
            continue;

       char pp[256] = "";

       if ( properties & dIpmiMcThreadInitialDiscover )
            strcat( pp, " initial_discover" );

       if ( properties &  dIpmiMcThreadPollAliveMc )
            strcat( pp, " poll_alive" );

       if ( properties &  dIpmiMcThreadPollDeadMc )
            strcat( pp, " poll_dead" );

       stdlog << "MC " << (unsigned char)i << " properties: " << pp << ".\n";

       NewFruInfo( i, 0, SAHPI_ENT_SYS_MGMNT_MODULE, GetFreeSlotForOther( i ), eIpmiAtcaSiteTypeUnknown, properties );
     }

  return true;
}


bool
cIpmi::IfOpen( GHashTable *handler_config )
{
  const char *entity_root = (const char *)g_hash_table_lookup( handler_config, "entity_root" );

  if ( !entity_root )
     {
       err( "entity_root is missing in config file" );
       return false;
     }

  if ( !m_entity_root.FromString( entity_root ) )
     {
       err( "cannot decode entity path string" );
       return false;
     }

  cIpmiCon *con = AllocConnection( handler_config );

  if ( !con )
     {
       stdlog << "IPMI cannot alloc connection !\n";
       return false;
     }

  if ( !GetParams( handler_config ) )
     {
       delete con;
       return false;
     }

  bool rv = con->Open();

  if ( rv == false )
     {
       stdlog << "IPMI open connection fails !\n";

       delete con;

       return false;
     }

  if ( !Init( con ) )
     {
       IfClose();
       return false;
     }

  return true;
}


void
cIpmi::IfClose()
{
  Cleanup();

  if ( m_con )
     {
       delete m_con;
       m_con = 0;
     }
}


int
cIpmi::IfGetEvent( oh_event *event )
{
  int rv = 0;

  m_event_lock.Lock();

  m_event_lock.Unlock();

  return rv;
}


SaErrorT
cIpmi::IfDiscoverResources()
{
  dbg( "ipmidirect discover_resources");

  bool loop;

  do
     {
       usleep( 10000 );

       m_initial_discover_lock.Lock();
       loop = m_initial_discover ? true : false;
       m_initial_discover_lock.Unlock();
     }
  while( loop );

  return SA_OK;
}


SaErrorT
cIpmi::IfSetResourceTag( cIpmiResource *ent, SaHpiTextBufferT *tag )
{
  // change tag in plugin cache
  SaHpiRptEntryT *rptentry = oh_get_resource_by_id( ent->Domain()->GetHandler()->rptcache,
                                                    ent->m_resource_id );
  if ( !rptentry )
      return SA_ERR_HPI_NOT_PRESENT;

  memcpy(&rptentry->ResourceTag, tag, sizeof(SaHpiTextBufferT));

  oh_add_resource(ent->Domain()->GetHandler()->rptcache,
                    rptentry, ent, 1);

  return SA_OK;
}


SaErrorT
cIpmi::IfSetResourceSeverity( cIpmiResource *ent, SaHpiSeverityT sev )
{
  // change severity in plugin cache
  SaHpiRptEntryT *rptentry = oh_get_resource_by_id( ent->Domain()->GetHandler()->rptcache,
                                                    ent->m_resource_id );
  if ( !rptentry )
      return SA_ERR_HPI_NOT_PRESENT;

  rptentry->ResourceSeverity = sev;

  oh_add_resource(ent->Domain()->GetHandler()->rptcache,
                    rptentry, ent, 1);

  return SA_OK;
}


SaErrorT
cIpmi::IfControlParm( cIpmiResource * /*res*/, SaHpiParmActionT act )
{
  // TODO: implementation
  switch( act )
     {
       case SAHPI_DEFAULT_PARM:
            break;

       case SAHPI_SAVE_PARM:
            break;

       case SAHPI_RESTORE_PARM:
            break;
     }

  return SA_OK;
}
