// bTypes.h : Ballista Template Base Definition Header
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
   bTypes.h

CLASSES
   paramAccess

ABSTRACT
   This file provides data types common to ballista modules.
   paramAccess is the root data type for the entire ballista 
   object hierarchy.


REVISION RECORD
Date            Engineer        Change
----            --------        ------
18 NOV 97       J DeVale        Original release

1. Provides general definitions for ballista data types

2. Many methods describe here take a typeName parameter.  This is to allow
   the objects to know which object in the inheritance tree is being 
   addressed.
 
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME
  
   paramAccess();

DESCRIPTION
   
   This is the object constructor.  It initializes the type name varible 
   _rootName to be ROOT_PARAMETER.

PARAMETERS

   None.

REQUIREMENTS

   None.

RETURNED VALUE

   None.

NOTES

   Derived classes will initialize string equivalent variables for dial 
   settings here, as well as any other type-specific initialization.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME
   virtual void *paramAccess::access(b_param data[])

DESCRIPTION

   This is the function which accepts dial settings, and returns a pointer to
   the variable type it represents.  In the case of the root class, it
   aborts program execution, and returns an error message.

PARAMETERS

   b_param        *in*           data[]   - Type name in 0, followed by 
                                            dial setting strings

REQUIREMENTS

   Valid type name in position 0 of the array.
   All required dial setting strings for the type in the subsequent array 
   slots.

RETURNED VALUE

   void pointer to the actual variable being returned.  Note then that if
   the type is itself a pointer, it returns a pointer to the pointer, or 
   as some might term is, a handle to the actual data type.

NOTES

   Every derived class MUST OVERRIDE this method in order to function
   properly.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME
   
   virtual int paramAccess::commit(b_param tname)

DESCRIPTION

   This is "phase two" of the variable creation - or as Phil Koopman 
   euphamisticly says, it is the "mangle phase".  This phase performs any
   actions necsarry to change the variable prior to its use.  Note that in
   order to implement this phase, the data object must keep a copy of the
   pointer to the data type returned.

   This can typically be deallocation of memory, closing a file, etc.


PARAMETERS

   tname          in            Data type name it is to mangle.

REQUIREMENTS

   Name to a type in the objects inheritance tree to be mangled exists 
   as a type in its class inheritance path.

   For each call to commit(), there must be exactly one sucessful call 
   to access() preceeding it, with no other interveneing calls to 
   commit(), or cleanup().

RETURNED VALUE

   0 for success, otherwise and error code.
   This base class returns an error, similar to access.

NOTES

   As with access(), commit() MUST BE OVERRIDDEN by each derived class.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   virtual int paramAccess::cleanup(b_param tname)

DESCRIPTION

   This is the final phase in the life of a generated parameter.  This 
   method is responsible for freeing any resources created during the 
   parameter generation phase.  Note that this means that the "unmangled"
   should be saved for freeing.  Of course, creation mangling and freeing
   are done in the forked process, so theoretically the resources should 
   be freed when the process terminates.

PARAMETERS

  tname       in        Data type name it is to mangle.

REQUIREMENTS

   Name to a type in the objects inheritance tree to be freed exists as
   a type in its class inheritance path.

   For each call to cleanup(), there must be exactly one sucessful call 
   to access() preceeding it, with no other interveneing calls to 
   cleanup().

RETURNED VALUE

   0 for success, otherwise and error code.
   This base class returns an error, similar to access.

NOTES

   As with access(), cleanup() MUST BE OVERRIDDEN by each derived class.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   virtual int paramAccess::numDials(b_param tname)

DESCRIPTION

   This method must return the integer value of the number of dials 
   associated with the data type it represents.

   The base class paramAccess returns 0.

PARAMETERS

  tname       in          Data type name it is to mangle.

REQUIREMENTS

   Name to a type in the objects inheritance tree to be freed exists 
   as a type in its class inheritance path.

RETURNED VALUE

   Integer value representing the number of dials which can be twiddled
   to build different parameters.

NOTES

   As with access(), numDials() MUST BE OVERRIDDEN by each derived class.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   virtual int paramAccess::numItems(b_param tname,int dialNumber)

DESCRIPTION

   This method returns the number of settings a particular dial has.
   The base class always returns 0.

PARAMETERS

  tname          in            Data type name it is to mangle.
  dialNumber     in            The number of the dial in question.

REQUIREMENTS

   Name to a type in the objects inheritance tree to be freed exists 
   as a type in its class inheritance path.

   The dial indicated exists for the specified data type.

RETURNED VALUE

   Integer value of the number of dial settings for the specified dial.

NOTES

   As with access(), numItems() MUST BE OVERRIDDEN by each derived class.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   b_param *paramAccess::typeName()

DESCRIPTION

   This method returns the name of the type the object is representing.

PARAMETERS

   None.

REQUIREMENTS

   None.

RETURNED VALUE

   The string value name of the type represented by the object.

NOTES

   Must be implemented by each derived class.

   This is *NOT* a virtual function.  THis is so that the typeName() 
   for the scope currently in execution will be dispatched.  Otherwise, 
   when calling inherited virtual methods that check typenames, the 
   typename string of the class actually instantiated would be called.  
   That would be bad, bad bad.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   int paramAccess::distanceFromBase()

DESCRIPTION

   This method returns the integer value representing the instantiated 
   class' distance from the root class paramAccess.

   Obviously, this being the root class, it returns 0.

   In derived classes this method functions using a recursive descent 
   into the class hierarchy.  In practice, if simply returns the value 
   of its immediate parent class' distanceFromBase() method +1.

PARAMETERS

   None.

REQUIREMENTS

   None.

RETURNED VALUE

   Integer value distance from base class paramAccess.  in this case 0;

NOTES

   As with access(), numDials() MUST BE OVERRIDDEN by each derived class.
     
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

   void paramAccess::typeList(b_param list[], int num)

DESCRIPTION

   This method returns an array with the typename of it, and each of 
   its parent classes.  

   The num parameter is used to tell the method which array slot to place
   the typename in.  The first outside call to this method should be called
   with num = 0, thus the name of the instantiated class' data type is in
   index 0, and the root classes type name is in list[distanceFromRoot()-1].

   The caller is responsible for creating an appropriately sized array, 
   and is most easily done as follows:

   b_param *theList= new b_param[distanceFromRoot()];

PARAMETERS

   list     in/out      The array in which to store typenames
   num      in          The index in list at which to store the typename


REQUIREMENTS

   The list parameter must have be an array of sufficient size to store
   all required data.

RETURNED VALUE

   None.

NOTES

   Must be implemented by each derived class.

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

NAME

DESCRIPTION

PARAMETERS

REQUIREMENTS

RETURNED VALUE

NOTES

*/

#ifndef BTYPES_H
#define BTYPES_H 

#ifdef SUN
extern char *sys_errlist[ ];    //bug in aix and sunos errno.h ( at least )
#endif 

#define B_PARAM_LENGTH 255

typedef char b_param[B_PARAM_LENGTH];

#define MAXP 16            //arbitrary parameter max
#define MAXD 32            //arbitrary max number of dials

class paramAccess
{

public:

  b_param _rootName;   //identifies the type associated with the object.
  
  
  paramAccess();
  
  virtual void *access(b_param data[]);
  virtual int commit(b_param tname);
  virtual int cleanup(b_param tname);
  virtual int numDials(b_param tname);
  virtual int numItems(b_param tname,int dialNumber);
  virtual int distanceFromBase();
  virtual b_param *paramName(b_param tname,int dialNumber, int position);
  b_param *typeName();
  virtual void typeList(b_param list[],int num);
};

#endif   //BTYPES_H
