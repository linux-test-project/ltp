// b_ptr_void.tpl : Ballista Datatype Template for a void pointer
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

name void* b_ptr_void;

parent paramAccess;

includes
[
 {
  #include "values.h"
  #include "bTypes.h"
 }
]

global_defines
[
 {
  #define MaxBytes sizeof(void *);
  static void *ptrRef = NULL;
  static void *cp_ptrRef = NULL;
  //void *saveloc;
 }
]

dials
[
  enum_dial POSITION : NULL, NEGONE, MAXINT, STATIC, DYNAMIC, CONST, FREED;
]

access
[
 {
  char stat[56];
 }

 NULL
    {
       _theVariable=(void *) NULL;
    }
 NEGONE
    {//Invalid pointer 
       _theVariable=(void *) -1;
    }
 MAXINT
    {
       _theVariable=(void *)MAXINT;
    }
 STATIC
    {
       _theVariable=(void*)&stat;
    }
 DYNAMIC
    {
       cp_ptrRef = ptrRef = malloc(16);
       _theVariable = (void *) ptrRef;
    }
 CONST
    {
       ptrRef =  (void*)"                                             ";
       _theVariable = ptrRef;
    }
 FREED
    {
       cp_ptrRef = ptrRef = malloc (10240); //allocate 10k block
       _theVariable = (void *) ptrRef;
    }
]

commit
[
 FREED
    {
       free(ptrRef);
    }
]

cleanup
[
 DYNAMIC
    {
       if (cp_ptrRef)
       {
          free(cp_ptrRef);
          ptrRef = NULL;
       }
    }
]
