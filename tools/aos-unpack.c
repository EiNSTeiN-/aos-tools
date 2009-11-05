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

int parse_flash_partition(uint8_t *data, unsigned int length, const char *partition_name, uint32_t offset, const char *filepath)
{
	struct flash_file *flash;
	int device;
	
	flash = flash_create(data, length);
	if(!flash) {
		printf("error: Memory\n");
		return 0;
	}
	
	switch(flash->header->magic) {
		case AOS_ZMfX_MAGIC: {
			printf("Detected ZMfX magic on file %s, probably the stage 2 bootloader\n", filepath);
			break;
		}
		case AOS_CPIO_MAGIC: {
			printf("Detected cpio magic on file %s, (init or recovery cpios)\n", filepath);
			break;
		}
		case AOS_GZIP_MAGIC: {
			printf("Detected gzip magic on file %s, (boot logo)\n", filepath);
			break;
		}
		default: {
			if(offset == 0x3f8000 || offset == 0x000000)
				// We know those are never signed, so we don't warn about them
				return 0;
			
			printf("Unknown magic on file %s, file is probably not signed.\n", filepath);
			return 0;
		}
	}
	
	if(flash->header->magic != AOS_GZIP_MAGIC) {
		if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &device)) {
			printf("error: Could not verify signature on file %s\n", filepath);
			return 0;
		}
		else
			printf("Verified signature on file %s, detected device type %s\n", filepath, mpk_device_type(device));
	}
	
	if(flash->header->magic == AOS_CPIO_MAGIC) {
		unsigned int gz_length;
		uint8_t *gz_data;
		const char *cpio_name;
		
		gz_data = (uint8_t *)flash->header+flash->header->cpio;
		gz_length = (flash->length - flash->header->cpio);
		
		if(*(uint32_t *)gz_data != AOS_GZIP_MAGIC) {
			printf("error: Could not find GZIP magic at given offset in %s.\n", filepath);
			return 0;
		}
		
		cpio_name = (const char *)&gz_data[10];
		
		// Unpack script
		log_write("unpack.sh", "## .cpio.gz: %s\n", filepath);
		log_write("unpack.sh", "rm -f -r unpacked/%s\n", cpio_name);
		log_write("unpack.sh", "mkdir -p unpacked/%s/\n", cpio_name);
		log_write("unpack.sh", "aos-flash %s -x unpacked/%s/%s.gz\n", filepath, cpio_name, cpio_name);
		log_write("unpack.sh", "gunzip -d unpacked/%s/%s.gz\n", cpio_name, cpio_name);
		log_write("unpack.sh", "(cd unpacked/%s/ && cpio -i -d -H newc -F %s --no-absolute-filenames)\n", cpio_name, cpio_name);
		log_write("unpack.sh", "rm unpacked/%s/%s\n", cpio_name, cpio_name);
		log_write("unpack.sh", "\n");
		
		// Repack script
		log_write("repack.sh", "## .cpio.gz: %s\n", filepath);
		log_write("repack.sh", "(cd unpacked/%s/ && find . | cpio -o -H newc -F ../%s.tmp)\n", cpio_name, cpio_name);
		log_write("repack.sh", "rm -f -r unpacked/%s\n", cpio_name);
		log_write("repack.sh", "mv unpacked/%s.tmp  unpacked/%s\n", cpio_name, cpio_name);
		log_write("repack.sh", "gzip -N --best unpacked/%s\n", cpio_name, cpio_name);
		log_write("repack.sh", "aos-flash %s -i unpacked/%s.gz\n", filepath, cpio_name);
		log_write("repack.sh", "rm -f unpacked/%s.gz\n", cpio_name);
		log_write("repack.sh", "\n");
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

int parse_flash_block(struct aos_block *block)
{
	struct aos_block_flash *flash = (struct aos_block_flash *)block->data;
	char *name;
	
	name = bprintf("flash/%s_0x%08x", flash_name(flash->offset), flash->offset);
	mkdir_recursive(name);
	file_write(name, flash->data, flash->size);
	
	log_write("digest", "flash 0x%08x /%s\n", flash->offset, name);
	
	parse_flash_partition(flash->data, flash->size, flash_name(flash->offset), flash->offset, name);
	free(name);
	
	return 1;
}

int parse_mtd_block(struct aos_block *block)
{
	struct aos_block_mtd *mtd = (struct aos_block_mtd  *)block->data;
	char *name;
	
	name = bprintf("flash/%s-0x%08x", mtd->name, mtd->offset);
	mkdir_recursive(name);
	file_write(name, mtd->data, mtd->size);
	
	log_write("digest", "mtd %s 0x%08x /%s\n", mtd->name, mtd->offset, name);
	
	parse_flash_partition(mtd->data, mtd->size, mtd->name, mtd->offset, name);
	free(name);
	
	return 1;
}

int parse_cramfs_archive(const char *filename, uint8_t *data, unsigned int length, const char *filepath)
{
	struct flash_file *flash;
	int device;
	char cramfs_name[2048];
	
	flash = flash_create(data, length);
	if(!flash) {
		printf("error: Memory\n");
		return 0;
	}
	
	if(flash->header->magic != AOS_CRAMFS_MAGIC) {
		printf("error: Invalid CRAMFS magic on file %s\n", filename);
		return 0;
	}
	
	if(!flash_detect_key(flash, Bootloader_Keys, MPK_KNOWN_DEVICES, &device)) {
		printf("error: Could not verify signature on file %s\n", filename);
		return 0;
	}
	
	printf("Verified signature on file %s, detected device type %s\n", filename, mpk_device_type(device));
	
	strcpy(cramfs_name, filename);
	cramfs_name[strlen(cramfs_name)-strlen(".secure")] = 0;
	
	log_write("unpack.sh", "## .cramfs.secure: %s\n", filepath);
	log_write("unpack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
	log_write("unpack.sh", "mkdir -p unpacked\n");
	if(*(uint32_t *)&flash->header->data[0] == 0x28cd3d45 /* CRAMFS_MAGIC */) {
		log_write("unpack.sh", "dd if=%s of=unpacked/%s.stripped bs=256 skip=1\n", filepath, cramfs_name);
		log_write("unpack.sh", "(cd unpacked/ && cramfsck -v -x %s %s.stripped)\n", cramfs_name, cramfs_name);
		log_write("unpack.sh", "rm unpacked/%s.stripped\n", cramfs_name);
	} else {
		log_write("unpack.sh", "cramfsck -x unpacked/%s %s\n", cramfs_name, filepath);
	}
	log_write("unpack.sh", "\n");
	
	log_write("repack.sh", "## .cramfs.secure: %s\n", filepath);
	if(*(uint32_t *)&flash->header->data[0] == 0x28cd3d45 /* CRAMFS_MAGIC */) {
		log_write("repack.sh", "mkcramfs unpacked/%s unpacked/%s.tmp\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
		log_write("repack.sh", "dd if=%s of=unpacked/%s.secure bs=256 count=1\n", filepath, cramfs_name);
		log_write("repack.sh", "dd if=unpacked/%s.tmp of=unpacked/%s.secure bs=256 seek=1\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f unpacked/%s.tmp\n", cramfs_name);
	} else {
		log_write("repack.sh", "mkcramfs -p unpacked/%s unpacked/%s.tmp\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f -r unpacked/%s\n", cramfs_name);
		log_write("repack.sh", "dd if=%s of=unpacked/%s.secure bs=512 count=1\n", filepath, cramfs_name);
		log_write("repack.sh", "dd if=unpacked/%s.tmp of=unpacked/%s.secure bs=512 skip=1 seek=1\n", cramfs_name, cramfs_name);
		log_write("repack.sh", "rm -f unpacked/%s.tmp\n", cramfs_name);
	}
	log_write("repack.sh", "aos-resize unpacked/%s.secure\n", cramfs_name);
	log_write("repack.sh", "mv unpacked/%s.secure %s\n", cramfs_name, filepath);
	log_write("repack.sh", "\n");
	
	return 1;
}

int parse_copy_block(struct aos_block *block)
{
	struct aos_block_copy *copy = (struct aos_block_copy  *)block->data;
	char *name;
	
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
	log_write("digest", "copy %u %s /%s\n", copy->partition, copy->name, name);
	
	// Handle .cramfs.secure files
	if(!strcasecmp(&copy->name[strlen(copy->name)-strlen(".cramfs.secure")], ".cramfs.secure")) {
		
		parse_cramfs_archive(copy->name, copy->data, copy->size, name);
	}
	
	free(name);
	
	return 1;
}

int parse_delete_block(struct aos_block *block)
{
	struct aos_block_delete *delete = (struct aos_block_delete  *)block->data;
	
	// Write the delete command to digest
	log_write("digest", "delete %u %s\n", delete->partition, delete->name);
	
	return 1;
}

int parse_other_block(struct aos_file *file, struct aos_block *block)
{
	char *name;
	
	// write the block to disk
	name = bprintf("raw/0x%08x_%c%c%c%c", (unsigned int)(((uint8_t *)&block->type)-file->data),
		*(uint8_t *)block, *((uint8_t *)block+1), *((uint8_t *)block+2), *((uint8_t *)block+3));
	mkdir_recursive(name);
	file_write(name, block->data, block->length-sizeof(struct aos_block));
	
	// log the block in the digest file
	log_write("digest", "raw /%s\n", name);
	free(name);
	
	return 1;
}

int parse_file(struct aos_file *file, const char *folder)
{
	struct aos_block *block;
	char cwd[2048];
	char *full_folder;
	
	getcwd(cwd, sizeof(cwd));
	full_folder = bprintf("%s/%s/", cwd, folder);
	
	// create the output folder and cd into it
	mkdir_recursive(full_folder);
	chdir(full_folder);
	free(full_folder);
	
	// possibly remove the digest & log file, if they exist
	log_clean("digest");
	log_clean("unpack.sh");
	log_clean("repack.sh");
	
	// Parse all blocks
	block = block_get(file, 5);
	while(block) {
		switch(block->type) {
			case AOS_TYPE_FLASH: {
				parse_flash_block(block);
				break;
			}
			case AOS_TYPE_MTD: {
				parse_mtd_block(block);
				break;
			}
			case AOS_TYPE_COPY: {
				parse_copy_block(block);
				break;
			}
			case AOS_TYPE_DELETE: {
				parse_delete_block(block);
				break;
			}
			default: {
				// All unknown blocks are dumped into the /raw subdirectory
				parse_other_block(file, block);
				break;
			}
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
	
	printf("AOS unpack utility, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc < 2) {
		printf("Usage: %s <input> <output>\n\n", argv[0]);
		printf("Expand the input file into its individual components.\n");
		printf("This utility writes the necessary information to disk\n");
		printf("so that the aos-repack utility can repack the .aos file later.\n");
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
	
	if(argc > 2)
		// Parse the file and extract everything
		parse_file(aos, argv[2]);
	else {
		char *folder = malloc(strlen(argv[1])-4+1);
		if(!folder)
			printf("error: Memory\n");
		else {
			strncpy(folder, argv[1], strlen(argv[1])-4);
			parse_file(aos, folder);
		}
	}
	
	printf("\n");
	printf("Done.\n");
	
	aos_free(aos);
	free(buffer);
	
	return 0;
}



