%{
  /* ballista.l : Ballista lexer - Compiler
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
#include "blexer.h"
#include "butil.h"

#include "tokens.h"

int inCode = 0;  /* if zero, not in user c-code somewhere*/
int lineNum = 0;
int helpFlag =0;
int dumpFlag =0;
int commentState=0; /* track if we are in a comment */


%}

%%

[\t ]+ {if (inCode!=0) return CODE_SP;}   /*ignore whitespace*/


\n {lineNum++;commentState=0;if (inCode!=0) return CODE_NL;return EOL_ID;}

name {if (inCode!=0) return CODE_ID;return NAME_ID;}
parent {if (inCode!=0) return CODE_ID;return PARENT_ID;}
includes {if (inCode!=0) return CODE_ID;return INCLUDES_ID;}
global_defines {if (inCode!=0) return CODE_ID;return GLOBAL_DEFINES_ID;}
dials {if (inCode!=0) return CODE_ID;return DIALS_ID;}
enum_dial {if (inCode!=0) return CODE_ID;return ENUM_DIAL_ID;}
int_dial {if (inCode!=0) return CODE_ID;return INT_DIAL_ID;}
access {if (inCode!=0) return CODE_ID;return  ACCESS_ID;}
commit {if (inCode!=0) return CODE_ID;return COMMIT_ID;}
cleanup {if (inCode!=0) return CODE_ID;return CLEANUP_ID;}
\/\/.* { commentState=1;}
\[ {if (inCode!=0) return CODE_ID;return LEFT_BRACKET_ID;}
\] {if (inCode!=0) return CODE_ID;return RIGHT_BRACKET_ID;}
\{ {inCode++;if (inCode!=1) return CODE_ID;return  LEFT_BRACE_ID;}
\} {
    inCode --; 
    if (inCode <0)
      {
	char s[50];
	sprintf(s,"Error, mismatched } - line number %i",lineNum);
	jerror(s);
      }
    if (inCode!=0) return CODE_ID;return RIGHT_BRACE_ID;
   }
: {if (inCode!=0) return CODE_ID;return COLON_ID;}
; {if (inCode!=0) return CODE_ID;return SEMI_ID;}
, {if (inCode!=0) return CODE_ID;return COMMA_ID;}
[a-zA-Z0-9_*:\-\<\>]+ {
  if (inCode!=0)
    return CODE_ID;
  else
    {
      symLook(yytext);
      return VAR_ID;
    }
}
. {if (inCode!=0) return CODE_ID;return CHAR_ID;}

%%
void parseCLine(int argc,char *argv[])
{
  int i;
  for (i=2;i<=argc;i++)
    {
      if (!strcmp(argv[i-1],"-h")) helpFlag=1;
      if (!strcmp(argv[i-1],"-dump")) dumpFlag=1;
    }
}

int main(int argc,char *argv[])
{
  int i;

  parseCLine(argc,argv);

  if (helpFlag)
  {
    printf("Ballista Lexical Analyzer v0.1\n");
    printf("options\n");
    printf("-h       Print this message\n");
    printf("-dump    Verbose output mode");
  }

  i=yylex();
  while (i!=0)
    {
      if (dumpFlag) printf("(%i)%i: %s\n",inCode,i,yytext);
      else if (i==VAR_ID || i==CODE_ID) printf("%i\t%s\n",i,yytext);
      else printf("%i\n",i);
      i=yylex();
    }

  if (dumpFlag)
  {
   printf("\nDumping Symbol Table\n");
   dumpTable();
  }

  freeTable();
  return 0;
}
