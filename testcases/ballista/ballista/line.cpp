// line.cpp : Ballista test line
// Copyright (C) 1998-2001  Carnegie Mellon University
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

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include "line.h"

using namespace std;

line::line()
{
  p=NULL;
}
line::line(const char *name)
{
  p=new char[strlen(name)+1];
  strcpy(p,name);
}
line::line(const line& rhs)
{
  p=new char[strlen(rhs.p)+1];
  strcpy(p,rhs.p);
}
line::~line()
{
  if (p!=NULL) delete[] p;
}
int line::operator=(const line& rhs)
{
  if (p!=NULL) delete[] p;
  p=new char[strlen(rhs.p)+1];
  if (p==NULL) return 0;
  strcpy(p,rhs.p);
  return 1;
}
int line::operator=(const char *rhs)
{
  if (p!=NULL) delete[] p;
  p=new char[strlen(rhs)+1];
  if (p==NULL) return 0;
  strcpy(p,rhs);
  return 1;
}

void line::test()
{
  line l1("this");
  line l2="a test";
  line l3(l2);
  l2 = " is ";
  cout<<l1<<l2<<l3<<endl;
}

ostream& operator<< (ostream& os,line l)
{
  os<<l.p;
  return os;
}
