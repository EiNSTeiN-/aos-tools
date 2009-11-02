/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#ifndef __FILES_H
#define __FILES_H

uint8_t *file_load(char *filename, unsigned int *size);
int file_write(const char *filename, const char *buffer, unsigned int length);

char *bprintf(const char *fmt, ...);
int mkdir_recursive(const char *folder);

int log_clean(const char *filename);
int log_write(const char *filename, const char *format, ...);

#endif /* __FILES_H */
