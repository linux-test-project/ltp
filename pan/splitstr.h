#ifndef _SPLITSTR_H_
#define _SPLITSTR_H_
/*
 * Synopsis
 *
 * const char **splitstr(const char *str, const char *separator, int *argcount)
 *
 * Description
 * This function splits a string (str) into components that are separated by
 * one or more of the characters in the (separator) string.  An array of
 * strings is returned, along with argcount being set to the number of strings
 * found.  Argcount can be NULL.  There will always be a NULL element in the
 * array after the last valid element.  If an error occurs, NULL will be
 * returned and argcount will be set to zero.
 *
 * To rid yourself of the memory allocated for splitstr(), pass the return
 * value from splitstr() unmodified to splitstr_free():
 *
 * void splitstr_free( const char ** return_from_splitstr );
 *
 */
const char **
splitstr(const char *, const char *, int *);

/*
 * splitster_free( const char ** )
 *
 * This takes the return value from splitster() and free()s memory
 * allocated by splitster.  Assuming: ret=splitster(...), this
 * requires that ret and *ret returned from splitster() have not
 * been modified.
 */
void
splitstr_free( const char ** );

#endif
