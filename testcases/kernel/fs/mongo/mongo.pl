#!/usr/bin/perl
#
# Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
#

#
# Mongo.pl is reiserfs benchmark.
#
# To run please use run_mongo script or :
#
# # ./mongo.pl reiserfs /dev/xxxx /testfs log1 5
# or
# # ./mongo.pl ext2 /dev/xxxx /testfs log2 5
#
# 5 - number of processes, you can set any number here
#
# Test will format partition /dev/xxxx by 'mkreiserfs' or 'mke2fs'
# mount it and run given number of processes during each phase :
# Create, Copy, Symlinks, Read, Stats, Rename and Delete.
#
# Also, the program calc fragmentations after Create and Copy phases:
# Fragm = number_of_fragments / number_of_files
# (Current version use the files more than 16KB to calc Fragm.)
#
# You can find the same results in files : log, log.tbl, log_table
#
# log       - raw results
# log.tbl   - results for compare program
# log_table - results in table form
#

$EXTENDED_STATISTICS = 1;


use POSIX;
use File::stat;

sub print_usage {

        print "\nUsage: mongo.pl <filesystem> <device>";
	print                  " <mount_point> <log> <processes>\n";

	print "<filesystem>  - the name of filesystem [reiserfs|ext2]\n";
	print "<device>      - the device for benchmark (e.g. /dev/hda9)\n";
	print "<mount_point> - mount-point for the filesystem";
	print " (e.g. /mnt/testfs)\n";
	print "<log>         - the name prefix for benchmark results\n";
	print "<processes>   - the number of benchmark processes\n";

	print "\nexamples:\n";
	print "mongo.pl reiserfs /dev/hda9 /testfs reiserfs_results 1\n";
	print "mongo.pl ext2 /dev/hda9 /testfs ext2_results 1\n";

	print "\nThe results will be put in ./results directory\n";
}


#------- Subroutines declaration --------
sub make_fsys;
sub mongo_x_process;
sub mongo_launcher;
sub set_params;

#------- main() -------------------------

if ( $#{ARGV} != 4 ) {
        print_usage;
	exit(0);
    }

#--------------------------------------------
# Set working directories
#--------------------------------------------
$TOPDIR  = "$ENV{PWD}";

$RESDIR  = "${TOPDIR}/results";
$HTMLDIR = "${RESDIR}/html";

$FILESYSTEM  = $ARGV[0];
$DEVICE      = $ARGV[1];
$TESTDIR     = $ARGV[2];
$PROCESSES   = $ARGV[4];

$LOGFILE     = "${RESDIR}/${ARGV[3]}";
$LOGFILE2    = "${LOGFILE}_table";
$LOGFILE3    = "${LOGFILE}.tbl";

$TMPFILE     = "${RESDIR}/mongo_tmp";
$nproc       = $PROCESSES;
$READIT      = "${TOPDIR}/mongo_read";
$SLINKS      = "${TOPDIR}/mongo_slinks";

#-------- reiser_fract_tree parameters----------------
$x1mb   = 1024 * 1024;
$x2mb   =    2 * $x1mb;
$x3mb   =    3 * $x1mb;

$x5mb   =    5 * $x1mb;
$x50mb  =   50 * $x1mb;
$x100mb =  100 * $x1mb;

# Total amount of bytes in all files on test partition
#-----------------------------------------------------

$small_bytes   = $x50mb;
$medium_bytes  = $x100mb;
$big_bytes     = $x100mb;
$large_bytes   = $x100mb;

# Median size of files in bytes for first tree to create
#-------------------------------------------------------
$small_size   = 100;
$medium_size  = 1000;
$big_size     = 10000;
$large_size   = 100000;

# Keep the largest file to one fifth (100 million bytes)
# of the total tree size.
#-------------------------------------------------------
$max_file_size = 100000000;

# Yuri Shevchuk says that 0 is the median size
# in real life, so I believe him.
#----------------------------------------------
$median_dir_nr_files = 0;

# This should be larger, change once done testing.
#-------------------------------------------------
$bytes_to_consume = 10000000;

$median_file_size = 100;
$max_file_size    = 1000000;

$median_dir_nr_files    = 100;
$max_directory_nr_files = 10000;

$median_dir_branching = 0;
$max_dir_branching    = 1;

# This should be varying, someday....
#------------------------------------
$write_buffer_size = 4096;

@numb_of_bytes = ($small_bytes, $medium_bytes, $big_bytes, $large_bytes);
@size_of_files = ($small_size,  $medium_size,  $big_size,  $large_size);

$reiser_fract_tree_rep_counter = 3;

$total_params = $#{numb_of_bytes};

#... Make directories for results
#--------------------------------
unless (-e $RESDIR) {
    print "Creating dir: ${RESDIR} \n";
    system("mkdir $RESDIR");
}

unless ( -e $HTMLDIR ) {
    print "Creating dir: ${HTMLDIR} \n";
    system("mkdir $HTMLDIR");
}

#... Compile *.c files if it is necessary
#----------------------------------------
sub compile
{
  my $file = shift @_;
  my $opt = shift @_ if @_ ;
  my $cfile = $file . ".c";
  die "source file \"${cfile}\" does not exist" unless (-e  "$cfile");
  if ( -e "$file" && (stat("$file")->mtime >= stat("$cfile")->mtime)) {
    print "$file is up to date ...\n";
  } else {
    print "Compiling ${cfile} ...\n";
    system ("gcc $cfile -o $file $opt");
  }
}

compile("reiser_fract_tree", "-lm");
compile("mongo_slinks");
compile("mongo_read");
compile("map5");
compile("summ");
compile("mongo_compare");

#... Check the command string parameters
#---------------------------------------
unless ( ($FILESYSTEM eq "reiserfs") or ($FILESYSTEM eq "ext2") ) {
    print "mongo.pl: not valid filesystem name: ${FILESYSTEM} \n";
    print "Usage: mongo.pl <filesystem> <device> <mount_point> <log> <repeat>\n";
    exit(0);
}

unless ( -b $DEVICE ) {
    print "mongo.pl: not valid device: ${DEVICE} \n";
    print "Usage: mongo.pl <filesystem> <device> <mount_point> <log> <repeat>\n";
    exit(0);
}


#------- Subroutines --------------------------------------
#----------------------------------------------------------
sub get_blocks_usage ($) {
  my ($mp) = @_;
  my $df = `df -k $mp | tail -n 1`;
  chomp $df;
  my @items = split / +/, $df;
  return $items[2];
}

sub make_fsys {

    system ("umount $TESTDIR") ;

    if ( $FILESYSTEM eq "reiserfs" ) {
	system("echo y | mkreiserfs $DEVICE") ;
	system("mount -t reiserfs $DEVICE $TESTDIR") ;
    }

    if ( $FILESYSTEM eq "ext2" ) {
	system("mke2fs $DEVICE") ;
	system("mount $DEVICE $TESTDIR") ;
    }
}


#------------------------------------------------------------------
# Mongo Launcher
#------------------------------------------------------------------
sub mongo_launcher {

    my ($phase_num, $phase_name, $cmd, $dir1, $dir2, $flag, $processes) = @_ ;


    print "$phase_num.$phase_name files of median size $median_file_size bytes ($p processes)...\n";
    print LOG "********* Phase $phase_num: $phase_name files of median size $median_file_size bytes ($p processes) *********\n";
    $i=0;
    $total=0;

# eliminate the rep counter and the while
    while ( $i < $reiser_fract_tree_rep_counter ) {
	print "$phase_name : ";
    	print LOG "$phase_name : ";
	$com = "";
	$pp=$processes;

	$j=0;
	while ($pp > 0) {
	    $pp--;

# the fact that this if statement was necessary indicated you
# attempted excessive generalization and abstraction where it was not
# natural to the task that makes the code harder to understand.  put
# every command on one line to execute.  I like it when I can read a
# one line command and see what that phase of the test does instead of
# looking in many places throughout the code.

	    if ($phase_num == 1) {
    		$com .= "$cmd $dir1-$i-$j $flag";
	    }
	    elsif ($phase_num == 2) {
		$com .= "$cmd $dir1-$i-$j $dir2-$i-$j";
	    }
	    elsif ($phase_num == 3) {
		$com .= "$cmd $dir1-$i-$j "."-type f | while read X; do echo \$X \$X.lnk ; done | $TOPDIR/mongo_slinks ";
	    }
	    elsif ($phase_num == 4) {
		$com .= "$cmd";
	    }
	    elsif ($phase_num == 5) {
		$com .= "$cmd";
	    }
	    elsif ($phase_num == 6) {
		$com .= "$cmd $dir1-$i-$j -type f | perl -e 'while (<>) { chomp; rename (\$_, \"\$_.r\"); };'";
		#$com .= " & $cmd $dir2-$i-$j "."-type d -exec mv {} {}.r ';'";
	    }
	    elsif ($phase_num == 7) {
		if ($processes > 1) {
		    $com .= "$cmd $dir1-$i-$j & $cmd $dir2-$i-$j";
		}
		else {
		    $com .= "$cmd $dir1-$i-$j ; $cmd $dir2-$i-$j";
		}
	    }
	    $com .= " & ";
	    $j++;
	}

	$com .= " wait";
	#print $com, "\n";

	@t=`(time -p $com) 2>&1`;

	@tt = split ' ', $t[0];
    	$res = $tt[1];
	unless ( $res =~ /\s*\d+/) {
	    print @t , "\n";
	    print LOG @t, "\n";
	} else {
	    print LOG "$res sec.\n";
	    print "$res sec.\n";
	}

	$total += $res;
    	$i++;
     }

    print "total $phase_name time: $total sec.\n";
    print LOG "total $phase_name time: $total sec.\n";

    $ares[$phase_num]=$total;  # ser array of results

    if ($EXTENDED_STATISTICS) {
	if( $phase_num < 3) {
	    $used = get_blocks_usage($TESTDIR) - $used0;
	    if ($phase_num == 1) {
		$used1=$used;
	    }elsif($phase_num == 2){
		$used2=$used;
	    }
	    print "Used disk space (df) : $used KB\n";
	    print LOG "Used disk space (df) : $used KB\n";

	    open (FIND_PIPE, "find $TESTDIR|") || die "cannnot open pipe from \"find\": $!\n";
	    $dirs = 0;
	    $files = 0;
	    $files16 = 0;

	    while(<FIND_PIPE>) {
		chomp;
		$st = lstat ($_);
		if (S_ISDIR($st->mode)) {
		    $dirs ++;
		} elsif (S_ISREG($st->mode)) {
		    $files ++;
		    $files16 ++ if ($st->size > 16384);
		}
	    }

	    close (FIND_PIPE);

	    print "Total dirs: $dirs\n";
	    print "Total files: $files\n";
	    print LOG "Total dirs: $dirs\n";
	    print LOG "Total files: $files\n";

	    #$f=$frag;
	    $f16  = $files16;
	    $fr16 =`find $TESTDIR -type f -size +16k | xargs $TOPDIR/map5 | $TOPDIR/summ | tail -n 1 2>&1`;
	    @ff16= split ' ', $f16;
	    @ffr16= split ' ', $fr16;
	    $files16 = $ff16[0];
	    $frag = $ffr16[0];
	    $procent = $frag / $files16;
	    print "Total fragments : $frag \n";
	    print LOG "Total fragments : $frag \n";

	    printf "Fragments / files :%.3f\n", $procent;
	    printf LOG "Fragments / files :%.3f\n", $procent;
	    $frag_res[$phase_num]=$procent;  # ser array of results
	}
    }

    system("sync");
    print "\n";
    print LOG "\n";

}

# and what is an x process?

#------------------------------------------------------------------
# MONGO_X_PROCESS ( x is number of processes to run )
#------------------------------------------------------------------
sub mongo_x_process {

    my ($processes) = @_ ;
    $p = $processes;

    make_fsys;       # make and mount the file system
    $used0 = get_blocks_usage($TESTDIR);

    open LOG,  ">>$LOGFILE"  or die "Can not open log file $LOGFILE\n";
    open LOG2, ">>$LOGFILE2" or die "Can not open log file $LOGFILE2\n";
    open LOG3, ">>$LOGFILE3" or die "Can not open log file $LOGFILE2\n";

    print LOG "FILESYSTEM=$FILESYSTEM \n";

    print "\n";
    if($p == 1) {
	print "mongo_single_process, the_set_of_param.N=$par_set_n of $total_params \n";
	print LOG "mongo_single_process, the_set_of_paramN=$par_set_n of $total_params \n";
    } elsif ($p > 1) {
        print "mongo_multi_process ($p processes), the_set_of_param.N=$par_set_n of $total_params \n";
	print LOG "mongo_multi_process ($p processes), the_set_of_paramN=$par_set_n of $total_params \n";
    }

    print "Results in file : $LOGFILE \n";
    print "\n";

    $dir1 = "$TESTDIR/testdir1";
    $dir2 = "$TESTDIR/testdir2";
    $flag = 0;

    $cmd_1 = "$TOPDIR/reiser_fract_tree $bytes_to_consume $median_file_size $max_file_size $median_dir_nr_files $max_directory_nr_files $median_dir_branching $max_dir_branching $write_buffer_size";
    $cmd_2 = "cp -r";
    $cmd_3 = "find";
    $cmd_4 = "find $TESTDIR -type f | xargs $TOPDIR/mongo_read";
    $cmd_5 = "find $TESTDIR -type f > /dev/null"; # it should be enough for stat all files. -zam
    $cmd_6 = "find"; #" $TESTDIR -type f -exec mv {} {}.r ';'";
    $cmd_7 = "rm -r";

    system("sync");
    $frag = 0;
    mongo_launcher ( 1, "Create", $cmd_1, $dir1, $dir2, $flag, $p); # phase 1
    mongo_launcher ( 2, "Copy  ", $cmd_2, $dir1, $dir2, $flag, $p); # phase 2
    mongo_launcher ( 3, "Slinks", $cmd_3, $dir1, $dir2, $flag, $p); # phase 3
    mongo_launcher ( 4, "Read  ", $cmd_4, $dir1, $dir2, $flag, $p); # phase 4
    mongo_launcher ( 5, "Stats ", $cmd_5, $dir1, $dir2, $flag, $p); # phase 5
    mongo_launcher ( 6, "Rename", $cmd_6, $dir1, $dir2, $flag, $p); # phase 6
    mongo_launcher ( 7, "Delete", $cmd_7, $dir1, $dir2, $flag, $p); # phase 7

    print LOG2 "\n";
    if ($processes > 1) {
	print LOG2 "MONGO_MULTI_PROCESS ($processes processes) BENCHMARK RESULTS (time in sec.)\n";
    }else {
	print LOG2 "MONGO_SINGLE_PROCESS BENCHMARK RESULTS (time in sec.)\n";
    }
    print LOG2 "  FILESYSTEM=$FILESYSTEM\n";
    print LOG2 "  parameters: files=$files, base_size=$median_file_size bytes, dirs=$dirs\n";
    print LOG2 "--------------------------------------------------------------\n";
    print LOG2 "Create\tCopy\tSlink\tRead\tStats\tRename\tDelete\n";
    print LOG2 " time \ttime\ttime\ttime\ttime \t time \t time\n";
    print LOG2 "--------------------------------------------------------------\n";
    print LOG2 "$ares[1]\t$ares[2]\t$ares[3]\t$ares[4]\t$ares[5]\t$ares[6]\t$ares[7]\n";
    print LOG2 "--------------------------------------------------------------\n";
    print LOG2 "The size of files tree : \n";
    print LOG2 "         after create = $used1 kb\n";
    print LOG2 "         after copy   = $used2 kb\n";
    print LOG2 "\n";


    print LOG3 "\n";
    if ($processes > 1) {
	print LOG3 "MONGO_MULTI_PROCESS  ($processes)    \n";
    }else {
	print LOG3 "MONGO_SINGLE_PROCESS      \n";
    }
    print LOG3 "parameters:              \n";
    print LOG3 "files=$files            \n";
    print LOG3 "base_size=$median_file_size bytes    \n";
    print LOG3 "dirs=$dirs              \n";
    print LOG3 "\n";

    print LOG3 "FSYS=$FILESYSTEM         \n";
    print LOG3 "(time in sec.)           \n";
    print LOG3 "Create : $ares[1]\n";
    print LOG3 "Fragm. : $frag_res[1]\n";
    print LOG3 "df     : $used1\n\n";
    print LOG3 "Copy   : $ares[2] \n";
    print LOG3 "Fragm. : $frag_res[2]\n";
    print LOG3 "df     : $used2\n\n";
    print LOG3 "Slinks : $ares[3]\n";
    print LOG3 "Read   : $ares[4]\n";
    print LOG3 "Stats  : $ares[5]\n";
    print LOG3 "Rename : $ares[6] \n";
    print LOG3 "Delete : $ares[7]\n";

    print LOG3 "\n";


    if($processes > 1) {
	print LOG "******* The end of mongo_multi_process *******";
    }else {
	print LOG "******* The end of mongo_single_process *******";
    }
}

#---------------------------------------------------
# Set parameters
#---------------------------------------------------
sub set_params {
    my ($n) = @_ ;

    $bytes_to_consume = $numb_of_bytes[$n];
    $median_file_size = $size_of_files[$n];

    #$max_file_size    = 1000000;

    #$median_dir_nr_files    = 100;
    #$max_directory_nr_files = 10000;

    #$median_dir_branching = 0;
    #$max_dir_branching    = 1;

}

#----------------------------------------------------------
#           TEST START
#----------------------------------------------------------

    $par_set_n = 0;
    foreach $fsize (@size_of_files) {
	set_params ($par_set_n);
	mongo_x_process( $nproc );    # run n processes
	$par_set_n++;
    }
    system("umount $TESTDIR");
    exit;


