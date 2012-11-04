#!/usr/bin/perl

use CGI qw(:standard);

# keep a copy of each 'uname -a' string so we don't have to search for it every time.
%uname_cache = {};

# When something goes wrong before we start output, use this
# function so there is still output
sub failure {
	print header("text/html"),start_html;
	print "$_[0]\n";
	print end_html;
	exit;
}
# get the UNAME line for a host, date, suite
sub get_result_uname {
	my($inhost, $indate, $insuite, $filename);
	my(@possible_files, $pf, $line);
	my($host, $datestr, $suite, $type, $gz);
	# build a filename
	$inhost = $_[0];
	$indate = $_[1];
	if ($#_ >= 2) {
		$insuite = $_[2];
	} else {
		$insuite = "*";
	}
	# check to see if we've done this already
	if (exists($uname_cache{"$inhost.$indate"})) {
		return $uname_cache{"$inhost.$indate"};
	}
	$filename = "$inhost.$indate.$insuite.*";
	@possible_files = <${filename}>;
	if ($#possible_files < 1) {
		return "";
	}
	foreach $pf (@possible_files) {
		($host, $datestr, $suite, $type, $gz) = split(/\./, $pf);
		if ($type eq "summary") {
			next;
		} elsif ($type eq "scanner") {
			open (UF, $pf) || next;
			while ($line = <UF>) {
				if ($line =~ /^UNAME/) {
					close(UF);
					$line =~ s/UNAME *//;
					$line =~ s/$inhost//;
					$uname_cache{"$inhost.$indate"} = $line;
					return $line;
				}
			}
		} elsif ($type eq "driver") {
			if ($gz) {
				open (UF, "gunzip -c $pf|") || next;
			} else {
				open (UF, "$pf") || next;
			}
			while ($line = <UF>) {
				if ($line =~ /^UNAME/) {
					close(UF);
					$line =~ s/UNAME="(.*)"/\1/;
					$line =~ s/$inhost//;
					$uname_cache{"$inhost.$indate"} = $line;
					return $line;
				}
			}
		} else {
			next;
		}
	}
	return "";
}

# Create the headers row, adding links for sorting options
sub print_headers {

	print "\n<tr>";

	for($i = 0; $i <= $#rso; $i++) {
		print "<th><a href=\"browse.cgi?sort=";
		for ($j = 0; $j <= $#rso; $j++) {
			if ($j == $i) { $rsd[$j] = $rsd[$j] * -1; }
			if ($rsd[$j] == -1) { print "-"; }
			if ($j == $i) { $rsd[$j] = $rsd[$j] * -1; }
			print $rso[$j];
			if ($j < $#rso) { print ","; }
		}
		print "\">$rso[$i]</a>\n";
	}

	print "</tr>\n";
}

############
# main()   #
############

# Most of the work is done in this directory
unless (chdir("/usr/tests/ltp/results")) {
	failure("Could not get to the results directory\n");
}

@extra_path = split(/\//, $ENV{PATH_INFO});

# rso = Result Sort Order
# rsd = Result Sort Direction
#@rso = (HOST,SUITE, DATE, UNAME);
@rso = (SUITE, HOST, DATE, UNAME);
@rsd = (1, 1, -1, 1);

# allow the user to specify the sort order
if ($sort_order = param("sort")) {
	@so = split(/,/, $sort_order);
	print $so;
	@rso = ();
	for($i = 0; $i <= $#so; $i++) {
		# parse the field
		if ($so[$i] =~ /host/i) { push(@rso, HOST); }
		elsif ($so[$i] =~ /date/i) { push(@rso, DATE); }
		elsif ($so[$i] =~ /suite/i) { push(@rso, SUITE); }
		elsif ($so[$i] =~ /uname/i) { push(@rso, UNAME); }
		# parse the direction
		if ($so[$i] =~ /-/) { $rsd[$i] = -1; }
		else { $rsd[$i] = 1; }
	}
}

if ($#extra_path > 0) {

} else {

	%results = ();

	# run through the files in the results directory
	@driver_files = <*driver*>;
	foreach $df (@driver_files) {

		($host, $datestr, $suite, $type, $gz) = split(/\./, $df);

		$a_rec = ();
		$a_rec->{HOST} = $host;
		$a_rec->{DATE} = $datestr;
		$a_rec->{SUITE} = $suite;
		$a_rec->{DRIVER_FILE} = $df;
		$a_rec->{UNAME} = get_result_uname($host, $datestr);

		$results{ $a_rec->{DRIVER_FILE} } = $a_rec;
	}

	# write the HTML file
	print header("text/html"),start_html;
	print "This is a demo page for browsing the Linux LTP results.  Select the results you want to compare and click the \"Compare\" button.", p, h2("Warning"), "The results are placed in a large table which may take a long time to render on some browsers", p;
	@ri = values %results;
	@ri = sort { ($a->{$rso[0]} cmp $b->{$rso[0]})*$rsd[0]
			|| ($a->{$rso[1]} cmp $b->{$rso[1]})*$rsd[1]
			|| ($a->{$rso[2]} cmp $b->{$rso[2]})*$rsd[2]
			|| ($a->{$rso[3]} cmp $b->{$rso[3]})*$rsd[3] } @ri;

	$last = ();
	$last->{$rso[0]} = "";
	$last->{$rso[1]} = "";
	$last->{$rso[2]} = "";
	$lasthost = "";
	$lastdate = "";
	$lastsuite = "";
	#$lastindent = 0;
	$thisindent = 0;
	print "<form method=get action=\"reconsile.cgi\">";
	print "<table border=1>\n";
	#print "<tr><th>Hostname<th>Date<th>Suite</tr>\n";
	print_headers();
	foreach $rp ( @ri ) {

		$this = ();
		$this->{$rso[0]} = $rp->{$rso[0]};
		$this->{$rso[1]} = $rp->{$rso[1]};
		$this->{$rso[2]} = $rp->{$rso[2]};
		$this->{$rso[3]} = $rp->{$rso[3]};

		# figure out the first column that is different
		for ($i = 0; $i <= $#rso; $i++) {
			if ($last->{$rso[$i]} ne $this->{$rso[$i]}) {
				$thisindent = $i;
				last;
			}
		}

		print "<tr>\n";
		for ($i = 0; $i < $thisindent; $i++) {
			print "<td>";

		}
		for ($i = $thisindent; $i <= $#rso; $i++) {
			print "<td>";
			if ($i == $#rso) {
				print "<a href=\"results.cgi?get_df=$this->{HOST}.$this->{DATE}.$this->{SUITE}.scanner\">";
			}
			print "$this->{$rso[$i]}";
			if ($i == $#rso) {
				print "</a>";
			}
			if ($i == $#rso) {
				# last column
				print " <input type=checkbox name=results value=\"$this->{HOST}.$this->{DATE}.$this->{SUITE}\">";

			}


		}
		print "</tr>\n";

		# make sure we update the $last... variables
		$last->{$rso[0]} = $this->{$rso[0]};
		$last->{$rso[1]} = $this->{$rso[1]};
		$last->{$rso[2]} = $this->{$rso[2]};
	}
	print "</table>\n";
	print "<input type=submit name=compare value=\"Compare\">\n";
	print "<input type=reset>\n";
	print "</form>";
	print end_html;

}

