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
#include "aos.h"
#include "crypto.h"

struct aos_file *aos_create(uint8_t *data, unsigned int length)
{
	struct aos_file *file;
	
	file = malloc(sizeof(struct aos_file));
	if(file == NULL)
		return NULL;
	
	memset(file, 0, sizeof(struct aos_file));
	
	file->data = data;
	file->length = length;
	
	return file;
}

int aos_verify_magic(struct aos_file *file)
{
	
	return (*(uint32_t *)file->data == AOS_MAGIC);
}

int aos_detect_key(struct aos_file *file, uint8_t **keys, unsigned int n, int *device)
{
	int i;
	
	for(i=0;i<n;i++) {
		if(aos_verify_signature(file, keys[i])) {
			*device = i;
			return 1;
		}
	}
	
	return 0;
}

int aos_verify_signature(struct aos_file *file, const uint8_t *mpk_key)
{
	struct aos_signature sign;
	struct aos_block *block;
	unsigned int length;
	const uint8_t *data;
	
	block = block_get(file, AOS_SIGN_BLOCK_ID);
	if(block == NULL)
		return 0;
	
	if(block->length != sizeof(struct aos_block)+AOS_SIGNATURE_LENGTH)
		return 0;
	
	aos_signature_init(&sign);
	aos_signature_set_data(&sign, block->data);
	aos_signature_set_key(&sign, mpk_key);
	
	data = (uint8_t *)block + block->length;
	length = file->length - (data - file->data);
	
	return aos_signature_check(&sign, data, length);
}

int aos_signature_type(struct aos_file *file)
{
	struct aos_block *block;
	
	block = block_get(file, AOS_SIGN_BLOCK_ID);
	if(block == NULL)
		return 0;
	
	return block->type;
}

int aos_is_encrypted(struct aos_file *file)
{
	struct aos_block *block;
	
	block = block_get(file, AOS_CIPHER_BLOCK_ID);
	if(block == NULL)
		return 0;
	
	if(block->type != AOS_TYPE_CIPHER)
		return 0;
	
	if(*(uint32_t *)&block->data[0] != AOS_CIPHER_MAGIC)
		return 0;
	
	return 1;
}

int aos_decrypt_file(struct aos_file *file, const uint8_t *aes_key)
{
	struct aos_encryption enc;
	struct aos_block *block;
	unsigned int length;
	uint8_t *decrypted;
	uint8_t *data;
	
	block = block_get(file, AOS_CIPHER_BLOCK_ID);
	if(block == NULL)
		return 0;
	
	if(block->length != sizeof(struct aos_block)+sizeof(struct aos_block_cipher))
		return 0;
	
	aos_cipher_set_iv(&enc, (const uint8_t *)&block->data[4]);
	aos_cipher_set_decrypt_key(&enc, aes_key);
	
	data = (uint8_t *)block + block->length;
	length = file->length - (data - file->data);
	
	decrypted = aos_cipher_decrypt(&enc, data, length);
	if(!decrypted)
		return 0;
	
	memcpy(data, decrypted, length);
	free(decrypted);
	
	return 1;
}

int aos_encrypt_file(struct aos_file *file, const uint8_t *aes_key)
{
	struct aos_encryption enc;
	struct aos_block *block;
	struct aos_block_cipher *cipher;
	unsigned int length;
	uint8_t *encrypted;
	uint8_t *data;
	
	block = block_get(file, AOS_CIPHER_BLOCK_ID);
	if(block == NULL)
		return 0;
	
	if(block->length != sizeof(struct aos_block)+sizeof(struct aos_block_cipher))
		return 0;
	
	cipher = (struct aos_block_cipher *)block->data;
	
	cipher->magic = AOS_CIPHER_MAGIC;
	aos_cipher_set_iv(&enc, (const uint8_t *)&cipher->data);
	aos_cipher_set_encrypt_key(&enc, aes_key);
	
	data = (uint8_t *)block + block->length;
	length = file->length - (data - file->data);
	
	encrypted = aos_cipher_encrypt(&enc, data, length);
	if(!encrypted)
		return 0;
	
	memcpy(data, encrypted, length);
	free(encrypted);
	
	return 1;
}

void aos_free(struct aos_file *file)
{
	free(file);
	return;
}

int aos_is_signed(struct aos_file *file)
{
	struct aos_block *block;
	unsigned int i;
	
	block = block_get(file, AOS_SIGN_BLOCK_ID);
	if(!block)
		return 0;
	
	for(i=0;i<AOS_SIGNATURE_LENGTH;i++)
		if(block->data[i] != 0) return 1;
	
	return 0;
}
