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


#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "ipmi_entity.h"
#include "ipmi.h"


static void
GetEntityEvent( cIpmiEntity    *entity,
                cIpmiMc        *mc,
                int             id,
                int             instance,
                SaHpiRptEntryT *entry )
{
  entry->ResourceInfo.ResourceRev      = 0;
  entry->ResourceInfo.SpecificVer      = 0;
  entry->ResourceInfo.DeviceSupport    = 0;
  entry->ResourceInfo.ManufacturerId   = (SaHpiManufacturerIdT)mc->ManufacturerId();
  entry->ResourceInfo.ProductId        = (SaHpiUint16T)mc->ProductId();
  entry->ResourceInfo.FirmwareMajorRev = (SaHpiUint8T)mc->MajorFwRevision();
  entry->ResourceInfo.FirmwareMinorRev = (SaHpiUint8T)mc->MinorFwRevision();

  entry->ResourceInfo.AuxFirmwareRev            = (SaHpiUint8T)mc->AuxFwRevision( 0 );
  entry->ResourceEntity.Entry[0].EntityType     = (SaHpiEntityTypeT)id;
  entry->ResourceEntity.Entry[0].EntityInstance = (SaHpiEntityInstanceT)instance;
  entry->ResourceEntity.Entry[1].EntityType     = SAHPI_ENT_UNSPECIFIED;
  entry->ResourceEntity.Entry[1].EntityInstance = 0;

  // let's append entity_root from config
  const char *entity_root = entity->Domain()->EntityRoot();

  dbg( "entity_root: %s", entity_root );
  SaHpiEntityPathT entity_ep;
  string2entitypath( entity_root, &entity_ep );
  append_root( &entity_ep );

  ep_concat( &entry->ResourceEntity, &entity_ep );

  entry->EntryId = 0;
  entry->ResourceId = oh_uid_from_entity_path( &entry->ResourceEntity );

  // TODO: set SAHPI_CAPABILITY_FRU only if FRU
  entry->ResourceCapabilities = SAHPI_CAPABILITY_RESOURCE|SAHPI_CAPABILITY_FRU;
  entry->ResourceSeverity = SAHPI_OK;
  entry->DomainId = 0;
  entry->ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;

  const char *eid = entity->EntityIdString();
  assert( eid );

  entry->ResourceTag.Language = SAHPI_LANG_ENGLISH;
  entry->ResourceTag.DataLength = strlen( eid ); 

  memcpy( entry->ResourceTag.Data, eid, strlen( eid ) + 1 );
}


void
cIpmi::IfEntityAdd( cIpmiEntity *ent )
{
  struct oh_event *e;
  cIpmiMc         *mc;

  IpmiLog( "adding entity: %d.%d (%s).\n",
           ent->EntityId(), ent->EntityInstance(),
           ent->EntityIdString() );

  cIpmiAddr addr( eIpmiAddrTypeIpmb, ent->Channel(), 
                  ent->Lun(), ent->AccessAddress() );

  // addr.m_type    = eIpmiAddrTypeSystemInterface;
  // addr.m_channel = dIpmiBmcChannel;

  mc = FindMcByAddr( addr );

  if ( !mc )
     {
       addr.Si();
       mc = FindMcByAddr( addr );
     }

  assert( mc );

  e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );
  e->type               = oh_event::OH_ET_RESOURCE;

  GetEntityEvent( ent, mc, ent->EntityId(), ent->EntityInstance(),
                  &e->u.res_event.entry );

  // assign the hpi resource id to ent, so we can find
  // the resource for a given entity
  ent->m_resource_id = e->u.res_event.entry.ResourceId;

  // add the entity to the resource cache

  int rv = oh_add_resource( ent->Domain()->GetHandler()->rptcache, &(e->u.res_event.entry), ent, 1);
  assert( rv == 0 );

  AddHpiEvent( e );
}


void
cIpmi::IfEntityRem( cIpmiEntity *ent )
{
  IpmiLog( "removing entity: %d.%d (%s).\n",
           ent->EntityId(), ent->EntityInstance(),
           ent->EntityIdString() );

  // remove resource from local cache
  int rv = oh_remove_resource( ent->Domain()->GetHandler()->rptcache, ent->m_resource_id );
  assert( rv == 0 );

  oh_event *e = (oh_event *)g_malloc0( sizeof( oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n");
       return;
     }

  memset( e, 0, sizeof( struct oh_event ));
  e->type               = oh_event::OH_ET_RESOURCE_DEL;
  e->u.res_event.entry.ResourceId = ent->m_resource_id;
  AddHpiEvent( e );
}
