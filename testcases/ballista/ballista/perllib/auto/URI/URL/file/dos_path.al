# NOTE: Derived from ./blib/lib/URI/URL/file.pm.  Changes made here will be lost.
package URI::URL::file;

sub dos_path
{
    my $self = shift;
    my @p;
    for ($self->path_components) {
	Carp::croak("Path component contains '/' or '\\'") if m|[/\\]|;
	push(@p, uc $_);
    }
    my $p = join("\\", @p);
    $p =~ s/^\\([A-Z]:)/$1/;  # Fix drive letter specification
    $p;
}

1;
