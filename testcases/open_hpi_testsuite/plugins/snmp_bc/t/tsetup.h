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

#ifndef TSETUP_H
#define TSETUP_H

SaErrorT tsetup (SaHpiSessionIdT *sessionid_ptr);

SaErrorT tfind_resource(SaHpiSessionIdT *sessionid_ptr,
                        SaHpiCapabilitiesT search_rdr_type,
                        SaHpiEntryIdT i_rptentryid,
                        SaHpiRptEntryT *rptentry, 
			SaHpiBoolT samecap);

SaErrorT tfind_resource_by_ep(SaHpiSessionIdT *sessionid_ptr,
			      SaHpiEntityPathT *ep,
			      SaHpiEntryIdT i_rptentryid,
			      SaHpiRptEntryT *rptentry);

SaErrorT tfind_rdr_by_name(SaHpiSessionIdT *sessionid_ptr,
			   SaHpiResourceIdT rid,
			   char *rdr_name,
			   SaHpiRdrT *rdr);
			
SaErrorT tcleanup(SaHpiSessionIdT *sessionid_ptr);

 
#define checkstatus(err, expected_err, testfail) 				\
do {										\
    if (err != expected_err) {							\
	printf("Error! Test fails: File=%s, Line=%d\n", __FILE__, __LINE__);	\
	printf("Returned err=%s, expected=%s\n",				\
		oh_lookup_error(err), oh_lookup_error(expected_err));		\
	testfail = -1;								\
    }										\
} while(0)

#define DECLARE_HANDLE()                 \
do {                                     \
        SaHpiSessionIdT sessionid;       \
        SaHpiDomainIdT did;              \
        struct oh_handler *h = NULL;     \
        struct oh_domain *d = NULL;      \
        unsigned int *hid = NULL;        \
	struct oh_handler_state *handle; \
} while(0)

#define INIT_HANDLE(did, d, hid, h, handle)                                \
do {                                                    \
	did = oh_get_session_domain(sessionid);      \
	d = oh_get_domain(did);                      \
        hid = oh_get_resource_data(&(d->rpt), id);   \
        h = oh_get_handler(*hid);                    \
	handle = (struct oh_handler_state *) h->hnd; \
} while(0)


#endif

