/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

static const char *program = "aos-repack";

static int aos = 0;
static int kernel = 0;

static char *digest = NULL;
static char *zimage = NULL;
static char *initramfs = NULL;
static char *output = NULL;
static int encrypt = 0;

static int device = -1;

static int verbose = 0;
static int help = 0;
static int force = 0;

static struct option options[] =
{
	/*  What to do? */
	{ "aos",		no_argument,	0, 'a' },
	{ "kernel",	no_argument,	0, 'k' },
	
	/* For --aos */
	{ "digest",	required_argument,	0, 'd' },
	
	/* For --kernel */
	{ "zimage",	required_argument,	0, 'i' },
	{ "initramfs",	required_argument,	0, 'r' },
	
	/* Other options */
	{ "output",	required_argument,	0, 'o' },
	{ "encrypt",	no_argument,	0, 'e' },
	
	/* Which device keys to use? */
	{ "a5",		no_argument,		&device, MPK_DEVICE_A5 },
	{ "a5it",		no_argument,		&device, MPK_DEVICE_A5IT },
	{ "a3g",		no_argument,		&device, MPK_DEVICE_A3GP },
	
	/* Generic options */
	{ "verbose",	no_argument,		0, 'v' },
	{ "help",		no_argument,		0, 'h' },
	{ "force",		no_argument,		0, 'f' },
	
	{ 0, 0, 0, 0 }
};

#define endswith(a, b) \
	(strlen(a) > strlen(b) && !strcasecmp(&a[strlen(a)-strlen(b)], b))

int parse_cipher(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_cipher *cipher;
	
	/*fprintf(stderr, "%s: Line %u: Requires a %u-char argument.\n", program, linenumber, (AES_BLOCK_SIZE*2));
	
	block = block_get(aos, AOS_CIPHER_BLOCK_ID);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}*/
	
	return 1;
}

int parse_unit(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	struct aos_block_unit *unit;
	char *key;
	
	block = block_get(aos, AOS_UNIT_BLOCK_ID);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	unit = (struct aos_block_unit *)&block->data;
	key = strchr(params, ' ');
	if(key) {
		*key++ = 0;
		strcpy(unit->product_key, key);
	}
	strcpy(unit->product_name, params);
	
	return 1;
}

int parse_version(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	struct aos_block_version *version;
	unsigned int major, minor, build;
	
	if(sscanf(params, "%u.%u.%u", &major, &minor, &build) != 3) {
		fprintf(stderr, "%s: Line %u: Requires a version number like \"x.y.z\".\n", program, linenumber);
		return 0;
	}
	
	block = block_get(aos, AOS_VERSION_BLOCK_ID);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	version = (struct aos_block_version *)&block->data;
	version->major = major;
	version->minor = minor;
	version->build = build;
	
	return 1;
}

int parse_raw(struct aos_file *aos, char *params, int linenumber, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	uint8_t type[4];
	char filename[1024];
	
	if(sscanf(params, "%c%c%c%c %s", &type[0], &type[1], &type[2], &type[3], (char *)&filename) != 5) {
		fprintf(stderr, "%s: Line %u: Requires 1 4-char string, 1 string argument.\n", program, linenumber);
		return 0;
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) {
		fprintf(stderr, "%s: Line %u: Could not load file %s.\n", program, linenumber, filename);
		return 0;
	}
	
	block = aos_append_block(aos, *(uint32_t *)type, length);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		free(buffer);
		return 0;
	}
	
	memcpy(&block->data, buffer, length);
	free(buffer);
	
	return 1;
}

int validate_flash(const char *filename, uint8_t *buffer, unsigned int length, uint32_t expected_magic)
{
	struct flash_file *flash;
	
	flash = flash_create(buffer, length);
	if(!flash) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	if(flash->header->magic != expected_magic) {
		fprintf(stderr, "%s: %s: The header is not valid (wrong magic).\n", program, filename);
		flash_free(flash);
		return 0;
	}
	
	if(!flash_is_signed(flash)) {
		// if the header is specifically not signed, we assume this is on purpose and
		// it does not constitute an error.
		if(verbose)
			printf("%s: The file is specifically not signed.\n", filename);
		flash_free(flash);
		return 1;
	}
	
	if(device == -1) {
		if(force) {
			printf("WARNING: %s: This file is signed and the signature could not be validated.\n"
				 "\tThis sanity check will not fail because --force is specified, but you should rather fix this by specifying --a5, --a5it or --a3g.\n", filename);
			return 1;
		}
		
		fprintf(stderr, "%s: This cramfs file has a signature, but no device type specified to valitate it.\n"
					"\tUse --a5, --a5it or --a3g.\n", program);
		flash_free(flash);
		return 0;
	}
	
	if(!flash_verify_signature(flash, Bootloader_Keys[device])) {
		if(force) {
			printf("WARNING: %s: This file is signed and the signature is invalid.\n"
				 "\tThis sanity check will not fail because --force is specified, but this may cause problems.\n", filename);
			return 1;
		}
		fprintf(stderr, "%s: %s: The signature on the file is invalid.\n", program, filename);
		flash_free(flash);
		return 0;
	}
	
	return 1;
}

int parse_flash(struct aos_file *aos, char *params, int linenumber, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	uint32_t offset;
	char filename[1024];
	struct aos_block_flash *flash;
	uint32_t expected_magic = 0;
	unsigned int max_filesize = 0;
	
	if(sscanf(params, "0x%08x %s", &offset, (char *)&filename) != 2) {
		fprintf(stderr, "%s: Line %u: Requires 1 doubleword hex number starting with 0x, 1 string argument.\n", program, linenumber);
		return 0;
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) {
		fprintf(stderr, "%s: Line %u: Could not load file %s.\n", program, linenumber, filename);
		return 0;
	}
	
	switch(offset) {
		case 0x00000000:
			max_filesize = 65536;
			break;
		case 0x00010000:
			max_filesize = 131072;
			break;
		case 0x00030000:
			expected_magic = AOS_ZMfX_MAGIC;
			max_filesize = 196608;
			break;
		case 0x00060000:
			max_filesize = 131072;
			break;
		case 0x00080000:
			expected_magic = AOS_KERNEL_MAGIC;
			max_filesize = 1638400;
			break;
		case 0x00210000:
			expected_magic = AOS_KERNEL_MAGIC;
			max_filesize = 1998848;
			break;
		case 0x003f8000:
			max_filesize = 32768;
			break;
		default:
			if(!force) {
				fprintf(stderr, "%s: Line %u: This offset (0x%08x) is unknown, are you sure this is right?\n", program, linenumber, offset);
				free(buffer);
				return 0;
			}
			else {
				printf("WARNING: Line %u: This offset (0x%08x) is unknown, but this sanity check is turned off because of --force.\n", linenumber, offset);
				max_filesize = 0;
			}
			break;
	}
	
	if(max_filesize != 0 && length > max_filesize) {
		if(!force) {
			fprintf(stderr, "%s: Line %u: This file is too big to be flashed at this offset (%08x).\n", program, linenumber, offset);
			free(buffer);
			return 0;
		}
		else {
			printf("WARNING: Line %u: This file is too big (%u bytes, maximum is %u) to be flashed at this offset (0x%08x), but this sanity check is turned off because of --force.\n", linenumber, length, max_filesize, offset);
		}
	}
	
	if(expected_magic != 0) {
		if(!validate_flash(filename, buffer, length, expected_magic)) {
			fprintf(stderr, "%s: Line %u: The file %s could not be validated.\n", program, linenumber, filename);
			free(buffer);
			return 0;
		}
		else {
			if(verbose)
				printf("%s: The file was validated successfuly.\n", filename);
		}
	}
	
	block = aos_append_block(aos, AOS_TYPE_FLASH, sizeof(struct aos_block_flash)+length);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
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

int parse_mtd(struct aos_file *aos, char *params, int linenumber, char *basepath)
{
	struct aos_block *block;
	char *filepath;
	unsigned int length;
	char *buffer;
	char partition[256];
	uint32_t offset;
	char filename[1024];
	struct aos_block_mtd *mtd;
	
	if(sscanf(params, "%s 0x%08x %s", (char *)&partition, &offset, (char *)&filename) != 3) {
		fprintf(stderr, "%s: Line %u: Requires 1 string, 1 doubleword hex number starting with 0x, 1 string argument.\n", program, linenumber);
		return 0;
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) {
		fprintf(stderr, "%s: Line %u: Could not load file %s.\n", program, linenumber, filename);
		return 0;
	}
	
	block = aos_append_block(aos, AOS_TYPE_MTD, sizeof(struct aos_block_mtd)+length);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
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

int parse_copy(struct aos_file *aos, char *params, int linenumber, char *basepath)
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
		if(sscanf(params, "%u %s %s", &partition, (char *)&target, (char *)&filename) != 3) {
			fprintf(stderr, "%s: Line %u: Requires 1 integer, 1 string, 1 string argument.\n", program, linenumber);
			return 0;
		}
	}
	else {
		if(sscanf(params, "%u \"%[^\"]\" \"%[^\"]\"", &partition, (char *)&target, (char *)&filename) != 3) {
			fprintf(stderr, "%s: Line %u: Requires 1 integer, 1 string, 1 string argument.\n", program, linenumber);
			return 0;
		}
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) {
		fprintf(stderr, "%s: Line %u: Could not load file %s.\n", program, linenumber, filename);
		return 0;
	}
	
	if(endswith(target, ".cramfs.secure")) {
		if(!validate_flash(filename, buffer, length, AOS_CRAMFS_MAGIC)) {
			fprintf(stderr, "%s: Line %u: The file %s could not be validated.\n", program, linenumber, filename);
			free(buffer);
			return 0;
		}
		else {
			if(verbose)
				printf("%s: The file was validated successfuly.\n", filename);
		}
	}
	
	block = aos_append_block(aos, AOS_TYPE_COPY, sizeof(struct aos_block_copy)+length);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
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

int parse_delete(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	uint32_t partition;
	char target[256];
	struct aos_block_delete *delete;
	
	if(strchr(params, '"') == NULL) { // Quirks to keep digest clean
		if(sscanf(params, "%u %s", &partition, (char *)&target) != 2) {
			fprintf(stderr, "%s: Line %u: Requires 1 integer, 1 string argument.\n", program, linenumber);
			return 0;
		}
	} else {
		if(sscanf(params, "%u \"%[^\"]\"", &partition, (char *)&target) != 2) {
			fprintf(stderr, "%s: Line %u: Requires 1 integer, 1 string argument.\n", program, linenumber);
			return 0;
		}
	}
	
	block = aos_append_block(aos, AOS_TYPE_DELETE, sizeof(struct aos_block_delete));
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	delete = (struct aos_block_delete *)&block->data;
	delete->partition = partition;
	strcpy(delete->name, target);
	
	return 1;
}

int parse_boot(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_boot *boot;
	
	if(sscanf(params, "%u", &value) != 1) {
		fprintf(stderr, "%s: Line %u: Requires 1 integer argument.\n", program, linenumber);
		return 0;
	}
	
	block = aos_append_block(aos, AOS_TYPE_BOOT, sizeof(struct aos_block_boot));
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	boot = (struct aos_block_boot *)&block->data;
	boot->value = value;
	
	return 1;
}

int parse_adel(struct aos_file *aos, char *params, int linenumber)
{
	struct aos_block *block;
	uint32_t value;
	struct aos_block_adel *adel;
	
	if(sscanf(params, "%u", &value) != 1) {
		fprintf(stderr, "%s: Line %u: Requires 1 integer argument.\n", program, linenumber);
		return 0;
	}
	
	block = aos_append_block(aos, AOS_TYPE_ADEL, sizeof(struct aos_block_adel));
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	adel = (struct aos_block_adel *)&block->data;
	adel->value = value;
	
	return 1;
}

int parse_shell(struct aos_file *aos, char *params, int linenumber, char *basepath)
{
	struct aos_block *block;
	char filename[1024];
	struct aos_block_shell *shell;
	char *filepath;
	unsigned int length;
	char *buffer;
	
	if(sscanf(params, "%s", (char *)&filename) != 1) {
		fprintf(stderr, "%s: Line %u: Requires 1 string argument.\n", program, linenumber);
		return 0;
	}
	
	filepath = bprintf("%s%s", basepath, filename);
	if(!filepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return 0;
	}
	
	buffer = file_load(filepath, &length);
	free(filepath);
	if(!buffer) {
		fprintf(stderr, "%s: Line %u: Could not load file %s.\n", program, linenumber, filename);
		return 0;
	}
	
	block = aos_append_block(aos, AOS_TYPE_SHELL, sizeof(struct aos_block_shell)+length);
	if(!block) {
		fprintf(stderr, "%s: Memory error.\n", program);
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
	if(!block) {
		fprintf(stderr, "%s: No duration block?.\n", program);
		return 0;
	}
	
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
	int linenumber;
	
	basepath = strdup(filename);
	if(!basepath) {
		fprintf(stderr, "%s: Memory error.\n", program);
		return NULL;
	}
	
	// remove the filename
	while(strlen(basepath) > 0 && basepath[strlen(basepath)-1] != '/')
		basepath[strlen(basepath)-1] = 0;
	
	buffer = malloc(sizeof(uint32_t));
	if(!buffer) {
		fprintf(stderr, "%s: Memory error.\n", program);
		free(basepath);
		return NULL;
	}
	
	// Initialize the file
	*(uint32_t *)buffer = AOS_MAGIC;
	
	// Create the aos file
	aos = aos_create(buffer, sizeof(uint32_t));
	if(!aos) {
		fprintf(stderr, "%s: Memory error.\n", program);
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
	
	linenumber = 1;
	line = digest;
	while(line) {
		char *params;
		int ret = 0;
		
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
			ret = parse_cipher(aos, params, linenumber);
		}
		else if(!strcasecmp(line, "unit")) {
			ret = parse_unit(aos, params, linenumber);
		}
		else if(!strcasecmp(line, "version")) {
			ret = parse_version(aos, params, linenumber);
		}
		else {
			block_count++;
			
			if(!strcasecmp(line, "raw")) {
				ret = parse_raw(aos, params, linenumber, basepath);
			}
			else if(!strcasecmp(line, "flash")) {
				ret = parse_flash(aos, params, linenumber, basepath);
			}
			else if(!strcasecmp(line, "mtd")) {
				ret = parse_mtd(aos, params, linenumber, basepath);
			}
			else if(!strcasecmp(line, "copy")) {
				ret = parse_copy(aos, params, linenumber, basepath);
			}
			else if(!strcasecmp(line, "delete")) {
				ret = parse_delete(aos, params, linenumber);
			}
			else if(!strcasecmp(line, "boot")) {
				ret = parse_boot(aos, params, linenumber);
			}
			else if(!strcasecmp(line, "adel")) {
				ret = parse_adel(aos, params, linenumber);
			}
			else if(!strcasecmp(line, "shell")) {
				ret = parse_shell(aos, params, linenumber, basepath);
			}
			else {
				fprintf(stderr, "%s: Line %u: Unknown block type \"%s\".\n", program, linenumber, line);
				free(basepath);
				free(aos->data);
				aos_free(aos);
				return NULL;
			}
		}
		
		if(!ret) {
			fprintf(stderr, "%s: Line %u: Block creation failed. Correct your digest file and try again.\n", program, linenumber, line);
			free(basepath);
			free(aos->data);
			aos_free(aos);
			return NULL;
		}
		
		line = nextline;
		linenumber++;
	}
	
	set_duration(aos, block_count);
	
	free(basepath);
	
	return aos;
}

int do_aos()
{
	struct aos_file *aos;
	unsigned int length;
	char *buffer;
	
	// Load the file
	buffer = file_load(digest, &length);
	if(buffer == NULL)
		return 0;
	
	aos = parse_digest(buffer, digest);
	
	if(encrypt) {
		if(!aos_encrypt_file(aos, AES_Keys[device])) {
			fprintf(stderr, "%s: Encryption of the output .aos file failed!\n", program);
			return 0;
		}
		else {
			if(verbose)
				printf("The output file was successfuly encrypted.\n");
		}
	}
	else {
		if(verbose)
			printf("Not encrypting the output file.\n");
	}
	
	file_write(output, aos->data, aos->length);
	
	free(aos->data);
	aos_free(aos);
	free(buffer);
	
	return 1;
}

int do_kernel()
{
	unsigned int zimage_length, initramfs_length;
	uint8_t *zimage_buffer, *initramfs_buffer;
	char *zimage_path, *initramfs_path;
	uint8_t *buffer;
	unsigned int length;
	char cwd[2048];
	struct flash_header *header;
	
	getcwd(cwd, sizeof(cwd));
	
	if(zimage[0] == '/')
		zimage_path = strdup(zimage);
	else
		zimage_path = bprintf("%s/%s", cwd, zimage);
	
	if(initramfs[0] == '/')
		initramfs_path = strdup(initramfs);
	else
		initramfs_path = bprintf("%s/%s", cwd, initramfs);
	
	zimage_buffer = file_load(zimage_path, &zimage_length);
	if(zimage_buffer == NULL) {
		fprintf(stderr, "%s: %s: Could not load file.\n", program, zimage);
		free(zimage_path);
		free(initramfs_path);
		return 0;
	}
	
	initramfs_buffer = file_load(initramfs_path, &initramfs_length);
	if(initramfs == NULL) {
		fprintf(stderr, "%s: %s: Could not load file.\n", program, initramfs);
		free(zimage_path);
		free(initramfs_path);
		free(zimage_buffer);
		return 0;
	}
	
	length = sizeof(struct flash_header) + zimage_length + initramfs_length;
	buffer = malloc(length);
	if(!buffer) {
		fprintf(stderr, "%s: Memory error.\n", program);
		free(zimage_path);
		free(initramfs_path);
		free(zimage_buffer);
		free(initramfs_buffer);
		return 0;
	}
	
	header = (struct flash_header *)buffer;
	
	memset(header, 0, length);
	memcpy(&header->data[0], zimage_buffer, zimage_length);
	memcpy(&header->data[zimage_length], initramfs_buffer, initramfs_length);
	
	header->magic = AOS_KERNEL_MAGIC;
	header->bits = 1024;
	header->filesize = length;
	header->cpio = sizeof(struct flash_header) + zimage_length;
	header->cpio_size = initramfs_length;
	
	if(!file_write(output, buffer, length)) {
		fprintf(stderr, "%s: %s: Could not write output file.\n", program, output);
		free(zimage_path);
		free(initramfs_path);
		free(zimage_buffer);
		free(initramfs_buffer);
		return 0;
	}
	
	printf("Wrote %u bytes to %s\n", length, output);
	
	free(zimage_path);
	free(initramfs_path);
	free(zimage_buffer);
	free(initramfs_buffer);
	
	return 1;
}

int main(int argc, char *argv[])
{
	if(argc > 0)
		program = argv[0];
	
	while(1)
	{
		int index = 0;
		int c;
		
		c = getopt_long(argc, argv, "vhfakd:i:r:o:e", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 0: break;
			case '?': break;
			
			case 'v': verbose = 1; break;
			case 'h': help = 1; break;
			case 'f': force = 1; break;
			
			case 'a': aos = 1; break;
			case 'k': kernel = 1; break;
			
			case 'd': digest = optarg; break;
			case 'i': zimage = optarg; break;
			case 'r': initramfs = optarg; break;
			case 'o': output = optarg; break;
			case 'e': encrypt = 1; break;
			
			default:
				printf("Error: unknown option %02x (%c)\n", c, c);
				break;
		}
	}
	
	if(verbose || help) {
		printf("aos-repack utility, part of aos-tools\n");
		printf("\thttp://code.google.com/p/aos-tools/\n");
		printf("\thttp://archos.g3nius.org/\n\n");
	}
	
	if(help) {
		printf("This utility will create\n");
		printf(" - an AOS2 archive from a digest file, or\n");
		printf(" - a kernel flash segment from a zImage and an initramfs.\n\n");
		printf("Usage:\n");
		printf("  %s [action] [options...]\n\n", program);
		printf("Actions:\n");
		printf("  --aos, -a\t\tCreate and AOS2 archive from a digest file.\n");
		printf("  --kernel, k\t\tCreate a kernel flash segment from a zImage and an initramfs.\n");
		printf("\n");
		printf("Options:\n");
		printf("  --digest, -d [file]\tRequired for --aos; specify the digest file to use.\n");
		printf("  --encrypt, -e\tFor --aos, optionally encrypt the output file. This is turned off by default.\n");
		printf("  --zimage, -i [file]\tRequired for --kernel, specify the zImage to use.\n");
		printf("  --initramfs, -r [file] Required for --kernel, specify the initramfs to use.\n");
		printf("  --output, -o [file]\tSpecify the name of the output file.\n");
		printf("  --force, -f\t\tForce the process past some sanity checks. Use with care.\n");
		printf("\n");
		printf("  --a5\t\t\tAssume the target .aos is for the Archos 5/7 devices\n");
		printf("  --a5it\t\tAssume the target .aos is for the Archos 5 Internet Tablet with Android\n");
		printf("  --a3g\t\t\tAssume the target .aos is for the Archos 3G+ from SFR\n");
		printf("    This may be needed for some sanity checks.\n");
		printf("\n");
		printf("  --help, -h\t\tDisplay this text\n");
		printf("  --verbose, -v\t\tBe more verbose\n");
		return 1;
	}
	
	if(!aos && !kernel) {
		printf("Note: Specify an action to perform. Use --help for help.\n");
		return 1;
	}
	
	if(argc-optind > 0) {
		printf("Note: Unrecognized option \"%s\". Use --help for help.\n", argv[optind]);
		return 1;
	}
	
	if(aos && !digest) {
		printf("Note: --digest is required with --aos. Use --help for help.\n");
		return 1;
	}
	
	if(kernel && !zimage) {
		printf("Note: --zimage is required with --kernel. Use --help for help.\n");
		return 1;
	}
	
	if(kernel && !initramfs) {
		printf("Note: --initramfs is required with --kernel. Use --help for help.\n");
		return 1;
	}
	
	if((aos || kernel) && !output) {
		printf("Note: --output is required with --aos or --kernel. Use --help for help.\n");
		return 1;
	}
	
	if(encrypt && device == -1) {
		printf("Note: You must specify a device type with --encrypt. Use --a5, --a5it or --a3g. Use --help for help.\n");
		return 1;
	}
	
	if(aos) do_aos();
	if(kernel) do_kernel();
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}



