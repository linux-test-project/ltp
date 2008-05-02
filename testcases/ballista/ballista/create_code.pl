: # use perl                            -*- mode: Perl; -*-
        eval 'exec perl -S $0 "$@"'
        if $running_under_some_shell;

use 5;

# create_code.pl: produces C++ code that reproduces a Ballista test case - bug reports
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

######################################################################################
#This cgi is given the function name, the paramater names, the
#specific paramater and the number of paramaters, and will generate
#c++ code that will reproduce the code that callGen creates. Note: If
#it takes a long time to generate the code with this cgi, check the
#template file for the datatype and make sure all ] are by themselves
#and flush left. It needs to know where to get the template files
#from.

# 9/14/98 added some include files, including userIncludes.h
#         added type cast to assignment/creation of parameters
#               for compatability with compilers more type-sensitive
#               than g++
# jdevale

# 5/23/99 Fixed a Major Bug:
#
#         The createcode stuff would not insert all the code
#         that applied to all dial settings for example if you had:
#
#         access
#         [
#            {
#              code block A
#            }
#          DIAL1
#            {
#              code block 1
#            }
#            {
#              code block B
#            }
#          DIAL2
#            {
#              code block 2
#            }
#          ]
#
#          Then neither block A or B would be included properly.
#
#          The fix was to essentially rewrite the getstuph function
# 
#          For each block (access, commit, cleanup) it now finds
#          the opening [, and locates and classifies code blocks
#          as being alone (not associated with a dial, and thus
#          applying to all dials) or being with a dial.  Blocks
#          associated with dials are included if specified in the
#          dial list, or skipped (eaten) if not.
#
#
#          Also replaced all ne, eq type string matching with // pattern
#          matching tests to account for whitespace.

# 6/15/99 Charles Shelton
#
# Changed create_code.cgi to a command line script create_code.pl
# This code generates bug reports based on one set of test case parameters
# and outputs to the screen.
#
# Had to modify main because you cannot load a main() function ontp the
# VxWorks target.  Instead, the main function is renamed [function_name]_test
#
# usage: create_code.pl [function_name] [failure type] [number of parameters] \
# [parameter 1 ballista datatype] [parameter 1 test case] ...


# 7/16/99 Charles Shelton
#
# Rewrote major portions of create_code.cgi.  Added comments and eliminated
# debugging statements to make code more readable and less cluttered.
# I redid the template file parsing for parameters to eliminate the problems
# with reading templates that are syntacticly correct but are not formatted
# in a specific style.  Also solves problem of mistaking brackets in an array
# indexing (e.g. int a[6];) for the end of a section block.  Some conventions
# must be preserved, though, such as putting the beginning and ending brackets
# and braces on lines by themselves. 
#

#CGI code for web interface that is no longer needed
#
#require 'cgi-lib.pl';
#print PrintHeader();
#$puter=$ENV{"REMOTE_HOST"};
#@nvpairs=split(/;/, $ENV{'HTTP_COOKIE'});
# foreach $pair (@nvpairs) {
#         ($name, $value) = split(/=/, $pair);
#         $cookie{$name} = $value;
# }
#$user=$cookie{user};
#&ReadParse(*input);

# Function name is first argument in list
$function = @ARGV[0];

# Failure type
$failure = @ARGV[1];

#print "// $failure failure\n";

#For a new datatype follow these instructions:
#put the template file into the directory called templates
#put what the data type maps to in the following hash table
%datatypes=(
	   fptest => "fptemplate.tpl",
	    fp => "fp.tpl",
	    bint => "bint.tpl",
#	    bbasicfloat => "basicfloat.tpl",
            b_ptr_buf => "b_ptr_buf.tpl",
	    bdouble => "bdouble.tpl",
	    bstring => "bstring.tpl",
	    bstr => "bstr.tpl",
	    clockid => "clockid.tpl",
 	    fname => "fname.tpl",
	    ptr => "ptrgentemplate.tpl",
	    size => "size.tpl",
	    buf => "buf.tpl",
	    timeout => "timeout.tpl",
	    string => "string.tpl",
	    dir => "dir.tpl",
	    ptrint => "ptrInt.tpl",
            aioc => "aioc.tpl",
	    fd => "fd.tpl",
	    pid => "pid.tpl",
            sem => "sem.tpl",
	    fmode => "fmode.tpl",
            messageq => "messageq.tpl",
	    mqattrptr => "mqattrptr.tpl",
	    fopenflags => "fopenflags.tpl",
	    schedparamptr => "schedparamptr.tpl",
	    sigset => "sigset.tpl",
	    time_t_ptr => "time_t_ptr.tpl",
	    tm_ptr => "tm_ptr.tpl",
	    ptrrandomaccess => "ptrrandomaccess.tpl",
	    POINTER_GENERIC => "ptrgentemplate.tpl"
	   );

#print "<HEAD></HEAD><BODY><pre><xmp>\n";
#exit(0);

# We need the callTable to generate the code
if(!(-e "./callTable")){
    die("Couldn't find: ./callTable");
}

# Run callGen to generate the code fragment for the function call
system("./callGen $function temp/functioncall.cpp callTable > temp/funny.txt");

open(INFILE2,"<temp/functioncall.cpp")||die("Couldn't open the file!!");
open(OUTFILE, ">temp/output.temp")||die("Couldn't open the file!!");

print OUTFILE "\n\n";
print OUTFILE '/* This code will work for most operating systems using g++ ';
print OUTFILE 'You may have to add a couple #include lines, especially if ';
print OUTFILE 'you are testing your own function */';
print OUTFILE "#include <stdio.h>\n#include <stdlib.h>\n#include <iostream>\n";
#print OUTFILE "#include <unistd.h>\n\n\n";

print OUTFILE '/* Includes from user */';
print OUTFILE "\n";
print OUTFILE "#include \"userIncludes.h\"\n";

@includes=split(/,/,$input{includes});
foreach $incl (@includes) {
    print OUTFILE "#include \"$incl\"\n";
}

# Put datatype names and dial settings for each parameter in two arrays

$num_of_params=@ARGV[2];
for ($i = 0; $i < $num_of_params; $i++) {
    @sparam[$i] = @ARGV[2*$i + 3];
    @sparams[$i] = @ARGV[2*$i+1 + 3];
}

# For each parameter parse it's template file to read code for
# parameter setup and tear down

for ($i = 0; $i < $num_of_params; $i++) {
    #print "template = @sparam[$i].tpl\n";
    open(INFILE,"<templates/@sparam[$i].tpl")||die("couldn't open the file!!!@sparam[$i].tpl");
    while ($#modes > 0){
	pop(@mode);
    }
    
    @mode = split(/lNw/, @sparams[$i]);
    &getstuph;
    close(INFILE);
}

print OUTFILE "\n";
print OUTFILE '/* Includes from paramaters */';
print OUTFILE "\n";

foreach $line (@includes) {
    unless($line !~ "\<") {
	print OUTFILE "$line\n";
    }
}
print OUTFILE "\n";


# Includes in callInclude.cpp from callTable entry for function
$notdone = 1;
while ($notdone) {
    $Line = <INFILE2>;
    unless ($Line !~ "#include") {
	print OUTFILE $Line;
    } else {
	$notdone = 0;
    }
}

print OUTFILE "\n";
print OUTFILE '/*Global defines from the paramaters*/';
print OUTFILE "\n";
foreach $line (@global_defines) {
    print OUTFILE  "$line\n";
}


#create the individual functions to set up the parameters

print OUTFILE "\n";
print OUTFILE  '/* These are the functions used to set up the environment */ ';
print OUTFILE "\n";

# Generate access functions
$j = 0;
for ($i = 0; $i < $num_of_params; $i++) {

    $type = `grep @sparam[$i] dataTypes`;
    ($fred, $realType) = split(/\s+/, $type);
    chomp($realType);
    
    $count = $i + 1;
    print OUTFILE  "\n$realType param$count";
    print OUTFILE  "access()\n{\n";
    
    print OUTFILE  "$realType _theVariable;\n";
    
    while ($access[$j] ne "-+THE_END+-"){
	print OUTFILE  "$access[$j]\n";
	$j++;
    }

    print OUTFILE  "\treturn _theVariable;\n}\n\n";
    $j++;
}


print OUTFILE "\n";
print OUTFILE  '/* These are the functions used to commit the environment*/ ';
print OUTFILE "\n";

# Generate commit functions
$j = 0;
for ($i = 0; $i < $num_of_params; $i++) {
    $count = $i + 1;

    $type = `grep @sparam[$i] dataTypes`;
    ($fred, $realType) = split(/\s+/, $type);
    chomp($realType);
    print OUTFILE  "\nvoid param$count";
    print OUTFILE  "commit($realType & _theVariable){\n";
    
    while ($commit[$j] ne "-+THE_END+-") {
	print OUTFILE  "$commit[$j]\n";
	$j++;
    }

    print OUTFILE  "\n}\n\n";
    $j++;
}


print OUTFILE "\n";
print OUTFILE  '/*These are the functions used to destroy the environment */ ';
print OUTFILE "\n";

# Generate cleanup functions
$j = 0;
for ($i = 0; $i < $num_of_params; $i++) {
    $count = $i + 1;

    $type = `grep @sparam[$i] dataTypes`;
    ($fred, $realType) = split(/\s+/, $type);
    chomp($realType);
    print OUTFILE  "\nvoid param$count";
    print OUTFILE  "cleanup($realType & _theVariable){\n";
    
    while ($cleanup[$j] ne "-+THE_END+-") {
	print OUTFILE  "$cleanup[$j]\n";
	$j++;
    }
    
    print OUTFILE  "\n}\n\n";
    $j++;
}


#print OUTFILE  "int ";
#print OUTFILE  $function;
#print OUTFILE  "_test(){\n";

print OUTFILE "int main(){\n";

print OUTFILE "#include \"userSetup.cpp\"\n";
print OUTFILE $Line;

#access phase

$j = 1;
for ($i = 0; $i < $num_of_params; $i++) {
    $notdone = 1;
    while ($notdone) {
	$Line = <INFILE2>;

	if ($Line =~ "temp") {
	    $Line =~ /=/;
	    $begin = $`;

	    # create casting code

	    @ary = split(' ', $begin);

	    $end = "= (@ary[0])param$j";
	    $endend = "access();\n";

	    print OUTFILE  "$begin$end$endend";
	    $notdone = 0;
	    $j++;
	} 
    }
}

#commit phase
#print "Commit\n";

$j = 1;
for ($i = 0; $i < $num_of_params; $i++) {
    $notdone = 1;
    while ($notdone) {
	$Line = <INFILE2>;

	if ($Line = ~'commit') {
            # typecast parameter to avoid compiler errors

            $k = $j - 1;
            #$parma = "sparam$k";
            $type = `grep @sparam[$k] dataTypes`;
            ($fred, $realType) = split(/\s+/, $type);
            chomp($realType);

            print OUTFILE  "param$j";
            print OUTFILE  "commit(($realType &) temp$j);\n";
            $notdone = 0;
            $j++;
	} else {
	    print OUTFILE  "$Line";
	}
    }
}

$notdone = 1;
while ($notdone) {
    $Line = <INFILE2>;
    unless ($Line =~ '#include') {
	print OUTFILE  $Line;
    } else {
	$notdone = 0;
    }
}

$j = 1;
for ($i = 0; $i < $num_of_params; $i++) {
    $notdone = 1;
    while($notdone){
	$Line = <INFILE2>;

	if ($Line = ~'cleanup') {
            # typecast parameter to avoid compiler errors

            $k = $j - 1;
            $parma = "sparam$k";
            $type = `grep @sparam[$k] dataTypes`;
            ($fred, $realType) = split(/\s+/, $type);
            chomp($realType);

            print OUTFILE  "param$j";
            print OUTFILE  "cleanup(($realType &) temp$j);\n";
            $notdone = 0;
            $j++;
	} else {
	    print OUTFILE  "$Line";
	}
    }
}

print OUTFILE "#include \"userShutdown.cpp\"\n";

close(INFILE2);
close(INFILE);

#print "$path2$puter";
#system("rm -f $path2$puter");	

print OUTFILE  "\nreturn 0;\n";
print OUTFILE  "}\n";
close OUTFILE;

# Indent code so it looks readable and nice
system("indent -l65 -fca -c25 -nbc -br -bad -bap -sob -fc1 -sc -ce -lp temp/output.temp -o temp/output.temp2");

open(BLAH,"<temp/output.temp2");
while(<BLAH>){
    print $_;
}

# Clean up extra files
#system("rm -f temp/output.temp");
#system("rm -f temp/output.temp2");
#system("rm -f temp/functioncall.cpp");
#system("rm -f temp/funny.txt");
#close CB;

#print "\n</xmp></pre></BODY></HTML>\n";
		       
exit(0);


# Subroutine getstuph reads the template file to get the appropriate setup
# and teardown code for the particular function parameters

sub getstuph {
  @includes;
  @global_defines;
  @access;
  @commit;
  @cleanup;

  # Get the includes;

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*includes.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  while ($Line !~ /.*\[.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  @temp_block = &get_block;

  foreach $block_line (@temp_block) {
      $add = 1;
      foreach $tempLine (@includes) {
	  if ($tempLine eq $block_line) {
	      $add = 0;
	  }
      }
      if ($add) {
	  push(@includes, $block_line);
      }
  }

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*\].*/) {
      $Line = <INFILE>;
      chomp($Line);
  }


  # Get the global_defines

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*global_defines.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  while ($Line !~ /.*\[.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  @temp_block = &get_block;

  foreach $block_line (@temp_block) {
      $add = 1;
      foreach $tempLine (@global_defines) {
	  if ($tempLine eq $block_line) {
	      $add = 0;
	  }
      }
      if ($add) {
	  push(@global_defines, $block_line);
      }
  }

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*\].*/) {
      $Line = <INFILE>;
      chomp($Line);
  }


  # Get the access

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*access.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  @temp_access = &get_section;
  foreach $temp_line (@temp_access) {
      push(@access, $temp_line);
  }


  # Get the commit

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*commit.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  @temp_commit = &get_section;
  foreach $temp_line (@temp_commit) {
      push(@commit, $temp_line);
  }


  # Get the cleanup

  $Line = <INFILE>;
  chomp($Line);
  while ($Line !~ /.*cleanup.*/) {
      $Line = <INFILE>;
      chomp($Line);
  }

  @temp_cleanup = &get_section;
  foreach $temp_line (@temp_cleanup) {
      push(@cleanup, $temp_line);
  }

}


# subroutine get_block will read one block of code between braces,
# accounting for all inner blocks

sub get_block {
    my @block_array;
    my $brace_count = 0;

    # search for line with first open brace

    while ($Line !~ /.*\{.*/) {
     	$Line = <INFILE>;
	chomp($Line);
    }

    # if there is code on the line after the brace, save it
    # and push it onto the array

    if ($Line =~ /\.*\{\s*\S+/) {
	@temp_array = split '{', $Line;
	shift @temp_array;
	$Line = join '{', @temp_array;

	# if there are other opening braces on the first line after
	# the first one, count them too

	$brace_count += @temp_array - 1;
	push (@block_array, $Line);
    }

    $brace_count++;

    # loop and read all input lines from input file until final
    # ending brace is found for this block

    while ($brace_count > 0) {
	$Line = <INFILE>;
	chomp($Line);

	if ($Line =~ /.*\{.*/) {
	    $brace_count++;
	}

	if ($Line =~ /.*\}.*/) {
	    $brace_count--;
	}

	# NOTE: The final ending brace must be on a line by itself
	# since it won't be put in the block_array

	if ($brace_count > 0) {
	    push (@block_array, $Line);
	}
    }

    return @block_array;
}


# Subroutine get_section will read one section from the template file,
# either the access, commit, or cleanup format.

sub get_section {
    my @section_array;
    my @temp_block;

    # Keep reading until the first open bracket is found, signaling the
    # beginning of the block
    # NOTE: the first open bracket must be on a line by itself and will not
    # be put into the output array

    while ($Line !~ /.*\[.*/) {
	$Line = <INFILE>;
	chomp($Line);
    }

    $notdone=1;

    while ($notdone) {
	$Line = <INFILE>;
	chomp($Line);

	if ($Line =~ /\s*\S+/) {

	    if ($Line =~ /\s*\{\s*/) {
		
		# Get any code blocks not associated with a dial
		
		@temp_block = &get_block;
		foreach $temp_line (@temp_block) {
		    push(@section_array, $temp_line);
		}
	    } elsif ($Line =~ /\s*\]\s*/) {

		# If closing bracket is found on a line by itself,
		# that signals the end of the section
		
		$notdone = 0;
	    } else {
		
		# If the line matches a dial setting we're looking for,
		# get the code block associated with it
		
		$mode_match = 0;
		foreach $Mode (@mode) {
		    if ($Line =~ /\b$Mode\b/) {
			$mode_match = 1;
			@temp_block = &get_block;

			foreach $temp_line (@temp_block) {
			    push(@section_array, $temp_line);
			}
		    }
		}

		# read and throw away code for unwanted dial settings

		if (!$mode_match) {
		    &get_block;
		}
	    }
	}
    }

    push(@section_array, "-+THE_END+-");
    return @section_array;
}
