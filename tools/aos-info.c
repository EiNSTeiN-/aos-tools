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
		if(aos_is_signed(file)) {
			if(aos_verify_signature(file, keys[*detected_device]))
				printf("  Signature is valid:\t Yes\n");
			else
				printf("  Signature is valid:\t No\n");
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
		
		printf("  Signature is valid:\t Yes\n");
		printf("  Device type:\t\t %s (detected from signature data)\n", mpk_device_type(*detected_device));
	}
	
	return 1;
}

int do_aos(const char *filename, uint8_t *buffer, unsigned int length)
{
	struct aos_file *aos;
	int detected_device = device;
	
	// Create the aos object
	aos = aos_create(buffer, length);
	if(aos == NULL) {
		fprintf(stderr, "%s: aos_create failed.\n", program);
		return 0;
	}
	
	// Parse the header informations
	if(!parse_header(aos, &detected_device)) {
		fprintf(stderr, "%s: Could not parse header for: %s\n", program, filename);
		aos_free(aos);
		return 0;
	}
	
	// Decrypt the file
	if(aos_is_encrypted(aos)) {
		printf("  File is Encrypted:\t Yes (decrypting...)\n");
		if(!aos_decrypt_file(aos, AES_Keys[detected_device])) {
			fprintf(stderr, "%s: Could not decrypt file.\n", program);
			
			aos_free(aos);
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
	
	/*
	if(metadata) {
		TODO: There is metadata to be extracted from the gzip archives, 
		the cramfs archives, any compiled binary:
			- compilation paths
			- compilation timestamps
			- gzip timestamp
			- etc.
	}
	*/
	
	printf("\n");
	
	aos_free(aos);
	
	return 1;
}

int do_flash(const char *filename, uint8_t *buffer, unsigned int length)
{
	struct flash_file *flash;
	int device;
	
	if(!header) {
		printf("%s: Nothing to do for %s.\n", program, filename);
		return 0;
	}
	
	// Create the flash object
	flash = flash_create(buffer, length);
	if(flash == NULL) {
		fprintf(stderr, "%s: flash_create failed.\n", program);
		return 0;
	}
	
	printf("\n");
	printf("Header Information:\n");
	
	switch(flash->header->magic) {
		case AOS_ZMfX_MAGIC: 
			printf("  File type:\t\t Second stage bootloader\n");
			break;
		case AOS_KERNEL_MAGIC:
			printf("  File type:\t\t Kernel (zImage+initramfs)\n");
			break;
		case AOS_CRAMFS_MAGIC:
			printf("  File type:\t\t Cramfs\n");
			break;
		default:
			printf("  File type:\t\t Unknown!\n");
			break;
	}
	
	if(!flash_is_signed(flash)) {
		printf("  Signature present:\t No\n");
	}
	else {
		printf("  Signature present:\t Yes\n");
		printf("    Signature size:\t %u bits\n", flash->header->bits);
		
		// Parse & verify the header
		if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &device)) {
			printf("    Signature is valid:\t No\n");
		}
		else {
			printf("    Signature is valid:\t Yes\n");
			printf("  Device type:\t\t %s (detected from signature data)\n", mpk_device_type(device));
		}
	}
	
	if(flash->header->filesize == length) {
		printf("  Filesize:\t\t %u bytes (this is correct)\n", flash->header->filesize);
	}
	else {
		printf("  Filesize:\t\t %u bytes (this is wrong, this file is %u bytes)\n", flash->header->filesize, length);
	}
	
	printf("  Entry point:\t\t 0x%08x\n", flash->header->entrypoint);
	
	if(flash->header->cpio) {
		printf("  Initramfs present:\t Yes\n");
		printf("    Initramfs offset:\t 0x%08x\n", flash->header->cpio);
		printf("    Initramfs size:\t %u bytes\n", flash->header->cpio_size);
	} else {
		printf("  Initramfs present:\t No\n");
	}
	
	printf("\n");
	
	flash_free(flash);
	
	return 1;
}

int do_file(const char *filename)
{
	unsigned int length;
	uint8_t *buffer;

	printf("Parsing \"%s\":\n", filename);
	
	// Load the file
	buffer = file_load(filename, &length);
	if(buffer == NULL) {
		fprintf(stderr, "%s: Could not load file.\n", program);
		return 0;
	}
	
	if(verbose)
		printf("  File size:\t\t %u bytes.\n", length);
	
	switch(*(uint32_t *)buffer) {
		case AOS_MAGIC:
			do_aos(filename, buffer, length);
			break;
		case AOS_ZMfX_MAGIC:
		case AOS_KERNEL_MAGIC:
		case AOS_CRAMFS_MAGIC:
			do_flash(filename, buffer, length);
			break;
		default:
			fprintf(stderr, "%s: Unknown file type.\n", filename);
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
		
		c = getopt_long(argc, argv, "avh", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 0: break;
			case '?': break;
			
			case 'v': verbose = 1; break;
			case 'h': help = 1; break;
			
			case 'a':
				header = 1;
				list = 1;
				break;
			
			default:
				printf("Error: unknown option %02x (%c)\n", c, c);
				break;
		}
	}
	
	if(verbose || help) {
		printf("aos-info utility, part of aos-tools\n");
		printf("\thttp://code.google.com/p/aos-tools/\n");
		printf("\thttp://archos.g3nius.org/\n\n");
	}
	
	if(help) {
		printf("Display useful information on the contents of an .aos archive\n\n");
		printf("Usage:\n");
		printf("  %s [options...] [files...]\n\n", program);
		printf("Options:\n");
		printf("  --header (default)\tDisplays only the header information.\n");
		printf("  --list\t\tDisplays a list of all blocks, for .aos containers only.\n");
		printf("  --all, -a\t\tDisplay all available information (all options above).\n");
		printf("\n");
		printf("  --a5\t\t\tAssume the target .aos is for the Archos 5/7 devices\n");
		printf("  --a5it\t\tAssume the target .aos is for the Archos 5 Internet Tablet with Android\n");
		printf("    In most cases, this can be auto-detected.\n");
		printf("\n");
		printf("  --help, -h\t\tDisplay this text\n");
		printf("  --verbose, -v\t\tBe more verbose\n");
		return 1;
	}
	
	if(optind >= argc) {
		printf("Note: Specify at least one file to parse. Use --help for help.\n");
		return 1;
	}
	
	if(!header && !list)
		header = 1; /* default behaviour is to echo header info */
	
	for(;optind<argc;optind++)
             do_file(argv[optind]);
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}
