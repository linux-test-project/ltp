/* vxworks.cpp: Implementation of Vxworks_test_manager class
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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <assert.h>

#include "ballista.h"
#include "ballistaUtil.h"
#include "marshal.h"
#include "vxworks.h"
#include "match.h"  // required for the match_file() function


/************************
 *
 * Function:        Vxworks_test_manager
 * Description:     Constructor for class Vxworks_test_manager
 * Function Inputs: char *file_name,
 *                  char *target,
 *                  char *ip_address,
 *                  int max_number_reboots,
 *                  int do_reboot_after_each_test
 * Global Inputs:   
 * Return Values:   
 * Global Outputs:  
 * Errors:          
 * Pre-Conditions:  
 * Post-Contidions: All corresponding member variables are set
 * Design:
 * Notes:           
 *
 ************************/

Vxworks_test_manager::Vxworks_test_manager(const char *file_name, 
					   const char *target,
					   const char *ip_address,
					   const int max_number_reboots,
					   const int do_reboot_after_each_test)
{
  // Abort if null pointers
  assert(file_name != NULL);
  assert(target != NULL);
  assert(ip_address != NULL);

  safe_strncpy(target_file_name, file_name, MAX_TARGET_FILE_NAME);
  safe_strncpy(target_name, target, MAX_TARGET_NAME);
  safe_strncpy(target_ip_address, ip_address, MAX_IP_ADDRESS);
  reboot_after_each_test = do_reboot_after_each_test;
  target_max_number_reboots = max_number_reboots;
  number_reboots = 0;
  passed_diagnose = 0;
  redirected_flag = BOOL_FALSE;
  pid = 0;
  child_exists_flag = BOOL_FALSE;
  need_to_unload = BOOL_FALSE;

  if (do_reboot_after_each_test != 0 && max_number_reboots == 0) {
    fprintf(stderr, "Error: In constructor for Vxworks_test_manager\n");
    fprintf(stderr, "Cannot reboot after each test if\n");
    fprintf(stderr, "max_number_reboots is set to zero\n");
    exit(-1);
  }

}

/************************
 *
 * Function:        ~Vxworks_test_manager
 * Description:     Destructor for class Vxworks_test_manager
 * Function Inputs: N/A
 * Global Inputs:   N/A
 * Return Values:   N/A
 * Global Outputs:  N/A
 * Errors:          N/A
 * Pre-Conditions:  Object must have been constructed previously
 * Post-Contidions: Object does not exist; standard in, out, error of
 *                  target will be un-redirected; leftover child process will
 *                  be reaped.
 * Design:          N/A
 * Notes:           Author: Mina Atanacio
 *
 ************************/
Vxworks_test_manager::~Vxworks_test_manager() {
  // Status of forked process, dummy variable
  int status;

  fprintf(stderr, "Cleaning up test manager.\n");


  if (unredirect() != 0) {
    fprintf(stderr, 
	    "Error in test manager: Cannot unredirect target stdin, stdout, or");
    fprintf(stderr, " stderr.\n");
  }
  
  if (child_exists_flag == BOOL_TRUE) {
    // clean up child process
    kill (-pid, SIGTERM);
    waitpid (pid, &status, 0);
  }
  
  if (unload() != 0) {
    fprintf(stderr,"Error: unable to unload mut from target.\n");
  }
  
}

/************************
 *
 * Function:        manage_test
 * Description:     VxWorks specific implementation of the virtual
 *                  function derived from the base class Test_manager
 *                  This function controls all the Ballista testing on the
 *                  target.
 * Function Inputs: MARSHAL_DATA_TYPE arguments,
 *                  long timeout_value,
 *                  char *pass_status,
 *                  int max_pass_status,
 *                  int *call_ret,
 *                  char *sys_err,
 *                  int max_sys_err,
 *                  char *mut_return,
 *                  int max_mut_return
 * Global Inputs:   
 * Return Values:   Integer 0 if function is successful, nonzero integer
 *                  otherwise
 * Global Outputs:  
 * Errors:          
 * Pre-Conditions:  
 * Post-Contidions: One test case will be completed.  All the output variables
 *                  will be set.
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::manage_test(MARSHAL_DATA_TYPE arguments,
				      long timeout_value,
				      char *pass_status,
				      int max_pass_status,
				      int *call_ret,
				      char *sys_err, int max_sys_err,
				      char *mut_return, 
				      int max_mut_return)
{
  // String to hold windsh command for calling execute_test_case
  char execute_mut_command[MAX_COMMAND_STRING];
  // Loop control variable
  int i;
  // Status of forked process
  int status;
  // Variable to hold return value of get_last_windsh_result
  int windsh_return;
  // Variable to hold return value of function to read rval
  int rval_return;
  // Return value of execute_test_case
  int return_value;
  // Return value of waitpid
  int wait_pid_return;
  // Flag for clean up operations
  hrtime_t start_time;
  hrtime_t timeout_nanosec;

  //signal-handling struct
  struct sigaction action;

  // Abort if null pointer
  assert(arguments != NULL);
  assert(pass_status != NULL);
  assert(call_ret != NULL);
  assert(sys_err != NULL);
  assert(mut_return != NULL);

  // Initialize mut_return to be an empty string
  if(max_mut_return) {
    mut_return[0] = 0;
  }
  
  // Perform diagnose function if this is the first test to be run
  // Otherwise don't do it
  if (passed_diagnose != 1) {
    if (diagnose(sys_err, max_sys_err) != 0) {
      return -1;
    } else {
      passed_diagnose = 1;
    }
  }
  
  // Load execute_test_case module onto the target
  if (load() != 0) {
    return -1;
  }
  
  // Create windsh script command string to run test case
  if (create_script_command(arguments, execute_mut_command) != 0) {
    return -1;
  }
  
  // Fork off a process to execute the windsh script command
  if ( (pid=fork()) == 0) {

    // remove signal handlers that were installed
    sigemptyset (&action.sa_mask);
    action.sa_handler = SIG_DFL;
    (void) sigaction (SIGINT, &action, NULL);
    (void) sigaction (SIGQUIT, &action, NULL);
    (void) sigaction (SIGTERM, &action, NULL);
    (void) sigaction (SIGABRT, &action, NULL);
        
    // set up a process group so that children processes can
    //  all be killed off easily
    setpgid(0, 0);
    
    if (windsh(execute_mut_command,".*") != 0) {
      // Not sure what to do here
    }
    exit(0);
  }
  
  //Set flag indicating a child process exists.
  child_exists_flag = BOOL_TRUE;

  // do this in two stages to avoid type casting problems from int to hrtime_t
  timeout_nanosec = timeout_value;
  timeout_nanosec = timeout_nanosec * 1000;

  start_time = gethrtime();
  i = 0;
  wait_pid_return = 0;
  while(start_time + timeout_nanosec > gethrtime())  {
    
    //?????????????????/
    // can wait_pid_return be -1????
    wait_pid_return = waitpid (pid, &status, WNOHANG);
    
    // check if process terminated
    if(wait_pid_return > 0) {
      child_exists_flag = BOOL_FALSE;
      break;
    }
    // sleep for 500 microseconds before checking again
    usleep(500);
    
    // On the second iteration try pinging, in case the machine
    // crashed right away.  This may avoid a lengthy timeout.
    if (i == 1) {
      if (ping(PING_TIMEOUT_VAL) != 0) {
	break;
      }
    }
    i++;
  }
  
  // check that the loop terminated because the process terminated
  if (wait_pid_return > 0) {
    // process has terminated
    // either a Pass or an Abort has occurred
    child_exists_flag = BOOL_FALSE;
    windsh_return = get_last_windsh_result(&return_value);
    rval_return = get_last_rval(mut_return, max_mut_return);
    
    if ( (windsh_return == WINDSH_RETURN_ABORT) ||
	 (rval_return != 0) 
	 ) {
      safe_strncpy(pass_status, "Done - Abort", max_pass_status);
    } else {
      *call_ret = return_value;
      safe_strncpy(pass_status, "Done - Pass", max_pass_status);
    }
  } else if (ping(PING_TIMEOUT_VAL) == 0) { 
    // did not terminate and still alive

    // clean up child processes
    kill (-pid, SIGTERM);
    waitpid (pid, &status, 0);
    child_exists_flag = BOOL_FALSE;
    
    safe_strncpy(pass_status, "Done - Restart", max_pass_status);
  } else {  
    // did not terminate and not alive

    // clean up child processes
    kill (-pid, SIGTERM);
    waitpid (pid, &status, 0);
    child_exists_flag = BOOL_FALSE;
    
    safe_strncpy(pass_status, "Done - Catastrophic",
		 max_pass_status);
    // We must reboot because there is a catastrophic, but we should skip
    // this if we are already rebooting after each test.
    if (!reboot_after_each_test) {
      // Not rebooting after each test, therefore reboot for catastrophic
      if (reboot() != 0) {
	return -2;
      }
    }
  }
  
  // If flag is set, then reboot after each test case
  if (reboot_after_each_test) {
    if (reboot() != 0) {
      return -2;
    }
  }

  // If target was not rebooted we need to unload the execute_test_case module
  if (unload() != 0) {
    safe_strncpy(sys_err,"Error: unable to unload mut from target.",
		   max_sys_err);
    return -1;
  }
  
  return 0;
}


/************************
 *
 * Function:        diagnose
 * Description:     This function will check that testing can proceed.
 * Function Inputs: char *error_message,
 *                  int max_error_message_len
 * Global Inputs:   
 * Return Values:   integer 0 if function is successful, nonzero integer
 *                  otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions: standard out, in, and error on target machine will be
 *                  redirected.
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::diagnose(char *error_message,
				   int max_error_message_len)
{

  //Abort if null pointer
  assert(error_message != NULL);


  if (target_max_number_reboots != 0 && reboot() != 0) {
    safe_strncpy(error_message, "Error: target could not be rebooted",
		 max_error_message_len);
    return -1;
  }

  if (ping(PING_TIMEOUT_VAL) != 0) {
    safe_strncpy(error_message, "Error: target is unreachable",
		 max_error_message_len);
    return -1;
  }
  if (load() != 0) {
    safe_strncpy(error_message,
		 "Error: test module could not be loaded onto target",
		 max_error_message_len);
    return -1;
  }

  if (unload() != 0) {
    safe_strncpy(error_message,
		 "Error: test module could not be unloaded from target",
		 max_error_message_len);
    return -1;
  }

  // Redirect stdin, stdout, and stderr to windsh interpreter
  // to get return value of test MuT
  if(redirect() != 0) {
    safe_strncpy(error_message,
		 "Error: unable to redirect standard output on target.",
		 max_error_message_len);
    return -1;
  }


  return 0;
}


/************************
 *
 * Function:        ping
 * Description:     Ping will launch the UNIX ping() command, passing
 *                  the target_ip_address member variable and the
 *                  timeout value in seconds, and returns
 *                  whether or not the machine is alive.
 * Function Inputs: int time_out
 * Global Inputs:   Vxworks_test_manager class member target_ip_address
 * Return Values:   integer 0 if function is successful and machine is
 *                  reachable, nonzero integer otherwise
 * Global Outputs:  
 * Errors:          
 * Pre-Conditions:  
 * Post-Contidions: 
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::ping(int time_out)
{
  char system_command_string[MAX_COMMAND_STRING];

  // Create command string to make system call ping
  // and execute it
  sprintf(system_command_string, "ping %s %d > /dev/null 2>&1",
	  target_ip_address, time_out);
  if (system(system_command_string) != 0) {
    return -1;
  }

  return 0;
}


/************************
 *
 * Function:        reboot
 * Description:     This function will reboot the target machine
 *                  through the use of a hardware device, regardless
 *                  of the state the target is in.
 * Function Inputs: none
 * Global Inputs:   
 * Return Values:   integer 0 if target is successfully rebooted,
 *                  nonzero integer otherwise
 * Global Outputs:  none
 * Errors:
 * Pre-Conditions:  TRUE (none)
 * Post-Contidions: target will be in initial "clean" state after
 *                  it is rebooted, need_to_unload and redirected_flag variables
 *                  will both be set to BOOL_FALSE.
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::reboot(void)
{
  int serial_port_fd;
  char buffer[MAX_COMMAND_STRING];
  struct termios termios_p;
  int val;
  int success = 0;
  int i = 0;

  // Exit if not unlimited reboots and exceeded the max number of reboots 
  if (target_max_number_reboots != -1 &&
      number_reboots >= target_max_number_reboots) {
    fprintf(stderr, "Error: maximum number of target reboots exceeded\n");
    exit(-1);
  }

  //printf("Starting to reboot...\n");
  number_reboots++;

  serial_port_fd = open(REBOOT_DEVICE_SERIAL_PORT, O_RDWR | O_NOCTTY);
  if (serial_port_fd == -1) {
    fprintf(stderr, "open_port: Unable to open %s - %s\n",
	    REBOOT_DEVICE_SERIAL_PORT, strerror(errno));
    return -1;
  }
    
  tcgetattr(serial_port_fd, &termios_p);
  termios_p.c_cflag |= B9600 | CS8;
  tcsetattr(serial_port_fd, 0, &termios_p);
    
  // Send command to power cycle Vxworks target
  tcflush(serial_port_fd, TCIOFLUSH);
    
  write(serial_port_fd, "///0,05;\r", 9);
  close(serial_port_fd);

  //After rebooting, set redirected and unload flags to false.
  redirected_flag = BOOL_FALSE;
  need_to_unload = BOOL_FALSE;

  // Sleep long enough to allow system to completely reboot
  sleep(VXWORKS_REBOOT_TIME);

  // Send a simple windsh command (like print hello) to make sure machine is alive
  // Try 20 times before giving up
  while(i < 20) {
    if (windsh("printf(\"hello\")","^printf\\(\"hello\"\\)\n" VALUE_QUIT) == 0) 
    {
      if (get_last_windsh_result(&val) == WINDSH_RETURN_OK) {
	if (val == 5) { //hello has 5 characters
	  success = 1;
	  break;
	}
      }
    }
    sleep(1);
    i++;
  }

  if (!success) {
    fprintf(stderr, "Target system failed to recover after reboot. \n");
    return -1; //was exit, changed to return.  **MINA**
  }

  // Redirect stdin, stdout, and stderr to windsh interpreter
  // to get return value of test MuT
  if(redirect() != 0) {
    fprintf(stderr, 
	    "Error: unable to redirect standard output on target.");
    return -1;
  }

  return 0;
}


/************************
 *
 * Function:        redirect
 * Description:     redirect will save the original file descriptors
 *                  for stdin, stdout, and sterr on the target, then
 *                  redirect them to the windsh script window for
 *                  I/O purposes.
 * Function Inputs: N/A
 * Global Inputs:   int *fd_stdin
 *                  int *fd_stdout
 *                  int *fd_stderr
 * Return Values:   integer 0 if command is successful, nozero integer
 *                  otherwise
 * Global Outputs:  int *fd_stdin
 *                  int *fd_stdout
 *                  int *fd_stderr
 * Errors:
 * Pre-Conditions: 
 * Post-Contidions: redirected_flag = BOOL_TRUE
 * Design:
 * Notes:           This function and unredirect were added during
 *                  implementation to remove dependancy on reading
 *                  data from the serial console during tests
 *
 ************************/

int Vxworks_test_manager::redirect()
{
  // File descriptor for I/O redirection on target
  int vf0;
  // Variable to hold return value from get_last_windsh_result
  int result;
    
  if (redirected_flag == BOOL_FALSE)
  {
    // Read original file descriptors and save them in input parameters
    if (windsh("ioGlobalStdGet(0)", "^ioGlobalStdGet\\(0\\)\n" VALUE_QUIT) != 0) 
    {
      return -1;
    }
    if (get_last_windsh_result(&fd_stdin) != 0) {
      return -1;
    }
    if (windsh("ioGlobalStdGet(1)", "^ioGlobalStdGet\\(1\\)\n" VALUE_QUIT) != 0) {
      return -1;
    }
    if (get_last_windsh_result(&fd_stdout) != 0) {
      return -1;
    }
    if (windsh("ioGlobalStdGet(2)", "^ioGlobalStdGet\\(2\\)\n" VALUE_QUIT) != 0) {
      return -1;
    }
    if (get_last_windsh_result(&fd_stderr) != 0) {
      return -1;
    }

    // Redirect stdin, stdout, and stderr to windsh window
    if(windsh("vf0=open(\"/vio/0\",2,0)", 
    		"^vf0=open\\(\"/vio/0\",2,0\\)\n"
		"new symbol \"vf0\" added to symbol table.\n"
		"vf0 = 0x.[0-9A-Fa-f]*: " VALUE_QUIT
    		 ) != 0) 
    {
      return -1;
    } 
    if(get_last_windsh_result(&vf0) != 0) {
      return -1;
    }
    // could not open the file descriptor on target
    if(vf0 == -1) {
      return -1;
    }
    
    if(windsh("ioGlobalStdSet(0,vf0);"
	      "ioGlobalStdSet(1,vf0);"
	      "ioGlobalStdSet(2,vf0);"
	      "logFdAdd(vf0)",
	      
	      "^ioGlobalStdSet\\(0,vf0\\);"
	      "ioGlobalStdSet\\(1,vf0\\);"
	      "ioGlobalStdSet\\(2,vf0\\);"
	      "logFdAdd\\(vf0\\)\n"
    	       VALUE VALUE VALUE VALUE
    	       "quit\n$") == 0) 
    {
      if(get_last_windsh_result(&result) != 0) {
	return -1;
      }
      if(result != 0) {
	return -1;
      }
    }
    else {
      return -1;
    }

    redirected_flag = BOOL_TRUE;
  }

  return 0;
}


/************************
 *
 * Function:        unredirect
 * Description:     unredirect will restore the original file descriptors
 *                  for stdin, stdout, and sterr on the target from
 *                  the windsh window to console, given the correct
 *                  input parameters.
 * Function Inputs: N/A
 * Global Inputs:   int fd_stdin
 *                  int fd_stdout
 *                  int fd_stderr
 * Return Values:   integer 0 if command is successful, nozero integer
 *                  otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions: 
 * Post-Contidions: redirected_flag = BOOL_FALSE if no errors occur
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::unredirect()
{
  // Variable to hold command string to be executed
  // on the target
  char command[MAX_COMMAND_STRING];
  // Variable to hold return value from get_last_windsh_result
  int result;

  if (redirected_flag == BOOL_TRUE) {    
    if(windsh("logFdDelete(vf0)", "^logFdDelete\\(vf0\\)\n" VALUE_QUIT )) {
      return -1;
    }
    
    sprintf(command,
	    "ioGlobalStdSet(0,%d);"
	    "ioGlobalStdSet(1,%d);"
	    "ioGlobalStdSet(2,%d);"
	    "close(vf0)",
	    fd_stdin, fd_stdout, fd_stderr);

    if(windsh(command,
    	      "^ioGlobalStdSet\\(0,.[0-9]*\\);"
	      "ioGlobalStdSet\\(1,.[0-9]*\\);"
	      "ioGlobalStdSet\\(2,.[0-9]*\\);"
	      "close\\(.[0-9]*\\)\n"
    	       VALUE VALUE VALUE VALUE
	      "quit\n$" )) {
      return -1;
    }

    if(get_last_windsh_result(&result)) {
      return -1;
    }
    
    if(result) {
      return -1;
    }
    
    redirected_flag = BOOL_FALSE;
  }

  return 0;
}


/************************
 *
 * Function:        windsh
 * Description:     windsh executes a command in the windsh interface
 *                  to the target.  It writes the command into the file
 *                  "windsh_input".  It will append "quit" to the end of
 *                  the file to force the windsh process to terminate
 *                  after processing the command.
 *                  command is put into int *result_value*
 * 
 *                  windsh also accepts an regular expression that is
 *                  used to match against the content of the output file
 *                  from executing the windshell script.  If the content
 *                  doesn't match the expression, the execution of the command
 *                  is assumed to be unsuccessful.
 *
 * Function Inputs: char *command
 * Global Inputs:   Vxworks_test_manager class member target_name
 * Return Values:   integer 0 if command is successful, nozero integer
 *                  otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::windsh(char *command, char* reg_expression)
{
  // File pointer for windsh script input file
  FILE *windsh_input_fptr;
  // String to call windsh script command
  char system_command_string[MAX_COMMAND_STRING];
  int good_match = 0;

  //Abort if null pointer
  assert( command != NULL);

  // Remove the output file, if any exists.  
  // This is required because we can potentially have a situation where
  // two processes are both sending their standard output to the file.
  // In the case of a restart, the child process is still writing to the file
  // however, the parent will wake up, kill the child and then try to 
  // unload the mut.  If the parent tries to unload the mut before the child
  // has actually terminated, then the windsh command for unld could be running
  // in the parent at the same time as the windsh command for 
  // execute_test_case in the child.
  // Both of these windsh commands are redirected to this file.  At times
  // the file can contain data from both processes.  

  // To fix this situation, we unlink the file before starting any windsh
  // command.  This makes sure that the file has been wiped out before starting
  // another windsh.  In the case of the restart described above, we might
  // lose the data from the child process, but we never read it anyway.

  // Ideally we would like to use pipes instead of files.  This would eliminate
  // this problem.  However, we do not have time for such a change right now.
  unlink(WINDSH_OUTPUT_FILE_NAME);

  // Open file "windsh_input" and write the command string
  // to the file along with the quit command
  if ((windsh_input_fptr = fopen(WINDSH_INPUT_FILE_NAME, "w")) == NULL) {
    return -1;
  }
  fprintf(windsh_input_fptr, "%s\n", command);
  fprintf(windsh_input_fptr, "quit\n");
  fclose(windsh_input_fptr);

    // Execute the windsh commands via a system call
  sprintf(system_command_string, "windsh %s < %s > %s 2>&1",
	  target_name, WINDSH_INPUT_FILE_NAME,WINDSH_OUTPUT_FILE_NAME);
  if (system(system_command_string) == -1) {
    return -1;
  }

  // Delete the windsh script input file
  unlink(WINDSH_INPUT_FILE_NAME);

  // Check if the content of the output file matches the expected regular
  // expression.	    
  if ( match_file(WINDSH_OUTPUT_FILE_NAME,reg_expression, &good_match) < 0) 
  {
    return -3;	  // Error in executing match_file
  }
  else if ( good_match ==  0)
  {
    return -4;  // The content of file did not match reg_expression
  }

  return 0;
}

/************************
 *
 * Function:        get_last_rval
 * Description:     This function will read the results of the
 *                  last windsh command from the file "windsh_output"
 *                  and extract the rval.  The actual rval will be delimited
 *                  by the words "rval:" at the beginning and "value =" at the
 *                  end
 * Function Inputs: char *value
 *                  int max_value_length
 * Global Inputs:   
 * Return Values:   integer 0 if command is successful, nozero integer
 *                  otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::get_last_rval(char *value, 
					int max_value_length)
{
  // File pointer for windsh script output file
  FILE *windsh_output_fptr = NULL;
  // String for reading contents of the output file
  // line by line
  char *line_string = NULL;
  // Variable for holding location of identifier
  // in line_string
  char *ptr = NULL;

  char *start_delimiter = "rval:";
  char *end_delimiter = "value =";
  
  int value_strlen = 0;

  // Abort if null pointer
  assert( value != NULL );

  // allocate this to be as big as the return value.  That's as big as we
  // ever need to support on one line.
  line_string = (char *)malloc(max_value_length);
  if(line_string == NULL) {
    return -1;
  }

    // Open windsh output file for reading
  if ((windsh_output_fptr = fopen(WINDSH_OUTPUT_FILE_NAME, "r"))
      == NULL) {
    free(line_string);
    return -1;
  }
  
  // this will ensure that value is null terminated
  // in case strncat tries to copy too much, this last null will be there
  value[max_value_length-1] = '\0';

  // loop over the input until we find the start delimiter or run
  //  out of input to process
  while (1) {
    if (!fgets(line_string, max_value_length, windsh_output_fptr)) {
      // end of file was reached before we found the start delimiter
      //  complain, bitterly
      fclose(windsh_output_fptr);
      free(line_string);
      return -1;
    }

    if (ptr = strstr(line_string, start_delimiter)) {
      // found the start delimiter; break out of the loop so we
      //  can read in the rest of the value
      break;
    }
  }

  // start building up the value string
  ptr += strlen(start_delimiter);
  safe_strncpy(value, ptr, max_value_length);
  
  // set this to NULL, so that we can track if we really did find the 
  // end delimiter
  ptr = NULL;

  // now read line by line until we find the end delimiter
  while (fgets(line_string, max_value_length, windsh_output_fptr)) {
    
    // quit when we find the end delimiter
    if(ptr = strstr(line_string, end_delimiter)) {
      *ptr = '\0'; // null terminate over the end delimiter
    }
    // keep building up the string
    strncat(value,line_string,max_value_length - 1 - strlen(value));

    if(ptr) { // since we found the end delimiter, we can break;
      break;
    }
  }
  
  // clean up
  fclose(windsh_output_fptr);
  free(line_string);
  
  // take out last '\n' put in by execute_test_case
  value_strlen = strlen(value);
  if(value_strlen > 0) {
    if(value[value_strlen - 1] == '\n') {
      value[value_strlen - 1] = '\0';
    }
  }

  if(ptr) {  // we successfully found the end delimiter
    return 0;
  }
  else {  // we did not find the end delimiter
    return -1;
  }
}


/************************
 *
 * Function:        get_last_identifier_value
 * Description:     This function will read the results of the
 *                  last windsh command from the file "windsh_output"
 *                  and search for the last string that starts
 *                  with the string indentifier.  It returns the
 *                  remainder of this string in value
 * Function Inputs: char *identifier
 *                  char *value
 *                  int max_value_length
 * Global Inputs:   
 * Return Values:   integer 0 if command is successful, nozero integer
 *                  otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::get_last_identifier_value(char *identifier, 
						    char *value, 
						    int max_value_length)
{
  // File pointer for windsh script output file
  FILE *windsh_output_fptr;
  // String for reading contents of the output file
  // line by line
  char line_string[MAX_COMMAND_STRING];
  // Return value being passed back
  int function_return_val = -1;
  // Variable for holding location of identifier
  // in line_string
  char *ptr;

  // Abort if null pointer
  assert( identifier != NULL );
  assert( value != NULL );

    // Open windsh output file for reading
  if ((windsh_output_fptr = fopen(WINDSH_OUTPUT_FILE_NAME, "r"))
      == NULL) {
    return -1;
  }

  // Keep reading until end of file is reached
  while (fgets(line_string, MAX_COMMAND_STRING, windsh_output_fptr)) {
    ptr = strstr(line_string, identifier);
    if (ptr) {
      ptr += strlen(identifier);
      safe_strncpy(value, ptr, max_value_length);
      function_return_val = 0;
    }
  }

  // Close output file
  fclose(windsh_output_fptr);
    
  return function_return_val;
}


/************************
 *
 * Function:        get_last_windsh_result
 * Description:     This function will read the results of the
 *                  last windsh command from the file "windsh_output"
 *                  and put the return value into the return_value
 *                  argument.
 * Function Inputs: int *last_result
 * Global Inputs:   
 * Return Values:   WINDSH_RETURN_OK,
 *                  WINDSH_RETURN_ABORT, or
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::get_last_windsh_result(int *last_result)
{
  // String to hold value returned from get_last_identifier_value
  char line_string[MAX_COMMAND_STRING];

  // Abort if null pointer
  assert( last_result != NULL );

  if(get_last_identifier_value("value = ",
			       line_string,
			       sizeof(line_string)) != 0)
  {
    return WINDSH_RETURN_ABORT;
  }

  sscanf(line_string, "%d", last_result);
  return WINDSH_RETURN_OK;
}


/************************
 *
 * Function:        load
 * Description:     This function will load the execute test module
 *                  into memory on the target.
 * Function Inputs: 
 * Global Inputs:   Vxworks_test_manager class member target_file_name
 * Return Values:   integer 0 if successful, nonzero integer otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions: 
 * Post-Contidions: need_to_unload == BOOL_TRUE if not error occurs
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::load(void)
{
  // String to hold windsh script command
  char command_string[MAX_COMMAND_STRING]; 

  //String to hold regular expression for load
  char reg_expression[MAX_COMMAND_STRING*2];

  // Return value read from ld call
  int return_val = -1;

  if (need_to_unload == BOOL_FALSE)
  {
    // Construct windsh command for loading object file to target
    sprintf(command_string, "ld < %s", target_file_name);

    // Construct the expected return expression for load
    sprintf(reg_expression, "^%s\n%s",command_string, VALUE_QUIT); 
    
    // Execute command and verify result
    if (windsh(command_string, reg_expression) != 0) {
      return -1;
    }

    if (get_last_windsh_result(&return_val) != WINDSH_RETURN_OK) {
      return -1;
    }
    
    // Return what ld passed back; should be -1 if unsuccessful
    if (return_val < 0) {
      return return_val;
    }

    need_to_unload = BOOL_TRUE;
  }

  return 0;

}


/************************
 *
 * Function:        unload
 * Description:     This function will unload the execute test module
 *                  from the target's memory.  It will parse out the actual
 *                  object file from the target_file_name
 * Function Inputs: 
 * Global Inputs:   Vxworks_test_manager class member target_file_name
 * Return Values:   integer 0 if successful, nonzero integer otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions: need_to_unload == BOOL_FALSE
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::unload(void)
{
  // String to hold windsh script command
  char command_string[MAX_COMMAND_STRING];

  //String to hold regular expression for unload
  char reg_expression[MAX_COMMAND_STRING*2];

  // String to hold object filename
  char *object_file_name;
  // Return value read from unld call
  int return_val=0;

  if (need_to_unload == BOOL_TRUE)
  {
    // Parse target_file_name to get object_file_name
    if (rindex(target_file_name, '/') != NULL) {
      object_file_name = rindex(target_file_name, '/') + 1;
    } else {
      object_file_name = target_file_name;
    }
    
    // Construct windsh command for unloading object file from target
    sprintf(command_string, "unld(\"%s\")", object_file_name);

    // Construct the expected return expression for load
    sprintf(reg_expression, "^unld\\(\"%s\"\\)\n%s",object_file_name, VALUE_QUIT);
    
    // Execute command and verify result
    if (windsh(command_string, reg_expression) != 0) {
      return -1;
    }

    if (get_last_windsh_result(&return_val) != WINDSH_RETURN_OK) {
      return -1;
    }

    need_to_unload = BOOL_FALSE;
  }

  // Return what unld passed back; should be zero if successful
  return return_val;
}


/************************
 *
 * Function:        create_script_command
 * Description:     This function will construct windsh command string
 *                  that can be used to run the execute test module on
 *                  the target for the parameter list defined by arguments
 * Function Inputs: MARSHAL_DATA_TYPE arguments,
 *                  char *command
 * Global Inputs:   
 * Return Values:   integer 0 if successful, nonzero integer otherwise
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Contidions:
 * Design:
 * Notes:
 *
 ************************/

int Vxworks_test_manager::create_script_command(MARSHAL_DATA_TYPE arguments,
						char *command)
{
  // String to hold marshaled arguments
  char argument_string[MAX_MARSHAL_STRING];
  // Return value from marshal function call
  int marshal_return_value;

    // Marshal parameter list
  if ((marshal_return_value = marshal(arguments, argument_string)) != 0) {
    return marshal_return_value;
  }

  // Construct command
  sprintf(command, "execute_test_case(\"%s\")", argument_string);

  return 0;
}
