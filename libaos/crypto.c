/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdint.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#include "libaos.h"
#include "crypto.h"
#include "md5.h"

// As our implementation of openssl is big endian, we need to reverse strings from time to time.
// Shamelessly copied from archutil (thanks CheBuzz)
static void aos_bignum_reverse(const unsigned char *in, unsigned char *out, int length)
{
        int i;
	
        for(i=0;i<length;i++)
                out[i] = in[length-i-1];
	
	return;
}

int aos_signature_init(struct aos_signature *sign)
{
	memset(sign, 0, sizeof(struct aos_signature));
	
	sign->rsa = RSA_new();
	if(sign->rsa == NULL)
		return 0;

	RSA_set_method(sign->rsa, RSA_PKCS1_OpenSSL());
	
	//sign->rsa->e = BN_new();
	//if(sign->rsa->e == NULL)
	//	return 0;
	
	//BN_set_word(sign->rsa->e, 3);
	
	return 1;
}

int aos_signature_set_data(struct aos_signature *sign, const uint8_t *sig_data)
{
	char data[AOS_SIGNATURE_LENGTH];
	
	aos_bignum_reverse(sig_data, data, AOS_SIGNATURE_LENGTH);
	memcpy(sign->data, data, AOS_SIGNATURE_LENGTH);
	
	return 1;
}

int aos_signature_set_key(struct aos_signature *sign, const uint8_t *mpk_data)
{
	char data[AOS_SIGNATURE_LENGTH];
	BIGNUM *n = NULL, *e = NULL;
	
	aos_bignum_reverse(mpk_data, data, AOS_SIGNATURE_LENGTH);

	n = BN_bin2bn(data, AOS_SIGNATURE_LENGTH, NULL);
	e = BN_new();
	BN_set_word(e, 3);
	
	RSA_set0_key(sign->rsa, n, e, NULL);

	//sign->rsa->n = BN_bin2bn(data, AOS_SIGNATURE_LENGTH, NULL);
	//if(sign->rsa->n == NULL)
	//	return 0;
	
	return 1;
}

int aos_signature_check(struct aos_signature *sign, const uint8_t *data, unsigned int length)
{
	char decrypted[AOS_SIGNATURE_LENGTH];
	char reversed[AOS_SIGNATURE_LENGTH];
	char digest[16];
	struct cvs_MD5Context md5_ctx;
	
	cvs_MD5Init(&md5_ctx);
	cvs_MD5Update(&md5_ctx, data, length);
	cvs_MD5Final(digest, &md5_ctx);
	
	if(RSA_public_decrypt(AOS_SIGNATURE_LENGTH, sign->data, 
			decrypted,  sign->rsa, RSA_NO_PADDING) == -1)
		return 0;
	
	aos_bignum_reverse(decrypted, reversed, AOS_SIGNATURE_LENGTH);
	
	return !memcmp(digest, reversed, sizeof(digest));
}

int aos_cipher_set_iv(struct aos_encryption *enc, const uint8_t *iv)
{
	memcpy(enc->iv, iv, AES_BLOCK_SIZE);
	
	return 1;
}

int aos_cipher_set_decrypt_key(struct aos_encryption *enc, const uint8_t *aes_data)
{
	AES_set_decrypt_key(aes_data, AES_BLOCK_SIZE*8 /* size in bits */, &enc->key);
	return 1;
}

int aos_cipher_set_encrypt_key(struct aos_encryption *enc, const uint8_t *aes_data)
{
	AES_set_encrypt_key(aes_data, AES_BLOCK_SIZE*8 /* size in bits */, &enc->key);
	return 1;
}

uint8_t *aos_cipher_decrypt(struct aos_encryption *enc, const uint8_t *in, unsigned int length)
{
	uint8_t *decrypted;
	unsigned int done = 0;
	
	decrypted = (uint8_t *)malloc(length);
	if(!decrypted)
		return NULL;
	
	while(done < length) {
		uint8_t iv[AES_BLOCK_SIZE];
		uint8_t data[AES_BLOCK_SIZE];
		struct aos_block *block = (struct aos_block *)&data;
		
		memcpy(iv, enc->iv, AES_BLOCK_SIZE);
		
		AES_cbc_encrypt(&in[done], data, AES_BLOCK_SIZE, &enc->key, iv, AES_DECRYPT);
		memcpy(&decrypted[done], &data, AES_BLOCK_SIZE);
		done += AES_BLOCK_SIZE;
		
		AES_cbc_encrypt(&in[done], &decrypted[done], block->length-AES_BLOCK_SIZE, &enc->key, iv, AES_DECRYPT);
		done += block->length-AES_BLOCK_SIZE;
	}
	
	return decrypted;
}

uint8_t *aos_cipher_encrypt(struct aos_encryption *enc, const uint8_t *in, unsigned int length)
{
	uint8_t *encrypted;
	unsigned int done = 0;
	
	encrypted = (uint8_t *)malloc(length);
	if(!encrypted)
		return NULL;
	
	while(done < length) {
		uint8_t iv[AES_BLOCK_SIZE];
		uint8_t data[AES_BLOCK_SIZE];
		struct aos_block *block = (struct aos_block *)&in[done];
		
		memcpy(iv, enc->iv, AES_BLOCK_SIZE);
		
		/*AES_cbc_encrypt(&in[done], data, AES_BLOCK_SIZE, &enc->key, iv, AES_ENCRYPT);
		memcpy(&encrypted[done], &data, AES_BLOCK_SIZE);
		done += AES_BLOCK_SIZE;*/
		
		AES_cbc_encrypt(&in[done], &encrypted[done], block->length, &enc->key, iv, AES_ENCRYPT);
		done += block->length;
	}
	
	return encrypted;
}

