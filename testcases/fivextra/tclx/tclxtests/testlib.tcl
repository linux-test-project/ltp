#
# testlib.tcl --
#
# Test support routines.  Some of these are based on routines provided with
# standard Tcl.
#------------------------------------------------------------------------------
# Copyright 1992-1999 Karl Lehenbauer and Mark Diekhans.
# Copyright 2002 ActiveState Corporation.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies.	 Karl Lehenbauer and
# Mark Diekhans make no representations about the suitability of this
# software for any purpose.  It is provided "as is" without express or
# implied warranty.
#------------------------------------------------------------------------------
# $Id: testlib.tcl,v 1.1 2005/03/22 17:52:15 mridge Exp $
#------------------------------------------------------------------------------
#

# Save the unknown command in a variable SAVED_UNKNOWN.	 To get it back, eval
# that variable.

if {[lsearch [namespace children] ::tcltest] == -1} {
    package require tcltest
    namespace import ::tcltest::*
}
package require Tclx 8.4

foreach need {
    fchown fchmod flock fsync ftruncate msgcats posix_signals symlink
    signal_restart truncate waitpid
} {
    set ::tcltest::testConstraints(need_$need) [infox have_$need]
}

set ::tcltest::testConstraints(need_chmod) [llength [info commands chmod]]
if {[cequal $::tcl_platform(platform) "unix"]} {
    set ::tcltest::testConstraints(isRoot)     [cequal [id user] "root"]
    set ::tcltest::testConstraints(isNotRoot)  \
	    [expr {![cequal [id user] "root"]}]
}


# Genenerate a unique file record that can be verified.	 The record
# grows quite large to test the dynamic buffering in the file I/O.

proc GenRec {id} {
    return [format "Key:%04d {This is a test of file I/O (%d)} KeyX:%04d %s" \
	    $id $id $id [replicate :@@@@@@@@: $id]]
}

#
# Routine to execute tests and compare to expected results.
#
proc Test {name description body int_result result} {
    if {$int_result == 0} {
	uplevel 1 [list test $name $description $body $result]
    } elseif {$int_result == 1} {
	uplevel 1 [list test $name $description \
		"list \[catch {$body} msg\] \$msg" [list 1 $result]]
    } else {
	puts stderr "FIX OUTDATED TEST: $test_name $test_description"
    }
}

# Proc to fork and exec child that loops until it gets a signal.
# Can optionally set its pgroup.  Wait till child has actually execed or
# kill breaks on some systems (i.e. AIX).  Windows is a drag, since the
# command line parsing is really dumb.	Pass it in a file instead.

proc ForkLoopingChild {{setPGroup 0}} {
    global tcl_platform

    set childProg {
	file delete CHILD.RUN
	catch {while {1} {after 1000;update}}
	exit 10
    }

    # Create semaphore (it also contains the program to run for windows).
    set fh [open CHILD.RUN w]
    puts $fh $childProg
    close $fh
    flush stdout
    flush stderr

    if {[cequal $tcl_platform(platform) unix]} {
	set newPid [fork]
	if {$newPid == 0} {
	    if $setPGroup {
		id process group set
	    }
	    catch {execl $::tcltest::tcltest CHILD.RUN} msg
	    puts stderr "execl failed (ForkLoopingChild): $msg"
	    exit 1
	}
    }
    if {[cequal $tcl_platform(platform) windows]} {
	if $setPGroup {
	    error "setpgroup not supported on windows"
	}
	set newPid [execl $::tcltest::tcltest CHILD.RUN]
    }

    # Wait till the child is actually running.
    while {[file exists CHILD.RUN]} {
	sleep 1
    }
    return $newPid
}

#
# Create a file.  If the directory doesn't exist, create it.
#
proc TestTouch file {
    file mkdir [file dirname $file]
    close [open $file w]
}

#
# Remove files and directories with out errors.
#
proc TestRemove args {
    foreach f $args {
	catch {file delete -force $f}
    }
}
