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
#include <getopt.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

static const char *program = "aos-fix";

#define ACTION_CLEAR_SIGNATURE		0
#define ACTION_FIX_FILESIZE			1
#define ACTION_ADD_HEADER			2

static int action = -1;

static int overwrite = 0;

static int verbose = 0;
static int help = 0;

static struct option options[] =
{
	/*  What to do? */
	{ "clear-signature", 		no_argument,	&action, ACTION_CLEAR_SIGNATURE },
	{ "fix-filesize", 			no_argument,	&action, ACTION_FIX_FILESIZE },
	{ "add-header",		no_argument,	&action, ACTION_ADD_HEADER },
	
	/* Flags */
	{ "overwrite",			no_argument,	&overwrite, 1 },
	
	/* Generic options */
	{ "verbose",			no_argument,		0, 'v' },
	{ "help",				no_argument,		0, 'h' },
	
	{ 0, 0, 0, 0 }
};

int do_clear_signature(const char *filename, uint8_t *buffer, unsigned int length)
{
	switch(*(uint32_t *)buffer) {
		case AOS_MAGIC: {
			struct aos_file *aos;
			
			// Create the flash object
			aos = aos_create(buffer, length);
			if(aos == NULL) {
				fprintf(stderr, "%s: aos_create failed.\n", program);
				return 0;
			}
			
			if(!aos_is_signed(aos)) {
				printf("%s: File was not signed, file is unchanged\n", filename);
			}
			else {
				aos_clear_signature(aos);
				file_write(filename, buffer, length);
				
				printf("%s: Signature was cleared.\n", filename);
			}
			
			aos_free(aos);
			
			break;
		}
		case AOS_ZMfX_MAGIC:
		case AOS_KERNEL_MAGIC:
		case AOS_CRAMFS_MAGIC: {
			struct flash_file *flash;
			
			// Create the flash object
			flash = flash_create(buffer, length);
			if(flash == NULL) {
				fprintf(stderr, "%s: flash_create failed.\n", program);
				return 0;
			}
			
			if(!flash_is_signed(flash)) {
				printf("%s: File was not signed, file is unchanged\n", filename);
			}
			else {
				flash_clear_signature(flash);
				file_write(filename, buffer, length);
				
				printf("%s: Signature was cleared.\n", filename);
			}
			
			flash_free(flash);
			
			break;
		}
		default: {
			fprintf(stderr, "%s: Unsupported file type.\n", program);
			return 0;
		}
	}
	
	return 1;
}

int do_fix_filesize(const char *filename, uint8_t *buffer, unsigned int length)
{
	struct flash_file *flash;
	
	switch(*(uint32_t *)buffer) {
		case AOS_ZMfX_MAGIC:
		case AOS_KERNEL_MAGIC:
		case AOS_CRAMFS_MAGIC: {
			
			// Create the flash object
			flash = flash_create(buffer, length);
			if(flash == NULL) {
				fprintf(stderr, "%s: flash_create failed.\n", program);
				return 0;
			}
			
			// Fix the header
			if(flash->header->filesize == length) {
				printf("%s: File size was correct, file is unchanged\n", filename);
			}
			else {
				flash->header->filesize = length;
				file_write(filename, buffer, length);
				printf("%s: File was modified, filesize corrected to %u bytes\n", filename, length);
			}
			
			flash_free(flash);
			
			break;
		}
		default: {
			fprintf(stderr, "%s: Unsupported file type.\n", program);
			return 0;
		}
	}
	
	return 1;
}

int do_add_header(const char *filename, uint8_t *buffer, unsigned int length)
{
	unsigned int new_length;
	uint8_t *new_buffer;
	struct flash_header *header;
	
	switch(*(uint32_t *)buffer) {
		case AOS_ZMfX_MAGIC:
		case AOS_KERNEL_MAGIC:
			fprintf(stderr, "%s: Can only add headers on cramfs files.\n", filename);
			break;
		case AOS_CRAMFS_MAGIC:
			if(!overwrite) {
				fprintf(stderr, "%s: The file already has a header! Use --overwrite to replace it.\n", filename);
				return 0;
			}
			break;
		default:
			if(overwrite) {
				fprintf(stderr, "%s: The file does not have a header! Not writing over this data.\n", filename);
				return 0;
			}
			break;
	}
	
	if(overwrite) {
		header = (struct flash_header *)buffer;
		memset(header, 0, sizeof(struct flash_header));
		
		header->magic = AOS_CRAMFS_MAGIC;
		header->bits = 1024;
		header->filesize = length;
		header->entrypoint = 0x100;
		
		file_write(filename, buffer, length);
	}
	else {
		new_length = length + sizeof(struct flash_header);
		new_buffer = malloc(new_length);
		if(!new_buffer) {
			fprintf(stderr, "%s: Memory error.\n", program);
			return 0;
		}
		
		header = (struct flash_header *)new_buffer;
		memset(header, 0, sizeof(struct flash_header));
		memcpy(header->data, buffer, length);
		
		header->magic = AOS_CRAMFS_MAGIC;
		header->bits = 1024;
		header->filesize = new_length;
		header->entrypoint = 0x100;
		
		file_write(filename, new_buffer, new_length);
	
		free(new_buffer);
	}
	
	return 1;
}

int do_file(const char *filename)
{
	unsigned int length;
	uint8_t *buffer;
	
	if(verbose)
		printf("Parsing \"%s\":\n", filename);
	
	// Load the file
	buffer = file_load(filename, &length);
	if(buffer == NULL) {
		fprintf(stderr, "%s: Could not load file.\n", program);
		return 0;
	}
	
	switch(action)
	{
	case ACTION_CLEAR_SIGNATURE:
		do_clear_signature(filename, buffer, length);
		break;
	case ACTION_FIX_FILESIZE:
		do_fix_filesize(filename, buffer, length);
		break;
	case ACTION_ADD_HEADER:
		do_add_header(filename, buffer, length);
		break;
	}
	
	free(buffer);
	
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
		
		c = getopt_long(argc, argv, "vh", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 0: break;
			case '?': break;
			
			case 'v': verbose = 1; break;
			case 'h': help = 1; break;
			
			default:
				printf("Error: unknown option %02x (%c)\n", c, c);
				break;
		}
	}
	
	if(verbose || help) {
		printf("aos-fix utility, part of aos-tools.\n");
		printf("\thttp://code.google.com/p/aos-tools/\n");
		printf("\thttp://archos.g3nius.org/\n\n");
	}
	
	if(help) {
		printf("Fix various aspects of .aos and flash segment headers\n\n");
		printf("Usage:\n");
		printf("  %s [options] [files...]\n\n", program);
		printf("Options:\n");
		printf("  --clear-signature\t\tClear the signature out of a SIGN block, or a flash segment header.\n");
		printf("  --fix-filesize\t\tFix the filesize field of a flash segment to the actual size of the file.\n");
		printf("  --add-header\t\tAdd an archos header to the specified file (for cramfs files only).\n");
		printf("\n");
		printf("  --overwrite\t\tFor --add-header, do not resize the file but rather overwrite the current header.\n");
		printf("\n");
		printf("  --help, -h\t\tDisplay this text\n");
		printf("  --verbose, -v\t\tBe more verbose\n");
		return 1;
	}
	
	if(optind >= argc) {
		printf("Note: Specify at least one file to parse. Use --help for help.\n");
		return 1;
	}
	
	if(action < 0) {
		printf("Note: You must specify an action to perform. Use --help for help.\n");
		return 1;
	}
	
	for(;optind<argc;optind++)
             do_file(argv[optind]);
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}
