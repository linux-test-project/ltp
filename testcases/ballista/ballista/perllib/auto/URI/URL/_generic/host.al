# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub host     { shift->_netloc_elem('host',    @_); }

1;
