/* testCaseIterator.cpp: class that will iterate through all test cases
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

#include <vector>

#include "ballista.h"
#include "testCaseIterator.h"

using namespace std;

/************************
 *
 * Function: Test_case_iterator
 * Description:constructor for the class
 * Function Inputs: server_comm - a server communication object
                    sample_size_val - the percent of tests to be run 
		    number_arguments - number of arguments to the MuT
		    vec - param file data
 * Global Inputs:
 * Return Values:
 * Global Outputs: prints server name to be consistent with old code
 * Errors:
 * Pre-Conditions:
 * Post-Conditions: get_array_1 is allocated, all member variables initialized
 * Design: setup get_array_1.  Then ask the server for the tests.
 * Notes:
 *
 ************************/

Test_case_iterator::Test_case_iterator(Server_communication *server_comm,
				       double sample_size_val,
				       int number_arguments,
				       vector<line> vec[MAXP])
{
  char server_name[255];
  int i;
  int j;

  server = server_comm;
  sample_size = sample_size_val;
  // here we print out the server name.  This is only done to keep the output
  // of BallistIC the same as Ballista
  server->get_name(server_name,sizeof(server_name));
  cout<<server_name<<endl;


  //even though RPC is supposed to be beautiful.... its not so
  //therefore I must change c++ and multidimensonial things into
  //something XDR understands
  
  get_array_1_arg.NumArg=number_arguments;
  get_array_1_arg.sample_size=sample_size;

  
  get_array_1_arg.mvl.my_arr_vec_line_len=MAXP;
  get_array_1_arg.mvl.my_arr_vec_line_val = 
    (my_vector_lines *)calloc(MAXP,sizeof(my_vector_lines));
  
  for(i=0;i<MAXP;i++){
    get_array_1_arg.mvl.my_arr_vec_line_val[i].my_vector_lines_len = 
      vec[i].size();
    if(vec[i].size()==0){
      get_array_1_arg.mvl.my_arr_vec_line_val[i].my_vector_lines_val=NULL;
    } 
    else {
      get_array_1_arg.mvl.my_arr_vec_line_val[i].my_vector_lines_val = 
	(my_line *)calloc(vec[i].size(),sizeof(my_line));
      for(j=0;j<vec[i].size();j++) {
	if(vec[i][j].p==NULL) {
	  get_array_1_arg.mvl.my_arr_vec_line_val[i].my_vector_lines_val[j] = 
	    "NULL";
	} 
	else {
	  get_array_1_arg.mvl.my_arr_vec_line_val[i].my_vector_lines_val[j] = 
	    strdup(vec[i][j].p);
	}
      }
    }
  }

  send_request_to_server(&get_array_1_arg);
}


/************************
 *
 * Function: send_request_to_server
 * Description: send a request to the server to get a test set
 * Function Inputs: data - param file info in the proper format for RPC
 * Global Inputs:
 * Return Values:
 * Global Outputs: message to user that test results were asked for 
                   and received
 * Errors:
 * Pre-Conditions: data is initialized
 * Post-Conditions: result_temp, result_1, total are set up
                    index is reset to 0
 * Design: ask the server object for the tests.  Adjust total and index
 * Notes:
 *
 ************************/
void Test_case_iterator::send_request_to_server(info *data)
{
  return_info *result_temp;

  cout<<"Fetching tests to do from server...."<<endl;
  result_temp = server->get_test_set(data);

  cout<<"Got the data"<<endl;
  cout.flush();

  result_1=&(result_temp->my_three_dim);
  total=result_temp->total;
  index = 0;

#ifdef UNIT_TEST
  FILE *fptr = fopen("iterator.txt","a");
  if(fptr) {
    char server_name[255];
    server->get_name(server_name,sizeof(server_name));
    fprintf(fptr,"received tests from %s\n",server_name);
    fprintf(fptr,"total = %d\n",total);
    fclose(fptr);
  }
#endif
}


/************************
 *
 * Function: next
 * Description: return the next two_dim to test
 * Function Inputs: 
 * Global Inputs:
 * Return Values: two_dim representing the next test to run
 * Global Outputs:
 * Errors: an exception may be thrown from the server object when it
           tries to get a new test set
 * Pre-Conditions: at least one set of tests has been requested from the server
 * Post-Conditions: index = index + 1 or 0 if a new test set was requested
                    if necessary a new test set may be requested
 * Design:  if there are no tests in this test set 
            (total == -1) return NULL marking the
	    end of the tests.
	    else if there are more tests in the current test set, 
	    then return the next one.  Finally if we have done all the tests 
	    in the current test set ask for a new one with the stall_args
 * Notes:   The strangeness here comes from the existing ballista code.  
            For some reason you have to ask for another set of tests after 
	    finishing one.  It is not entirely clear when -1 is
            return from the server as total.  We are assuming that eventually 
	    -1 will be returned from the server.  Otherwise this will be an
	    endless recursive loop.  From the original code, it appears that
	    -1 will be returned.
	    
	    total represents the total number of tests that are to be run
	    from the current set returned by the server
	    sample_size is the percentage of these tests that will actually
	    be run. It is an integer between 1 and 100.
 *
 ************************/
two_dim *Test_case_iterator::next()
{
  if(total == -1) {
    return NULL; // end of the list
  }
  else {
    if(index < total*(sample_size)/100.0) {
      two_dim *return_value = &result_1->three_dim_val[index];
      index++;

#ifdef UNIT_TEST
      FILE *fptr = fopen("iterator.txt","a");
      if(fptr) {
	int j;
	int k;
	for(j=0;j<return_value->two_dim_len;j++) {
	  for(k=0;k<return_value->two_dim_val[j].one_dim_len;k++) {
	    fprintf(fptr,"%s\t",return_value->two_dim_val[j].one_dim_val[k]);
	  }
	  fprintf(fptr,"\n");
	}
	fprintf(fptr,"---\n");
      }
      fclose(fptr);
#endif

      return return_value;
    }
    else {
      // we have used up all the tests given to us by the server
      // now ask for a new set of tests
      // after we get them, call next() again to get the first test or
      // to determine that we are done
      info stall_arg;
      stall_arg.NumArg=0;
      stall_arg.sample_size=0;
      stall_arg.mvl.my_arr_vec_line_len=0;
      send_request_to_server(&stall_arg);
      return next();
    }
  }
}


/************************
 *
 * Function: ~Test_case_iterator
 * Description: destructor for the class
 * Function Inputs:
 * Global Inputs:
 * Return Values:
 * Global Outputs:
 * Errors:
 * Pre-Conditions: get_array_1_arg is allocated
 * Post-Contidions: get_array_1_arg is deallocated
 * Design:  free memory allocated in get_array_1_arg
 * Notes:
 *
 ************************/
Test_case_iterator::~Test_case_iterator()
{
  int il;

  // free up the memory needed to send the info to the server
  // check that the pointer is not null before freeing it
  for(il=0;il<MAXP;il++) {
    if(get_array_1_arg.mvl.my_arr_vec_line_val[il].my_vector_lines_val)
      free(get_array_1_arg.mvl.my_arr_vec_line_val[il].my_vector_lines_val);
  }
  free(get_array_1_arg.mvl.my_arr_vec_line_val);
}











