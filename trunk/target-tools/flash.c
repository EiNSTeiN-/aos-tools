/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "flashrw.h"

int flash(const char *filename, const char *address)
{
	struct flashrw_params p;
	int fd;
	uint8_t *data;
	uint32_t _address;
	unsigned int filesize;
	
	if(sscanf(address, "0x%08x", &_address) != 1)
	{
		printf("error: invalid address specified: %s\n", address);
		return 0;
	}
	
	printf("Flashing at address 0x%08x\n", _address);
	
	data = file_load(filename, &filesize);
	if(!data) {
		printf("error: file read failed: %s\n", filename);
		return 0;
	}
	
	printf("Writing %u bytes from file %s\n", filesize, filename);
	
	fd = FLASH_Init();
	if(fd < 0) {
		printf("error: kernel module load failed.\n");
		free(data);
		return 0;
	}
	
	p.address = _address;
	p.size = filesize;
	p.data = data;
	
	if(FLASH_Write(fd, &p) != 0) {
		printf("error: flash write failed.\n");
	}
	
	if(FLASH_Exit(fd) != 0) {
		printf("error: kernel module unload failed.\n");
		return 0;
	}
	
	free(data);
	
	return 1;
}
	
int dump(const char *filename, const char *address, const char *size)
{
	struct flashrw_params p;
	int fd;
	char *data;
	uint32_t _address, _size;
	
	if(sscanf(address, "0x%08x", &_address) != 1)
	{
		printf("error: invalid address specified: %s\n", address);
		return 0;
	}
	
	if(sscanf(size, "%u", &_size) != 1)
	{
		printf("error: invalid size specified: %s\n", size);
		return 0;
	}
	
	printf("Dumping %u bytes from address 0x%08x\n", _size, _address);
	
	data = malloc(_size);
	if(data == NULL) {
		printf("error: memory: %u\n", _size);
		return 0;
	}
	
	fd = FLASH_Init();
	if(fd < 0) {
		printf("error: kernel module load failed.\n");
		return 0;
	}
	
	p.address = _address;
	p.size = _size;
	p.data = data;
	
	if(FLASH_Read(fd, &p) != 0) {
		printf("error: flash read failed.\n");
		free(data);
		return 0;
	}
	
	printf("Writing %u bytes to %s\n", _size, filename);
	
	if(file_write(filename, data, _size) != 1) {
		printf("error: could not write data to %s.\n", filename);
		free(data);
		return 0;
	}
	
	if(FLASH_Exit(fd) != 0) {
		printf("error: kernel module unload failed.\n");
		free(data);
		return 0;
	}
	
	free(data);
	
	return 1;
}
	
int main(int argc, char *argv[])
{	
	printf("FLASH tool, written by EiNSTeiN_\n");
	printf("\thttp://archos.g3nius.org/\n\n");
	
	if(argc < 4) {
		printf("Usage: %s [-f|-d] <filename> <address> [size]\n\n", argv[0]);
		printf("Commands:\n");
		printf("\t-f (flash): writes the content of <filename> to <address>\n");
		printf("\t-d (dump): dumps [size] bytes at <address> into <filename>\n");
		printf("\n");
		printf("For example:\n");
		printf("\t%s -d output.bin 0x060000 1536000\n", argv[0]);
		printf("\t%s -f input.bin 0x060000\n", argv[0]);
		return 1;
	}
	
	switch(argv[1][1]) {
		case 'f': {
			flash(argv[2], argv[3]);
			break;
		}
		case 'd': {
			dump(argv[2], argv[3], argc > 4 ? argv[4] : NULL);
			break;
		}
		default: return 1;
	}
	
	
	printf("\n");
	printf("Done.\n");
	
	return 0;
}
