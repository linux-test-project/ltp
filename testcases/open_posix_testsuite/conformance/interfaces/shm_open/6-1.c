/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the effec of a name which does not begin with the slash
 * character because it is implementation-defined.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the effect of a name which does not begin with the slash\ncharacter because it is implementation-defined.\n");
        return PTS_UNTESTED;
}