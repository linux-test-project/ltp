/* tokens.h : Ballista header file - Compiler
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
/*
#define NAME_ID 1001
#define PARENT_ID 1002
#define INCLUDES_ID 1003
#define GLOBAL_DEFINES_ID 1004
#define DIALS_ID 1005
#define ENUM_DIAL_ID 1006
#define INT_DIAL_ID 1007
#define ACCESS_ID 1008
#define STRING_ID 1009
#define VAR_ID 1010
#define CODE_ID 1011
#define COMMIT_ID 1012
#define CLEANUP_ID 1013
#define LEFT_BRACE_ID 1014
#define RIGHT_BRACE_ID 1015
#define LEFT_BRACKET_ID 1016
#define RIGHT_BRACKET_ID 1017
#define EOL_ID 1018
#define CHAR_ID 1019
#define COLON_ID 1020
#define COMMA_ID 1021
#define SEMI_ID 1022
*/
/* no not change the first or last item */

#define NUM_TOKENS 27
#define TOKEN_BASE 1000
enum tokenid{
ACCESS_ID=TOKEN_BASE,
CHAR_ID,
CLEANUP_ID,
CODE_ID,
CODE_SP,
CODE_NL,
COLON_ID,
COMMA_ID,
COMMIT_ID,
DIALS_ID,
ENUM_DIAL_ID,
EOL_ID,
END,
GLOBAL_DEFINES_ID,
INCLUDES_ID,
INT_DIAL_ID,
LEFT_BRACE_ID,
LEFT_BRACKET_ID,
NAME_ID,
PARENT_ID,
RIGHT_BRACE_ID,
RIGHT_BRACKET_ID,
SEMI_ID,
STRING_ID,
VAR_ID,
EPSILON,
SYNCH
};

#define NUM_NONTERMS 18
#define NONTERM_BASE 2000
enum nonterms{
DECLARATION=NONTERM_BASE,
HERITAGE,
INCLUDE_BLOCK,
GLOBAL_DBLOCK,
DIALSBLOCK,
ACCESSBLOCK,
COMMITBLOCK,
CLEANUPBLOCK,
DIAL_LIST,
ENUM_DECL,
INT_DECL,
BLOCK_LIST,
VARBLOCK,
VAREST,
CBLOCK,
CODELIST,
SET_LIST,
SETREST
};

#define NUM_ACTIONS 24
#define ACTION_BASE 3000
enum actions{
NAME_DEFINES=ACTION_BASE,
TYPE_DEFINES,
HERITAGE_DEFINES,
SET_H2,
SET_H3,
OUTPUT_CODE,
OUTPUT_CODE_SP,
OUTPUT_CODE_NL,
SET_NULL,
INC_DIAL,
OUTPUT_BPARAM,
OUTPUT_DNUM,
OUTPUT_INTPARAM,
GET_DNAME,
FINISH_ENUM,
GENERATE_CONDITIONAL,
CLOSE_CONDITIONAL,
CLOSE_BLOCK,
RESET_CNUM,
SET_C7,
SET_C8,
SET_C9,
SET_C10,
SET_C12
};

void getTokenString(char *s,int i);

int isNonTerm(int i);
int isTerm(int i);

