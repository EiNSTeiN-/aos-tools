/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <getopt.h>

#include "../libaos/libaos.h"
#include "files.h"
#include "mpk.h"

static const char *program = "aos-info";

static int header = 0;
static int list = 0;

static int device = -1;

static int verbose = 0;
static int help = 0;

static struct option options[] =
{
	/*  What to do? */
	{ "header",	no_argument,		&header, 1 },
	{ "list",		no_argument,		&list, 1 },
	{ "all",		no_argument,		0, 'a' },
	
	/* Which device keys to use? */
	{ "a5",		no_argument,		&device, MPK_DEVICE_A5 },
	{ "a5it",		no_argument,		&device, MPK_DEVICE_A5IT },
	
	/* Generic options */
	{ "verbose",	no_argument,		0, 'v' },
	{ "help",		no_argument,		0, 'h' },
	
	{ 0, 0, 0, 0 }
};

int parse_unit(struct aos_file *file)
{
	struct aos_block *block;
	struct aos_block_unit *unit;
	
	block = block_get(file, AOS_UNIT_BLOCK_ID);
	if(!block) {
		fprintf(stderr, "%s: Could not get UNIT block.\n", program);
		return 0;
	}
	
	if(block->type != AOS_TYPE_UNIT) {
		fprintf(stderr, "%s: Expected UNIT (%08x) block, got %c%c%c%c (%08x)\n", program, AOS_TYPE_UNIT, 
			((uint8_t *)&block->type)[0], ((uint8_t *)&block->type)[1], ((uint8_t *)&block->type)[2],
			((uint8_t *)&block->type)[3], block->type);
		return 0;
	}
	
	unit = (void *)block->data;
	
	printf("  Product Name:\t\t %s\n", unit->product_name);
	if(unit->product_key[0] == 0)
		printf("  Locked to Product Key: No\n");
	else
		printf("  Locked to Product Key: %s\n", unit->product_key);
	
	return 1;
}

int parse_version(struct aos_file *file)
{
	struct aos_block *block;
	struct aos_block_version *version;
	
	block = block_get(file, AOS_VERSION_BLOCK_ID);
	if(!block) {
		fprintf(stderr, "%s: Could not get VERSION block.\n", program);
		return 0;
	}
	
	if(block->type != AOS_TYPE_VERSION) {
		fprintf(stderr, "%s: Expected VERS (%08x) block, got %c%c%c%c (%08x)\n", program, AOS_TYPE_VERSION, 
			((uint8_t *)&block->type)[0], ((uint8_t *)&block->type)[1], ((uint8_t *)&block->type)[2],
			((uint8_t *)&block->type)[3], block->type);
		return 0;
	}
	
	version = (void *)block->data;
	
	printf("  Product Version:\t %u.%u.%02u\n", version->major, version->minor, version->build);
	
	return 1;
}

/* Parse the file header, determine which device key to use if needed. */
int parse_header(struct aos_file *file, int *detected_device)
{
	struct aos_block *block;
	unsigned char **keys;

	if(!aos_verify_magic(file)) {
		fprintf(stderr, "%s: File is not an AOS2 file (wrong magic).\n", program);
		return 0;
	}
	
	keys = mpk_possible_aos_keys(aos_signature_type(file));
	if(keys == NULL) {
		fprintf(stderr, "%s: Unknown signature type.\n", program);
		return 0;
	}

	printf("  Signature type:\t %s\n", mpk_signature_name(aos_signature_type(file)));
	
	if(aos_is_signed(file))
		printf("  Signature present:\t Yes\n");
	else {
		printf("  Signature present:\t No\n");
	}
	
	if(*detected_device != -1) {
		printf("  Device type:\t\t %s (forced by user)\n", mpk_device_type(*detected_device));
		
		/* The device type is specified by the user, 
			we verify the signature to make sure. */
		if(aos_is_signed(file) && !aos_verify_signature(file, keys[*detected_device])) {
			fprintf(stderr, "%s: File is signed, but signature could not be verified.\n", program);
			return 1;
		}
	}
	else {
		if(!aos_is_signed(file)) {
			
			/* TODO: if the file is not signed, but not encrypted either, 
				the device type can be detected from the UNIT block. */
			
			fprintf(stderr, "%s: Could not detect device type because the file is not signed.\n"
							"\tSpecify --a5 or --a5it.\n", program);
			return 0;
		}
		
		if(!aos_detect_key(file, keys, MPK_KNOWN_DEVICES, detected_device)) {
			fprintf(stderr, "%s: Could not detect device type from signature data.\n"
							"Specify --a5 or --a5it.", program);
			return 0;
		}
		
		printf("  Device type:\t\t %s (detected from signature data)\n", mpk_device_type(*detected_device));
	}
	
	return 1;
}

int do_file(const char *filename)
{
	struct aos_file *aos;
	unsigned int length;
	uint8_t *buffer;
	int detected_device = device;

	printf("Parsing \"%s\":\n", filename);
	
	// Load the file
	buffer = file_load(filename, &length);
	if(buffer == NULL) {
		fprintf(stderr, "%s: Could not load file.\n", program);
		return 0;
	}
	
	if(verbose)
		printf("  File size:\t\t %u bytes.\n", length);
	
	// Create the aos object
	aos = aos_create(buffer, length);
	if(aos == NULL) {
		fprintf(stderr, "%s: aos_create failed.\n", program);
		free(buffer);
		return 0;
	}
	
	// Parse the header informations
	if(!parse_header(aos, &detected_device)) {
		fprintf(stderr, "%s: Could not parse header for: %s\n", program, filename);
		aos_free(aos);
		free(buffer);
		return 0;
	}
	
	// Decrypt the file
	if(aos_is_encrypted(aos)) {
		printf("  File is Encrypted:\t Yes (decrypting...)\n");
		if(!aos_decrypt_file(aos, AES_Keys[detected_device])) {
			fprintf(stderr, "%s: Could not decrypt file.\n", program);
			
			aos_free(aos);
			free(buffer);
			return 0;
		}
	}
	else {
		printf("  File is Encrypted:\t No\n");
	}
	
	if(header) {
		printf("\n");
		printf("  Header Information:\n");
		parse_unit(aos);
		parse_version(aos);
	}
	
	if(list) {
		printf("\n");
		printf("  Blocks List:\n");
		
		/* TODO: list all blocks */
	}
	
	printf("\n");
	
	aos_free(aos);
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
		
		c = getopt_long(argc, argv, "avh", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 'a':
				header = 1;
				list = 1;
				break;
			
			case 'v':
				verbose = 1;
				break;
			
			case 'h':
				help = 1;
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
	
	if(optind >= argc) {
		printf("Note: Specify at least one file to parse.\n\n");
		help = 1;
	}
	
	if(help) {
		printf("Display useful information on the contents of an .aos archive\n\n");
		printf("Usage:\n");
		printf("  %s [options...] [files...]\n\n", program);
		printf("Options:\n");
		printf("  --header (default)\tDisplays only the header information (signature, unit, version)\n");
		printf("  --list\t\tDisplays a list of all blocks\n");
		printf("  --all, -a\t\tCombines both options above\n");
		printf("\n");
		printf("  --a5\t\t\tAssume the target .aos is for the Archos 5/7 devices\n");
		printf("  --a5it\t\tAssume the target .aos is for the Archos 5 Internet Tablet with Android\n");
		printf("    In most cases, this can be auto-detected.\n");
		printf("\n");
		printf("  --help, -h\t\tDisplay this text\n");
		printf("  --verbose, -v\t\tBe more verbose\n");
		return 1;
	}
	
	if(!header && !list)
		header = 1; /* default behaviour is to echo header info */
	
	for(;optind < argc;optind++)
             do_file(argv[optind]);
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}



