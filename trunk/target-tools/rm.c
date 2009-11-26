/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

/* Simple replacement for /bin/rm which does not require busybox */

#include <stdio.h>

int main(int argc, char *argv[])
{
	int i;
	
	if(argc < 2) {
		printf("Usage: rm <file1> [file...]\n");
		return 1;
	}
	
	for(i=1;i<argc;i++) {
		if(remove(argv[i]) != 0)
			perror(argv[i]);
	}
	
	return 0;
}
