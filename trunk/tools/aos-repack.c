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

/*
int parse_cipher(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_cipher *cipher;
	
	if(sscanf(params, "%u", &value) != 1)
		return 0;
	
	block = block_get(aos, AOS_CIPHER_BLOCK_ID);
	if(!block)
		return 0;
	
	
	
	return 1;
}*/

int parse_unit(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	struct aos_block_unit *unit;
	char *key;
	
	block = block_get(aos, AOS_UNIT_BLOCK_ID);
	if(!block)
		return 0;
	
	unit = (struct aos_block_unit *)&block->data;
	key = strchr(params, ' ');
	if(key) {
		*key++ = 0;
		strcpy(unit->product_key, key);
	}
	strcpy(unit->product_name, params);
	
	return 1;
}

int parse_version(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	struct aos_block_version *version;
	unsigned int major, minor, build;
	
	block = block_get(aos, AOS_VERSION_BLOCK_ID);
	if(!block)
		return 0;
	
	version = (struct aos_block_version *)&block->data;
	if(sscanf(params, "%u.%u.%u", &major, &minor, &build) != 3)
		return 0;
	
	version->major = major;
	version->minor = minor;
	version->build = build;
	
	return 1;
}

int parse_raw(struct aos_file *aos, char *params, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	uint8_t type[4];
	char filename[1024];
	
	if(sscanf(params, "%c%c%c%c %s", &type[0], &type[1], &type[2], &type[3], (char *)&filename) != 5)
		return 0;
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) return 0;
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) return 0;
	
	block = aos_append_block(aos, *(uint32_t *)type, length);
	if(!block) {
		free(buffer);
		return 0;
	}
	
	memcpy(&block->data, buffer, length);
	free(buffer);
	
	return 1;
}

int parse_flash(struct aos_file *aos, char *params, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	uint32_t offset;
	char filename[1024];
	struct aos_block_flash *flash;
	
	if(sscanf(params, "0x%08x %s", &offset, (char *)&filename) != 2)
		return 0;
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) return 0;
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) return 0;
	
	block = aos_append_block(aos, AOS_TYPE_FLASH, sizeof(struct aos_block_flash)+length);
	if(!block) {
		free(buffer);
		return 0;
	}
	
	flash = (struct aos_block_flash *)&block->data;
	flash->offset = offset;
	flash->size = length;
	memcpy(flash->data, buffer, length);
	
	free(buffer);
	
	return 1;
}

int parse_mtd(struct aos_file *aos, char *params, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	char partition[256];
	uint32_t offset;
	char filename[1024];
	struct aos_block_mtd *mtd;
	
	if(sscanf(params, "%s 0x%08x %s", (char *)&partition, &offset, (char *)&filename) != 3)
		return 0;
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) return 0;
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) return 0;
	
	block = aos_append_block(aos, AOS_TYPE_MTD, sizeof(struct aos_block_mtd)+length);
	if(!block) {
		free(buffer);
		return 0;
	}
	
	mtd = (struct aos_block_mtd *)&block->data;
	strcpy(mtd->name, partition);
	mtd->offset = offset;
	mtd->size = length;
	memcpy(mtd->data, buffer, length);
	
	free(buffer);
	
	return 1;
}

int parse_copy(struct aos_file *aos, char *params, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	uint32_t partition;
	char target[256];
	char filename[1024];
	struct aos_block_copy *copy;
	
	if(strchr(params, '"') == NULL) { // Quirks to keep digest clean
		if(sscanf(params, "%u %s %s", &partition, (char *)&target, (char *)&filename) != 3)
			return 0;
	}
	else {
		if(sscanf(params, "%u \"%[^\"]\" \"%[^\"]\"", &partition, (char *)&target, (char *)&filename) != 3)
			return 0;
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) return 0;
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) return 0;
	
	block = aos_append_block(aos, AOS_TYPE_COPY, sizeof(struct aos_block_copy)+length);
	if(!block) {
		free(buffer);
		return 0;
	}
	
	copy = (struct aos_block_copy *)&block->data;
	copy->partition = partition;
	strcpy(copy->name, target);
	copy->size = length;
	memcpy(copy->data, buffer, length);
	
	free(buffer);
	
	return 1;
}

int parse_delete(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	uint32_t partition;
	char target[256];
	struct aos_block_delete *delete;
	
	if(strchr(params, '"') == NULL) { // Quirks to keep digest clean
		if(sscanf(params, "%u %s", &partition, (char *)&target) != 2)
			return 0;
	} else {
		if(sscanf(params, "%u \"%[^\"]\"", &partition, (char *)&target) != 2)
			return 0;
	}
	
	block = aos_append_block(aos, AOS_TYPE_DELETE, sizeof(struct aos_block_delete));
	if(!block) return 0;
	
	delete = (struct aos_block_delete *)&block->data;
	delete->partition = partition;
	strcpy(delete->name, target);
	
	return 1;
}

int parse_boot(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_boot *boot;
	
	if(sscanf(params, "%u", &value) != 1)
		return 0;
	
	block = aos_append_block(aos, AOS_TYPE_BOOT, sizeof(struct aos_block_boot));
	if(!block) return 0;
	
	boot = (struct aos_block_boot *)&block->data;
	boot->value = value;
	
	return 1;
}

int parse_adel(struct aos_file *aos, char *params)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_adel *adel;
	
	if(sscanf(params, "%u", &value) != 1)
		return 0;
	
	block = aos_append_block(aos, AOS_TYPE_ADEL, sizeof(struct aos_block_adel));
	if(!block) return 0;
	
	adel = (struct aos_block_adel *)&block->data;
	adel->value = value;
	
	return 1;
}

int parse_shell(struct aos_file *aos, char *params, char *basepath)
{
	struct aos_block *block;
	char filename[1024];
	struct aos_block_shell *shell;
	char *filepath;
	unsigned int length;
	char *buffer;
	
	if(sscanf(params, "%s", (char *)&filename) != 1)
		return 0;
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) return 0;
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) return 0;
	
	block = aos_append_block(aos, AOS_TYPE_SHELL, sizeof(struct aos_block_shell)+length);
	if(!block) {
		free(buffer);
		return 0;
	}
	
	shell = (struct aos_block_shell *)&block->data;
	shell->length = length;
	memcpy(shell->data, buffer, length);
	
	free(buffer);
	
	return 1;
}

// right now, we set all durations to 24×60×60 (1 day)
int set_duration(struct aos_file *aos, unsigned int block_count)
{
	struct aos_block *block;
	struct aos_block_duration *duration;
	unsigned int i;
	
	block = block_get(aos, AOS_DURATION_BLOCK_ID);
	if(!block)
		return 0;
	
	duration = (struct aos_block_duration *)&block->data;
	duration->count = block_count;
	for(i=0;i<block_count;i++)
		duration->time[i] = 24*60*60;
	
	return 1;
}

// The digest is parsed in-place, don't use it after this call.
struct aos_file *parse_digest(char *digest, char *filename)
{
	char *basepath;
	struct aos_file *aos;
	unsigned int current_length;
	char *buffer;
	char *line;
	unsigned int block_count = 0;
	
	basepath = strdup(filename);
	if(!basepath) {
		printf("error: Memory\n");
		return NULL;
	}
	
	// remove the filename
	while(strlen(basepath) > 0 && basepath[strlen(basepath)-1] != '/')
		basepath[strlen(basepath)-1] = 0;
	
	buffer = malloc(sizeof(uint32_t));
	if(!buffer) {
		printf("error: Memory\n");
		free(basepath);
		return NULL;
	}
	
	// Initialize the file
	*(uint32_t *)buffer = AOS_MAGIC;
	
	// Create the aos file
	aos = aos_create(buffer, sizeof(uint32_t));
	if(!aos) {
		printf("error: Memory\n");
		free(basepath);
		free(buffer);
		return NULL;
	}
	
	// Add the header blocks, those need to be there no matter what.
	aos_append_block_nopadding(aos, AOS_TYPE_SIG0, sizeof(struct aos_block_signature));
	aos_append_block_nopadding(aos, AOS_TYPE_CIPHER, sizeof(struct aos_block_cipher));
	aos_append_block(aos, AOS_TYPE_UNIT, sizeof(struct aos_block_unit));
	aos_append_block(aos, AOS_TYPE_VERSION, sizeof(struct aos_block_version));
	aos_append_block(aos, AOS_TYPE_DURATION, sizeof(struct aos_block_duration));
	
	line = digest;
	while(line) {
		char *params;
		
		char *nextline = strchr(line, '\n');
		if(nextline) *nextline++ = 0;
		
		// Sanitize a bit
		while(*line == '\r' || *line == '\n' || *line == ' ') line++;
		while(line[strlen(line)-1] == '\r') line[strlen(line)-1] = 0;
		
		params = strchr(line, ' ');
		if(!params) {
			line = nextline;
			continue;
		}
		*params++ = 0;
		
		if(!strcasecmp(line, "cipher")) {
			//parse_cipher(aos, params);
		}
		else if(!strcasecmp(line, "unit")) {
			parse_unit(aos, params);
		}
		else if(!strcasecmp(line, "version")) {
			parse_version(aos, params);
		}
		else {
			block_count++;
			
			if(!strcasecmp(line, "raw")) {
				parse_raw(aos, params, basepath);
			}
			else if(!strcasecmp(line, "flash")) {
				parse_flash(aos, params, basepath);
			}
			else if(!strcasecmp(line, "mtd")) {
				parse_mtd(aos, params, basepath);
			}
			else if(!strcasecmp(line, "copy")) {
				parse_copy(aos, params, basepath);
			}
			else if(!strcasecmp(line, "delete")) {
				parse_delete(aos, params);
			}
			else if(!strcasecmp(line, "boot")) {
				parse_boot(aos, params);
			}
			else if(!strcasecmp(line, "adel")) {
				parse_adel(aos, params);
			}
			else if(!strcasecmp(line, "shell")) {
				parse_shell(aos, params, basepath);
			}
			else {
				printf("Unknown line: \"%s\" = \"%s\"\n", line, params);
			}
		}
		
		line = nextline;
	}
	
	set_duration(aos, block_count);
	
	free(basepath);
	
	return aos;
}

int main(int argc, char *argv[])
{
	struct aos_file *aos;
	unsigned int length;
	char *buffer;
	
	printf("AOS repack utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc < 2) {
		printf("Usage: %s <digest>\n\n", argv[0]);
		printf("This utility uses the information from the digest file\n");
		printf("created by aos-unpack to recreate a valid .aos file\n");
		return 1;
	}
	
	// Load the file
	buffer = file_load(argv[1], &length);
	if(buffer == NULL)
		return 1;
	
	printf("File %s loaded, %u bytes.\n", argv[1], length);
	
	aos = parse_digest(buffer, argv[1]);
	//aos_encrypt_file(aos, A5_AES);
	
	printf("file is %u bytes\n", aos->length);
	
	file_write("output.aos", aos->data, aos->length);
	
	printf("\n");
	printf("Done.\n");
	
	free(aos->data);
	aos_free(aos);
	free(buffer);
	
	return 0;
}



