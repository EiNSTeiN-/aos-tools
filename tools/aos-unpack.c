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

static const char *program = "aos-unpack";

static int extract = 0;
static int decrypt = 0;
static char output[2048];
static int force = 0;
static int create_subfolder = 1;
static const char *zimage = NULL;
static const char *initramfs = NULL;

static int device = -1;

static int verbose = 0;
static int help = 0;

static struct option options[] =
{
	/*  What to do? */
	{ "extract",	no_argument,		0, 'x' },
	{ "decrypt",	no_argument,		0, 'd' },
	
	/* Options */
	{ "output",	required_argument,	0, 'o' },
	{ "force",		no_argument,		0, 'f' },
	
	/* For --extract with kernel flash segment */
	{ "zimage",	required_argument,	0, 'i' },
	{ "initramfs",	required_argument,	0, 'r' },
	
	/* Which device keys to use? */
	{ "a5",		no_argument,		&device, MPK_DEVICE_A5 },
	{ "a5it",		no_argument,		&device, MPK_DEVICE_A5IT },
	
	/* Generic options */
	{ "verbose",	no_argument,		0, 'v' },
	{ "help",		no_argument,		0, 'h' },
	
	{ 0, 0, 0, 0 }
};

#define endswith(a, b) \
	(strlen(a) > strlen(b) && !strcasecmp(&a[strlen(a)-strlen(b)], b))

int parse_flash_partition(uint8_t *data, unsigned int length, const char *partition_name, uint32_t offset, const char *filepath, int detected_device)
{
	struct flash_file *flash;
	int file_device;
	
	flash = flash_create(data, length);
	if(!flash) {
		fprintf(stderr, "%s: Memory error\n", program);
		return 0;
	}
	
	switch(flash->header->magic) {
		case AOS_ZMfX_MAGIC: {
			if(verbose) printf("\t%s: Stage 2 bootloader\n", filepath);
			break;
		}
		case AOS_KERNEL_MAGIC: {
			if(verbose) printf("\t%s: Kernel (zImage+initramfs)\n", filepath);
			break;
		}
		case AOS_GZIP_MAGIC: {
			if(verbose) printf("\t%s: Boot Logo\n", filepath);
			break;
		}
		default: {
			if(offset == 0x3f8000 || offset == 0x000000) {
				// We know those are never signed, so we don't warn about them
				flash_free(flash);
				return 0;
			}
			
			printf("\t%s: Unknown magic, this is weird.\n", filepath);
			flash_free(flash);
			return 0;
		}
	}
	
	if(flash->header->magic != AOS_GZIP_MAGIC) {
		if(flash->header->bits != 0x400) {
			if(verbose)
				printf("\t%s: Signature is 0 bits long (SDE Firmware?).\n", filepath);
		}
		else {
			if(!flash_is_signed(flash)) {
				if(verbose)
					printf("\t%s: File is not signed.\n", filepath);
			}
			else {
				if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &file_device)) {
					printf("WARNING: %s: Signature is not valid.\n", filepath);
				}
				else {
					if(detected_device != file_device) {
						printf("WARNING: %s: The detected device type for this file does not match the .aos device type (detected %s).\n", filepath, mpk_device_type(file_device));
					}
					else {
						if(verbose)
							printf("\t%s: Signature is valid, detected device type %s.\n", filepath, mpk_device_type(file_device));
					}
				}
			}
		}
	}
	
	if(flash->header->magic == AOS_KERNEL_MAGIC) {
		unsigned int gz_length;
		uint8_t *gz_data;
		char *folder_name;
		
		gz_data = (uint8_t *)flash->header+flash->header->cpio;
		gz_length = (flash->length - flash->header->cpio);
		
		if(*(uint32_t *)gz_data == AOS_GZIP_MAGIC) {
			folder_name = strdup((const char *)&gz_data[10]);
			if(endswith(folder_name, ".cpio"))
				folder_name[strlen(folder_name)-strlen(".cpio")] = 0;
		}
		else if((*(uint32_t *)gz_data & AOS_GZIP_NONAME_MASK) == AOS_GZIP_NONAME_MAGIC) {
			folder_name = bprintf("%s", partition_name);
		}
		else {
			printf("error: Could not find GZIP magic at given offset in %s.\n", filepath);
			flash_free(flash);
			return 0;
		}
		
		// Unpack script
		log_write("unpack.sh", "## zImage+initramfs: %s\n", filepath);
		log_write("unpack.sh", "rm -f -r unpacked/%s/\n", folder_name);
		log_write("unpack.sh", "mkdir -p unpacked/%s/initramfs/\n", folder_name);
		log_write("unpack.sh", "aos-unpack %s --output unpacked/%s/ --zimage zImage --initramfs initramfs.cpio.gz\n", filepath, folder_name);
		log_write("unpack.sh", "gunzip --decompress unpacked/%s/initramfs.cpio.gz\n", folder_name);
		log_write("unpack.sh", "(cd unpacked/%s/initramfs/ && cpio -i -d -H newc -F ../initramfs.cpio --no-absolute-filenames)\n", folder_name, folder_name);
		log_write("unpack.sh", "rm unpacked/%s/initramfs.cpio\n", folder_name);
		log_write("unpack.sh", "\n");
		
		// Repack script
		log_write("repack.sh", "## zImage+initramfs: %s\n", filepath);
		log_write("repack.sh", "(cd unpacked/%s/initramfs/ && find . | cpio -o -H newc -F ../initramfs.cpio)\n", folder_name);
		log_write("repack.sh", "rm -f -r unpacked/%s/initramfs/\n", folder_name);
		log_write("repack.sh", "gzip --name --best unpacked/%s/initramfs.cpio\n", folder_name);
		log_write("repack.sh", "aos-repack --kernel %s --zimage unpacked/%s/zImage --initramfs unpacked/%s/initramfs.cpio.gz\n", filepath, folder_name, folder_name);
		log_write("repack.sh", "rm -f -r  unpacked/%s/\n", folder_name);
		log_write("repack.sh", "\n");
		
		free(folder_name);
	}
	else if(flash->header->magic == AOS_GZIP_MAGIC) {
		const char *gz_name = (const char *)&data[10];
		
		log_write("unpack.sh", "## .gz: %s\n", filepath);
		log_write("unpack.sh", "mkdir -p unpacked/\n", gz_name);
		log_write("unpack.sh", "rm -f unpacked/%s\n", gz_name);
		log_write("unpack.sh", "cp %s unpacked/%s.gz\n", filepath, gz_name);
		log_write("unpack.sh", "gunzip -d unpacked/%s.gz\n", gz_name);
		log_write("unpack.sh", "\n");
		
		log_write("repack.sh", "## .gz: %s\n", filepath);
		log_write("repack.sh", "gzip -N --best unpacked/%s\n", gz_name);
		log_write("repack.sh", "cp unpacked/%s.gz %s\n", gz_name, filepath);
		log_write("repack.sh", "rm -f unpacked/%s.gz\n", gz_name);
		log_write("repack.sh", "\n");
	}
	
	flash_free(flash);
	
	return 1;
}

const char *flash_name(uint32_t offset)
{
	switch(offset) {
		case 0x00000000: return "boot0";
		case 0x00010000: return "keystore";
		case 0x00030000: return "boot1";
		case 0x00060000: return "logo";
		case 0x00080000: return "recovery";
		case 0x00210000: return "init";
		case 0x003F8000: return "params";
		default: return "unknown";
	}
	return "unknown";
}

int parse_flash_block(struct aos_block *block, unsigned int blocknumber, int detected_device)
{
	struct aos_block_flash *flash = (struct aos_block_flash *)block->data;
	char *name;
	
	if(verbose)
		printf("(%u) Flash segment \"%s\" at offset 0x%08x.\n", blocknumber, flash_name(flash->offset), flash->offset);
	
	switch(flash->offset) {
		case 0x00000000: break;
		case 0x003f8000: break;
		case 0x00060000: break;
		
		case 0x00010000:
			printf("WARNING: keys are changed in this update!\n");
			break;
		case 0x00030000:
			if(*(uint32_t *)flash->data != AOS_ZMfX_MAGIC)
				printf("WARNING: Wrong ZMfX header flashed at 0x030000 (this is bad!)\n");
			break;
		case 0x00080000:
			if(*(uint32_t *)flash->data != AOS_KERNEL_MAGIC)
				printf("WARNING: Wrong kernel header flashed at 0x080000 (this is bad!)\n");
			break;
		case 0x00210000:
			if(*(uint32_t *)flash->data != AOS_KERNEL_MAGIC)
				printf("WARNING: Wrong kernel header flashed at 0x210000 (this is bad!)\n");
			break;
	}
	
	name = bprintf("flash/%s_0x%08x", flash_name(flash->offset), flash->offset);
	mkdir_recursive(name);
	file_write(name, flash->data, flash->size);
	
	log_write("digest", "flash 0x%08x %s\n", flash->offset, name);
	
	parse_flash_partition(flash->data, flash->size, flash_name(flash->offset), flash->offset, name, detected_device);
	free(name);
	
	return 1;
}

int parse_mtd_block(struct aos_block *block, unsigned int blocknumber, int detected_device)
{
	struct aos_block_mtd *mtd = (struct aos_block_mtd  *)block->data;
	char *name;
	
	if(verbose)
		printf("(%u) MTD \"%s\" at offset 0x%08x.\n", blocknumber, mtd->name, mtd->offset);
	
	name = bprintf("flash/%s-0x%08x", mtd->name, mtd->offset);
	mkdir_recursive(name);
	file_write(name, mtd->data, mtd->size);
	
	log_write("digest", "mtd %s 0x%08x %s\n", mtd->name, mtd->offset, name);
	
	parse_flash_partition(mtd->data, mtd->size, mtd->name, mtd->offset, name, detected_device);
	free(name);
	
	return 1;
}

int parse_cramfs_archive(const char *filename, uint8_t *data, unsigned int length, const char *filepath, int detected_device)
{
	struct flash_file *flash;
	int file_device;
	char cramfs_name[2048];
	
	flash = flash_create(data, length);
	if(!flash) {
		fprintf(stderr, "%s: Memory error\n", program);
		return 0;
	}
	
	if(flash->header->magic != AOS_CRAMFS_MAGIC) {
		fprintf(stderr, "%s: Invalid Cramfs magic on file.\n", filename);
		flash_free(flash);
		return 0;
	}
	
	if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &file_device)) {
		printf("WARNING: %s: Signature is invalid.\n", filename);
	}
	else {
		if(detected_device != file_device) {
			printf("WARNING: %s: The detected device type for this file does not match the .aos device type (detected %s).\n", filename, mpk_device_type(file_device));
		}
		else {
			if(verbose)
				printf("\t%s: Signature is valid, detected device type %s\n", filename, mpk_device_type(file_device));
		}
	}
	
	strcpy(cramfs_name, filename);
	cramfs_name[strlen(cramfs_name)-strlen(".secure")] = 0;
	
	log_write("unpack.sh", "## .cramfs.secure: %s\n", filepath);
	log_write("unpack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
	log_write("unpack.sh", "mkdir -p unpacked\n");
	if(*(uint32_t *)&flash->header->data[0] == 0x28cd3d45 /* CRAMFS_MAGIC */) {
		/* this is a cramfs with a small header (0x100 bytes), which is nonstandard for a cramfs file. */
		log_write("unpack.sh", "dd if=%s of=unpacked/%s.stripped bs=256 skip=1\n", filepath, cramfs_name);
		log_write("unpack.sh", "(cd unpacked/ && cramfsck -v -x %s %s.stripped)\n", cramfs_name, cramfs_name);
		log_write("unpack.sh", "rm unpacked/%s.stripped\n", cramfs_name);
	} else {
		/* this is a cramfs file with either no header or a standard cramfs header (0x200 bytes). */
		log_write("unpack.sh", "cramfsck -x unpacked/%s %s\n", cramfs_name, filepath);
	}
	log_write("unpack.sh", "\n");
	
	log_write("repack.sh", "## .cramfs.secure: %s\n", filepath);
	if(*(uint32_t *)&flash->header->data[0] == 0x28cd3d45 /* CRAMFS_MAGIC */) {
		log_write("repack.sh", "mkcramfs unpacked/%s unpacked/%s.tmp\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
		log_write("repack.sh", "aos-fix --add-header unpacked/%s.tmp\n", cramfs_name);
		log_write("repack.sh", "mv unpacked/%s.tmp unpacked/%s.secure\n", cramfs_name);
	} else {
		// create a cramfs with padding for the header, and write the header in-place
		log_write("repack.sh", "mkcramfs -p unpacked/%s unpacked/%s.secure\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
		log_write("repack.sh", "aos-fix --add-header --overwrite unpacked/%s.secure\n", cramfs_name);
	}
	log_write("repack.sh", "mv unpacked/%s.secure %s\n", cramfs_name, filepath);
	log_write("repack.sh", "\n");
	
	flash_free(flash);
	
	return 1;
}

int parse_copy_block(struct aos_block *block, unsigned int blocknumber, int detected_device)
{
	struct aos_block_copy *copy = (struct aos_block_copy  *)block->data;
	char *name;
	
	if(verbose)
		printf("(%u) Copy \"%s\".\n", blocknumber, copy->name);
	
	// Choose the destination name
	switch(copy->partition) {
		case AOS_PARTITION_SYSTEM: {
			name = bprintf("root/system/%s", copy->name);
			break;
		}
		case AOS_PARTITION_DATA: {
			name = bprintf( "root/data/%s", copy->name);
			break;
		}
		case AOS_PARTITION_CRAMFS: {
			name = bprintf("root/cramfs/%s", copy->name);
			break;
		}
		default: {
			name = bprintf("root/copy-%u/%s", copy->partition, copy->name);
			break;
		}
	}
	
	// Make sure the folder exist, and write the file to disk
	mkdir_recursive(name);
	file_write(name, copy->data, copy->size);
	
	// Write to digest
	if(strchr(copy->name, ' ') == NULL) // Quirks to keep digest clean
		log_write("digest", "copy %u %s %s\n", copy->partition, copy->name, name);
	else
		log_write("digest", "copy %u \"%s\" \"%s\"\n", copy->partition, copy->name, name);
	
	// Handle .cramfs.secure files
	if(!strcasecmp(&copy->name[strlen(copy->name)-strlen(".cramfs.secure")], ".cramfs.secure")) {
		parse_cramfs_archive(copy->name, copy->data, copy->size, name, detected_device);
	}
	
	free(name);
	
	return 1;
}

int parse_shell_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_shell *shell= (struct aos_block_shell  *)block->data;
	char *name;
	
	if(verbose)
		printf("(%u) Shell script.\n", blocknumber);
	
	name = bprintf("shell/script_%u.sh", blocknumber);
	
	// Make sure the folder exist, and write the file to disk
	mkdir_recursive(name);
	file_write(name, shell->data, shell->length);
	
	// Write to digest
	log_write("digest", "shell %s\n", name);
	
	free(name);
	
	return 1;
}

int parse_delete_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_delete *delete = (struct aos_block_delete  *)block->data;
	
	if(verbose)
		printf("(%u) Delete \"%s\".\n", blocknumber, delete->name);
	
	// Write the delete command to digest
	if(strchr(delete->name, ' ') == NULL) // Quirks to keep digest clean
		log_write("digest", "delete %u %s\n", delete->partition, delete->name);
	else
		log_write("digest", "delete %u \"%s\"\n", delete->partition, delete->name);
	
	return 1;
}

int parse_other_block(struct aos_block *block, unsigned int blocknumber)
{
	char *name;
	
	if(verbose)
		printf("(%u) Raw block, type %c%c%c%c.\n", blocknumber, 
			*(uint8_t *)block, *((uint8_t *)block+1), *((uint8_t *)block+2), *((uint8_t *)block+3));
	
	// write the block to disk
	name = bprintf("raw/%u_%c%c%c%c", blocknumber,
		*(uint8_t *)block, *((uint8_t *)block+1), *((uint8_t *)block+2), *((uint8_t *)block+3));
	mkdir_recursive(name);
	file_write(name, block->data, block->length-sizeof(struct aos_block));
	
	// log the block in the digest file
	log_write("digest", "raw %c%c%c%c %s\n",
		*(uint8_t *)block, *((uint8_t *)block+1), *((uint8_t *)block+2),
		*((uint8_t *)block+3), name);
	free(name);
	
	return 1;
}

int parse_unit_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_unit *unit = (struct aos_block_unit  *)block->data;
	
	if(verbose)
		printf("(%u) Unit type %s.\n", blocknumber, unit->product_name);
	
	if(verbose && unit->product_key[0] != 0)
		printf("(%u) Locked to product key %s.\n", blocknumber, unit->product_key);
	
	if(unit->product_key[0] != 0)
		log_write("digest", "unit %s %s\n", unit->product_name, unit->product_key);
	else
		log_write("digest", "unit %s\n", unit->product_name);
	
	return 1;
}

int parse_version_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_version *version = (struct aos_block_version  *)block->data;
	
	if(verbose)
		printf("(%u) Version number %u.%u.%02u.\n", blocknumber, version->major, version->minor, version->build);
	
	log_write("digest", "version %u.%u.%02u\n", version->major, version->minor, version->build);
	
	return 1;
}

int parse_boot_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_boot *boot = (struct aos_block_boot  *)block->data;
	
	if(verbose)
		printf("(%u) Flag \"reboot device\" is %s.\n", blocknumber, (boot->value ? "set" : "specifically not set"));
	
	log_write("digest", "boot %u\n", boot->value);
	
	return 1;
}

int parse_adel_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_adel *adel = (struct aos_block_adel  *)block->data;
	
	if(verbose)
		printf("(%u) Flag \"delete .aos\" is %s.\n", blocknumber, (adel->value ? "set" : "specifically not set"));
	
	log_write("digest", "adel %u\n", adel->value);
	
	return 1;
}

int parse_cipher_block(struct aos_block *block, unsigned int blocknumber)
{
	struct aos_block_cipher *cipher = (struct aos_block_cipher  *)block->data;
	char str[AES_BLOCK_SIZE*2 + 1];
	unsigned int i;
	
	for(i=0;i<AES_BLOCK_SIZE;i++) {
		sprintf(&str[i*2], "%02x", cipher->data[i]);
	}
	
	str[AES_BLOCK_SIZE*2] = 0;
	
	if(verbose)
		printf("(%u) Cipher data: %s\n", blocknumber, str);
	
	log_write("digest", "cipher %s\n", str);
	
	return 1;
}

int do_extract_aos(const char *filename, struct aos_file *file, int detected_device)
{
	struct aos_block *block;
	char cwd[2048];
	char *tmp_folder;
	unsigned int blocknumber;
	
	// In case of multiple files to extract, we create subfolders
	if(create_subfolder) {
		const char *tmp_filename = filename;
		
		while(strchr(tmp_filename, '/')) tmp_filename = strchr(tmp_filename, '/')+1;
		tmp_folder = bprintf("%s/%s/", output, tmp_filename);
		if(endswith(tmp_folder, ".aos/")) {
			tmp_folder[strlen(tmp_folder)-strlen(".aos/")] = '/';
			tmp_folder[strlen(tmp_folder)-strlen(".aos/")+1] = 0;
		}
	}
	else {
		tmp_folder = bprintf("%s/", output);
	}
	
	// we keep a copy of the cwd
	getcwd(cwd, sizeof(cwd));
	
	// create the output folder and cd into it
	mkdir_recursive(tmp_folder);
	chdir(tmp_folder);
	free(tmp_folder);
	
	// possibly remove the digest & log file, if they exist
	log_clean("digest");
	log_clean("unpack.sh");
	log_clean("repack.sh");
	
	log_write("unpack.sh", "#!/bin/sh\n");
	log_write("repack.sh", "#!/bin/sh\n");
	
	// Parse all blocks, starting at the cipher block.
	blocknumber = 0;
	block = block_get(file, AOS_CIPHER_BLOCK_ID);
	while(block) {
		switch(block->type) {
			case AOS_TYPE_CIPHER:		parse_cipher_block(block, blocknumber); break;
			case AOS_TYPE_UNIT:		parse_unit_block(block, blocknumber); break;
			case AOS_TYPE_VERSION:	parse_version_block(block, blocknumber); break;
			case AOS_TYPE_DURATION:	/* Nothing to do */ break;
			case AOS_TYPE_FLASH:		parse_flash_block(block, blocknumber, detected_device); break;
			case AOS_TYPE_MTD:		parse_mtd_block(block, blocknumber, detected_device); break;
			case AOS_TYPE_COPY:		parse_copy_block(block, blocknumber, detected_device); break;
			case AOS_TYPE_DELETE:		parse_delete_block(block, blocknumber); break;
			case AOS_TYPE_BOOT:		parse_boot_block(block, blocknumber); break;
			case AOS_TYPE_ADEL:		parse_adel_block(block, blocknumber); break;
			case AOS_TYPE_SHELL:		parse_shell_block(block, blocknumber); break;
			default: 					parse_other_block(block, blocknumber); break;
		}
		blocknumber++;
		block = block_next(file, block);
	}
	
	printf("Extracted %u blocks from %s.\n", blocknumber, filename);
	
	// return to where we were before...
	chdir(cwd);
	
	return 1;
}

int parse_aos_header(struct aos_file *file, int *detected_device)
{
	unsigned char **keys;

	if(!aos_verify_magic(file)) {
		fprintf(stderr, "%s: Not an AOS2 file (wrong magic).\n", program);
		return 0;
	}
	
	keys = mpk_possible_aos_keys(aos_signature_type(file));
	if(keys == NULL) {
		fprintf(stderr, "%s: Unknown signature type.\n", program);
		return 0;
	}
	
	if(*detected_device != -1) {
		
		if(verbose)
			printf("Device type: %s (forced by user)\n", mpk_device_type(*detected_device));
		
		/* The device type is specified by the user, 
			we verify the signature to make sure. */
		if(aos_is_signed(file) && !aos_verify_signature(file, keys[*detected_device])) {
			fprintf(stderr, "%s: File is signed, but signature is invalid. %s\n", program, (force ? " Continuing anyway (forced by user)." : " Bailing out, use --force to do it anyway."));
			return (force ? 1 : 0);
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
		
		if(verbose)
			printf("Device type: %s (detected from signature data)\n", mpk_device_type(*detected_device));
	}
	
	return 1;
}

int do_dump_aos(const char *filename, struct aos_file *aos)
{
	char *ptr, *b;
	char *dump_path;
	
	b = ptr = strdup(filename);
	while(strchr(ptr, '/')) ptr = strchr(ptr, '/')+1;
	
	// remove .aos extension from filename
	if(endswith(ptr, ".aos"))
		ptr[strlen(ptr)-strlen(".aos")] = 0;
	
	dump_path = bprintf("%s/%s-decrypted.aos", output, ptr);
	
	file_write(dump_path, aos->data, aos->length);
	printf("Wrote %u bytes to %s\n", aos->length, dump_path);
	
	free(b);
	free(dump_path);
	
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
		return 1;
	}
	
	// Parse & verify the header
	if(!parse_aos_header(aos, &detected_device)) {
		aos_free(aos);
		return 1;
	}
	
	// Decrypt the file
	if(aos_is_encrypted(aos)) {
		if(!aos_decrypt_file(aos, AES_Keys[detected_device])) {
			fprintf(stderr, "%s: Failed to decrypt the file.\n", program);
			
			aos_free(aos);
			return 1;
		}
		else {
			if(verbose)
				printf("File was decrypted successfully.\n");
		}
	}
	else {
		if(verbose)
			printf("File is NOT encrypted.\n");
	}
	
	if(decrypt) {
		// dump the decrypted file to disk
		do_dump_aos(filename, aos);
	}
	
	if(extract) {
		// extract individual blocks to disk
		do_extract_aos(filename, aos, detected_device);
	}
	
	aos_free(aos);
	
	return 0;
}

int do_extract_flash(const char *filename, struct flash_file *flash)
{
	const char *ptr;
	char *zimage_path, *initramfs_path;
	uint8_t *zimage_buffer;
	unsigned int zimage_length;
	uint8_t *initramfs_buffer;
	unsigned int initramfs_length;
	
	ptr = filename;
	while(strchr(ptr, '/')) ptr = strchr(ptr, '/')+1;
	
	if(zimage)
		zimage_path = bprintf("%s/%s", output, zimage);
	else
		zimage_path = bprintf("%s/%s-zImage", output, ptr);
	
	if(initramfs)
		initramfs_path = bprintf("%s/%s", output, initramfs);
	else
		initramfs_path = bprintf("%s/%s-initramfs.cpio.gz", output, ptr);
	
	zimage_buffer = (uint8_t *)&flash->header->data;
	zimage_length = flash->header->cpio - sizeof(struct flash_header);
	
	initramfs_buffer = (uint8_t *)flash->header + flash->header->cpio;
	initramfs_length = flash->header->cpio_size;
	
	mkdir_recursive(zimage_path);
	file_write(zimage_path, zimage_buffer, zimage_length);
	printf("Wrote %u bytes to %s\n", zimage_length, zimage_path);
	free(zimage_path);
	
	mkdir_recursive(initramfs_path);
	file_write(initramfs_path, initramfs_buffer, initramfs_length);
	printf("Wrote %u bytes to %s\n", initramfs_length, initramfs_path);
	free(initramfs_path);
	
	return 1;
}

int parse_flash_header(struct flash_file *file, int *detected_device)
{
	if(file->header->magic != AOS_KERNEL_MAGIC) {
		fprintf(stderr, "%s: Not a kernel flash segment file (wrong magic).\n", program);
		return 0;
	}
	
	if(*detected_device != -1) {
		if(verbose)
			printf("Device type: %s (forced by user)\n", mpk_device_type(*detected_device));
		
		/* The device type is specified by the user, 
			we verify the signature to make sure. */
		if(flash_is_signed(file) && !flash_verify_signature(file, Bootloader_Keys[*detected_device])) {
			fprintf(stderr, "%s: File is signed, but signature is invalid. %s\n", program, (force ? " Continuing anyway (forced by user)." : " Bailing out, use --force to do it anyway."));
			return (force ? 1 : 0);
		}
	}
	else {
		if(!flash_is_signed(file)) {
			fprintf(stderr, "%s: Could not detect device type because the file is not signed.\n"
							"\tSpecify --a5 or --a5it.\n", program);
			return 0;
		}
		
		if(!flash_detect_key(file, Bootloader_Keys, MPK_KNOWN_DEVICES, detected_device)) {
			fprintf(stderr, "%s: Could not detect device type from signature data.\n"
							"Specify --a5 or --a5it.", program);
			return 0;
		}
		
		if(verbose)
			printf("Device type: %s (detected from signature data)\n", mpk_device_type(*detected_device));
	}
	
	return 1;
}

int do_flash(const char *filename, uint8_t *buffer, unsigned int length)
{
	struct flash_file *flash;
	int detected_device = device;

	// Create the aos object
	flash = flash_create(buffer, length);
	if(flash == NULL) {
		fprintf(stderr, "%s: flash_create failed.\n", program);
		return 1;
	}
	
	// Parse & verify the header
	if(!parse_flash_header(flash, &detected_device)) {
		flash_free(flash);
		return 1;
	}
	
	if(decrypt)
		printf("%s: --decrypt is meaningless with this file type.\n", program);
	
	if(extract) {
		// extract individual blocks to disk
		do_extract_flash(filename, flash);
	}
	
	flash_free(flash);
	
	return 0;
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
	
	switch(*(uint32_t *)buffer) {
		case AOS_MAGIC:
			do_aos(filename, buffer, length);
			break;
		case AOS_KERNEL_MAGIC:
			do_flash(filename, buffer, length);
			break;
		default:
			fprintf(stderr, "%s: Unsupported file type.\n", program);
			break;
	}
	
	free(buffer);
	
	return 1;
}

int main(int argc, char *argv[])
{
	if(argc > 0)
		program = argv[0];
	
	memset(output, 0, sizeof(output));
	
	while(1)
	{
		int index = 0;
		int c;
		
		c = getopt_long(argc, argv, "vhxdo:fi:r:", options, &index);
		if(c == -1) break;
		
		switch(c)
		{
			case 0: break;
			case '?': break;
			
			case 'v': verbose = 1; break;
			case 'h': help = 1; break;
			
			case 'x': extract = 1; break;
			case 'd': decrypt = 1; break;
			case 'o': if(optarg) strcpy(output, optarg); break;
			case 'f': force = 1; break;
			case 'i': zimage = optarg; break;
			case 'r': initramfs = optarg; break;
			
			default:
				printf("Error: unknown option %02x (%c)\n", c, c);
				break;
		}
	}
	
	if(verbose || help) {
		printf("aos-unpack utility, part of aos-tools\n");
		printf("\thttp://code.google.com/p/aos-tools/\n");
		printf("\thttp://archos.g3nius.org/\n\n");
	}
	
	if(help) {
		printf("Unpack the content of an aos archive or a kernel flash segment to disk.\n\n");
		printf("Usage:\n");
		printf("  %s [options...] [files...]\n\n", program);
		printf("Options:\n");
		printf("  --extract, -x (default)\tExtract all parts of the file to disk.\n");
		printf("  --decrypt, -d\t\tDecrypt the file (.aos only), and dump the decrypted output to disk.\n");
		printf("\n");
		printf("  --output, -o [out]\t\tSpecify the output folder.\n");
		printf("  --force, -f\t\tForce unpacking even if signature is invalid.\n");
		printf("  --zimage, -i [file]\tUse this filename for zImage. Meaningful only with a kernel flash segment.\n");
		printf("  --initramfs, -r [file]\tUse this filename for initramfs. Meaningful only with a kernel flash segment.\n");
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
	
	if(output[0] == 0) {
		// Defaults output folder to the current working directory
		getcwd(output, sizeof(output));
		
		create_subfolder = 1;
	} else {
		// if more than one files to parse, then we create subfolders
		create_subfolder = ((argc-optind != 1) ? 1 : 0);
		
		if(output[0] != '/') {
			char tmp_output[2048];
			
			// expand relative path
			getcwd(tmp_output, sizeof(tmp_output));
			if(strlen(tmp_output)+1+strlen(output) < sizeof(output)) {
				strcat(tmp_output, "/");
				strcat(tmp_output, output);
				strcpy(output, tmp_output);
			}
			else {
				fprintf(stderr, "%s: Memory error.\n", program);
				return 1;
			}
		}
	}
	
	// remove tailing slash from the output path
	if(strlen(output) > 0 && output[strlen(output)-1] == '/')
		output[strlen(output)-1]  = 0;
	
	if(verbose)
		printf("Will create output files in %s\n", output);
	
	if(!extract && !decrypt)
		extract = 1; /* default behaviour */
	
	for(;optind<argc;optind++)
             do_file(argv[optind]);
	
	if(verbose) {
		printf("\n");
		printf("Done.\n");
	}
	
	return 0;
}



