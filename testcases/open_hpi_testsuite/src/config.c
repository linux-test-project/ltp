/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
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
 * Contributors:
 *     Thomas Kangieser <Thomas.Kanngieser@fci.com>
 *     Renier Morales <renierm@users.sf.net>
 */

#include <openhpi.h>
#include <oh_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*******************************************************************************
 *  global_plugin_list - list of all the plugins that should be loaded
 *
 *  global_handler_list - list of handlers that have been loaded
 ******************************************************************************/

GSList *global_plugin_list = NULL;
GSList *global_handler_list = NULL;
GSList *global_handler_configs = NULL;

/*******************************************************************************
 *  In order to use the glib lexical parser we need to define token 
 *  types which we want to switch on, and also 
 ******************************************************************************/

enum {
        HPI_CONF_TOKEN_HANDLER = G_TOKEN_LAST,
        HPI_CONF_TOKEN_PLUGIN
} hpiConfType;

struct tokens {
        gchar *name;
        guint token;
};
struct tokens oh_conf_tokens[] = {
	  {
		.name = "handler", 
		.token = HPI_CONF_TOKEN_HANDLER
	  },
	  { 
		.name = "plugin", 
		.token = HPI_CONF_TOKEN_PLUGIN 
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

/*******************************************************************************
 *  prototypes for functions internal to this file
 ******************************************************************************/

int process_plugin_token (GScanner *);

int process_handler_token (GScanner *);
void free_hash_table (gpointer key, gpointer value, gpointer user_data);

struct oh_plugin_config * new_plugin_config (char *);


/**
 * new_plugin_config: generates a new plugin config
 * @plugin: 
 * 
 * 
 * 
 * Return value: 
 **/

struct oh_plugin_config *new_plugin_config (char *plugin) 
{
        struct oh_plugin_config *pc;
        
        pc = calloc(1,sizeof(*pc));
        if(pc == NULL) {
                dbg("Couldn't allocate memory for handler_config");
                return pc;
        }
        
        if(plugin != NULL) {
                pc->name = calloc(strlen(plugin)+1, sizeof(char));
                strcpy(pc->name,plugin);
        }
        pc->refcount = 0;

        return pc;
}

/**
 * process_plugin_token:
 * @oh_scanner: 
 * 
 * 
 * 
 * Return value: 
 **/
int process_plugin_token (GScanner *oh_scanner) 
{
        guint my_token;
        int refcount;
        
        data_access_lock();

        my_token = g_scanner_get_next_token(oh_scanner); 
        if (my_token != HPI_CONF_TOKEN_PLUGIN) {
                dbg("Token is not what I was promissed");
                data_access_unlock();
                return -1;
        }
        my_token = g_scanner_get_next_token(oh_scanner);
        if(my_token != G_TOKEN_STRING) {
                dbg("Where the heck is my string!");
                data_access_unlock();
                return -1;
        }
        
        refcount = plugin_refcount(oh_scanner->value.v_string);
        if(refcount < 0) {
                global_plugin_list = g_slist_append(
                        global_plugin_list,
                        (gpointer) new_plugin_config(oh_scanner->value.v_string)
                        );
        } else {
                dbg("WARNING: Attempt to load a plugin more than once");
                data_access_unlock();
                return -1;
        }
        
        data_access_unlock();

        return 0;
}

/**
 * plugin_refcount:
 * @name: 
 * 
 * 
 * 
 * Return value: 
 **/
int plugin_refcount (char *name) 
{
        struct oh_plugin_config *temp_config;
        int refcount = -1;
        
        temp_config = plugin_config(name); 

        if(temp_config != NULL) {
                refcount = temp_config->refcount;
        }
        return refcount;
}

struct oh_plugin_config * plugin_config (char *name) 
{
        GSList *node;
        struct oh_plugin_config *return_config = NULL;

        data_access_lock();

        g_slist_for_each(node, global_plugin_list) {
                struct oh_plugin_config *pconf = node->data;
                if(strcmp(pconf->name, name) == 0) {
                        return_config = pconf;
                        break;
                }
        }
        
        data_access_unlock();
        
        return return_config;
}

/**
 * process_handler_token:  handles parsing of handler tokens into 
 * @oh_scanner: 
 * 
 * 
 * 
 * Return value: 0 on sucess, < 0 on fail
 **/
int process_handler_token (GScanner* oh_scanner) 
{
        GHashTable *handler_stanza = NULL;
        char *tablekey, *tablevalue;
        int found_right_curly = 0;

        data_access_lock();

        
        if (g_scanner_get_next_token(oh_scanner) != HPI_CONF_TOKEN_HANDLER) {
                dbg("Processing handler: Expected handler token.");
                data_access_unlock();
                return -1;
        }

        /* Get the plugin type and store in Hash Table */
        if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_STRING) {
                dbg("Processing handler: Expected string token.");
                data_access_unlock();
                return -1;
        } else {
                handler_stanza = g_hash_table_new(g_str_hash, g_str_equal);
                tablekey = g_strdup("plugin");
		if (!tablekey) {
		        dbg("Processing handler: Unable to allocate memory");
		        goto free_table;
		}
		tablevalue = g_strdup(oh_scanner->value.v_string);
		if (!tablevalue) {
		        dbg("Processing handler: Unable to allocate memory");
		        goto free_table_and_key;
		}
                g_hash_table_insert(handler_stanza,
                                    (gpointer) tablekey,
                                    (gpointer) tablevalue);
        }        

        /* Check for Left Brace token type. If we have it, then continue parsing. */
        if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_LEFT_CURLY) {
                dbg("Processing handler: Expected left curly token.");
                goto free_table;
        }

        while(!found_right_curly) {
                /* get key token in key\value pair set (e.g. key = value) */
                if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_STRING) {
                        dbg("Processing handler: Expected string token.");
                        goto free_table;
                } else {
                        tablekey = g_strdup(oh_scanner->value.v_string);
			if (!tablekey) {
			        dbg("Processing handler: Unable to allocate memory");
		                goto free_table;
			}                        
                }

                /* Check for the equal sign next. If we have it, continue parsing */
                if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_EQUAL_SIGN) {
                        dbg("Processing handler: Expected equal sign token.");
                        goto free_table_and_key;
                }

                /**
                Now check for the value token in the key\value set. Store the key\value value pair
                in the hash table and continue on.
                */
                if (g_scanner_peek_next_token(oh_scanner) != G_TOKEN_INT &&
                    g_scanner_peek_next_token(oh_scanner) != G_TOKEN_FLOAT &&
                    g_scanner_peek_next_token(oh_scanner) != G_TOKEN_STRING) {
                        dbg("Processing handler: Expected string, integer, or float token.");
                        goto free_table_and_key;
                } else { /* The type of token tells us how to fetch the value from oh_scanner */
                        gulong *value_int;
                        gdouble *value_float;
                        gchar *value_string;
                        gpointer value;
                        int current_token = g_scanner_get_next_token(oh_scanner);

                        if (current_token == G_TOKEN_INT) {
                                value_int = (gulong *)g_malloc(sizeof(gulong));
                                if (value_int != NULL) *value_int = oh_scanner->value.v_int;
                                value = (gpointer)value_int;                                
                        } else if (current_token == G_TOKEN_FLOAT) {
                                value_float = (gdouble *)g_malloc(sizeof(gdouble));
                                if (value_float != NULL) *value_float = oh_scanner->value.v_float;
                                value = (gpointer)value_float;
                        } else {
                                value_string = g_strdup(oh_scanner->value.v_string);
                                value = (gpointer)value_string;
                        }                        
                        
                        if (value == NULL) {
                                dbg("Processing handler: Unable to allocate memory for value. Token Type: %d",
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
        if(handler_stanza != NULL) {
                global_handler_configs = g_slist_append(
                        global_handler_configs,
                        (gpointer) handler_stanza);
        }        
        
        data_access_unlock();

        return 0;

free_table_and_key:
        free(tablekey);
free_table:
        /**
        There was an error reading a token so we need to error out,
        but not before cleaning up. Iterate through the table
        freeing each key and value set. Then destroy the table.
        */
        g_hash_table_foreach(handler_stanza, free_hash_table, NULL);
        g_hash_table_destroy(handler_stanza);

        data_access_unlock();

        return -1;
}

/**
 * free_hash_table: used in the g_hash_table_foreach call in process_handler_token.
 * This funtion is passed to that table call by reference. Frees the memory allocated
 * for the key and value arguments it receives.
 *
 * @key: key into a GHashTable
 * @value: value correspoinding to key in GHasgTable
 * @user_data: It is NULL. Not used here.
 *
 * Return value: None (void).
 **/
void free_hash_table (gpointer key, gpointer value, gpointer user_data)
{
        g_free(key);
        g_free(value);
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

  dbg("%s:%d: %s%s\n",
              scanner->input_name ? scanner->input_name : "<memory>",
              scanner->line, is_error ? "error: " : "", message );
}

/**
 * oh_load_config:
 * @filename: 
 * 
 * 
 * 
 * Return value: 
 **/
int oh_load_config (char *filename) 
{
        int oh_conf_file, i;
        GScanner* oh_scanner;
        int done = 0;
        int num_tokens = sizeof(oh_conf_tokens) / sizeof(oh_conf_tokens[0]);

        init_plugin();
        add_domain(SAHPI_DEFAULT_DOMAIN_ID);    
        
        oh_scanner = g_scanner_new(&oh_scanner_config);
        if(!oh_scanner) {
                dbg("Couldn't allocate g_scanner for file parsing");
                return -1;
        }

        oh_scanner->msg_handler = scanner_msg_handler;
        oh_scanner->input_name = filename;

        oh_conf_file = open(filename, O_RDONLY);
        if(oh_conf_file < 0) {
                dbg("Configuration file '%s' could not be opened", filename);
                g_scanner_destroy(oh_scanner);
                return -2;
        }

        g_scanner_input_file(oh_scanner, oh_conf_file);
        
        for (i = 0; i < num_tokens; i++) {
                g_scanner_add_symbol (oh_scanner, oh_conf_tokens[i].name, 
                                      GINT_TO_POINTER (oh_conf_tokens[i].token));
        }

        while(!done) {
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
                case HPI_CONF_TOKEN_PLUGIN:
                        process_plugin_token(oh_scanner);
                        break;
                default:
                        /* need to advance it */
                        my_token = g_scanner_get_next_token(oh_scanner);
                        g_scanner_unexp_token(oh_scanner, G_TOKEN_SYMBOL,
                                              NULL, "\"handle\" or \"plugin\"", NULL, NULL, 1);
                        break;
                }
        }
        
        if(close(oh_conf_file) != 0) {
                dbg("Couldn't close file '%s'.", filename);
                g_scanner_destroy(oh_scanner);
                return -2;
        }

        done = oh_scanner->parse_errors;

        g_scanner_destroy(oh_scanner);
        dbg("Done processing conf file.\nNumber of parse errors:%d", done);
        
        return 0;
}


void oh_unload_config()
{
        while(global_handler_configs) {
                GHashTable *hash_table = global_handler_configs->data;

                global_handler_configs = g_slist_remove(
                        global_handler_configs,
                        (gpointer)hash_table );

                g_hash_table_destroy(hash_table);
        }

        uninit_plugin();

}




