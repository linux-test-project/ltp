// genCodeCreator.cpp: creates the script file contain the instructions to create the bug report code
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

//-------------------------------------------------------------------------------------------
// genCodeCreator:  The purpose of this program is to convert an outfile produced by running 
//                  Ballista or the OSTest into a csh script file.  The csh script file will 
//                  contain the instructions to generate stand-alone code to reproduce the 
//                  Abort and Restart failures detected by Ballista.  This stand-alone code
//                  is useful for bug reports.
//-------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ballista.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    cout <<"Usage for genCodeCreator is: genCodeCreator <function Name> <input File Name>" 
         <<" <number of parameters> <output File Name>" << endl;
    cout <<"   where <input File Name> is the results file created by running ballista "
         <<"or ostest" << endl;
    cout <<"         <output File Name> is the file containing the .createCode commands" 
         << endl;
    return 1;
  } 

  
  char buf[1024];
  int fileExists = 0;
  ifstream os;
  ifstream instr(argv[2],ios::in);
  ifstream tempStr(argv[4],ios::in|ios::in);

  if (tempStr)
  {
    fileExists = 1;
    tempStr.close();
  }

  ofstream outstr(argv[4],ios::out|ios::app);

  char* paramType[MAXP];
  char* paramValue[MAXP];

  int numParam = atoi(argv[3]);

  if (!fileExists)
  {
    outstr << "#!/bin/csh" << endl;
    outstr << "cd .." << endl;
  }

  while (!instr.eof())
  {
    int count = 0;
    
    while ((count < (numParam)) && (!instr.eof()))
    {
      instr.ignore(9999,'#');
      if (instr.eof())
      {
        return 0;
      }
      instr >> buf;

      if ((strncmp(buf,"param:",6)) == 0)
      {
        // At this point buf should contain the #param:PARAMTYPE
        paramType[count] = (char *)malloc(strlen(buf)*sizeof(char));
        paramValue[count] = (char *)malloc(sizeof(char) * 1024);

        if ((paramType[count] == NULL) || (paramValue[count] == NULL))
        {       
          outstr <<"# OUT OF MEMORY - INCOMPLETE" <<endl;
          cout <<" ERROR: Ran out of memory running genCodeCreate" << endl;
          return 1; 
        }
        strcpy(paramType[count], &(buf[6]));

        char* temp;
        int first = 1;       

        // parse the paramValue - Note the paramValue is a series of Dial Settings 
        // on the same line as #param:PARAMTYPE each preceed by the paramType_ and 
        // separated by a tab.
        instr.getline(buf,1024,'\n');
        char* dialSetting = NULL;
        char* endSetting = NULL;
        char* bufPtr = buf;

        while ((dialSetting = strstr(bufPtr,paramType[count])) != NULL)
        {
          if (!first)
          {
            temp = strcat(paramValue[count],"lNw");
          }
          else
          {
            strcpy(paramValue[count], "\0");
            first = 0;
          }

          //move dialSetting to the start of the setting value
          dialSetting = &(dialSetting[strlen(paramType[count]) + 1]);
          
          //determine the end of the setting value
          endSetting = strstr(dialSetting,"\t");
          *endSetting = '\0';
          
          temp = strcat(paramValue[count], dialSetting);
          bufPtr = endSetting + 1;
        }
        count++;
      }
    }
       
    // Determine status of the test  - process only Abort and Restarts
    int detStatus = 0;
    char status[10];

    while ( (!detStatus) && (!instr.eof()))
    {
      instr.getline(buf,1024,'\n');

      //ignore fail bit being set when 1023+ chars read
      if (instr.rdstate() == ios::failbit)
      {
        instr.clear();
      }

      if (strncmp(buf,"Done - ",7) == 0)
      {
        if (strstr(buf,"Abort") != NULL)
        {
          strcpy(status,"Abort");
          detStatus = 1;
        }
        else if (strstr(buf,"Restart") != NULL)
        {
          strcpy(status,"Restart");
          detStatus = 1;
        }
        else   //ignore Passes and Setup Failed
        {
          detStatus = 99;
        }
      }            
               
      if (detStatus == 1)
      {
        //output instruction to outstr
        outstr<<"perl ./create_code_standAlone.pl " << argv[1] <<" " << status <<" "
              << numParam <<" ";

        for (int i = 0; i < numParam; i++)
        {
          outstr<<paramType[i] <<" "<<paramValue[i] <<" ";
        }
         
        outstr<<" > "<< argv[1]<<"_bug_report/"<<argv[1];
        for (int i = 0; i < numParam; i++)
        {
          outstr<<"_"<<paramType[i] <<paramValue[i];
        }
        outstr << ".cpp" << endl;
      }
    }
  }
  instr.close();
  outstr.close();
}

