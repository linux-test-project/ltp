/* -*- linux-c -*-
 *
 * (C) Copyright IBM Corp 2006
 *
 * Author(s):
 *     Renier Morales <renier@openhpi.org>
 *
 * hpiel - Displays HPI event log entries.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <getopt.h>


#include <SaHpi.h>
#include <oHpi.h>
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.11 $"

#define err(format, ...) \
        do { \
                if (opts.dbg) { \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)

#define show_error_quit(msg) \
        do { \
                if (error) { \
			opts.dbg = 1; \
                        err(msg, oh_lookup_error(error)); \
                        exit(-1); \
                } \
        } while(0)

/* Globals */
static struct hpiel_opts {
        int  del;        /* Domain Event Log option. */
        char *ep;       /* Resource Event Log option. */
        int  clear;      /* Clear the event log before traversing it. */
        int  resource;   /* Get resource along with event log entry. */
        int  rdr;        /* Get RDR along with event log entry. */
        int  dbg;        /* Display debug messages. */
} opts = { 0, NULL, 0, 0, 0, 0 };

/* Prototypes */
SaErrorT parse_options(int argc, char ***argv, struct hpiel_opts *opts);
SaErrorT harvest_sels(SaHpiSessionIdT sid, SaHpiDomainInfoT *dinfo, char *ep);
SaErrorT display_el(SaHpiSessionIdT sid, SaHpiResourceIdT rid, SaHpiTextBufferT *tag);

int main(int argc, char **argv)
{
        SaErrorT error = SA_OK;
        SaHpiSessionIdT sid;
        SaHpiDomainInfoT dinfo;

        /* Print version strings */
	oh_prog_version(argv[0], OH_SVN_REV);

        /* Parsing options */
        if (parse_options(argc, &argv, &opts)) {
                fprintf(stderr, "There was an error parsing the options. Exiting.\n");
                exit(-1);
        }

        /* Program really begins here - all options parsed at this point */
        error = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sid, NULL);
        show_error_quit("saHpiSessionOpen() returned %s. Exiting.\n");

        error = saHpiDiscover(sid);
        show_error_quit("saHpiDiscover() returned %s. Exiting.\n");

        error = saHpiDomainInfoGet(sid, &dinfo);
        show_error_quit("saHpiDomainInfoGet() returned %s. Exiting.\n");

        printf("Domain Info: UpdateCount = %d, UpdateTime = %lx\n",
               dinfo.RptUpdateCount, (unsigned long)dinfo.RptUpdateTimestamp);

        if (opts.ep) { /* Entity path specified */
                error = harvest_sels(sid, &dinfo, opts.ep);
        } else if (opts.del) { /* Domain event log specified */
                error = display_el(sid, SAHPI_UNSPECIFIED_RESOURCE_ID, &dinfo.DomainTag);
        } else { /* Else, show SELs of all supporting resources */
                error = harvest_sels(sid, &dinfo, NULL);
        }

        if (error) err("An error happened. Gathering event log entries returned %s",
                       oh_lookup_error(error));

        error = saHpiSessionClose(sid);
        if (error) err("saHpiSessionClose() returned %s.",
                       oh_lookup_error(error));

        return error;
}

#define print_usage_quit() \
        do { \
                printf("\nUsage:\n" \
                       "  hpiel - Displays HPI event log entries.\n\n" \
                       "    --del, -d                        display domain event log entries\n" \
                       "    --entitypath=\"<arg>\", -e \"<arg>\"     display resource event log entries\n" \
                       "            (e.g. -e \"{SYSTEM_CHASSIS,2}{SBC_BLADE,5}\")\n" \
                       "    --clear, -c                      clear log before reading event log entries\n" \
                       "    --resource, -p                   pull resource info along with log entry\n" \
                       "    --rdr, -r                        pull RDR info along with log entry\n" \
                       "    --xml, -x                        print output in xml format (not implemented)\n" \
                       "    --verbose, -v                    print debug messages\n" \
                       "    --help, -h                       print this usage message\n" \
                       "\n    If neither -d or -e \"<arg>\" are specified, event log entries will be shown\n" \
                       "    for all supporting resources by default.\n\n"); \
                exit(0); \
        } while(0)

SaErrorT parse_options(int argc, char ***argv, struct hpiel_opts *opts)
{
        static struct option long_options[] = {
                {"del",        no_argument,       0, 'd'},
                {"entitypath", required_argument, 0, 'e'},
                {"clear",      no_argument,       0, 'c'},
                {"resource",   no_argument,       0, 'p'},
                {"rdr",        no_argument,       0, 'r'},
                {"verbose",    no_argument,       0, 'v'},
                {"help",       no_argument,       0, 'h'},
                {0, 0, 0, 0}
        };
        int c, option_index = 0;

        while ((c = getopt_long(argc, *argv, "de:cprvh", long_options, &option_index)) != -1) {
                switch(c) {
                        case 'd':
                                opts->del = 1;
                                break;
                        case 'e':
                                opts->ep = strdup(optarg);
                                break;
                        case 'c':
                                opts->clear = 1;
                                break;
                        case 'p':
                                opts->resource = 1;
                                break;
                        case 'r':
                                opts->rdr = 1;
                                break;
                        case 'v':
                                opts->dbg = 1;
                                break;
                        case 'h':
                                print_usage_quit();
                                break;
                        case '?':
                                printf("extraneous option found, %d\n", optopt);
                                break;
                        default:
                                printf("Found bad option %d.\n", c);
                                return -1;
                }
        }

        return 0;
}

SaErrorT harvest_sels(SaHpiSessionIdT sid, SaHpiDomainInfoT *dinfo, char *ep)
{
        SaErrorT error = SA_OK;
        SaHpiRptEntryT rptentry;
        SaHpiEntryIdT entryid, nextentryid;
        SaHpiResourceIdT rid;
        oh_big_textbuffer bigbuf;
        SaHpiEntityPathT entitypath;
        SaHpiBoolT found_entry = SAHPI_FALSE;

        if (!sid || !dinfo) {
                err("Invalid parameters in havest_sels()\n");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (opts.ep && ep) {
                error = oh_encode_entitypath(ep, &entitypath);
                if (error) {
                        err("oh_encode_entitypath() returned %s from %s\n",
                            oh_lookup_error(error), ep);
                        return error;
                }
        }

        entryid = SAHPI_FIRST_ENTRY;
        while (error == SA_OK && entryid != SAHPI_LAST_ENTRY) {
                error = saHpiRptEntryGet(sid, entryid, &nextentryid, &rptentry);

                err("saHpiRptEntryGet() returned %s\n", oh_lookup_error(error));
                if (error == SA_OK) {
                        if (opts.ep && ep) {
                                if (!oh_cmp_ep(&entitypath, &rptentry.ResourceEntity)) {
                                        entryid = nextentryid;
                                        continue;
                                }
                        }

                        if (!(rptentry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)) {
                                err("RPT doesn't have SEL\n");
                                entryid = nextentryid;
                                continue;  /* no SEL here, try next RPT */
                        }
                        found_entry = SAHPI_TRUE;

                        rid = rptentry.ResourceId;
                        err("RPT %d capabilities = %x\n",
                            rid, rptentry.ResourceCapabilities);
                        rptentry.ResourceTag.Data[rptentry.ResourceTag.DataLength] = 0;

                        oh_init_bigtext(&bigbuf);
                        error = oh_decode_entitypath(&rptentry.ResourceEntity, &bigbuf);
                        printf("%s\n", bigbuf.Data);
                        printf("rptentry[%d] tag: %s\n", rid, rptentry.ResourceTag.Data);

                        error = display_el(sid, rid, &rptentry.ResourceTag);

                        if (opts.ep && ep) return SA_OK;
                }

                entryid = nextentryid;
        }

        if (!found_entry) {
                if (opts.ep && ep) {
                        err("Could not find resource matching %s\n", ep);
                } else {
                        err("No resources supporting event logs were found.\n");
                }
        }

        return error;
}

SaErrorT display_el(SaHpiSessionIdT sid, SaHpiResourceIdT rid, SaHpiTextBufferT *tag)
{
        SaErrorT error = SA_OK;
        SaHpiEventLogEntryIdT entryid, nextentryid, preventryid;
        SaHpiEventLogInfoT elinfo;
        SaHpiEventLogEntryT elentry;
        SaHpiRdrT rdr;
        SaHpiRptEntryT res;

        if (!sid || !rid) {
                err("Invalid parameters in display_el()\n");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        error = saHpiEventLogInfoGet(sid, rid, &elinfo);
        if (error) {
                err("saHpiEventLogInfoGet() returned %s. Exiting\n",
                    oh_lookup_error(error));
                return error;
        }

        printf("EventLogInfo for %s, ResourceId %d\n",
               tag->Data, rid);
        oh_print_eventloginfo(&elinfo, 4);

        if (elinfo.Entries == 0) {
                printf("%s Resource %d has an empty event log.\n", tag->Data, rid);
                return SA_OK;
        }

        if (opts.clear) {
                error = saHpiEventLogClear(sid, rid);
                if (error == SA_OK)
                        printf("EventLog successfully cleared\n");
                else {
                        printf("saHpiEventLogClear() returned %s\n",
                               oh_lookup_error(error));
                        return error;
                }

        }

        entryid = SAHPI_OLDEST_ENTRY;
        while (entryid != SAHPI_NO_MORE_ENTRIES) {
                error = saHpiEventLogEntryGet(sid, rid,
                                              entryid, &preventryid,
                                              &nextentryid, &elentry,
                                              &rdr,
                                              &res);

                err("saHpiEventLogEntryGet() returned %s\n", oh_lookup_error(error));
                if (error == SA_OK) {
                	SaHpiEntityPathT *ep = NULL;
                	/* Get a reference to the entity path for this log entry */
                	if (res.ResourceCapabilities) {
                		ep = &res.ResourceEntity;
                	} else if (rdr.RdrType != SAHPI_NO_RECORD) {
                		ep = &rdr.Entity;
                	}
                	/* Print the event log entry */
                        oh_print_eventlogentry(&elentry, ep, 6);
                        if (opts.rdr) {
                                if (rdr.RdrType == SAHPI_NO_RECORD)
                                        printf("            No RDR associated with EventType =  %s\n\n",
                                               oh_lookup_eventtype(elentry.Event.EventType));
                                else
                                        oh_print_rdr(&rdr, 12);
                        }

                        if (opts.resource) {
                                if (res.ResourceCapabilities == 0)
                                        printf("            No RPT associated with EventType =  %s\n\n",
                                               oh_lookup_eventtype(elentry.Event.EventType));
                                else
                                        oh_print_rptentry(&res, 10);
                        }

                        preventryid = entryid;
                        entryid = nextentryid;
                } else {
                        return error;
                }
        }

        return SA_OK;
}
