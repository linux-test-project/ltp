#
# $Id: Request.pm,v 1.1 2004/10/18 17:58:15 mridge Exp $

package HTTP::Request;

=head1 NAME

HTTP::Request - Class encapsulating HTTP Requests

=head1 SYNOPSIS

 require HTTP::Request;
 $request = HTTP::Request->new(GET => 'http://www.oslonett.no/');

=head1 DESCRIPTION

C<HTTP::Request> is a class encapsulating HTTP style requests,
consisting of a request line, some headers, and some (potentially empty)
content. Note that the LWP library also uses this HTTP style requests
for non-HTTP protocols.

Instances of this class are usually passed to the C<request()> method
of an C<LWP::UserAgent> object:

 $ua = LWP::UserAgent->new;
 $request = HTTP::Request->new(GET => 'http://www.oslonett.no/');
 $response = $ua->request($request);

C<HTTP::Request> is a subclass of C<HTTP::Message> and therefore
inherits its methods.  The inherited methods often used are header(),
push_header(), remove_header(), headers_as_string() and content().
See L<HTTP::Message> for details.

The following additional methods are available:

=over 4

=cut

require HTTP::Message;
@ISA = qw(HTTP::Message);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

use URI::URL ();
use strict;

=item $r = HTTP::Request->new($method, $url, [$header, [$content]])

Constructs a new C<HTTP::Request> object describing a request on the
object C<$url> using method C<$method>.  The C<$url> argument can be
either a string, or a reference to a C<URI::URL> object.  The $header
argument should be a reference to an C<HTTP::Headers> object.

=cut

sub new
{
    my($class, $method, $url, $header, $content) = @_;
    my $self = $class->SUPER::new($header, $content);
    $self->method($method);
    $self->url($url);
    $self;
}


sub clone
{
    my $self = shift;
    my $clone = bless $self->SUPER::clone, ref($self);
    $clone->method($self->method);
    $clone->url($self->url);
    $clone;
}


=item $r->method([$val])

=item $r->url([$val])

These methods provide public access to the member variables containing
respectively the method of the request and the URL object of the
request.

If an argument is given the member variable is given that as its new
value. If no argument is given the value is not touched. In either
case the previous value is returned.

The url() method accept both a reference to a URI::URL object and a
string as its argument.  If a string is given, then it should be
parseable as an absolute URL.

=cut

sub method  { shift->_elem('_method', @_); }

sub url
{
    my $self = shift;
    my $old = $self->{'_url'};
    if (@_) {
	my $url = shift;
	if (!defined $url) {
	    # that's ok
	} elsif (ref $url) {
	    $url = $url->abs;
	} else {
	    $url = eval { URI::URL->new($url) };
	}
	$self->{'_url'} = $url;
    }
    $old;
}

*uri = \&url;  # this is the same for now

=item $r->as_string()

Method returning a textual representation of the request.
Mainly useful for debugging purposes. It takes no arguments.

=cut

sub as_string
{
    my $self = shift;
    my @result;
    #push(@result, "---- $self -----");
    my $req_line = $self->method || "[NO METHOD]";
    my $url = $self->url;
    $url = (defined $url) ? $url->as_string : "[NO URL]";
    $req_line .= " $url";
    my $proto = $self->protocol;
    $req_line .= " $proto" if $proto;

    push(@result, $req_line);
    push(@result, $self->headers_as_string);
    my $content = $self->content;
    if (defined $content) {
	push(@result, $content);
    }
    #push(@result, ("-" x 40));
    join("\n", @result, "");
}

1;

=back

=head1 SEE ALSO

L<HTTP::Headers>, L<HTTP::Message>, L<HTTP::Request::Common>

=head1 COPYRIGHT

Copyright 1995-1998 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
