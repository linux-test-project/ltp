#ifndef _INC_SIM_PARSER
#define _INC_SIM_PARSER
#include <glib.h>
typedef struct {
       char* name;
       enum{
          SIM_FHS_STRING,
          SIM_FHS_SEQ,
          SIM_FHS_ARRAY,
       }type;
       union{
          char*   str;
          GSList* seq;
          GSList* array;
       }value;
}fhs_node;
void free_fhs_node(GSList *node);
GSList* sim_parser(char *filename);
int  sim_generate(char *filename, GSList * fhs);
#endif
