/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Peter D. Phan <pdphan@users.sourceforge.net>
 *
 * Log: 
 *	Start from hpitree.c 
 *	This routine display highlevel topology for a managed openHPI complex
 *
 * Changes:
 *     
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <SaHpi.h> 
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.6 $"

/* 
 * Function prototypes
 */
static SaErrorT list_resources(SaHpiSessionIdT sessionid, SaHpiResourceIdT res_id);
static SaErrorT list_rpt(SaHpiRptEntryT *rptptr);
static SaErrorT list_rdr(SaHpiSessionIdT sessionid, 
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

static SaErrorT show_rpt(SaHpiRptEntryT *rptptr,SaHpiResourceIdT resourceid);

static SaErrorT show_rdr(oh_big_textbuffer *buffer);

static SaErrorT show_inv(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

static SaErrorT show_sens(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

static SaErrorT show_ann(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

static SaErrorT show_ctrl(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

static SaErrorT show_wdog(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid);

void same_system(oh_big_textbuffer *bigbuf);
void show_trailer(char *system);
						
#define all_resources 255
#define EPATHSTRING_END_DELIMITER "}"
#define rpt_startblock "    |\n    +--- "
#define rdr_startblock "    |    |__ "
/* 
 * Globals for this driver
 */
int fdebug = 0;
int f_rpt     = 0;
int f_inv     = 0;
int f_sensor     = 0;
int f_wdog     = 0;
int f_ann     = 0;
int f_ctrl     = 0;
int f_overview = 0;

char previous_system[SAHPI_MAX_TEXT_BUFFER_LENGTH] = "";
/* 
 * Main                
 */
int
main(int argc, char **argv)
{
	SaErrorT 	rv = SA_OK;
	
	SaHpiSessionIdT sessionid;
	SaHpiResourceIdT resourceid = all_resources;

	int c;
	    
	oh_prog_version(argv[0], OH_SVN_REV);
	while ( (c = getopt( argc, argv,"rsicawn:x?")) != EOF ) {
		switch(c) {
			case 'r': f_rpt     = 1; break;
			case 's': f_sensor = 1; break;
			case 'i': f_inv = 1; break;
			case 'c': f_ctrl = 1; break;
			case 'w': f_wdog = 1; break;
			case 'a': f_ann = 1; break;
			case 'n':
				if (optarg)
					resourceid = atoi(optarg);
				else 
					resourceid = all_resources;
				break;
			case 'x': fdebug = 1; break;
			default:
				printf("\n\tUsage: %s [-option]\n\n", argv[0]);
				printf("\t      (No Option) Display system topology: rpt & rdr headers\n");	
				printf("\t           -r     Display only rpts\n");
				printf("\t           -s     Display only sensors\n");
				printf("\t           -c     Display only controls\n");
				printf("\t           -w     Display only watchdogs\n");
				printf("\t           -i     Display only inventories\n");
				printf("\t           -a     Display only annunciators\n");												
				printf("\t           -x     Display debug messages\n");
				printf("\n\n\n\n");
				exit(1);
		}
	}
 
	if (argc == 1)  f_overview = 1;
	memset (previous_system, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH);

	if (fdebug) printf("saHpiSessionOpen\n");
        rv = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID,&sessionid,NULL);
	if (rv != SA_OK) {
		printf("saHpiSessionOpen returns %s\n",oh_lookup_error(rv));
		exit(-1);
	}
	if (fdebug)
	       	printf("saHpiSessionOpen returns with SessionId %d\n", sessionid);

	/*
	 * Resource discovery
	 */
	if (fdebug) printf("saHpiDiscover\n");
	rv = saHpiDiscover(sessionid);
	if (rv != SA_OK) {
		printf("saHpiDiscover returns %s\n",oh_lookup_error(rv));
		exit(-1);
	}

	if (fdebug)  printf("Discovery done\n");
	list_resources(sessionid, resourceid);

	rv = saHpiSessionClose(sessionid);
	
	exit(0);
}


/* 
 *
 */
static 
SaErrorT list_resources(SaHpiSessionIdT sessionid,SaHpiResourceIdT resourceid)
{
	SaErrorT rv       = SA_OK,
	         rvRdrGet = SA_OK,
 		 rvRptGet = SA_OK; 

	SaHpiRptEntryT rptentry;
	SaHpiEntryIdT rptentryid;
	SaHpiEntryIdT nextrptentryid;
	SaHpiEntryIdT entryid;
	SaHpiEntryIdT nextentryid;
	SaHpiRdrT rdr;
	SaHpiResourceIdT l_resourceid;
	SaHpiTextBufferT working;
		
	oh_init_textbuffer(&working);																		
																
	/* walk the RPT list */
	rptentryid = SAHPI_FIRST_ENTRY;
	do {
		
		if (fdebug) printf("saHpiRptEntryGet\n");
		rvRptGet = saHpiRptEntryGet(sessionid,rptentryid,&nextrptentryid,&rptentry);
		if ((rvRptGet != SA_OK) || fdebug) 
		       	printf("RptEntryGet returns %s\n",oh_lookup_error(rvRptGet));
		
		rv = show_rpt(&rptentry, resourceid);
						
		if (rvRptGet == SA_OK 
                   	&& (rptentry.ResourceCapabilities & SAHPI_CAPABILITY_RDR) 
			&& ((resourceid == 0xFF) || (resourceid == rptentry.ResourceId)))
		{
			l_resourceid = rptentry.ResourceId;
			if (resourceid != 0xFF) 
				 nextrptentryid = SAHPI_LAST_ENTRY;

			/* walk the RDR list for this RPT entry */
			entryid = SAHPI_FIRST_ENTRY;			

			if (fdebug) printf("rptentry[%d] resourceid=%d\n", entryid,resourceid);

			do {
				rvRdrGet = saHpiRdrGet(sessionid,l_resourceid, entryid,&nextentryid, &rdr);
				if (fdebug) printf("saHpiRdrGet[%d] rv = %s\n",entryid,oh_lookup_error(rvRdrGet));


				if (rvRdrGet == SA_OK)
				{
					// Add zero terminator to RDR Id String
					SaHpiUint32T last = rdr.IdString.DataLength;
					if ( last >= SAHPI_MAX_TEXT_BUFFER_LENGTH ) {
						last = SAHPI_MAX_TEXT_BUFFER_LENGTH - 1;
					}
					rdr.IdString.Data[last] = '\0';

					if (f_overview) list_rdr(sessionid, &rptentry, &rdr, l_resourceid);
					if (f_inv) show_inv(sessionid, &rptentry, &rdr, l_resourceid); 
					if (f_sensor) show_sens(sessionid, &rptentry, &rdr, l_resourceid); 
					if (f_ctrl) show_ctrl(sessionid, &rptentry, &rdr, l_resourceid); 
					if (f_wdog) show_wdog(sessionid, &rptentry, &rdr, l_resourceid); 
					if (f_ann) show_ann(sessionid, &rptentry, &rdr, l_resourceid); 						
				}
				entryid = nextentryid;
			} while ((rvRdrGet == SA_OK) && (entryid != SAHPI_LAST_ENTRY)) ;
		}
		rptentryid = nextrptentryid;
	} while ((rvRptGet == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY));
	
	show_trailer(previous_system);
	
	return(rv);
}


/*
 *
 */
void same_system(oh_big_textbuffer *bigbuf)
{

	int size = strcspn((char *)bigbuf->Data, EPATHSTRING_END_DELIMITER);
	int old_size = strcspn(previous_system, EPATHSTRING_END_DELIMITER); 
	if  ( (old_size != size) || 
			(strncmp((char *)bigbuf->Data, previous_system, size+1) != 0)) {
		if (previous_system[0] == '{') show_trailer(previous_system);
		memset (previous_system, 0, SAHPI_MAX_TEXT_BUFFER_LENGTH); 
		strncpy (previous_system, (char *)(bigbuf->Data), size+1);
		previous_system[size+2] = '\0';
		printf("\n\n%s\n", previous_system);
	} 

}


/*
 *
 */
void show_trailer(char *system)
{
	printf("    |\nEnd of %s\n\n", system);

}
/*
 *
 */
static 
SaErrorT show_rpt(SaHpiRptEntryT *rptptr,SaHpiResourceIdT resourceid)
{
	SaErrorT rv = SA_OK, err = SA_OK;
	oh_big_textbuffer bigbuf1, bigbuf2;
	SaHpiTextBufferT  textbuffer;
	
	
	err = oh_init_textbuffer(&textbuffer);
	if (err) return(err);
	err = oh_init_bigtext(&bigbuf1);
	if (err) return(err);
	err = oh_init_bigtext(&bigbuf2);
	if (err) return(err);

		
	if ((resourceid == all_resources) ||
		(resourceid == rptptr->ResourceId)) {

		rv  = oh_decode_entitypath(&rptptr->ResourceEntity, &bigbuf2);
		same_system(&bigbuf2);
		/* printf("ResourceId: %d", rptptr->ResourceId);  */
		err = oh_append_bigtext(&bigbuf1, rpt_startblock);
		
		err = oh_append_bigtext(&bigbuf1, (char *)bigbuf2.Data);
		if (err) return(err);
		err = oh_append_bigtext(&bigbuf1, "\n");
		if (err) return(err);
		
		printf("%s", bigbuf1.Data);
										
		if (f_rpt) list_rpt(rptptr);
	}
	return(rv);
}


/*
 *
 */
static 
SaErrorT list_rpt(SaHpiRptEntryT *rptptr)
{
	SaErrorT rv = SA_OK, err = SA_OK;
	oh_big_textbuffer bigbuf1, bigbuf2;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	SaHpiTextBufferT  textbuffer;
	
	
	err = oh_init_textbuffer(&textbuffer);				
	if (err) return(err);
	err = oh_init_bigtext(&bigbuf1);
	if (err) return(err);
	err = oh_init_bigtext(&bigbuf2);
	if (err) return(err);

	snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "    |    ResourceId:  %d\n", rptptr->ResourceId);
	oh_append_bigtext(&bigbuf1, str);
	
	oh_decode_capabilities(rptptr->ResourceCapabilities, &textbuffer);
	snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "    |    Capability:  %s\n",(char *)textbuffer.Data);
	oh_append_bigtext(&bigbuf1, str);

	snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "    |    Tag:  %s\n",rptptr->ResourceTag.Data);
	oh_append_bigtext(&bigbuf1, str);
	
	printf("%s", bigbuf1.Data);

	return(rv);
}

/* 
 *
 */
static 
SaErrorT show_inv(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)
{
 	SaErrorT  rvInvent = SA_OK;
	SaHpiIdrInfoT	idrInfo;
	SaHpiIdrIdT	idrid;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	oh_big_textbuffer buffer;
	rvInvent = oh_init_bigtext(&buffer);
									

	if (rdrptr->RdrType == SAHPI_INVENTORY_RDR) {
		idrid = rdrptr->RdrTypeUnion.InventoryRec.IdrId;
		rvInvent = saHpiIdrInfoGet(
					sessionid,
					l_resourceid,
					idrid,
					&idrInfo);		

		if (rvInvent !=SA_OK) {
			snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Inventory: ResourceId %d IdrId %d, error %s\n",
					l_resourceid, idrid, oh_lookup_error(rvInvent));
			oh_append_bigtext(&buffer, str);
			show_rdr(&buffer);			
		} else {

			snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Inventory Idr Num: %d, ", rdrptr->RdrTypeUnion.InventoryRec.IdrId);
			oh_append_bigtext(&buffer, str);
			snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Num Areas: %d, ", idrInfo.NumAreas);
			oh_append_bigtext(&buffer, str);
			snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Tag: %s\n", rdrptr->IdString.Data);
			oh_append_bigtext(&buffer, str);
			show_rdr(&buffer);
		}
	}
	return(rvInvent);

}

/* 
 *
 */
static 
SaErrorT show_sens(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)
{
	SaErrorT rv  = SA_OK;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	oh_big_textbuffer buffer;
	rv = oh_init_bigtext(&buffer);

	if (rdrptr->RdrType == SAHPI_SENSOR_RDR) {							
		/* Sensor Num */
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Sensor Num: %d, ", rdrptr->RdrTypeUnion.SensorRec.Num);
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s, ", 
		 	oh_lookup_sensortype(rdrptr->RdrTypeUnion.SensorRec.Type));
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Category: %s, ", 
		 	oh_lookup_eventcategory(rdrptr->RdrTypeUnion.SensorRec.Category));
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Tag: %s\n", 
		 	rdrptr->IdString.Data);
		oh_append_bigtext(&buffer, str);
		show_rdr(&buffer);
	} 
	
	return(rv);
}


/* 
 *
 */
static 
SaErrorT show_rdr(oh_big_textbuffer *buffer)
{
	
	SaErrorT rv = SA_OK;
	oh_big_textbuffer buffer1;
	rv = oh_init_bigtext(&buffer1);

	oh_append_bigtext(&buffer1, rdr_startblock);
	oh_append_bigtext(&buffer1, (char *) buffer->Data);		
	printf("%s", buffer1.Data);

	return(rv);

}
/* 
 *
 */
static 
SaErrorT list_rdr(SaHpiSessionIdT sessionid, 
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)
{
	SaErrorT rv = SA_OK;
	show_inv(sessionid, rptptr, rdrptr, l_resourceid); 
	show_sens(sessionid, rptptr, rdrptr, l_resourceid); 
	show_ctrl(sessionid, rptptr, rdrptr,  l_resourceid); 
	show_wdog(sessionid, rptptr, rdrptr,  l_resourceid); 
	show_ann(sessionid, rptptr, rdrptr,  l_resourceid); 
	return(rv);
}


/* 
 *
 */
static 
SaErrorT show_ctrl(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)
{
	SaErrorT rv       = SA_OK;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	oh_big_textbuffer buffer;
	rv = oh_init_bigtext(&buffer);
																
	if (rdrptr->RdrType == SAHPI_CTRL_RDR){
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Control Num: %d, ", rdrptr->RdrTypeUnion.CtrlRec.Num);
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s, ", 
			oh_lookup_ctrltype(rdrptr->RdrTypeUnion.CtrlRec.Type));
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Output Type: %s, ", 
			oh_lookup_ctrloutputtype(rdrptr->RdrTypeUnion.CtrlRec.OutputType));
		oh_append_bigtext(&buffer, str);						
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Tag: %s\n", 
		 	rdrptr->IdString.Data);
		oh_append_bigtext(&buffer, str);
		show_rdr(&buffer);
	} 
	return(rv);
}


/* 
 *
 */
static  
SaErrorT show_wdog(SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)
{
	SaErrorT rv       = SA_OK;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	oh_big_textbuffer buffer;
	rv = oh_init_bigtext(&buffer);						
																
	if (rdrptr->RdrType == SAHPI_WATCHDOG_RDR) {
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Watchdog Num: %d, ", rdrptr->RdrTypeUnion.WatchdogRec.WatchdogNum);
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Tag: %s\n", 
		 	rdrptr->IdString.Data);
		oh_append_bigtext(&buffer, str);
		show_rdr(&buffer);
		
	} 
	return(rv);
}




/* 
 *
 */
static  
SaErrorT show_ann (SaHpiSessionIdT sessionid,
			SaHpiRptEntryT *rptptr,	
			SaHpiRdrT *rdrptr, 
			SaHpiResourceIdT l_resourceid)

{
	SaErrorT rv       = SA_OK;
	char str[SAHPI_MAX_TEXT_BUFFER_LENGTH];
	oh_big_textbuffer buffer;
	rv = oh_init_bigtext(&buffer);						
																
	if (rdrptr->RdrType == SAHPI_ANNUNCIATOR_RDR) {
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Annunciator Num: %d, ",
					 rdrptr->RdrTypeUnion.AnnunciatorRec.AnnunciatorNum);
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Type: %s, ",
			oh_lookup_annunciatortype(rdrptr->RdrTypeUnion.AnnunciatorRec.AnnunciatorType));
		oh_append_bigtext(&buffer, str);
		snprintf(str, SAHPI_MAX_TEXT_BUFFER_LENGTH, "Tag: %s\n", 
		 	rdrptr->IdString.Data);
		oh_append_bigtext(&buffer, str);
		show_rdr(&buffer);
		
	} 
	return(rv);
}

 /* end hpitop.c */
