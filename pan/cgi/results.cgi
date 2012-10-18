#!/usr/bin/perl

use CGI qw(:standard escapeHTML);

# When something goes wrong before we start output, use this function
# so there is still output
sub failure {
	print header("text/html"),start_html;
	print "$_[0]\n";
	print end_html;
	exit;
}

# Most of the work is done in this directory
unless (chdir("/usr/tests/ltp/results")) {
	failure("Could not get to the results directory\n");
}


# grab the parameters that determine what's going on then branch
$get_df = param("get_df");
if ($get_df) {
	# copy a driver file and output it.
	$get_df = (<$get_df*>)[0];
	($host, $datestr, $suite, $type, $gz) = split(/\./, $get_df);
	#print start_html, "<pre>\n";
	if ($gz) {
		open (DF, "gunzip -c $get_df|") || print "$get_df not found\n";
	} else {
		open (DF, "$get_df") || print "$get_df not found";
	}
	if ($type eq "driver" || $type eq "summary") {
		print header("text/plain");
		$zoom_tag = param("zoom_tag");
		if ($zoom_tag) {
			while (<DF>) {
				# find the start of a test
				while (<DF>) {
					if (/\<\<\<test_start\>\>\>/) {
						$line = <DF>;
						if ($line =~ /^tag=$zoom_tag /) {
							print "<<<test_start>>>\n";
							print $line;

							do {
								$line = <DF>;
								print $line;
							} until ($line =~ /\<\<\<test_end\>\>\>/);
							exit;
						}
					}
				}
			}
			print "Did not find tag $zoom_tag\n";
		} else {
			while (<DF>) {
				print $_;
			}
		}
	} elsif ($type eq "scanner") {
		print header("text/html");
		print start_html, "<pre>\n";
		while (<DF>) {
			print;
			if (/^-+/) { last;}
		}
		@rest = <DF>;
		# this is just to put the * at the end of the test case list
		unless (param("raw")) {
			foreach (@rest) { s/\*/{/; }
			foreach (@rest) { s/(\s)-(\s)/\1}\2/; }
			@rest = sort @rest;
			foreach (@rest) { s/{/*/; }
			foreach (@rest) { s/}/-/; }
		}

		foreach (@rest) {
			s/(\S+)/<a href="results.cgi?get_df=$host.$datestr.$suite.driver&zoom_tag=\1">\1<\/a>/;
			# colorize the status column
			s/\bPASS\b/\<font color\=green\>PASS\<\/font\>/i;
			s/\bFAIL\b/\<font color\=\"red\"\>FAIL\<\/font\>/i;
			s/\bCONF\b/\<font color\=\"yellow\"\>CONF\<\/font\>/i;
			s/\bBROK\b/\<font color\=\"blue\"\>BROK\<\/font\>/i;
			print;
		}
		print "\n</pre>",end_html;
	}
	close(DF);
	#print "\n</pre>\n",end_html;
} else {
	%results = ();

	# run through the files in the results directory
	@driver_files = <*driver*>;
	foreach $df (sort(@driver_files)) {

		($host, $datestr, $suite, $type, $gz) = split(/\./, $df);

		$a_rec = ();
		$a_rec->{HOST} = $host;
		$a_rec->{DATE} = $datestr;
		$a_rec->{SUITE} = $suite;
		$a_rec->{DRIVER_FILE} = $df;

		$results{ $a_rec->{DRIVER_FILE} } = $a_rec;
	}

	# write the HTML file
	print header("text/html"),start_html;

	@ri = values %results;
	@ri = sort { $a->{HOST} cmp $b->{HOST}
			||$b->{DATE} <=> $a->{DATE}
			||$a->{SUITE} cmp $b->{SUITE} } @ri;
	$lasthost = "";
	$lastdate = "";
	$lastsuite = "";
	$indent = 0;
	print "<table>\n";
	print "<tr><th>Hostname<th>Date<th>Suite</tr>\n";
	foreach $rp ( @ri ) {
		$thishost = $rp->{HOST};
		$thisdate = $rp->{DATE};
		$thissuite = $rp->{SUITE};

		# figure out where is the table we need to start
		if ($lasthost ne $thishost) {
			$indent = 0;
		} elsif ($lastdate ne $thisdate) {
			$indent = 1;
		} elsif ($lastsuite ne $thissuite) {
			$indent = 2;
		}

		# write the rows we need depending on the starting point
		# host level
		if ($indent <= 0) {
			print "<tr><td>$thishost\n";
		}
		# date level
		if ($indent <= 1) {
			($year, $month, $day, $hour, $min) = ($thisdate =~ /(\d+)(\d{2})(\d{2})(\d{2})(\d{2})/);
			print "<tr><td><td>$year-$month-$day $hour:$min\n";
		}
		# suite level
		if ($indent <= 2) {
			print "<tr><td><td><td>";
			print "$thissuite";
			print " [<a href=\"results.cgi?get_df=$rp->{DRIVER_FILE}\">driver output</a>]";
			print " [<a href=\"results.cgi?get_df=$thishost.$thisdate.$thissuite.scanner\">results</a>]";
			print " [<a href=\"results.cgi?get_df=$thishost.$thisdate.$thissuite.summary\">summary</a>]";

			print "\n";
		}

		# make sure we update the $last... variables
		$lasthost = $thishost;
		$lastdate = $thisdate;
		$lastsuite = $thissuite;
	}
	print "</table>\n";
	print end_html;
}

