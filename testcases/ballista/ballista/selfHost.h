/* selfHost.h:  Header file for definition of Selfhost_test_manager class
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

#ifndef _B_SELFHOST_H
#define _B_SELFHOST_H

#include "ballista.h"
#include "ballistaError.h"
#include "testManager.h"


// Constant for maximum length of the marshaled dial settings
// Enough space to hold all the the b_param fields
// PLUS a single character seperator between the dials in each list
// PLUS a single character terminator for each list
#define MAX_MARSHAL_STRING ((MAXP * MAXD) * sizeof(b_param)+(MAXD -1) * MAXP + MAXP)

//Return values of manage_test method
#define READING_PIPE_FAILED -7
#define MALLOC_FAILED -6
#define PIPE_FAILED -5
#define UNDEFINED_RESULT -4
#define WAITPID_FAILED -3
#define MARSHAL_STRING_FAILED -2
#define FORK_FAILED -1
#define RETURN_OK 0

/************************
 *
 * Function:        class Selfhost_test_manager
 * Description:     Class definition for self-host-specific Test_manager
 *                  implementation
 * Function Inputs: N/A
 * Global Inputs:   N/A
 * Return Values:   N/A
 * Global Outputs:  N/A
 * Errors:          Throws exceptions if test execution file cannot be
 *                  dynamically loaded
 * Pre-Conditions:  test execution file exists
 * Post-Contidions: N/A
 * Design:          Based on original Ballista functionality, 
 *                  and implementation of VxWorks-specific Test_manager
 * Notes:           N/A
 *
 ************************/

class Selfhost_test_manager : public Test_manager {

 public:

  // Constructor
  Selfhost_test_manager (const char *mut_filename)
    throw (Ballistic_error);

  //Destructor
  virtual ~Selfhost_test_manager (void);
  
  virtual int manage_test(MARSHAL_DATA_TYPE arguments,
			  long timeout_value,
			  char *pass_status, int max_pass_status,
			  int *call_ret,
			  char *sys_err, int max_sys_err,
			  char *mut_return, int max_mut_return);
  
 private:
  // Private member variables
  
  // Variable to hold process id number of child for test
  // 0 for no child, positive integer if child exists
  pid_t pid;
  //Function pointer for the execute_test_case function (to be dynamically
  //loaded in)
  int (*test_exec_ptr) (char *);
};

#endif // _B_SELFHOST_H
