/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "files.h"

uint8_t *file_load(char *filename, unsigned int *size)
{
	FILE *file;
	long filesize, readsize;
	uint8_t *buffer;
	
	file = fopen(filename, "r+b");
	if(!file) {
		printf("fopen: error opening file\n");
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	
	if(filesize <= 0) {
		printf("filesize <= 0: filesize is %lu\n", filesize);
		fclose(file);
		return NULL;
	}
	
	buffer = malloc(filesize);
	if(!buffer) {
		printf("malloc: error allocating %lu bytes\n", filesize);
		fclose(file);
		return NULL;
	}
	
	fseek(file, 0, SEEK_SET);
	readsize = fread(buffer, 1, filesize, file);
	if(readsize != filesize) {
		printf("fread: error reading file: wanted %lu bytes and got %lu bytes\n", filesize, readsize);
		fclose(file);
		return NULL;
	}
	fclose(file);
	
	if(size)
		*size = readsize;
	
	return buffer;
}

int file_write(const char *filename, const char *buffer, unsigned int length)
{
	FILE *file;
	long writesize;
	
	file = fopen(filename, "w+b");
	if(!file) {
		printf("fopen: error opening file: %s\n", filename);
		return 0;
	}
	
	writesize = fwrite(buffer, 1, length, file);
	if(writesize != length) {
		printf("fwrite: error reading file: wanted %u bytes and got %lu bytes\n", length, writesize);
		fclose(file);
		return 0;
	}
	fclose(file);
	
	return 1;
}

int mkdir_recursive(const char *folder)
{
	char *tempdir, *ptr;
	unsigned int i;
	
	if(!strchr(folder, '/'))
		return 0;
	
	tempdir = strdup(folder);
	if(!tempdir)
		return 0;
	
	ptr = tempdir;
	while(1) {
		ptr = strchr(ptr, '/');
		if(!ptr)
			break;
		
		*ptr = 0;
		mkdir(tempdir, 0777);
		*ptr = '/';
		
		ptr++;
	}
	free(tempdir);
	
	return 1;
}

/* more practical form of asprintf() */
char *bprintf(const char *fmt, ...)
{
	char *strp = NULL;

	va_list args;
	va_start(args, fmt);
	vasprintf(&strp, fmt, args);
	va_end(args);

	return strp;
}

int log_clean(const char *filename)
{
	remove(filename);
	return 1;
}

int log_write(const char *filename, const char *format, ...)
{
	int result;
	char *str = NULL;
	FILE* file;
	
	va_list args;
	va_start(args, format);
	result = vasprintf(&str, format, args);
	if(result == -1)
		return 0;
	va_end(args);
	
	file = fopen(filename,"a");
	if(!file) {
		free(str);
		return 0;
	}
	
	if(fwrite(str,strlen(str),1,file) != 1) {
		free(str);
		fclose(file);
		return 0;
	}
	
	free(str);
	fclose(file);
	
	return 1;
}
