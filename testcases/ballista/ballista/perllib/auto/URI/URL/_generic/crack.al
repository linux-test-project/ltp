# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub crack
{
    my $self = shift;
    return $self unless wantarray;
    my @c = @{$self}{qw(scheme user password host port path params query frag)};
    if (!$c[0]) {
	# try to determine scheme
	my $base = $self->base;
	$c[0] = $base->scheme if $base;
	$c[0] ||= 'http';  # last resort, default in URI::URL::new
    }
    $c[4] ||= $self->default_port;
    @c;
}

1;
