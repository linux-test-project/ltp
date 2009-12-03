/*      -*- linux-c -*-
 *
 * Copyright (c) 2003 by Intel Corp.
 * (C) Copyright IBM Corp. 2003, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     Sean Dague <http://dague.net/sean>
 *     David Judkovics <djudkovi@us.ibm.com>
 *     Renier Morales <renier@openhpi.org>
 *     Anton Pak <anton.pak@pigeonpoint.com>
 */

#include <string.h>
#include <ltdl.h>

#include <glib.h>
#include <oh_plugin.h>
#include <oh_config.h>
#include <oh_error.h>
#include <oh_lock.h>
#include <oh_domain.h>
#include <config.h>

/*
 * Structure containing global list of plugins (oh_plugin).
 */
struct oh_plugins oh_plugins = {
        .list = NULL,
        .lock = G_STATIC_REC_MUTEX_INIT
};

/*
 * Structure containing global table of handlers (oh_handler).
 */
struct oh_handlers oh_handlers = {
        .table = NULL,
        .list = NULL,
        .lock = G_STATIC_REC_MUTEX_INIT
};

/**
 * oh_close_handlers
 *
 * When a client calls exit() this function
 * gets called to clean up the handlers
 * and any handler connections to the hardware.
 * Until clients use dynamic oHPI interface
 *
 * Returns: void
 **/
void oh_close_handlers()
{
        struct oh_handler *h = NULL;
        GSList *node = NULL;

        g_static_rec_mutex_lock(&oh_handlers.lock);
        if (oh_handlers.list == NULL) {
                g_static_rec_mutex_unlock(&oh_handlers.lock);
                return;
        }

        for (node = oh_handlers.list; node; node = node->next) {
                h = node->data;
                if (h && h->abi && h->abi->close) {
                        h->abi->close(h->hnd);
                }
        }
        g_static_rec_mutex_unlock(&oh_handlers.lock);
}

/**
 * oh_exit_ltdl
 *
 * Does everything needed to close the ltdl structures.
 *
 * Returns: 0 on Success.
 **/
static int oh_exit_ltdl(void)
{
        int rv;

        rv = lt_dlexit();
        if (rv < 0) {
                err("Could not exit ltdl!");
                return -1;
        }

        return 0;
}

/**
 * oh_init_ltdl
 *
 * Does all the initialization needed for the ltdl process to
 * work. It takes no arguments, and returns 0 on success, < 0 on error
 *
 * Returns: 0 on Success.
 **/
static int oh_init_ltdl(void)
{
        struct oh_global_param path_param = { .type = OPENHPI_PATH };
        int err;
        static int init_done = 0;

        data_access_lock();
        if (init_done) {
                data_access_unlock();
                return 0;
        }

        err = lt_dlinit();
        if (err != 0) {
                err("Can not init ltdl");
                data_access_unlock();
                return -1;
        }

        oh_get_global_param(&path_param);

        err = lt_dlsetsearchpath(path_param.u.path);
        if (err != 0) {
                err("Can not set lt_dl search path");
                oh_exit_ltdl();
                data_access_unlock();
                return -1;
        }

        init_done = 1;
        data_access_unlock();

        return 0;
}

static void __inc_plugin_refcount(struct oh_plugin *p)
{
        g_static_rec_mutex_lock(&p->refcount_lock);
        p->refcount++;
        g_static_rec_mutex_unlock(&p->refcount_lock);
}

static void __dec_plugin_refcount(struct oh_plugin *p)
{
        g_static_rec_mutex_lock(&p->refcount_lock);
        p->refcount--;
        g_static_rec_mutex_unlock(&p->refcount_lock);
}

static void __delete_plugin(struct oh_plugin *p)
{
        if (!p) return;

        g_free(p->name);
        g_static_rec_mutex_free(&p->lock);
        g_static_rec_mutex_free(&p->refcount_lock);
        g_free(p->abi);
        if (p->dl_handle) {
                lt_dlclose(p->dl_handle);
        }
        g_free(p);
}

/**
 * oh_get_plugin
 * @plugin_name: name of plugin. string.
 *
 * Lookup and get a reference for @plugin_name.
 * Locks out the plugin. Need to call oh_putback_plugin
 * to unlock it.
 *
 * Returns: oh_plugin reference or NULL if plugin_name was not found.
 **/
struct oh_plugin *oh_get_plugin(char *plugin_name)
{
        GSList *node = NULL;
        struct oh_plugin *plugin = NULL;

        if (!plugin_name) {
                err("ERROR getting plugin. Invalid parameter.");
                return NULL;
        }

        g_static_rec_mutex_lock(&oh_plugins.lock);
        for (node = oh_plugins.list; node; node = node->next) {
                struct oh_plugin *p = (struct oh_plugin *)(node->data);
                if (strcmp(p->name, plugin_name) == 0) {
                        __inc_plugin_refcount(p);
                        g_static_rec_mutex_unlock(&oh_plugins.lock);
                        g_static_rec_mutex_lock(&p->lock);
                        plugin = p;
                        return plugin;
                }
        }
        g_static_rec_mutex_unlock(&oh_plugins.lock);

        return plugin;
}

/**
 * oh_release_plugin
 * @plugin: Pointer to plugin
 *
 * Decrements refcount on plugin and unlocks it.
 *
 * Returns: void
 **/
void oh_release_plugin(struct oh_plugin *plugin)
{
        if (!plugin) {
                err("WARNING - NULL plugin parameter passed.");
                return;
        }

        __dec_plugin_refcount(plugin);
        if (plugin->refcount < 0)
                __delete_plugin(plugin);
        else
                g_static_rec_mutex_unlock(&plugin->lock);
}

/**
 * oh_getnext_plugin_name
 * @plugin_name: IN. If NULL is passed, the first plugin in the list is returned.
 * @next_plugin_name: OUT. Buffer to print the next plugin name to.
 * @size: IN. Size of the buffer at @next_plugin_name.
 *
 * Returns: 0 on Success.
 **/
int oh_getnext_plugin_name(char *plugin_name,
                           char *next_plugin_name,
                           unsigned int size)
{
        GSList *node = NULL;

        if (!next_plugin_name) {
                err("ERROR. Invalid parameter.");
                return -1;
        }
        memset(next_plugin_name, '\0', size);

        if (!plugin_name) {
                g_static_rec_mutex_lock(&oh_plugins.lock);
                if (oh_plugins.list) {
                        struct oh_plugin *plugin = oh_plugins.list->data;
                        strncpy(next_plugin_name, plugin->name, size);
                        g_static_rec_mutex_unlock(&oh_plugins.lock);
                        return 0;
                } else {
                        g_static_rec_mutex_unlock(&oh_plugins.lock);
                        dbg("No plugins have been loaded yet.");
                        return -1;
                }
        } else {
                g_static_rec_mutex_lock(&oh_plugins.lock);
                for (node = oh_plugins.list; node; node = node->next) {
                        struct oh_plugin *p = node->data;
                        if (strcmp(p->name, plugin_name) == 0) {
                                if (node->next) {
                                        p = node->next->data;
                                        strncpy(next_plugin_name, p->name, size);
                                        g_static_rec_mutex_unlock(&oh_plugins.lock);
                                        return 0;
                                } else {
                                        break;
                                }
                        }
                }
                g_static_rec_mutex_unlock(&oh_plugins.lock);
        }

        return -1;
}

/* list of static plugins. defined in plugin_static.c.in */
extern struct oh_static_plugin static_plugins[];
/**
 * oh_load_plugin
 * @plugin_name: name of plugin to be loaded (e.g. "libdummy").
 *
 * Load plugin by name
 *
 * Returns: 0 on Success.
 **/
int oh_load_plugin(char *plugin_name)
{

        struct oh_plugin *plugin = NULL;
        struct oh_static_plugin *p = static_plugins;
        int err;

        if (!plugin_name) {
                err("ERROR. NULL plugin name passed.");
                return -1;
        }

        if (oh_init_ltdl()) {
                err("ERROR. Could not initialize ltdl for loading plugins.");
                return -1;
        }

        plugin = oh_get_plugin(plugin_name);
        if (plugin) {
                oh_release_plugin(plugin);
                dbg("Plugin %s already loaded. Not loading twice.",
                    plugin_name);
                return 0;
        }

        plugin = (struct oh_plugin *)g_malloc0(sizeof(struct oh_plugin));
        if (!plugin) {
                err("Out of memory.");
                return -1;
        }
        plugin->name = g_strdup(plugin_name);
        plugin->handler_count = 0;
        plugin->refcount = 0;
        g_static_rec_mutex_init(&plugin->lock);
        g_static_rec_mutex_init(&plugin->refcount_lock);

        /* first take search plugin in the array of static plugin */
        while (p->name) {
                if (!strcmp(plugin->name, p->name)) {
                        plugin->dl_handle = 0;
                        err = (*p->get_interface)((void **)&plugin->abi, UUID_OH_ABI_V2);

                        if (err < 0 || !plugin->abi || !plugin->abi->open) {
                                err("Can not get ABI V2");
                                goto cleanup_and_quit;
                        }

                        dbg("found static plugin %s", p->name);

                        g_static_rec_mutex_lock(&oh_plugins.lock);
                        oh_plugins.list = g_slist_append(oh_plugins.list, plugin);
                        g_static_rec_mutex_unlock(&oh_plugins.lock);

                        return 0;
                }
                p++;
        }

        plugin->dl_handle = lt_dlopenext(plugin->name);
        if (plugin->dl_handle == NULL) {
                err("Can not open %s plugin: %s", plugin->name, lt_dlerror());
                goto cleanup_and_quit;
        }

        err = oh_load_plugin_functions(plugin, &plugin->abi);

        if (err < 0 || !plugin->abi || !plugin->abi->open) {
                err("Can not get ABI");
                goto cleanup_and_quit;
        }
        g_static_rec_mutex_lock(&oh_plugins.lock);
        oh_plugins.list = g_slist_append(oh_plugins.list, plugin);
        g_static_rec_mutex_unlock(&oh_plugins.lock);

        return 0;
cleanup_and_quit:
        __delete_plugin(plugin);
        return -1;
}

/**
 * oh_unload_plugin
 * @plugin_name: String. Name of plugin to unload.
 *
 * Returns: 0 on Success.
 **/
int oh_unload_plugin(char *plugin_name)
{
        struct oh_plugin *plugin = NULL;

        if (!plugin_name) {
                err("ERROR unloading plugin. NULL parameter passed.");
                return -1;
        }

        plugin = oh_get_plugin(plugin_name);
        if (!plugin) {
                err("ERROR unloading plugin. Plugin not found.");
                return -2;
        }

        if (plugin->handler_count > 0) {
                oh_release_plugin(plugin);
                err("ERROR unloading plugin. Handlers are still referencing it.");
                return -3;
        }

        g_static_rec_mutex_lock(&oh_plugins.lock);
        oh_plugins.list = g_slist_remove(oh_plugins.list, plugin);
        g_static_rec_mutex_unlock(&oh_plugins.lock);

        __dec_plugin_refcount(plugin);
        if (plugin->refcount < 1)
                __delete_plugin(plugin);
        else
                oh_release_plugin(plugin);

        return 0;
}

static void __inc_handler_refcount(struct oh_handler *h)
{
        g_static_rec_mutex_lock(&h->refcount_lock);
        h->refcount++;
        g_static_rec_mutex_unlock(&h->refcount_lock);
}

static void __dec_handler_refcount(struct oh_handler *h)
{
        g_static_rec_mutex_lock(&h->refcount_lock);
        h->refcount--;
        g_static_rec_mutex_unlock(&h->refcount_lock);
}

static void __delete_handler(struct oh_handler *h)
{
        struct oh_plugin *plugin = NULL;

        if (!h) return;

        /* Subtract one from the number of handlers using this plugin */
        plugin = oh_get_plugin(h->plugin_name);
        if (!plugin) {
                err("BAD ERROR - Handler loaded, but plugin does not exist!");
        } else {
                plugin->handler_count--;
                oh_release_plugin(plugin);
        }

        /* Free the oh_handler members first, then the handler. */
        /* FIXME: Where/When should the handler config table be freed? */
        g_static_rec_mutex_free(&h->lock);
        g_static_rec_mutex_free(&h->refcount_lock);
        g_free(h);
}

/**
 * oh_get_handler
 * @hid: id of handler being requested
 *
 * Returns: NULL if handler was not found.
 **/
struct oh_handler *oh_get_handler(unsigned int hid)
{
        GSList *node = NULL;
        struct oh_handler *handler = NULL;

        g_static_rec_mutex_lock(&oh_handlers.lock);
        node = g_hash_table_lookup(oh_handlers.table, &hid);
        handler = node ? node->data : NULL;
        if (!handler) {
                g_static_rec_mutex_unlock(&oh_handlers.lock);
                err("Error - Handler %d was not found", hid);
                return NULL;
        }
        __inc_handler_refcount(handler);
        g_static_rec_mutex_unlock(&oh_handlers.lock);
        g_static_rec_mutex_lock(&handler->lock);

        return handler;
}

/**
 * oh_release_handler
 * @handler: a handler, previously obtained (i.e. locked) with
 * oh_get_handler(), to be released (i.e. unlocked).
 *
 * Returns: void
 **/
void oh_release_handler(struct oh_handler *handler)
{
        if (!handler) {
                err("Warning - NULL parameter passed.");
                return;
        }

        __dec_handler_refcount(handler);
        if (handler->refcount < 0)
                __delete_handler(handler);
        else
                g_static_rec_mutex_unlock(&handler->lock);
}

/**
 * oh_getnext_handler_id
 * @hid: If 0, will return the first handler id in the list.
 * Otherwise, indicates handler id previous to the one being requested.
 * @next_hid: Place where the next handler id after @hid
 * will be put.
 *
 * Returns: 0 on Success.
 **/
int oh_getnext_handler_id(unsigned int hid, unsigned int *next_hid)
{
        GSList *node = NULL;
        struct oh_handler *h = NULL;

        if (!next_hid) {
                err("ERROR. Invalid parameter.");
                return -1;
        }
        *next_hid = 0;

        if (!hid) { /* Return first handler id in the list */
                g_static_rec_mutex_lock(&oh_handlers.lock);
                if (oh_handlers.list) {
                        h = oh_handlers.list->data;
                        *next_hid = h->id;
                        g_static_rec_mutex_unlock(&oh_handlers.lock);
                        return 0;
                } else {
                        g_static_rec_mutex_unlock(&oh_handlers.lock);
                        err("Warning - no handlers");
                        return -1;
                }
        } else { /* Return handler id coming after hid in the list */
                g_static_rec_mutex_lock(&oh_handlers.lock);
                node = g_hash_table_lookup(oh_handlers.table, &hid);
                if (node && node->next && node->next->data) {
                        h = node->next->data;
                        *next_hid = h->id;
                        g_static_rec_mutex_unlock(&oh_handlers.lock);
                        return 0;
                }
        }
        g_static_rec_mutex_unlock(&oh_handlers.lock);

        return -1;
}

static struct oh_handler *new_handler(GHashTable *handler_config)
{       /* Return a new oh_handler instance */
        struct oh_plugin *plugin = NULL;
        struct oh_handler *handler = NULL;
	char *plugin_name = NULL;
        static unsigned int handler_id = 1;

        if (!handler_config) {
                err("ERROR creating new handler. Invalid parameter.");
                return NULL;
        }

	plugin_name = (char *)g_hash_table_lookup(handler_config, "plugin");
	if (!plugin_name) {
		err("ERROR creating new handler. No plugin name received.");
		return NULL;
	}

        handler = (struct oh_handler *)g_malloc0(sizeof(struct oh_handler));

        plugin = oh_get_plugin(plugin_name);
        if(!plugin) { /* Attempt to load plugin here once */
		int rc = oh_load_plugin(plugin_name);
		if (rc) {
                	err("Could not create handler. Plugin %s not loaded",
                    	    plugin_name);
                	goto cleanexit;
		}

		plugin = oh_get_plugin(plugin_name);
		if (!plugin) {
			err("Tried but could not get a plugin to "
			    "create this handler.");
			goto cleanexit;
		}
        }

        /* Initialize handler */
        handler->abi = plugin->abi;
        plugin->handler_count++; /* Increment # of handlers using the plugin */
        oh_release_plugin(plugin);
        g_static_rec_mutex_lock(&oh_handlers.lock);
        handler->id = handler_id++;
        g_static_rec_mutex_unlock(&oh_handlers.lock);
        handler->plugin_name = (char *)g_hash_table_lookup(handler_config, "plugin");
        handler->config = handler_config;
        handler->refcount = 0;
        g_static_rec_mutex_init(&handler->lock);
        g_static_rec_mutex_init(&handler->refcount_lock);

        return handler;
cleanexit:
        g_free(handler);
        return NULL;
}

/**
 * oh_create_handler
 * @handler_config: Hash table containing the configuration for a handler
 * read from the configuration file.
 * @hid: pointer where hid of newly created handler will be stored.
 *
 * Returns: SA_OK on success. If handler failed to open, then @hid will
 * contain a valid handler id, but SA_ERR_HPI_INTERNAL_ERROR will be
 * returned.
 **/
SaErrorT oh_create_handler (GHashTable *handler_config, unsigned int *hid)
{
        struct oh_handler *handler = NULL;

        if (!handler_config || !hid) {
                err("ERROR creating handler. Invalid parameters.");
                return SA_ERR_HPI_INVALID_PARAMS;
        }

	*hid = 0;

        handler = new_handler(handler_config);
        if (!handler) return SA_ERR_HPI_ERROR;

        *hid = handler->id;
        g_static_rec_mutex_lock(&oh_handlers.lock);
        oh_handlers.list = g_slist_append(oh_handlers.list, handler);
        g_hash_table_insert(oh_handlers.table,
                            &(handler->id),
                            g_slist_last(oh_handlers.list));

        handler->hnd = handler->abi->open(handler->config,
                                          handler->id,
                                          &oh_process_q);
        if (!handler->hnd) {
                err("A handler #%d on the %s plugin could not be opened.",
                    handler->id, handler->plugin_name);
		g_static_rec_mutex_unlock(&oh_handlers.lock);
		return SA_ERR_HPI_INTERNAL_ERROR;
        }
        g_static_rec_mutex_unlock(&oh_handlers.lock);

        return SA_OK;
}

/**
 * oh_destroy_handler
 * @hid: Id of handler to destroy
 *
 * Returns: 0 on Success.
 **/
int oh_destroy_handler(unsigned int hid)
{
        struct oh_handler *handler = NULL;

        if (hid < 1) {
                err("ERROR - Invalid handler 0 id passed.");
                return -1;
        }

        handler = oh_get_handler(hid);
        if (!handler) {
                err("ERROR - Handler %d not found.", hid);
                return -1;
        }

        if (handler->abi && handler->abi->close)
                handler->abi->close(handler->hnd);

        g_static_rec_mutex_lock(&oh_handlers.lock);
        g_hash_table_remove(oh_handlers.table, &handler->id);
        oh_handlers.list = g_slist_remove(oh_handlers.list, &(handler->id));
        g_static_rec_mutex_unlock(&oh_handlers.lock);

        __dec_handler_refcount(handler);
        if (handler->refcount < 1)
                __delete_handler(handler);
        else
                oh_release_handler(handler);

        return 0;
}

/**
 * oh_discover_resources
 *
 * Returns: SA_OK on success.
 **/
SaErrorT oh_discovery(void)
{
        unsigned int hid = 0, next_hid;
        struct oh_handler *h = NULL;
        SaErrorT error = SA_ERR_HPI_ERROR;

        oh_getnext_handler_id(hid, &next_hid);
        while (next_hid) {
                hid = next_hid;

                SaErrorT cur_error;

                h = oh_get_handler(hid);
                if (!h) {
                        err("No such handler %d", hid);
                        break;
                }

		if (h->abi->discover_resources && h->hnd) {
                        cur_error = h->abi->discover_resources(h->hnd);
                        if (cur_error == SA_OK && error) {
                                error = cur_error;
                        }
                }
                oh_release_handler(h);
                oh_getnext_handler_id(hid, &next_hid);
        }

        return error;
}

/**
 * oh_load_plugin_functions
 * @plugin: plugin structure.
 * @abi: oh_abi struct
 *
 * This function will load the symbol table from the plugin name and
 * assign the plugin functions to the abi struct.
 *
 * Return value: 0 on success, otherwise any negative value on failure.
 **/
int oh_load_plugin_functions(struct oh_plugin *plugin, struct oh_abi_v2 **abi)
{

        *abi = (struct oh_abi_v2 *)g_malloc0(sizeof(struct oh_abi_v2));


        if (!(*abi)) {
                err("Out of Memory!");
                return -1;
        }

        (*abi)->open                      = lt_dlsym(plugin->dl_handle,
                                                "oh_open");
        (*abi)->close                     = lt_dlsym(plugin->dl_handle,
                                                "oh_close");
        (*abi)->get_event                 = lt_dlsym(plugin->dl_handle,
                                                "oh_get_event");
        (*abi)->discover_resources        = lt_dlsym(plugin->dl_handle,
                                                "oh_discover_resources");
        (*abi)->set_resource_tag          = lt_dlsym(plugin->dl_handle,
                                                "oh_set_resource_tag");
        (*abi)->set_resource_severity     = lt_dlsym(plugin->dl_handle,
                                                "oh_set_resource_severity");
        (*abi)->resource_failed_remove    = lt_dlsym(plugin->dl_handle,
                                                "oh_resource_failed_remove");
        (*abi)->get_el_info               = lt_dlsym(plugin->dl_handle,
                                                "oh_get_el_info");
        (*abi)->get_el_caps               = lt_dlsym(plugin->dl_handle,
                                                "oh_get_el_caps");
        (*abi)->set_el_time               = lt_dlsym(plugin->dl_handle,
                                                "oh_set_el_time");
        (*abi)->add_el_entry              = lt_dlsym(plugin->dl_handle,
                                                "oh_add_el_entry");
        (*abi)->get_el_entry              = lt_dlsym(plugin->dl_handle,
                                                "oh_get_el_entry");
        (*abi)->clear_el                  = lt_dlsym(plugin->dl_handle,
                                                "oh_clear_el");
        (*abi)->set_el_state              = lt_dlsym(plugin->dl_handle,
                                                "oh_set_el_state");
        (*abi)->reset_el_overflow         = lt_dlsym(plugin->dl_handle,
                                                "oh_reset_el_overflow");
        (*abi)->get_sensor_reading        = lt_dlsym(plugin->dl_handle,
                                                "oh_get_sensor_reading");
        (*abi)->get_sensor_thresholds     = lt_dlsym(plugin->dl_handle,
                                                "oh_get_sensor_thresholds");
        (*abi)->set_sensor_thresholds     = lt_dlsym(plugin->dl_handle,
                                                "oh_set_sensor_thresholds");
        (*abi)->get_sensor_enable         = lt_dlsym(plugin->dl_handle,
                                                "oh_get_sensor_enable");
        (*abi)->set_sensor_enable         = lt_dlsym(plugin->dl_handle,
                                                "oh_set_sensor_enable");
        (*abi)->get_sensor_event_enables  = lt_dlsym(plugin->dl_handle,
                                                "oh_get_sensor_event_enables");
        (*abi)->set_sensor_event_enables  = lt_dlsym(plugin->dl_handle,
                                                "oh_set_sensor_event_enables");
        (*abi)->get_sensor_event_masks    = lt_dlsym(plugin->dl_handle,
                                                "oh_get_sensor_event_masks");
        (*abi)->set_sensor_event_masks    = lt_dlsym(plugin->dl_handle,
                                                "oh_set_sensor_event_masks");
        (*abi)->get_control_state         = lt_dlsym(plugin->dl_handle,
                                                "oh_get_control_state");
        (*abi)->set_control_state         = lt_dlsym(plugin->dl_handle,
                                                "oh_set_control_state");
        (*abi)->get_idr_info              = lt_dlsym(plugin->dl_handle,
                                                "oh_get_idr_info");
        (*abi)->get_idr_area_header       = lt_dlsym(plugin->dl_handle,
                                                "oh_get_idr_area_header");
        (*abi)->add_idr_area              = lt_dlsym(plugin->dl_handle,
                                                "oh_add_idr_area");
        (*abi)->add_idr_area_id           = lt_dlsym(plugin->dl_handle,
                                                "oh_add_idr_area_id");
        (*abi)->del_idr_area              = lt_dlsym(plugin->dl_handle,
                                                "oh_del_idr_area");
        (*abi)->get_idr_field             = lt_dlsym(plugin->dl_handle,
                                                "oh_get_idr_field");
        (*abi)->add_idr_field             = lt_dlsym(plugin->dl_handle,
                                                "oh_add_idr_field");
        (*abi)->add_idr_field_id          = lt_dlsym(plugin->dl_handle,
                                                "oh_add_idr_field_id");
        (*abi)->set_idr_field             = lt_dlsym(plugin->dl_handle,
                                                "oh_set_idr_field");
        (*abi)->del_idr_field             = lt_dlsym(plugin->dl_handle,
                                                "oh_del_idr_field");
        (*abi)->get_watchdog_info         = lt_dlsym(plugin->dl_handle,
                                                "oh_get_watchdog_info");
        (*abi)->set_watchdog_info         = lt_dlsym(plugin->dl_handle,
                                                "oh_set_watchdog_info");
        (*abi)->reset_watchdog            = lt_dlsym(plugin->dl_handle,
                                                "oh_reset_watchdog");
        (*abi)->get_next_announce         = lt_dlsym(plugin->dl_handle,
                                                "oh_get_next_announce");
        (*abi)->get_announce              = lt_dlsym(plugin->dl_handle,
                                                "oh_get_announce");
        (*abi)->ack_announce              = lt_dlsym(plugin->dl_handle,
                                                "oh_ack_announce");
        (*abi)->add_announce              = lt_dlsym(plugin->dl_handle,
                                                "oh_add_announce");
        (*abi)->del_announce              = lt_dlsym(plugin->dl_handle,
                                                "oh_del_announce");
        (*abi)->get_annunc_mode           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_annunc_mode");
        (*abi)->set_annunc_mode           = lt_dlsym(plugin->dl_handle,
                                                "oh_set_annunc_mode");
	(*abi)->get_dimi_info		  = lt_dlsym(plugin->dl_handle,
						"oh_get_dimi_info");
        (*abi)->get_dimi_test             = lt_dlsym(plugin->dl_handle,
                                                "oh_get_dimi_test");
        (*abi)->get_dimi_test_ready       = lt_dlsym(plugin->dl_handle,
                                                "oh_get_dimi_test_ready");
        (*abi)->start_dimi_test           = lt_dlsym(plugin->dl_handle,
                                                "oh_start_dimi_test");
        (*abi)->cancel_dimi_test          = lt_dlsym(plugin->dl_handle,
                                                "oh_cancel_dimi_test");
        (*abi)->get_dimi_test_status      = lt_dlsym(plugin->dl_handle,
                                                "oh_get_dimi_test_status");
        (*abi)->get_dimi_test_results     = lt_dlsym(plugin->dl_handle,
                                                "oh_get_dimi_test_results");
        (*abi)->get_fumi_spec             = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_spec");
        (*abi)->get_fumi_service_impact = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_service_impact");
        (*abi)->set_fumi_source           = lt_dlsym(plugin->dl_handle,
                                                "oh_set_fumi_source");
        (*abi)->validate_fumi_source      = lt_dlsym(plugin->dl_handle,
                                                "oh_validate_fumi_source");
        (*abi)->get_fumi_source           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_source");
        (*abi)->get_fumi_source_component = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_source_component");
        (*abi)->get_fumi_target           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_target");
        (*abi)->get_fumi_target_component = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_target_component");
        (*abi)->get_fumi_logical_target   = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_logical_target");
        (*abi)->get_fumi_logical_target_component = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_logical_target_component");
        (*abi)->start_fumi_backup         = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_backup");
        (*abi)->set_fumi_bank_order       = lt_dlsym(plugin->dl_handle,
                                                "oh_set_fumi_bank_order");
        (*abi)->start_fumi_bank_copy      = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_bank_copy");
        (*abi)->start_fumi_install        = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_install");
        (*abi)->get_fumi_status           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_status");
        (*abi)->start_fumi_verify         = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_verify");
        (*abi)->start_fumi_verify_main    = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_verify_main");
        (*abi)->cancel_fumi_upgrade       = lt_dlsym(plugin->dl_handle,
                                                "oh_cancel_fumi_upgrade");
        (*abi)->get_fumi_autorollback_disable = lt_dlsym(plugin->dl_handle,
                                                "oh_get_fumi_autorollback_disable");
        (*abi)->set_fumi_autorollback_disable = lt_dlsym(plugin->dl_handle,
                                                "oh_set_fumi_autorollback_disable");
        (*abi)->start_fumi_rollback       = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_rollback");
        (*abi)->activate_fumi             = lt_dlsym(plugin->dl_handle,
                                                "oh_activate_fumi");
        (*abi)->start_fumi_activate       = lt_dlsym(plugin->dl_handle,
                                                "oh_start_fumi_activate");
        (*abi)->cleanup_fumi              = lt_dlsym(plugin->dl_handle,
                                                "oh_cleanup_fumi");
        (*abi)->hotswap_policy_cancel     = lt_dlsym(plugin->dl_handle,
                                                "oh_hotswap_policy_cancel");
        (*abi)->get_hotswap_state         = lt_dlsym(plugin->dl_handle,
                                                "oh_get_hotswap_state");
        (*abi)->set_autoinsert_timeout    = lt_dlsym(plugin->dl_handle,
                                                "oh_set_autoinsert_timeout");
        (*abi)->set_hotswap_state         = lt_dlsym(plugin->dl_handle,
                                                "oh_set_hotswap_state");
        (*abi)->request_hotswap_action    = lt_dlsym(plugin->dl_handle,
                                                "oh_request_hotswap_action");
	(*abi)->get_autoextract_timeout   = lt_dlsym(plugin->dl_handle,
                                                "oh_get_autoextract_timeout");
	(*abi)->set_autoextract_timeout   = lt_dlsym(plugin->dl_handle,
                                                "oh_set_autoextract_timeout");
        (*abi)->get_power_state           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_power_state");
        (*abi)->set_power_state           = lt_dlsym(plugin->dl_handle,
                                                "oh_set_power_state");
        (*abi)->get_indicator_state       = lt_dlsym(plugin->dl_handle,
                                                "oh_get_indicator_state");
        (*abi)->set_indicator_state       = lt_dlsym(plugin->dl_handle,
                                                "oh_set_indicator_state");
        (*abi)->control_parm              = lt_dlsym(plugin->dl_handle,
                                                "oh_control_parm");
        (*abi)->load_id_get               = lt_dlsym(plugin->dl_handle,
                                                "oh_load_id_get");
        (*abi)->load_id_set               = lt_dlsym(plugin->dl_handle,
                                                "oh_load_id_set");
        (*abi)->get_reset_state           = lt_dlsym(plugin->dl_handle,
                                                "oh_get_reset_state");
        (*abi)->set_reset_state           = lt_dlsym(plugin->dl_handle,
                                                "oh_set_reset_state");
        (*abi)->inject_event            = lt_dlsym(plugin->dl_handle,
                                                "oh_inject_event");

        return 0;

}
