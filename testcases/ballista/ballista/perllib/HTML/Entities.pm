package HTML::Entities;

# $Id: Entities.pm,v 1.1 2004/10/18 17:58:14 mridge Exp $

=head1 NAME

HTML::Entities - Encode or decode strings with HTML entities

=head1 SYNOPSIS

 use HTML::Entities;

 $a = "V&aring;re norske tegn b&oslash;r &#230res";
 decode_entities($a);
 encode_entities($a, "\200-\377");

=head1 DESCRIPTION

This module deals with encoding and decoding of strings with HTML
character entites.  The module provide the following functions:

=over 4

=item decode_entities($string)

This routine replaces HTML entities found in the $string with the
corresponding ISO-8859/1 character.  Unrecognized entities are left
alone.

=item endode_entities($string, [$unsafe_chars])

This routine replaces unsafe characters in $string with their entity
representation.  A second argument can be given to specify which
characters to concider as unsafe.  The default set of characters to
expand are control chars, high-bit chars and the '<', '&', '>' and '"'
character.

=back

Both routines modify the string passed in as the first argument if
called in void context.  In scalar and array context the encoded or
decoded string is returned (and the argument string is left
unchanged).

If you prefer not to import these routines into your namespace you can
call them as:

  use HTML::Entities ();
  $encoded = HTML::Entities::encode($a);
  $decoded = HTML::Entities::decode($a);

The module can also export the %char2entity and the %entity2char
hashes which contains the mapping from all characters to the
corresponding entities.

=head1 COPYRIGHT

Copyright 1995-1998 Gisle Aas. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION);
use vars qw(%entity2char %char2entity);

require 5.004;
require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(encode_entities decode_entities);
@EXPORT_OK = qw(%entity2char %char2entity);

$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);
sub Version { $VERSION; }


%entity2char = (
 # Some normal chars that have special meaning in SGML context
 amp    => '&',  # ampersand 
'gt'    => '>',  # greater than
'lt'    => '<',  # less than
 quot   => '"',  # double quote

 # PUBLIC ISO 8879-1986//ENTITIES Added Latin 1//EN//HTML
 AElig	=> 'Æ',  # capital AE diphthong (ligature)
 Aacute	=> 'Á',  # capital A, acute accent
 Acirc	=> 'Â',  # capital A, circumflex accent
 Agrave	=> 'À',  # capital A, grave accent
 Aring	=> 'Å',  # capital A, ring
 Atilde	=> 'Ã',  # capital A, tilde
 Auml	=> 'Ä',  # capital A, dieresis or umlaut mark
 Ccedil	=> 'Ç',  # capital C, cedilla
 ETH	=> 'Ð',  # capital Eth, Icelandic
 Eacute	=> 'É',  # capital E, acute accent
 Ecirc	=> 'Ê',  # capital E, circumflex accent
 Egrave	=> 'È',  # capital E, grave accent
 Euml	=> 'Ë',  # capital E, dieresis or umlaut mark
 Iacute	=> 'Í',  # capital I, acute accent
 Icirc	=> 'Î',  # capital I, circumflex accent
 Igrave	=> 'Ì',  # capital I, grave accent
 Iuml	=> 'Ï',  # capital I, dieresis or umlaut mark
 Ntilde	=> 'Ñ',  # capital N, tilde
 Oacute	=> 'Ó',  # capital O, acute accent
 Ocirc	=> 'Ô',  # capital O, circumflex accent
 Ograve	=> 'Ò',  # capital O, grave accent
 Oslash	=> 'Ø',  # capital O, slash
 Otilde	=> 'Õ',  # capital O, tilde
 Ouml	=> 'Ö',  # capital O, dieresis or umlaut mark
 THORN	=> 'Þ',  # capital THORN, Icelandic
 Uacute	=> 'Ú',  # capital U, acute accent
 Ucirc	=> 'Û',  # capital U, circumflex accent
 Ugrave	=> 'Ù',  # capital U, grave accent
 Uuml	=> 'Ü',  # capital U, dieresis or umlaut mark
 Yacute	=> 'Ý',  # capital Y, acute accent
 aacute	=> 'á',  # small a, acute accent
 acirc	=> 'â',  # small a, circumflex accent
 aelig	=> 'æ',  # small ae diphthong (ligature)
 agrave	=> 'à',  # small a, grave accent
 aring	=> 'å',  # small a, ring
 atilde	=> 'ã',  # small a, tilde
 auml	=> 'ä',  # small a, dieresis or umlaut mark
 ccedil	=> 'ç',  # small c, cedilla
 eacute	=> 'é',  # small e, acute accent
 ecirc	=> 'ê',  # small e, circumflex accent
 egrave	=> 'è',  # small e, grave accent
 eth	=> 'ð',  # small eth, Icelandic
 euml	=> 'ë',  # small e, dieresis or umlaut mark
 iacute	=> 'í',  # small i, acute accent
 icirc	=> 'î',  # small i, circumflex accent
 igrave	=> 'ì',  # small i, grave accent
 iuml	=> 'ï',  # small i, dieresis or umlaut mark
 ntilde	=> 'ñ',  # small n, tilde
 oacute	=> 'ó',  # small o, acute accent
 ocirc	=> 'ô',  # small o, circumflex accent
 ograve	=> 'ò',  # small o, grave accent
 oslash	=> 'ø',  # small o, slash
 otilde	=> 'õ',  # small o, tilde
 ouml	=> 'ö',  # small o, dieresis or umlaut mark
 szlig	=> 'ß',  # small sharp s, German (sz ligature)
 thorn	=> 'þ',  # small thorn, Icelandic
 uacute	=> 'ú',  # small u, acute accent
 ucirc	=> 'û',  # small u, circumflex accent
 ugrave	=> 'ù',  # small u, grave accent
 uuml	=> 'ü',  # small u, dieresis or umlaut mark
 yacute	=> 'ý',  # small y, acute accent
 yuml	=> 'ÿ',  # small y, dieresis or umlaut mark

 # Some extra Latin 1 chars that are listed in the HTML3.2 draft (21-May-96)
 copy   => '©',  # copyright sign
 reg    => '®',  # registered sign
 nbsp   => "\240", # non breaking space

 # Additional ISO-8859/1 entities listed in rfc1866 (section 14)
 iexcl  => '¡',
 cent   => '¢',
 pound  => '£',
 curren => '¤',
 yen    => '¥',
 brvbar => '¦',
 sect   => '§',
 uml    => '¨',
 ordf   => 'ª',
 laquo  => '«',
'not'   => '¬',    # not is a keyword in perl
 shy    => '­',
 macr   => '¯',
 deg    => '°',
 plusmn => '±',
 sup1   => '¹',
 sup2   => '²',
 sup3   => '³',
 acute  => '´',
 micro  => 'µ',
 para   => '¶',
 middot => '·',
 cedil  => '¸',
 ordm   => 'º',
 raquo  => '»',
 frac14 => '¼',
 frac12 => '½',
 frac34 => '¾',
 iquest => '¿',
'times' => '×',    # times is a keyword in perl
 divide => '÷',
);

# Make the oposite mapping
while (my($entity, $char) = each(%entity2char)) {
    $char2entity{$char} = "&$entity;";
}

# Fill inn missing entities
for (0 .. 255) {
    next if exists $char2entity{chr($_)};
    $char2entity{chr($_)} = "&#$_;";
}

my %subst;  # compiled encoding regexps


sub decode_entities
{
    my $array;
    if (defined wantarray) {
	$array = [@_]; # copy
    } else {
	$array = \@_;  # modify in-place
    }
    my $c;
    for (@$array) {
	s/(&\#(\d+);?)/$2 < 256 ? chr($2) : $1/eg;
	s/(&\#[xX]([0-9a-fA-F]+);?)/$c = hex($2); $c < 256 ? chr($c) : $1/eg;
	s/(&(\w+);?)/$entity2char{$2} || $1/eg;
    }
    wantarray ? @$array : $array->[0];
}

sub encode_entities
{
    my $ref;
    if (defined wantarray) {
	my $x = $_[0];
	$ref = \$x;     # copy
    } else {
	$ref = \$_[0];  # modify in-place
    }
    if (defined $_[1]) {
	unless (exists $subst{$_[1]}) {
	    # Because we can't compile regex we fake it with a cached sub
	    $subst{$_[1]} =
	      eval "sub {\$_[0] =~ s/([$_[1]])/\$char2entity{\$1}/g; }";
	    die $@ if $@;
	}
	&{$subst{$_[1]}}($$ref);
    } else {
	# Encode control chars, high bit chars and '<', '&', '>', '"'
	$$ref =~ s/([^\n\t !\#\$%\'-;=?-~])/$char2entity{$1}/g;
    }
    $$ref;
}

# Set up aliases
*encode = \&encode_entities;
*decode = \&decode_entities;

1;
