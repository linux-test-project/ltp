/* bparser.c : Ballista parser - Compiler
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

#include <ctype.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <vector>
#include <vector>
#include <unistd.h>
#include "tokens.h"
#include "butil.h"

#define MYMAP map<int, translationEntry,less<int>>
#define MAXLIST 10
using std::endl;
using std::cerr;
using std::cout;
using std::cin;
using std::ios;
using std::vector;
using std::ofstream;
using std::ostream;

vector<int> parseStack;

//quote character
#define qt (char)34 


int helpFlag=0;
int dumpFlag=0;
int lineNumber=1;
int stackDump=0;
int dumpActions=0;
int dialCount=0;
int dialSetting=0;
int conditionNum=0;

char lexeme[256];
char dname[256];

char *h1name=tempnam(NULL,"h1x");
char *h2name=tempnam(NULL,"h2x");
char *h3name=tempnam(NULL,"h3x");
char *h4name=tempnam(NULL,"h4x");

char *c1name=tempnam(NULL,"c1x");
char *c2name=tempnam(NULL,"c2x");
char *c3name=tempnam(NULL,"c3x");
char *c4name=tempnam(NULL,"c4x");
char *c5name=tempnam(NULL,"c5x");
char *c6name=tempnam(NULL,"c6x");
char *c7name=tempnam(NULL,"c7x");
char *c8name=tempnam(NULL,"c8x");
char *c9name=tempnam(NULL,"c9x");
char *c10name=tempnam(NULL,"c10x");
char *c11name=tempnam(NULL,"c11x");
char *c12name=tempnam(NULL,"c12x");
char *cfname=tempnam(NULL,"cfx");



ofstream h1(h1name,ios::out); //name defines
ofstream h2(h2name,ios::out); //includes and global defines
ofstream h3(h3name,ios::out); //private member data
ofstream h4(h4name,ios::out); //public access functions

ofstream c1(c1name,ios::out);
ofstream c2(c2name,ios::out);
ofstream c3(c3name,ios::out);
ofstream c4(c4name,ios::out);
ofstream c5(c5name,ios::out);
ofstream c6(c6name,ios::out);
ofstream c7(c7name,ios::out);
ofstream c8(c8name,ios::out);
ofstream c9(c9name,ios::out);
ofstream c10(c10name,ios::out);
ofstream c11(c11name,ios::out);
ofstream c12(c12name,ios::out);
ofstream cf(cfname,ios::out);


ofstream *codeOut;              //where code goes

char obName[256];               //name of object
char obNamebak[256];	   //backup for uppering of include control directives.

void getTokenString(char *s,int i);

struct translationEntry
{
  int num;
  enum tokenid data[MAXLIST];
};

translationEntry tTable[NUM_NONTERMS][NUM_TOKENS];

ostream& operator<<(ostream& s, translationEntry& t)
{
  int i;
  for (i=0;i<t.num;i++)
    s<<t.data[i]<<"/t";
  if (i==0) s<<"No entries in translation table";
  s<<endl;
  return s;
}

void addEntry(translationEntry& e,int i)
{
  char s[30];
  strcpy(s,"Translation table entry full");
  if (e.num==MAXLIST)
    jerror(s);
  e.data[e.num]=(enum tokenid)i;
  e.num++;
}

int isNonTerm(int i)
{
  int t=0;
  if (i>=NONTERM_BASE && i<(NONTERM_BASE+NUM_NONTERMS)) t=1;
  return t;
}

int isTerm(int i)
{
  int t=0;
  if (i>=TOKEN_BASE && i<(TOKEN_BASE+NUM_TOKENS)) t=1;
  return t;
}

int isAction(int i)
{
  int t=0;
  if (i>=ACTION_BASE && i<(ACTION_BASE+NUM_ACTIONS)) t=1;
  return t;
}

int doAction(int i)
{
  char s[255];
  
  if (dumpActions)
    {
      getTokenString(s,i);
      cout<<"Performing action "<<s<<" lexeme = "<<lexeme<<endl;
    }

  switch(i)
    {
    case (NAME_DEFINES): //define the name in the .h file
    	// don't lower case this any longer to support the SEI naming conventions
      //lower(lexeme);
      h1<<"#define CLASSNAME "<<lexeme<<endl;
      h1<<"#define CLASS_STRING "<<(char)34<<lexeme<<(char)34<<"\n";
      strcpy(obName,lexeme);
      strcpy(obNamebak,lexeme);
      break;
    case (TYPE_DEFINES): //define the class type in the .h file
      h1<<"#define CLASSTYPE "<<lexeme<<endl;
      break;
    case (HERITAGE_DEFINES): //define its parent class
      h1<<"#define CLASSPARENT "<<lexeme<<endl;
      break;
    case (SET_H2): //Sets the default out stream for code to H2
      codeOut=&h2;
      break;
    case (SET_NULL): //sets the default out stream to null
      codeOut=NULL;
      break;
    case (OUTPUT_CODE): //outputs code to wherever CodeOut is set to
      if (codeOut==NULL) 
	//jerror("Internal Parse Rule error.  Code output file set to NULL");
	cerr<<lexeme;
      else
	*codeOut<<lexeme;
      break;
    case (OUTPUT_CODE_SP): //outputs whitespace to wherever CodeOut is set to
      if (codeOut==NULL) 
	//jerror("Internal Parse Rule error.  Code output file set to NULL");
	cerr<<" ";
      else
	*codeOut<<" ";
      break;
    case (OUTPUT_CODE_NL): //outputs newln to wherever CodeOut is set to
      if (codeOut==NULL) 
	//jerror("Internal Parse Rule error.  Code output file set to NULL");
	cerr<<endl;
      else
	*codeOut<<endl;
      break;
    case (SET_H3):
      codeOut = &h3;
      break;
    case (INC_DIAL):
      dialCount++;
      dialSetting=0;

      //output The dial cases in paramName
      c11<<endl<<"case "<<dialCount<<":"<<endl;

      break;
    case (OUTPUT_BPARAM):
      upper(lexeme);
      dialSetting++;  //another (or first) setting for current dial
      
      //generate switch statements in paramName method
      if (dialSetting==1)
	{
	  c11<<" switch (position)"<<endl;
	  c11<<"   {"<<endl;
	}
      c11<<"      case "<<dialSetting<<":"<<endl;
      c11<<"         return "<<obName<<lexeme<<"();"<<endl;
      c11<<"         break;"<<endl;


      if(dialSetting==1 && dialCount==1) c5<<"   int dataPTR =0;"<<endl<<endl;

      /*
	if(dialSetting==1 && dialCount>1 && strcmp(dname,"done")) //output final else block 
	{
	c5<<"  else"<<endl;
	c5<<"    {"<<endl;
	c5<<"      cerr<<"<<qt<<"Error: Unknown setting for the "<<qt<<endl;
	c5<<"          <<"<<qt<<dname<<qt<<endl;
	c5<<"          <<"<<qt<<" dial of the data object "<<qt<<endl;
	c5<<"          <<CLASS_STRING"<<endl; 
	c5<<"          <<"<<qt<<". "<<qt<<"<<endl"<<endl; 
	c5<<"          <<"<<qt<<"The offending string is : "<<qt<<endl;  
	c5<<"          <<data[dataPTR]"<<endl; 
	c5<<"          <<endl;"<<endl; 
	c5<<"      exit(1);"<<endl; 
	c5<<"    }"<<endl;
	strcpy(dname,"done");
	
	}
      */
      if(dialSetting==1) c5<<endl<<"   dataPTR++;"<<endl;
      else c5<<"   else ";

 
      c5<<"if (strcmp(data[dataPTR],_"<<obName<<lexeme<<")==0)"<<endl;
      lower(lexeme);
      c5<<"      "<<obName<<"_"<<lexeme<<" = 1;"<<endl;
      upper(lexeme);
	
      h3<<"b_param _"<<obName<<lexeme<<";\n";   //declaration
      //initialization in constructor
      c1<<"strcpy(_"<<obName<<lexeme<<","<<qt<<obName<<"_"<<lexeme<<qt<<");\n";
      h4<<"b_param *"<<obName<<lexeme<<"();\n"; //access function decl
      
      //create access function
      c2<<"b_param *"<<obName<<"::"<<obName<<lexeme<<"()"<<endl;
      c2<<"{"<<endl;
      c2<<"   return &_"<<obName<<lexeme<<";"<<endl;
      c2<<"}"<<endl<<endl;

      lower(lexeme);
      h3<<"int "<<obName<<"_"<<lexeme<<";\n";
      c4<<obName<<"_"<<lexeme<<" = 0;\n";

      break;
    case (OUTPUT_DNUM):
      h1<<"#define NUMBER_OF_DIALS "<<dialCount<<endl;
      break;
    case (OUTPUT_INTPARAM):
      upper(lexeme);

      //numItems method outputs
      c10<<endl<<"   case "<<dialCount<<":"<<endl;
      c10<<"      return 1;"<<endl;
      c10<<"      break;"<<endl;

      //output case statements for paramName
      c11<<"      if (position==1)"<<endl;
      c11<<"        return "<<obName<<"NO"<<lexeme<<"();"<<endl;
      c11<<"      else "<<endl;
      c11<<"          cerr<<"<<qt<<"Error, invalid position number passed to "<<qt<<endl;
      c11<<"              <<CLASS_STRING<<"<<qt<<"::paramName\\n"<<qt<<endl;
      c11<<"              <<"<<qt
	  <<"Please check declaration files.  Dial number passed was"<<qt<<endl;
      c11<<"              <<dialNumber<<"<<qt<<" position "
	  <<qt<<"<<position<<"<<qt<<".\\n"<<qt<<";"<<endl;
      c11<<"          exit(1);"<<endl;
      c11<<"      break;"<<endl;

      //create declarations in class definition
      h3<<"b_param _"<<obName<<lexeme<<";\n";
      h3<<"b_param _"<<obName<<"NO"<<lexeme<<";\n";
      h3<<"int _"<<obName<<lexeme<<"_INT;\n";
      h3<<"#define _"<<lexeme<<"_INT _"<<obName<<lexeme<<"_INT"<<endl;
      //create initialization code in constructor
      
      c1<<"strcpy(_"<<obName<<"NO"<<lexeme<<","<<qt<<"INTDIAL_"<<obName<<"_NO"
	<<lexeme<<qt<<");\n";
      c1<<"strcpy(_"<<obName<<lexeme<<","<<qt<<"INTDIAL_"<<obName
	<<"_"<<lexeme<<qt<<");\n";
      c1<<"_"<<obName<<lexeme<<"_INT = 0;"<<endl;

      //create declarations for access functions
      h4<<"b_param *"<<obName<<lexeme<<"();\n";
      h4<<"b_param *"<<obName<<"NO"<<lexeme<<"();\n";

      //create access methods
      c2<<"b_param *"<<obName<<"::"<<obName<<"NO"<<lexeme<<"()"<<endl;
      c2<<"{"<<endl;
      c2<<"   return &_"<<obName<<"NO"<<lexeme<<";"<<endl;
      c2<<"}"<<endl<<endl;

      c2<<"b_param *"<<obName<<"::"<<obName<<lexeme<<"()"<<endl;
      c2<<"{"<<endl;
      c2<<"   return &_"<<obName<<lexeme<<";"<<endl;
      c2<<"}"<<endl<<endl;

      //create access initializer...
      if(dialSetting==1) c5<<"   int dataPTR =0;"<<endl<<endl;
      c5<<endl;
      c5<<"  dataPTR++;"<<endl;
      c5<<"  if (strcmp(data[dataPTR],_"<<obName<<lexeme<<")==0)"<<endl;
      c5<<"     sscanf(data[++dataPTR],"<<qt<<"%i"<<qt
	<<",&_"<<obName<<lexeme<<"_INT);"<<endl;
      c5<<"  else if (strcmp(data[dataPTR],_"
	<<obName<<"NO"<<lexeme<<")!=0)"<<endl;
      c5<<"    {"<<endl;
      c5<<"      cerr<<"<<qt<<"Error: Unknown setting for the "<<qt<<endl;
      c5<<"          <<"<<qt<<lexeme<<qt<<endl;
      c5<<"          <<"<<qt<<" dial of the data object "<<qt<<endl;
      c5<<"          <<CLASS_STRING"<<endl; 
      c5<<"          <<"<<qt<<". "<<qt<<"<<endl"<<endl; 
      c5<<"          <<"<<qt<<"The offending string is : "<<qt<<endl;  
      c5<<"          <<data[dataPTR]"<<endl; 
      c5<<"          <<endl;"<<endl; 
      c5<<"      exit(1);"<<endl; 
      c5<<"    }"<<endl<<endl;;



      lower(lexeme);

      c4<<obName<<"_"<<lexeme<<" = 0;"<<endl;
      h3<<"int "<<obName<<"_"<<lexeme<<";\n";

     break;    
    case (GET_DNAME):
      strcpy(dname,lexeme);
      upper(dname);
      break;
    case (FINISH_ENUM):
      c5<<"  else"<<endl;
      c5<<"    {"<<endl;
      c5<<"      cerr<<"<<qt<<"Error: Unknown setting for the "<<qt<<endl;
      c5<<"          <<"<<qt<<dname<<qt<<endl;
      c5<<"          <<"<<qt<<" dial of the data object "<<qt<<endl;
      c5<<"          <<CLASS_STRING"<<endl; 
      c5<<"          <<"<<qt<<". "<<qt<<"<<endl"<<endl; 
      c5<<"          <<"<<qt<<"The offending string is : "<<qt<<endl;  
      c5<<"          <<data[dataPTR]"<<endl; 
      c5<<"          <<endl;"<<endl; 
      c5<<"      exit(1);"<<endl; 
      c5<<"    }"<<endl;

      //paramName method outputs
      c11<<endl<<"      default:"<<endl;
      c11<<"          cerr<<"<<qt<<"Error, invalid position number passed to "<<qt<<endl;
      c11<<"              <<CLASS_STRING<<"<<qt<<"::paramName\\n"<<qt<<endl;
      c11<<"              <<"<<qt
	  <<"Please check declaration files.  Dial number passed was"<<qt<<endl;
      c11<<"              <<dialNumber<<"<<qt<<" position "
	  <<qt<<"<<position<<"<<qt<<".\\n"<<qt<<";"<<endl;
      c11<<"          exit(1);"<<endl;
      c11<<"   }"<<endl;
      c11<<"   break;"<<endl;
     
      //numItems method outputs
      c10<<endl<<"   case "<<dialCount<<":"<<endl;
      c10<<"      return "<<dialSetting<<";"<<endl;
      c10<<"      break;"<<endl;
      break;
    case (GENERATE_CONDITIONAL):
      //output conditionals for action blocks
      conditionNum++;
      lower(lexeme);
      
      if (codeOut!=NULL)
	{
	  if (conditionNum==1) 
	    *codeOut<<endl<<"if ("<<obName<<"_"<<lexeme<<"==1";
	  else *codeOut<<" || "<<obName<<"_"<<lexeme<<"==1";
	}
      else
	{
	  if (conditionNum==1) cout<<endl<<"if ("<<obName<<"_"<<lexeme<<"==1";
	  else cout<<" || "<<obName<<"_"<<lexeme<<"==1";
	}

      break;
    case (CLOSE_CONDITIONAL):
      if (codeOut!=NULL)
	*codeOut<<")"<<endl<<"{"<<endl;
      else
	cout<<")"<<endl<<"{"<<endl;
      break;
    case (CLOSE_BLOCK):
      if (codeOut!=NULL)
	*codeOut<<endl<<"}"<<endl;
      else
	cout<<endl<<"}"<<endl;
      break;
    case (RESET_CNUM):
      conditionNum=0;
      break;
    case (SET_C7):
      codeOut = &c7;
      break;
    case (SET_C8):
      codeOut = &c8;
      break;
    case (SET_C9):
      codeOut = &c9;
      break;
    case (SET_C10):
      codeOut = &c10;
    case (SET_C12):
      codeOut = &c12;
      break;
    default:  
      sprintf(s,"Internal parse error - undefined action #%i",i);
      jerror(s);
   }
  return 0;
}

void getTokenString(char *s,int i)
{
  switch(i)
    {
    case (DECLARATION): strcpy(s,"delaration");
      break;
    case (HERITAGE): strcpy(s,"heritage");
      break;
    case (INCLUDE_BLOCK): strcpy(s,"include_block");
      break;
    case (GLOBAL_DBLOCK): strcpy(s,"global_define_block");
      break;
    case (DIALSBLOCK): strcpy(s,"dials_block");
      break;
    case (ACCESSBLOCK): strcpy(s,"access_block");
      break;
    case (COMMITBLOCK): strcpy(s,"commit_block");
      break;
    case (CLEANUPBLOCK): strcpy(s,"cleanup_block");
      break;
    case (DIAL_LIST): strcpy(s,"dial_list");
      break;
    case (ENUM_DECL): strcpy(s,"enum_dial_decl.");
      break;
    case (INT_DECL): strcpy(s,"int_dial_decl.");
      break;
    case (BLOCK_LIST): strcpy(s,"blocklist");
      break;
    case (VARBLOCK): strcpy(s,"varblock");
      break;
    case (VAREST): strcpy(s,"varREST");
      break;
    case (CBLOCK): strcpy(s,"code_block");
      break;
    case (CODELIST): strcpy(s,"code_list");
      break;
    case (SET_LIST): strcpy(s,"set_list");
      break;
    case (SETREST): strcpy(s,"set*rest*");
      break;

    case (ACCESS_ID): strcpy(s,"access");
      break;
    case (CHAR_ID): strcpy(s,"*char*");
      break;
    case (CLEANUP_ID): strcpy(s,"cleanup");
      break;
    case (CODE_ID): strcpy(s,"*code*");
      break;
    case (CODE_SP): strcpy(s,"*code space*");
      break;
    case (CODE_NL): strcpy(s,"*code newline*");
      break;
    case (COLON_ID): strcpy(s,":");
      break;
    case (COMMA_ID): strcpy(s,",");
      break;
    case (COMMIT_ID): strcpy(s,"commit");
      break;
    case (DIALS_ID): strcpy(s,"dials");
      break;
    case (ENUM_DIAL_ID): strcpy(s,"enum_dial");
      break;
    case (EOL_ID): strcpy(s,"*eoln*");
      break;
    case (END): strcpy(s,"*end*");
      break;
    case (GLOBAL_DEFINES_ID): strcpy(s,"global_defines");
      break;
    case (INCLUDES_ID): strcpy(s,"includes");
      break;
    case (INT_DIAL_ID): strcpy(s,"int_dial");
      break;
    case (LEFT_BRACE_ID): strcpy(s,"{");
      break;
    case (LEFT_BRACKET_ID): strcpy(s,"[");
      break;
    case (NAME_ID): strcpy(s,"name");
      break;
    case (PARENT_ID): strcpy(s,"parent");
      break;
    case (RIGHT_BRACE_ID): strcpy(s,"}");
      break;
    case (RIGHT_BRACKET_ID): strcpy(s,"]");
      break;
    case (SEMI_ID): strcpy(s,";");
      break;
    case (STRING_ID): strcpy(s,"stringID");
      break;
    case (VAR_ID): strcpy(s,"var_ID");
      break;
    case (EPSILON): strcpy(s,"*epsilon*");
      break;
    case (SYNCH): strcpy(s,"*synch*");
      break;

    case (NAME_DEFINES): strcpy(s,"@name define action@");
      break;
    case (TYPE_DEFINES): strcpy(s,"@type define action@");
      break;
    case (HERITAGE_DEFINES): strcpy(s,"@heritage define action@");
      break;
    case (SET_H2): strcpy(s,"@set code output to file h2@");
      break;
    case (OUTPUT_CODE): strcpy(s,"@output code@");
      break;
    case (OUTPUT_CODE_SP): strcpy(s,"@output code sp@");
      break;
    case (OUTPUT_CODE_NL): strcpy(s,"@output code nl@");
      break;
    case (SET_NULL): strcpy(s,"@set code out to NULL@");
      break;
    case (SET_H3): strcpy(s,"@set code out to h3@");
      break;
    case (INC_DIAL): strcpy(s,"@increment dial#@");
      break;
    case (OUTPUT_BPARAM): strcpy(s,"@generate enum memberVars@");
      break;
    case (OUTPUT_DNUM): strcpy(s,"@output NUMBER_OF_DIALS@");
      break;
    case (OUTPUT_INTPARAM): strcpy(s,"@generate intDial vars@");
      break;
    case (GET_DNAME): strcpy(s,"@capture dial name@");
      break;
    case (FINISH_ENUM): strcpy(s,"@finish enum dial instance@");
      break;
    case (GENERATE_CONDITIONAL): strcpy(s,"@generate conditional@");
      break;
    case (CLOSE_CONDITIONAL): strcpy(s,"@close conditional@");
      break;
    case (CLOSE_BLOCK): strcpy(s,"@close block@");
      break;
    case (RESET_CNUM): strcpy(s,"@ reset conditional number@");
      break;
    case (SET_C7): strcpy(s,"@set code out to c7@");
      break;
    case (SET_C8): strcpy(s,"@set code out to c8@");
      break;
    case (SET_C9): strcpy(s,"@set code out to c9@");
      break;
    case (SET_C10): strcpy(s,"@set code out to c10@");
      break;
    case (SET_C12): strcpy(s,"@set code out to c12@");
      break;

    default: sprintf(s,"Error, unrecognized token %i",i);
    }
}

void dumpEntry(translationEntry &e)
  //Output the data in one entry to stdout
{
  char s[60];
  for (int i=e.num-1;i>=0;i--)
    {
      getTokenString(s,e.data[i]);
      cout<<s<<" ";
    }
}

void dumpTable()
{
  char s[60];
  char arrow[100];
  for (int i=0;i<NUM_NONTERMS;i++)
    for (int j=0;j<NUM_TOKENS;j++)
      {
	if (tTable[i][j].num!=0)
	  {
	    getTokenString(s,i+NONTERM_BASE);
	    printf("%20s",s);
	    getTokenString(s,j+TOKEN_BASE);
	    sprintf(arrow," -- %s --> \t",s);
	    printf("%25s",arrow);
	    dumpEntry(tTable[i][j]);
	    cout<<endl<<endl;
	  }
      }
}
/* initialize table with translation mappings staticly rather than
   read in from file.  Its a bit more confusing and less modifiable
   but its faster, and people can't muck with the file.
*/

void initTable()
{
  //Declaration NT
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],SEMI_ID);
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],VAR_ID);
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],NAME_DEFINES);
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],VAR_ID);
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],TYPE_DEFINES);
  addEntry(tTable[DECLARATION-NONTERM_BASE][NAME_ID - TOKEN_BASE],NAME_ID);

  addEntry(tTable[DECLARATION-NONTERM_BASE][PARENT_ID - TOKEN_BASE],SYNCH);

  //Heritage NT

  addEntry(tTable[HERITAGE-NONTERM_BASE][PARENT_ID - TOKEN_BASE],SEMI_ID);
  addEntry(tTable[HERITAGE-NONTERM_BASE][PARENT_ID - TOKEN_BASE],VAR_ID);
  addEntry(tTable[HERITAGE-NONTERM_BASE][PARENT_ID - TOKEN_BASE],
	   HERITAGE_DEFINES);

  addEntry(tTable[HERITAGE-NONTERM_BASE][PARENT_ID - TOKEN_BASE],PARENT_ID);

  addEntry(tTable[HERITAGE-NONTERM_BASE][INCLUDES_ID - TOKEN_BASE],SYNCH);

  //INCLULDE_BLOCK NT

  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],RIGHT_BRACKET_ID);
  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],SET_NULL);
  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],CBLOCK);
  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],SET_H2);
  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],LEFT_BRACKET_ID);
  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [INCLUDES_ID - TOKEN_BASE],INCLUDES_ID);

  addEntry(tTable[INCLUDE_BLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID - TOKEN_BASE],SYNCH);

  //GLOBAL_DBLOCK NT 
 
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],RIGHT_BRACKET_ID);
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],SET_NULL);
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],CBLOCK);
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],SET_C12);
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],LEFT_BRACKET_ID);
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [GLOBAL_DEFINES_ID-TOKEN_BASE],GLOBAL_DEFINES_ID);
  
  addEntry(tTable[GLOBAL_DBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE],SYNCH);
  
  //DIALS_BLOCK
  
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE],RIGHT_BRACKET_ID );
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE],OUTPUT_DNUM );
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE],DIAL_LIST);
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE],LEFT_BRACKET_ID );
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [DIALS_ID-TOKEN_BASE], DIALS_ID);
  
  addEntry(tTable[DIALSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE],SYNCH);
  
  //ACCESS BLOCK
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], RIGHT_BRACKET_ID);
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], SET_NULL);
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], BLOCK_LIST);
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], SET_C7);
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], LEFT_BRACKET_ID);
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [ACCESS_ID-TOKEN_BASE], ACCESS_ID);
  
  addEntry(tTable[ACCESSBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE], SYNCH);
  
  //COMMIT BLOCK
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],RIGHT_BRACKET_ID);
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],SET_NULL);
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],BLOCK_LIST);
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],SET_C8);
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],LEFT_BRACKET_ID);
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [COMMIT_ID-TOKEN_BASE],COMMIT_ID);
  
  addEntry(tTable[COMMITBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],SYNCH);
  
  //CLEANUP BLOCK
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],RIGHT_BRACKET_ID);
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],SET_NULL);
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],BLOCK_LIST);
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],SET_C9);
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],LEFT_BRACKET_ID);
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [CLEANUP_ID-TOKEN_BASE],CLEANUP_ID);
  
  addEntry(tTable[CLEANUPBLOCK-NONTERM_BASE]
	   [END-TOKEN_BASE],SYNCH);
  
  //DIAL LIST
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE], DIAL_LIST);
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE], INT_DECL);
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE], INC_DIAL);
  
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], DIAL_LIST);
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], ENUM_DECL);
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], INC_DIAL);
  
  addEntry(tTable[DIAL_LIST-NONTERM_BASE]
	   [RIGHT_BRACKET_ID-TOKEN_BASE], EPSILON);
  
  //ENUM DIAL DECLARATIONS
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], SEMI_ID);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], FINISH_ENUM);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], SET_LIST);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], COLON_ID);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], VAR_ID);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], GET_DNAME);
  addEntry(tTable[ENUM_DECL-NONTERM_BASE]
	   [ENUM_DIAL_ID-TOKEN_BASE], ENUM_DIAL_ID);

  //INTEGER DIAL DECLARATIONS
  addEntry(tTable[INT_DECL-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE], SEMI_ID);
  addEntry(tTable[INT_DECL-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE],VAR_ID );
  addEntry(tTable[INT_DECL-NONTERM_BASE]
  	   [INT_DIAL_ID-TOKEN_BASE],OUTPUT_INTPARAM );
  addEntry(tTable[INT_DECL-NONTERM_BASE]
	   [INT_DIAL_ID-TOKEN_BASE],INT_DIAL_ID );

  //BLOCKLIST
  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],BLOCK_LIST);
  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],CBLOCK);

  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],BLOCK_LIST);
  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],VARBLOCK);
  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],RESET_CNUM);

  addEntry(tTable[BLOCK_LIST-NONTERM_BASE]
	   [RIGHT_BRACKET_ID-TOKEN_BASE],EPSILON);

  //VARBLOCK
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],CLOSE_BLOCK);
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],CBLOCK);
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],CLOSE_CONDITIONAL);
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],VAREST);
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],VAR_ID);
  addEntry(tTable[VARBLOCK-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],GENERATE_CONDITIONAL);

  //VAREST
  addEntry(tTable[VAREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],VAREST);
  addEntry(tTable[VAREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],VAR_ID);
  addEntry(tTable[VAREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],GENERATE_CONDITIONAL);
  addEntry(tTable[VAREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],COMMA_ID);

  addEntry(tTable[VAREST-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],EPSILON);
  
  //CBLOCK
  addEntry(tTable[CBLOCK-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],RIGHT_BRACE_ID);
  addEntry(tTable[CBLOCK-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],CODELIST);
  addEntry(tTable[CBLOCK-NONTERM_BASE]
	   [LEFT_BRACE_ID-TOKEN_BASE],LEFT_BRACE_ID);

  //CODELIST
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_ID-TOKEN_BASE],CODELIST);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_ID-TOKEN_BASE],CODE_ID);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_ID-TOKEN_BASE],OUTPUT_CODE);

  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_NL-TOKEN_BASE],CODELIST);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_NL-TOKEN_BASE],CODE_NL);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_NL-TOKEN_BASE],OUTPUT_CODE_NL);

  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_SP-TOKEN_BASE],CODELIST);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_SP-TOKEN_BASE],CODE_SP);
  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [CODE_SP-TOKEN_BASE],OUTPUT_CODE_SP);

  addEntry(tTable[CODELIST-NONTERM_BASE]
	   [RIGHT_BRACE_ID-TOKEN_BASE],EPSILON);

  //CODE_TYPE


  //SET_LIST
  addEntry(tTable[SET_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],SETREST);
  addEntry(tTable[SET_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],VAR_ID);
  addEntry(tTable[SET_LIST-NONTERM_BASE]
	   [VAR_ID-TOKEN_BASE],OUTPUT_BPARAM);

  //SETREST
  addEntry(tTable[SETREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],SET_LIST);
  addEntry(tTable[SETREST-NONTERM_BASE]
	   [COMMA_ID-TOKEN_BASE],COMMA_ID);


  addEntry(tTable[SETREST-NONTERM_BASE]
	   [SEMI_ID-TOKEN_BASE],EPSILON);
}

void parseCLine(int argc,char *argv[])
{
  int i;
  for (i=2;i<=argc;i++)
    {
      if (!strcmp(argv[i-1],"-h")) helpFlag=1;
      if (!strcmp(argv[i-1],"-dump")) dumpFlag=1;
      if (!strcmp(argv[i-1],"-s")) stackDump=1;
      if (!strcmp(argv[i-1],"-a")) dumpActions=1;
    }
}

void dumpStack()
{
  int l=1;
  char s[60];
  cout<<"Dumping parse stack...\n";
  for (vector<int>::reverse_iterator i=parseStack.rbegin();
       i!=parseStack.rend();
       i++)
    {
      getTokenString(s,*i);
      cout<<"Level "<<l++<<": "<<s<<endl;
    }
}

int match(int token)
  //returns 1 for match, 0 for NT conversion
{
  int i=parseStack.back();
  char s[60];
  char arrow[100];
  //  int flag=1;
  
  if (token==EOL_ID) //end of line
    {
      lineNumber++;
      if (stackDump)
	cout<<"Eating end of line\n";
      return 1;
    }
  if (isAction(i))
    {
      parseStack.pop_back();
      doAction(i);
      if (stackDump)
	{
	  getTokenString(s,i);
	  cout<<"Popping "<<s<<" off the stack\n";
	  dumpStack();
	}
      return 0;
    }
  else
  if (isTerm(i)) //terminal symbol
    {
      if (i==token)  //match token
	{
	  parseStack.pop_back();
	  if (stackDump)
	    {
	      getTokenString(s,i);
	      cout<<"Popping "<<s<<" off the stack\n";
	      dumpStack();
	    }
	  return 1;
	}
      else           //error
	{
	  i=parseStack.back();
	  getTokenString(s,i);
	  cout<<lineNumber<<" - Error: expected "<<s<<" found ";
	  getTokenString(s,token);
	  cout<<s<<endl;
	  dumpStack();
	  jerror("Error recovery mechanism not yet implemented");
	}
    }
  else 
    {
      if (tTable[i-NONTERM_BASE][token-TOKEN_BASE].num==0)
	{
	  getTokenString(s,i);
	  cout<<lineNumber<<" - Error expected declaration of type "<<s<<endl;
	  dumpStack();
	  jerror("Error recovery mechanism not yet implemented");
	}
      else
	{
	  parseStack.pop_back(); //get rid of nonterm
	  translationEntry e=tTable[i-NONTERM_BASE][token-TOKEN_BASE];

	  //Push new symbols onto the stack, except EPSILON transitions
	  for (int ii=0;ii<e.num;ii++)
	    if (e.data[ii]!=EPSILON) parseStack.push_back(e.data[ii]);

	  if (stackDump)
	    {
	      cout<<"Transforming non-terminal symbol according to rule:";
	      getTokenString(s,i);
	      printf("%20s",s);
	      getTokenString(s,token);
	      sprintf(arrow," -- %s --> \t",s);
	      printf("%25s",arrow);
	      dumpEntry(e);
	      cout<<endl;
	      cout.flush();
	    }
	  return 0;
	}
    }
  return -1; //should never reach this
}

void initStack()
{
  parseStack.push_back(END);
  parseStack.push_back(CLEANUPBLOCK);
  parseStack.push_back(COMMITBLOCK);
  parseStack.push_back(ACCESSBLOCK);
  parseStack.push_back(DIALSBLOCK);
  parseStack.push_back(GLOBAL_DBLOCK);
  parseStack.push_back(INCLUDE_BLOCK);
  parseStack.push_back(HERITAGE);
  parseStack.push_back(DECLARATION);
}

void buildH()
{
  char *b1=tempnam(NULL,NULL);
  char s[255];
  time_t t0=time(NULL);
  if (t0!=(time_t)-1)
    strftime(s,255,"%A, %B %d at %I:%M %p %Z",localtime(&t0));
  else s[0]='\0';

  h1.close();
  h2.close();
  h3.close();
  h4.close();
  
  h1.open(b1,ios::out);

  h1<<"/*"<<endl;
  h1<<"   "<<obName<<".h";
  h1<<"   Generated by the Ballista(tm) Project data object compiler"<<endl;
  h1<<"   Copyright (C) 1998-2001  Carnegie Mellon University"<<endl<<endl;

  h1<<"   This program is free software; you can redistribute it and/or"<<endl;
  h1<<"   modify it under the terms of the GNU General Public License"<<endl;
  h1<<"   as published by the Free Software Foundation; either version 2"<<endl;
  h1<<"   of the License, or (at your option) any later version."<<endl<<endl;

  h1<<"   This program is distributed in the hope that it will be useful,"<<endl;
  h1<<"   but WITHOUT ANY WARRANTY; without even the implied warranty of"<<endl;
  h1<<"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"<<endl;
  h1<<"   GNU General Public License for more details."<<endl<<endl;

  h1<<"   You should have received a copy of the GNU General Public License"<<endl;
  h1<<"   along with this program; if not, write to the Free Software"<<endl;
  h1<<"   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA."<<endl<<endl;

  h1<<"   File generated "<<s<<endl;
  h1<<endl;
  h1<<"TITLE"<<endl;
  h1<<"   "<<obName<<".h"<<endl;
  h1<<"*/"<<endl<<endl;
  h1<<"//include control"<<endl;
  upper(obNamebak);
  h1<<"#ifndef "<<obNamebak<<"_H"<<endl;
  h1<<"#define "<<obNamebak<<"_H"<<endl;
  h1<<"#include <errno.h>"<<endl;
  h1<<"#include <iostream>"<<endl;
  h1<<"#include <stdio.h>"<<endl;
  h1<<"#include <stdlib.h>"<<endl;
  h1<<"#include <fstream>"<<endl;
  h1<<"#include <string.h>"<<endl;
  h1<<"#include <sys/types.h>"<<endl;
  h1<<"#include <sys/stat.h>"<<endl;
  h1<<"#include <unistd.h>"<<endl;
  h1<<"#include "<<(char)34<<"bTypes.h"<<(char)34<<endl;
  h1<<"using namespace std;"<<endl;
  h1.close();

  sprintf(s,"mv %s ./%s.h",b1,obName);
  system(s);
  sprintf(s,"cat %s>> ./%s.h",h2name,obName);
  system(s);
  sprintf(s,"cat %s>> ./%s.h",h1name,obName);
  system(s);
  sprintf(s,"rm %s",h2name);
  system(s);
  sprintf(s,"rm %s",h1name);
  system(s);

  b1=tempnam(NULL,NULL);
  h1.open(b1,ios::out);
  h1<<endl<<endl;
  h1<<"class CLASSNAME:public CLASSPARENT"<<endl;
  h1<<"{"<<endl;
  h1<<"private:"<<endl;
  h1<<"  //CLASS DIAL SETTING STRING VARIABLES"<<endl;
  
  h1<<"  b_param _"<<obName<<"TYPENAME;"<<endl;
  h1.close();
  
  sprintf(s,"cat %s>> ./%s.h",b1,obName);
  system(s);
  sprintf(s,"rm %s",b1);
  system(s);

  sprintf(s,"cat %s>> ./%s.h",h3name,obName);
  system(s);
  sprintf(s,"rm %s",h3name);
  system(s);

  b1=tempnam(NULL,NULL);
  h1.open(b1,ios::out);

  h1<<"  //TYPE VARIABLE TO SAVE VALUE FOR DESTRUCTION"<<endl;
  h1<<"  CLASSTYPE _theVariable;"<<endl;
  h1<<endl;
  h1<<" public:"<<endl;
  h1<<"  //CLASS DIAL SETTING STRING ACCESS METHODS"<<endl;
  h1.close();

  sprintf(s,"cat %s>> ./%s.h",b1,obName);
  system(s);
  sprintf(s,"rm %s",b1);
  system(s);

  sprintf(s,"cat %s>> ./%s.h",h4name,obName);
  system(s);
  sprintf(s,"rm %s",h4name);
  system(s);
 
  b1=tempnam(NULL,NULL);
  h1.open(b1,ios::out);
  h1<<"  //CLASS CONSTRUCTOR"<<endl;
  h1<<"  CLASSNAME();"<<endl;
  h1<<endl;
  h1<<"public:"<<endl;
  h1<<"  //Mandatory Methods"<<endl;
  h1<<"  b_param *typeName();           //returns the type of parameter"<<endl;
  h1<<"  virtual void *access(b_param data[]);"<<endl;
  h1<<"  virtual int commit(b_param tname);"<<endl;
  h1<<"  virtual int cleanup(b_param tname);"<<endl;
  h1<<"  "<<endl;
  h1<<"  virtual int numDials(b_param tname);"<<endl;
  h1<<"  virtual int numItems(b_param tname,int dialNumber);"<<endl;
  h1<<"  virtual b_param *paramName(b_param tname,int dialNumber, int position);"<<endl;
  h1<<endl; 
  h1<<"  virtual int distanceFromBase();"<<endl;
  h1<<"  virtual void typeList(b_param list[],int num);"<<endl;
  h1<<"  "<<endl;
  h1<<"};"<<endl;
  h1<<endl;
  h1<<endl;
  h1<<endl;
  h1<<"#endif      //CLASSNAME_H"<<endl;

  h1.close();

  sprintf(s,"cat %s>> ./%s.h",b1,obName);
  system(s);
  sprintf(s,"rm %s",b1);
  system(s);
  unlink(h1name);

  unlink(h2name);

  unlink(h3name);

  unlink(h4name);

}

void buildCPP()
{
  char s[255];
  time_t t0=time(NULL);
  if (t0!=(time_t)-1)
    strftime(s,255,"%A, %B %d at %I:%M %p %Z",localtime(&t0));
  else s[0]='\0';

  c1.close();
  c2.close();
  c3.close();
  c4.close();
  c5.close();
  c6.close();
  c7.close();
  c8.close();
  c9.close();
  c10.close();
  c11.close();
  c12.close();
  
  cf<<"/*"<<endl;
  cf<<"   "<<obName<<".cpp";
  cf<<"   Generated by the Ballista(tm) Project data object compiler"<<endl;
  cf<<"   Copyright (C) 1998-2001  Carnegie Mellon University"<<endl<<endl;
  
  cf<<"   This program is free software; you can redistribute it and/or"<<endl;
  cf<<"   modify it under the terms of the GNU General Public License"<<endl;   
  cf<<"   as published by the Free Software Foundation; either version 2"<<endl;
  cf<<"   of the License, or (at your option) any later version."<<endl<<endl;
  
  cf<<"   This program is distributed in the hope that it will be useful,"<<endl;
  cf<<"   but WITHOUT ANY WARRANTY; without even the implied warranty of"<<endl;
  cf<<"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"<<endl;
  cf<<"   GNU General Public License for more details."<<endl<<endl;

  cf<<"   You should have received a copy of the GNU General Public License"<<endl;
  cf<<"   along with this program; if not, write to the Free Software"<<endl;
  cf<<"   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA."<<endl<<endl;

  cf<<"   File generated "<<s<<endl;
  cf<<endl;
  cf<<"TITLE"<<endl;
  cf<<"   "<<obName<<".cpp"<<endl;
  cf<<"*/"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl<<endl;
  cf<<"#include <errno.h>"<<endl;
  cf<<"#include <iostream>"<<endl;
  cf<<"#include <stdio.h>"<<endl;
  cf<<"#include <stdlib.h>"<<endl;
  cf<<"#include <fstream>"<<endl;
  cf<<"#include <string.h>"<<endl;
  cf<<"#include <sys/types.h>"<<endl;
  cf<<"#include <sys/stat.h>"<<endl;
  cf<<"#include <unistd.h>"<<endl;
  cf<<endl<<"#include "<<qt<<obName<<".h"<<qt<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl<<endl;
  cf.close();

  //start cpp file with header
  sprintf(s,"cat %s >%s.cpp",cfname,obName);
  system(s);
  //start cpp file with header
  sprintf(s,"cat %s >>%s.cpp",c12name,obName);
  system(s);

  cf.open(cfname,ios::out);
  cf<<"//--------------------------------------------------------------------"
    <<endl<<endl;
  cf<<"CLASSNAME::CLASSNAME()"<<endl;
  cf<<"{"<<endl;
  cf<<"  //DIAL DECLARATIONS HERE"<<endl;
  cf<<""<<endl;
  cf<<"  //generated"<<endl;
  cf<<""<<endl;
  cf<<"   strcpy(_"<<obName<<"TYPENAME,CLASS_STRING);"<<endl;
  cf.close();

  //add c1
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c1name,obName);
  system(s);

  cf.open(cfname,ios::out);
  cf<<"}"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl;

  cf.close();
  //add c2
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c2name,obName);
  system(s);

  cf.open(cfname,ios::out);
  //c2 is a block of methods, does not need closing brace
  //  cf<<"}"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"//type name return method"<<endl;
  cf<<""<<endl;
  cf<<"b_param *CLASSNAME::typeName()"<<endl;
  cf<<"{"<<endl;
  cf<<"  return &_"<<obName<<"TYPENAME;"<<endl;
  cf<<"}"<<endl;
  cf<<""<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"int CLASSNAME::distanceFromBase()"<<endl;
  cf<<"{"<<endl;
  cf<<"  return CLASSPARENT::distanceFromBase() +1;"<<endl;
  cf<<"}"<<endl;
  cf<<""<<endl;
  cf<<""<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"void CLASSNAME::typeList(b_param list[], int num)"<<endl;
  cf<<"{"<<endl;
  cf<<"  strcpy(list[num],(char *) typeName());"<<endl;
  cf<<"  CLASSPARENT::typeList(list,num+1);"<<endl;
  cf<<"}"<<endl;
  cf<<"//--------------------------------------------------------------------"<<endl;
  cf<<"  void *CLASSNAME::access(b_param data[])"<<endl;
  cf<<"{"<<endl;
  cf<<"  if (strcmp(data[0],(char *)typeName())!=0)"<<endl;
  cf<<"    return CLASSPARENT::access(data);"<<endl;
  cf<<""<<endl;
  cf<<"  //ACCESS CODE"<<endl;
  cf.close();

  //add c4, c5, c7
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c4name,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c5name,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c7name,obName);
  system(s);

  cf.open(cfname,ios::out);
  cf<<"   return &_theVariable;"<<endl;
  cf<<"}"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl;
  
  cf<<"  int CLASSNAME::commit(b_param tname)"<<endl;
  cf<<"{"<<endl;
  cf<<"  if (strcmp(tname,(char *)typeName())!=0)"<<endl;
  cf<<"    return CLASSPARENT::commit(tname);"<<endl;
  cf<<"  //COMMIT CODE HERE"<<endl;
  cf<<"//generated"<<endl;
  cf.close();

  //add c8
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c8name,obName);
  system(s);
  cf.open(cfname,ios::out);
  cf<<"   return 0;"<<endl;
  cf<<"}"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl;

  cf<<"  int CLASSNAME::cleanup(b_param tname)"<<endl;
  cf<<"{"<<endl;
  cf<<"  if (strcmp(tname,(char *)typeName())!=0)"<<endl;
  cf<<"    return CLASSPARENT::cleanup(tname);"<<endl;
  cf<<"      "<<endl;
  cf<<"  //CLEANUP CODE"<<endl;
  cf<<"//generated"<<endl;
  cf.close();

  //add c9
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c9name,obName);
  system(s);
  cf.open(cfname,ios::out);
  cf<<"   return 0;"<<endl;
  cf<<"}"<<endl<<endl;
  cf<<"//--------------------------------------------------------------------"
    <<endl;
  cf<<"int CLASSNAME::numDials(b_param tname)"<<endl;
  cf<<"{"<<endl;
  cf<<"  if (!strcmp(tname,(char *)typeName()))"<<endl;
  cf<<"    return NUMBER_OF_DIALS;"<<endl;
  cf<<"  else return CLASSPARENT::numDials(tname);"<<endl;
  cf<<"      "<<endl;
  cf<<"}"<<endl;
  cf<<""<<endl;
  cf<<""<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"int CLASSNAME::numItems(b_param tname,int dialNumber)"<<endl;
  cf<<"{"<<endl;
  cf<<"  if (strcmp(tname,(char *)typeName())!=0)"<<endl;
  cf<<"    return CLASSPARENT::numItems(tname,dialNumber);"<<endl;
  cf<<"  switch (dialNumber)"<<endl;
  cf<<"    {"<<endl;
  cf<<"      //NUMITEMS SWITCH CASES HERE"<<endl;
  cf<<"      //generated"<<endl;
  
  cf.close();

  //add c10
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c10name,obName);
  system(s);
  cf.open(cfname,ios::out);
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf<<"      //end generated"<<endl;
  cf<<""<<endl;
  cf<<"    default:"<<endl;
  cf<<"      cerr<<"<<qt<<"Error, invalid dial number passed to "<<qt<<endl;
  cf<<"          <<CLASS_STRING<<"<<qt<<"::numItems\\n"<<qt<<endl;
  cf<<"          <<"<<qt<<"Please check declaration files.  Dial number passed was "<<qt<<endl;
  cf<<"          <<dialNumber<<endl;"<<endl;
  cf<<"      exit(1);"<<endl;
  cf<<"    }"<<endl;
  cf<<"  return 0;"<<endl;
  cf<<"}"<<endl<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;

  cf<<"b_param *CLASSNAME::paramName(b_param tname,"<<endl;
  cf<<"                                   int dialNumber,"<<endl; 
  cf<<"                                   int position)"<<endl;
  cf<<""<<endl;
  cf<<"{"<<endl;
  cf<<"  if (strcmp(tname,(char *)typeName())!=0)"<<endl;
  cf<<"    return CLASSPARENT::paramName(tname,dialNumber,position);"<<endl;
  cf<<"  "<<endl;
  cf<<"  switch (dialNumber)"<<endl;
  cf<<"    {"<<endl;
  cf<<"      //PARAMNAME SWITCH CASES HERE"<<endl;
  cf<<"      //generated"<<endl;

  cf.close();

  //add c11
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  sprintf(s,"cat %s >>%s.cpp",c11name,obName);
  system(s);
  cf.open(cfname,ios::out);
  cf<<"    default:"<<endl;
  cf<<"      cerr<<"<<qt<<"Error, invalid dial number passed to "<<qt<<endl;
  cf<<"          <<CLASS_STRING<<"<<qt<<"::paramName\\n"<<qt<<endl;
  cf<<"          <<"<<qt<<"Please check declaration files.  Dial number passed was "<<qt<<endl;
  cf<<"          <<dialNumber<<endl;"<<endl;
  cf<<"      exit(1);"<<endl;
  cf<<""<<endl;
  cf<<"    }"<<endl;
  cf<<"  return NULL;"<<endl;
  cf<<"}"<<endl;
  cf<<""<<endl;
  cf<<"//---------------------------------------------------------------------------"<<endl;
  cf.close();

  //add final
  sprintf(s,"cat %s >>%s.cpp",cfname,obName);
  system(s);
  unlink(c1name);
  unlink(c2name);
  unlink(c3name);
  unlink(c4name);
  unlink(c5name);
  unlink(c6name);
  unlink(c7name);
  unlink(c8name);
  unlink(c9name);
  unlink(c10name);
  unlink(c11name);
  unlink(c12name);
  unlink(cfname);
}

int main(int argc,char *argv[])
{
  int cur;
  int lcv=0;
  char s[50];

  initTable();
  initStack();
  parseCLine(argc,argv);
  if (dumpFlag==1)
    {
      cout<<"Dumping translation table...\n";
      dumpTable();
      dumpStack();
      exit(0);
    }
  if (helpFlag==1)
    {
      cout<<"Ballista parser pre-pre alpha\n"
	  <<"options\n"
	  <<"-h      Prints this message\n"
	  <<"-dump   Dumps the parse table and exits\n"
	  <<"-s      Provides diagnostic stack dumps\n"
	  <<"-a      Verbose actions\n";
      exit(0);
      
    }

  cin>>s;
  for (int ii=0;ii<(int)strlen(s);ii++)
    if (!isdigit(s[ii]))
      jerror("Error- invalid data in token stream");
  if(!(strlen(s)>0))
    jerror("Error - Empty token stream");
  cur=atoi(s);
  
  while (!cin.eof())
    {
      if (cur==VAR_ID || cur==CODE_ID) cin>>lexeme;
      lcv=0;
      while (lcv==0) lcv=match(cur);

      cin>>s;
      for (int ii=0;ii<(int)strlen(s);ii++)
	if (!isdigit(s[ii]))
	  jerror("Error- invalid data in token stream");
      if(!(strlen(s)>0) &&parseStack[0]!=END)
	jerror("Error - Empty token stream");
      cur=atoi(s);

    }
  if (parseStack.size()>1) //more than just the *END* token
  	{	
  		dumpStack();
  		jerror("Error - Premature end of file.  Bailing out\n");
  	}
  buildH();
  buildCPP();
  return 0;
}
