/* vxworks.h: Implementation of Vxworks_test_manager class
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

#ifndef _B_VXWORKS_H
#define _B_VXWORKS_H

#include "ballista.h"
#include "testManager.h"


// Constant definitions for maximum string lengths of member variables
#define MAX_TARGET_FILE_NAME 257
#define MAX_TARGET_NAME      257
#define MAX_IP_ADDRESS       257

// Constant definitons for retrun values from windsh method
#define WINDSH_RETURN_OK       0
#define WINDSH_RETURN_ABORT    1

// Constant for maximum length of the marshaled dial settings
// Enough space to hold all the the b_param fields
// PLUS a single character seperator between the dials in each list
// PLUS a single character terminator for each list
#define MAX_MARSHAL_STRING ((MAXP * MAXD) * sizeof(b_param)+(MAXD -1) * MAXP + MAXP)

// Maximum length of a command string passed to windsh script 
#define MAX_COMMAND_STRING (MAX_MARSHAL_STRING + 257)

// Serial port for standard output from target
#define TARGET_OUTPUT_SERIAL_PORT "/dev/cua/a"

// Serial port for accessing the reboot device
#define REBOOT_DEVICE_SERIAL_PORT "/dev/cua/b"

// Time to sleep in reboot function before reestablishing a connection
#define VXWORKS_REBOOT_TIME 20

// Timeout value in seconds for issuing a ping command
#define PING_TIMEOUT_VAL 5

// Input filename for windsh script commands
#define WINDSH_INPUT_FILE_NAME "windsh_input"

// Output filename for windsh script commands
#define WINDSH_OUTPUT_FILE_NAME "windsh_output"

// Regular expressions commonly used to compare output in the VxWorks test managers
#define VALUE "value = [0-9]+ = 0x[0-9A-Fa-f]+\n"
#define VALUE_QUIT "value = [0-9]+ = 0x[0-9A-Fa-f]+( = .*)?\nquit\n$"



/************************
 *
 * Function:        class Vxworks_test_manager
 * Description:     Class definition for VxWorks specific Test_manager
 *                  implementation
 * Function Inputs: N/A
 * Global Inputs:   N/A
 * Return Values:   N/A
 * Global Outputs:  N/A
 * Errors:          N/A
 * Pre-Conditions:  N/A
 * Post-Contidions: N/A
 * Design:          N/A
 * Notes:           N/A
 *
 ************************/

class Vxworks_test_manager : public Test_manager {

 public:

  // Constructor
  Vxworks_test_manager(const char *file_name, const char *target, 
		       const char *ip_address, const int max_number_reboots,
		       const int do_reboot_after_each_test);

  //Destructor
  virtual ~Vxworks_test_manager();
  
  virtual int manage_test(MARSHAL_DATA_TYPE arguments,
			  long timeout_value,
			  char *pass_status, int max_pass_status,
			  int *call_ret,
			  char *sys_err, int max_sys_err,
			  char *mut_return, int max_mut_return);
  
 private:
  // Private member variables
  
  // Path and file name of object file to be loaded onto the
  // VxWorks target
  char target_file_name[MAX_TARGET_FILE_NAME];
  
  // Target server name to be passed with windsh script command
  char target_name[MAX_TARGET_NAME];
  
  // IP address or network name of target machine
  char target_ip_address[MAX_IP_ADDRESS];
  
  // Maximum number of reboots allowed for the target
  int target_max_number_reboots;
  
  // Number of times the target has been rebooted for
  // this instantiation
  int number_reboots;
  
  // This variable is set when the diagnose functionis completed
  // successfully before the first test
  int passed_diagnose;
  
  // This flag tells whether or not the target will be rebooted
  // after running each test case
  int reboot_after_each_test;
  
  // These variables contain the file descriptors for stdin, stdout, 
  // and stderr on Vxworks target
  int fd_stdin;
  int fd_stdout;
  int fd_stderr;
  
  //Flag to keep state of whether we have already redirected output
  BOOL_TYPE redirected_flag;
  
  // Variable to hold process id number of child for test
  int pid;
  
  //Flag to keep state of whether we have child processes to clean up
  BOOL_TYPE child_exists_flag;
  
  BOOL_TYPE need_to_unload;
  
 protected:
  // Protected member methods
  
  int diagnose(char *error_message, int max_error_message_len);
  
  int ping(int time_out);
  
  int reboot(void);
  
  int redirect();
  
  int unredirect();
  
  int windsh(char *command, char* reg_expression);
  
  int get_last_rval(char *value, int max_value_length);

  int get_last_identifier_value(char *identifier, char *value,
				int max_value_length);
  
  int get_last_windsh_result(int *last_result);
  
  int load(void);
  
  int unload(void);
  
  int create_script_command(MARSHAL_DATA_TYPE arguments,
			    char *command);
  
};

#endif // _B_VXWORKS_H
