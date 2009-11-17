/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

int info_parse_unit(struct aos_file *file)
{
	struct aos_block *block;
	struct aos_block_unit *unit;
	
	block = block_get(file, AOS_UNIT_BLOCK_ID);
	if(!block) {
		printf("error: Could not get UNIT block.\n");
		return 0;
	}
	
	if(block->type != AOS_TYPE_UNIT) {
		printf("error: Expected UNIT (%08x) block, got %c%c%c%c (%08x)\n", AOS_TYPE_UNIT, 
			((uint8_t *)&block->type)[0], ((uint8_t *)&block->type)[1], ((uint8_t *)&block->type)[2],
			((uint8_t *)&block->type)[3], block->type);
		return 0;
	}
	
	unit = (void *)block->data;
	
	printf("UNIT: Product Name: %s\n", unit->product_name);
	if(unit->product_key[0] == 0)
		printf("UNIT: Product Key: (none)\n");
	else
		printf("UNIT: Product Key: %s\n", unit->product_key);
	
	return 1;
}

int info_parse_version(struct aos_file *file)
{
	struct aos_block *block;
	struct aos_block_version *version;
	
	block = block_get(file, AOS_VERSION_BLOCK_ID);
	if(!block) {
		printf("error: Could not get VERSION block.\n");
		return 0;
	}
	
	if(block->type != AOS_TYPE_VERSION) {
		printf("error: Expected VERS (%08x) block, got %c%c%c%c (%08x)\n", AOS_TYPE_VERSION, 
			((uint8_t *)&block->type)[0], ((uint8_t *)&block->type)[1], ((uint8_t *)&block->type)[2],
			((uint8_t *)&block->type)[3], block->type);
		return 0;
	}
	
	version = (void *)block->data;
	
	printf("VERSION: %u.%u.%02u\n", version->major, version->minor, version->build);
	
	return 1;
}

int info_parse_time(struct aos_file *file)
{
	struct aos_block *block;
	
	block = block_get(file, AOS_DURATION_BLOCK_ID);
	if(!block) {
		printf("error: Could not get DURATION block.\n");
		return 0;
	}
	
	if(block->type != AOS_TYPE_DURATION) {
		printf("error: Expected DURATION (%08x) block, got %c%c%c%c (%08x)\n", AOS_TYPE_DURATION, 
			((uint8_t *)&block->type)[0], ((uint8_t *)&block->type)[1], ((uint8_t *)&block->type)[2],
			((uint8_t *)&block->type)[3], block->type);
		return 0;
	}
	
	//printf("TIME: %u\n", time->timestamp);
	
	return 1;
}


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

int main(int argc, char *argv[])
{
	struct aos_file *aos;
	unsigned int length;
	uint8_t *buffer;
	int device;
	
	printf("AOS info utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc != 2) {
		printf("Usage: %s <input>\n\n", argv[0]);
		printf("Display the information about an .aos archive\n");
		return 1;
	}
	
	// Load the file
	buffer = file_load(argv[1], &length);
	if(buffer == NULL)
		return 1;
	
	printf("File %s loaded, %u bytes.\n", argv[1], length);
	
	// Create the aos object
	aos = aos_create(buffer, length);
	if(aos == NULL) {
		printf("error: aos_create failed.\n");
		free(buffer);
		return 1;
	}
	
	// Parse the header informations
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
	
	info_parse_unit(aos);
	info_parse_version(aos);
	info_parse_time(aos);
	
	printf("\n");
	printf("Done.\n");
	
	aos_free(aos);
	free(buffer);
	
	return 0;
}



