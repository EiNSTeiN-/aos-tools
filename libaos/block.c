/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdint.h>

#include "libaos.h"
#include "block.h"

struct aos_block *block_first(struct aos_file *file)
{
	struct aos_block *first;
	
	first = (struct aos_block *)&file->data[4];
	if(!block_is_valid(file, first))
		return NULL;
	
	return first;
}

struct aos_block *block_next(struct aos_file *file, struct aos_block *block)
{
	struct aos_block *next;
	
	next = (struct aos_block *)((uint8_t *)block+block->length);
	if(!block_is_valid(file, next))
		return NULL;
	
	return next;
}


int block_is_valid(struct aos_file *file, struct aos_block *block)
{
	
	if((uint8_t *)block < file->data)
		return 0;
	
	if(block->length < 8)
		return 0;
	
	if((uint8_t *)block+block->length > file->data+file->length)
		return 0;
	
	return 1;
}

struct aos_block *block_get(struct aos_file *file, int num)
{
	struct aos_block *block;
	int i;
	
	block = block_first(file);
	if(!block)
		return NULL;
	
	for(i=0;i<num;i++) {
		block = block_next(file, block);
		if(!block)
			return NULL;
	}
	
	return block;
}

unsigned int block_offset(struct aos_file *file, struct aos_block *block)
{
	
	return (uint8_t *)block - file->data;
}
