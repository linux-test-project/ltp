// genCode.cpp: generates the Ballista test execution code from various pieces  
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
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

// From: John Peter DeVale <jdevale@ece.cmu.edu>
// To: karner@andrew.cmu.edu
// Message-ID: <Pine.OSF.4.02.9811090812520.3462-100000@skate.ece.cmu.edu>
//
// Replacer, which as you point out only inlines strings, is an artifact of
// some wierd bug in cxx on digital unix.  As it turns out, cxx dies a
// horrible screaming death if you try to inline code with #include
// directives in the middle of a function.  The same thing compiles clean on
// gcc.  It's odd.  So, I wrote replacer to actually shove the code into
// place to work around the problem.

int main()
{
  char buf[1024];
  ifstream os;

  while (!cin.eof())
    {
      cin.getline(buf,1024,'\n');
      if (strcmp(buf,"//replacekey")==0)
	{
	  
	  cout<<"//inlining userSetup, callInclude, and userShutdown\n";

	  os.open("userSetup.cpp",ios::in);
	  while (!os.eof())
	    {
	      os.getline(buf,1024,'\n');
	      cout<<buf<<endl;
	    }
	  os.close();
	  os.open("callInclude.cpp",ios::in);
	  while (!os.eof())
	    {
	      os.getline(buf,1024,'\n');
	      cout<<buf<<endl;
	    }
	  os.close();
	  os.open("userShutdown.cpp",ios::in);
	  while (!os.eof())
	    {
	      os.getline(buf,1024,'\n');
	      cout<<buf<<endl;
	    }
	  os.close();


	  cin.getline(buf,1024,'\n');
	  cin.getline(buf,1024,'\n');
	  cin.getline(buf,1024,'\n');
	}
      else if (strcmp(buf,"//replacetoken")==0)
	{
	  
	  cout<<"//inlining userCatches\n";

	  os.open("userCatches.cpp",ios::in);
	  while (!os.eof())
	    {
	      os.getline(buf,1024,'\n');
	      cout<<buf<<endl;
	    }
	  os.close();
	  cin.getline(buf,1024,'\n');
	}
      else if (strcmp(buf,"//placeVariables")==0)
	{
	  
	  cout<<"//inlining variable instantiations\n";

	  os.open("vardefs.cpp",ios::in);
	  while (!os.eof())
	    {
	      os.getline(buf,1024,'\n');
	      cout<<buf<<endl;
	    }
	  os.close();
	  cin.getline(buf,1024,'\n');
	}
      else
	cout<<buf<<endl;

    }

}
