#
# $Id: Message.pm,v 1.1 2004/10/18 17:58:15 mridge Exp $

package HTTP::Message;

=head1 NAME

HTTP::Message - Class encapsulating HTTP messages

=head1 SYNOPSIS

 package HTTP::Request;  # or HTTP::Response
 require HTTP::Message;
 @ISA=qw(HTTP::Message);

=head1 DESCRIPTION

A C<HTTP::Message> object contains some headers and a content (body).
The class is abstract, i.e. it only used as a base class for
C<HTTP::Request> and C<HTTP::Response> and should never instantiated
as itself.

The following methods are available:

=over 4

=cut

#####################################################################

require HTTP::Headers;
require Carp;
use strict;
use vars qw($VERSION $AUTOLOAD);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

=item $mess = new HTTP::Message;

This is the object constructor.  It should only be called internally
by this library.  External code should construct C<HTTP::Request> or
C<HTTP::Response> objects.

=cut

sub new
{
    my($class, $header, $content) = @_;
    if (defined $header) {
	Carp::croak("Bad header argument") unless ref $header;
	$header = $header->clone;
    } else {
	$header = HTTP::Headers->new;
    }
    $content = '' unless defined $content;
    bless {
	'_headers' => $header,
	'_content' => $content,
    }, $class;
}


=item $mess->clone()

Returns a copy of the object.

=cut

sub clone
{
    my $self  = shift;
    my $clone = HTTP::Message->new($self->{'_headers'}, $self->{'_content'});
    $clone;
}

=item $mess->protocol([$proto])

Sets the HTTP protocol used for the message.  The protocol() is a string
like "HTTP/1.0" or "HTTP/1.1".

=cut

sub protocol { shift->_elem('_protocol',  @_); }

=item $mess->content([$content])

The content() method sets the content if an argument is given.  If no
argument is given the content is not touched.  In either case the
previous content is returned.

=item $mess->add_content($data)

The add_content() methods appends more data to the end of the previous
content.

=cut

sub content   { shift->_elem('_content',  @_); }

sub add_content
{
    my $self = shift;
    if (ref($_[0])) {
	$self->{'_content'} .= ${$_[0]};  # for backwards compatability
    } else {
	$self->{'_content'} .= $_[0];
    }
}

=item $mess->content_ref

The content_ref() method will return a reference to content string.
It can be more efficient to access the content this way if the content
is huge, and it can be used for direct manipulation of the content,
for instance:

  ${$res->content_ref} =~ s/\bfoo\b/bar/g;

=cut

sub content_ref
{
    my $self = shift;
    \$self->{'_content'};
}

sub as_string
{
    "";  # To be overridden in subclasses
}

=item $mess->headers;

Return the embedded HTTP::Headers object.

=item $mess->headers_as_string([$endl])

Call the HTTP::Headers->as_string() method for the headers in the
message.

=cut

sub headers            { shift->{'_headers'};                }
sub headers_as_string  { shift->{'_headers'}->as_string(@_); }

=back

All unknown C<HTTP::Message> methods are delegated to the
C<HTTP::Headers> object that is part of every message.  This allows
convenient access to these methods.  Refer to L<HTTP::Headers> for
details of these methods:

  $mess->header($field => $val);
  $mess->scan(\&doit);
  $mess->push_header($field => $val);
  $mess->remove_header($field);

  $mess->date;
  $mess->expires;
  $mess->if_modified_since;
  $mess->if_unmodified_since;
  $mess->last_modified;
  $mess->content_type;
  $mess->content_encoding;
  $mess->content_length;
  $mess->content_language
  $mess->title;
  $mess->user_agent;
  $mess->server;
  $mess->from;
  $mess->referer;
  $mess->www_authenticate;
  $mess->authorization;
  $mess->proxy_authorization;
  $mess->authorization_basic;
  $mess->proxy_authorization_basic;

=cut


# delegate all other method calls the the _headers object.
sub AUTOLOAD
{
    my $self = shift;
    my $method = substr($AUTOLOAD, rindex($AUTOLOAD, '::')+2);
    return if $method eq "DESTROY";
    $self->{'_headers'}->$method(@_);
}

# Private method to access members in %$self
sub _elem
{
    my $self = shift;
    my $elem = shift;
    my $old = $self->{$elem};
    $self->{$elem} = $_[0] if @_;
    return $old;
}

1;

=head1 COPYRIGHT

Copyright 1995-1997 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
