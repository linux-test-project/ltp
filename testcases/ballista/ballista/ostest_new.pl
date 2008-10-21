# ostest.pl: perl script associated with Ballista OS Robustness Test Suite
# Copyright (C) 1998-2001  Carnegie Mellon University
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#------------------------------------------------------------------------
#
#Description: run all Ballista tests for the functions specified in callTable.all. 
#
#   Files: callTable.all, ballista client download
#   Directories: ./outfiles/ and subdirectories               
#
#Outputs: all stored in /outfiles/: faillog, outfile.$function.@params, named after 
#         each function name tested, os_results.txt.  
#
#         Additionally subdirectories under 
#         /outfiles are created one per function.  These subdirectories have the 
#         following naming convention <functionName>_bug_report.  Included in these 
#         bug_report subdirectories is a script called genCode.  If executed genCode 
#         will generate c++ code that reproduces the robustness failures detected.  
#         (One cpp file per robustness failure)   
#          
#         A html file called outfiles/os_results.html can be used to navigate the 
#         scripts.  
#
#Note: catastrophic failures (whose which crash or hang the entire machine) are not 
#      automatically detected and therefore not available in the result files.
#

# ** Maintain updated version number and release date **
$version_num = "1.0";
$release_date = "May 3, 2001";

# MB: separator text within results files -- used to search for next test results
$results_separator = "----";
chomp($results_separator);

#find user's paths to necessary files:
$env_path = $ENV{'PATH'};
$ENV{'PATH'} .= ":.:";
# removed because of problem on Linux systems:
#$ENV{'PATH'} .= `pwd`;
$ENV{'PATH'} .= ":/usr/include";
$Path = $ENV{'PATH'};
# find header files
@Path = split(":", $Path);
$found_headers = 0;
foreach $Dir (@Path)
{
  opendir(DIR, $Dir);
  @files = readdir(DIR);
  closedir(DIR);
  foreach $file (@files)
  {
    if ($file =~ /stdio\.h$/)
    {
      $found_headers = 1;
      $ENV{'PATH'} .= ":";
      $ENV{'PATH'} .= $Dir;
    }
  }
}
if ($found_headers != 1)
{
  print "\nCannot find directory containing header files \(\".h\" files\).  Please set your environment variable \n";
  print "to contain the path to the correct directory. \(ex. set path=\(\$path = /usr/include\)\)\n";
  exit(1);
}

# Determine user's operating system
$os_string = `uname -rsv`;
@OSID = split(/ /, $os_string);
chomp($OSID[2]);
$os = $OSID[0];
$version = $OSID[1];
$release = $OSID[2];

# copy any os specific templates over the standard templates
if ($os eq Linux)
{
  $os_name =  LINUX;
}
elsif ($os eq  SunOS)
{
  $os_name = SUN;
}
elsif ($os eq OSF1)
{
  $os_name = DUNIX;
}

opendir(TPL_PATH,  "templates");
@tplOSfiles = grep(/$os_name/,readdir(TPL_PATH));

foreach $tplfile(@tplOSfiles)
{
  ($tplname, $suffix) = split (/$os_name/,$tplfile);
   
  $cpy_string = "cp templates/$tplfile templates/$tplname$suffix";
  #print  "COMMAND $cpy_string\n";
  if(system($cpy_string) != 0)
  {
     printf ("\n Problems setting up OS specific templates.  Try to execute\n");
     printf ("$cpy_string\n \n");
  }
} 

closedir (TPL_PATH);

# copy over callTable.all for platform specific callTables
if ($os_name eq DUNIX)
{
  #  $cpy_string = "cp callTableDUNIX.all callTable.all";
  $cpy_string = "cp callTable.some callTable.all";  
  if(system($cpy_string) != 0)
  {
     printf ("\n Problems setting up OS specific callTable.  Trying to execute\n");
     printf ("$cpy_string\n \n");
  }
}




  print "\nCleaning up previous results...\n";
  opendir(OUTFILES_PATH, $path);
  @allfiles = grep(/^outfile\./, readdir(OUTFILES_PATH));
  if ($os !~ "OSF1")
  {
    foreach $file (@allfiles)
    {
      $outfile = "$path/$file";
      chomp($outfile);
      unlink($outfile) || die "Cannot remove $file: $!";
    }
  }
  else
  {
    foreach $file(@allfiles)
    {
      $outfile = "$path/$file";
      chomp($outfile);
      system("rm $outfile");
    }
  }
  closedir(OUTFILES_PATH);
  system("rm -r outfiles");


#print "number of arguments $#ARGV \n";
if($#ARGV < 0)
{
$user_percent=100;
}
else
{

    $user_percent=$ARGV[0];

}

print "$user_percent % \n";

#exit;

#print "\nThis package will limit individual function tests to 5,000 total test cases.  If this is acceptable, \n";
#print "press Enter.  If you would like to test all possible cases \(100\%\), type \"100.\"   To test fewer \n";
#print "cases, enter the desired percentage of test cases to run.  \(Note test cases per function are still \n";
#print "limited to a maximum of 5,000 per function even set at 100\%.\ \n) ";
#print " By default the value will be 100 %, and will timeout in 10 seconds \n";
#print " If no percentage is entered \n";


#my $timeout_value=10;

# start
#$SIG{ALRM} = sub { die "timeout" };

# eval {

# alarm( $timeout_value );

# get the input
#$user_percent = <STDIN>;

#alarm( 0 );

# };



# if ( $@ ) 
# {

#     if ( $@ =~ /timeout/i ) 
#     {
#	 $user_percent=100;

#     } 
#     else 
#     {
#	 alarm( 0 );
#	 die;
#     }

# }







#print "$user_percent % \n";

#    exit;

if ($user_percent > 100)
{
  print "\nYou cannot run more than 100\% of the test cases.  Reseting to 5,000 test cases...";
  $user_percent = 100;
}


  if (! (-d "outfiles"))
  {
    mkdir(outfiles, 0777) || die "Can't make outfiles directory: $!\n";
    mkdir("outfiles/temp",0777) || die "Can't make outfiles/temp directory: $!\n";
  }


$path = "outfiles";
$template_path = "templates";
$logfile="outfiles/logfile.txt";

open(CALLTABLEIN,"<callTable.all")||die "Can't read callTable.all\n";
open(FAILFILE, ">$path/faillog")||die "Can't open faillog: $!\n";
open(VERSIONFILE, ">$path/ostest_version.txt")||die "Can't open ostest_version.txt \n";
print VERSIONFILE "$version_num\n";
close VERSIONFILE;

if ((system("./configure"))/256 == 1)
{
	print "Error running configure\n";
	print FAILFILE "Error running configure\n";
	exit(1);
}
if ((system("make -f MakefileHost"))/256 == 1)
{
        print "Error mak(e)ing Ballista MakefileHost\n";
        print FAILFILE "Error mak(e)ing Ballista makefileHost\n";
        exit(1);
}

if ((system("make -C compile"))/256 == 1)
{
        print "Error mak(e)ing Ballista datatype compiler\n";
        print FAILFILE "Error mak(e)ing Ballista datatype compiler\n";
        exit(1);
}


open LOGFILE,">$logfile" or die "Cannot open $outfile for write :$!";
while(<CALLTABLEIN>)
{
  if (!($_ =~ "#")&&(!($_=~":"))&&($_ ne "\n")){#not comment, or empty, or title
    open(CALLTABLEOUT,">callTable")||die "can not write to call table file: $!\n";;
    print (CALLTABLEOUT $_);
    close(CALLTABLEOUT);
  ($inc, $identity, $rval, $function, @params)=split(/\s+/, $_);
   open(INCLUDEFILE, ">userIncludes.h") || die "Can't open userIncludes.h: $!";
   print INCLUDEFILE "#include <$inc>";
   close(INCLUDEFILE);

# if ((system("./makeBallista  $function")/256) != 0)
 if ((system("./makeBallista  $function > /dev/null 2>&1")/256) != 0)	# if makeBallista fails, don't try to run ballista!
  {
	print "\nmakeBallista failed for $function.  $function will not be tested -- this function may not exist on your operating system.";
	print FAILFILE "makeBallista failed for $function.  $function will not be tested -- this function may not exist on your operating system.\n";
  }
  else
  {

# *** note ballista server information ignored
      print LOGFILE "**************************************\n";
      print LOGFILE "* Currently testing \"$function\"      \n";
      print "**************************************\n";
      print "* Currently testing \"$function\"      \n";

    $exec_str = "./ballista ballista.ece.cmu.edu $function -t 5000000 -r ";
    $num_tests = 1;
    $test_percent = 0;
    $param_str = "";
    foreach $param(@params)
    {
      $num_tests *= `wc -l $template_path/$param.param`; 
      $param_str .= "$template_path/$param.param ";
    }
# limit number of tests to 5,000 (can't just specify straight percent for all functions because may lose smaller tests)
    if(($user_percent <= 100) && ($user_percent >0))
    {
      $test_percent = $user_percent;
    }
    else
    {
# ******** changed to 10 to test: change back to 1000!!!!
      if ($num_tests >4000)
      {
        $test_percent = (4000/$num_tests)*100;
      }
      else
      {
        $test_percent = 100;
      }
    }

    $exec_str_bkp = $exec_str;
    $exec_str .= "$test_percent $param_str ";
    $exec_str .= " |tee $path/outfile.$function";

    foreach $param(@params)
    {
      $exec_str .= ".$param";
    }
  
  
#  print $exec_str;
#run the test
#  if ((system("$exec_str")/256) == 1)	# if ballista fails with fractional percents, round down
 # {
#    $test_percent = int($test_percent);
    if($test_percent < 0.001)	# in case round down to 0
    {
        print "\nThe test percentage was specified below 0.001.  Percentage changed to 0.001.\n";
	$test_percent = 0.001;
    }
    else
    {
       $test_percent = 1.0 * $test_percent;
    }
    $exec_str = $exec_str_bkp;
    $exec_str .= "$test_percent $param_str ";
    $exec_str .= " > $path/outfile.$function";
    $find_fail = "grep -i -q Fail ";
    $output_filename ="";
    $output_filename .= "outfile.$function"; 

    $find_fail .= $path."/outfile.".$function; 

    foreach $param(@params){
      $exec_str .= ".$param"; 
      $find_fail .= ".$param";  
      $output_filename .= ".$param"; 
    }


      $out_filename{$function} = $output_filename;

 #   print "$find_fail \n\n";
 #  print "$function  $out_filename{$function}\n";
 #     print "$exec_str ";
    if ((system("$exec_str")/256) == 1)
    {
      print "\nError running ballista.";
      print FAILFILE "\nError running ballista.";
      exit(1);
    }
 # }	# does not appear to be working to test for fractional failure -- does not attempt to run ballista again upon failure

#  $start_time = time;
#  print "\nWaiting for TCP to reset (about 2 minutes)....\n";
#  while(time < $start_time+120)
#  {	# wait for 2 minutes to elapse to allow RPC to reset
#  }

      if ((system("$find_fail")/256) == 0)
       {
	  print LOGFILE "* $function has failed at least once  \n"; 
	  print "* $function has failed at least once  \n";  
          $status{$function} = "Yes";
       }
       else
       { 
	  print LOGFILE "* $function passed                    \n";
	  print "* $function passed                    \n";  
	  $status{$function} = "No";
       }

          print LOGFILE "**************************************\n\n";
          print LOGFILE "Compling... \n";

          print "**************************************\n\n";
          print "Compling... \n";


} # end if makeBallista


#"./ballista ballista.ece.cmu.edu $function -r 100 $template_path/@params.param ");
}#if
}#while(<CALLTABLEIN>)



close(CALLTABLEIN);
close(FAILFILE);
close LOGFILE;


  #copy files necessary for create_code_standAlone to work to outfiles directory
  if (( (system("cp create_code_standAlone.pl outfiles/."))/256 == 1) ||
      ( (system("cp callTable.all outfiles/callTable"))/256 == 1) ||
      ( (system("cp callGen_standAlone outfiles/."))/256 == 1))
  {
     print "ERROR copying files needed to create genCode - bug reports will be suspect\n";
     print " trying to:\n cp create_code_standAlone.pl outfiles/. \ncp callTable.all outfiles/callTable\n cp callGen_standAlone outfiles/.\n";
     die "Terminating Ballista OS Robustness Test Suite due to errors\n";
  }

# compute failure rates for each function and total failure rate

open(SUMMARY, ">$path/os_results.txt") || die "Can't open os_results.txt: $!";
($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);
$mon++;		# adjust for Perl's labelling of months as 0-11
$year = $year + 1900;
$date_stamp = "at $hour:$min:$sec on $mon/$mday/$year";
printf(SUMMARY "\nBallista OS test, version %s released %s", $version_num, $release_date);
#printf(SUMMARY "\nTHIS IS AN INCOMPLETE VERSION -- SOME FUNCTIONS WILL NOT BE TESTED BECAUSE THEY REQUIRE MORE TEST CASES THAN THE SERVER CAN CURRENTLY HANDLE");
printf(SUMMARY "\nResults for: %s version %s release %s, completed %s", $os, $version, $release, $date_stamp);
printf(SUMMARY "\n\n");

#header information 
system("cp htmlHeader.html $path/os_results.html");
open(OUTHTML,">>$path/os_results.html") || die "Can't open os_results.html: $!";

# derived from parse.cgi by David Guttendorf
# modified to use Perl functions rather than spawn Unix functions, which could be precarious
# to run this locally, want to use the files stored locally in outfiles/outfile.$function.@param_types
# get all filenames beginning with "outfile."
opendir(OUTFILES_PATH, $path) || die "Can't open directory $path: $!";
@possiblefiles = grep(/^outfile\./, readdir(OUTFILES_PATH));
closedir(OUTFILES_PATH);

$failures = 0;
$total_percent = 0;
$total_abort_percent = 0;
$total_restart_percent = 0;
$total_setup = 0;
$total_functions = 0;
$setup_string = "Setup Failed";
chomp($setup_string);

foreach $file (@possiblefiles){
        $outfile = "$path/$file";
        chomp($outfile);
        ($junk, $function, @param_types)=split(/\./,$file);    # find just the function name
        
        open(OUTFILE, "<$outfile") || die "Can't open $outfile: $!";
        @file_lines = <OUTFILE>;
        close(OUTFILE);
        $tests_run = grep(/^$results_separator/, @file_lines);
        if ($tests_run != 0)          # if no tests, may have timed out - don't record any data or attempt to divide by zero
        {
# create the directory and code necessary for generating bug reports
               $genCodeDir = "outfiles/";
               $genCodeDir .= $function;
               $genCodeDir .= "_bug_report";
               if (mkdir($genCodeDir, 0777) == 0)
               {
                  print "Multiple function calls using $genCodeDir directory\n";
               }
               $genCodeDir .= "/genCode";



# the following are "+=" because a function may occur more than once in the callTable; this assumes the variables are initialized to zero!
        	$abort_failures{$function} += grep(/Abort/, @file_lines);
        	$restart_failures{$function} += grep(/Restart/, @file_lines);
        	$setup_failures{$function} += grep(/$setup_string/, @file_lines);

                if (($abort_failures{$function} > 0) || ($restart_failures{$function} > 0))
                {
                   $num_parameters = @param_types;
                   if ((system("./genCodeCreator $function $outfile  $num_parameters $genCodeDir"))/256 == 1)
                   {
                      print "ERROR encountered creating genCode file for $function\n";
                   }
                   else
                   {
                      chmod 0700, $genCodeDir;
                   }
                }
                else
                {
                   open(GENCODE,">$genCodeDir");
                   print GENCODE "# No Abort or Restart Robustness Failures encountered\n";
                   close(GENCODE);
                }
                $totaltests{$function} += $tests_run;
                $totalruns{$function} =  $totaltests{$function} - $setup_failures{$function};
        	$failures{$function} += $abort_failures{$function} + $restart_failures{$function};
        	$passes{$function} += $totaltests{$function} - $failures{$function};

                if (($totaltests{$function} == 0) ||($totalruns{$function} <=0))
                {
                  $fail_percent{$function} = 0;
                  $restart_percent{$function} = 0;
                  $abort_percent{$function} = 0;
                }
                else
                {
                  $fail_percent{$function} = ($failures{$function}*100/$totalruns{$function});
                  $restart_percent{$function} = ($restart_failures{$function}*100/$totalruns{$function});
                  $abort_percent{$function} = ($abort_failures{$function}*100/$totalruns{$function});
                }

# moved because functions with 2 different call signature counted incorrectly
#               $total_alltests += $totaltests{$function};
#                $total_setup += $setup_failures{$function};
#                $total_percent += $fail_percent{$function};
#                $total_abort_percent += $abort_percent{$function};
#                $total_restart_percent += $restart_percent{$function};
                $total_functions++;
        }else
        {
        print FAILFILE "makeBallista failed for $function.  $function will not be tested -- this function may have timed out because of large test cases.\n";
        }
  }
foreach $function(sort keys(%totaltests))
{
	write STDOUT;  
	write SUMMARY; 
        print OUTHTML "<TR>\n";
        $tempStr = $function;
        $pass_fail=$function;
	$pass_fail  .= "  pass/fail stats";
        $tempStr .= "_bug_report/genCode";
        print OUTHTML "<TD ALIGN=\"LEFT\"><A HREF = \"$tempStr\">$function</A></TD>\n";
        print OUTHTML "<TD ALIGN=\"LEFT\"><A HREF =\"$out_filename{$function}\">$pass_fail</A></TD>\n"; 
        print OUTHTML "<TD ALIGN=\"RIGHT\">$status{$function}</TD>\n";
	print OUTHTML "<TD ALIGN=\"RIGHT\">$totaltests{$function}</TD>\n";
        print OUTHTML "<TD ALIGN=\"RIGHT\">$restart_failures{$function}</TD>\n";
        print OUTHTML "<TD ALIGN=\"RIGHT\">$abort_failures{$function}</TD>\n";
        print OUTHTML "<TD ALIGN=\"RIGHT\">$setup_failures{$function}</TD>\n";
        print OUTHTML "</TD>\n";

        $total_alltests += $totaltests{$function};
        $total_setup += $setup_failures{$function};
        $total_percent += $fail_percent{$function};
        $total_abort_percent += $abort_percent{$function};
        $total_restart_percent += $restart_percent{$function};
}
# compute simple average of individual functions' failure rates --
#  non-existent POSIX functions do not produce results files, so do not skew percentages
$overall_fail_rate = ($total_percent/($total_functions));  # if using as an int, add .5 for rounding?
$overall_abort_rate = ($total_abort_percent/$total_functions);
$overall_restart_rate = ($total_restart_percent/$total_functions);

printf(SUMMARY "\n----------------------------------------------------------------------------------------------\n");
printf(STDOUT "\n-----------------------------------------------------------------------------------------------\n");

$function = "OVERALL FAILURE RATE:";
$totaltests{$function} = $total_alltests - $total_setup;
$setup_failures{$function} = $total_setup;
$fail_percent{$function} = $overall_fail_rate;
$restart_percent{$function} = $overall_restart_rate;
$abort_percent{$function} = $overall_abort_rate;
write SUMMARY;
write STDOUT;

printf(SUMMARY "\nTests which failed to properly setup are not included in the above percentages\n");
printf(STDOUT "\nTests which failed to properly setup are not included in the above percentages\n");
print OUTHTML "</TR></TABLE>\n";
print OUTHTML "\n<p> Finish Date: $mon/$mday/$year";
print OUTHTML "</BODY></HTML>\n";


close(OUTHTML);
close(SUMMARY);
close(FAILFILE);
print "Ballista OS Robustness Test Suite complete   ";

##############
# format for output results file:

format SUMMARY_TOP =
Function                    Num. tests   (#Setup Fail)  | Restart %    Abort %      Total %
--------                    ----------   -------------  | ---------   ---------    ---------
.

format SUMMARY =
@<<<<<<<<<<<<<<<<<<<<<<    @##########   (@##########)  |@###.####   @###.####    @###.#### 
$function                $totaltests{$function}  $setup_failures{$function}  $restart_percent{$function}  $abort_percent{$function}  $fail_percent{$function}
.

format STDOUT_TOP =
Function                    Num. tests   (#Setup Fail)  |  Restart      Abort        Total
--------                    ----------   -------------  | ---------   ---------    ---------
.

format STDOUT =
@<<<<<<<<<<<<<<<<<<<<<<    @##########   (@##########)  |@###.####%  @###.####%   @###.####%
$function                $totaltests{$function}  $setup_failures{$function}  $restart_percent{$function}  $abort_percent{$function}  $fail_percent{$function}
.

