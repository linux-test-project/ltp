/* tempname.c : Ballista function to create a temporary name - Compiler
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

char theDir[255];
char thePre[255];
int dir=0;
int pre=0;

void parseCLine(int argc,char *argv[])
{
  int i;
  for (i=2;i<=argc;i++)
    {
      if (i==2)
	{
	  dir=1;
	  strcpy(theDir,argv[i-1]);
	}
      if (i==3)
	{
	  pre=1;
	  strcpy(thePre,argv[i-1]);
	}

    }
}

int main(int argc,char *argv[])
{
  char* ret;

  theDir[0]='\0';
  thePre[0]='\0';


  ret=NULL;


  parseCLine(argc,argv);
  
  if (!dir)
    {

      ret=tmpnam(NULL);
      if (ret==NULL)
	{
	  printf("could not get temp filename\n");
	  exit(1);
	}
      printf("%s\n",ret);
      exit(0);
    }
  ret = tempnam(theDir,thePre);
      if (ret==NULL)
	{
	  printf("could not get temp filename\n");
	  exit(1);
	}
      printf("%s\n",ret);
      exit(0);
}
