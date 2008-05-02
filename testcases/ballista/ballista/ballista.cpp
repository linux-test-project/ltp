// ballista.cpp  Main file for ballista test harness
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


#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <values.h>
#include <ctype.h>

#undef _XOPEN_SOURCE //dec unix hack
#include <unistd.h>
#include <vector>
#include <assert.h>
#include <sys/stat.h>

#include "line.h"

#include "ballistaRPC.h"

#include "ballista.h"
#include "ballistaUtil.h"
#include "ballistaError.h"
#include "parseArguments.h"
#include "serverCommunication.h"
#include "testCaseIterator.h"
#include "testManager.h"

using namespace std;

#ifdef B_VXWORKS
    #include "vxworks.h"
#else
    #include "selfHost.h"
#endif

#define NO_SERVER   
#ifdef NO_SERVER   
int LCVs[MAXP];
int MAXs[MAXP];
#endif


// the file that holds the test execution
#define MUT_OUT "./mut.out"
#define NO_SERVER 

// This is a kludge to set an upper bound on the number of test cases
#define MAXIMUM_TEST_CASES 4000

//This macro deletes a dynamically allocated object if it exists, and sets
//its handle to NULL.
#define DELETE_NULL(ptr) if (ptr) { \
                              delete ptr; \
                              ptr = NULL; \
                         }


char     pBuf[1048];       //don't want to pay the overhead of creating
                           //and deleting many times

//These are global so they can be destroyed properly when aborting
//Since only one of each is created during the entire program, having them
//as global should not be a problem.
Test_manager *test_manager_obj = NULL;
Parse_arguments * arguments = NULL;
Server_communication *server = NULL;
Test_case_iterator *biterator = NULL;

vector<line> vec[MAXP];    //contains array of all setting for each arg

int parameterize(b_param param[],line l);

int checkLCVs(int numParam)
//This returns 1 if LCV is maxed out, 0 if not
//according to the values stored in LCVs and the max's stored in MAXs
//
//PRE: MAXs and LCV initialized to something
{
    int max_found=1;
    int index=0;
    while (max_found && index<numParam)  //do it until we have checked all or found one not maxed out
    {
        max_found=LCVs[index]==(MAXs[index]-1);
        index++;
    }
//      for (int ii=0;ii<numParam;ii++)
//              cout<<LCVs[ii]<<" - ";
//      cout<<endl;
    return max_found;
}

int incLCVs(int numParam)
//this function increments the LCVs and indicates if it overflowed by returning a 1
//ie, if it overflowed stop.  Like reaching EOF
{
    int done=0;
    int index=0;

    if (checkLCVs(numParam)) return 1;   //already maxed out

    while (!done && index<numParam)  
    {
        LCVs[index]++;
        if (LCVs[index]==MAXs[index]) //carry - set to zero inc next place
        {
            LCVs[index]=0;
        }
        else
            done=1; //no carry
        index++;
    }
    return 0;   //since we were not maxed out at the start, we are guaranteed to be able
    //to inc at least once
}

/************************
 *
 * Function:  two_dim_to_marshalled_data
 * Description: convert a two dim into a marshalled_data
 * Function Inputs: marshal - destination MARSHAL_DATA_TYPE
                    parameter_list - source two_dim
 * Global Inputs:
 * Return Values: none
 * Global Outputs:
 * Errors:  
 * Pre-Conditions:
 * Post-Conditions: marshal contains the equivalent of parameter_list
 * Design:  copy all of the characters from the parameter list into the 
            marshal.  Null terminate each list of test value attributes as
	    well as the list of dial settings
 * Notes:
 *
 ************************/
void two_dim_to_marshalled_data(MARSHAL_DATA_TYPE marshal, 
				const two_dim *parameter_list)
{
    int parameter;
    int attribute;

    assert(marshal != NULL);
    assert(parameter_list != NULL);

    // check that we won't overflow the array
    assert(parameter_list->two_dim_len <= MAXP);

    for(parameter=0;parameter < parameter_list->two_dim_len;parameter++) 
    {
      for(attribute = 0; 
  	  attribute < parameter_list->two_dim_val[parameter].one_dim_len;
	  attribute++) 
      {

          // check that we don't overflow the array
          assert(parameter_list->two_dim_val[parameter].one_dim_len <= MAXD);

          safe_strncpy(marshal[parameter][attribute],
		       parameter_list->two_dim_val[parameter].
		       one_dim_val[attribute],
		       sizeof(marshal[parameter][attribute]));
        }
        // null terminate attribute list
        marshal[parameter][attribute][0] = 0; 
    }
    // null parameter attribute list
    marshal[parameter][0][0] = 0; // null terminate
}

/************************
 *
 * Function: ballista_1
 * Description: This function iterates through all of the required tests and 
                executes them with the supplied test manager.  Then it sends 
		the results of the tests to the server.

		There is a kludge to set an upper bound on the number of tests
		using MAXIMUM_TEST_CASES.
		
 * Function Inputs:  server_name - name of the ballista server
		     function_name - name of the MuT that is being tested
		     timeout - how long to wait in microseconds for a restart
		     sample_size - what percent of tests should be run (1-100)
		     NumArg - number of arguments to the parameter
		     myvec - contains all of the data from the param files
 * Global Inputs: biterator, server, test_manager_obj, arguments
 * Return Values: 0 if successful, nonzero if any errors
 * Global Outputs:
 * Errors: If an object cannot be created or the test manager fails, 
           then the function will return -1
 * Pre-Conditions: test_manager_obj has been created
 * Post-Conditions: tests have been executed and results have 
                    been sent to server
 * Design:  Create a server and iterator object.  
            Iterate through all of the tests and
            send the results to the server.
 * Notes:
 *
 ************************/
int ballista_1(const char *server_name,
	   char *function_name,
	   int timeout,
	   double sample_size, 
	   int number_arguments,
	   vector<line> myvec[MAXP])
{
    two_dim *parameter_list = NULL;
    MARSHAL_DATA_TYPE marshalled_data;
    int i;
    int j;
    int test_results;
    char pass_status[255];
    int call_ret;
    char sys_err[255];
    char mut_return[MAX_RETURN];
    struct data_from_client data;
    int test_count = 0;
    int comb_number=0;  //combination number of the test actually executed as test_count #
    int test_total=1;

    struct stat tmp_stat_struct;

    assert(! stat(MUT_OUT, &tmp_stat_struct));
    assert(server_name != NULL);
    assert(function_name != NULL);
    assert(myvec != NULL);


   

#ifdef NO_SERVER //no server to communicate with
    try
    {
        //first initialize all the LCV arrays   
        //not really needed to zero it all, only to num_arguments, but CYA
        for (int ii=0;ii<MAXP;ii++)
            LCVs[ii]=MAXs[ii]=0;
 
        //now setup the maximums
        for (int ii=0;ii<number_arguments;ii++)
        {
            MAXs[ii]=vec[ii].size();
            test_total*=vec[ii].size();
        }
        int done_testing=0;
        double side1,side2;
        int rnum;
        while (!done_testing)
        { 
            comb_number++;
            rnum = rand();
            side1 = (double)rnum/(double)RAND_MAX;
            //side2 = (double)MAXIMUM_TEST_CASES/(double)test_total;
            
            // Note old side (above) did not take into consideration the percentage
            
            if (MAXIMUM_TEST_CASES < (test_total*sample_size/100.0))
            {
              side2 = (double)MAXIMUM_TEST_CASES/(double)test_total;
            }
            else
            {
              side2 = (double) sample_size/100.0;
            }

            // normally the second part of if (after the or) would be here but wanted to 
            // perform at least 1 test.
            if ((side1<side2) || ((test_count == 0) && (comb_number == test_total -1)))
            {
    
                test_count++;
                // here we print out the dial settings used for this test, and setup the marshalled
                // data
                for (i=0;i<number_arguments;i++)
                {
                    j=parameterize(marshalled_data[i],vec[i][LCVs[i]]);
                    cout <<"#param:";
                    for (int jj=0;jj<j;jj++)
                        cout<<marshalled_data[i][jj]<<"\t";
                    cout<<endl;
                }
                // remove template Log File
                FILE* filePtr = NULL;
                if ((filePtr = fopen("/tmp/templateLog.txt","r")) != NULL)
                {
                    fclose(filePtr);
                    system ("rm -f /tmp/templateLog.txt");
                }

                test_results = test_manager_obj->manage_test(marshalled_data,
                                                             (long)timeout,
                                                             pass_status,
                                                             sizeof(pass_status),
                                                             &call_ret,
                                                             sys_err,
                                                             sizeof(sys_err),
                                                             mut_return,
                                                             sizeof(mut_return));
                if (!test_results)
                {
                    cout<<"rval:" << mut_return << endl;
                    cout<<pass_status;
                    cout<<"\t"<<call_ret<<endl;
                    if((call_ret!=-1) && (call_ret!=0))
                    {
                        if (call_ret == 99) //problem in setup
                        {
                            char str[256];
                            if ((filePtr = fopen("/tmp/templateLog.txt","r")) != NULL)
                            {
                                while(!feof(filePtr))
                                {
                                    if (fgets(str,254,filePtr))
                                    {
                                        cout << str << endl;
                                    }
                                }                                
                                fclose (filePtr);
                                system ("rm /tmp/templateLog.txt");
                            }
                            else
                            {
                                cout <<sys_err<<endl;
                            }
                        }   
                        else
                        {
                            //intentionally changed for ostest - push button
                            //cerr <<sys_err<<endl;
                            cout <<sys_err<<endl;
                        }
                   }

                } // if test succeeded
                else
                {
                    cerr <<"Error in the test manager" << endl << sys_err << endl;
                    return -1;
                }
                if (MAXIMUM_TEST_CASES < (test_total*sample_size/100.0))
                {
                  cout<<"------- "<<test_count<<" of ~"<<MAXIMUM_TEST_CASES<<" Combination #"<<comb_number
                      <<" of "<<test_total<<"----"<<endl;
                }
                else
                {
                  cout<<"------- "<<test_count<<" of ~"<< test_total * sample_size/100.0 <<" Combination #"<<comb_number
                      <<" of "<<test_total<<"----"<<endl;
                }
   
            }
            done_testing=incLCVs(number_arguments);
                        
        } // while there are still tests
    } // try
    catch (Ballistic_error be)
    {
        cerr << be.get_error_message();
        return -1;
    }
#else  //running with server

    try 
    {
        server = new Server_communication(server_name);
        if(!server) 
        {
            cerr << "Error: Out of memory while allocating server ";
            cerr << "communication objects." << endl;
            return -1;
        }
    }
    catch (Ballistic_error be) 
    {
        cerr << be.get_error_message() << endl;
        return -1;
    }

    try 
    {
        biterator = new Test_case_iterator(server,sample_size,
	              			   number_arguments,myvec);
        if(!biterator) 
        {
            cerr << "Error: Out of memory while allocating iterator." << endl;
            return -1;
        }
        test_count = 0;
        while(test_count < MAXIMUM_TEST_CASES && 
	     (parameter_list = biterator->next())) 
        {
            test_count++;

            two_dim_to_marshalled_data(marshalled_data,parameter_list);

            // here we print out the dial settings used for this test
            for(i=0;marshalled_data[i][0][0];i++) 
            {
	        for(j=0;marshalled_data[i][j][0];j++) 
                {
	            cout<<marshalled_data[i][j]<<"\t";
	        }
	        cout<<endl;
            }
            // remove template Log File
            FILE* filePtr = NULL;
            if ((filePtr = fopen("/tmp/templateLog.txt","r")) != NULL)
            {
                system ("rm -f /tmp/templateLog.txt");
            }    
            test_results = test_manager_obj->manage_test(marshalled_data,
  					                (long)timeout,
					                pass_status,
					                sizeof(pass_status),
					                &call_ret,
					                sys_err,
					                sizeof(sys_err),
					                mut_return,
					                sizeof(mut_return));
            if(!test_results) 
            {

  	        cout<<"rval:" << mut_return << endl;
	        cout<<pass_status;
	        cout<<"\t"<<call_ret<<endl;
	        if((call_ret!=-1) && (call_ret!=0))
                {
                    if (call_ret == 99) //problem in setup
                    {
                        char str[256];
                        if ((filePtr = fopen("/tmp/templateLog.txt","r")) != NULL)
                        {
                            while(!feof(filePtr))
                            {
                                if (fgets(str,254,filePtr))
                                {
                                    cout << str << endl;
                                }
                            }
                            fclose (filePtr);
                            system ("rm /tmp/templateLog.txt");
                        }
                        else
                        {
                            cout <<sys_err<<endl;
                        }
                    }
                    else
                    {
                        //intentionally changed for ostest - push button
	                //cerr <<sys_err<<endl;
	                cout <<sys_err<<endl;
                    }
	        }    
	
 	        // now start packing the results into "data" to send to the server
	        // in Ballistic we no longer have a NULL, but rather an empty string
	        // XDR cannot handle NULL string so send over the word NULL instead
	        if(pass_status[0] == '\0') 
                {
	            data.passtatus="NULL";
	        }
	        else 
                {
	            data.passtatus = pass_status;
	        }

	        data.call_ret = call_ret;
	
	        // in Ballistic we no longer have a NULL, but rather an empty string
 	        //XDR cannot handle NULL string so send over the word NULL instead
	        if(sys_err[0] == '\0') 
                {
	            data.sys_err="NULL";
	        }
	        else 
                {
	            data.sys_err = sys_err;
	        }

	        data.params=*parameter_list;

	        data.NumArg=number_arguments;
	        data.function=function_name;
	
	        //Send the results of this test over to the server
	        server->send_test_results(&data);
            } // if test succeeded
            else 
            {
	        cerr <<"Error in the test manager" << endl << sys_err << endl;
	        return -1;
            }
      
            cout<<"----"<<endl;

      /*      char dummy;
	      printf("Press return to continue...");
	      scanf ("%c", &dummy);
      */
        } // while there are still tests
    } // try
    catch(Ballistic_error be) 
    {
        cerr << be.get_error_message();
        return -1;
    }
#endif //NO_SERVER

    return 0;
}

void lineVecTest()
  //test the line vector template
{
  vector <line> lineVec;

  vector<line>::iterator i;

  lineVec.push_back( line("This is"));
  lineVec.push_back( line(" a"));
  lineVec.push_back( line(" test.\n"));

  for (i=lineVec.begin();i!=lineVec.end();i++)
    cout<<*i;
}


/************************
 *
 * Function: lineIn
 * Description: reads a line and inserts it into a line vector
 * Function Inputs: ins - input stream, v - line vector
 * Global Inputs:
 * Return Values: 
 * Global Outputs:pBuf will be set to the line read in from the stream
 * Errors:
 * Pre-Conditions:
 * Post-Conditions: the next line from ins will be added to v if it 
                    has text in it.
 * Design:
            read in the next line from the stream
	    check if there is a non space character in the line
	    if so, then copy the line into l and then put l in v

 * Notes:   This function was written by the ECE department
            It had to be modified by the Ballistic team to accomodate 
	    blank lines in the .param files, since they are common when
	    creating .param files by hand for testing Ballistic
 *
 ************************/
int lineIn(istream& ins,vector<line> &v)
  //this function reads in a string and stores it in the linevector
  //read in from ins, store in v

{
    line l;
    int i;
    int length;
    BOOL_TYPE hasText;

    ins.getline(pBuf,sizeof(pBuf),'\n');

    hasText = BOOL_FALSE;
    length = strlen(pBuf);
    for(i=0;i<length && !hasText;i++) 
    {
        if(!isspace(pBuf[i])) 
        {
            hasText = BOOL_TRUE;
        }
    }
    if(hasText) 
    {
        l = pBuf;
        if (ins.gcount()>0) 
        {
            v.push_back(l);
        }
    }

    return (ins.gcount());
}

void getVec(istream& ins,vector<line> &v)
  // This function reads an entire file (ins) and stores it
  // line by line in vector v
{
    while (!ins.eof())
    {
        lineIn(ins,v);
    } 
}

void test()
{
    //just run a few short tests

    vector<line> v;
    int i;
    line l;
    char *t=NULL;

    lineVecTest();

    getVec(cin,v);
    for (i=0;i<v.size();i++)
        cout<<v[i].p<<endl;

    cout<<"\n\nTokenization of:\n"<<v[1]<<"\n\n";
    t=strtok(v[1].p,"\t");
    while (t!=NULL)
    {
        cout<<t<<endl;
        t=strtok(NULL,"\t");
    }
  
}

int parameterize(b_param param[],line l)
    //this function breaks a given line into its components
    //seperated by a tab (/t)
{
    char *t=NULL;
    int i=0;
    int firstOne=1;

    t=strtok(l.p,"\t");
    while (t!=NULL)
    {
        if (!(i==1 && firstOne==1)) //skip the number of dials datum
  	    strcpy(param[i++],t);
        else
	    firstOne=0;


        if (i==MAXP)
	{
	    cerr<<"Error, attempted to parameterize string with more than <<"<<MAXD
	        <<" dial components\n"<<l<<endl;
	    exit(1);
	}
        t=strtok(NULL,"\t");
    }
    return i;
  
}


/************************
 *
 * Function: usage
 * Description: display the usage message.
 * Function Inputs: executable_name
 * Global Inputs:
 * Return Values: 
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Conditions:
 * Design:
          display the usage of the program.  use ifdefs to show the differences
	  between different operating systems

 * Notes:
 *
 ************************/
void usage(char *executable_name) 
{
    assert(executable_name != NULL);

#ifdef B_VXWORKS
    cerr<<"Usage: "<<executable_name;
    cerr<<" server function_name [-r percent][-t timeout] ";
    cerr<<"-i target_address -n target_name [-m number | -c {yes,no}] "; 
    cerr<<"<param file>..." << endl;
    cerr<<"Where" << endl;
  
    cerr<<"  server is the ballista server" << endl;
    cerr<<"  function_name is the function to be tested" << endl;
    cerr<<"  -r percent is the percent of tests to be run." << endl;
    cerr<<"     valid range --  1 to 100 (default 100)" << endl;
  
    cerr<<"  -t timeout is maximum time in microseconds for restarts " << endl;
    cerr<<"     and catastrophics failures" << endl;
    cerr<<"     valid range --  100000 to 100000000 (default 10,000,000)" << endl;
    
    cerr<<"  -i target_address is the IP address of the target machine" << endl;
    cerr<<"     * This is required. *" << endl;
  
    cerr<<"  -n target_name is the target's name that is registered ";
    cerr<<"with Tornado." << endl;
    cerr<<"     * This is required. *"<<endl;
  
    cerr<<"  -m number is the maximum number of reboots. " << endl;
    cerr<<"     Use -1 (default) for unlimited rebooting."<<endl;
  
    cerr<<"  -c {yes,no} - Specify a clean test each time or not. " << endl;
    cerr<<"     yes will reboot the target after each test."<<endl;
  
    cerr<<"  param file is the data type defined by file."<<endl;
#else
    cerr<<"Usage: "<<executable_name<<" server function_name ";
    cerr<<"[-r percent] [-t timeout] ";
    cerr<<"<type file>..."<<endl;
    cerr<<"Where"<<endl;
    cerr<<"  server is the ballista server"<<endl;
    cerr<<"  function_name is the function to be tested" << endl;
    cerr<<"  percent is between 1 and 100 (default 100)"<<endl;
    cerr<<"  timeout is maximum time in microseconds for restarts ";
    cerr<<"(default 10,000,000)"<<endl;
    cerr<<"  type is the data type defined by file."<<endl;
#endif    
}


/************************
 *
 * Function: clean_up
 * Description: clean up after catching signals
 * Function Inputs: int exit status
 * Global Inputs: biterator, server, test_manager_obj, arguments
 * Return Values: N/A
 * Global Outputs: exits with exit_status
 * Errors: N/A
 * Pre-Conditions: N/A
 * Post-Conditions: Program has cleaned-up after itself.  Then exit is
 *                  called with the status input to the function
 * Design: N/A
 * Notes: Author: Mina Atanacio
 *
 ************************/

void clean_up (int exit_status) 
{
    DELETE_NULL(biterator)
    DELETE_NULL(server)
    DELETE_NULL(test_manager_obj)
    DELETE_NULL(arguments)
    exit(exit_status); 
}

/************************
 *
 * Function: catch_signal
 * Description: clean up after catching signals
 * Function Inputs: int signal_number
 * Global Inputs: N/A
 * Return Values: N/A
 * Global Outputs: N/A
 * Errors: N/A
 * Pre-Conditions: Signal was caught
 * Post-Conditions: Program has cleaned-up after itself.  This is done
 *                  by calling clean_up with exit status 2
 * Design: N/A
 * Notes: Author: Mina Atanacio
 *
 ************************/

void catch_signal (int signal_number) 
{
    cerr << "Caught signal "<< signal_number<< ", cleaning up..." << endl;
    clean_up(2);
}


/************************
 *
 * Function: main
 * Description: main function of the harness
 * Function Inputs: argc, argv
 * Global Inputs:
 * Return Values: 0 upon success
 * Global Outputs:
 * Errors:
 * Pre-Conditions:
 * Post-Conditions:
 * Design:
          Set up signal-handling procedure
          Parse the command line arguements.  
	  Some arguments are always used
	  others are OS specific.  All OS specific stuff is ifdefed
	  Do error checking on command line arguments.
	  Create a test manager object, read param files, 
	  and pass them to ballista_1

 * Notes:
 *
 ************************/

int main(int argc,char *argv[])
{
    char *server_name;
    char *function_name;
    int timeout;
    int numParam=0;
    int numP=0;

    double sample=100.0;        //random sampling vs exhaustive
    int startArg;    //where the file arguments start
    int numArg;
    int ballista_return = 0;  //return value of ballista_1, the testing function

    struct sigaction action;   //signal-handling struct

    struct stat tmp_stat_struct;  // temp structure for call to stat()

    // could not have specified server and function name
    if(argc < 3) 
    {
        usage(argv[0]);
        clean_up(1);
    }

    //Set up signal-handling for entire program.  Signals caught are:
    //SIGINT, SIGQUIT, SIGTERM, SIGABRT.
    sigemptyset (&action.sa_mask);
    action.sa_handler = catch_signal;  //catch_signal is signal-handler
    (void) sigaction (SIGINT, &action, NULL);
    (void) sigaction (SIGQUIT, &action, NULL);
    (void) sigaction (SIGTERM, &action, NULL);
    (void) sigaction (SIGABRT, &action, NULL);
    // End of signal handling set-up


    arguments = new Parse_arguments;
    assert(arguments);

    startArg = arguments->init_and_find_index(3,argc,argv);
  
    if(arguments->get_and_validate_float(&sample,"r",100.0,0.0,100.0)) 
    {
        cout<<"Start random sample - ~ "<<sample<<"% of space." << endl;
    }
  
    if(arguments->get_and_validate_integer(&timeout,
  					   "t", 
					   10000000,
					   100000,
					   100000000)) 
    {
        cout <<"Timeout set to " << timeout << endl;
    }
  
    cout.flush();

    if (stat(MUT_OUT, &tmp_stat_struct)) 
    {
        cerr << "Could not find test execution file " << MUT_OUT << endl;
        clean_up(1);
    }

#ifdef B_VXWORKS
    char ip_address[255];
    char target_name[255];
    int maximum_number_of_reboots;
    BOOL_TYPE reboot_after_each_test;
    char clean_temp[255];

    if(arguments->get_argument("i",ip_address,sizeof(ip_address))) 
    {
        cout << "Target IP address is " << ip_address << endl;
    }
    else 
    {
        cerr << "Target IP address was not specified. Use -i option." << endl;
        clean_up(1);
    }

    if(arguments->get_argument("n",target_name,sizeof(target_name))) 
    {
        cout << "Target name is " << target_name << endl;
    }
    else 
    {
        cerr << "Target name was not specified. Use -n option." << endl;
        clean_up(1);
    }

    reboot_after_each_test = BOOL_FALSE;

    if(arguments->get_argument("c",clean_temp,sizeof(clean_temp))) 
    {
        if(!strcasecmp(clean_temp,"yes"))  
        { // if  case insensitive equal
            reboot_after_each_test = BOOL_TRUE;
            cout << "Rebooting after each test" << endl;
        }
        else 
        {
            cout << "Not rebooting after each test" << endl;
        }
    }

    if(arguments->get_and_validate_integer(&maximum_number_of_reboots,
					 "m",
					 -1,
					 -1,
					 MAXINT)) 
    {
        if(maximum_number_of_reboots == -1) 
        {
            cout << "Unlimited number of reboots allowed." << endl;
        }
        else 
        {
            cout << "Maximum number of reboots is " << 
	    maximum_number_of_reboots << endl;
        }
    }

    // if we are rebooting after each test, but yet there is a maximum
    // limit on the number of reboots, give an error and exit
    if((reboot_after_each_test) && (maximum_number_of_reboots != -1)) 
    {
        cerr << "Error: cannot specify maximum number of reboots and ";
        cerr << "reboot after each test options together" << endl;
        clean_up(1);
    }

    cout.flush();

    // if no param files
    if (argc == startArg) 
    {
        usage(argv[0]);
        clean_up(1);
    }
  
    test_manager_obj = new Vxworks_test_manager(MUT_OUT,
					      target_name,
					      ip_address,
					      maximum_number_of_reboots,
					      reboot_after_each_test);
    if(!test_manager_obj) 
    {
        cerr << "Error: could not create test manager."<<endl;
        clean_up(1);
    }
#else
  
    if (argc==startArg) 
    {
        usage(argv[0]);
        clean_up(1);
    }

    try 
    {
        /* Prepend a ./ to avoid having the user set their LD_LIBRARY_PATH 
           environment variable.*/
        test_manager_obj = new Selfhost_test_manager(MUT_OUT);
    }
    catch (Ballistic_error be) 
    {
        cerr << "Error in creating the test manager:" << endl;
        cerr << be.get_error_message() << endl;
        cerr << "Exiting" << endl;
        clean_up(1);
    }
    if(!test_manager_obj) 
    {
        cerr<< "Error: could not create test manager. Exiting..."<<endl;
        clean_up(1);
    }

#endif //B_VXWORKS
  
    numArg = argc-startArg;
    for (int i=startArg;i<argc;i++) 
    {
		 ifstream is;
        is.open(argv[i]);
        if(is.bad()) 
        {
            cerr<<"Could not open the file "<<argv[i]<<".\n";
            clean_up(1);
        }
        getVec(is,vec [i-startArg]);
        is.close();
        numParam++;
        if (numParam==MAXP) 
        {
            cerr<<"Error, only "<<MAXP<<" parameters supported.\n";
            clean_up(1);
        }
    }
  
    //numP=parameterize(b,vec[0][2]);
  
    vector<line>::iterator iter[MAXP];
  
    for (int ji=0;ji<numArg;ji++)
        iter[ji]= vec[ji].begin();

    int caseNum=1;
  
    server_name=argv[1];
    function_name = argv[2];

#ifdef UNIT_TEST
    // these two files are used when unit testing.  They should be identical
    // after running if sample == 100
    unlink("server.txt");
    unlink("iterator.txt");
#endif

    ballista_return = ballista_1(server_name,function_name,
  			         timeout,sample, numArg, vec);

    //Call clean-up macro to delete objects
    DELETE_NULL(biterator)
    DELETE_NULL(server)
    DELETE_NULL(test_manager_obj)
    DELETE_NULL(arguments)
  
    return ballista_return;
}






