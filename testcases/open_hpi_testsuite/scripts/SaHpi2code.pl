#!/usr/bin/perl

#######################################################################
# (C) COPYRIGHT IBM Corp 2004
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
#  file and program are licensed under a BSD style license.  See
#  the Copying file included with the OpenHPI distribution for
#  full licensing terms.
#
#  Author(s):
#       Steve Sherman <stevees@us.ibm.com>
########################################################################

########################################################################
# Script Description:
#
# This script reads SaHpi.h and generates the C code necessary to 
# print the string definitions of the SaHpi.h's enum typedefs (and some
# #defines). The idea is to automatically generate and centralize these
# utility functions for use in OpenHPI client and testing programs. 
#
# These utility functions can also be used to determine if an arbitrary
# value is valid for a given enum typedef (a NULL return code indicates
# an invalid value).
# 
# Script also generates code to support SaHpi_event_utils.c routines.
#
# This script is run as part of the OpenHPI build process.
#
# Script Input:
#
# --debug     (optional)   Turn on debug info.
#                          Default is no.
# --ifile     (optional)   Full path name to header file, including the
#                          header's filename.
#                          Default is "current_directory/SaHpi.h".
# --atca      (optional)   Auto-generating should take account of additional
#                          ATCA spec (SaHpiAtac.h).
# --odir      (optional)   Directory for generated output files.
#                          Default is current directory.
# --tdir      (optional)   Directory for generated testcase files.
#                          If not defined; no testcase files are generated;
# Exit codes
# - 0 successful
# - 1 error occurred
#########################################################################

use strict;
use Getopt::Long;

GetOptions(
  "debug"   => \my $debug,
  "atca"    => \my $atca,
  "ifile=s" => \my $ifile,
  "odir=s"  => \my $odir,
  "tdir=s"  => \my $tdir,
);

sub normalize_file(*);
sub parse_category_events(*$);
sub print_copywrite(*);
sub print_hfile_header(*$);
sub print_cfile_header;
sub print_xfile_header;
sub print_xhfile_header;
sub print_testfile_header;
sub print_xtestfile_header; 
sub beautify_type_name($);
sub print_hfile_func($$);
sub print_cfile_func($);
sub beautify_enum_name($$);
sub print_cfile_case($$);
sub print_cfile_encode_array_header($);
sub print_cfile_encode_array($$);
sub print_cfile_encode_func($);
sub print_testfile_case($$);
sub print_xtestfile_case($$$);
sub print_xtestfile_badevent($);
sub print_cfile_endfunc;
sub print_testfile_endfunc($);
sub print_hfile_ending(*);
sub print_testfile_ending;
sub print_xtestfile_ending;

######################################
# Main Code - Set directory/file names
######################################

my $cur_dir = `pwd`;
chomp $cur_dir;

if ($ifile eq "") {
   $ifile = $cur_dir . "/SaHpi.h";
}

if ($odir eq "") {
   $odir = $cur_dir;
}

#if ($oname eq "") {
my ($dir, $base, $ext) = ($ifile =~ m:(.*)/([^/]+)\.([^\./]+)$:);
($base, $ext) = ($ifile =~ m:^([^/]+)\.([^\./]+)$:) unless $base;
die "cannot find base for file $ifile" unless $base;

$base = lc($base);
my $ocfile = $base . "_enum_utils.c";
my $ohfile = $base . "_enum_utils.h";
my $oxcfile = $base . "_event_encode.c";
my $oxhfile = $base . "_event_encode.h";
#}
#else {
#    $ocfile = $oname . ".c";
#    $ohfile = $oname . ".h";
#}

my $file_c  = $odir . "/$ocfile";
my $file_h  = $odir . "/$ohfile";
my $file_x  = $odir . "/$oxcfile";
my $file_xh = $odir . "/$oxhfile";

my ($atca_base) = "sahpiatca";
my ($atca_header) = 0;
if ($base eq $atca_base) {
    $atca_header = 1;
}

unlink $file_c;
unlink $file_h;
if (!$atca_header) {
    unlink $file_x;
    unlink $file_xh;
}
open(INPUT_HEADER, $ifile) or die "$0 Error! Cannot open $ifile. $! Stopped";
open(FILE_C, ">>$file_c") or die "$0 Error! Cannot open file $file_c. $! Stopped";
open(FILE_H, ">>$file_h") or die "$0 Error! Cannot open file $file_h. $! Stopped";
if (!$atca_header) {
    open(FILE_X, ">>$file_x") or die "$0 Error! Cannot open file $file_x. $! Stopped";
    open(FILE_XH, ">>$file_xh") or die "$0 Error! Cannot open file $file_xh. $! Stopped";
}

my $tbase_name = "$ocfile";
my $txbase_name = "$oxcfile";
$tbase_name =~ s/\.c$/_test\.c/;
$txbase_name =~ s/\.c$/_test\.c/;
my $tfile = "$tdir/$tbase_name";
my $txfile = "$tdir/$txbase_name";

if ($tdir) {
    unlink $tfile;
    open(FILE_TEST, ">>$tfile") or die "$0 Error! Cannot open file $tfile. $! Stopped";
    if (!$atca_header) {
        unlink $txfile;
        open(XFILE_TEST, ">>$txfile") or die "$0 Error! Cannot open file $txfile. $! Stopped";
    }
}

#########################
# Parse input header file
#########################

my $rtn_code = 0;

my $cat_type = "SaHpiEventCategoryT";
my $err_type = "SaErrorT";
my $entity_type = "SaHpiEntityTypeT";
my $atca_entity_type = "AtcaHpiEntityTypeT";

my $atca_pretty_type = beautify_type_name($atca_entity_type);
my $atca_entity_encode_name = "oh_encode_" . "$atca_pretty_type";
my $atca_entity_lookup_name = "oh_lookup_" . "$atca_pretty_type";

my @cat_array = ();
my @err_array = ();
my @atca_entity_array = ();
my @enum_array = ();
my @normalized_array = ();

my %category = ();
my %global_category = ();

if (normalize_file INPUT_HEADER) { $rtn_code = 1; goto CLEANUP; }

print_copywrite *FILE_C;
print_copywrite *FILE_H;
if (!$atca_header) {
    print_copywrite *FILE_X;
    print_copywrite *FILE_XH;
}
if ($tdir) { 
    print_copywrite *FILE_TEST;
    if (!$atca_header) {
        print_copywrite *XFILE_TEST;
    }
}

print_hfile_header *FILE_H, $ohfile;
print_cfile_header();
if (!$atca_header) {
    print_hfile_header *FILE_XH, $oxhfile;
    print_xfile_header();
    print_xhfile_header();
}
if ($tdir) { 
    print_testfile_header();
    if (!$atca_header) {
        print_xtestfile_header();
    }
}

my $in_enum = 0;
my $line_count = 0;
my $max_global_events = 0;
my $max_events = 0;


for ( my ($i) = 0; $i < @normalized_array; ++$i ) {
    $_ = $normalized_array[ $i ];

    # Handle SaErrorT definitions
    my ($err_code) = /^\s*\#define\s+(\w+)\s*\($err_type\).*$/;
    if ($err_code) {
        push(@err_array, $err_code);
	next;
    }

    if (/^\s*\($atca_entity_type\).*$/) {
    	$_ = @normalized_array[ $i - 1 ];
	my ($atca_entity_code) = /^\s*\#define\s+(\w+).*$/;
        push(@atca_entity_array, $atca_entity_code);
	next;
    }

    # Handle SaHpiEventCategoryT definitions
    my ($cat_code) = /^\s*\#define\s+(\w*)\s*\($cat_type\).*$/;
    if ($cat_code) {
        push(@cat_array, $cat_code);
	next;
    }

    if ( /^\s*typedef\s+enum.*$/ ) {
	$in_enum = 1;
	next;
    }
	 
    if ($in_enum) {
	# Check for end of enum definition - Assumming all on one line
	my ($enum_end, $enum_type) = /^\s*(\}+)\s*(\w*)\s*\;\s*$/;
	if ( $enum_end ne "" ) {
	    $in_enum = 0;
	    $line_count++;
	    print_cfile_func($enum_type);
	    my $max_enums = 0;
	    foreach my $case (@enum_array) {
		$max_enums++;
		print_cfile_case($enum_type, $case);
		if ($tdir) { print_testfile_case($enum_type, $case); }
	    }
	    print_cfile_endfunc($enum_type);

	    # Create encoding code
	    print_cfile_encode_array_header($enum_type);
	    foreach my $case (@enum_array) {
		print_cfile_encode_array($enum_type, $case);
	    }
	    print FILE_C "};\n\n";
	    print_cfile_encode_func($enum_type);

	    if ($tdir) { print_testfile_endfunc($enum_type); }
	    print_hfile_func($enum_type, $max_enums);
	    @enum_array = ();
	    next;
	}

	# Find enum definition - sometimes "{" is on the next line
	my ($enum_def) = /^\s*\{*\s*(\w+).*$/;
	if ($enum_def) {
	    push(@enum_array, $enum_def);
	}		     
    }
}

if ($in_enum) { die "$0 Error! Open enum definition. $! Stopped"; }
if ($#err_array > 0) {
    my $max_enums = 0;
    $line_count++;
    print_cfile_func($err_type);
    foreach my $case (@err_array) {
	$max_enums++;
	print_cfile_case($err_type, $case);
	if ($tdir) { print_testfile_case($err_type, $case); }
    }
    print_cfile_endfunc();

    # Create encode function
    print_cfile_encode_array_header($err_type);
    foreach my $case (@err_array) {
	print_cfile_encode_array($err_type, $case);
    }
    print FILE_C "};\n\n";
    print_cfile_encode_func($err_type);

    if ($tdir) { print_testfile_endfunc($err_type); }
    print_hfile_func($err_type, $max_enums);
}

if (($#cat_array > 0) && (!$atca_header)) {
    my $max_enums = 0;
    $line_count++;
    print_cfile_func($cat_type);
    foreach my $case (@cat_array) {
	$max_enums++;
	print_cfile_case($cat_type, $case);
	if ($tdir) { 
	    print_testfile_case($cat_type, $case); 
	}
    }
    print_cfile_endfunc();

    # Create encode function
    print_cfile_encode_array_header($cat_type);
    foreach my $case (@cat_array) {
	print_cfile_encode_array($cat_type, $case);
    }
    print FILE_C "};\n\n";
    print_cfile_encode_func($cat_type);

    if ($tdir) { print_testfile_endfunc($cat_type); }
    print_hfile_func($cat_type, $max_enums);
}

if ($#atca_entity_array > 0) {
    my $max_enums = 0;
    $line_count++;
    print_cfile_func($atca_entity_type);
    foreach my $case (@atca_entity_array) {
	$max_enums++;
	print_cfile_case($atca_entity_type, $case);
	if ($tdir) { 
	    print_testfile_case($atca_entity_type, $case); 
	}
    }
    print_cfile_endfunc();

    # Create encode function
    print_cfile_encode_array_header($atca_entity_type);
    foreach my $case (@atca_entity_array) {
	print_cfile_encode_array($atca_entity_type, $case);
    }
    print FILE_C "};\n\n";
    print_cfile_encode_func($atca_entity_type);

    if ($tdir) { print_testfile_endfunc($atca_entity_type); }
    print_hfile_func($atca_entity_type, $max_enums);
}

####################################
# Handle event states and categories 
####################################

if (!$atca_header) {
print FILE_X "oh_categorystate_map state_global_strings[] = {\n";
foreach my $gc (keys %global_category) {
    foreach my $gevt (sort {$global_category{$gc}->{$a}->{value} <=>
			     $global_category{$gc}->{$b}->{value}} keys %{$global_category{$gc}}) {
	$max_global_events++;
	if ($debug) { print("CAT=$gc; EVENT=$gevt; STR=$global_category{$gc}->{$gevt}->{string}\n"); }
	print FILE_X "{SAHPI_EC_UNSPECIFIED, $gevt, \"$global_category{$gc}->{$gevt}->{string}\"},\n";
	if ($tdir) { 
	    print_xtestfile_case($gevt, "SAHPI_EC_UNSPECIFIED", $global_category{$gc}->{$gevt}->{string});
	}
    }
    print_xtestfile_badevent("SAHPI_EC_UNSPECIFIED");
}
print FILE_X "};\n\n";

print FILE_XH "\#define OH_MAX_STATE_GLOBAL_STRINGS $max_global_events\n";
print FILE_XH "extern oh_categorystate_map state_global_strings[OH_MAX_STATE_GLOBAL_STRINGS];\n\n";

print FILE_X "oh_categorystate_map state_strings[] = {\n";
foreach my $c (keys %category) {
    foreach my $evt (sort {$category{$c}->{$a}->{value} <=>
			    $category{$c}->{$b}->{value}} keys %{$category{$c}}) {
        $max_events++;
	if ($debug) { print("CAT=$c; EVENT=$evt; STR=$category{$c}->{$evt}->{string}\n"); }
	print FILE_X "{$c, $evt, \"$category{$c}->{$evt}->{string}\"},\n";
	if ($tdir) { 
	    print_xtestfile_case($evt, $c, $category{$c}->{$evt}->{string});
	}
    }
    print_xtestfile_badevent($c);
}
print FILE_X "};\n\n";

print FILE_XH "\#define OH_MAX_STATE_STRINGS $max_events\n";
print FILE_XH "extern oh_categorystate_map state_strings[OH_MAX_STATE_STRINGS];\n\n";

print_hfile_ending *FILE_XH;
}
print_hfile_ending *FILE_H;

if ($tdir) { 
    print_testfile_ending();
    if (!$atca_header) {
        print_xtestfile_ending();
    }
}

CLEANUP:
close INPUT_HEADER;
close FILE_C;
close FILE_H;
if (!$atca_header) {
    close FILE_X;
    close FILE_XH;
}
if ($tdir) { 
    close FILE_TEST;
    if (!$atca_header) {
        close XFILE_TEST;
    }
}

if ($line_count == 0) {
    print "$0 Warning! No code can be generated from header file - $ifile\n";
    unlink $file_c;
    unlink $file_h;
    if ($tdir) { 
	unlink $tfile;
    }
}

if (($max_events == 0) && (!$atca_header)) {
    print "$0 Warning! No Events found in header file - $ifile\n";
    if ($tdir) { 
	rm $txfile;
    }
}

exit ($rtn_code);

#############
# Subroutines
#############
sub normalize_file(*) {
    my( $input_handle ) = @_;
    my $in_comments = 0;
    my $in_cat = 0;

    while ( <$input_handle> ) {
	chomp;
	
	# Handle special case for Event Categories and their states
	if (/^.*\s+SaHpiEventCategoryT\s*==.*$/ ) {
	    parse_category_events($input_handle, $_);
	}

	next if /^\s*$/;               # Skip blank lines
	next if /^\s*\/\/.*$/;         # Skip // lines
	next if /^\s*\/\*.*\*\/\s*$/;  # Skip /* ... */ lines
	
	my $line = $_;

	($line) =~ s/^(.*?)\s*$/$1/;    # Strip blanks from end of line
	($line) =~ s/^(.*?)\/\/.*$/$1/; # Strip trailing C++ comments

        # Multi-line C comments
	if ( ( /^.*\/\*.*$/ ) && !( /^.*\*\/.*$/ ) ) {  
	    $in_comments = 1;
	    ($line) =~ s/^(.*?)\/\*.*$/$1/; # Get part of line before comment
	    chomp $line;
	    if ($line ne "") {
		push(@normalized_array, $line);
	    }
	    if ($debug) {
		print "In multi-line comment section\n";
		print "Characters before first comment = $line\n";
	    }
	    next;
	}

	# End C comment
	if ( !( /^.*\/\*.*$/ ) && ( /^.*\*\/.*$/ ) ) {
	    $in_comments = 0;
	    ($line) =~ s/^.*\*\/(.*?)$/$1/; # Get part of line after comment
	    if ($debug) { 
		print "Out of multi-line comment section\n";
		print "Characters after last comment = $line\n";
	    }
	}

	# Embedded single line comment after C code
	if ( ( /^.*\/\*.*$/ ) && ( /^.*\*\/.*$/ ) ) {
	    my ($token1, $comment, $token2) = /^(.*)(\/\*.*\*\/)+(.*)$/;
	    $line = $token1 . "\n$token2";
	    if ($debug) { 
		print "Embedded single line comment\n";
		print "Line without comment = $line\n";
	    }
	}

	next if ($in_comments);
	chomp $line;
	next if ($line eq "");

        # Change commas to NLs
	$line =~ s/,/\n/g;
	my @fields = split/\n/,$line;
	foreach my $field (@fields) {
	    chomp $field;
	    push(@normalized_array, $field);
	}
    }

    return 0;
}

sub parse_category_events(*$) {
    my ($file_handle, $line) = @_;

    my $in_global_cat = 0;
    my @global_events = ();
    my @cat_list = ();

    if (/\<any\>/) {
	$in_global_cat = 1;
    }
    else {
	my ($cat, $rest_of_line) = /^.*SaHpiEventCategoryT\s*==\s*(\w+)\s+(.*)$/;
	if ($debug) { print("CAT=$cat\n"); }
	push(@cat_list, $cat);
	# Handle multiple categories || together
	while ($rest_of_line =~ /\|\|/) {
	    $rest_of_line =~ s/^\|\|//; # Strip off beginning || 
	    my ($cat, $rol) = ($rest_of_line =~ /\s*(\w+)\s+(.*)$/);
	    $rest_of_line = $rol;
	    if ($debug) { print("CAT=$cat\n"); }
	    push(@cat_list, $cat);
	}
    }
    
    # Find events - assume blank lines end #define section; but support 
    # line continuation characters
    while (($line = <$file_handle>) !~ /^\s*$/) {
	my ($event_state, $event_hex) = ($line =~ /^\s*\#define\s+(\w+)\s+.*?(0x\w+)\s*$/);
	if ($event_state eq "") {
	    ($event_state) = ($line =~ /^\s*\#define\s+(\w+)\s+\\\s*$/);
	    if ($event_state) {
		$line = <$file_handle>;
		($event_hex) = ($line =~ /^\s*.*?(0x\w+)\s*$/);
		die "Cannot parse continuation event line" unless ($event_hex);
	    }
        }

	# Push event state definition to global hashes   
	if ($event_state && $event_hex) {
	    my $str = $event_state;
	    $str =~ s/SAHPI_ES_//;
	    if ($in_global_cat) {
		$global_category{"ANY"}->{$event_state}->{value} = hex($event_hex);
		$global_category{"ANY"}->{$event_state}->{string} = $str;
	    }
	    else {
		foreach my $cat (@cat_list) {
		    if ($debug) {
			print("CAT=$cat; EVENT STATE=$event_state; HEX=$event_hex; STR=$str x\n");
		    }
		    $category{$cat}->{$event_state}->{value} = hex($event_hex);
		    $category{$cat}->{$event_state}->{string} = $str;
		}
	    }
	}
    }

    return 0;
}

####################################
# Print h file's static leading text 
####################################
sub print_copywrite(*) {
    my ( $file_handle ) = @_;

    print $file_handle <<EOF;
/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees\@us.ibm.com> 
 */

/******************************************************************* 
 * WARNING! This file is auto-magically generated by:
 *          $0. 
 *          Do not change this file manually. Update script instead
 *******************************************************************/

EOF

    return 0;
}

#######################################
# Print header file static leading text 
#######################################
sub print_hfile_header(*$) {
    my ( $filehandle, $filename ) = @_;

    my $hdef_name = $filename;
    $hdef_name =~ tr/a-z/A-Z/;
    $hdef_name =~ s/\./_/g;	

    print $filehandle <<EOF;
#ifndef __$hdef_name
#define __$hdef_name

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#ifdef __cplusplus
extern "C" {
#endif 

EOF

    return 0;
}

####################################
# Print c file's static leading text 
####################################
sub print_cfile_header {

    print FILE_C <<EOF;
#include <strings.h>

#include <SaHpi.h>
#include <oh_utils.h>

EOF
    return 0;
}

####################################
# Print xfile's static leading text 
####################################
sub print_xfile_header {

    print FILE_X <<EOF;
#include <SaHpi.h>
#include <oh_utils.h>

EOF

    return 0;
}

##########################################
# Print xfile header's static leading text 
##########################################
sub print_xhfile_header {

    print FILE_XH <<EOF;
#define OH_ENCODE_DELIMITER " | "
#define OH_ENCODE_DELIMITER_CHAR "|"
#define OH_ENCODE_DELIMITER_LENGTH 3

typedef struct {
    SaHpiEventCategoryT category;
    SaHpiEventStateT state;
    char *str;
} oh_categorystate_map;

EOF
    return 0;
}

###########################################
# Print testcase file's static leading text 
###########################################
sub print_testfile_header {

    print FILE_TEST <<EOF;
#include <stdio.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

#define BAD_ENUM_VALUE -1

int main(int argc, char **argv) 
{
        char *expected_str;
        char *str;

EOF
    return 0;
}

############################################
# Print xtestcase file's static leading text 
############################################
sub print_xtestfile_header {

    print XFILE_TEST <<EOF;
#include <stdio.h>
#include <string.h>

#include <SaHpi.h>
#include <oh_utils.h>

int main(int argc, char **argv) 
{
        char *expected_str;
        SaErrorT err;
        SaHpiEventStateT event_state, expected_state;
        SaHpiEventCategoryT event_cat, expected_cat;
        SaHpiTextBufferT buffer;

        #define BAD_EVENT 0xFFFF

EOF
    return 0;
}

##############################
# Beautify SaHpi typedef names
##############################
sub beautify_type_name($) {
    my ($name) = @_;
    $name =~ s/^Sa//;   # Strip off beginning Sa - for SaErrorT
    $name =~ s/^Hpi//;  # Strip off beginning Hpi - for rest of SaHpi types
    $name =~ s/T$//;    # Strip off trailing T
    $name = lc($name);  # Lower case name
    return $name;
}

###############################
# Print h file's func prototype 
###############################
sub print_hfile_func($$) {
    my( $type, $max ) = @_;
    
    my $pretty_type = beautify_type_name($type);
    my $lookup_name = "oh_lookup_" . "$pretty_type";
    my $encode_name = "oh_encode_" . "$pretty_type";
    my $map_name = "oh_" . "$pretty_type" . "_map";
    my $upper_pretty_type = uc($pretty_type);
    my $max_name = "OH_MAX_" . "$upper_pretty_type";
    my $array_name = "$pretty_type" . "_strings[$max_name]";

    print FILE_H <<EOF;
#define $max_name $max 
extern struct $map_name {
  $type  entity_type;
  char *str;
} $array_name;

char * $lookup_name($type value);
SaErrorT $encode_name(SaHpiTextBufferT *buffer, $type *type);

EOF

    return 0;
}

###########################
# Print c file's func start 
###########################
sub print_cfile_func($) {
    my( $type ) = @_;

    my $pretty_type = beautify_type_name($type);
    my $lookup_name = "oh_lookup_" . "$pretty_type";
 
    print FILE_C <<EOF;
/**
 * $lookup_name:
 * \@value: enum value of type $type.
 *
 * Converts \@value into a string based on \@value\'s HPI enum definition.
 * 
 * Returns:
 * string - normal operation.
 * NULL - if \@value not a valid $type.
 **/

char * $lookup_name($type value)
{
        switch (value) {
EOF
    
    return 0;
}

############################################
# Beautify enum string from SaHpi definition
# This is highly SaHpi.h dependent.
############################################
sub beautify_enum_name($$) {
    my ($enum_type, $enum_name) = @_;
    if ($debug) { print("TYPE=$enum_type: ENUM=$enum_name: "); }
    
    if ($enum_type eq "SaHpiStatusCondTypeT" ||
        $enum_type eq "SaHpiSensorReadingTypeT") 
      {
          $enum_name =~ s/(\s*[A-Za-z]+_{1}){4}//;
      }
    
    elsif ($enum_type eq "SaErrorT" ||
           $enum_type eq "SaHpiAnnunciatorModeT" ||
           $enum_type eq "SaHpiAnnunciatorTypeT" ||
           $enum_type eq "SaHpiDomainEventTypeT" ||
           $enum_type eq "SaHpiCtrlStateDigitalT" ||
           $enum_type eq "SaHpiCtrlModeT" ||
           $enum_type eq "SaHpiCtrlTypeT" ||
           $enum_type eq "SaHpiIdrAreaTypeT" ||
           $enum_type eq "SaHpiIdrFieldTypeT" ||
           $enum_type eq "SaHpiHsActionT" ||
           $enum_type eq "SaHpiHsIndicatorStateT" ||
           $enum_type eq "SaHpiHsStateT" ||
           $enum_type eq "SaHpiResourceEventTypeT" ||
           $enum_type eq "SaHpiTextTypeT") 
      {
          $enum_name =~ s/(\s*[A-Za-z]+_{1}){3}//;
      } 
    
    elsif ($enum_type eq "SaHpiParmActionT" ||
           $enum_type eq "SaHpiRdrTypeT" ||
           $enum_type eq "SaHpiResetActionT" ||
           $enum_type eq "SaHpiSensorTypeT" ||
           $enum_type eq "SaHpiSeverityT") 
      {
          $enum_name =~ s/\s*SAHPI_//;
      } 
    
    elsif ($enum_type eq "SaHpiSensorUnitsT")
      {
          $enum_name =~ s/(\s*[A-Za-z]+_{1}){2}//;
          $enum_name =~ s/_/ /g;
          $enum_name = lc($enum_name);
          $enum_name =~ s/\b(\w)/\U$1\E/g;
      }
    else 
      {
          $enum_name =~ s/(\s*[A-Za-z]+_{1}){2}//;	
      }
    
    if ($debug) { print("STR=$enum_name\n"); }
    
    return $enum_name;
}

###############################
# Print c file's case statement 
###############################
sub print_cfile_case($$) {
    my( $type, $case ) = @_;
    my $casestr = beautify_enum_name($type, $case);

    print FILE_C <<EOF;
        case $case:
                return \"$casestr\";
EOF

    return 0;
}

####################################
# Print c file's encode array header
####################################
sub print_cfile_encode_array_header($) {
    my( $type ) = @_;
    my $pretty_type = beautify_type_name($type);
    my $map_name = "oh_" . "$pretty_type" . "_map";
    my $array_name = "$pretty_type" . "_strings[]";"";

    print FILE_C <<EOF;
struct $map_name $array_name = {
EOF

    return 0;
}

####################################
# Print c file's encode array member
####################################
sub print_cfile_encode_array($$) {
    my( $type, $case ) = @_;

    my $casestr = beautify_enum_name($type, $case);

    print FILE_C <<EOF;
       {$case, \"$casestr\"},
EOF

    return 0;
}

################################
# Print c file's encode function
################################
sub print_cfile_encode_func($) {
    my( $type ) = @_;
    my $pretty_type = beautify_type_name($type);
    my $encode_name = "oh_encode_" . "$pretty_type";
    my $lookup_name = "oh_lookup_" . "$pretty_type";
    my $upper_pretty_type = uc($pretty_type);
    my $max_name = "OH_MAX_" . "$upper_pretty_type";
    my $array_member = "$pretty_type" . "_strings[i]";
    my $ret = "SA_ERR_HPI_INVALID_DATA";

    if (($type eq $entity_type) && $atca ) {
    	$ret = "$atca_entity_encode_name(buffer, type)";
    }

    print FILE_C <<EOF;
/**
 * $encode_name:
 * \@buffer: Pointer to SaHpiTextBufferT that contains enum\'s string representation.
 * \@type: Location (of $type) to place encoded result.
 * 
 * Converts a \@buffer->Data string, generated by $lookup_name(), back 
 * into an $type type. 
 *
 * Returns:
 * $type value - normal operation.
 * SA_ERR_HPI_INVALID_PARAMS - if \@buffer or \@type is NULL or \@buffer->Data empty.
 * SA_ERR_HPI_INVALID_DATA - if \@buffer->Data is invalid.
 **/
SaErrorT $encode_name(SaHpiTextBufferT *buffer, $type *type)
{
	int i, found;

	if (!buffer || !type || buffer->Data == NULL || buffer->Data[0] == \'\\0\') {
		return(SA_ERR_HPI_INVALID_PARAMS);
	}
	
	found = 0;
	for (i=0; i<$max_name; i++) {
		if (strcasecmp((char *)buffer->Data, $array_member.str) == 0) {
			found++;
			break;
		}
	}

	if (found) {
		*type = $array_member.entity_type;
	}
	else {
		return($ret);
	}
	
	return(SA_OK);
}

EOF

    return 0;
}

################################
# Print testcase file's testcase
################################
sub print_testfile_case($$) {
    my( $type, $case ) = @_;
    my $pretty_type = beautify_type_name($type);
    my $lookup_name = "oh_lookup_" . "$pretty_type";
    my $encode_name = "oh_encode_" . "$pretty_type";
    my $casestr = beautify_enum_name($type, $case);

    print FILE_TEST <<EOF;
        /* $type - $case testcase */
        {
	        $type value = $case;
		$type enum_type;
                expected_str = "$casestr";
		SaErrorT err;
		SaHpiTextBufferT buffer;

                str = $lookup_name(value);
                if (strcmp(expected_str, str)) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
			printf("  Received string=%s\\n", str);
			printf("  Expected string=%s\\n", expected_str);
                        return -1;             
                }

		err = oh_init_textbuffer(&buffer);
		err = oh_append_textbuffer(&buffer, str);
		
                err = $encode_name(&buffer, &enum_type);
                if (err != SA_OK) {
		    printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
		    printf("  Received error=%d\\n", err);
		    return -1;
                }
    
                if ($case != enum_type) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
                        printf("  Received enum type=%x\\n", enum_type);
                        return -1;
                }
	}

EOF

    return 0;
}

#################################
# Print xtestcase file's testcase
#################################
sub print_xtestfile_case($$$) {
    my( $state, $cat, $str ) = @_;
    my $valid_cat_test = "";

    # Special case categories with same event definitions
    if ($cat eq "SAHPI_EC_GENERIC" || $cat eq "SAHPI_EC_SENSOR_SPECIFIC") {
	$valid_cat_test = "(SAHPI_EC_GENERIC == event_cat) || (SAHPI_EC_SENSOR_SPECIFIC == event_cat)";
    }
    else {
	$valid_cat_test = "expected_cat == event_cat";
    }

    # Special case Thresholds
    if ($state eq "SAHPI_ES_LOWER_CRIT") {
	$state = "SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_MAJOR | SAHPI_ES_LOWER_CRIT";
	$str = "LOWER_MINOR | LOWER_MAJOR | LOWER_CRIT";
    }
    if ($state eq "SAHPI_ES_LOWER_MAJOR") {
	$state = "SAHPI_ES_LOWER_MINOR | SAHPI_ES_LOWER_MAJOR";
	$str = "LOWER_MINOR | LOWER_MAJOR";
    }
    if ($state eq "SAHPI_ES_UPPER_CRIT") {
	$state = "SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR | SAHPI_ES_UPPER_CRIT";
	$str = "UPPER_MINOR | UPPER_MAJOR | UPPER_CRIT";
    }
    if ($state eq "SAHPI_ES_UPPER_MAJOR") {
	$state = "SAHPI_ES_UPPER_MINOR | SAHPI_ES_UPPER_MAJOR";
	$str = "UPPER_MINOR | UPPER_MAJOR";
    }

    print XFILE_TEST <<EOF;
        /* $cat - $state testcase */
        {
                expected_cat = $cat;
                expected_state = $state;
                expected_str = "$str";
  
                err = oh_decode_eventstate($state, $cat, &buffer);
                if (err != SA_OK) {
		    printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
		    printf("  Received error=%d\\n", err);
		    return -1; 
                }
    
                if (strcmp(expected_str, (char *)buffer.Data)) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
                        printf("  Received string=%s\\n", buffer.Data);
			printf("  Expected string=%s\\n", expected_str);
                        return -1;             
                }
    
                err = oh_encode_eventstate(&buffer, &event_state, &event_cat);
                if (err != SA_OK) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
			printf("  Received error=%d\\n", err);
                        return -1;
                }
    
                if ((expected_state != event_state) || !($valid_cat_test)) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
                        printf("  Received state=%x; Received cat=%x\\n", 
			       event_state, event_cat);
                        return -1;
                }
        }
	    
EOF

    return 0;
}

###########################################
# Print xtestcase file's bad event testcase
###########################################
sub print_xtestfile_badevent($) {
    my( $cat ) = @_;

    print XFILE_TEST <<EOF;
        /* $cat - Bad event testcase */
        {
		if (oh_valid_eventstate(BAD_EVENT, $cat, SAHPI_FALSE)) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
                        return -1;
                }
	}

EOF

    return 0;
}

#########################
# Print c file's func end
#########################
sub print_cfile_endfunc($) {
   my( $type ) = @_;
   my( $ret ) = "NULL";

    if (($type eq $entity_type) && $atca) {
    	$ret = "$atca_entity_lookup_name(value)";
    }

   print FILE_C <<EOF;
        default:
                return $ret;
        }
}
EOF

    print FILE_C "\n";
    return 0;
}

####################################
# Print testcase file's default test
####################################
sub print_testfile_endfunc($) {
    my( $type ) = @_;
    my $pretty_type = beautify_type_name($type);
    my $lookup_name = "oh_lookup_" . "$pretty_type";
    my $encode_name = "oh_encode_" . "$pretty_type";
   
    print FILE_TEST <<EOF;
        /* $type - Default testcase */
        {
	        $type value = BAD_ENUM_VALUE;
                expected_str = NULL;

                str = $lookup_name(value);
                if (str != expected_str) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
			printf("  Received string=%s; Expected string=%s\\n", str, expected_str); 
                        return -1;             
                }

	}
    
	{ 
                /* $type - NULL buffer testcase */
	        SaErrorT  err, expected_err;     
		SaHpiTextBufferT buffer;
		$type enum_type;

		expected_err = SA_ERR_HPI_INVALID_PARAMS;
                err = $encode_name(0, &enum_type);
                if (err != expected_err) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\\n", err, expected_err);
                        return -1;
                }
	             	
                /* $type - Invalid type testcase */
		err = oh_init_textbuffer(&buffer);		
		err = oh_append_textbuffer(&buffer, \"INVALID_TYPE\");

		expected_err = SA_ERR_HPI_INVALID_DATA;
                err = $encode_name(&buffer, &enum_type);
                if (err != expected_err) {
                        printf("  Error! Testcase failed. Line=%d\\n", __LINE__);
			printf("  Received error=%d; Expected error=%d\\n", err, expected_err);
                        return -1;
                }
	}

EOF

    return 0;
}

#####################################
# Print h file's static trailing text 
#####################################
sub print_hfile_ending(*) {
    my ( $filehandler) = @_;

    print $filehandler <<EOF;

#ifdef __cplusplus
}
#endif

#endif
EOF

    return 0;
}

############################################
# Print testcase file's static trailing text 
############################################
sub print_testfile_ending {

    print FILE_TEST <<EOF;
        return 0;
}
EOF

    return 0;
}

#############################################
# Print xtestcase file's static trailing text 
#############################################
sub print_xtestfile_ending {

    print XFILE_TEST <<EOF;
        return 0;
}
EOF

    return 0;
}
