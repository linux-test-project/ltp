# consolidate.pl: consolidates results from os test suite into tgz file
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
#Description: consolidates the results from the os test suite (ostest.pl) into
#             a tarred and gzipped file.  User questioned as script progresses.
#
#   Existing Files: callTable.all, os_results.txt, os_results.html, configHints.txt, faillog, ostest_version.txt 
#   Created Files: catastrophic.txt, sysInfo.txt, configInfo.txt, tplInfo.txt, compileInfo.txt
#                  userInfo.txt
#   Directories: ./templates, ./consolidate, ./outfiles                
#
#Outputs: A single tar(red), gzip(ped) and uuencoded file named consolidate.tar.gz.uu is created.  
#          
#         Additionally files listed under "Created Files" are created.
#

# ** Maintain updated version number and release date **
$version_num = "1.0.0";
$release_date = "May 30, 2001";
$path = "consolidate";

# Header Information
print "Thank you for choosing to report your Ballista OS Robustness test results.\n";
print "If you choose to cancel the reporting you may do so by entering <Ctrl-C> at any time.\n";
print "\nThe results are actually sent (by you) in a separate step\n\n";


# Check for existing results files and query about deletion. 
if (-d $path)
{  
  print "It appears that you have previously consolidated your results.  If you wish to save the\n";
  print "prior consolidation then type \"NO\", copy the consolidate subdirectory to another location, \n";
  print "and restart the script.  Otherwise your prior consolidation will be deleted \n";
  print "Continue consolidation?  <Yes/No>\n";

  if(<STDIN> =~ /^no/i)
  {
    exit(1);
  }
  else
  {
    print "\nCleaning up previous consolidation...\n";
    opendir(CON_DIR,$path);

    while (defined($file = readdir(CON_DIR)))
    {
      $outfile = "$path/$file";
      chomp($outfile);

      if (($file != ".")&&($file != ".."))
      { 
        if ($os !~ "OSF1")
        {
          unlink($outfile) || die "Cannot remove $path\$file: $!";
        }   
        else
        {
          system("rm $outfile");
        }
      }
    }
    closedir(CON_DIR); 
    system("rm -rf $path");
  }
}

if (! (-d $path))
{
  mkdir($path, 0777) || die "Cannot make $path directory: $!\n";
}

###################################
# Process Catastrophic Functions

open(CATFILE,">$path/catastrophic.txt") || die "Cannot open $path/catastrophic.txt.  Consolidation halted prematurely\n $!"; 

open(CALLTABLEIN,"<callTable.all")|| die "Cannot read the file named callTable.all.  The consolidate script should be run in the same ballista directory ostest.pl was run.$!\n";
$count = 0;
while(<CALLTABLEIN>)
{
  if ($_ =~ /^#CAT/i)
  {
    $count= $count +1;
    print CATFILE "$_";
  }
}

if ($count == 0)
{
  print "Did you encounter any catastrophic errors when executing the test suite? \n";
  print "(Did the system crash when running the test suite?)  <Yes/No>\n";

  if(<STDIN> =~ /^YES/i)
  {
    print CATFILE "#%#START USER COMMENTS#%#\n";
    print "Please list the functions that produced catastrophic errors.  (Enter a single \n";
    print "period (\".\") on a line to indicate the end of the function list.)\n";

    #gather input and append it to catastrophic.txt
    $finished = 0;
    while($finished < 99)
    {
      $userinput = <STDIN>;
      if ($userinput =~ /^.$/)
      {
        $finished = 999;
      }
      else
      {
        print CATFILE "$userinput";
        $finished++;
      }  
    }
    if ($finished == 99)
    {
      print "Input halted after 99 lines.\n";
    }
  }
  else
  {
    print CATFILE "No catastrophic robustness failures reported\n";
  }
}
else
{
  close(CATFILE);
  open(CATFILE, "<$path/catastrophic.txt")|| die "Cannot open $path/catastrophic.txt.  Consolidation halted prematurely\n $!";
  print "Based on callTable.all the following functions appear to produce catastrophic errors:\n";
  #output the contents of catastrophic.txt
  while (<CATFILE>)
  {
    print "$_";
  }
  close(CATFILE);
  open(CATFILE,">>$path/catastrophic.txt")|| die "Cannot open $path/catastrophic.txt.  Consolidation halted prematurely\n $!";

  print "\nAre there additional functions that produced catastrophics or comments about the above functions \n";
  print "you wish to add. <Yes/No>\n";

  if(<STDIN> =~ /^YES/i)
  {
    print CATFILE "#%#START USER COMMENTS#%#\n";

    print "Please report any additional functions producing catastrophic errors or \n";
    print "add comments about the above catastrophic errors.  (Enter a single \n";
    print "period (\".\") on a line to indicate the end of the function list.)\n";

    #gather input and append it to catastrophic.txt
    $finished = 0;
    while($finished < 99)
    {
      $userinput = <STDIN>;
      if ($userinput =~ /^.$/)
      {
        $finished = 999;
      }
      else
      {
        print CATFILE "$userinput";
        $finished++;
      }
    }
    if ($finished == 99)
    {
      print "Input halted after 99 lines.\n";
    }
  }
}  
 
close(CATFILE);

#################################
# Process System Information

if ((system("uname -rsvm > $path/sysInfo.txt"))/256 == 1)
{
  print "\nProblems encountered trying to gather system information.  Command uname -rsvm returned unsuccessfully\n";
  print "Please report the following system information:\n";
  print " - operating system under test (e.g. Linux)\n";
  print " - operating system release (e.g. 2.2.16-22smp)\n";
  print " - operating system version (e.g. \#1 SMP Tue Aug 22 16:39:21 EDT 2000)\n";
  print " - machine (hardware) type (e.g. i686)\n\n";
  $nothing = 1;
}
else
{
  $nothing = 0;
  open(SYSFILE, "<$path/sysInfo.txt")|| die "Cannot open $path/sysInfo.txt.  Consolidation halted prematurely\n $!";
  print "\nThe following system information has been recorded:\n";

  #output the contents of sysInfo.txt
  while (<SYSFILE>)
  {
    print "$_";
  }
  close(SYSFILE);
}

open(SYSFILE,">>$path/sysInfo.txt")|| die "Cannot open $path/sysInfo.txt.  Consolidation halted prematurely\n $!";

print "\nIs there additional system or machine information you wish to record? <Yes/No>\n";
       
if(<STDIN> =~ /^YES/i)
{
  print SYSFILE "#%#START USER COMMENTS#%#\n";
   
  print "Please report any additional system or machine information. \n";
  print "(Enter a single period (\".\") on a line to indicate the end of the information.)\n";
     
  #gather input and append it to sysInfo.txt
  $finished = 0;
  while($finished < 99)
  {
    $userinput = <STDIN>;
    if ($userinput =~ /^.$/)
    {
      $finished = 999;
    }
    else
    {
      print SYSFILE "$userinput";
      $finished++;
    }
  }
  if ($finished == 99)
  {
    print "Input halted after 99 lines.\n";
  }
}
else
{
  if ($nothing == 1)
  {
    print SYSFILE "No system information recorded\n";
  }
}

close(SYSFILE);

############################
# Configuration Information

open(CONFIGFILE,">>$path/configInfo.txt")|| die "Cannot open $path/configInfo.txt.  Consolidation halted prematurely\n $!";

print "\nAre there any changes you made to the system configuration that are likely to \n";
print "affect the results?  <Yes/No> \n";
if(<STDIN> =~ /^YES/i)
{
  print CONFIGFILE "#%#START USER COMMENTS#%#\n";
  
  print "Please enter additional system configuration information. \n";
  print "(Enter a single period (\".\") on a line to indicate the end of the information.)\n";
  
  #gather input and append it to configInfo.txt
  $finished = 0;
  while($finished < 99)
  { 
    $userinput = <STDIN>;
    if ($userinput =~ /^.$/)
    {
      $finished = 999;
    }
    else
    {
      print CONFIGFILE "$userinput";
      $finished++;
    }
  }   
  if ($finished == 99)
  {
    print "Input halted after 99 lines.\n";
  }
}
else
{
  print CONFIGFILE "No system configuration information recorded\n";
}

close(CONFIGFILE);

################################################################
# Compiler Version - Determine the compiler used and then query

# Determine user's operating system
$os_string = `uname`;
chomp($os_string);

# copy any os specific templates over the standard templates
if ($os_string eq Linux)
{
  $os_name = linux;
}
elsif ($os_string eq SunOS)
{
  $os_name = solaris;
}
elsif ($os_string eq OSF1)
{
  $os_name = osf1;
}
else
{
  $os_name = default;
}

$host_header = "<Begin host:";
$host_cc = "host-CC";

open (CONFIGFILE, "configHints.txt")|| die "Cannot open configHints.txt.  Consolidation halted prematurely\n $!";
@configList = <CONFIGFILE>;

@subsetList = grep /$host_header|$host_cc/, @configList;

# os_spec_flag values:
#   0  when the <Begin host:$os_name has not been encountered
#   1  after the <Begin host:$os_name has been encountered but before $host_cc or another $host_header
#   2  after the <Begin host:$os_name and the associated $host_cc have been encountered
#   3  after the <Begin host:$os_name is followed by a $host_header without a $host_cc inbetween
#
# only 2 is a successful exit condition.  
$os_spec_flag = 0;
foreach $config_line (@subsetList)
{
  if ($os_spec_flag == 0)
  {
    if ($config_line =~ /^$host_header$os_name/)
    {
      $os_spec_flag = 1;
    }
  }
  elsif ($os_spec_flag ==1)
  {
    if($config_line =~ /$host_cc/)
    {
      @bits = split(/\s+/, $config_line);
      @compiler = split(/]/,$bits[4]);
      $os_spec_flag = 2;
    }

    if($config_line =~ /^$host_header/)
    { 
      print "\nAn ERROR was encountered trying to determine the C++ compiler specified by configHints.txt\n";
      print " No compiler specification listed after $host_header$os_name and before $config_line\n";
      $os_spec_flag = 3;
    }
  }
}

if ($os_spec_flag ==1)
{
  print "\nAn ERROR was encountered trying to determine the C++ compiler specified by configHints.txt\n";
  print " No compiler specification listed after $host_header$os_name\n";
}

if ($os_spec_flag == 0)
{
  print "\nAn ERROR was encountered trying to determine the C++ compiler specified by configHints.txt\n";
  print "$host_header$os_name> is not listed\n";
}
$version_flag = 0;
 
if ($os_spec_flag == 2)
{
  open (SHELLSCRIPT, ">compiler_version") ||die "Cannot open file compiler_version";
  print SHELLSCRIPT "#!/bin/csh\n";
  if ($compiler[0] eq "cxx")
  {
    print SHELLSCRIPT "cxx -V >& $path/compileInfo.txt\n";
  }
  elsif ($compiler[0] eq "g++")
  {
    print SHELLSCRIPT "g++ -v >& $path/compileInfo.txt\n";
  }
  else
  {
    print "\nUnable to determine the version associated with the C++ compiler $compiler[0] specified in configHints.txt\n";
    print "Please specify\n";
    $version_flag = -1;
  }
  close SHELLSCRIPT;

  @dummy_list = ('compiler_version');
  chmod 0700, @dummy_list;

  if (((system('compiler_version'))/256) == 1)
  {
    print "\nError encountered trying to determine compiler version for $compiler[0].  Please specify\n";
    open (COMPILEFILE,">$path/compileInfo.txt") || die "Cannot open file $path/compileInfo.txt";
    print COMPILEFILE "compiler = $compiler[0]\n";
    close COMPILEFILE;
    $version_flag = -1;
  }
  else
  {
    $version_flag = 1;
    print "\nThe compiler version was determined to be:\n";
    open (COMPILEFILE, "<$path/compileInfo.txt") || die "Cannot open file $path/compileInfo.txt";
    while (<COMPILEFILE>)
    {
      print $_;
    }
    print "\nWould you like to add an additional compile information? <Yes/No>\n";
    if(<STDIN> =~ /^YES/i)
    {
      $version_flag = 0;
    }
  }
}

if ($version_flag != 1)
{
  print "(Enter a single period (\".\") on a line to indicate the end of the compiler version information.)\n";
  open (COMPILEFILE, ">>$path/compileInfo.txt") || die "Cannot open file $path/compileInfo.txt";
        
  print COMPILEFILE "#%#START USER COMMENTS#%#\n";
  
  #gather input and append it to compileInfo.txt
  $finished = 0;
  while($finished < 25)
  {
    $userinput = <STDIN>;
    if ($userinput =~ /^.$/)
    {
      $finished = 999;   
    }
    else
    {
      print COMPILEFILE "$userinput";
      $finished++;
    }  
  }
  if ($finished == 25)
  {
    print "Input halted after 25 lines.\n";
  }
  close COMPILEFILE;
}

##############################
# Template Question

open(TPLFILE, ">>$path/tplInfo.txt")|| die "Cannot open $path/tplInfo.txt.  Consolidation halted prematurely\n $!";

print "\nDid you make any changes to the ballista datatype template files?  (These files have the .tpl suffix) <Yes/No>\n";
$tplinput = <STDIN>;

if($tplinput =~ /^YES/i)
{
  print TPLFILE "Template files changed\n";
}
else
{
  print TPLFILE "No template file changes\n";
}
close TPLFILE;

###################################
# user information

open(USERFILE,">>$path/userInfo.txt")|| die "Cannot open $path/userInfo.txt.  Consolidation halted prematurely\n $!";

print "\nName of tester: \n";
$userinput = <STDIN>;
print USERFILE "Tester Name: $userinput";

print "\nName of organization: \n";
$userinput = <STDIN>;
print USERFILE "Organization Name: $userinput";

print "\nLocation: (please include country)\n";
print "(Enter a single period (\".\") on a line to indicate the end of the information.)\n";
print USERFILE "Address:\n";

#gather input and append it to userInfo.txt
$finished = 0;
while($finished < 9)
{
  $userinput = <STDIN>;
  if ($userinput =~ /^.$/)
  {
    $finished = 999;
  }
  else
  {
    print USERFILE "$userinput";
    $finished++;
  }
}

if ($finished == 9)
{
  print "Input halted after 9 lines.\n";
}
 
print "\nThe next two pieces of information: email and phone number are for our internal use only.\n";
print "\nThey will not be posted on the web nor distributed.  Email and phone number allow us to 
contact you if we have questions about your results.\n";
print "\nEmail Address: ";
$userinput = <STDIN>;
print USERFILE "Email Address: $userinput";

print "\nPhone Number: ";
$userinput = <STDIN>;   
print USERFILE "Phone Number: $userinput";

close USERFILE;

###################################
# tar gzip and uuencode the results


if ((system("tar -cf consolidate.tar $path callTable.all outfiles/os_results.txt outfiles/os_results.html outfiles/faillog outfiles/ostest_version.txt configHints.txt"))/256 == 1)
{
  print "Error encountered trying to tar files.  Consolidation incomplete. \n";
  exit(1);
}

# Add template files only if changed.
if($tplinput =~ /^YES/i)
{
  if ((system("tar -uf consolidate.tar templates/*.tpl"))/256 == 1)
  {
    print "Error encountered trying to add templates to tar file.  Consolidation incomplete. \n";
    exit(1);
  }
}

#gzip
if ((system("gzip -f consolidate.tar"))/256 == 1)
{
  print "Error encountered trying to gzip the tar file.  Consolidation incomplete. \n";
  exit(1);
}

#add subject header to uuencode output file
open (UUFILE, ">consolidate.tar.gz.uu") || die "Cannot open file consolidate.tar.gz.uu";
print UUFILE 'Subject: Consolidated Results for Ballista OS Robustness Test Suite';
print UUFILE "\n";
close UUFILE;

#uuencode 
if ((system("uuencode consolidate.tar.gz consolidate.tar.gz >> consolidate.tar.gz.uu"))/256 ==1)
{
  print "Error encountered trying to uuencode the gzipped tar file.  Consolidation incomplete. \n";
  exit(1);
}
print "\nYour results have been tarred, gzipped and uuencoded. \n\n";
print "To actually send your results at the command line please type:\n";
if ($os_name eq linux)
{
  print 'mail -s "Consolidated Results for Ballista OS Robustness Test Suite" ballista@ece.cmu.edu < consolidate.tar.gz.uu'; 
}
else
{
  print 'mail ballista@ece.cmu.edu < consolidate.tar.gz.uu';
}
print "\n\nThank you for consolidating your results.\n";


