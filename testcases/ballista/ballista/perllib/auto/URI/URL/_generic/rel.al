# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

# The oposite of $url->abs.  Return a URL as much relative as possible
sub rel {
    my($self, $base) = @_;
    my $rel = $self->clone;
    $base = $self->base unless $base;
    return $rel unless $base;
    $base = new URI::URL $base unless ref $base;
    $rel->base($base);

    my($scheme, $netloc, $path) = @{$rel}{qw(scheme netloc path)};
    if (!defined($scheme) && !defined($netloc)) {
	# it is already relative
	return $rel;
    }

    my($bscheme, $bnetloc, $bpath) = @{$base}{qw(scheme netloc path)};
    for ($bscheme, $bnetloc, $netloc) { $_ = '' unless defined }

    unless ($scheme eq $bscheme && $netloc eq $bnetloc) {
	# different location, can't make it relative
	return $rel;
    }

    for ($path, $bpath) {  $_ = "/$_" unless m,^/,; }

    # Make it relative by eliminating scheme and netloc
    $rel->{'scheme'} = undef;
    $rel->netloc(undef);

    # This loop is based on code from Nicolai Langfeldt <janl@ifi.uio.no>.
    # First we calculate common initial path components length ($li).
    my $li = 1;
    while (1) {
	my $i = index($path, '/', $li);
	last if $i < 0 ||
                $i != index($bpath, '/', $li) ||
	        substr($path,$li,$i-$li) ne substr($bpath,$li,$i-$li);
	$li=$i+1;
    }
    # then we nuke it from both paths
    substr($path, 0,$li) = '';
    substr($bpath,0,$li) = '';

    if ($path eq $bpath && defined($rel->frag) && !defined($rel->equery)) {
        $rel->epath('');
    } else {
        # Add one "../" for each path component left in the base path
        $path = ('../' x $bpath =~ tr|/|/|) . $path;
	$path = "./" if $path eq "";
        $rel->epath($path);
    }

    $rel;
}

1;
