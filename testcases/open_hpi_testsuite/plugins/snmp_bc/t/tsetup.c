/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *     Peter D Phan <pdphan@users.sourceforge.net>
 */

#include <snmp_bc_plugin.h>
#include <sim_init.h>
#include <tsetup.h>

SaErrorT tsetup (SaHpiSessionIdT *sessionid_ptr)
{
        SaErrorT err;

        /********************************	 	 
	 * Hook in simulation environment
	 ********************************/
        err = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, sessionid_ptr, NULL);
        if (err != SA_OK) {
                printf("Error! Cannot open session.\n");
		printf("  File=%s, Line=%d\n", __FILE__, __LINE__);
                printf("  Received error=%s\n", oh_lookup_error(err));
        }
        
        if (!err) err = saHpiDiscover(*sessionid_ptr);
        if (err != SA_OK) {
                printf("Error! Cannot discover resources.\n");
		printf("   File=%s, Line=%d\n", __FILE__, __LINE__);
                printf("   Received error=%s\n", oh_lookup_error(err));
                err = saHpiSessionClose(*sessionid_ptr);
        }
        
        return err;	 
}

SaErrorT tfind_resource(SaHpiSessionIdT *sessionid_ptr,
                        SaHpiCapabilitiesT search_rdr_type,
                        SaHpiEntryIdT i_rptentryid,
                        SaHpiRptEntryT *rptentry, 
			SaHpiBoolT samecap)
{
	SaErrorT rvRptGet;
        SaHpiRptEntryT l_rptentry;
        SaHpiEntryIdT  rptentryid;
        SaHpiEntryIdT  nextrptentryid;
        SaHpiCapabilitiesT cap_mask;
  	     
        if (!sessionid_ptr) {
                printf("Error! Invalid test setup.\n");
		printf("   File=%s, Line=%d\n", __FILE__, __LINE__);
                return(SA_ERR_HPI_INVALID_PARAMS);
	} 
        
        cap_mask =	(SAHPI_CAPABILITY_RESOURCE 	   |
                         SAHPI_CAPABILITY_AGGREGATE_STATUS |
                         SAHPI_CAPABILITY_CONFIGURATION	   |
                         SAHPI_CAPABILITY_MANAGED_HOTSWAP  |
                         SAHPI_CAPABILITY_WATCHDOG	   |
                         SAHPI_CAPABILITY_CONTROL	   |
                         SAHPI_CAPABILITY_FRU		   |
                         SAHPI_CAPABILITY_ANNUNCIATOR	   |
                         SAHPI_CAPABILITY_POWER		   |
                         SAHPI_CAPABILITY_RESET		   |
                         SAHPI_CAPABILITY_INVENTORY_DATA   |
                         SAHPI_CAPABILITY_EVENT_LOG	   |
                         SAHPI_CAPABILITY_RDR		   |
                         SAHPI_CAPABILITY_SENSOR); 
        
        if ((search_rdr_type & cap_mask) == 0) {
                printf("Error! Invalid resource type.\n");
		printf("   File=%s, Line=%d\n", __FILE__, __LINE__);
                return(SA_ERR_HPI_INVALID_PARAMS);	
        }
	
        /***************	 	 
         * Find resource 
         ***************/
	rptentryid = SAHPI_FIRST_ENTRY;
	do {
		rvRptGet = saHpiRptEntryGet(*sessionid_ptr, rptentryid, &nextrptentryid, &l_rptentry);
		if (rvRptGet != SA_OK) {
			printf("Cannot get resource; Error=%s\n", oh_lookup_error(rvRptGet));
		}
		else {
		    	if (l_rptentry.ResourceFailed == SAHPI_FALSE) {
		 		if (samecap) {
					if ((l_rptentry.ResourceCapabilities & search_rdr_type)) 
					{
                				memcpy(rptentry,&l_rptentry, sizeof(SaHpiRptEntryT));	
			     			break;
					}
				} else {
					if (!(l_rptentry.ResourceCapabilities & search_rdr_type))
					{
                				memcpy(rptentry,&l_rptentry, sizeof(SaHpiRptEntryT));	
			     			break;
					}			
				}
			} else {
				printf("Resource %s is marked failed.\n", l_rptentry.ResourceTag.Data);
			} 
		}
		rptentryid = nextrptentryid;
	} while ((rvRptGet == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY));
		  
	if (rptentryid != SAHPI_LAST_ENTRY) return(SA_OK);
	return(SA_ERR_HPI_NOT_PRESENT);
}

SaErrorT tfind_resource_by_ep(SaHpiSessionIdT *sessionid_ptr,
			      SaHpiEntityPathT *ep,
			      SaHpiEntryIdT i_rptentryid,
			      SaHpiRptEntryT *rptentry) 
{
	SaErrorT err;
        SaHpiRptEntryT l_rptentry;
        SaHpiEntryIdT  rptentryid;
        SaHpiEntryIdT  nextrptentryid;
  	     
        if (!sessionid_ptr || !ep || !rptentry) {
                printf("Error! Invalid test setup.\n");
		printf("   File=%s, Line=%d\n", __FILE__, __LINE__);
                return(SA_ERR_HPI_INVALID_PARAMS);
	} 
	
        /***************	 	 
         * Find resource 
         ***************/
	rptentryid = SAHPI_FIRST_ENTRY;
	do {
		err = saHpiRptEntryGet(*sessionid_ptr, rptentryid, &nextrptentryid, &l_rptentry);
		if (err) {
			printf("Cannot get Resource; Error=%s\n", oh_lookup_error(err));
		}
		else {
		    	if (l_rptentry.ResourceFailed == SAHPI_FALSE) {
		 		if (oh_cmp_ep(ep, &(l_rptentry.ResourceEntity))) {
					memcpy(rptentry, &l_rptentry, sizeof(SaHpiRptEntryT));	
					break;
				}
			} else {
				printf("Resource %s is marked failed.\n", l_rptentry.ResourceTag.Data);
			} 
		}
		rptentryid = nextrptentryid;
	} while ((err == SA_OK) && (rptentryid != SAHPI_LAST_ENTRY));
		  
	if (rptentryid != SAHPI_LAST_ENTRY) return(SA_OK);
	return(SA_ERR_HPI_NOT_PRESENT);
}

SaErrorT tfind_rdr_by_name(SaHpiSessionIdT *sessionid_ptr,
			   SaHpiResourceIdT rid,
			   char *rdr_name,
			   SaHpiRdrT *rdr)
{
	SaErrorT       err;
        SaHpiEntryIdT  entryid, nextentryid;
	SaHpiRdrT      working_rdr;

        if (!sessionid_ptr || !rdr_name || !rdr) {
                printf("Error! Invalid test setup.\n");
		printf("   File=%s, Line=%d\n", __FILE__, __LINE__);
                return(SA_ERR_HPI_INVALID_PARAMS);
	} 

	/********** 
	 * Find RDR
	 **********/
	entryid = SAHPI_FIRST_ENTRY;
	do {
		err = saHpiRdrGet(*sessionid_ptr, rid, entryid, &nextentryid, &working_rdr);
		if (err) {
			printf("Cannot get RDR; Error=%s\n", oh_lookup_error(err));
		}
		else {
			if (strncmp((char *)working_rdr.IdString.Data, rdr_name, SAHPI_MAX_TEXT_BUFFER_LENGTH) == 0) {
				memcpy(rdr, &working_rdr, sizeof(SaHpiRdrT));
				break;
			}
			entryid = nextentryid;
		}
	} while ((err == SA_OK) && (entryid != SAHPI_LAST_ENTRY));

	if (entryid != SAHPI_LAST_ENTRY) return(SA_OK);
	return(SA_ERR_HPI_NOT_PRESENT);
}

SaErrorT tcleanup(SaHpiSessionIdT *sessionid_ptr)
{
	SaErrorT err = SA_OK;
	/***************************
	 * Close session, free memory
	 ***************************/
	 err = saHpiSessionClose(*sessionid_ptr);
	 return(err);
}
