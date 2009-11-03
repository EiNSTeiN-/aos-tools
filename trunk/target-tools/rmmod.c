/* rmmod.c: remove a module from the kernel.
    Copyright (C) 2001  Rusty Russell.
    Copyright (C) 2002  Rusty Russell, IBM Corporation.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <asm/unistd.h>
#include <stdarg.h>
#include <getopt.h>
#include <syslog.h>

extern long delete_module(const char *, unsigned int);

int rmmod(const char *modname, int flags)
{
	long ret;

	ret = delete_module(modname, flags);
	if (ret != 0) {
		fprintf(stderr, "rmmod: can't remove '%s': %s\n",
			modname, strerror(errno));
	}
	
	return ret;
}

