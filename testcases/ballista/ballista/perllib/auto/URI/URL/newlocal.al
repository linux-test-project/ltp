# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

sub newlocal
{
    require URI::URL::file;
    my $class = shift;
    URI::URL::file->newlocal(@_);  # pass it on the the file class
}

1;
