// line.h : Ballista Test Line
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

#ifndef _LINE_H_
#define _LINE_H_

#include <iostream>
#include <fstream>

using namespace std;

class line
{
public:
  char *p;
  int operator=(const line& rhs);
  int operator=(const char *rhs);
  line(const line& rhs);
  line (const char *name);
  line();
  ~line();
  void test();
};

ostream& operator<< (ostream& os,line l);

#endif // _LINE_H_
