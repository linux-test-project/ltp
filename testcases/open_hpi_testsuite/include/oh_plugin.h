/*      -*- linux-c -*-
 *
 * (C) Copright IBM Corp 2004-2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Renier Morales <renier@users.sourceforge.net>
 */

#ifndef __OH_PLUGIN_H
#define __OH_PLUGIN_H

#include <glib.h>
#include <oh_handler.h>

/*
 * Plugins are kept in a list within the oh_plugins struct
 */
struct oh_plugins {
        GSList *list;
        GStaticRecMutex lock;
};
struct oh_plugin {
        char *name; /* Name of plugin preceded by 'lib' (e.g. "libdummy"). */
        /* handle returned by lt_dlopenext or 0 for static plugins */
        void *dl_handle;
        struct oh_abi_v2 *abi; /* pointer to associated plugin interface */
        int handler_count; /* How many handlers use this plugin */

        /* Synchronization - used internally by plugin interfaces below. */
        GStaticRecMutex lock; /* Exclusive lock for working with plugin */
        /* These are used to keep the plugin from being unloaded while it
         * is being referenced.
         */
        int refcount;
        GStaticRecMutex refcount_lock;
};
extern struct oh_plugins oh_plugins;

/*
 * Representation of a handler (plugin instance)
 */
struct oh_handlers {
        GHashTable *table;
        GSList *list;
        GStaticRecMutex lock;
};
struct oh_handler {
        unsigned int id; /* id of handler */
        char *plugin_name;
        GHashTable *config; /* pointer to handler configuration */
        struct oh_abi_v2 *abi; /* pointer to associated plugin interface */
        /*
         * private pointer used by plugin implementations to distinguish
         * between different instances
         */
        void *hnd;

        /* Synchronization - used internally by handler interfaces below. */
        GStaticRecMutex lock; /* Exclusive lock for working with handler */
        /* These are used to keep the handler from being destroyed while it
         * is being referenced.
         */
        int refcount;
        GStaticRecMutex refcount_lock;
};
extern struct oh_handlers oh_handlers;

/* Finalization of plugins and handlers. */
void oh_close_handlers(void);

/* Plugin interface functions  */
struct oh_plugin *oh_get_plugin(char *plugin_name);
void oh_release_plugin(struct oh_plugin *plugin);
int oh_getnext_plugin_name(char *plugin_name,
                           char *next_plugin_name,
                           unsigned int size);
int oh_load_plugin(char *plugin_name);
int oh_unload_plugin(char *plugin_name);

/* Handler (plugin instances) interface functions */
struct oh_handler *oh_get_handler(unsigned int hid);
void oh_release_handler(struct oh_handler *handler);
int oh_getnext_handler_id(unsigned int hid, unsigned int *next_hid);
SaErrorT oh_create_handler(GHashTable *handler_config, unsigned int *hid);
int oh_destroy_handler(unsigned int hid);
SaErrorT oh_discovery(void);

/* Bind abi functions into plugin */
int oh_load_plugin_functions(struct oh_plugin *plugin, struct oh_abi_v2 **abi);

#endif /*__OH_PLUGIN_H*/
