/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Racing Guo <racing.guo@intel.com>
 *     Aaron  Chen <yukun.chen@intel.com>
 * Changes:
 *	11.30.2004 - Kouzmich: porting to HPI-B
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <SaHpi.h>
#include <oh_utils.h>
#include "hpi_cmd.h"

static pthread_t	ge_thread;
static pthread_t	prog_thread;
int			prt_flag = 0;
int			show_event_short = 0;
static int		in_progress = 0;
static GCond		*thread_wait = NULL;
static GMutex		*thread_mutex = NULL;
static char		*progress_mes;
Domain_t		*Domain;	// curreny domain
GSList			*domainlist;	// domain list


#define PROGRESS_BUF_SIZE	80

/* Progress bar implementation */
static void* progress_bar(void *unused)
{
	GTimeVal	time;
	char		buf[PROGRESS_BUF_SIZE], A[20];
	int		i = 0, t = 0, len, mes_len;

	memset(buf, 0, PROGRESS_BUF_SIZE);
	mes_len = strlen(progress_mes);
	while (in_progress) {
		snprintf(A, 10, " %d.%d s ", t / 10, t % 10);
		len = strlen(A);
		memset(buf + mes_len, '.', i);
		strncpy(buf, progress_mes, mes_len);
		if (i > 8)
			strncpy(buf + mes_len + (i - len) / 2, A, len);
		printf("%s\r", buf);
		fflush(stdout);
		g_get_current_time(&time);
		g_time_val_add(&time, G_USEC_PER_SEC / 10);
		g_cond_timed_wait(thread_wait, thread_mutex, &time);
		if (i < (PROGRESS_BUF_SIZE - mes_len - 1)) i++;
		t++;
	};
        g_thread_exit(0);
	return (void *)1;
}

/* This function creates thread for progress bar.
 *	mes - progress bar title.
 */
void do_progress(char *mes)
{
	progress_mes = mes;
	in_progress = 1;
	pthread_create(&prog_thread, NULL, progress_bar, NULL);
}

/* This function deletes thread for progress bar. */
void delete_progress()
{
	char	buf[PROGRESS_BUF_SIZE];
	
	in_progress = 0;
	memset(buf, ' ', PROGRESS_BUF_SIZE);
	buf[PROGRESS_BUF_SIZE - 1] = 0;
	printf("%s\n", buf);
}

static void* get_event(void *unused)
{
	SaHpiEventT	event;
	SaErrorT	rv;        
        SaHpiRptEntryT rptentry;
        SaHpiRdrT rdr;
        SaHpiEntryIdT rptentryid;
        SaHpiEntryIdT nextrptentryid;

	rv = saHpiSubscribe(Domain->sessionId);
	if (rv != SA_OK) {
		printf("OpenHPI>Fail to Subscribe event\n");
		return (void *)0;
	}	
	
	while(1) {
		for(;;) {
                        rdr.RdrType = SAHPI_NO_RECORD;
                        rptentry.ResourceId = 0;
			memset(&event, 0xF, sizeof(event));
			rv = saHpiEventGet(Domain->sessionId,
				SAHPI_TIMEOUT_BLOCK, &event,
				&rdr, &rptentry, NULL);		
			if (rv != SA_OK ) {
				printf("saHpiEventGet failed with error <%d>\n", rv);
				break;
			}
			if (prt_flag == 1) {
				if (show_event_short)
					show_short_event(&event, ui_print);
                                else if (rdr.RdrType != SAHPI_NO_RECORD)
                                        oh_print_event(&event, &rdr.Entity, 4);
                                else if (rptentry.ResourceId != 0)
                                        oh_print_event(&event, &rptentry.ResourceEntity, 4);
                                else {
                                        rptentryid = event.Source;
                                        rv = saHpiRptEntryGet(Domain->sessionId,
                                                              rptentryid,
                                                              &nextrptentryid, &rptentry);
                                        if(rv == SA_OK)
                                                oh_print_event(&event, &rptentry.ResourceEntity, 4);
                                        else {
                                                oh_print_event(&event, NULL, 4);
                                        }
                                }
			}
		}/*the loop for retrieving event*/
		sleep(1);
	}
	return (void *)1;
}

void set_Subscribe(Domain_t *domain, int as)
//  as = 1  - Subscribe
//  as = 0  - UnSubscribe
//   if domain == NULL - for all opened domains
{
	int		i, n;
	gpointer	ptr;
	Domain_t	*dmn;

	if ((domain != (Domain_t *)NULL) && domain->session_opened) {
		if (as) saHpiSubscribe(domain->sessionId);
		else saHpiUnsubscribe(domain->sessionId);
		return;
	};
	n = g_slist_length(domainlist);
	for (i = 0; i < n; i++) {
		ptr = g_slist_nth_data(domainlist, i);
		if (ptr == (gpointer)NULL) return;
		dmn = (Domain_t *)ptr;
		if (dmn->session_opened) {
			if (as) saHpiSubscribe(dmn->sessionId);
			else saHpiUnsubscribe(dmn->sessionId);
		}
	}
}

SaErrorT get_sessionId(Domain_t *domain)
{
	SaErrorT		rv;
	SaHpiDomainInfoT	info;

	if (domain->session_opened) return(SA_OK);
	rv = saHpiSessionOpen(domain->domainId, &(domain->sessionId), NULL);
	if (rv != SA_OK) {
		printf("saHpiSessionOpen error %s\n", oh_lookup_error(rv));
		return rv;
	};
	domain->session_opened = 1;
	rv = saHpiDomainInfoGet(domain->sessionId, &info);
	if (rv != SA_OK) {
		printf("ERROR!!! saHpiDomainInfoGet: %s\n", oh_lookup_error(rv));
		return(rv);
	};
	domain->domainId = info.DomainId;
	return(SA_OK);
}

SaErrorT do_discover(Domain_t *domain)
{
	SaErrorT rv;

	if (!domain->sessionId) {
		rv = get_sessionId(domain);
		if (rv != SA_OK) return(-1);
	};
	if (domain->discovered) return(SA_OK);
	do_progress("Discover");
	rv = saHpiDiscover(domain->sessionId);
	if (rv != SA_OK) {
		delete_progress();
		printf("saHpiDiscover error %s\n", oh_lookup_error(rv));
		return rv;
	};
	delete_progress();
	domain->discovered = 1;
	printf("Discovery done\n");
	return(SA_OK);
}

int add_domain(Domain_t *domain)
{
	SaErrorT	rv;
	GSList		*ptr;

	rv = do_discover(domain);
	if (rv != SA_OK) return(-1);
	ptr = g_slist_find(domainlist, domain);
	if (ptr == (GSList *)NULL)
		domainlist = g_slist_append(domainlist, domain);
	return(0);
}

int open_session(int eflag)
{
	Domain_t	*par_domain;

        if (!g_thread_supported()) {
                g_thread_init(NULL);
	};
	thread_wait = g_cond_new();
	thread_mutex = g_mutex_new();
	par_domain = (Domain_t *)malloc(sizeof(Domain_t));
	memset(par_domain, 0, sizeof(Domain_t));
	par_domain->domainId = SAHPI_UNSPECIFIED_DOMAIN_ID;
	if (get_sessionId(par_domain) != SA_OK) return(-1);
	// set current domain
	Domain = par_domain;

	if (eflag) {
		show_event_short = 1;
		prt_flag = 1;
		pthread_create(&ge_thread, NULL, get_event, NULL);
	};
	// add main domain to the domain list
	if (add_domain(par_domain) != SA_OK) return(-1);


	printf("\tEnter a command or \"help\" for list of commands\n");

	if (! eflag)
		pthread_create(&ge_thread, NULL, get_event, NULL);
	return 0;
}

int close_session()
{
	SaErrorT rv;

	/* Bug 2171901, replace pthread_kill(ge_thread, SIGKILL); */
	pthread_cancel(ge_thread);
	
	rv = saHpiSessionClose(Domain->sessionId);
	if (rv != SA_OK) {
                printf("saHpiSessionClose error %s\n", oh_lookup_error(rv));
                return -1;
        }
	return 0;
}
