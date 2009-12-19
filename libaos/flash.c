/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libaos.h"
#include "flash.h"

struct flash_file *flash_create(uint8_t *data, unsigned int length)
{
	struct flash_file *file;
	
	file = malloc(sizeof(struct flash_file));
	if(file == NULL)
		return NULL;
	
	memset(file, 0, sizeof(struct flash_file));
	
	file->header = (struct flash_header *)data;
	file->length = length;
	
	return file;
}

int flash_detect_key(struct flash_file *file, uint8_t **keys, unsigned int n, int *device)
{
	int i;
	
	for(i=0;i<n;i++) {
		if(flash_verify_signature(file, keys[i])) {
			*device = i;
			return 1;
		}
	}
	
	return 0;
}

int flash_verify_signature(struct flash_file *file, const uint8_t *mpk_key)
{
	struct aos_signature sign;
	unsigned int length;
	const uint8_t *data;
	
	if(file->header->filesize > file->length)
		return 0;
	
	aos_signature_init(&sign);
	aos_signature_set_data(&sign, file->header->signature);
	aos_signature_set_key(&sign, mpk_key);
	
	data = (const uint8_t *)&file->header->unk_88;
	length = file->header->filesize - (data - (uint8_t *)file->header);
	
	return aos_signature_check(&sign, data, length);
}

void flash_free(struct flash_file *file)
{
	free(file);
	return;
}

int flash_is_signed(struct flash_file *file)
{
	unsigned int i;
	
	for(i=0;i<AOS_SIGNATURE_LENGTH;i++)
		if(file->header->signature[i] != 0) return 1;
	
	return 0;
}

int flash_clear_signature(struct flash_file *file)
{
	memset(file->header->signature, 0, AOS_SIGNATURE_LENGTH);
	return 1;
}
