/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
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
 *     Peter D. Phan <pdphanusers.sourceforge.net>
 *
 * Log: 
 *     Copied from hpifru.c and modified for general use
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "SaHpi.h"

#define NCT 25

char progver[] = "0.a";
char *chasstypes[NCT] = {
	"Not Defined", "Other", "Unknown", "Desktop", "Low Profile Desktop",
	"Pizza Box", "Mini Tower", "Tower", "Portable", "Laptop",
	"Notebook", "Handheld", "Docking Station", "All in One", "Subnotebook",
	"Space Saving", "Lunch Box", "Main Server", "Expansion", "SubChassis",
	"Buss Expansion Chassis", "Peripheral Chassis", "RAID Chassis", 
	"Rack-Mount Chassis", "New"
};
int fasset = 0;
int fdebug = 0;
int fxdebug = 0;
int i,j,k = 0;
SaHpiUint32T buffersize;
SaHpiUint32T actualsize;
char progname[] = "hpi_invent";
char *asset_tag;
char inbuff[10240];
char outbuff[256];
SaHpiInventoryDataT *inv;
SaHpiInventChassisTypeT chasstype;
SaHpiInventGeneralDataT *dataptr;
SaHpiTextBufferT *strptr;

static void
fixstr(SaHpiTextBufferT *strptr)
{ 
        if ( strptr == 0 )
        {
                outbuff[0] = 0;
                return;
        }
        
	size_t datalen;
	if ((datalen=strptr->DataLength) != 0)
		strncpy ((char *)outbuff, (char *)strptr->Data, datalen);
	outbuff[datalen] = 0;
}

static void
prtchassinfo(void)
{
	chasstype = (SaHpiInventChassisTypeT)inv->DataRecords[i]->RecordData.ChassisInfo.Type;
	for (k=0; k<NCT; k++) {
		if ((unsigned int)k == chasstype)
			printf( "\tChassis Type        : %s\n", chasstypes[k]);
	}	  

	dataptr = (SaHpiInventGeneralDataT *)&inv->DataRecords[i]->RecordData.ChassisInfo.GeneralData;
	strptr=dataptr->Manufacturer;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Manufacturer: %s\n", outbuff);

	strptr=dataptr->ProductName;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Name        : %s\n", outbuff);

	strptr=dataptr->ProductVersion;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Version     : %s\n", outbuff);

	strptr=dataptr->ModelNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Model Number: %s\n", outbuff);

	strptr=dataptr->SerialNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Serial #    : %s\n", outbuff);

	strptr=dataptr->PartNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Part Number : %s\n", outbuff);

	strptr=dataptr->FileId;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis FRU File ID : %s\n", outbuff);

	strptr=dataptr->AssetTag;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tChassis Asset Tag   : %s\n", outbuff);
	if (dataptr->CustomField[0] != 0)
	{
		if (dataptr->CustomField[0]->DataLength != 0)
		strncpy ((char *)outbuff, (char *)dataptr->CustomField[0]->Data,
						dataptr->CustomField[0]->DataLength);
		outbuff[dataptr->CustomField[0]->DataLength] = 0;
		printf( "\tChassis OEM Field   : %s\n", outbuff);
	}
}

static void
prtprodtinfo(void)
{
	int j;
	dataptr = (SaHpiInventGeneralDataT *)&inv->DataRecords[i]->RecordData.ProductInfo;
	strptr=dataptr->Manufacturer;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Manufacturer: %s\n", outbuff);

	strptr=dataptr->ProductName;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Name        : %s\n", outbuff);

	strptr=dataptr->ProductVersion;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Version     : %s\n", outbuff);

	strptr=dataptr->ModelNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Model Number: %s\n", outbuff);

	strptr=dataptr->SerialNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Serial #    : %s\n", outbuff);

	strptr=dataptr->PartNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Part Number : %s\n", outbuff);

	strptr=dataptr->FileId;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct FRU File ID : %s\n", outbuff);

	strptr=dataptr->AssetTag;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tProduct Asset Tag   : %s\n", outbuff);

	for (j = 0; j < 10; j++) {
		int ii;
		if (dataptr->CustomField[j] != NULL) {
			if ((dataptr->CustomField[j]->DataType == 0) &&
				(dataptr->CustomField[j]->DataLength == 16)) { /*binary GUID*/
				printf( "IPMI SystemGUID     : ");
				for (ii=0; ii< dataptr->CustomField[j]->DataLength; ii++)
					printf("%02x", dataptr->CustomField[j]->Data[ii]);
				printf("\n");
			} else {  /* other text field */
				dataptr->CustomField[j]->Data[
				dataptr->CustomField[j]->DataLength] = 0;
				printf( "\tProduct OEM Field   : %s\n",
				dataptr->CustomField[j]->Data);
			}
		} else /* NULL pointer */
			break;
	}/*end for*/
}

static void
prtboardinfo(void)
{
	dataptr = (SaHpiInventGeneralDataT *)&inv->DataRecords[i]->RecordData.BoardInfo;
	strptr=dataptr->Manufacturer;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Manufacturer  : %s\n", outbuff);

	strptr=dataptr->ProductName;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Product Name  : %s\n", outbuff);

	strptr=dataptr->ModelNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Model Number  : %s\n", outbuff);

	strptr=dataptr->PartNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Part Number   : %s\n", outbuff);

	strptr=dataptr->ProductVersion;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Version       : %s\n", outbuff);

	strptr=dataptr->SerialNumber;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Serial #      : %s\n", outbuff);

	strptr=dataptr->FileId;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard FRU File ID   : %s\n", outbuff);

	strptr=dataptr->AssetTag;
	fixstr((SaHpiTextBufferT *)strptr);
	printf( "\tBoard Asset Tag     : %s\n", outbuff);

	for (j = 0; j < 10 && dataptr->CustomField[j] ; j++)
		if (dataptr->CustomField[j] != NULL
                    && dataptr->CustomField[0]->DataLength != 0) {
                        strncpy ((char *)outbuff, (char *)dataptr->CustomField[j]->Data,
							dataptr->CustomField[j]->DataLength);
                        outbuff[dataptr->CustomField[j]->DataLength] = 0;
                        printf( "\tBoard OEM Field     : %s\n", outbuff);
                }
}

int
main(int argc, char **argv)
{
	int prodrecindx=0;
	int asset_len=0;
	char c;
	SaErrorT rv;
	SaErrorT rvx;
	SaErrorT rvxx;
	SaHpiVersionT hpiVer;
	SaHpiSessionIdT sessionid;
	SaHpiRptInfoT rptinfo;
	SaHpiRptEntryT rptentry;
	SaHpiEntryIdT rptentryid;
	SaHpiEntryIdT nextrptentryid;
	SaHpiEntryIdT entryid;
	SaHpiEntryIdT nextentryid;
	SaHpiResourceIdT resourceid;
	SaHpiRdrT rdr;
	SaHpiEirIdT eirid;

	printf("%s ver %s\n",argv[0],progver);

	while ( (c = getopt( argc, argv,"a:xz?")) != EOF ) {
		switch(c) {
			case 'z': fxdebug = 1; /* fall thru to include next setting */
			case 'x': fdebug = 1; break;
			case 'a':
				fasset = 1;
				if (optarg) 
				{
					asset_tag = (char *)strdup(optarg);
					asset_len = strlen(optarg);
				}
				/*
				printf( "String Length = %d\n", asset_len);
				printf( "String Length = %d\n", strlen(optarg));
				*/
				break;
			default:
				printf("Usage: %s [-x] [-a asset_tag]\n", progname);
				printf("   -a  Sets the asset tag\n");
				printf("   -x  Display debug messages\n");
				printf("   -z  Display extra debug messages\n");
				exit(1);
		}
	}

	inv = (SaHpiInventoryDataT *)&inbuff[0];
	rv = saHpiInitialize(&hpiVer);
	if (rv != SA_OK) {
		printf("saHpiInitialize error %d\n",rv);
		exit(-1);
	}
	
	rv = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID,&sessionid,NULL);
	if (rv != SA_OK) {
		printf("saHpiSessionOpen error %d\n",rv);
		exit(-1);
	}
 
	rv = saHpiResourcesDiscover(sessionid);
	if (fxdebug) printf("saHpiResourcesDiscover rv = %d\n",rv);

	rv = saHpiRptInfoGet(sessionid,&rptinfo);
	if (fxdebug) printf("saHpiRptInfoGet rv = %d\n",rv);
	if (fdebug) printf("RptInfo: UpdateCount = %d, UpdateTime = %lx\n",
			rptinfo.UpdateCount, (unsigned long)rptinfo.UpdateTimestamp);
 
	/* walk the RPT list */
	rptentryid = SAHPI_FIRST_ENTRY;
	rvx = SA_OK;
	while ((rvx == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY))
	{
		rv = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
		if (rvx != SA_OK) printf("RptEntryGet: rv = %d\n",rv);
		if (rvx == SA_OK)
		{
			/* walk the RDR list for this RPT entry */
			entryid = SAHPI_FIRST_ENTRY;
			rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;
			resourceid = rptentry.ResourceId;

			if (fdebug) printf("rptentry[%d] resourceid=%d\n", entryid,resourceid);
			printf("Resource Tag: %s\n", rptentry.ResourceTag.Data);

			rvxx = SA_OK;
			while ((rvxx == SA_OK) && (entryid != SAHPI_LAST_ENTRY))
			{
				rv = saHpiRdrGet(sessionid,resourceid, entryid,&nextentryid, &rdr);
				if (fxdebug) printf("saHpiRdrGet[%d] rv = %d\n",entryid,rv);
				if (rvxx == SA_OK)
				{
					if (rdr.RdrType == SAHPI_INVENTORY_RDR)
					{ 
						/*type 3 includes inventory records*/
						eirid = rdr.RdrTypeUnion.InventoryRec.EirId;
						rdr.IdString.Data[rdr.IdString.DataLength] = 0;	    

						if (fdebug) printf( "RDR[%d]: type=%d num=%d %s\n",
								rdr.RecordId,
								rdr.RdrType, eirid, rdr.IdString.Data);

						buffersize = sizeof(inbuff);
						if (fdebug) printf("BufferSize=%d InvenDataRecSize=%zd\n",
										buffersize, sizeof(inbuff));
						rv = saHpiEntityInventoryDataRead( sessionid, resourceid,
									eirid, buffersize, inv, &actualsize);

						if (fxdebug) printf(
							"saHpiEntityInventoryDataRead[%d] rv = %d\n", eirid, rv);
						if (fdebug) printf("ActualSize=%d\n", actualsize);

						if (rv == SA_OK)
	      					{
	 						/* Walk thru the list of inventory data */
		 					if (inv->Validity == SAHPI_INVENT_DATA_VALID) 
		  					{
                                                                for( i = 0; inv->DataRecords[i] != NULL; i++ )
                                                                {
                                                                        if (fdebug) printf( "Record = %d Index = %d type=%x len=%d\n", i, 
                                                                                            i, inv->DataRecords[i]->RecordType, 
                                                                                            inv->DataRecords[i]->DataLength);
                                                                        switch (inv->DataRecords[i]->RecordType)
                                                                        {
									case SAHPI_INVENT_RECTYPE_INTERNAL_USE:
										if (fdebug) printf( "Internal Use\n");
										break;
									case SAHPI_INVENT_RECTYPE_PRODUCT_INFO:
										if (fdebug) printf( "Product Info\n");
										prodrecindx = 0;
										prtprodtinfo();
										break;
									case SAHPI_INVENT_RECTYPE_CHASSIS_INFO:
										if (fdebug) printf( "Chassis Info\n");
										prtchassinfo();
										break;
									case SAHPI_INVENT_RECTYPE_BOARD_INFO:
										if (fdebug) printf( "Board Info\n");
										prtboardinfo();
										break;
									case SAHPI_INVENT_RECTYPE_OEM:
										if (fdebug) printf( "OEM Record\n");
										break;
									default:
										printf(" Invalid Invent Rec Type =%x\n",  
                                                                                       inv->DataRecords[i]->RecordType);
										break;
                                                                        }
                                                                }
                                                        }
						} else { printf(" InventoryDataRead returns HPI Error: rv=%d\n", rv); }
					} 
				} /* Inventory Data Records - Type 3 */
				entryid = nextentryid;
			}
			rptentryid = nextrptentryid;
		}
	}
	rv = saHpiSessionClose(sessionid);
	rv = saHpiFinalize();
	exit(0);
}
 /* end hpi_invent.c */
