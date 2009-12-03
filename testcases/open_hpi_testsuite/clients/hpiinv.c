/*      -*- linux-c -*-
 *
 * hpiinv.c  
 *
 * Copyright (c) 2003-2005 by Intel Corp.
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Andy Cress <arcress@users.sourceforge.net>
 *     Peter D. Phan <pdphan@users.sourceforge.net>
 *     Renier Morales <renier@openhpi.org>
 *     Tariq Shureih <tariq.shureih@intel.com>
 *     Racing Guo <racing.guo@intel.com>
 *
 * Log: 
 *   6/29/04  Copied from hpifru.c and modified for general use
 *  11/30/04  ver 0.2 for openhpi-2.0.0
 *   2/09/05  ARCress re-merged with hpifru.c, adding IdrFieldSet,
 *            generalized for other HPI libraris also.
 */
#define OPENHPI_USED  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <SaHpi.h>
#include <oh_clients.h>
#ifdef OPENHPI_USED
#include <oh_utils.h>
#endif

#define OH_SVN_REV "$Revision: 1.6 $"

#define NCT 25
#define MAX_STRSIZE  64

char *chasstypes[NCT] = {
	"Not Defined", "Other", "Unknown", "Desktop", "Low Profile Desktop",
	"Pizza Box", "Mini Tower", "Tower", "Portable", "Laptop",
	"Notebook", "Handheld", "Docking Station", "All in One", "Subnotebook",
	"Space Saving", "Lunch Box", "Main Server", "Expansion", "SubChassis",
	"Buss Expansion Chassis", "Peripheral Chassis", "RAID Chassis", 
	"Rack-Mount Chassis", "New"
};
int fasset = 0;
int fverbose = 0;
int foundasset = 0;
int fdebug = 0;
int fzdebug = 0;
int i,j,k = 0;
SaHpiUint32T buffersize;
SaHpiUint32T actualsize;
char inbuff[2048];
char outbuff[1024];
SaHpiIdrAreaHeaderT  *inv;
SaHpiIdrFieldTypeT chasstype;  
SaHpiTextBufferT *strptr;
struct {
   SaHpiResourceIdT rid;
   SaHpiEntryIdT    idrid;
   SaHpiEntryIdT    areaid;
   SaHpiEntryIdT    fieldid;
   char *tag;
   int  tlen;
   } atag;
#define NRW_ENTS  12
SaHpiEntityTypeT rw_entities[NRW_ENTS] = {   /*read-writable entities*/
   SAHPI_ENT_SYS_MGMNT_MODULE,
   SAHPI_ENT_SYSTEM_BOARD,
   SAHPI_ENT_FRONT_PANEL_BOARD,
   SAHPI_ENT_SYSTEM_CHASSIS,
   SAHPI_ENT_SYS_MGMNT_SOFTWARE,
   SAHPI_ENT_COMPACTPCI_CHASSIS,
   SAHPI_ENT_ADVANCEDTCA_CHASSIS,
   SAHPI_ENT_RACK_MOUNTED_SERVER,
   SAHPI_ENT_SYSTEM_BLADE,
   SAHPI_ENT_SWITCH_BLADE,
   SAHPI_ENT_SPEC_PROC_BLADE,
   SAHPI_ENT_ALARM_MANAGER };

#ifdef BMCONLY
char bmctag[]  = "Basbrd Mgmt Ctlr";       /* see IsTagBmc() */
char bmctag2[] = "Management Controller";  /* see IsTagBmc() */

/*
 * findmatch
 * returns offset of the match if found, or -1 if not found.
 */
static int
findmatch(char *buffer, int sbuf, char *pattern, int spattern, char figncase)
{
    int c, j, imatch;
    j = 0;
    imatch = 0;
    for (j = 0; j < sbuf; j++) {
        if (sbuf - j < spattern && imatch == 0) return (-1);
        c = buffer[j];
        if (c == pattern[imatch]) {
            imatch++;
            if (imatch == spattern) return (++j - imatch);
        } else if (pattern[imatch] == '?') {  /*wildcard char*/
            imatch++;
            if (imatch == spattern) return (++j - imatch);
        } else if (figncase == 1) {
            if ((c & 0x5f) == (pattern[imatch] & 0x5f)) {
                imatch++;
                if (imatch == spattern) return (++j - imatch);
            } else
                imatch = 0;
        } else
            imatch = 0;
    }
    return (-1);
}                               /*end findmatch */

static int 
IsTagBmc(char *dstr, int dlen)
{
   int ret = 0;

   if (strncmp(dstr, bmctag, sizeof(bmctag)) == 0)  /* Sahalee */
	ret = 1;
   else if (findmatch(dstr,dlen,"BMC",3,1) >= 0) /* mBMC or other */
	ret = 1;
   else if (findmatch(dstr,dlen,bmctag2,sizeof(bmctag2),1) >= 0)
        ret = 1;    /* BMC tag for OpenHPI with ipmi plugin */
   return(ret);
}
#else
static int 
IsTagBmc(char *dstr, int dlen)
{
   /* if OpenHPI, always return TRUE for any Inventory RDR */
   return(1);
}
#endif

static int 
ent_writable(SaHpiEntityPathT *ep, SaHpiIdrInfoT *idrinfo)
{
   int i, rv;
   /* The IdrInfo ReadOnly field might be ok, but we don't
      even want to attempt to write to DIMMs or Power Supplies,
      even if the plugin reports it otherwise, so only check 
      for known writable entity types.   */
   for (i = 0; i < NRW_ENTS; i++)
      if (rw_entities[i] == ep->Entry[0].EntityType) break;
   if (i >= NRW_ENTS) rv = 0;
   else rv = 1;
   return(rv);
}

static void
fixstr(SaHpiTextBufferT *strptr, char *outstr)
{ 
	size_t datalen;

	if (outstr == NULL) return;
	outstr[0] = 0;
	if (strptr == NULL) return;
	datalen = strptr->DataLength;
	if (datalen > MAX_STRSIZE) datalen = MAX_STRSIZE-1;
	if (datalen != 0) {
		strncpy ((char *)outstr, (char *)strptr->Data, datalen);
		outstr[datalen] = 0;
		if (fdebug) { 
		  printf("TextBuffer len=%d, dtype=%x lang=%d: %s\n",
			strptr->DataLength,strptr->DataType,strptr->Language,
			strptr->Data );
		}
	}
#ifdef LENGTH_BAD
	else   /* may be bogus length, try anyway */
		strcpy ((char *)outstr, (char *)strptr->Data);
#endif
}

#define NAREATYP   6
static struct { SaHpiUint8T type; char *str;
} map_areatype[NAREATYP] = {
 { SAHPI_IDR_AREATYPE_INTERNAL_USE, "Internal Area" },
 { SAHPI_IDR_AREATYPE_CHASSIS_INFO, "Chassis Area" },
 { SAHPI_IDR_AREATYPE_BOARD_INFO,   "Board Area" },
 { SAHPI_IDR_AREATYPE_PRODUCT_INFO, "Product Area" },
 { SAHPI_IDR_AREATYPE_OEM,          "OEM Area" },
 { SAHPI_IDR_AREATYPE_UNSPECIFIED,  "Unspecified Area" }
};

#define NFIELDTYP   11
static struct { SaHpiUint8T type; char *str;
} map_fieldtype[NFIELDTYP] = {
  { SAHPI_IDR_FIELDTYPE_CHASSIS_TYPE,    "Chassis Type "},
  { SAHPI_IDR_FIELDTYPE_MFG_DATETIME,    "Mfg DateTime "},
  { SAHPI_IDR_FIELDTYPE_MANUFACTURER,    "Manufacturer "},
  { SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,    "Product Name "},
  { SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION, "Product Versn"},
  { SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,   "Serial Number"},
  { SAHPI_IDR_FIELDTYPE_PART_NUMBER,     "Part Number  "},
  { SAHPI_IDR_FIELDTYPE_FILE_ID,         "File ID      "},
  { SAHPI_IDR_FIELDTYPE_ASSET_TAG,       "Asset Tag    "},
  { SAHPI_IDR_FIELDTYPE_CUSTOM,          "Custom Field "},
  { SAHPI_IDR_FIELDTYPE_UNSPECIFIED,     "Unspecified  "}
};

#define NEPTYPES   67
struct { SaHpiEntityTypeT type; char *str; } map_epstr[NEPTYPES] = {
   { SAHPI_ENT_ADD_IN_CARD, "ADD_IN_CARD" },
   { SAHPI_ENT_ADVANCEDTCA_CHASSIS, "ADVANCEDTCA_CHASSIS" },
   { SAHPI_ENT_ALARM_MANAGER, "ALARM_MANAGER" },
   { SAHPI_ENT_BACK_PANEL_BOARD, "BACK_PANEL_BOARD" },
   { SAHPI_ENT_BATTERY, "BATTERY" },
   { SAHPI_ENT_BIOS, "BIOS" },
   { SAHPI_ENT_BOARD_SET_SPECIFIC, "BOARD_SET_SPECIFIC" },
   { SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD, "CHASSIS_BACK_PANEL_BOARD" },
   { SAHPI_ENT_CHASSIS_SPECIFIC, "CHASSIS_SPECIFIC" },
   { SAHPI_ENT_COMPACTPCI_CHASSIS, "COMPACTPCI_CHASSIS" },
   { SAHPI_ENT_COOLING_DEVICE, "COOLING_DEVICE" },
   { SAHPI_ENT_COOLING_UNIT, "COOLING_UNIT" },
   { SAHPI_ENT_DEVICE_BAY, "DEVICE_BAY" },
   { SAHPI_ENT_DISK_BAY, "DISK_BAY" },
   { SAHPI_ENT_DISK_BLADE, "DISK_BLADE" },
   { SAHPI_ENT_DISK_DRIVE, "DISK_DRIVE" },
   { SAHPI_ENT_DISK_DRIVE_BAY, "DISK_DRIVE_BAY" },
   { SAHPI_ENT_DISPLAY_PANEL, "DISPLAY_PANEL" },
   { SAHPI_ENT_DRIVE_BACKPLANE, "DRIVE_BACKPLANE" },
   { SAHPI_ENT_EXTERNAL_ENVIRONMENT, "EXTERNAL_ENVIRONMENT" },
   { SAHPI_ENT_FAN, "FAN" },
   { SAHPI_ENT_FRONT_PANEL_BOARD, "FRONT_PANEL_BOARD" },
   { SAHPI_ENT_GROUP, "GROUP" },
   { SAHPI_ENT_INTERCONNECT, "INTERCONNECT" },
   { SAHPI_ENT_IO_BLADE, "IO_BLADE" },
   { SAHPI_ENT_IO_SUBBOARD, "IO_SUBBOARD" },
   { SAHPI_ENT_MEMORY_DEVICE, "MEMORY_DEVICE" },
   { SAHPI_ENT_MEMORY_MODULE, "MEMORY_MODULE" },
   { SAHPI_ENT_OEM_SYSINT_SPECIFIC, "OEM_SYSINT_SPECIFIC" },
   { SAHPI_ENT_OPERATING_SYSTEM, "OPERATING_SYSTEM" },
   { SAHPI_ENT_OTHER, "OTHER" },
   { SAHPI_ENT_OTHER_CHASSIS_BOARD, "OTHER_CHASSIS_BOARD" },
   { SAHPI_ENT_OTHER_SYSTEM_BOARD, "OTHER_SYSTEM_BOARD" },
   { SAHPI_ENT_PERIPHERAL_BAY, "PERIPHERAL_BAY" },
   { SAHPI_ENT_PERIPHERAL_BAY_2, "PERIPHERAL_BAY_2" },
   { SAHPI_ENT_PHYSICAL_SLOT, "PHYSICAL_SLOT" },
   { SAHPI_ENT_POWER_DISTRIBUTION_UNIT, "POWER_DISTRIBUTION_UNIT" },
   { SAHPI_ENT_POWER_MGMNT, "POWER_MGMNT" },
   { SAHPI_ENT_POWER_MODULE, "POWER_MODULE" },
   { SAHPI_ENT_POWER_SUPPLY, "POWER_SUPPLY" },
   { SAHPI_ENT_POWER_SYSTEM_BOARD, "POWER_SYSTEM_BOARD" },
   { SAHPI_ENT_POWER_UNIT, "POWER_UNIT" },
   { SAHPI_ENT_PROCESSOR, "PROCESSOR" },
   { SAHPI_ENT_PROCESSOR_BOARD, "PROCESSOR_BOARD" },
   { SAHPI_ENT_PROCESSOR_MODULE, "PROCESSOR_MODULE" },
   { SAHPI_ENT_RACK, "RACK" },
   { SAHPI_ENT_RACK_MOUNTED_SERVER, "RACK_MOUNTED_SERVER" },
   { SAHPI_ENT_REMOTE, "REMOTE" },
   { SAHPI_ENT_ROOT, "ROOT" },
   { SAHPI_ENT_SBC_BLADE, "SBC_BLADE" },
   { SAHPI_ENT_SBC_SUBBOARD, "SBC_SUBBOARD" },
   { SAHPI_ENT_SHELF_MANAGER, "SHELF_MANAGER" },
   { SAHPI_ENT_SPEC_PROC_BLADE, "SPEC_PROC_BLADE" },
   { SAHPI_ENT_SUBBOARD_CARRIER_BLADE, "SUBBOARD_CARRIER_BLADE" },
   { SAHPI_ENT_SUB_CHASSIS, "SUB_CHASSIS" },
   { SAHPI_ENT_SUBRACK, "SUBRACK" },
   { SAHPI_ENT_SWITCH, "SWITCH" },
   { SAHPI_ENT_SWITCH_BLADE, "SWITCH_BLADE" },
   { SAHPI_ENT_SYS_EXPANSION_BOARD, "SYS_EXPANSION_BOARD" },
   { SAHPI_ENT_SYS_MGMNT_MODULE, "SYS_MGMNT_MODULE" },
   { SAHPI_ENT_SYS_MGMNT_SOFTWARE, "SYS_MGMNT_SOFTWARE" },
   { SAHPI_ENT_SYSTEM_BLADE, "SYSTEM_BLADE" },
   { SAHPI_ENT_SYSTEM_BOARD, "SYSTEM_BOARD" },
   { SAHPI_ENT_SYSTEM_BUS, "SYSTEM_BUS" },
   { SAHPI_ENT_SYSTEM_CHASSIS, "SYSTEM_CHASSIS" },
   { SAHPI_ENT_UNKNOWN, "UNKNOWN" },
   { SAHPI_ENT_UNSPECIFIED, "UNSPECIFIED" },
};

static void print_epath(SaHpiEntityPathT *epath, int len);
static void print_epath(SaHpiEntityPathT *epath, int len)
{
   int i,j;
   SaHpiEntityTypeT t;
   char *pstr;

#ifdef OPENHPI_USED
   if (fverbose) {
	oh_print_ep(epath, len);
   } else 
#endif
   {
	for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
	   t = epath->Entry[i].EntityType;
	   pstr = "";
	   for (j = 0; j < NEPTYPES; j++) {
		if (t == map_epstr[j].type) { 
			pstr = map_epstr[j].str; 
			break; 
		}
	   }
           if (j == NEPTYPES) j--;
	   printf("{%s:%d} ",pstr, epath->Entry[i].EntityLocation);
	   if (t == SAHPI_ENT_ROOT) break;
	}
	printf("\n");
   }
   return;
}

static void print_idrinfo(SaHpiIdrInfoT *idrInfo,int len);
static void print_idrinfo(SaHpiIdrInfoT *idrInfo,int len)
{
#ifdef OPENHPI_USED
   if (fverbose) {
	oh_print_idrinfo(idrInfo, len);
   } else 
#endif
   {  /* don't need to show this if not fverbose */
   }
}

static void print_idrareaheader(SaHpiIdrAreaHeaderT *areaHeader, int len);
static void print_idrareaheader(SaHpiIdrAreaHeaderT *areaHeader, int len)
{
   int i;
#ifdef OPENHPI_USED
   if (fverbose) {
	oh_print_idrareaheader(areaHeader, len);
   } else
#endif
   {
	for (i = 0; i < NAREATYP; i++) {
		if (map_areatype[i].type == areaHeader->Type) break;
	}
	if (i == NAREATYP) i--;
	printf("AreaId[%d] %s\n",areaHeader->AreaId,map_areatype[i].str);
   }
}

static void print_idrfield(SaHpiIdrFieldT *field, int len);
static void print_idrfield(SaHpiIdrFieldT *field, int len)
{
	SaHpiTextBufferT *strptr;
	char fieldstr[MAX_STRSIZE];
	int i;

#ifdef OPENHPI_USED
   if (fverbose) {
	oh_print_idrfield(field, len);
   } else 
#endif
   {
	for (i = 0; i < NFIELDTYP; i++) {
		if (map_fieldtype[i].type == field->Type) break;
	}
	if (i == NFIELDTYP) i--;
	strptr = &(field->Field);  
	fixstr(strptr,fieldstr);  /*stringify if needed*/
	printf("    FieldId[%d] %s : %s\n",
		field->FieldId,map_fieldtype[i].str ,fieldstr);
   }
}

int walkInventory(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
		  SaHpiIdrInfoT *idrInfo);
int walkInventory(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
		  SaHpiIdrInfoT 	*idrInfo)
{

   SaErrorT 	rv = SA_OK, 
   		rvField = SA_OK;

   SaHpiUint32T	numAreas;
   SaHpiUint32T	countAreas = 0;
   SaHpiUint32T	countFields = 0;

   SaHpiEntryIdT	areaId;
   SaHpiEntryIdT	nextareaId;
   SaHpiIdrAreaTypeT 	areaType;
   SaHpiIdrAreaHeaderT  areaHeader;

   SaHpiEntryIdT	fieldId;
   SaHpiEntryIdT	nextFieldId;
   SaHpiIdrFieldTypeT 	fieldType;
   SaHpiIdrFieldT  	thisField;

   numAreas = idrInfo->NumAreas;
   areaType = SAHPI_IDR_AREATYPE_UNSPECIFIED;
   areaId = SAHPI_FIRST_ENTRY; 
   foundasset = 0;
   while ((rv == SA_OK) && (areaId != SAHPI_LAST_ENTRY)) 
   {
	rv = saHpiIdrAreaHeaderGet(sessionid, resourceid,
				   idrInfo->IdrId, areaType,
				   areaId, &nextareaId,
				   &areaHeader);
	if (rv == SA_OK) {
		countAreas++;
		print_idrareaheader(&areaHeader, 8);

		fieldType = SAHPI_IDR_FIELDTYPE_UNSPECIFIED;
		fieldId = SAHPI_FIRST_ENTRY;
		countFields = 0;
	
		while ((rvField == SA_OK) && (fieldId != SAHPI_LAST_ENTRY))
		{
			rvField = saHpiIdrFieldGet( sessionid,	resourceid,
						idrInfo->IdrId,
						areaHeader.AreaId, fieldType,
						fieldId, &nextFieldId,
						&thisField);
			if (fdebug) 
				printf("saHpiIdrFieldGet[%x] rv = %d type=%d\n",
					idrInfo->IdrId,rvField,
					thisField.Type);
			if (rvField == SA_OK) {
				countFields++; 
				print_idrfield(&thisField, 12);
				if (thisField.Type == SAHPI_IDR_FIELDTYPE_ASSET_TAG) {
				   atag.rid   = resourceid;
				   atag.idrid = idrInfo->IdrId;
				   atag.areaid = areaHeader.AreaId;
				   atag.fieldid = fieldId;
				   foundasset = 1;
				}
			}
			fieldId = nextFieldId;
		}  /*while fields*/
			
		if ( countFields != areaHeader.NumFields) 
			printf("Area Header error: areaHeader.NumFields %d, countFields %d\n",
				areaHeader.NumFields, countFields);
	} else {
		printf("saHpiIdrAreaHeaderGet error %d\n",rv);
	}
	areaId = nextareaId;
	
   }   /*while areas*/

   if ((rv == SA_OK) && (countAreas != numAreas)) 
	printf("idrInfo error! idrInfo.NumAreas = %d; countAreas = %d\n", 
			numAreas, countAreas);

   if (countFields > 0) rv = 0;
   return(rv);
}  /*end walkInventory*/

int
main(int argc, char **argv)
{
  int c;
  SaErrorT rv,rv_rdr;
  SaHpiSessionIdT sessionid;
  SaHpiDomainInfoT rptinfo;
  SaHpiRptEntryT rptentry;
  SaHpiEntryIdT rptentryid;
  SaHpiEntryIdT nextrptentryid;
  SaHpiEntryIdT entryid;
  SaHpiEntryIdT nextentryid;
  SaHpiResourceIdT resourceid;
  SaHpiRdrT rdr;
  SaHpiIdrInfoT idrInfo;
  SaHpiIdrIdT 	idrid;
  int invfound = 0;
  int nloops = 0;

  oh_prog_version(argv[0], OH_SVN_REV);
  atag.tlen = 0;

  while ( (c = getopt( argc, argv,"a:vxz?")) != EOF )
  switch(c) {
    case 'z': fzdebug = 1; /* fall thru to include next setting */
    case 'x': fdebug = 1; break;
    case 'v': fverbose = 1; break;
    case 'a':
          fasset = 1;
          if (optarg) {
	    atag.tag  = (char *)strdup(optarg);
	    atag.tlen = strlen(optarg);
	  }
          break;
    default:
          printf("Usage: %s [-x] [-a asset_tag]\n", argv[0]);
          printf("   -a  Sets the asset tag\n");
          printf("   -x  Display debug messages\n");
          printf("   -z  Display extra debug messages\n");
          exit(1);
  }

	/* compile error */
//  inv = (SaHpiIdrAreaHeaderT *)&inbuff[0];
  inv = (SaHpiIdrAreaHeaderT *)(void *)&inbuff[0];
  rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID,&sessionid,NULL);
  if (fdebug) printf("saHpiSessionOpen rv = %d sessionid = %x\n",rv,sessionid);
  if (rv != SA_OK) {
    printf("saHpiSessionOpen error %d\n",rv);
    exit(-1);
  }
 
  rv = saHpiDomainInfoGet(sessionid,&rptinfo);
  if (fdebug) printf("saHpiDomainInfoGet rv = %d\n",rv);
  // if (fdebug) printf("RptInfo: UpdateCount = %x, UpdateTime = %lx\n",
  //      rptinfo.UpdateCount, (unsigned long)rptinfo.UpdateTimestamp);

  while (!invfound && (nloops < 7)) 
  {
    /*
     * The OpenHPI ipmi plugin has a bug whereby the FRU RDR is added 
     * much later, and always requires a rediscovery. (bug #1095256)
     * This should not apply to other well-behaved plugins.
     */
    nloops++;
    if (fdebug) printf("Starting Discovery, pass %d ...\n",nloops);
    rv = saHpiDiscover(sessionid);
    if (fdebug) printf("saHpiDiscover rv = %d\n",rv);
    if (rv != SA_OK) {
        printf("saHpiDiscover error %d\n",rv);
        break;
    }
 
  /* walk the RPT list */
  rptentryid = SAHPI_FIRST_ENTRY;
  while ((rv == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
  {
    rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
    if (rv != SA_OK) printf("RptEntryGet: rid=%d rv = %d\n",rptentryid,rv);
    if (rv == SA_OK)
    {
      /* walk the RDR list for this RPT entry */
      entryid = SAHPI_FIRST_ENTRY;
      /* OpenHPI plugin sometimes has bad RPT Tag DataLength here. */
      // rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
      resourceid = rptentry.ResourceId;
      if (fdebug) printf("rptentry[%d] resourceid=%d\n", rptentryid,resourceid);
      if (rptentry.ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)
      {
        printf("Resource[%d] Tag: %s \thas inventory capability\n", rptentryid,rptentry.ResourceTag.Data);
	rv_rdr = SA_OK;
	while ((rv_rdr == SA_OK) && (entryid != SAHPI_LAST_ENTRY))
	{
          rv_rdr = saHpiRdrGet(sessionid,resourceid, entryid,&nextentryid, &rdr);
  	  if (fdebug) printf("saHpiRdrGet[%x] rv = %d\n",entryid,rv_rdr);
	  if (rv_rdr == SA_OK)
	  {
  	    if (fdebug) printf("Rdr[%x] type = %d tag = %s\n",entryid,
				rdr.RdrType,rdr.IdString.Data);
	    if (rdr.RdrType == SAHPI_INVENTORY_RDR)
	    { 
	      invfound = 1;
	      /*type 3 includes inventory records*/
	      rdr.IdString.Data[rdr.IdString.DataLength] = 0;	    
	      idrid = rdr.RdrTypeUnion.InventoryRec.IdrId;
	      buffersize = sizeof(inbuff);
	      if (fdebug) {
		 printf("Rdr[%x] is inventory, IdrId=%x\n",rdr.RecordId,idrid);
		 printf("Inv BufferSize=%d\n", buffersize);
	      }
	      if ( IsTagBmc((char *)rdr.IdString.Data, rdr.IdString.DataLength) )
	      {
		/* Get all of the inventory data areas and fields */
		memset(inv,0,buffersize);
		rv_rdr = saHpiIdrInfoGet(sessionid, resourceid, idrid, &idrInfo);
		if (rv_rdr != SA_OK) {
		   printf("IDR Info error: rv_rdr = %d\n",rv_rdr);
		} else {  /* idrInfo is ok */
		   if (fdebug) printf("IDR Info: ok \n");
		   print_epath(&rptentry.ResourceEntity, 1);
	           printf("RDR[%x]: Inventory, IdrId=%x %s\n",rdr.RecordId,
			idrid,rdr.IdString.Data);
		   print_idrinfo(&idrInfo,4);
		   rv_rdr = walkInventory(sessionid, resourceid, &idrInfo);
		   if (fdebug) printf("walkInventory rv_rdr = %d\n",rv_rdr);
		}
		
		if (!ent_writable(&rptentry.ResourceEntity,&idrInfo))
			foundasset = 0;
		if ((fasset == 1) && (foundasset == 1)) {
			SaHpiIdrFieldT  atagField;
			atagField.Type = SAHPI_IDR_FIELDTYPE_ASSET_TAG;
			atagField.ReadOnly = SAHPI_FALSE;
			atagField.AreaId = atag.areaid;
			atagField.FieldId = atag.fieldid;
			strptr=&(atagField.Field);
			strptr->DataType = SAHPI_TL_TYPE_TEXT; 
			strptr->Language = SAHPI_LANG_ENGLISH;
			strptr->DataLength = (SaHpiUint8T)atag.tlen;
			strncpy((char *)strptr->Data, atag.tag, atag.tlen);
			strptr->Data[atag.tlen] = 0;
			printf( "Writing new asset tag: %s\n",strptr->Data);
		        rv_rdr = saHpiIdrFieldSet(sessionid, resourceid, 
						atag.idrid, &atagField);
			printf("Return Write Status = %d\n", rv_rdr);
			if (rv_rdr == SA_OK) {
			   printf ("Good write - re-reading!\n");
			   rv_rdr = walkInventory(sessionid, resourceid, &idrInfo);
			   if (fdebug) printf("walkInventory rv_rdr = %d\n",rv_rdr);
			} /* Good write - re-read */
		   }  /*endif fasset*/
  		}  /*endif RDR tag ok*/
	      } /* Inventory Data Records - Type 3 */ 
	      else if (fdebug) printf("rdr type = %d\n", rdr.RdrType);
	    }  /*endif RdrGet ok*/
	    entryid = nextentryid;
          } /*end while rdr*/
        } /*endif rpt invent capab*/
        else 
	  if (fdebug) printf("Resource[%d] Tag: %s\n", rptentryid,
				rptentry.ResourceTag.Data);
      }  /*endif rpt ok*/
      rptentryid = nextrptentryid;
  }  /*end rpt loop */
    if (fdebug) printf("loop %d inventory found = %d\n",nloops,invfound);
  }  /*end while no inv */
  rv = saHpiSessionClose(sessionid);
  exit(0);
}
 /* end hpifru.c */
