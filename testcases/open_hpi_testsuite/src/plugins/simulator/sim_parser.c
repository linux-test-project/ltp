/*      -*- linux-c -*-
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
 * Authours:
 *      Racing Guo <racing.guo@intel.com>
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>

#ifndef UNIT_TEST
#include <openhpi.h>
#define trace(f, ...)  do {;} while(0)
#else
#define dbg(format, ...)                                        \
        do {                                                    \
                fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
                fprintf(stderr, format "\n", ## __VA_ARGS__);   \
        } while(0)

#define trace(f, ...) printf(__FILE__":%s(" f ")\n", __FUNCTION__, ## __VA_ARGS__)
#undef trace
#define trace(f, ...) do{;}while(0)
#endif

#include "sim_parser.h"


static GScannerConfig sim_scanner_config =
        {
                (
                        " \t\n\r"
                        )                       /* cset_skip_characters */,
                (
                        G_CSET_a_2_z
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_first */,
                (
                        G_CSET_a_2_z
                        "_-0123456789"
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_nth */,
                ( "#\n" )               /* cpair_comment_single */,
                TRUE                    /* case_sensitive */,
                TRUE                    /* skip_comment_multi */,
                TRUE                    /* skip_comment_single */,
                TRUE                    /* scan_comment_multi */,
                TRUE                    /* scan_identifier */,
                TRUE                    /* scan_identifier_1char */,
                FALSE                   /* scan_identifier_NULL */,
                FALSE                   /* scan_symbols */,
                FALSE                   /* scan_binary */,
                FALSE                   /* scan_octal */,
                FALSE                    /* scan_float */,
                FALSE                   /* scan_hex */,
                FALSE                   /* scan_hex_dollar */,
                TRUE                    /* scan_string_sq */,
                TRUE                    /* scan_string_dq */,
                FALSE                   /* numbers_2_int */,
                FALSE                   /* int_2_float */,
                FALSE                   /* identifier_2_string */,
                TRUE                    /* char_2_token */,
                FALSE                   /* symbol_2_token */,
                FALSE                   /* scope_0_fallback */,
        };

static void sim_scanner_msg_handler (GScanner *scanner, gchar *message, gboolean is_error)
{
       g_return_if_fail (scanner != NULL);
       dbg("flie(%s):line(%d):position(%d):%s\n",
           scanner->input_name, scanner->line, scanner->position, message);
}

void free_fhs_node(GSList *fhs)
{
    GSList *list;

    for (list = fhs; list; list = g_slist_next(list)) {
       fhs_node *n = (fhs_node*)(list->data);
       
       free(n->name);
       switch (n->type) {
          case SIM_FHS_STRING:
               free(n->value.str);
               break;
          case SIM_FHS_ARRAY:
               {
                   GSList *tmp;  
                   tmp = n->value.array;
                   for (tmp = n->value.array; tmp; tmp = g_slist_next(tmp))
                       free(tmp->data);
                   g_slist_free(n->value.array);
               }
               break;
          case SIM_FHS_SEQ:
               free_fhs_node(n->value.seq);
               break;
          default:
               dbg("error type(%d) in fhs_node", n->type);
               break;
       }
       free(n);
    }
    g_slist_free(fhs);
}
static GSList* __sim_parser(GScanner* sim_scanner, guint end_token)
{
#define NULL_CHECK(x)                             \
 do {                                             \
     if ((x)==NULL) {                             \
        dbg("Couldn't allocate memory for "#x);   \
        goto error_process ;                      \
     }                                            \
 }while(0)

       GSList    *retval = NULL; 
       fhs_node  *node = NULL;
       char      *str;
       guint     token;
       for (;;) {  

            token = g_scanner_get_next_token (sim_scanner);

            if (token == end_token) {
                 break;
            }else if (token == G_TOKEN_IDENTIFIER) {
                 
                 node = g_malloc0(sizeof(*node));
                 NULL_CHECK(node);
                 retval = g_slist_append(retval, node);
                 str = strdup(sim_scanner->value.v_string);
                 NULL_CHECK(str);
                 node->name = str;
                 trace("id:%s", node->name);
            }else {
                 sim_scanner->msg_handler(sim_scanner, 
                                          "expected id, EOF or }", 1);
                 goto error_process; 
            }
            
            token = g_scanner_get_next_token (sim_scanner);
            if (token != G_TOKEN_EQUAL_SIGN) {
                 sim_scanner->msg_handler(sim_scanner, "expected =", 1);
                 goto error_process;
            }

            token = g_scanner_get_next_token (sim_scanner);
            if (token == G_TOKEN_STRING) {
                  node->type = SIM_FHS_STRING;
                  str = strdup(sim_scanner->value.v_string);
                  NULL_CHECK(str);
                  node->value.str = str;
                  trace("string:%s", node->value.str);
           
            }else if (token == G_TOKEN_LEFT_CURLY) {
                  token = g_scanner_peek_next_token (sim_scanner);
                  if (token == G_TOKEN_IDENTIFIER) {
                      GSList *tmp; 
                      tmp = __sim_parser(sim_scanner, G_TOKEN_RIGHT_CURLY); 
                      if (tmp == NULL)
                          goto error_process;
                      node->type = SIM_FHS_SEQ;
                      node->value.seq = tmp;   
                  }else if (token == G_TOKEN_STRING) {
                       GSList *array = NULL;
                       for(;;) {
                             token = g_scanner_get_next_token (sim_scanner);
                             if (token != G_TOKEN_STRING) {
                                  sim_scanner->msg_handler(sim_scanner,
                                          "expected string", 1);
                                  goto error_process;
                             }
                             str = strdup(sim_scanner->value.v_string);
                             NULL_CHECK(str);
                             trace("string:%s", str);
                             array = g_slist_append(array, str);
                             token = g_scanner_get_next_token (sim_scanner);
                             if (token == G_TOKEN_RIGHT_CURLY) {
                                  break;
                             }else if (token != G_TOKEN_COMMA) {
                                  sim_scanner->msg_handler(sim_scanner,
                                                           "expected ,", 1);
                                  goto error_process;
                             }
                       }
                       node->type = SIM_FHS_ARRAY;
                       node->value.array = array;

                  }else {
                       sim_scanner->msg_handler(sim_scanner, 
                                                "expected id or string", 1);
                       goto error_process;
                  }
            }else {
                  sim_scanner->msg_handler(sim_scanner, 
                                           "expected string or {", 1);
                  goto error_process;
            }
       }
       return retval;
error_process:
       
       if (retval)
           free_fhs_node(retval);
       return NULL;
}

GSList* sim_parser(char *filename) 
{
        int       sim_parser_file;
        GScanner  *sim_scanner;
        GSList    *retval;
        
        sim_scanner = g_scanner_new(&sim_scanner_config);
        if(!sim_scanner) {
                dbg("Couldn't allocate g_scanner for file parsing");
                return NULL;
        }

        sim_scanner->msg_handler = sim_scanner_msg_handler;
        sim_scanner->input_name = filename;

        sim_parser_file = open(filename, O_RDONLY);
        if(sim_parser_file < 0) {
                dbg("Configuration file '%s' couldn't be opened", filename);
                return NULL;
        }

        g_scanner_input_file(sim_scanner, sim_parser_file);
        
        retval = __sim_parser(sim_scanner, G_TOKEN_EOF);        
        if(close(sim_parser_file) != 0)
               dbg("Configuration file '%s' couldn't be closed", filename);
        g_scanner_destroy(sim_scanner);
        return retval;

}

static int __sim_generate(GSList * fhs, FILE *file, int hi_num)
{
    int i;

    if (hi_num > 0) {
        fprintf(file, "{\n");
    }
  
    for ( ; fhs; fhs = g_slist_next(fhs)) {
       fhs_node *n;

       n = (fhs_node*) fhs->data;
       for (i = 0; i < hi_num; i++)
           fprintf(file, "    ");
       fprintf(file, "%s = ", n->name);
       switch (n->type) {
          case SIM_FHS_STRING:
               fprintf(file, "\"%s\"\n",n->value.str);
               break;
          case SIM_FHS_ARRAY:
               {
                   GSList *list;
         
                   fprintf(file, "{");     
                   list = n->value.array;
                   fprintf(file, "\"%s\"", (char*)list->data);
                   for (list = g_slist_next(list); list; 
                                list = g_slist_next(list)) 
                         fprintf(file, ", \"%s\"", (char*)list->data); 
                   fprintf(file, "}\n");
               }
               break;
          case SIM_FHS_SEQ:
               __sim_generate(n->value.seq, file, hi_num+1);
               break;
          default:
               dbg("error type(%d) in fhs_node", n->type);
       }
    } 

    if (hi_num > 0) {
      for (i = 0; i < hi_num - 1; i++)
           fprintf(file, "    ");
      fprintf(file,"}\n");
    }
    return 0;
}

int sim_generate(char *filename, GSList * fhs)
{
    int retval;
    FILE*  file;
    
    file = fopen(filename, "w");
    if (file == NULL) {
       dbg("The file %s could not be opened", filename);
       return -1;
    }
    retval = __sim_generate(fhs, file, 0);
    fclose(file);
    return retval;
}


#ifdef UNIT_TEST
int main()
{
    GSList *fhs;
  
    fhs = sim_parser("sim.in"); 
    sim_generate("sim.out", fhs);
    free_fhs_node(fhs);
    return 0;
}

/*
 *   How to do module test
 *   gcc -DUNIT_TEST -I/usr/include/glib-1.2/ -I/usr/lib/glib/include/ -lglib
 *   sim_parser.c
 *   copy the following example, save as sim.in and run './a.out'
 *   then the FHS is parsered as fhs tree and sim.out is generated
 *   according to the tree

Example1

name1 = {
    item11 = "value11"
    item12 = "value12"
}
name2 = {"value21", "value22", "value23"}
name3 = {
    name31 = "value31"
    name32 = {"value321", "value322", "value323"}
    name33 = {
        name331 = "value331"
        name332 = {"value3321", "value3322", "value3323"}
    }
}



Example2
# No Identifier
= "value1"



Example3
# No =
name1  "value1"


*/
#endif

