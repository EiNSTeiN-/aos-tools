/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

int main(int argc, char *argv[])
{
	struct flash_file *flash;
	unsigned int length;
	uint8_t *buffer;
	int device;
	
	printf("AOS resize utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc != 2 && argc != 4) {
		printf("Usage: %s <input>\n\n", argv[0]);
		printf("The <input> file can be one of the following raw flash partitions or file\n");
		printf("as extracted by aos-extract or aos-unpack.\n");
		printf("\t1. the second stage bootloader\n");
		printf("\t2. the init or recovery cpio\n");
		printf("\t3. a .cramfs.secure file\n");
		printf("\n");
		printf("This utility will fix the 'filesize' field in the file header.\n");
		return 1;
	}
	
	// Load the file
	buffer = file_load(argv[1], &length);
	if(buffer == NULL) {
		printf("error: Could not load %s\n", argv[1]);
		return 1;
	}
	
	printf("File %s loaded, %u bytes.\n", argv[1], length);
	
	// Create the flash object
	flash = flash_create(buffer, length);
	if(flash == NULL) {
		printf("error: flash_create failed.\n");
		free(buffer);
		return 1;
	}
	
	// Parse & verify the header
	if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &device)) {
		printf("error: Signautre verification failed (continuing anyway...)\n");
	}
	else {
		printf("Successfully verified file signature, detected device type %s\n", mpk_device_type(device));
	}
	
	// Fix the header
	if(flash->header->filesize == length) {
		printf("File size was correct, file is unchanged\n");
	}
	else {
		flash->header->filesize = length;
		file_write(argv[1], buffer, length);
	}
	
	printf("\n");
	printf("Done.\n");
	
	flash_free(flash);
	free(buffer);
	
	return 0;
}



