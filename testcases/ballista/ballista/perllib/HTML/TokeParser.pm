package HTML::TokeParser;

# $Id: TokeParser.pm,v 1.1 2004/10/18 17:58:14 mridge Exp $

require HTML::Parser;
@ISA=qw(HTML::Parser);

use strict;
use Carp qw(croak);
use HTML::Entities qw(decode_entities);


sub new
{
    my $class = shift;
    my $file = shift;
    croak "Usage: $class->new(\$file)" unless defined $file;
    unless (ref $file) {
	require IO::File;
	$file = IO::File->new($file, "r") || return;
    }
    my $self = $class->SUPER::new;
    $self->{file} = $file;
    $self->{tokens} = [];
    $self->{textify} = {img => "alt", applet => "alt"};
    $self;
}

# Set up callback methods
for (qw(declaration start end text comment)) {
    my $t = uc(substr($_,0,1));
    no strict 'refs';
    *$_ = sub { my $self = shift; push(@{$self->{tokens}}, [$t, @_]) };
}


sub get_token
{
    my $self = shift;
    while (!@{$self->{tokens}} && $self->{file}) {
	# must try to parse more of the file
	my $buf;
	if (read($self->{file}, $buf, 512)) {
	    $self->parse($buf);
	} else {
	    $self->eof;
	    delete $self->{file};
	}
    }
    shift @{$self->{tokens}};
}


sub unget_token
{
    my $self = shift;
    unshift @{$self->{tokens}}, @_;
    $self;
}


sub get_tag
{
    my $self = shift;
    my $wanted = shift;
    my $token;
  GET_TOKEN:
    {
	$token = $self->get_token;
	if ($token) {
	    my $type = shift @$token;
	    redo GET_TOKEN if $type !~ /^[SE]$/;
	    substr($token->[0], 0, 0) = "/" if $type eq "E";
	    redo GET_TOKEN if defined($wanted) && $token->[0] ne $wanted;
	}
    }
    $token;
}


sub get_text
{
    my $self = shift;
    my $endat = shift;
    my @text;
    while (my $token = $self->get_token) {
	my $type = $token->[0];
	if ($type eq "T") {
	    push(@text, decode_entities($token->[1]));
	} elsif ($type =~ /^[SE]$/) {
	    my $tag = $token->[1];
	    if ($type eq "S") {
		if (exists $self->{textify}{$tag}) {
		    my $alt = $self->{textify}{$tag};
		    my $text;
		    if (ref($alt)) {
			$text = &$alt(@$token);
		    } else {
			$text = $token->[2]{$alt || "alt"};
			$text = "[\U$tag]" unless defined $text;
		    }
		    push(@text, $text);
		    next;
		}
	    } else {
		$tag = "/$tag";
	    }
	    if (!defined($endat) || $endat eq $tag) {
		 $self->unget_token($token);
		 last;
	    }
	}
    }
    join("", @text);
}


sub get_trimmed_text
{
    my $self = shift;
    my $text = $self->get_text(@_);
    $text =~ s/^\s+//; $text =~ s/\s+$//; $text =~ s/\s+/ /g;
    $text;
}

1;


__END__

=head1 NAME

HTML::TokeParser - Alternative HTML::Parser interface

=head1 SYNOPSIS

 require HTML::TokeParser;
 $p = HTML::TokeParser->new("index.html") || die "Can't open: $!";
 while (my $token = $p->get_token) {
     #...
 }

=head1 DESCRIPTION

The HTML::TokeParser is an alternative interface to the HTML::Parser class.
It basically turns the HTML::Parser inside out.  You associate a file
(or any IO::Handle object) with the parser at construction time and
then repeatedly call $parser->get_token to obtain the tags and text
found in the parsed document.  No need to make a subclass to make the
parser do anything.

Calling the methods defined by the HTML::Parser base class will be
confusing, so don't do that.  Use the following methods instead:

=over 4

=item $p = HTML::TokeParser->new( $file );

The object constructor needs a file name or a reference to some file
handle object as argument.  If a file name (plain scalar) is passed to
the constructor and the file can't be opened for reading, then the
constructor will return an undefined value.

=item $p->get_token

This method will return the next I<token> found in the HTML document,
or C<undef> at the end of the document.  The token is returned as an
array reference.  The first element of the array will be a single
character string denoting the type of this token; "S" for start tag,
"E" for end tag, "T" for text, "C" for comment, and "D" for
declaration.  The rest of the array is the same as the arguments
passed to the HTML::Parser callbacks (see L<HTML::Parser>).  This
summarize the tokens that can occur:

  ["S", $tag, %$attr, @$attrseq, $origtext]
  ["E", $tag, $origtext]
  ["T", $text]
  ["C", $text]
  ["D", $text]

=item $p->unget_token($token,...)

If you find out you have read too many tokens you can push them back,
so that they are returned the next time $p->get_token is called.

=item $p->get_tag( [$tag] )

This method return the next tag (skipping any other tokens), or undef
if there is no more tags in the document.  If an argument is given,
then we skip tokens until the specified tag is found.  The tags are
returned as a hash reference of the same form as for $p->get_token
above, but the type code (first element) is missing and the name of
end tags is prefixed with "/".  This means that the tags returned look
like this:

  [$tag, %$attr, @$attrseq, $origtext]
  ["/$tag", $origtext]

=item $p->get_text( [$endtag] )

This method returns all text found at the current position. It might
return a zero length string if there is no text.  The optional $endtag
argument specify that any text occurring before the given tag is to be
returned.  Any entities will be expanded to their corresponding
character.

The $p->{textify} attribute is a hash that define how certain tags can
be treated as text.  If the name of a start tag match a key in this
hash then this tag is converted to text.  The hash value is used to
specify which tag attribute to obtain the text from.  If this
attribute is missing, then the upper case name of the tag enclosed in
brackets is returned, e.g. "[IMG]".  The hash value can also be a
subroutine reference.  In this case the routine is called with the
token content as parameters to obtain the text.

The default $p->{textify} value is:

  {img => "alt", applet => "alt"}

This means that <IMG> and <APPLET> tags are treated as text, and that
the text to substitute can be found as ALT attribute.

=item $p->get_trimmed_text( [$endtag] )

Same as $p->get_text above, but will collapse any sequence of white
space to a single space character.  Leading and trailing space is
removed.

=back

=head1 EXAMPLES

This example extract all links from a document.  It will print one
line for each link, containing the URL and the textual description
between the <A>...</A> tags:

  use HTML::TokeParser;
  $p = HTML::TokeParser->new(shift||"index.html");

  while (my $token = $p->get_tag("a")) {
      my $url = $token->[1]{href} || "-";
      my $text = $p->get_trimmed_text("/a");
      print "$url\t$text\n";
  }

This example extract the <TITLE> from the document:

  use HTML::TokeParser;
  $p = HTML::TokeParser->new(shift||"index.html");
  if ($p->get_tag("title")) {
      my $title = $p->get_trimmed_text;
      print "Title: $title\n";
  }

=head1 SEE ALSO

L<HTML::Parser>

=head1 COPYRIGHT

Copyright 1998 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
