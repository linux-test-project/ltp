package URI::URL::gopher;
require URI::URL::_generic;
@ISA = qw(URI::URL::_generic);

use URI::Escape qw(uri_unescape);

sub default_port { 70 }

sub _parse {
    my($self, $init)   = @_;
    $self->URI::URL::_generic::_parse($init, qw(netloc path));
    $self->_parse_gopherpath;
}

sub path {
    my $self = shift;
    my $old = $self->URI::URL::_generic::path(@_);
    return $old unless @_;
    $self->_parse_gopherpath;
    $old;
}

sub epath {
    my $self = shift;
    my $old = $self->URI::URL::_generic::epath(@_);
    return $old unless @_;
    $self->_parse_gopherpath;
    $old;
}

sub _parse_gopherpath {
    my $self = shift;
    my $p = $self->{'path'};
    # not according to RFC1738, but many popular browsers accept
    # gopher URLs with a '?' before the search string.
    $p =~ s/\?/\t/;
    $p = uri_unescape($p);

    if (defined($p) && $p ne '/' && $p =~ s!^/?(.)!!) {
	$self->{'gtype'} = $1;
    } else {
	$self->{'gtype'} = "1";
	$p = "";
    }

    delete $self->{'selector'};
    delete $self->{'search'};
    delete $self->{'string'};

    my @parts = split(/\t/, $p, 3);
    $self->{'selector'} = shift @parts if @parts;
    $self->{'search'}   = shift @parts if @parts;
    $self->{'string'}   = shift @parts if @parts;
}


sub gtype    { shift->_path_elem('gtype',    @_); }
sub selector { shift->_path_elem('selector', @_); }
sub search   { shift->_path_elem('search',   @_); }
sub string   { shift->_path_elem('string',   @_); }

sub _path_elem {
    my($self, $elem, @val) = @_;
    my $old = $self->_elem($elem, @val);
    return $old unless @val;

    # construct new path based on elements
    my $path = "/$self->{'gtype'}";
    $path .= "$self->{'selector'}" if defined $self->{'selector'};
    $path .= "\t$self->{'search'}" if defined $self->{'search'};
    $path .= "\t$self->{'string'}" if defined $self->{'string'};
    $self->{'path'} = $path;

    $old;
}

*params  = \&URI::URL::bad_method;
*qparams = \&URI::URL::bad_method;
*query   = \&URI::URL::bad_method;
*equery  = \&URI::URL::bad_method;

1;
