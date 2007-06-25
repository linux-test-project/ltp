#!/usr/bin/perl
#
# Filter to make documentation look like website
#

my @files = @ARGV;

foreach my $file (@files) {
	system("tidy -c -m $file 2>/dev/null");
}

foreach my $file (@files) {
    local $/ = undef;
    open(IN,"<$file");
    my $contents = <IN>;
    close(IN);

    my $css = '<link rel="stylesheet" href="/openhpi.css" type="text/css">';
    my $banner = '<div id="banner"><div><h1>The OpenHPI Project</h1><small>Open Hardware Platform Interface</small></div></div><table><tr>';
    my $sidebar = '<!--#include virtual="/sidebar.html" -->';
    my $end = '</td></tr></table>';
    # we want to do only one of these
    if($contents =~ s{(</style>\s*)($css)*}{$1$css\n}is) {
        # tidy output
    } elsif ($contents =~ s{($css\s*)*</head}{$css\n</head}is) {
        # not tidy output
    }
    
    $contents =~ s{(<body.*?>\s*).*?(<div)}{$1$banner\n$sidebar\n<td id="maincolumn"><div class="mainsegment">\n$2}is;
    $contents =~ s{(</td></tr></table>\s*)*(</body>)}{</div></td></tr></table></body>}is;

    open(OUT,">$file");
    print OUT $contents;
    close(OUT);
}
