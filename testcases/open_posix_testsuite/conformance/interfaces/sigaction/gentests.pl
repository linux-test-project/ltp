#!/usr/bin/perl -w

# Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
# Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.



my (@signals) = ("SIGABRT", "SIGALRM", "SIGBUS", "SIGCHLD", "SIGCONT",
		 "SIGFPE", "SIGHUP", "SIGILL", "SIGINT", "SIGPIPE",
		 "SIGQUIT", "SIGSEGV", "SIGTERM", "SIGTSTP", "SIGTTIN",
		 "SIGTTOU", "SIGUSR1", "SIGUSR2", "SIGPOLL", "SIGPROF",
		 "SIGSYS", "SIGTRAP", "SIGURG", "SIGVTALRM", "SIGXCPU",
		 "SIGXFSZ");

my (%testcases, $prev);

$prev = "SIGALRM";

open (LIST, "ls ./templates/*.in|") or die "Could not get listing";
while (<LIST>) {
    my ($fname) = $_;
    chomp $fname;

    if ($fname =~ /template_([0-9]*)-.*\.in/) {
	my ($assertion) = $1;

	open (TEMPLATE, "$fname") or die "Could not open $fname";
	my (@t) = <TEMPLATE>;
	close TEMPLATE;

	print "Building source based on $fname\n";
	foreach (@signals) {
	    my ($signal) = $_;

	    $testcases{$assertion}++;
	    open (OUT, ">$assertion-" . $testcases{$assertion} . ".c")
		or die "Could not open source file";
	    foreach (@t) {
		my ($line) = $_;
		$line =~ s/%%MYSIG%%/$signal/;
		$line =~ s/%%MYSIG2%%/$prev/;
		print OUT $line;
	    }
	    close OUT;

	    $prev = $signal;
	}
    }
}


