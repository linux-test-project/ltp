/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Renier Morales <renier@openhpi.org>
 *
 */

#include <oh_threaded.h>
#include <oh_config.h>
#include <oh_plugin.h>
#include <oh_hotswap.h>
#include <oh_error.h>

#define OH_DISCOVERY_THREAD_SLEEP_TIME 180 * G_USEC_PER_SEC
#define OH_EVTGET_THREAD_SLEEP_TIME 3 * G_USEC_PER_SEC

GCond *oh_evtget_thread_wait = NULL;
GThread *oh_evtget_thread = NULL;
GError *oh_evtget_thread_error = NULL;
GMutex *oh_evtget_thread_mutex = NULL;
GStaticMutex oh_wake_evtget_mutex = G_STATIC_MUTEX_INIT;

GThread *oh_evtpop_thread = NULL;
GError *oh_evtpop_thread_error = NULL;
GMutex *oh_evtpop_thread_mutex = NULL;

GThread *oh_discovery_thread = NULL;
GError *oh_discovery_thread_error = NULL;
GMutex *oh_discovery_thread_mutex = NULL;
GCond *oh_discovery_thread_wait = NULL;
GStaticMutex oh_wake_discovery_mutex = G_STATIC_MUTEX_INIT;

static int oh_discovery_init(void)
{
        /* Nothing to do here...for now */
        return 0;
}

static int oh_discovery_final(void)
{
        g_mutex_free(oh_discovery_thread_mutex);
        g_cond_free(oh_discovery_thread_wait);

        return 0;
}

static int oh_event_final(void)
{
        /*g_async_queue_unref(oh_process_q);*/
        g_mutex_free(oh_evtget_thread_mutex);
        g_cond_free(oh_evtget_thread_wait);
        g_mutex_free(oh_evtpop_thread_mutex);

        return 0;
}

static gpointer oh_discovery_thread_loop(gpointer data)
{
        GTimeVal time;
        SaErrorT error = SA_OK;

        g_mutex_lock(oh_discovery_thread_mutex);
        while (1) {
                dbg("Doing threaded discovery on all handlers");
                error = oh_discovery();
                if (error) {
                        dbg("Got error on threaded discovery return.");
                }

                /* Let oh_wake_discovery_thread know this thread is done */
                g_cond_broadcast(oh_discovery_thread_wait);
                g_get_current_time(&time);
                g_time_val_add(&time, OH_DISCOVERY_THREAD_SLEEP_TIME);
                /* Go to sleep; let oh_wake_discovery_thread take the mutex */
                dbg("Going to sleep");
                if (g_cond_timed_wait(oh_discovery_thread_wait,
                                      oh_discovery_thread_mutex, &time))
                        dbg("SIGNALED: Got signal from saHpiDiscover()");
                else
                        dbg("TIMEDOUT: Woke up, am doing discovery again");
        }
        g_mutex_unlock(oh_discovery_thread_mutex);
        g_thread_exit(0);

        return data;
}

static gpointer oh_evtpop_thread_loop(gpointer data)
{
        SaErrorT error = SA_OK;

        g_mutex_lock(oh_evtpop_thread_mutex);
        while(1) {
                dbg("Thread processing events");
                error = oh_process_events();
                if (error != SA_OK) err("Error on processing of events.");
        }
        g_mutex_unlock(oh_evtpop_thread_mutex);
        g_thread_exit(0);

        return data;
}

static gpointer oh_evtget_thread_loop(gpointer data)
{
        GTimeVal time;
        SaErrorT error = SA_OK;
        static int first_loop = 1;

        g_mutex_lock(oh_evtget_thread_mutex);
        while (1) {
                /* Give the discovery time to start first -> FIXME */
                if (first_loop) {
                        struct timespec sleepytime =
                                { .tv_sec = 0, .tv_nsec = 500000000};
                        first_loop = 0;
                        nanosleep(&sleepytime, NULL);
                }

                dbg("Thread Harvesting events");
                error = oh_harvest_events();
                if (error != SA_OK) err("Error on harvest of events.");

                /* Let oh_wake_evtget_thread know this thread is done */
                g_cond_broadcast(oh_evtget_thread_wait);
                g_get_current_time(&time);
                g_time_val_add(&time, OH_EVTGET_THREAD_SLEEP_TIME);
                dbg("Going to sleep");
                if (g_cond_timed_wait(oh_evtget_thread_wait, oh_evtget_thread_mutex, &time))
                        dbg("SIGNALED: Got signal from plugin");
                else
                        dbg("TIMEDOUT: Woke up, am looping again");
        }
        g_mutex_unlock(oh_evtget_thread_mutex);
        g_thread_exit(0);

        return data;
}

int oh_threaded_init()
{
        int error = 0;

        dbg("Attempting to init event");
        if (!g_thread_supported()) {
                dbg("Initializing thread support");
                g_thread_init(NULL);
        } else {
                dbg("Already supporting threads");
        }

        error = oh_event_init();
        if (oh_discovery_init() || error) error = 1;

        return error;
}

int oh_threaded_start()
{
        dbg("Starting discovery thread");
        oh_discovery_thread_wait = g_cond_new();
        oh_discovery_thread_mutex = g_mutex_new();
        oh_discovery_thread = g_thread_create(oh_discovery_thread_loop,
                                NULL, FALSE,
                                &oh_discovery_thread_error);

        dbg("Starting event threads");
        oh_evtget_thread_wait = g_cond_new();
        oh_evtget_thread_mutex = g_mutex_new();
        oh_evtget_thread = g_thread_create(oh_evtget_thread_loop,
                                NULL, FALSE, &oh_evtget_thread_error);
        oh_evtpop_thread_mutex = g_mutex_new();
        oh_evtpop_thread = g_thread_create(oh_evtpop_thread_loop,
                                NULL, FALSE, &oh_evtpop_thread_error);

        return 0;
}

int oh_threaded_final()
{
        oh_discovery_final();
        oh_event_final();

        return 0;
}

/**
 * oh_wake_discovery_thread
 * @wait: Says whether we should wait for the discovery thread
 * to do one round through the plugin instances. Otherwise, we
 * just knock on the discovery thread's door and return quickly.
 *
 * If wait is true, the discovery thread is woken up
 * and we wait until it does a round throughout the
 * plugin instances. If the thread is already running,
 * we will wait for it until it completes the round.
 *
 * Returns: void
 **/
void oh_wake_discovery_thread(SaHpiBoolT wait)
{
        if (!wait) { /* If not waiting, just signal the thread and go. */
                g_cond_broadcast(oh_discovery_thread_wait);
                return;
        }

        g_static_mutex_lock(&oh_wake_discovery_mutex);
        if (g_mutex_trylock(oh_discovery_thread_mutex)) {
                /* The thread was asleep; wake it up. */
                dbg("Going to wait for discovery thread to loop once.");
                g_cond_broadcast(oh_discovery_thread_wait);
                g_cond_wait(oh_discovery_thread_wait,
                                oh_discovery_thread_mutex);
                dbg("Got signal from discovery"
                    " thread being done. Giving lock back");
                g_mutex_unlock(oh_discovery_thread_mutex);
        } else {
                /* Thread was already up. Wait until it completes */
                dbg("Waiting for discovery thread...");
                g_mutex_lock(oh_discovery_thread_mutex);
                dbg("...Done waiting for discovery thread.");
                g_mutex_unlock(oh_discovery_thread_mutex);
        }
        g_static_mutex_unlock(&oh_wake_discovery_mutex);

        return;
}

/**
 * oh_wake_event_thread
 * @wait: Says whether we should wait for the event thread
 * to do one round through the plugin instances. Otherwise, we
 * just knock on the event thread's door and return quickly.
 *
 * If wait is true, the event thread is woken up
 * and we wait until it does a round throughout the
 * plugin instances. If the thread is already running,
 * we will wait for it until it completes the round.
 *
 * Returns: void
 **/
void oh_wake_event_thread(SaHpiBoolT wait)
{
        if (!wait) { /* If not waiting, just signal the thread and go. */
                g_cond_broadcast(oh_evtget_thread_wait);
                return;
        }

        g_static_mutex_lock(&oh_wake_evtget_mutex);
        if (g_mutex_trylock(oh_evtget_thread_mutex)) {
                /* The thread was asleep; wake it up. */
                dbg("Going to wait for event thread to loop once.");
                g_cond_broadcast(oh_evtget_thread_wait);
                g_cond_wait(oh_evtget_thread_wait,
                            oh_evtget_thread_mutex);
                dbg("Got signal from event"
                    " thread being done. Giving lock back");
                g_mutex_unlock(oh_evtget_thread_mutex);
        } else {
                /* Thread was already up. Wait until it completes */
                dbg("Waiting for event thread...");
                g_mutex_lock(oh_evtget_thread_mutex);
                dbg("...Done waiting for event thread.");
                g_mutex_unlock(oh_evtget_thread_mutex);
        }
        g_static_mutex_unlock(&oh_wake_evtget_mutex);

        return;
}
