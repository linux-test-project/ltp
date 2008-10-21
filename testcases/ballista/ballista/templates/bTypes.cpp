// bTypes.cpp : Ballista Template Base Definition
// Copyright (C) 1997-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
/*

TITLE
   
   bTypes.cpp

REVISION RECORD

Date            Engineer        Change
----            --------        ------
18 NOV 97       J DeVale        Original release

IMPLEMENTATION DESIGN

1. General
   
   Implementation for accessing random space in file pointer variable

   Type name must not have whitespace
   
2. Specification
   fp_gen.h
                                                
*/

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "bTypes.h"

using namespace std;
//---------------------------------------------------------------------------

paramAccess::paramAccess()
{
  strcpy(_rootName,"ROOT_PARAMETER");
}

//---------------------------------------------------------------------------

void *paramAccess::access(b_param data[])
{
  cerr<<"Error - paramAccess::access() should never be invoked\n";
  cerr<<"The offending type string is "<<data[0]<<endl;
  exit(1);
  return NULL;
}


//---------------------------------------------------------------------------

int paramAccess::commit(b_param tname)
{
  cerr<<"Error - paramAccess::commit() should never be invoked\n";
  cerr<<"The offending type string is "<<tname<<endl;
  exit(1);
  return 1;
}

//---------------------------------------------------------------------------

int paramAccess::cleanup(b_param tname)
{
  cerr<<"Error - paramAccess::commit() should never be invoked\n";
  cerr<<"The offending type string is "<<tname<<endl;
  exit(1);
  return 1;
}

//---------------------------------------------------------------------------

int paramAccess::numDials(b_param tname)
{
  if (strcmp(tname,(char *)typeName())!=0)
    {
      cerr<<"Error - paramAccess::numDials() was invoked ";
      cerr<<"with an invalid typename\n";
      cerr<<"The offending type string is "<<tname<<endl;
      exit(1);
    }
  return 0;
}

//---------------------------------------------------------------------------

int paramAccess::numItems(b_param tname,int dialNumber)
{
  if (strcmp(tname,(char *)typeName())!=0)
    {
      cerr<<"Error - paramAccess::numItems() was invoked with an ";
      cerr<<"invalid typename\n";
      cerr<<"The offending type string is "<<tname<<endl;
      exit(1);
    }
  return 0;
}

//---------------------------------------------------------------------------

b_param *paramAccess::paramName(b_param tname,int dialNumber, int position)
{
  if (strcmp(tname,(char *)typeName())!=0)
    {
      cerr<<"Error - paramAccess::paramName() was invoked with ";
      cerr<<"an invalid typename\n";
      cerr<<"The offending type string is "<<tname<<endl;
      exit(1);
    }
  return NULL;
}

//---------------------------------------------------------------------------

b_param *paramAccess::typeName()
{
  return &_rootName;
}

//---------------------------------------------------------------------------

int paramAccess::distanceFromBase()
{
  return 0;
}

//---------------------------------------------------------------------------

void paramAccess::typeList(b_param list[], int num)
{
  strcpy(list[num],(char *) typeName());
}

//---------------------------------------------------------------------------
