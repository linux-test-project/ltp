# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

# Generic-RL: Resolving Relative URL into an Absolute URL
#
# Based on RFC1808 section 4
#
sub abs
{
    my($self, $base, $allow_relative_scheme) = @_;
    $allow_relative_scheme = $URI::URL::ABS_ALLOW_RELATIVE_SCHEME if @_ < 3;
    my $embed = $self->clone;

    $base = $self->base unless $base;      # default to default base
    return $embed unless $base;            # we have no base (step1)

    $base = new URI::URL $base unless ref $base; # make obj if needed

    my($scheme, $host, $path, $params, $query, $frag) =
	@{$embed}{qw(scheme host path params query frag)};

    # just use base if we are empty             (2a)
    return $base->clone
      unless grep(defined($_) && $_ ne '',
		  $scheme,$host,$port,$path,$params,$query,$frag);

    # if we have a scheme we must already be absolute   (2b),
    #
    # but sec. 5.2 also says: Some older parsers allow the scheme name
    # to be present in a relative URL if it is the same as the base
    # URL scheme.  This is considered to be a loophole in prior
    # specifications of the partial URLs and should be avoided by
    # future parsers.
    #
    # The old behavoir can be enabled by passing a TRUE value to the
    # $allow_relative_scheme parameter.
    return $embed if $scheme &&
      (!$allow_relative_scheme || $scheme ne $base->{'scheme'});

    $embed->{'_str'} = '';                      # void cached string
    $embed->{'scheme'} = $base->{'scheme'};     # (2c)

    return $embed if $embed->{'netloc'};        # (3)
    $embed->netloc($base->{'netloc'});          # (3)

    return $embed if $path =~ m:^/:;            # (4)

    if ($path eq '') {                          # (5)
	$embed->{'path'} = $base->{'path'};     # (5)

	return $embed if defined $embed->{'params'}; # (5a)
	$embed->{'params'} = $base->{'params'};      # (5a)

	return $embed if defined $embed->{'query'};  # (5b)
	$embed->{'query'} = $base->{'query'};        # (5b)

	return $embed;
    }

    # (Step 6)  # draft 6 suggests stack based approach

    my $basepath = $base->{'path'};
    my $relpath  = $embed->{'path'};

    $basepath =~ s!^/!!;
    $basepath =~ s!/$!/.!;                # prevent empty segment
    my @path = split('/', $basepath);     # base path into segments
    pop(@path);                           # remove last segment

    $relpath =~ s!/$!/.!;                 # prevent empty segment

    push(@path, split('/', $relpath));    # append relative segments

    my @newpath = ();
    my $isdir = 0;
    my $segment;

    foreach $segment (@path) {            # left to right
	if ($segment eq '.') {            # ignore "same" directory
	    $isdir = 1;
	}
	elsif ($segment eq '..') {
	    $isdir = 1;
	    my $last = pop(@newpath);
	    if (!defined $last) {         # nothing to pop
		push(@newpath, $segment); # so must append
	    }
	    elsif ($last eq '..') {       # '..' cannot match '..'
		# so put back again, and append
		push(@newpath, $last, $segment);
	    }
	    #else
		# it was a component,
		# keep popped
	} else {
	    $isdir = 0;
	    push(@newpath, $segment);
	}
    }

    if ($URI::URL::ABS_REMOTE_LEADING_DOTS) {
	shift @newpath while @newpath && $newpath[0] =~ /^\.\.?$/;
    }

    $embed->{'path'} = '/' . join('/', @newpath) .
	($isdir && @newpath ? '/' : '');

    $embed;
}

1;
