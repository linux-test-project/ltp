/* blexer.c : Ballista lexer - Compiler
   Copyright (C) 1998-2001  Carnegie Mellon University
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include "butil.h"
#include "jlist.h"
#include "blexer.h"

struct jlist symTable;
int init=0;

int yywrap()
{
  return 1;
}

int myCompare(struct jelement *el1,struct jelement *el2)
{
  int i=0;
  if (el1==NULL || el2==NULL)
    jerror ("Error jlist compare function called with null ptr");

#ifdef DEBUG
  printf("Comparing %s and %s \n",((struct tableData *)el1->theData)->lexeme,((struct tableData*)el2->theData)->lexeme);
#endif

  i = strcmp(((struct tableData *)el1->theData)->lexeme,((struct tableData*)el2->theData)->lexeme);

#ifdef DEBUG
  printf ("strcmp returned %i\n",i);
#endif

  return i;

}

void myCopy(struct jelement *el1,struct jelement *el2)
{
  struct tableData *t1,*t2;
  if (el1==NULL ||el2==NULL) 
    jerror("Error, jlist copy function called with null ptr");
  t1 = (struct tableData *) el1->theData;
  t2 = (struct tableData *) el2->theData;
  strcpy (t2->lexeme,t1->lexeme);
  strcpy (t2->owner,t1->owner);  
}

void initSym()
{
#ifdef DEBUG
  printf ("Initializing list: symTable\n");
#endif
  init=1;
  symTable.compare = &myCompare;
  symTable.copydata = &myCopy;
  symTable.sort_ascending = 1;
  symTable.i_am_sorted = 1;
  symTable.stay_sorted = 1;
  symTable.circular = 0;
  symTable.insert_mode = insert_in_order;
  symTable.data_size = sizeof(struct tableData);
  symTable.head = NULL;
  symTable.tail = NULL;
}

struct tableData *symLook(char sym[255])
{
  struct jelement *el;
  struct jelement *rel;
  struct tableData *data;

  data = malloc (sizeof(struct tableData));
  
  if (!init) initSym();
  
  el = (struct jelement *)malloc (sizeof(struct jelement));

  strcpy (data->lexeme,sym);

  el->theData = data;

#ifdef DEBUG
  printf("looking for %s \n",sym);
#endif
  
  rel = find_element(&symTable,el);
  
  if (rel!=NULL) 
    {
#ifdef DEBUG
      printf("found: %s",((struct tableData *)rel->theData)->lexeme);
#endif
      free(el);
      free (data);
      return (struct tableData *)rel->theData;
    }
  else
    {
#ifdef DEBUG
      printf("Adding to symtable: %s\n",data->lexeme);
      fflush (stdout);
#endif
      insert_element(&symTable,el);
      return (struct tableData *)el->theData;
    }
}

int printout(struct jelement *el)
{
  printf ("%s: %s\n",((struct tableData *)el->theData)->lexeme,((struct tableData *)el->theData)->owner);
  return 1;
}

void dumpTable()
{
  do_action(&symTable,&printout,NULL);
}

void freeTable()
{
  destroy_list(&symTable);
}


