/* butil.c : Ballista Utilities - Compiler
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
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void jerror(char error_text[])
{
	fprintf(stderr,"Ballista Parser run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

void upper(char *text)
{

  int i=0;
  if (text==NULL) jerror("null string passed to upper");
  for (i=0;i<(int)strlen(text);i++)
    text[i] = (char)toupper(text[i]);
}

void lower(char *text)
{
  int i=0;
  if (text==NULL) jerror("null string passed to lower");
  for (i=0;i<(int)strlen(text);i++)
    text[i] = (char)tolower(text[i]);
}
