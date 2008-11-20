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
 *     Sean Dague <http://dague.net/sean>
 *     Louis Zhuang <louis.zhuang@linux.intel.com>
 *     David Judkovics <djudkovi@us.ibm.com>
 *     Thomas Kangieser <Thomas.Kanngieser@fci.com>
 *     Renier Morales <renier@openhpi.org>
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <oh_config.h>
#include <oh_plugin.h>
#include <oh_error.h>
#include <oh_lock.h>
#include <config.h>

/*
 * Global Parameters
 */
static const char *known_globals[] = {
        "OPENHPI_ON_EP",
        "OPENHPI_LOG_ON_SEV",
        "OPENHPI_EVT_QUEUE_LIMIT",
        "OPENHPI_DEL_SIZE_LIMIT",
        "OPENHPI_DEL_SAVE",
        "OPENHPI_DAT_SIZE_LIMIT",
        "OPENHPI_DAT_USER_LIMIT",
        "OPENHPI_DAT_SAVE",
        "OPENHPI_PATH",
        "OPENHPI_VARPATH",
        "OPENHPI_CONF",
	"OPENHPICLIENT_CONF",
        NULL
};

static struct {
        SaHpiEntityPathT on_ep;
        SaHpiSeverityT log_on_sev;
        SaHpiUint32T evt_queue_limit;
        SaHpiUint32T del_size_limit;
        SaHpiBoolT del_save;
        SaHpiUint32T dat_size_limit;
        SaHpiUint32T dat_user_limit;
        SaHpiBoolT dat_save;
        char path[OH_MAX_TEXT_BUFFER_LENGTH];
        char varpath[OH_MAX_TEXT_BUFFER_LENGTH];
        char conf[OH_MAX_TEXT_BUFFER_LENGTH];
        char client_conf[OH_MAX_TEXT_BUFFER_LENGTH];	
        unsigned char read_env;
        GStaticRecMutex lock;
} global_params = { /* Defaults for global params are set here */
        .on_ep = { .Entry[0] = { .EntityType = SAHPI_ENT_ROOT, .EntityLocation = 0 } },
        .log_on_sev = SAHPI_MINOR,
        .evt_queue_limit = 10000,
        .del_size_limit = 10000, /* 0 is unlimited size */
        .del_save = SAHPI_FALSE,
        .dat_size_limit = 0, /* Unlimited size */
        .dat_user_limit = 0, /* Unlimited size */
        .dat_save = SAHPI_FALSE,
        .path = OH_PLUGIN_PATH,
        .varpath = VARPATH,
        .conf = OH_DEFAULT_CONF,
	.client_conf = OH_CLIENT_DEFAULT_CONF,
        .read_env = 0,
        .lock = G_STATIC_REC_MUTEX_INIT
};

/*
 *  List of handler configs (parameter tables).  This list is
 *  populated during config file parse, and used to build the handler_table
 */
static GSList *handler_configs = NULL;

/*******************************************************************************
 *  In order to use the glib lexical parser we need to define token
 *  types which we want to switch on
 ******************************************************************************/

enum {
        HPI_CONF_TOKEN_HANDLER = G_TOKEN_LAST
} hpiConfType;

struct tokens {
        gchar *name;
        guint token;
};

static struct tokens oh_conf_tokens[] = {
        {
                .name = "handler",
                .token = HPI_CONF_TOKEN_HANDLER
        }

};

/*******************************************************************************
 * In order to initialize the lexical scanner, you need the following config.
 * This config was figured out by reading the glib sources, and lots of
 * trial and error (documentation for this isn't very good).
 *
 * G_TOKEN_STRING will be created when anything starts with a-zA-z_/.
 * due to cset_identifier_first and identifier2string values below.
 * Therefor, if you want 0 to be scanned as a string, you need to quote
 * it (e.g. "0")
 *
 *******************************************************************************/

static GScannerConfig oh_scanner_config =
        {
                (
                        " \t\n"
                        )                       /* cset_skip_characters */,
                (
                        G_CSET_a_2_z
                        "_/."
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_first */,
                (
                        G_CSET_a_2_z
                        "_-0123456789/."
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_nth */,
                ( "#\n" )               /* cpair_comment_single */,
                FALSE                   /* case_sensitive */,
                TRUE                    /* skip_comment_multi */,
                TRUE                    /* skip_comment_single */,
                TRUE                    /* scan_comment_multi */,
                TRUE                    /* scan_identifier */,
                TRUE                    /* scan_identifier_1char */,
                TRUE                    /* scan_identifier_NULL */,
                TRUE                    /* scan_symbols */,
                TRUE                    /* scan_binary */,
                TRUE                    /* scan_octal */,
                TRUE                    /* scan_float */,
                TRUE                    /* scan_hex */,
                TRUE                    /* scan_hex_dollar */,
                TRUE                    /* scan_string_sq */,
                TRUE                    /* scan_string_dq */,
                TRUE                    /* numbers_2_int */,
                FALSE                   /* int_2_float */,
                TRUE                    /* identifier_2_string */,
                TRUE                    /* char_2_token */,
                TRUE                    /* symbol_2_token */,
                FALSE                   /* scope_0_fallback */,
        };

static void process_global_param(const char *name, char *value)
{
        if (!strcmp("OPENHPI_ON_EP", name)) {
                g_static_rec_mutex_lock(&global_params.lock);
                oh_encode_entitypath(value, &global_params.on_ep);
                g_static_rec_mutex_unlock(&global_params.lock);
        } else if (!strcmp("OPENHPI_LOG_ON_SEV", name)) {
                SaHpiTextBufferT buffer;
                strncpy((char *)buffer.Data, value, SAHPI_MAX_TEXT_BUFFER_LENGTH);
                g_static_rec_mutex_lock(&global_params.lock);
                oh_encode_severity(&buffer, &global_params.log_on_sev);
                g_static_rec_mutex_unlock(&global_params.lock);
        } else if (!strcmp("OPENHPI_EVT_QUEUE_LIMIT", name)) {
                global_params.evt_queue_limit = atoi(value);
        } else if (!strcmp("OPENHPI_DEL_SIZE_LIMIT", name)) {
                global_params.del_size_limit = atoi(value);
        } else if (!strcmp("OPENHPI_DEL_SAVE", name)) {
                if (!strcmp("YES", value)) {
                        global_params.del_save = SAHPI_TRUE;
                } else {
                        global_params.del_save = SAHPI_FALSE;
                }
        } else if (!strcmp("OPENHPI_DAT_SIZE_LIMIT", name)) {
                global_params.dat_size_limit = atoi(value);
        } else if (!strcmp("OPENHPI_DAT_USER_LIMIT", name)) {
                global_params.dat_user_limit = atoi(value);
        } else if (!strcmp("OPENHPI_DAT_SAVE", name)) {
                if (!strcmp("YES", value)) {
                        global_params.dat_save = SAHPI_TRUE;
                } else {
                        global_params.dat_save = SAHPI_FALSE;
                }
        } else if (!strcmp("OPENHPI_PATH", name)) {
                g_static_rec_mutex_lock(&global_params.lock);
                memset(global_params.path, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                strncpy(global_params.path, value, OH_MAX_TEXT_BUFFER_LENGTH-1);
                g_static_rec_mutex_unlock(&global_params.lock);
        } else if (!strcmp("OPENHPI_VARPATH", name)) {
                g_static_rec_mutex_lock(&global_params.lock);
                memset(global_params.varpath, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                strncpy(global_params.varpath, value, OH_MAX_TEXT_BUFFER_LENGTH-1);
                g_static_rec_mutex_unlock(&global_params.lock);
        } else if (!strcmp("OPENHPI_CONF", name)) {
                g_static_rec_mutex_lock(&global_params.lock);
                memset(global_params.conf, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                strncpy(global_params.conf, value, OH_MAX_TEXT_BUFFER_LENGTH-1);
                g_static_rec_mutex_unlock(&global_params.lock);
        } else if (!strcmp("OPENHPICLIENT_CONF", name)) {
                g_static_rec_mutex_lock(&global_params.lock);
                memset(global_params.client_conf, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                strncpy(global_params.client_conf, value, OH_MAX_TEXT_BUFFER_LENGTH-1);
                g_static_rec_mutex_unlock(&global_params.lock);
	} else {
                err("ERROR. Invalid global parameter %s in config file", name);
        }

        return;
}

static void read_globals_from_env(int force)
{
        char *tmp_env_str = NULL;
        int i;

        if (!force && global_params.read_env) return;

        g_static_rec_mutex_lock(&global_params.lock);

        for (i = 0; known_globals[i]; i++) {
                if ((tmp_env_str = getenv(known_globals[i])) != NULL) {
                        process_global_param(known_globals[i], tmp_env_str);
                        tmp_env_str = NULL;
                }
        }

        global_params.read_env = 1;
        g_static_rec_mutex_unlock(&global_params.lock);
}

/**
 * process_handler_token
 * @oh_scanner
 *
 * handles parsing of handler tokens into a hash table.
 *
 * Return value: 0 on sucess, < 0 on fail
 **/
static int process_handler_token (GScanner* oh_scanner)
{
        GHashTable *handler_stanza = NULL;
        char *tablekey, *tablevalue;
        int found_right_curly = 0;

        data_access_lock();

        if (g_scanner_get_next_token(oh_scanner) != HPI_CONF_TOKEN_HANDLER) {
                err("Processing handler: Unexpected token.");
                data_access_unlock();
                return -1;
        }

        /* Get the plugin type and store in Hash Table */
        if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_STRING) {
                err("Processing handler: Expected string token.");
                data_access_unlock();
                return -1;
        } else {
                handler_stanza = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                       g_free, g_free);
                tablekey = g_strdup("plugin");
                tablevalue = g_strdup(oh_scanner->value.v_string);
                g_hash_table_insert(handler_stanza,
                                    (gpointer) tablekey,
                                    (gpointer) tablevalue);
                oh_load_plugin(tablevalue);
        }

        /* Check for Left Brace token type. If we have it, then continue parsing. */
        if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_LEFT_CURLY) {
                err("Processing handler: Expected left curly token.");
                goto free_table;
        }

        while (!found_right_curly) {
                /* get key token in key\value pair set (e.g. key = value) */
                if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_STRING) {
                        err("Processing handler: Expected string token.");
                        goto free_table;
                } else {
                        tablekey = g_strdup(oh_scanner->value.v_string);
                }

                /* Check for the equal sign next. If we have it, continue parsing */
                if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_EQUAL_SIGN) {
                        err("Processing handler: Expected equal sign token.");
                        goto free_table_and_key;
                }

                /*
                 * Now check for the value token in the key\value set.
                 * Store the key\value value pair in the hash table and continue on.
                 */
                if (g_scanner_peek_next_token(oh_scanner) != G_TOKEN_INT &&
                    g_scanner_peek_next_token(oh_scanner) != G_TOKEN_FLOAT &&
                    g_scanner_peek_next_token(oh_scanner) != G_TOKEN_STRING) {
                        err("Processing handler: Expected string, integer, or float token.");
                        goto free_table_and_key;
                } else { /* The type of token tells us how to fetch the value from oh_scanner */
                        gpointer value = NULL;
                        int current_token = g_scanner_get_next_token(oh_scanner);

                        if (current_token == G_TOKEN_INT) {
                                gulong *value_int =
                                        (gulong *)g_malloc(sizeof(gulong));
                                *value_int = oh_scanner->value.v_int;
                                value = (gpointer)value_int;
                        } else if (current_token == G_TOKEN_FLOAT) {
                                gdouble *value_float =
                                        (gdouble *)g_malloc(sizeof(gdouble));
                                *value_float = oh_scanner->value.v_float;
                                value = (gpointer)value_float;
                        } else {
                                gchar *value_string =
                                        g_strdup(oh_scanner->value.v_string);
                                value = (gpointer)value_string;
                        }

                        if (value == NULL) {
                                err("Processing handler:"
                                    " Unable to allocate memory for value."
                                    " Token Type: %d",
                                    current_token);
                                goto free_table_and_key;
                        }

                        g_hash_table_insert(handler_stanza,
                                            (gpointer) tablekey,
                                            value);
                }

                if (g_scanner_peek_next_token(oh_scanner) == G_TOKEN_RIGHT_CURLY) {
                        g_scanner_get_next_token(oh_scanner);
                        found_right_curly = 1;
                }
        } /* end of while(!found_right_curly) */

        /* Attach table describing handler stanza to the global linked list of handlers */
        if (handler_stanza != NULL) {
                handler_configs = g_slist_append(
                        handler_configs,
                        (gpointer) handler_stanza);
        }

        data_access_unlock();

        return 0;

free_table_and_key:
        g_free(tablekey);
free_table:
        /**
        There was an error reading a token so we need to error out,
        but not before cleaning up. Destroy the table.
        */
        g_hash_table_destroy(handler_stanza);

        data_access_unlock();

        return -1;
}

static int process_global_token(GScanner *scanner)
{
        char *name = NULL, *value = NULL;
        guint current_token;

        data_access_lock();

        /* Get the global parameter name */
        current_token = g_scanner_get_next_token(scanner);
        if (current_token != G_TOKEN_STRING) {
                err("Processing global: Expected string token. Got %d",
                    current_token);
                goto quit;
        }

        name = g_strdup(scanner->value.v_string);
        if (!name) {
                err("Unable to allocate for global param name.");
                goto quit;
        }

        current_token = g_scanner_get_next_token(scanner);
        if (current_token != G_TOKEN_EQUAL_SIGN) {
                err("Did not get expected '=' token. Got %d", current_token);
                goto free_and_quit;
        }

        current_token = g_scanner_get_next_token(scanner);
        if (current_token != G_TOKEN_STRING && current_token != G_TOKEN_INT) {
                err("Did not get expected string value for global parameter."
                    " Got %d", current_token);
                goto free_and_quit;
        }

        if (current_token == G_TOKEN_INT) {
                guint num_chars = 0, result;
                result = scanner->value.v_int;
                while (result) {
                        result = result / 10;
                        num_chars++;
                }
                value = (char *)g_malloc0(++num_chars);
                result = scanner->value.v_int;
                snprintf(value, num_chars, "%u", result);
        } else {
                value = g_strdup(scanner->value.v_string);
        }

        if (!value) {
                err("Unable to allocate for global param value.");
                goto free_and_quit;
        }

        process_global_param(name, value);
        g_free(name);
        g_free(value);
        data_access_unlock();
        return 0;

free_and_quit:
        g_free(name);
quit:
        data_access_unlock();
        return -1;
}

/**
 * scanner_msg_handler: a reference of this function is passed into the GScanner.
 * Used by the GScanner object to output messages that come up during parsing.
 *
 * @scanner: Object used to parse the config file.
 * @message: Message string.
 * @is_error: Bit to say the message is an error.
 *
 * Return value: None (void).
 **/
static void scanner_msg_handler (GScanner *scanner, gchar *message, gboolean is_error)
{
        g_return_if_fail (scanner != NULL);

        err("%s:%d: %s%s\n",
            scanner->input_name ? scanner->input_name : "<memory>",
            scanner->line, is_error ? "error: " : "", message );
}

/**
 * oh_load_config
 * @filename: OpenHPI configuration filename
 * @config: place where the parsed configuration will be placed.
 *
 * Parses an OpenHPI configuration file and gives the results
 * which can be processed by the caller.
 *
 * Return value: 0 on success, otherwise a failure.
 **/
int oh_load_config (char *filename, struct oh_parsed_config *config)
{
        int oh_conf_file, i;
        GScanner *oh_scanner;
        int done = 0;
        int num_tokens = sizeof(oh_conf_tokens) / sizeof(oh_conf_tokens[0]);

        if (!filename || !config) {
                err("Error. Invalid parameters");
                return -1;
        }

        handler_configs = NULL;
        oh_scanner = g_scanner_new(&oh_scanner_config);
        if (!oh_scanner) {
                err("Couldn't allocate g_scanner for file parsing");
                return -2;
        }

        oh_scanner->msg_handler = scanner_msg_handler;
        oh_scanner->input_name = filename;

        oh_conf_file = open(filename, O_RDONLY);
        if (oh_conf_file < 0) {
                err("Configuration file '%s' could not be opened", filename);
                g_scanner_destroy(oh_scanner);
                return -4;
        }

        g_scanner_input_file(oh_scanner, oh_conf_file);

        for (i = 0; i < num_tokens; i++) {
                g_scanner_scope_add_symbol(
                        oh_scanner, 0,
                        oh_conf_tokens[i].name,
                        (void *)((unsigned long)oh_conf_tokens[i].token));
        }

        while (!done) {
                guint my_token;
                my_token = g_scanner_peek_next_token (oh_scanner);
                /*dbg("token: %d", my_token);*/
                switch (my_token)
                {
                case G_TOKEN_EOF:
                        done = 1;
                        break;
                case HPI_CONF_TOKEN_HANDLER:
                        process_handler_token(oh_scanner);
                        break;
                case G_TOKEN_STRING:
                        process_global_token(oh_scanner);
                        break;
                default:
                        /* need to advance it */
                        my_token = g_scanner_get_next_token(oh_scanner);
                        g_scanner_unexp_token(oh_scanner, G_TOKEN_SYMBOL,
                                              NULL, "\"handle\" or \"domain\"", NULL, NULL, 1);
                        break;
                }
        }

        read_globals_from_env(1);

        if (close(oh_conf_file) != 0) {
                err("Couldn't close file '%s'.", filename);
                g_scanner_destroy(oh_scanner);
                return -5;
        }

        done = oh_scanner->parse_errors;

        g_scanner_destroy(oh_scanner);

        dbg("Done processing conf file.\nNumber of parse errors:%d", done);

        config->handler_configs = handler_configs;

        handler_configs = NULL;

        return 0;
}

/**
 * oh_process_config
 * @config: pointer to parsed configuration for processing
 *
 * This will process a parsed configuration by loading
 * the specified plugins and corresponding handlers.
 *
 * Returns: SA_OK on success, otherwise the call failed.
 **/
SaErrorT oh_process_config(struct oh_parsed_config *config)
{
        GSList *node = NULL;

        if (!config) return SA_ERR_HPI_INVALID_PARAMS;

        /* Initialize handlers */
        for (node = config->handler_configs; node; node = node->next) {
                GHashTable *handler_config = (GHashTable *)node->data;
		unsigned int hid = 0;
		SaErrorT error = SA_OK;

		error = oh_create_handler(handler_config, &hid);
                if (error == SA_OK) {
                        dbg("Loaded handler for plugin %s",
                            (char *)g_hash_table_lookup(handler_config, "plugin"));
                        config->handlers_loaded++;
                } else {
                        err("Couldn't load handler for plugin %s",
                            (char *)g_hash_table_lookup(handler_config, "plugin"));
                        if (hid == 0) g_hash_table_destroy(handler_config);
                }
                config->handlers_defined++;
        }

        return SA_OK;
}

void oh_clean_config(struct oh_parsed_config *config)
{
        /* Free list of handler configuration blocks */
        g_slist_free(config->handler_configs);

}

/**
 * oh_get_global_param
 * @param
 *
 * Returns: 0 on Success.
 **/
int oh_get_global_param(struct oh_global_param *param)
{
        if (!param || !(param->type)) {

            if (!param) {
                err("ERROR. Invalid parameters param NULL");
            }

            if (!param->type) {
                err("ERROR. Invalid parameters param->type NULL");
            }

                return -1;
        }

        read_globals_from_env(0);

        switch (param->type) {
                case OPENHPI_ON_EP:
                        g_static_rec_mutex_lock(&global_params.lock);
                        param->u.on_ep = global_params.on_ep;
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_LOG_ON_SEV:
                        param->u.log_on_sev = global_params.log_on_sev;
                        break;
                case OPENHPI_EVT_QUEUE_LIMIT:
                        param->u.evt_queue_limit = global_params.evt_queue_limit;
                        break;
                case OPENHPI_DEL_SIZE_LIMIT:
                        param->u.del_size_limit = global_params.del_size_limit;
                        break;
                case OPENHPI_DEL_SAVE:
                        param->u.del_save = global_params.del_save;
                        break;
                case OPENHPI_DAT_SIZE_LIMIT:
                        param->u.dat_size_limit = global_params.dat_size_limit;
                        break;
                case OPENHPI_DAT_USER_LIMIT:
                        param->u.dat_user_limit = global_params.dat_user_limit;
                        break;
                case OPENHPI_DAT_SAVE:
                        param->u.dat_save = global_params.dat_save;
                        break;
                case OPENHPI_PATH:
                        g_static_rec_mutex_lock(&global_params.lock);
                        strncpy(param->u.path,
                                global_params.path,
                                OH_MAX_TEXT_BUFFER_LENGTH);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_VARPATH:
                        g_static_rec_mutex_lock(&global_params.lock);
                        strncpy(param->u.varpath,
                                global_params.varpath,
                                OH_MAX_TEXT_BUFFER_LENGTH);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_CONF:
                        g_static_rec_mutex_lock(&global_params.lock);
                        strncpy(param->u.conf,
                                global_params.conf,
                                OH_MAX_TEXT_BUFFER_LENGTH);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPICLIENT_CONF:
                        g_static_rec_mutex_lock(&global_params.lock);
                        strncpy(param->u.conf,
                                global_params.client_conf,
                                OH_MAX_TEXT_BUFFER_LENGTH);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
               default:
                        err("ERROR. Invalid global parameter %d!", param->type);
                        return -2;
        }

        return 0;
}

/**
 * oh_set_global_param
 * @param
 *
 * Returns: 0 on Success.
 **/
int oh_set_global_param(struct oh_global_param *param)
{
        if (!param || !(param->type)) {
                err("ERROR. Invalid parameters");
                return -1;
        }

        read_globals_from_env(0);

        switch (param->type) {
                case OPENHPI_ON_EP:
                        g_static_rec_mutex_lock(&global_params.lock);
                        global_params.on_ep = param->u.on_ep;
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_LOG_ON_SEV:
                        global_params.log_on_sev = param->u.log_on_sev;
                        break;
                case OPENHPI_EVT_QUEUE_LIMIT:
                        global_params.evt_queue_limit = param->u.evt_queue_limit;
                        break;
                case OPENHPI_DEL_SIZE_LIMIT:
                        global_params.del_size_limit = param->u.del_size_limit;
                        break;
                case OPENHPI_DEL_SAVE:
                        global_params.del_save = param->u.del_save;
                        break;
                case OPENHPI_DAT_SIZE_LIMIT:
                        global_params.dat_size_limit = param->u.dat_size_limit;
                        break;
                case OPENHPI_DAT_USER_LIMIT:
                        global_params.dat_user_limit = param->u.dat_user_limit;
                        break;
                case OPENHPI_DAT_SAVE:
                        global_params.dat_save = param->u.dat_save;
                        break;
                case OPENHPI_PATH:
                        g_static_rec_mutex_lock(&global_params.lock);
                        memset(global_params.path, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                        strncpy(global_params.path,
                                param->u.path,
                                OH_MAX_TEXT_BUFFER_LENGTH-1);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_VARPATH:
                        g_static_rec_mutex_lock(&global_params.lock);
                        memset(global_params.varpath, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                        strncpy(global_params.varpath,
                                param->u.varpath,
                                OH_MAX_TEXT_BUFFER_LENGTH-1);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPI_CONF:
                        g_static_rec_mutex_lock(&global_params.lock);
                        memset(global_params.conf, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                        strncpy(global_params.conf,
                                param->u.conf,
                                OH_MAX_TEXT_BUFFER_LENGTH-1);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;
                case OPENHPICLIENT_CONF:
                        g_static_rec_mutex_lock(&global_params.lock);
                        memset(global_params.conf, 0, OH_MAX_TEXT_BUFFER_LENGTH);
                        strncpy(global_params.client_conf,
                                param->u.conf,
                                OH_MAX_TEXT_BUFFER_LENGTH-1);
                        g_static_rec_mutex_unlock(&global_params.lock);
                        break;			
                default:
                        err("ERROR. Invalid global parameter %d!", param->type);
                        return -2;
        }

        return 0;
}
