/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#ifndef __FILES_H
#define __FILES_H

#include <stdint.h>

uint8_t *file_load(const char *filename, unsigned int *size);
int file_write(const char *filename, const char *buffer, unsigned int length);

#endif /* __FILES_H */
