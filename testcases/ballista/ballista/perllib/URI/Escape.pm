#
# $Id: Escape.pm,v 1.1 2004/10/18 17:58:13 mridge Exp $
#

package URI::Escape;
use strict;

=head1 NAME

URI::Escape - Escape and unescape unsafe characters

=head1 SYNOPSIS

 use URI::Escape;
 $safe = uri_escape("10% is enough\n");
 $verysafe = uri_escape("foo", "\0-\377");
 $str  = uri_unescape($safe);

=head1 DESCRIPTION

This module provide functions to escape and unescape URI strings.
Some characters are regarded as "unsafe" and must be escaped in
accordance with RFC 1738.  Escaped characters are represented by a
triplet consisting of the character "%" followed by two hexadecimal
digits.  The following functions are provided (and exported by
default):

=over 4

=item uri_escape($string, [$unsafe])

This function replaces all unsafe characters in the $string with their
escape sequence and return the result.

The uri_escape() function takes an optional second argument that
overrides the set of characters that are to be escaped.  The set is
specified as a string that can be used in a regular expression
character class (between [ ]).  E.g.:

  \x00-\x1f\x7f-\xff          # all control and hi-bit characters
  a-z                         # all lower case characters
  ^A-Za-z                     # everything not a letter

The default set of characters to be escaped is:

  \x00-\x20"#%;<>?{}|\\^~`\[\]\x7F-\xFF

=item uri_unescape($string)

Returns a string with all %XX sequences replaced with the actual
character.

=back

The module can also export the %escapes hash which contains the
mapping from all characters to the corresponding escape code.

=head1 SEE ALSO

L<URI::URL>


=head1 COPYRIGHT

Copyright 1995-1997 Gisle Aas.

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION);
use vars qw(%escapes);

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(uri_escape uri_unescape);
@EXPORT_OK = qw(%escapes);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

use Carp ();

# Build a char->hex map
for (0..255) {
    $escapes{chr($_)} = sprintf("%%%02X", $_);
}

my %subst;  # compiled patternes

sub uri_escape
{
    my($text, $patn) = @_;
    return undef unless defined $text;
    if (defined $patn){
	unless (exists  $subst{$patn}) {
	    # Because we can't compile regex we fake it with a cached sub
	    $subst{$patn} =
	      eval "sub {\$_[0] =~ s/([$patn])/\$escapes{\$1}/g; }";
	    Carp::croak("uri_escape: $@") if $@;
	}
	&{$subst{$patn}}($text);
    } else {
	# Default unsafe characters. (RFC1738 section 2.2)
	$text =~ s/([\x00-\x20\"#%;<>?{}|\\^~`\[\]\x7F-\xFF])/$escapes{$1}/g;
    }
    $text;
}

sub uri_unescape
{
    # Note from RFC1630:  "Sequences which start with a percent sign
    # but are not followed by two hexadecimal characters are reserved
    # for future extension"
    my @copy = @_;
    for (@copy) { s/%([\dA-Fa-f]{2})/chr(hex($1))/eg; }
    wantarray ? @copy : $copy[0];
}

1;
