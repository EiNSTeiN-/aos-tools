/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "insmod.h"
#include "rmmod.h"

#include "flashrw.h"

int FLASH_Init()
{
	if(insmod("/lib/modules/flashrw.ko", "") < 0)
		return -1;
	
	return open("/dev/flashrw", O_RDWR);
}

int FLASH_Read(int fd, struct flashrw_params *ctx)
{
	struct flashrw_request argp;
	
	if(ctx == NULL)
		return -1;
	
	argp.size = sizeof(struct flashrw_request);
	memcpy(&argp.ctx, ctx, sizeof(struct flashrw_params));
	
	if(ioctl(fd, FLASHRW_READ_REQUEST, &argp) < 0)
		return -1;
	
	return 0;
}

int FLASH_Write(int fd, struct flashrw_params *ctx)
{
	struct flashrw_request argp;
	
	if(ctx == NULL)
		return -1;
	
	argp.size = sizeof(struct flashrw_request);
	memcpy(&argp.ctx, ctx, sizeof(struct flashrw_params));
	
	if(ioctl(fd, FLASHRW_WRITE_REQUEST, &argp) < 0) {
		/*if(errno == 300)
			return -2;*/
		
		return -1;
	}
	
	return 0;
}

int FLASH_Exit(int fd)
{
	if(close(fd) != 0)
		return -1;
	
	if(rmmod("flashrw", O_NONBLOCK) != 0)
		return -1;
	
	return 0;
}



