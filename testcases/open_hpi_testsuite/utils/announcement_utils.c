/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003-2006
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
 *      David Ashley <dashley@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

/* allocate and initialize an announcement list */
oh_announcement *oh_announcement_create(void)
{
        oh_announcement *ann;

        ann = (oh_announcement *)g_malloc0(sizeof(oh_announcement));
        if (ann != NULL) {
                ann->nextId = SAHPI_OLDEST_ENTRY + 1; // always start at 1
                ann->annentries = NULL;
        }
        return ann;
}


/* close and free all memory associated with an announcement list */
SaErrorT oh_announcement_close(oh_announcement *ann)
{

        if (ann == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        oh_announcement_clear(ann);
        g_free(ann);
        return SA_OK;
}


/* append a new entry to the announcement list */
SaErrorT oh_announcement_append(oh_announcement *ann, SaHpiAnnouncementT *myann)
{
        oh_ann_entry *entry;
        time_t tt1;

        /* check for valid el params and state */
        if (ann == NULL || myann == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* alloc and copy the new entry */
        entry = (oh_ann_entry *) g_malloc0(sizeof(oh_ann_entry));
        if (entry == NULL) {
                return SA_ERR_HPI_OUT_OF_SPACE;
        }
        memcpy(&entry->annentry, myann, sizeof(SaHpiAnnouncementT));

        /* initialize the struct and append the new entry */
        entry->annentry.EntryId = ann->nextId++;
        time(&tt1);
        entry->annentry.Timestamp = ((SaHpiTimeT) tt1) * 1000000000;
        entry->annentry.AddedByUser = TRUE;
        ann->annentries = g_list_append(ann->annentries, entry);

        /* copy the new generated info back to the user's announcement */
        memcpy(myann, &entry->annentry, sizeof(SaHpiAnnouncementT));

        return SA_OK;
}


/* clear all announcement entries */
SaErrorT oh_announcement_clear(oh_announcement *ann)
{
        GList *temp;

        if (ann == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* free the list data elements */
        temp = g_list_first(ann->annentries);
        while (temp != NULL) {
                g_free(temp->data);
                temp = g_list_next(temp);
        }
        /* free the list nodes */
        g_list_free(ann->annentries);
        /* reset the control structure */
        ann->nextId = SAHPI_OLDEST_ENTRY + 1; // always start at 1
        ann->annentries = NULL;

        return SA_OK;
}


/* get an announcement entry */
SaErrorT oh_announcement_get(oh_announcement *ann, SaHpiEntryIdT srchid,
                             SaHpiAnnouncementT *entry)
{
        oh_ann_entry *myentry;
        GList *annlist;

        if (ann == NULL || entry == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        annlist = g_list_first(ann->annentries);
        if (annlist == NULL) return SA_ERR_HPI_NOT_PRESENT;
        
        if (srchid == SAHPI_FIRST_ENTRY && annlist != NULL) {
                myentry = (oh_ann_entry *) annlist->data;
                memcpy(entry, &myentry->annentry, sizeof(SaHpiAnnouncementT));
                return SA_OK;
        }
        if (srchid == SAHPI_LAST_ENTRY && annlist != NULL) {
                annlist = g_list_last(ann->annentries);
                myentry = (oh_ann_entry *) annlist->data;
                memcpy(entry, &myentry->annentry, sizeof(SaHpiAnnouncementT));
                return SA_OK;
        }
        while (annlist != NULL) {
                myentry = (oh_ann_entry *) annlist->data;
                if (srchid == myentry->annentry.EntryId) {
                        memcpy(entry, &myentry->annentry, sizeof(SaHpiAnnouncementT));
                        return SA_OK;
                }
                annlist = g_list_next(annlist);
        }
        return SA_ERR_HPI_NOT_PRESENT;
}


/* get next announcement entry */
SaErrorT oh_announcement_get_next(oh_announcement *ann, SaHpiSeverityT sev,
                                  SaHpiBoolT ack, SaHpiAnnouncementT *entry)
{
        GList *annlist = NULL;

        if (ann == NULL || entry == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if (entry->EntryId == SAHPI_FIRST_ENTRY) {
                annlist = ann->annentries; /* start search at beginning */
        } else {
                /* find the previous matching entry */
                for (annlist = ann->annentries;
                     annlist; annlist = annlist->next) {
                        oh_ann_entry *annentry = annlist->data;
                        if (entry->EntryId == annentry->annentry.EntryId) {
                                if (entry->Timestamp ==
                                    annentry->annentry.Timestamp) {
                                        break;
                                } else {
                                        return SA_ERR_HPI_INVALID_DATA;
                                }
                        }
                }
        
                /* Set list node for searching for next matching entry */
                if (annlist)
                        annlist = g_list_next(annlist);
                else {
                        dbg("Did not find previous entry."
                            " Searching from first one.");
                        annlist = g_list_first(ann->annentries);
                }
        }

        /* Find the matching entry based on severity and ack */
        for (; annlist; annlist = annlist->next) {
                oh_ann_entry *annentry = annlist->data;
                if (annentry && (sev == SAHPI_ALL_SEVERITIES ||
                                 sev == annentry->annentry.Severity) &&
                    (ack ? !annentry->annentry.Acknowledged : 1)) {
                        dbg("Severity searched for is %d."
                            " Severity found is %d",
                            sev, annentry->annentry.Severity);
                        *entry = annentry->annentry;
                        return SA_OK;
                }
        }

        return SA_ERR_HPI_NOT_PRESENT;
}


SaErrorT oh_announcement_ack(oh_announcement *ann, SaHpiEntryIdT srchid,
                             SaHpiSeverityT sev)
{
        oh_ann_entry *myentry;
        GList *annlist;

        if (ann == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        /* Search for one announcement if entryid is specified */
        if (srchid != SAHPI_ENTRY_UNSPECIFIED) {
                annlist = g_list_first(ann->annentries);
                while (annlist != NULL) {
                        myentry = (oh_ann_entry *) annlist->data;
                        if (srchid == myentry->annentry.EntryId) {
                                myentry->annentry.Acknowledged = TRUE;
                                return SA_OK;
                        }
                        annlist = g_list_next(annlist);
                }
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* EntryId not specified, so ack announcements which have the specified severity */
        annlist = g_list_first(ann->annentries);
        if (annlist == NULL) return SA_OK;
        
        while (annlist != NULL) {
                myentry = (oh_ann_entry *) annlist->data;
                if (sev == SAHPI_ALL_SEVERITIES ||
                    sev == myentry->annentry.Severity) {
                        myentry->annentry.Acknowledged = TRUE;
                }
                annlist = g_list_next(annlist);
        }

        return SA_OK;
}


SaErrorT oh_announcement_del(oh_announcement *ann, SaHpiEntryIdT srchid,
                             SaHpiSeverityT sev)
{
        oh_ann_entry *myentry;
        GList *annlist;

        if (ann == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        /* Search for one announcement if entryid is specified */
        if (srchid != SAHPI_ENTRY_UNSPECIFIED) {
                annlist = g_list_first(ann->annentries);
                while (annlist != NULL) {
                        myentry = (oh_ann_entry *) annlist->data;
                        if (srchid == myentry->annentry.EntryId) {
                                free(annlist->data);
                                ann->annentries =
                                        g_list_remove(ann->annentries, myentry);
                                return SA_OK;
                        }
                        annlist = g_list_next(annlist);
                }
                return SA_ERR_HPI_NOT_PRESENT;
        }

        /* remove all announcements with a specified severity */
        annlist = g_list_first(ann->annentries);
        if (annlist == NULL) return SA_OK;
        
        while (annlist != NULL) {
                myentry = (oh_ann_entry *) annlist->data;
                if (sev == SAHPI_ALL_SEVERITIES ||
                    sev == myentry->annentry.Severity) {
                        free(annlist->data);
                        ann->annentries = g_list_remove(ann->annentries, myentry);
                        annlist = g_list_first(ann->annentries);
                } else {
                        annlist = g_list_next(annlist);
                }
        }
        return SA_OK;
}








