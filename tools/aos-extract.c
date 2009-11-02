/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <string.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

int parse_header(struct aos_file *file, int *device)
{
	struct aos_block *block;
	unsigned char **keys;

	if(!aos_verify_magic(file)) {
		printf("error: Not an AOS2 file.\n");
		return 0;
	}
	
	keys = mpk_possible_aos_keys(aos_signature_type(file));
	if(!keys) {
		printf("error: Unknown signature type on file.\n");
		return 0;
	}
	
	if(!aos_detect_key(file, keys, MPK_KNOWN_DEVICES, device)) {
		printf("error: Could not verify signature.\n");
		return 0;
	}
	
	return 1;
}

int flash_individual(struct aos_file *file, const char *folder)
{
	struct aos_block *block;
	
	mkdir(folder, 0777);
	
	block = block_get(file, 5);
	while(block) {
		
		if(block->type == AOS_TYPE_FLASH) {
			struct aos_block_flash *flash = (struct aos_block_flash *)block->data;
			char *dest;
			
			dest = malloc(strlen(folder)+1 /* slash */+10 /* 0x00000000 */+1 /* zero */);
			if(!dest) {
				printf("error: Memory\n");
				return 0;
			}
			
			sprintf(dest, "%s/0x%08x", folder, flash->offset);
			file_write(dest, flash->data, flash->size);
			free(dest);
			
			printf("FLASH: offset 0x%08x, size %u\n", flash->offset, flash->size);
		}
		
		if(block->type == AOS_TYPE_MTD) {
			struct aos_block_mtd *mtd = (struct aos_block_mtd *)block->data;
			char *dest;
			
			dest = malloc(strlen(folder)+1/* slash */+strlen(mtd->name)+1/* underscore */+10 /* 0x00000000 */+1 /* zero */);
			if(!dest) {
				printf("error: Memory\n");
				return 0;
			}
			
			sprintf(dest, "%s/%s_0x%08x", folder, mtd->name, mtd->offset);
			file_write(dest, mtd->data, mtd->size);
			free(dest);
			
			printf("MTD: %s, at offset 0x%08x, size %u\n", mtd->name, mtd->offset, mtd->size);
		}
		
		block = block_next(file, block);
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
	struct aos_file *aos;
	unsigned int length;
	uint8_t *buffer;
	int device;
	
	printf("AOS extract utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc != 4) {
		printf("Usage: %s <command> <input> <output>\n\n", argv[0]);
		printf("Commands:\n");
		printf("\tdecrypt: write the decrypted .aos file into <output> file\n");
		printf("\tflash: extract any FLSH and MTDF blocks into the <output> folder\n");
		return 1;
	}
	
	// Load the file
	buffer = file_load(argv[2], &length);
	if(buffer == NULL)
		return 1;
	
	printf("File %s loaded, %u bytes.\n", argv[2], length);
	
	// Create the aos object
	aos = aos_create(buffer, length);
	if(aos == NULL) {
		printf("error: aos_create failed.\n");
		free(buffer);
		return 1;
	}
	
	// Parse & verify the header
	if(!parse_header(aos, &device)) {
		aos_free(aos);
		free(buffer);
		return 1;
	}
	
	printf("Successfully parsed AOS2 headers, detected device type %s\n", mpk_device_type(device));
	
	// Decrypt the file
	if(aos_is_encrypted(aos)) {
		if(!aos_decrypt_file(aos, AES_Keys[device])) {
			printf("error: Could not decrypt file.\n");
			
			aos_free(aos);
			free(buffer);
			return 1;
		}
		else
			printf("File was decrypted successfully.\n");
	}
	else
		printf("File is NOT encrypted.\n");
	
	printf("\n");
	
	// Write decrypted file to disk
	if(!strcasecmp(argv[1], "decrypt")) {
		file_write(argv[3], aos->data, aos->length);
		
		printf("Decrypted file written to %s\n", argv[3]);
	}
	// Extract FLSH and MTDF blocks
	else if(!strcasecmp(argv[1], "flash")) {
		// extract individual flash files
		flash_individual(aos, argv[3]);
	}
	
	printf("\n");
	printf("Done.\n");
	
	aos_free(aos);
	free(buffer);
	
	return 0;
}



