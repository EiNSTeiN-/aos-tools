/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <stdint.h>

#include "../libaos/libaos.h"
#include "keys.h"
#include "mpk.h"

unsigned char **mpk_possible_aos_keys(uint32_t sign_type)
{
	switch(sign_type) {
		case AOS_TYPE_SIG0:	return Rel_Keys;
		case AOS_TYPE_SIG1:	return Dev_Keys;
		case AOS_TYPE_SIG2:	return Plug_Keys;
		case AOS_TYPE_SIG3:	return HDD_Keys;
		case AOS_TYPE_SIG4:	return Games_Keys;
	}
	return NULL;
}

unsigned char **mpk_possible_bootloader_keys()
{
	return Bootloader_Keys;
}

unsigned char **mpk_possible_aes_keys()
{
	return AES_Keys;
}

const char *mpk_device_type(int device)
{
	switch(device) {
	case MPK_DEVICE_A5:	return "Archos 5";
	case MPK_DEVICE_A5IT:	return "Archos 5 Internet Tablet with Android";
	}
	
	return "Unknown";
}