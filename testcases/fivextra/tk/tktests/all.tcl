# all.tcl --
#
# This file contains a top-level script to run all of the Tk
# tests.  Execute it by invoking "source all.tcl" when running tktest
# in this directory.
#
# Copyright (c) 1998-1999 by Scriptics Corporation.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
# 
# RCS: @(#) $Id: all.tcl,v 1.1 2005/03/22 17:52:19 mridge Exp $

package require Tcl 8.4
package require tcltest 2.1
tcltest::configure -testdir [file join [pwd] [file dirname [info script]]]
tcltest::configure -singleproc 1
eval tcltest::configure $argv
tcltest::runAllTests
