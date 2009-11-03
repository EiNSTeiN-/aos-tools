/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#ifndef __FLASHRW_H
#define __FLASHRW_H

#include <stdint.h>

#define FLASHRW_WRITE_REQUEST		4
#define FLASHRW_READ_REQUEST		5

struct flashrw_params {
	uint32_t address;
	uint32_t size;
	uint8_t *data;
};

struct flashrw_request {
	char size;
	struct flashrw_params ctx;
};

int FLASH_Init();
int FLASH_Read(int fd, struct flashrw_params *ctx);
int FLASH_Write(int fd, struct flashrw_params *ctx);
int FLASH_Exit(int fd);

#endif /* __FLASHRW_H */
