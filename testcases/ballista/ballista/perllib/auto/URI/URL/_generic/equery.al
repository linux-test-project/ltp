# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub equery   { shift->_elem('query',   @_); }

1;
