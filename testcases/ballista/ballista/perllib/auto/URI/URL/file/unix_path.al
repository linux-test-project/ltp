# NOTE: Derived from ./blib/lib/URI/URL/file.pm.  Changes made here will be lost.
package URI::URL::file;

sub unix_path
{
    my $self = shift;
    my @p;
    for ($self->path_components) {
	Carp::croak("Path component contains '/' or '\0'") if m|[\0/]|;
	if (@p) {
	    next unless length $_;   # skip empty path segments
	    next if $_ eq '.';       # skip these too
	    if ($_ eq '..' && $p[-1] ne '..') {  # go up one level
		pop(@p) if $p[-1] ne '';
		next;
	    }
	}
	push(@p, $_);
    }
    shift(@p) if @p > 1 && $p[0] eq '.';   # './' rendundant if there is more
    return '/' if !@p || (@p == 1 && $p[0] eq '');
    join('/', @p);
}

1;
