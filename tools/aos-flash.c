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

int flash_extract_cpio(struct flash_file *file, const char *output)
{
	unsigned int length;
	uint8_t *data;
	
	if(!file->header->cpio) {
		printf("error: No CPIO offset in file header.\n");
		return 0;
	}
	
	if(file->header->cpio >= file->length) {
		printf("error: CPIO offset is past end of file.\n");
		return 0;
	}
	
	data = (uint8_t *)file->header+file->header->cpio;
	length = (file->length - file->header->cpio);
	
	if(*(uint32_t *)data != AOS_GZIP_MAGIC) {
		printf("error: Could not find GZIP magic at given offset.\n");
		return 0;
	}
	
	mkdir_recursive(output);
	file_write(output, data, length);
	
	printf("Gzip archive successfully written to %s\n", output);
	
	return 1;
}

int flash_insert_cpio(struct flash_file *file, const char *input, const char *output)
{
	unsigned int length, newlength;
	uint8_t *data, *newdata;
	
	if(!file->header->cpio) {
		printf("error: No CPIO offset in file header.\n");
		return 0;
	}
	
	if(file->header->cpio >= file->length) {
		printf("error: CPIO offset is past end of file.\n");
		return 0;
	}
	
	data = file_load(input, &length);
	if(!data) {
		printf("error: Could not load file %s\n", input);
		return 0;
	}
	
	if(*(uint32_t *)data != AOS_GZIP_MAGIC) {
		printf("error: Not a gzip magic in file %s.\n", input);
		return 0;
	}
	
	newlength = file->header->cpio + length;
	newdata = malloc(newlength);
	if(!newdata) {
		printf("error: Memory\n");
		free(data);
		return 0;
	}
	
	// copy the old file up to the cpio
	memcpy(newdata, &file->header->magic, file->header->cpio);
	
	// copy the new cpio after the old file
	memcpy(&newdata[file->header->cpio], data, length);
	
	// Set the new file size
	((struct flash_header *)newdata)->filesize = newlength;
	
	mkdir_recursive(output);
	file_write(output, newdata, newlength);
	
	printf("New cpio successfully written to %s\n", output);
	
	free(data);
	free(newdata);
	
	return 1;
}

int main(int argc, char *argv[])
{
	struct flash_file *flash;
	unsigned int length;
	uint8_t *buffer;
	int device;
	
	printf("AOS flash utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc != 2 && argc != 4) {
		printf("Usage: %s <input> [-x file] [-i file]\n\n", argv[0]);
		printf("The <input> file can be one of the following raw flash partitions\n");
		printf("as extracted by aos-extract or aos-unpack.\n");
		printf("\t1. the second stage bootloader\n");
		printf("\t2. the init or recovery cpio\n");
		printf("\n");
		printf("The default behavior is to validate the signature on the file.\n");
		printf("\n");
		printf("Optional commands, for cpio files only:\n");
		printf("\t-x: extract the .cpio.gz to 'file'\n");
		printf("\t-i: insert the specified .cpio.gz 'file' into the 'input' flash partition.\n");
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
	
	// Detect file type
	switch(flash->header->magic) {
		case AOS_ZMfX_MAGIC: {
			printf("Detected ZMfX magic (kernel).\n");
			break;
		}
		case AOS_CPIO_MAGIC: {
			printf("Detected CPIO magic (init or recovery cpio).\n");
			
			if(!strcasecmp(argv[2], "-x")) {
				flash_extract_cpio(flash, argv[3]);
			}
			else if(!strcasecmp(argv[2], "-i")) {
				flash_insert_cpio(flash, argv[3], argv[1]);
			}
			
			break;
		}
		default: {
			printf("Could NOT detect file type.\n");
			break;
		}
	}
	
	printf("\n");
	printf("Done.\n");
	
	flash_free(flash);
	free(buffer);
	
	return 0;
}



