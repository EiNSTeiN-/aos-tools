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

static const char *program = "aos-flash";

#define ACTION_CPIO_EXTRACT	0
#define ACTION_CPIO_INSERT		1

static int action = -1;

static int device = -1;

static int verbose = 0;
static int help = 0;

static const char *output = NULL;
static const char *input = NULL;

static struct option options[] =
{
	/*  What to do? */
	{ "extract", 	optional_argument,	0, 'x' },
	{ "insert", 	required_argument,	0, 'i'},
	
	/* Which device keys to use? */
	{ "a5",		no_argument,		&device, MPK_DEVICE_A5 },
	{ "a5it",		no_argument,		&device, MPK_DEVICE_A5IT },
	
	/* Generic options */
	{ "verbose",	no_argument,		0, 'v' },
	{ "help",		no_argument,		0, 'h' },
	
	{ 0, 0, 0, 0 }
};

int flash_extract_cpio(struct flash_file *file, const char *output)
{
	unsigned int length;
	uint8_t *data;
	
	if(!file->header->cpio) {
		fprintf(stderr, "%s: Offset error.\n", program);
		return 0;
	}
	
	if(file->header->cpio >= file->length) {
		fprintf(stderr, "%s: Offset error.\n", program);
		return 0;
	}
	
	data = (uint8_t *)file->header+file->header->cpio;
	length = (file->length - file->header->cpio);
	
	if(*(uint32_t *)data != AOS_GZIP_MAGIC) {
		fprintf(stderr, "%s: The data to be extracted is not a gzip archive.\n", program);
		return 0;
	}
	
	mkdir_recursive(output);
	file_write(output, data, length);
	
	printf("File was extracted successfuly.\n");
	
	return 1;
}

int flash_insert_cpio(struct flash_file *file, const char *filename, const char *input)
{
	unsigned int length, newlength;
	uint8_t *data, *newdata;
	
	if(!file->header->cpio) {
		fprintf(stderr, "%s: Offset error.\n", program);
		return 0;
	}
	
	if(file->header->cpio >= file->length) {
		fprintf(stderr, "%s: Offset error.\n", program);
		return 0;
	}
	
	data = file_load(input, &length);
	if(!data) {
		fprintf(stderr, "%s: Could not open file.\n", program);
		return 0;
	}
	
	if(*(uint32_t *)data != AOS_GZIP_MAGIC) {
		fprintf(stderr, "%s: The file to insert does not have a gzip magic.\n", program);
		return 0;
	}
	
	newlength = file->header->cpio + length;
	newdata = malloc(newlength);
	if(!newdata) {
		fprintf(stderr, "%s: Memory error.\n", program);
		free(data);
		return 0;
	}
	
	// copy the old file up to the cpio
	memcpy(newdata, &file->header->magic, file->header->cpio);
	
	// copy the new cpio after the old file
	memcpy(&newdata[file->header->cpio], data, length);
	
	// Set the new file size
	((struct flash_header *)newdata)->filesize = newlength;
	((struct flash_header *)newdata)->cpio_size = length;
	
	mkdir_recursive(filename);
	file_write(filename, newdata, newlength);
	
	printf("File was modified successfuly.\n");
	
	free(data);
	free(newdata);
	
	return 1;
}

int do_file(const char *filename)
{
	struct flash_file *flash;
	unsigned int length;
	uint8_t *buffer;
	
	if(verbose)
		printf("Parsing \"%s\":\n", filename);
	
	// Load the file
	buffer = file_load(filename, &length);
	if(buffer == NULL) {
		fprintf(stderr, "%s: Could not load file.\n", program);
		return 1;
	}
	
	// Create the flash object
	flash = flash_create(buffer, length);
	if(flash == NULL) {
		fprintf(stderr, "%s: flash_create failed.\n", program);
		free(buffer);
		return 0;
	}
	
	if(device != -1) {
		if(verbose)
			printf("Device type: %s (forced by user)\n", mpk_device_type(device));
		
		/* The device type is specified by the user, 
			we verify the signature to make sure. */
		if(flash_is_signed(flash) && !flash_verify_signature(flash, Bootloader_Keys[device])) {
			fprintf(stderr, "%s: File is signed, but signature could not be verified.\n", program);
			flash_free(flash);
			free(buffer);
			return 0;
		}
	}
	else {
		if(!flash_is_signed(flash)) {
			/* The device type is not specified by the user, but the file is not signed.
				We can't detect the device type in this case. */
			fprintf(stderr, "%s: Could not detect device type because the file is not signed.\n"
							"\tSpecify --a5 or --a5it.\n", program);
			flash_free(flash);
			free(buffer);
			return 0;
		}
		
		// Parse & verify the header
		if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &device)) {
			fprintf(stderr, "%s: Could not detect device type from signature data.\n"
							"Specify --a5 or --a5it.", program);
			flash_free(flash);
			free(buffer);
			return 0;
		}
		else {
			if(verbose)
				printf("Device type: %s (detected from signature data)\n", mpk_device_type(device));
		}
	}
	
	// Detect file type
	if(flash->header->magic != AOS_CPIO_MAGIC) {
		fprintf(stderr, "%s: The file is not an \"init\" or \"recovery\" flash segment.", program);
		flash_free(flash);
		free(buffer);
		return 0;
	}
	
	switch(action) {
		case ACTION_CPIO_EXTRACT: {
			char *tmp_output;
			
			if(output)
				tmp_output = strdup(output);
			else
				tmp_output = bprintf("%s-initramfs.cpio.gz", filename);
			
			if(verbose)
				printf("Extracting to \"%s\"\n", tmp_output);
			
			if(!tmp_output) {
				fprintf(stderr, "%s: Memory error.\n", program);
				break;
			}
			
			flash_extract_cpio(flash, tmp_output);
			free(tmp_output);
			break;
		}
		case ACTION_CPIO_INSERT:
			flash_insert_cpio(flash, filename, input);
			break;
	}
	
	flash_free(flash);
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
		
		c = getopt_long(argc, argv, "vhx::i:", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 'v':
				verbose = 1;
				break;
			
			case 'h':
				help = 1;
				break;
			
			case 'x':
				action = ACTION_CPIO_EXTRACT;
				output = optarg;
				break;
			
			case 'i':
				action = ACTION_CPIO_INSERT;
				input = optarg;
				break;
			
			case '?':
			       /* getopt_long already printed an error message. */
			       break;
			
			default:
				printf("Error: unknown option %02x (%c)\n", c, c);
				break;
		}
	}
	
	if(verbose || help) {
		printf("AOS info utility, written by EiNSTeiN_\n");
		printf("\thttp://archos.g3nius.org/\n\n");
	}
	
	if(action < 0) {
		printf("Note: You must specify either --extract or --insert\n\n");
		help = 1;
	}
	
	if(argc-optind > 1 || argc-optind <= 0) {
		printf("Note: Please specify only one target file.\n\n");
		help = 1;
	}
	
	if(help) {
		printf("Extract the .cpio.gz filesystem (initramfs) stored in the \"init\" and \"kernel\" flash segments\n\n");
		printf("Usage:\n");
		printf("  %s [options...] [TARGET]\n\n", program);
		printf("Options:\n");
		printf("  --extract, -x[=FILE]\tExtract the initramfs from TARGET, optionally specifying an output FILE.\n");
		printf("  --insert, -i=FILE\tWrite a new initramfs image FILE into TARGET.\n");
		printf("\n");
		printf("  --a5\t\t\tAssume the target is from an Archos 5/7 device\n");
		printf("  --a5it\t\tAssume the target is from an Archos 5 Internet Tablet with Android\n");
		printf("    In most cases, this can be auto-detected.\n");
		printf("\n");
		printf("  --help, -h\t\tDisplay this text\n");
		printf("  --verbose, -v\t\tBe more verbose\n");
		return 1;
	}
	
	if(!do_file(argv[optind]))
		return 1;
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}



