/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004, 2006
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

/* allocate and initialize an EL */
oh_el *oh_el_create(SaHpiUint32T size)
{
        oh_el *el;

        el = (oh_el *) g_malloc0(sizeof(oh_el));
        if (el != NULL) {
		el->basetime = 0;
		el->sysbasetime = 0;
		el->nextid = SAHPI_OLDEST_ENTRY + 1;
		el->gentimestamp = SAHPI_TRUE;
		
		el->info.Entries = 0;
		el->info.Size = size;
		el->info.UserEventMaxSize = SAHPI_MAX_TEXT_BUFFER_LENGTH;
		el->info.UpdateTimestamp = SAHPI_TIME_UNSPECIFIED;
		el->info.CurrentTime = SAHPI_TIME_UNSPECIFIED;
                el->info.Enabled = SAHPI_TRUE;
                el->info.OverflowFlag = SAHPI_FALSE;
		el->info.OverflowResetable = SAHPI_TRUE;
        	el->info.OverflowAction = SAHPI_EL_OVERFLOW_OVERWRITE;
		
                el->list = NULL;
        }
        return el;
}


/* close and free all memory associated with an EL */
SaErrorT oh_el_close(oh_el *el)
{
        if (el == NULL) return SA_ERR_HPI_INVALID_PARAMS;

	oh_el_clear(el);	
        g_free(el);
	
        return SA_OK;
}


/* append a new entry to the EL */
SaErrorT oh_el_append(oh_el *el,
		       const SaHpiEventT *event,
		       const SaHpiRdrT *rdr,
		       const SaHpiRptEntryT *res)
{
        oh_el_entry *entry;
        struct timeval tv;
	SaHpiTimeT cursystime;

        /* check for valid el params and state */
        if (el == NULL || event == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (el->info.Enabled == FALSE &&
		    event->EventType != SAHPI_ET_USER) {
                return SA_ERR_HPI_INVALID_REQUEST;
        }

        /* alloc the new entry */
        entry = (oh_el_entry *) g_malloc0(sizeof(oh_el_entry));
        if (entry == NULL) {
                el->info.OverflowFlag = TRUE;
                return SA_ERR_HPI_OUT_OF_SPACE;
        }
	
        if (rdr) entry->rdr = *rdr;
        if (res) entry->res = *res;

        /* if necessary, wrap the el entries */
        if (el->info.Size != OH_EL_MAX_SIZE &&
	    g_list_length(el->list) == el->info.Size) {
		g_free(el->list->data);
                el->list = g_list_delete_link(el->list, el->list);
                el->info.OverflowFlag = SAHPI_TRUE;
        }

        /* Set the event log entry id and timestamp */
        entry->event.EntryId = el->nextid++;
	if (el->gentimestamp) {
        	gettimeofday(&tv, NULL);
		cursystime = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
        	el->info.UpdateTimestamp =
			el->basetime + (cursystime - el->sysbasetime);
	} else {
		el->info.UpdateTimestamp = event->Timestamp;
		/* Setting time based on the event to have some sense of what
		 * the current time is going to be when providing the el info.
		 */
		oh_el_timeset(el, event->Timestamp);
	}
	entry->event.Timestamp = el->info.UpdateTimestamp;

	/* append the new entry */
	entry->event.Event = *event;
        el->list = g_list_append(el->list, entry);
	
        return SA_OK;
}


/* prepend a new entry to the EL */
SaErrorT oh_el_prepend(oh_el *el,
			const SaHpiEventT *event,
			const SaHpiRdrT *rdr,
			const SaHpiRptEntryT *res)
{
        GList *node = NULL;
	oh_el_entry *entry;        
        struct timeval tv;
	SaHpiTimeT cursystime;

        /* check for valid el params and state */
        if (el == NULL || event == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (el->info.Enabled == FALSE &&
		    event->EventType != SAHPI_ET_USER) {
                return SA_ERR_HPI_INVALID_REQUEST;
        }

        /* see if el is full */
        if (el->info.Size != OH_EL_MAX_SIZE &&
	    g_list_length(el->list) == el->info.Size) {
                return SA_ERR_HPI_OUT_OF_SPACE;
        }

        /* alloc the new entry */
        entry = (oh_el_entry *) g_malloc0(sizeof(oh_el_entry));
        if (entry == NULL) {
                el->info.OverflowFlag = TRUE;
                return SA_ERR_HPI_OUT_OF_SPACE;
        }
        
	if (rdr) entry->rdr = *rdr;
        if (res) entry->res = *res;

        /* since we are adding entries in reverse order we have to renumber
         * existing entries
         */        
	for (node = el->list; node; node = node->next) {
                oh_el_entry *tmpentry = (oh_el_entry *)node->data;
		tmpentry->event.EntryId++;
        }
	el->nextid++;

        /* prepare & prepend the new entry */
        entry->event.EntryId = SAHPI_OLDEST_ENTRY + 1;
	if (el->gentimestamp) {
        	gettimeofday(&tv, NULL);
		cursystime = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
        	el->info.UpdateTimestamp =
			el->basetime + (cursystime - el->sysbasetime);
	} else {
		el->info.UpdateTimestamp = event->Timestamp;
		/* Setting time based on the event to have some sense of what
		 * the current time is going to be when providing the el info.
		 */
		oh_el_timeset(el, event->Timestamp);
	}
        entry->event.Timestamp = el->info.UpdateTimestamp;
	
	/* prepend the new entry to the list */
	entry->event.Event = *event;
        el->list = g_list_prepend(el->list, entry);
	
        return SA_OK;
}


/* clear all EL entries */
SaErrorT oh_el_clear(oh_el *el)
{
        GList *node;

        if (el == NULL) return SA_ERR_HPI_INVALID_PARAMS;

        /* free the data for every element in the list */
	for (node = el->list; node; node = node->next) {        
                g_free(node->data);
        }
	
        /* free the list nodes */
        g_list_free(el->list);
        
	/* reset the control structure */
        el->info.OverflowFlag = SAHPI_FALSE;
        el->info.UpdateTimestamp = SAHPI_TIME_UNSPECIFIED;
	el->info.Entries = 0;
        el->nextid = SAHPI_OLDEST_ENTRY + 1; // always start at 1
        el->list = NULL;

        return SA_OK;
}


/* get an EL entry */
SaErrorT oh_el_get(oh_el *el,
		   SaHpiEventLogEntryIdT entryid,
		   SaHpiEventLogEntryIdT *prev,
                   SaHpiEventLogEntryIdT *next,
		   oh_el_entry **entry)
{
        SaHpiEventLogEntryIdT eid;
	GList *node = NULL;
	oh_el_entry *elentry = NULL;	
	
	if (!el || !prev || !next || !entry ||
	    entryid == SAHPI_NO_MORE_ENTRIES) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
	
        if (g_list_length(el->list) == 0) {
                return SA_ERR_HPI_NOT_PRESENT;
        }
	
	/* FIXME: There is a bug here because this does not take into account
	 * the case when oh_el_prepend would have been used. In such case the
	 * OLDEST entry would technically not be the first one in the list.
	 * To be continued...
	 * 	-- Renier Morales (08/30/06)
	 */
        if (entryid == SAHPI_OLDEST_ENTRY) {
		node = g_list_first(el->list);
	} else if (entryid == SAHPI_NEWEST_ENTRY) {
		node = g_list_last(el->list);
	}

	if (node) {
		elentry = (oh_el_entry *)node->data;
		eid = elentry->event.EntryId;
	} else {
		eid = entryid;
	}
	
	for (node = el->list; node; node = node->next) {
		elentry = (oh_el_entry *)node->data;
		if (eid == elentry->event.EntryId) {
			*entry = elentry;
			if (node->prev) {
				elentry = (oh_el_entry *)node->prev->data;
				*prev = elentry->event.EntryId;
			} else {
				*prev = SAHPI_NO_MORE_ENTRIES;
			}
			if (node->next) {
				elentry = (oh_el_entry *)node->next->data;
				*next = elentry->event.EntryId;
			} else {
				*next = SAHPI_NO_MORE_ENTRIES;
			}
			return SA_OK;
		}
	}

	return SA_ERR_HPI_NOT_PRESENT;
}


/* get EL info */
SaErrorT oh_el_info(oh_el *el, SaHpiEventLogInfoT *info)
{
        struct timeval tv;
	SaHpiTimeT cursystime;

        if (el == NULL || info == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        
        *info = el->info;
	info->Entries = g_list_length(el->list);
        gettimeofday(&tv, NULL);
	cursystime = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;	
        info->CurrentTime = el->basetime + (cursystime - el->sysbasetime);
        
        return SA_OK;
}


/* reset EL overflowflag */
SaErrorT oh_el_overflowreset(oh_el *el)
{
        if (el == NULL) return SA_ERR_HPI_INVALID_PARAMS;

	if (el->info.OverflowResetable) {
        	el->info.OverflowFlag = SAHPI_FALSE;
		return SA_OK;
	} else {
		return SA_ERR_HPI_INVALID_CMD;
	}
}

SaErrorT oh_el_overflowset(oh_el *el, SaHpiBoolT flag)
{
	if (!el) return SA_ERR_HPI_INVALID_PARAMS;
	
	el->info.OverflowFlag = flag;
	
	return SA_OK;
}


/* write a EL entry list to a file */
SaErrorT oh_el_map_to_file(oh_el *el, char *filename)
{
        int file;
        GList *node = NULL;

        if (el == NULL || filename == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
        if (file < 0) {
                err("EL file '%s' could not be opened", filename);
                return SA_ERR_HPI_ERROR;
        }
        
	for (node = el->list; node; node = node->next) {
                if (write(file, (void *)node->data, sizeof(oh_el_entry)) != sizeof(oh_el_entry)) {
			err("Couldn't write to file '%s'.", filename);
			close(file);
                	return SA_ERR_HPI_ERROR;
		}
        }

        if (close(file) != 0) {
                err("Couldn't close file '%s'.", filename);
                return SA_ERR_HPI_ERROR;
        }

        return SA_OK;
}


/* read a EL entry list from a file */
SaErrorT oh_el_map_from_file(oh_el *el, char *filename)
{
        int file;
        oh_el_entry entry;

        /* check el params and state */
        if (el == NULL || filename == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        } else if (el->info.Enabled == FALSE) {
                return SA_ERR_HPI_INVALID_REQUEST;
        }

        file = open(filename, O_RDONLY);
        if (file < 0) {
                err("EL file '%s' could not be opened", filename);
                return SA_ERR_HPI_ERROR;
        }

        oh_el_clear(el); // ensure list is empty
        while (read(file, &entry, sizeof(oh_el_entry)) == sizeof(oh_el_entry)) {
		oh_el_entry *elentry = (oh_el_entry *)g_malloc0(sizeof(oh_el_entry));
		el->nextid = entry.event.EntryId;
		el->nextid++;
		*elentry = entry;
		el->list = g_list_append(el->list, elentry);
        }

        if (close(file) != 0) {
                err("Couldn't close file '%s'.", filename);
                return SA_ERR_HPI_ERROR;
        }

        return SA_OK;
}


/* set the EL timestamp offset */
SaErrorT oh_el_timeset(oh_el *el, SaHpiTimeT timestamp)
{
        struct timeval tv;
	
	if (el == NULL || timestamp == SAHPI_TIME_UNSPECIFIED) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }
	
	gettimeofday(&tv, NULL);
	el->sysbasetime = (SaHpiTimeT) tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
	el->basetime = timestamp;

        return SA_OK;
}

/* set the timestamp generate flag */
SaErrorT oh_el_setgentimestampflag(oh_el *el, SaHpiBoolT flag)
{
        if (el == NULL) {
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        el->gentimestamp = flag;
        return SA_OK;
}

SaErrorT oh_el_enableset(oh_el *el, SaHpiBoolT flag)
{
	if (!el) return SA_ERR_HPI_INVALID_PARAMS;

	el->info.Enabled = flag;
	
	return SA_OK;
}
