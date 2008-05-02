/* selfHost.cpp: Implementation of Selfhost_test_manager class
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

#include <assert.h>  //for asserts in manage_test
#include <dlfcn.h>  //for dynamic loading in constructor
#include <iostream>  //for cout and cerr
#include <signal.h>  //for SIGKILL
#include <stdio.h>  //generally needed
#include <stdlib.h>  //generally needed
#include <string.h>  //for strncpy, strcat, strstr in manage_test
#include <sys/types.h>  //for pid_t and other types
#include <sys/wait.h>  //for waitpid
#undef _XOPEN_SOURCE //dec unix hack by ece team
#include <unistd.h>  //generally needed, fork, process stuff
#include <errno.h>   //added 11/16/99 for error reporting

#include "ballista.h"
#include "ballistaError.h"
#include "ballistaUtil.h"
#include "marshal.h"
#include "selfHost.h"

using namespace std;

extern "C" int execute_test_case (char *marshalled_parameters);

#ifdef SUN
//Our Sun's at CMU seem to be missing the usleep proto
//from unistd.h

extern "C" int usleep(unsigned int useconds);
#endif

#define TEMP_BUF_SIZE 1024

#define RVAL_STRING "rval:"



/************************
 *
 * Function:        Selfhost_test_manager
 * Description:     Constructor for class Selfhost_test_manager
 * Function Inputs: None
 * Global Inputs:   
 * Return Values:   N/A
 * Global Outputs:  
 * Errors:          throws an error if handle or test_exec_ptr cannot be set
 * Pre-Conditions:  None
 * Post-Contidions: pid, child_exists_flag, test_exec_ptr, handle are set
 * Design: Since we are using dynamic loading, the test_exec_ptr and handle
 *         must be set here.
 * Notes: 
 *        set test_exec_ptr to execute_test_case in that file, 
 *        set pid to 0.
 *
 ************************/

Selfhost_test_manager::Selfhost_test_manager(const char *mut_filename)
  throw (Ballistic_error)
{
  void *handle;  //handle for dynamically loaded file

  if (!(handle = dlopen(mut_filename, RTLD_NOW))) {
    throw Ballistic_error(dlerror());
  }
  
  if (!(test_exec_ptr = (int (*) (char *)) dlsym (handle, "execute_test_case"))) {
    throw Ballistic_error(dlerror());
  }
  
  pid = 0;

}

/************************
 *
 * Function:        ~Selfhost_test_manager
 * Description:     Destructor for class Selfhost_test_manager
 * Function Inputs: N/A
 * Global Inputs:   N/A
 * Return Values:   N/A
 * Global Outputs:  N/A
 * Errors:          N/A
 * Pre-Conditions:  Object must have been constructed previously
 * Post-Contidions: Object does not exist; leftover child process will
 *                  be reaped
 * Design:          N/A
 * Notes:  If child exists, reap it
 *
 ************************/
Selfhost_test_manager::~Selfhost_test_manager() {
  cout << "Cleaning up test manager." << endl;
  
  if (pid != 0) {
    // clean up child process
    kill (pid, SIGTERM);
    waitpid (pid, NULL, 0);  //we don't care about status, so we use NULL
  }  
}

/************************
 *
 * Function:        manage_test
 * Description:     Selfhost-specific implementation of the virtual
 *                  function derived from the base class Test_manager
 *                  This function controls all the Ballista testing on the
 *                  client.
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
 * Errors:          waitpid fails, marshal fails, fork fails, undefined test
 *                  result
 * Pre-Conditions:  All pointers in parameters are valid
 * Post-Contidions: One test case will be completed.  All the output variables
 *                  will be set.
 * Design: Section 5.2.2.3 has the pseudocode algorithm
 * Notes:
 *
 ************************/

int Selfhost_test_manager::manage_test(MARSHAL_DATA_TYPE arguments,
				      long timeout_value,
				      char *pass_status,
				      int max_pass_status,
				      int *call_ret,
				      char *sys_err, int max_sys_err,
				      char *mut_return, 
				      int max_mut_return) {

  char marshalled_arguments [MAX_MARSHAL_STRING];   //marshalled string
  int marshal_return_value = 0;    //return value of marshal()
  int status = 0;  //Status of forked process
                                           //to see if there are problems.
  pid_t wait_pid_return = 0;  // Return value of waitpid
  int pipe_file_desc [2]; //file descriptors for pipe between parent & child
  FILE * mut_output;
  char * temp_mut_return = NULL; //temporary string to hold piped output
  char * mut_return_start = NULL; //pointer to actual return value from mut
  long long start_time = 0;
  long long timeout = timeout_value;

  int num_matching = 0; // how many characters did we match out of 'rval:'
  int num_to_match = 0; // how many characters must we match
  char *string_to_match = NULL;  // string to match against
  int c = 0;  // character read from file
  int bytes_read = 0; // number of bytes read from pipe

  struct sigaction action; //signal-handling struct


  // Abort if null pointer
  assert(arguments != NULL);
  assert(call_ret != NULL);
  assert(sys_err != NULL);
  assert(mut_return != NULL);
  assert(pass_status != NULL);
  
  //marshal arguments into string for test execution
  if ((marshal_return_value = marshal(arguments, marshalled_arguments)) != 0) {
    return MARSHAL_STRING_FAILED;
  }

  //set up pipe for communication between parent and child processes
  if (pipe (pipe_file_desc) < 0) {
    cerr << "Pipe Failed" << endl;
    return PIPE_FAILED;
  }
  
  // Fork off a process and check for errors
  if ((pid = fork()) < 0) {
    cerr << "Fork Failed." << endl;
    return FORK_FAILED;
  }
  
  else if (pid == 0) {  //child process to execute test
    close(pipe_file_desc[0]);
    //redirect Output only from this child to the pipe
    dup2 (pipe_file_desc[1], STDOUT_FILENO); //stdout

    // remove signal handlers that were installed
    sigemptyset (&action.sa_mask);
    action.sa_handler = SIG_DFL;
    (void) sigaction (SIGINT, &action, NULL);
    (void) sigaction (SIGQUIT, &action, NULL);
    (void) sigaction (SIGTERM, &action, NULL);
    (void) sigaction (SIGABRT, &action, NULL);
        
    exit(test_exec_ptr (marshalled_arguments)); //exits with errno status
  }  //end child process
  
 //parent process to monitor child
  close(pipe_file_desc[1]);
  mut_output = fdopen(pipe_file_desc[0], "r");

  start_time = get_time_in_microseconds();

  while(start_time + timeout > get_time_in_microseconds())  {
    wait_pid_return = waitpid (pid, &status, WNOHANG);
    
    //check if waitpid error
    if (wait_pid_return < 0) {
      cerr << "Error waiting for child process.  Child %d " << pid;
      cerr << "does not exist, its group does not exist," << endl;
      cerr << "or the process is not a child of the Test Manager." << endl;
      return WAITPID_FAILED;
    }
    // check if process terminated
    if(wait_pid_return > 0) {
      pid = 0;  //signal to parent child no longer exists
      break;
    }
    
    // sleep for 500 microseconds before checking again
    usleep(500);      
  } //end while
  
  // check that the loop terminated because the process terminated
  if (wait_pid_return > 0) {
    pid = 0;

    // read one character at a time from the pipe.
    // keep checking against RVAL_STRING to see if we match
    // after we match, copy up until max_mut_return-1 characters into 
    // mut_return.  Then NULL terminate mut_return.  Finally remove the
    // last carriage return in mut_return.  That '\n' comes from 
    // execute_test_case

    num_matching = 0;
    num_to_match = strlen(RVAL_STRING);
    string_to_match = RVAL_STRING;

    while((c = fgetc(mut_output)) != EOF) {
      if(c == string_to_match[num_matching]) {
	num_matching++;
	if(num_matching == num_to_match) {
	  // we've found 'rval:', now read in the rest
	  bytes_read = fread(mut_return,
			     sizeof(char), 
			     max_mut_return-1,
			     mut_output);
	  
	  if((bytes_read < 0) || (bytes_read > max_mut_return-1)) {
	    cerr << "Error reading from pipe." << endl;
	    return READING_PIPE_FAILED;
	  }
	  mut_return[bytes_read] = '\0'; // null terminate

	  // take out trailing carriage return if it is there
	  if(bytes_read > 0) {
	    if(mut_return[bytes_read - 1] == '\n') {
	      mut_return[bytes_read - 1] = '\0';
	    }
	  }
	  break;
	}
      }
      else {  // we didn't match this char, so reset the count
	num_matching = 0;
      }
    }

    
    fclose(mut_output);

    // process has terminated, either a Pass or an Abort has occurred
    if (WIFSIGNALED(status)) {
      *call_ret = -1;
      safe_strncpy(pass_status, "Done - Abort", max_pass_status);
    }
    else if (WIFEXITED(status)) {
      *call_ret = WEXITSTATUS(status);
      if (*call_ret == 99)
      {
        safe_strncpy(pass_status, "Done - Setup Failed", max_pass_status);
      }
      else
      {
        safe_strncpy(pass_status, "Done - Pass", max_pass_status);
      }
      if (*call_ret >= 0) {
        // Copy error message string for reporting. added 11/16/99
        safe_strncpy(sys_err, strerror(*call_ret), max_sys_err);
      }
    }
    else if (WIFSTOPPED(status)) {
      *call_ret = -1;
      safe_strncpy(pass_status, "Done - Stopped", max_pass_status);
    }
    else { //something has gone wrong, we should never fall through here
      cerr << "Test is not an Abort, Restart, or Pass" << endl;
      return UNDEFINED_RESULT;
    }
  } //end if process termination ended loop
  else { 
    // did not terminate, clean up child processes
    kill (pid, SIGKILL);
    *call_ret = waitpid (pid, &status, 0);
    pid = 0;
    safe_strncpy(pass_status, "Done - Restart", max_pass_status);
  } 

  return RETURN_OK;
} //end manage_test()

