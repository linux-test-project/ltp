# NOTE: Derived from ./blib/lib/URI/URL.pm.  Changes made here will be lost.
package URI::URL;

sub rel { shift->clone; }

# This method should always be overridden in subclasses
1;
