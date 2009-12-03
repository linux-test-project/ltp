#!/usr/bin/perl

#  $Id: gcov2html.pl,v 1.10 2009/12/03 15:31:01 subrata_modak Exp $
 
#  (C) Copyright IBM Corp. 2004
 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
#  file and program are licensed under a BSD style license.  See
#  the Copying file included with the OpenHPI distribution for
#  full licensing terms.
 
#  Authors:
#      Sean Dague <http://dague.net/sean>

use strict;
use HTML::Entities;

foreach my $file (@ARGV) {
#    my $outfile = $file . ".html";
    open(IN,"$file");
    my @lines = <IN>;
    close(IN);
    
    my $html = make_html_head($file);
    $html .= make_html_body(parse_gcov(@lines));
    $html .= make_html_tail();

    print $html;
#    open(OUT,">$outfile");
#    print OUT $html;
#    close(OUT);
}

sub make_html_head {
    my $title = shift;
    $title =~ s/.*\/(.*)\.gcov$/$1/;
    return <<END;
<html>
<head><title>GCOV execution analyis for $title</title>
<link rel="stylesheet" href="/openhpi.css" type="text/css">
</head>
<body>
<div id="banner"><div><h1>The OpenHPI Project</h1><small>Open Hardware Platform Interface</small></div></div>
<table>
<tr>
<!--#include virtual="/sidebar.html" -->
<td id="maincolumn"><div class="mainsegment">
<h3>GCOV Execution Analysis for $title</h3>
<div>
The left column is the number of times the code was executed
during the unit test suites.
<p>
<table class="report">
<tr><th>Exec</th><th>&nbsp;</th><th>Code</th><th>Line #</th></tr>
END
}

sub make_html_tail {
    return <<END;
</table></div>
</div></td></tr></table>
</body>
</html>
END
}

sub set_status {
    my $exec = shift;
    if($exec eq "-") {
        return "na";
    } elsif($exec eq "#####") {
        return "notexec";
    } elsif($exec < 10) {
        return "low";
    } else {
        return "good";
    }
}

sub make_html_body {
    my $lines = shift;
    my $html;
    my $linecount = 1;
    foreach my $line (@$lines) {
        my $status = set_status($line->{exec});
        my $exec = ($status eq "na") ? " " : $line->{exec} + 0;
	
        my $data = $line->{data};
	$data =~ s{<(\S+)\@(.*?)\.(net|com)>}{< address removed >}ig; 
	$data = encode_entities($data);

        $html .= "<tr class='$status'><td align='right'><a name='line$linecount'>$exec</a></td><td>&nbsp;</td><td><pre class='report'>$data</pre></td><td align='right' class='linecount'>$linecount</td></tr>\n";
        $linecount++;
    }
    return $html;
}

sub parse_gcov {
    my @in = @_;
    my $lines = [];
    foreach my $line (@in) {
        if($line =~ /^\s+(.*?):\s+(.*?):(.*)/) {
            my $hash = {
                        exec => $1,
                        line => $2,
                        data => $3,
                       };
            push @$lines, $hash;
        }
    }
    return $lines;
}
