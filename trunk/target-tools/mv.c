/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

/* Simple replacement for /bin/mv which does not require busybox */

#include <stdio.h>

int main(int argc, char *argv[])
{
	int i;
	
	if(argc != 3) {
		printf("Usage: mv <src> <dest>\n");
		return 1;
	}
	
	if(rename(argv[1], argv[2]) != 0) {
		perror("rename() error");
		return 1;
	}
	
	return 0;
}
