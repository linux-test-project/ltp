package URI::Heuristic;

# $Id: Heuristic.pm,v 1.1 2004/10/18 17:58:13 mridge Exp $

=head1 NAME

uf_urlstr - Expand URL using heuristics

=head1 SYNOPSIS

 use URI::Heuristic qw(uf_urlstr);
 $url = uf_urlstr("perl");             # http://www.perl.com
 $url = uf_urlstr("www.sol.no/sol");   # http://www.sol.no/sol
 $url = uf_urlstr("aas");              # http://www.aas.no
 $url = uf_urlstr("ftp.funet.fi");     # ftp://ftp.funet.fi
 $url = uf_urlstr("/etc/passwd");      # file:/etc/passwd

=head1 DESCRIPTION

This module provide functions that expand strings into real absolute
URLs using some builtin heuristics.  Strings that already represent
absolute URLs (i.e. start with a C<scheme:> part) are never modified
and are returned unchanged.  The main use of these functions are to
allow abbreviated URLs similar to what many web browsers allow for URLs
typed in by the user.

The following functions are provided:

=over 4

=item uf_urlstr($str)

The uf_urlstr() function will try to make the string passed as
argument into a proper absolute URL string.  The "uf_" prefix stands
for "User Friendly".

=item uf_url($str)

This functions work the same way as uf_urlstr() but it will
return a C<URI::URL> object.

=back

=head1 ENVIRONMENT

If the hostname portion of a URL does not contain any dots, then
certain qualified guesses will be made.  These guesses are governed be
the following two environment variables.

=over 10

=item COUNTRY

This is the two letter country code (ISO 3166) for your location.  If
the domain name of your host ends with two letters, then it is taken
to be the default country. See also L<Locale::Country>.

=item URL_GUESS_PATTERN

Contain a space separated list of URL patterns to try.  The string
"ACME" is for some reason used as a placeholder for the host name in
the URL provided.  Example:

 URL_GUESS_PATTERN="www.ACME.no www.ACME.se www.ACME.com"
 export URL_GUESS_PATTERN

Specifying URL_GUESS_PATTERN disables any guessing rules based on
country.  An empty URL_GUESS_PATTERN disables any guessing that
involves host name lookups.

=back

=head1 COPYRIGHT

Copyright 1997-1998, Gisle Aas

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut

use strict;

use vars qw(@EXPORT_OK $VERSION $MY_COUNTRY %LOCAL_GUESSING $DEBUG);

require Exporter;
*import = \&Exporter::import;
@EXPORT_OK = qw(uf_url uf_urlstr);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

eval {
    require Net::Domain;
    my $fqdn = Net::Domain::hostfqdn();
    $MY_COUNTRY = lc($1) if $fqdn =~ /\.([a-zA-Z]{2})$/;

    # Some other heuristics to guess country?  Perhaps looking
    # at some environment variable (LANG, LC_ALL, ???)
    $MY_COUNTRY = $ENV{COUNTRY} if exists $ENV{COUNTRY};
};

%LOCAL_GUESSING =
(
 'us' => [qw(www.ACME.gov www.ACME.mil)],
 'uk' => [qw(www.ACME.co.uk www.ACME.org.uk www.ACME.ac.uk)],
 'au' => [qw(www.ACME.com.au www.ACME.org.au www.ACME.edu.au)],
 'il' => [qw(www.ACME.co.il www.ACME.org.il www.ACME.net.il)],
 # send corrections and new entries to <aas@sn.no>
);


sub uf_url ($)
{
    require URI::URL;
    URI::URL->new(uf_urlstr($_[0]));
}


sub uf_urlstr ($)
{
    local($_) = @_;
    print STDERR "uf_urlstr: resolving $_\n" if $DEBUG;
    return unless defined;

    s/^\s+//;
    s/\s+$//;

    if (/^(www|web|home)\./) {
	$_ = "http://$_";

    } elsif (/^(ftp|gopher|news|wais|http|https)\./) {
	$_ = "$1://$_";

    } elsif (m,^/,      ||          # absolute file name
	     m,^\.\.?/, ||          # relative file name
	     m,^[a-zA-Z]:[/\\],)    # dosish file name
    {
	$_ = "file:$_";

    } elsif (/^\w+([\.\-]\w+)*\@(\w+\.)+\w{2,3}$/) {
	$_ = "mailto:$_";

    } elsif (!/^[.+\-\w]+:/) {      # no scheme specified
	if (s/^(\w+(?:\.\w+)*)([\/:\?\#]|$)/$2/) {
	    my $host = $1;

	    if ($host !~ /\./ && $host ne "localhost") {
		my @guess;
		if (exists $ENV{URL_GUESS_PATTERN}) {
		    @guess = map { s/\bACME\b/$host/; $_ }
		             split(' ', $ENV{URL_GUESS_PATTERN});
		} else {
		    if ($MY_COUNTRY) {
			my $special = $LOCAL_GUESSING{$MY_COUNTRY};
			if ($special) {
			    my @special = @$special;
			    push(@guess, map { s/\bACME\b/$host/; $_ }
                                               @special);
			} else {
			    push(@guess, "www.$host.$MY_COUNTRY");
			}
		    }
		    push(@guess, map "www.$host.$_",
			             "com", "org", "net", "edu", "int");
		}


		my $guess;
		for $guess (@guess) {
		    print STDERR "uf_urlstr: gethostbyname('$guess')..."
		      if $DEBUG;
		    if (gethostbyname($guess)) {
			print STDERR "yes\n" if $DEBUG;
			$host = $guess;
			last;
		    }
		    print STDERR "no\n" if $DEBUG;
		}
	    }
	    $_ = "http://$host$_";

	} else {
	    # pure junk, just return it unchanged...

	}
    }
    print STDERR "uf_urlstr: ==> $_\n" if $DEBUG;

    $_;
}

1;
