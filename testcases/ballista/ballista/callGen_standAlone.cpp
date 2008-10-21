// callGen_standAlone.cpp: generates the function call - no server 
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#define qt (char)34 

using namespace std;

char *temp=tempnam(NULL,"cg");

int main(int argc,char *argv[])
{
  char s[255];
  char v[64][255];
  char rt[1024];
  char ft[64]; //function type - function or constructor
  char fname[255];
  ofstream os("callInclude.cpp",ios::out);
  ofstream def("vardefs.cpp",ios::out);
  int rval=0;
  int argnum=0;
  int extras=0;

  if (argc==1)
    {
      cout<<"Usage: "<<argv[0]<<" <callName>"<<endl;
      cout<<"Where callname is the name fo the function to test.\n";
      exit(1);
    }

  sprintf(s,"grep -w %s callTable > %s",argv[1],temp);
  if (system(s))
    {
      cout<<"Error - could not locate"<<argv[1]<<"  in the calltable.\n";
      exit(1);
    }
  cout<<s<<endl;

  ifstream is(temp,ios::in);

  is>>s; //get the include file

  if (strcmp(s,"NULL")!=0)  //include a file
  {
//    cout <<"inside if statement" << endl;
    os<<"#include <"<<s<<">  //included from calltable include field\n";
//    cout <<" after writing out to os:" << endl;
//    cout<<"#include <"<<s<<">  //included from calltable include field\n\n" <<endl;
  }
  
  is>>ft; //get the function type
  is>>rt; //get return type
  
  // Check if there is a printable return type
  if ((strcmp(rt,"NULL")!=0) && 
      (strcmp(rt,"void")!=0) &&
      (strcmp(rt,"div_t")!=0) &&
      (strcmp(rt,"ldiv_t")!= 0))
  {
      rval=1;
      def <<rt<<"\trval;\n";
      // standalone addition
      os <<rt<<" rval;\n";
  }
  else  // added for standalone
  {
      os <<"\n";
  }


  is>>fname; //get function call
  //cout <<"after fname set " <<fname <<endl;

  
  while (is >> ws, !is.eof())
    {
      is>>s;//get param?
//      cout <<"s is set to " <<s << endl;      

      if(strcmp(s,"+")==0)
	extras=1;  //extra type for creation follow, but not passed as args
      else
      {
      if (strlen(s)>0)
	{
 //         cout << "strlen > 0" << endl;
	  argnum++;
	  if (extras!=0)extras++;
	  strcpy(v[argnum],s);
	  def<<s<<" param"<<argnum<<";\n";
//	  
          os<<"ref["<<argnum<<"] = (void **)"<<" param"<<argnum<<".access(params["<<argnum-1<<"]);\n";
	  
	  //find the real type of the parameter
	  ifstream ins("dataTypes");
	  char z[255];
	  int flag=0;
	  while (!ins.eof()&&!flag)
	    {
	      ins>>z;
	      if (strcmp(s,z)==0)  //found it?
		{
		  ins>>z;
		  cout<<"found it: "<<s<<" = "<<z<<endl;
		  flag=1;
		}
	    }

	  if (!flag) //didn't find it
	    {
	      cerr<<"Error, could not find type associated with "<<s<<" in dataType file\n";
	      exit(1);
	    }

//	  
          os<<z<<"* temp"<<argnum<<" = ("<<z<<"*) ref["<<argnum<<"];\n";

	}
      }
    }

// 2 lines
  for (int i=1;i<=(argnum);i++)
    os<<"param"<<i<<".commit(params["<<i-1<<"][0]);\n";

  //output try catch framework

  os<<"\n";


  if (rval==1)
    os<<"rval = ";

  if (strcmp(ft,"constructor")==0) //its a constructor
      os <<"new ";


  os<<fname<<"(";
  
  
  for (int i=1;i<=(argnum-(extras==0?0:extras-1));i++)
    {
      if (i>1) os<<" , ";
      os<<"*temp"<<i;
    }
  os<<");\n";

  if (rval ==1)
  {
    os <<"/* cout statement needed so that function call is not optimized out */ \n";
    os <<"cout<<"<<qt<<"rval:"<<qt<<"<<rval<<endl;\n";
  }

// 2 lines
  for (int i=1;i<=argnum;i++)
    os<<"param"<<i<<".cleanup(params["<<i-1<<"][0]);\n";

  os<<"\t //done\n";

  if (rval==1)
    os<<"cout<<"<<qt<<"rval:"<<qt<<"<<rval<<endl;\n";
  
  //finish try-catch framework
  os<<"\n//replacetoken\n#include "<<qt<<"userCatches.cpp"<<qt<<endl<<endl;

  is.close();
  os.close();
  def.close();
}
