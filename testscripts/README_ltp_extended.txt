*********************************************************************************************
Description:

The extented version of LTP runs tests from the $LTPRoot/testcase/fivextra directory.
After the tests are run, runltp.sh is called.  A brief explanation of the tests can be found 
in the $LTPROOT/doc directory in the file ltp_extended_doc.txt file.  The tests that are 
selected to run can be configured by the user. 
********************************************************************************************

INSTRUCTIONS:

In order to run the run_extended_ltp.sh script you must first configure the test suite
for your architeture. This can be achieved with two steps. In the testcase/fivextra there
are EXCLUDE files for different architecures. Typically there extention is the architecture
(EXCLUDE.architecture) for example EXCLUDE.i686  Copy that file to EXCLUDE and recompile.
 If that doesn't solve the problem the you may want to copy EXLUDE.general to EXCLUDE and 
recomplie.  An alternative to this is to run MAKEFILE.sh.  It will automatically generate 
an EXCLUDE file for your system.  This will need to be done if the direcory didn't compile 
with ltp.

Once compilation has concluded, the next step needed to configure this test suite is to comment/uncomment (using # ) out tests in the .scenario and .exclude configuration files 
found in the runtest directories.  At the beginning of each scenario file, a pkgchk
 (package check) script is run to see if certain software packages are installed on your
 system.  A corresponding test will be run for those packages that are found.  If the 
software package isn't found then the test will more than likey fail.  Tests that you 
may wish to exclude should be placed into the .exclude files or just do not include it 
into the scenario file. An .exclude file is optional.


In order to run run_extended_ltp.sh you need to run the script with an argument. 
The choices for the arguments are located in the runtest directory and are named with 
a .scenario and .exclude files.  An example of how to run this is the following:

In the runtest directory there are files called extend30baseia32.exclude and 
extend30baseia32.scenario, hence you need to add extend30baseia32 to the command line 
to run run_extended_ltp.sh. 


	run_extended_ltp.sh extend30baseia32 


If there are any testcases you wish to exclude from running, add the into the .exclude file.
The log files that shows the pass/fail status of individual test cases  will be 
generated in the /tmp/results_extended directory.  Also addtional log files will be generated 
in the /tmp directory.  One file that has a -test extention shows the testcases run(same as .scenario file). The other file that is generated is a   ( .out) file that shows the test execution status.