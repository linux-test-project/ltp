/*      -*- watchdog-c -*-
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
 *     Rusty Lynch <rusty.lynch@linux.intel.com>
 *     Julie Fleischer <julie.n.fleischer@intel.com>
 *
 * watchdog.c: This OpenHPI plug-in implements a simple wrapper for the 
 *             standard Linux watchdog interface.
 *
 *             Note:  This program is currently only written to the
 *             functionality in drivers/char/watchdog/softdog.c, so
 *             the functionality is limited.
 *             Will be expanded to functionality in Documentation/
 *             watchdog/watchdog-api.txt eventually.  Even then, 
 *             functionality is still quite limited.
 *             Need to use IPMI watchdog for full watchdog
 *             functionality.  (This is part of the IPMI plugin.)
 *
 *             Temperature/Fan RDR code has not been created as could
 *             not find a watchdog defined in watchdog-api.txt that
 *             implemented this functionality.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/watchdog.h>
#include <glib-1.2/glib.h>

#include <SaHpi.h>
#include <openhpi.h>
#include <uid_utils.h>
#include <epath_utils.h>

/* These two IOCTL's were not added to watchdog.h in older kernels */
#define WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT        _IOR(WATCHDOG_IOCTL_BASE, 7, int)

#define MAX_PATH 256

#define DEFAULT_TIMEOUT 10;

#define WD_CAPS \
    SAHPI_CAPABILITY_RESOURCE|SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_WATCHDOG

SaHpiEntityPathT g_epbase; /* root entity path (from config) */

struct wdtitems {
	int initialized;
	int fd;
	char path[MAX_PATH];
	SaHpiWatchdogT data;
};

/**
 * *watchdog_open:
 * @handler_config: pointer to config file
 *
 * This function creates an instance for the watchdog plugin
 * and returns a handler to the instance.
 * addr corresponds to the location of the watchdog device
 **/

static void *watchdog_open(GHashTable *handler_config)
{
        struct oh_handler_state *hnd;
        struct wdtitems *wdt;
        char *er;

        if (!handler_config) {
                dbg("empty handler_config");
                return NULL;
        }

        /* set up entity root in g_epbase */
        er = (char *)g_hash_table_lookup(handler_config,"entity_root");
        if (!er) {
                dbg("no entity root present");
                return NULL;
        }
        string2entitypath(er, &g_epbase);
        append_root(&g_epbase);

        hnd = malloc(sizeof(*hnd));
        if (!hnd) {
                dbg("unable to allocate main handler");
                return NULL;
        }

        memset(hnd, '\0', sizeof(*hnd));

        /* assign config to handler_config and initialize rptcache */
        hnd->config = handler_config;

        hnd->rptcache = (RPTable *)g_malloc0(sizeof(RPTable));

        wdt = malloc(sizeof(*wdt));
        if (!wdt) {
                dbg("unable to allocate wdtitems structure");
                return NULL;
        }
        memset(wdt, '\0', sizeof(*wdt));
	strncpy(wdt->path,
                (char *)g_hash_table_lookup(handler_config, "addr"),
                MAX_PATH);

        hnd->data = (void *) wdt;

        return hnd;

}

/**
 * watchdog_close:
 * @hnd: pointer to instance
 *
 * Close the instance for the watchdog plugin.
 * Note:  There is currently no way to test this code
 * as it is not called by the framework.
 **/

static void watchdog_close(void *hnd)
{
	struct oh_handler_state *tmp = (struct oh_handler_state *)hnd;
        struct wdtitems *wdt;
	
        if (!tmp) {
                dbg("no instance to delete");
                return;
        }

        wdt = tmp->data;
	if (wdt->data.Running) {
		write(wdt->fd, "V", 1);
		close(wdt->fd);
	}

        /* Free unused events */
        g_slist_free(tmp->eventq);

	free(tmp);

	return;
}

/**
 * watchdog_get_event:
 * @hnd: pointer to handler instance
 * @event: pointer to oh_event
 * @timeout: struct timeval
 *
 * This function gets a watchdog event from the watchdog event table
 * in instance.events.  It copies the event to memory and then
 * deletes it from the instance.events table.
 *
 * Return value: 0 if times out, > 0 is event is returned.
 **/
static int watchdog_get_event(void *hnd, struct oh_event *event, 
			      struct timeval *timeout)
{
	struct oh_handler_state *tmp = (struct oh_handler_state *) hnd;
	GSList *i;
	
	if (!tmp) {
		dbg("no handler given");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	g_slist_for_each(i, tmp->eventq) {
		struct oh_event *e = (struct oh_event *)i->data;
		if (e) {
			memcpy(event, e, sizeof(*e));
			tmp->eventq = g_slist_remove_link(tmp->eventq, i);
			g_slist_free(i);
			free(e);

			return 1;
		}
	}
	return 0;
}

/**
 * watchdog_discover_resources:
 * @hnd: void pointer to handler
 *
 * Discover the resources in watchdog.
 *
 * Return value: 0 for success | Error code
 **/
static int watchdog_discover_resources(void *hnd)
{
	struct oh_event *e;
	struct oh_handler_state *tmp = (struct oh_handler_state *)hnd;
	int puid, timeout = DEFAULT_TIMEOUT;
	struct wdtitems *wdt;

	if (!tmp) {
		dbg("no handler given");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	wdt = tmp->data;
	if (!wdt->initialized) {
		wdt->initialized = 1;

		/*
		 * Verify we really have a watchdog available that
		 * interacts with the watchdog char device in the standard way
		 * as described in the kernel watchdog-api.txt
		 * documentation.
		 *
		 * If there are any problems with the standard watchdog
		 * interface, consider the watchdog device undetected
		 * and do not return an error, but just do not bubble
		 * up and RPT and RDR entries.
		 */
		wdt->fd = open(wdt->path, O_RDWR);
		if (-1 == wdt->fd) {
			dbg("watchdog device is not enabled");
			return 0;
		}
		/* the clock is ticking... set the default timeout */
                /* before it is too late                           */
		if ( -1 == ioctl(wdt->fd, WDIOC_SETTIMEOUT, &timeout)) {
			dbg("unable to set watchdog timeout");
			write(wdt->fd, "V", 1);
			close(wdt->fd);
			return 0;
		}
		if ( -1 == ioctl(wdt->fd, WDIOC_GETTIMEOUT, &timeout)) {
			dbg("unable to read watchdog timeout");
			write(wdt->fd, "V", 1);
			close(wdt->fd);
			return 0;
		}

		/* writing "V" and closing the wdt disables it */
		if (-1 == write(wdt->fd, "V", 1)) {
			dbg("Unable to write to watchdog - cannot close");
			return 0;
		}
		close(wdt->fd);

		/* Set wdt to contain watchdog timer information.
		 * Note:  Using watchdog-api.txt, pretimer interrupt
		 * and pretimeout interval functionality is not available.
		 * In addition, event notification of a timeout is
		 * unavailable.
		 */
		wdt->data.Log = SAHPI_FALSE;  /* don't issue event on timeout */
		wdt->data.Running = SAHPI_FALSE; /* not currently running */
		wdt->data.TimerUse = SAHPI_WTU_SMS_OS;
		wdt->data.TimerAction = SAHPI_WA_RESET;
		wdt->data.PretimerInterrupt = SAHPI_WPI_NONE;
		wdt->data.PreTimeoutInterval = 0;
		wdt->data.TimerUseExpFlags = 0; /* not used -- cannot set on timeout */
		wdt->data.InitialCount = timeout * 1000;
		wdt->data.PresentCount = 0; 

		/*
		 * Since event notification on timeout is not available,
		 * the only events we can send back to the OpenHPI
		 * infrastructure are the initial RPT and RDR creation
		 * events to populate the domain RPT table.
		 */

		/* 
		 * create RPT creation event
		 */	
		e = (struct oh_event *)malloc(sizeof(*e));
		if (!e) {
			dbg("unable to allocate event");
			return SA_ERR_HPI_OUT_OF_SPACE;
		}
		memset(e, '\0', sizeof(struct oh_event));
		e->type = OH_ET_RESOURCE;
		/* Note:  .res_event.entry.ResourceInfo currently unassigned */
		e->u.res_event.entry.ResourceEntity.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD;
		e->u.res_event.entry.ResourceEntity.Entry[0].EntityInstance = 0;
		ep_concat( &(e->u.res_event.entry.ResourceEntity), &g_epbase);
		puid = oh_uid_from_entity_path(&(e->u.res_event.entry.ResourceEntity));
		e->u.res_event.entry.ResourceId = puid;
		e->u.res_event.entry.EntryId = puid;
		e->u.res_event.entry.ResourceCapabilities = WD_CAPS;
		e->u.res_event.entry.ResourceSeverity = SAHPI_CRITICAL;
		/* Note e->u.res_event.entry.DomainId as well as  e->u.res_event.domainid.ptr not set */
		e->u.res_event.entry.ResourceTag.DataType = SAHPI_TL_TYPE_ASCII6;
		e->u.res_event.entry.ResourceTag.Language = SAHPI_LANG_ENGLISH;
		e->u.res_event.entry.ResourceTag.DataLength = 12;
		strcpy(e->u.res_event.entry.ResourceTag.Data, "System-Board");
		
		/* add resource */
		if (0 != oh_add_resource(tmp->rptcache, &(e->u.res_event.entry), NULL, 0)) {
			dbg("unable to add resource to RPT");
			return SA_ERR_HPI_ERROR;
		}

		/* add event */
		tmp->eventq = g_slist_append(tmp->eventq, e);

		/* 
		 * create RDR creation event
		 */	
		/* note:  reusing e; okay so long as we don't do a free(e) before */
		e = (struct oh_event *)malloc(sizeof(*e));
		if (!e) {
			dbg("unable to allocate event");
			return SA_ERR_HPI_OUT_OF_SPACE;
		}
		memset(e, '\0', sizeof(*e));
		e->type = OH_ET_RDR;
		e->u.rdr_event.rdr.RecordId = 0; /* set to 0 b/c first -- and only -- RDR*/
		e->u.rdr_event.rdr.RdrType = SAHPI_WATCHDOG_RDR;
		e->u.rdr_event.rdr.RdrTypeUnion.WatchdogRec.WatchdogNum = 
			SAHPI_DEFAULT_WATCHDOG_NUM; /* set to default b/c only wdt */
		e->u.rdr_event.rdr.RdrTypeUnion.WatchdogRec.Oem = 0; /* n/a */
		e->u.rdr_event.rdr.Entity.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD;
		e->u.rdr_event.rdr.Entity.Entry[0].EntityInstance = 0;
		ep_concat( &(e->u.rdr_event.rdr.Entity), &g_epbase);
		e->u.rdr_event.rdr.IdString.DataType = SAHPI_TL_TYPE_ASCII6;
		e->u.rdr_event.rdr.IdString.Language = SAHPI_LANG_ENGLISH;
		e->u.rdr_event.rdr.IdString.DataLength = 8;
		strcpy(e->u.rdr_event.rdr.IdString.Data, "Watchdog");

		/* add event */
		tmp->eventq = g_slist_append(tmp->eventq, e);

		/* add RDR */
        	if (oh_add_rdr(tmp->rptcache, puid, &e->u.rdr_event.rdr, NULL, 0)) {
                	dbg("unable to add RDR to RPT");
                	return SA_ERR_HPI_ERROR;
		}
	}
	
	return 0;
}

/**
 * watchdog_get_watchdog_info:
 * @hnd: void pointer to handler
 * @id: RDR for watchdog
 * @wdt: pointer to watchdog info sent back
 *
 * Return watchdog information.
 *
 * Return value: 0 for success | Error code
 **/
static int watchdog_get_watchdog_info(void *hnd, SaHpiResourceIdT id,
		                      SaHpiWatchdogNumT num,
				      SaHpiWatchdogT *wdt)
{
	struct oh_handler_state *i = (struct oh_handler_state *)hnd;
	struct wdtitems *wdtitems;

	if (!i) {
		dbg("no handler given");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	wdtitems = i->data;
	if (!wdtitems) {
		dbg("no watchdog info with this handler");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	/* We only have one possible watchdog: wdtitems->data */

	/* Note that software watchdog does not support all watchdog
	 * functionality, so many items below are overwritten with default
	 * values. */
	wdtitems->data.Log = SAHPI_FALSE;  /* impossible to issue events on timeout */
	wdtitems->data.TimerAction = SAHPI_WA_RESET; /* only reset is supported */
	wdtitems->data.PretimerInterrupt = SAHPI_WPI_NONE; /* can't do pretimer */
	wdtitems->data.PreTimeoutInterval = 0; /* can't do pretimer */
	/* Note:  No need to ask wdt for timeout value, since this should
	 * be set correctly already during discover and set. */
	wdtitems->data.PresentCount = 0; /* can't do current count */

	memcpy( wdt, &wdtitems->data, sizeof(SaHpiWatchdogT));

	return 0;
}

/**
 * watchdog_set_watchdog_info:
 * @hnd: void pointer to handler
 * @id: RDR for watchdog
 * @wdt: pointer to watchdog info sent to fcn
 *
 * Set watchdog information in hnd->data to that sent in wdt.
 * Also, stop or restart the watchdog timer, depending on
 * the value of wdt->Running.  If it is set to TRUE and wdt
 * is already running, it is restarted.  Otherwise, it will stay
 * stopped.  If it is set to FALSE, it will stop the timer.
 *
 * Note:  Assuming that lines in the SAF HPI spec that discuss
 * what to do when InitialCount == 0 apply only when
 * wdt is already running and wdt->Running is TRUE.
 *
 * Return value: 0 for success | Error code
 **/
static int watchdog_set_watchdog_info(void *hnd, SaHpiResourceIdT id,
		                                 SaHpiWatchdogNumT num,
						 SaHpiWatchdogT *wdt)
{
	int ret = 0;
	struct oh_handler_state *i = (struct oh_handler_state *)hnd;
	struct wdtitems *wdtitems;
	SaHpiWatchdogT w;

	if (!i) {
		dbg("no handler given");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	wdtitems = i->data;
	if (!wdtitems) {
		dbg("no watchdog info with this handler");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	w = wdtitems->data;
	/* We only have one possible watchdog: wdtitems->data, or w */

	if (SAHPI_FALSE != wdt->Log) {
		/* impossible to issue events on timeout */
		dbg("Request for unsupported watchdog action");
		ret = SA_ERR_HPI_INVALID_PARAMS;
	}
	w.Log = SAHPI_FALSE;

	w.InitialCount = wdt->InitialCount;
	if (SAHPI_TRUE == wdt->Running) {
		if (SAHPI_TRUE == w.Running) {
			/* restart timer */
			int timeout;

			/* reset timeout */
			timeout = wdt->InitialCount / 1000;
			if (0 == wdt->InitialCount) {
				/* timeout in 1ms ~= immediately */
				timeout = 1;
			}

			if ( -1 == ioctl(wdtitems->fd, WDIOC_SETTIMEOUT, &timeout)) {
				dbg("unable to set watchdog timeout");
				ret = SA_ERR_HPI_ERROR;
			}
			/* we read the timeout value after writing */
			/* because some watchdog device can only be set */
			/* to descrete values, so if we want to keep */
			/* the InitialCount accurate then we need to */
			/* ask the watchdog device what it really set */
			/* the timeout too */
			if ( -1 == ioctl(wdtitems->fd, WDIOC_GETTIMEOUT, &timeout)) {
				dbg("unable to read watchdog timeout");
				ret = SA_ERR_HPI_ERROR;
			}
			w.InitialCount = timeout * 1000;
	
			/* pat the dog to restart the timer from the initial
 			* countdown value */
			dbg("reset the watchdog");
			if (-1 == write(wdtitems->fd, "1", 1)) {
				dbg("could not reset watchdog");
				ret = SA_ERR_HPI_ERROR;
			}
		} 
		/* if w.Running == SAHPI_FALSE, wdt remains stopped */
	} else {
		if (SAHPI_TRUE == w.Running) {
			/* stop the watchdog device */
			dbg("Stop watchdog");
			if (-1 == write(wdtitems->fd, "V", 1)) {
				dbg("Unable to write to watchdog");
				ret = SA_ERR_HPI_ERROR;
			}
			close(wdtitems->fd);
			w.Running = SAHPI_FALSE;
		}
		/* if w.Running == SAHPI_FALSE, wdt remains stopped */
	}

	w.TimerUse = wdt->TimerUse;

	if (SAHPI_WA_RESET != wdt->TimerAction) {
		/* only reset is supported */
		dbg("Request for unsupported watchdog action");
		ret = SA_ERR_HPI_INVALID_PARAMS;
	}
	w.TimerAction = SAHPI_WA_RESET;

	if (SAHPI_WPI_NONE != wdt->PretimerInterrupt ||
	    0 != wdt->PreTimeoutInterval) {
		/* we have no way of doing a pre-timeout interrupt */
		dbg("pretimeout functionality is not available");
		ret = SA_ERR_HPI_INVALID_PARAMS;
	}
	w.PretimerInterrupt = SAHPI_WPI_NONE;
	w.PreTimeoutInterval = 0;

	w.TimerUseExpFlags = wdt->TimerUseExpFlags; 

	/* According to SaHpi.h, PresentCount should be ignored in this call */

	wdtitems->data = w;
	return ret;
}

/**
 * watchdog_reset_watchdog:
 * @hnd: void pointer to handler
 * @id: RDR for watchdog
 *
 * Reset the watchdog timer from the initial countdown value.
 *
 * Return value: 0 for success | Error code
 **/
static int watchdog_reset_watchdog(void *hnd, SaHpiResourceIdT id,
		                              SaHpiWatchdogNumT num)
{
	struct oh_handler_state *i = (struct oh_handler_state *)hnd;
	struct wdtitems *wdtitems;

	if (!i) {
		dbg("no handler given");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	wdtitems = i->data;
	if (!wdtitems) {
		dbg("no watchdog info with this handler");
		return SA_ERR_HPI_INVALID_PARAMS;
	}

	if (wdtitems->data.Running == SAHPI_FALSE) {
		int timeout;

		dbg("start up the watchdog");
		/* calling reset on stopped watchdog will */
		/* cause the watchdog to start            */
		wdtitems->fd = open(wdtitems->path, O_RDWR);
		if (-1 == wdtitems->fd) {
			dbg("could not open watchdog device");
			return SA_ERR_HPI_ERROR;
		}
		wdtitems->data.Running = SAHPI_TRUE;

		timeout = wdtitems->data.InitialCount / 1000;
		if ( -1 == ioctl(wdtitems->fd, WDIOC_SETTIMEOUT, &timeout)) {
			dbg("unable to set watchdog timeout");
			return SA_ERR_HPI_ERROR;
		}
		/* we read the timeout value after writing */
		/* because some watchdog device can only be set */
		/* to descrete values, so if we want to keep */
		/* the InitialCount accurate then we need to */
		/* ask the watchdog device what it really set */
		/* the timeout too */
		if ( -1 == ioctl(wdtitems->fd, WDIOC_GETTIMEOUT, &timeout)) {
			dbg("unable to read watchdog timeout");
			return SA_ERR_HPI_ERROR;
		}
		wdtitems->data.InitialCount = timeout * 1000;
	}

	/* pat the dog to restart the timer from the initial
	 * countdown value */
	dbg("reset the watchdog");
	if (-1 == write(wdtitems->fd, "1", 1)) {
		dbg("unable to reset the watchdog");
		return SA_ERR_HPI_ERROR;
	}

	return 0;
}

struct oh_abi_v2 oh_watchdog_plugin = {
	.open			= watchdog_open,
	.close			= watchdog_close,
	.get_event		= watchdog_get_event,
	.discover_resources     = watchdog_discover_resources,
	.get_watchdog_info	= watchdog_get_watchdog_info,
	.set_watchdog_info	= watchdog_set_watchdog_info,
	.reset_watchdog		= watchdog_reset_watchdog
};

int get_interface(void **pp, const uuid_t uuid)
{
	if (uuid_compare(uuid, UUID_OH_ABI_V2)==0) {
		*pp = &oh_watchdog_plugin;
		return 0;
	}

	*pp = NULL;
	return -1;
}
