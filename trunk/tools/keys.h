/*
 * Written by EiNSTeiN_
 * 		http://archos.g3nius.org/
 *
 * Released under the GNU General Public License v2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#ifndef __KEYS_H
#define __KEYS_H

static unsigned char A5_AES[] = {
	0xa3, 0x8a, 0x83, 0xaa, 0xef, 0x10, 0x48, 0x5e,
	0x98, 0x71, 0x11, 0x60, 0xc2, 0x49, 0x6f, 0x1e,
};

static unsigned char A5_BOOTLOADER[] = {
	0xaf, 0x4e, 0x44, 0xf3, 0x30, 0x0f, 0x74, 0xa3, 0x4d,
	0x1a, 0xd6, 0x84, 0xd7, 0x70, 0x27, 0x13, 0xb4, 0xb0,
	0x75, 0xf8, 0x42, 0xc2, 0xad, 0x5c, 0xbf, 0x21, 0x50,
	0xe8, 0xcc, 0x7a, 0x19, 0x93, 0x7b, 0xbc, 0xf8, 0x54,
	0x15, 0x60, 0xad, 0x33, 0xca, 0x9f, 0xed, 0xf0, 0xc6,
	0xba, 0x46, 0xe5, 0x71, 0x2e, 0xc7, 0x87, 0x8f, 0x58,
	0x3c, 0x3b, 0xf5, 0x38, 0x97, 0x77, 0x47, 0x49, 0x60,
	0x99, 0x11, 0x16, 0x02, 0x94, 0x90, 0x00, 0x6a, 0x98,
	0xc6, 0xe8, 0xcd, 0x66, 0x75, 0x5c, 0x1e, 0x4f, 0xc9,
	0x9b, 0x49, 0x94, 0x8f, 0x7a, 0x66, 0x6d, 0xca, 0x5a,
	0x0d, 0x6f, 0xb2, 0x01, 0xb2, 0x90, 0x36, 0xbf, 0x08,
	0x9f, 0x03, 0x4f, 0x38, 0x54, 0x82, 0x4d, 0xe3, 0x32,
	0xb4, 0xc5, 0xb7, 0x30, 0x6a, 0xf5, 0x4f, 0xb7, 0xde,
	0xad, 0x90, 0x61, 0x1a, 0xb6, 0xd5, 0x43, 0xc6, 0x24,
	0xb0, 0xad,
};

static unsigned char A5_RELMPK[] = {
	0x3d, 0x85, 0x84, 0x68, 0x90, 0x55, 0xe9, 0xc8, 0x6c,
	0x1d, 0x43, 0x2f, 0xe0, 0x72, 0x01, 0x82, 0x51, 0xb4,
	0x12, 0x3d, 0x12, 0x67, 0x64, 0xe2, 0xac, 0xe0, 0x7d,
	0x3e, 0x98, 0xba, 0x6d, 0xbb, 0x0f, 0x38, 0x25, 0x68,
	0xe4, 0xef, 0x81, 0x6b, 0xb5, 0x01, 0x59, 0x61, 0x4c,
	0xfc, 0xf9, 0x20, 0xe5, 0x77, 0x35, 0xb0, 0x3f, 0x89,
	0x81, 0x11, 0xab, 0x99, 0xb7, 0x05, 0x03, 0x1f, 0x48,
	0x28, 0xb6, 0x5f, 0x12, 0xcd, 0xc6, 0x3b, 0xe4, 0xc1,
	0x76, 0x40, 0x33, 0x82, 0xd3, 0x6e, 0x6f, 0xd9, 0xae,
	0xbc, 0xac, 0xc1, 0x05, 0xaa, 0x39, 0x51, 0xdb, 0x1e,
	0x5d, 0x3a, 0xf3, 0xc1, 0x64, 0xf2, 0xf7, 0xbe, 0x80,
	0x7b, 0xea, 0xd0, 0xd1, 0x17, 0xf8, 0x70, 0x2e, 0x5e,
	0xd5, 0xe8, 0x53, 0x41, 0x2c, 0x6b, 0xa3, 0x0d, 0x63,
	0x2a, 0x61, 0xc3, 0x7c, 0xe2, 0x54, 0x2c, 0x6c, 0x52,
	0x2d, 0xbd
};

static unsigned char A5_DEVMPK[] = {
	0x85, 0x24, 0x81, 0x72, 0xe0, 0x6f, 0x36, 0x6a, 0x1e,
	0xa5, 0x5a, 0x92, 0x81, 0x4d, 0x9a, 0xdf, 0x3b, 0x22,
	0xce, 0x44, 0xd5, 0x39, 0xd4, 0x9f, 0x7f, 0x0e, 0x86,
	0xba, 0x02, 0xe5, 0xb7, 0xd3, 0x38, 0x40, 0x6a, 0x91,
	0x8c, 0x88, 0xe1, 0x90, 0xeb, 0x72, 0xa9, 0x1d, 0x90,
	0x67, 0x60, 0xbf, 0x50, 0x72, 0xf6, 0x02, 0x48, 0x80,
	0x01, 0x87, 0x1e, 0x93, 0x61, 0x52, 0x02, 0xd8, 0x9e,
	0xe7, 0x12, 0x46, 0x72, 0x89, 0xa1, 0xe8, 0x10, 0x2e,
	0x76, 0xc4, 0xf2, 0xdc, 0xf6, 0x2e, 0x6d, 0x8c, 0x46,
	0x3a, 0x77, 0xa7, 0xb4, 0x5c, 0x00, 0x35, 0x2a, 0xac,
	0xd6, 0xb3, 0xbb, 0x2a, 0x4c, 0x12, 0x95, 0x29, 0xc2,
	0x27, 0x4a, 0xa6, 0xfa, 0xf7, 0x8e, 0x53, 0x0a, 0xc8,
	0xbf, 0x95, 0x51, 0x1f, 0xbb, 0x9d, 0x27, 0x21, 0x05,
	0x8f, 0x82, 0xbc, 0x1b, 0x0c, 0x71, 0xb7, 0x5f, 0x79,
	0x8c, 0xa8
};

static unsigned char A5_PLUGMPK[] = {
	0x69, 0x6b, 0xea, 0x98, 0xb4, 0xc3, 0xb8, 0x98, 0x81,
	0x32, 0x2b, 0x87, 0x68, 0x9d, 0xa1, 0x1b, 0x48, 0xf3,
	0xa5, 0xa0, 0xd4, 0xbb, 0x85, 0x52, 0x49, 0xd7, 0x64,
	0xb0, 0xbb, 0x41, 0x20, 0xce, 0xa0, 0x21, 0x85, 0xeb,
	0x73, 0x05, 0x83, 0xf1, 0x0a, 0x48, 0xb1, 0x06, 0x2d,
	0x2b, 0xab, 0x66, 0x49, 0x36, 0x68, 0xe6, 0xe2, 0xab,
	0xaf, 0x60, 0xa7, 0x98, 0x56, 0xab, 0x63, 0x46, 0x5b,
	0x67, 0x54, 0xa8, 0xe1, 0xb7, 0x97, 0x87, 0xf5, 0x28,
	0xa9, 0x77, 0x62, 0x21, 0xfb, 0x00, 0xc0, 0x6f, 0xf5,
	0x61, 0xec, 0x13, 0x93, 0x8c, 0x3c, 0x4b, 0xcb, 0x4f,
	0x4b, 0x04, 0xd2, 0xe8, 0xfb, 0x53, 0x24, 0xe5, 0x91,
	0x43, 0x9d, 0xa9, 0xa1, 0x2c, 0x04, 0xba, 0x36, 0x27,
	0xcb, 0x31, 0x0a, 0x2a, 0x8f, 0x3d, 0xd4, 0x26, 0x11,
	0xd5, 0x8d, 0x45, 0x34, 0x60, 0xb4, 0xdc, 0x13, 0x3c,
	0x0b, 0xac
};

static unsigned char A5_HDDMPK[] = {
	0xb5, 0x4a, 0x0e, 0xe6, 0x42, 0x6a, 0x68, 0x41, 0x17,
	0x53, 0x70, 0xd2, 0x02, 0x86, 0xfb, 0xb3, 0x36, 0xdc,
	0xaf, 0x5a, 0xa3, 0x37, 0x0c, 0x57, 0xb9, 0x61, 0x70,
	0x98, 0xe7, 0x6a, 0x2c, 0x10, 0xb0, 0x2b, 0x66, 0x97,
	0x45, 0x3e, 0x0c, 0xc5, 0x38, 0x5d, 0x22, 0x7e, 0xe2,
	0xa8, 0xe2, 0x81, 0xea, 0xfc, 0x40, 0x38, 0xc9, 0x2d,
	0x7d, 0xae, 0x78, 0xc1, 0x6b, 0x12, 0x5b, 0xfd, 0x66,
	0x77, 0x55, 0x3e, 0x45, 0xf3, 0x51, 0xfb, 0x41, 0xd7,
	0xd0, 0xa0, 0x42, 0x24, 0x6c, 0x7a, 0xe4, 0x91, 0x55,
	0x0e, 0x51, 0x14, 0x2b, 0x8a, 0x87, 0xc5, 0xe7, 0x57,
	0xf3, 0x0c, 0x5c, 0x8d, 0x21, 0xf0, 0xce, 0x4e, 0x98,
	0xd5, 0x3e, 0x1e, 0x17, 0x69, 0x45, 0x5f, 0x48, 0xee,
	0xa3, 0x92, 0x39, 0x0f, 0xbd, 0x80, 0x71, 0xe9, 0xad,
	0x15, 0x15, 0x74, 0xe9, 0xcd, 0xc3, 0x5c, 0x1a, 0x68,
	0x99, 0xc6
};


static unsigned char A5_GAMESMPK[] = {
	0x40, 0x00, 0x61, 0xED, 0xB2, 0xCB, 0xD6, 0x90, 0x3F, 0xC6,
	0xE9, 0x52, 0x8F, 0x88, 0xD9, 0x61, 0xC4, 0x8C, 0x69, 0x61,
	0x35, 0x5C, 0x28, 0xC1, 0xC6, 0xDA, 0x51, 0x78, 0x65, 0x43,
	0x4C, 0x68, 0x6D, 0xAB, 0x57, 0xF1, 0xC7, 0xC7, 0xC5, 0x48,
	0xC9, 0x42, 0xCC, 0x9C, 0x47, 0x54, 0x74, 0x35, 0x20, 0xD1,
	0xE3, 0xB5, 0xA6, 0x58, 0x97, 0x52, 0x6F, 0x13, 0x34, 0xEA,
	0xAB, 0xC0, 0x6C, 0xE4, 0x29, 0xC9, 0x2B, 0x7E, 0x81, 0x57,
	0x30, 0x94, 0xC2, 0x4C, 0xC3, 0x23, 0xCA, 0xC7, 0x85, 0xE6,
	0xF3, 0xA0, 0x50, 0x7C, 0x00, 0xC5, 0x0B, 0xEE, 0xD4, 0x98,
	0x6D, 0x4E, 0x2B, 0x52, 0x06, 0xBF, 0xB0, 0xFE, 0x8D, 0xA5,
	0x87, 0xCF, 0x19, 0x19, 0xF1, 0xA7, 0x3E, 0xF4, 0x21, 0x27,
	0xA6, 0xE8, 0x28, 0xA3, 0x19, 0x10, 0x1B, 0x89, 0xE7, 0x33,
	0x7A, 0xA3, 0x17, 0xF1, 0x72, 0x7C, 0x1D, 0x51, 0xCA, 0xED,
	0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};


static unsigned char A5IT_AES[] = {
	0x68, 0xE0, 0xDF, 0x3C, 0xA0, 0x17, 0xE9, 0xA3, 0xA1, 0xF6, // AES key
	0xA0, 0x71, 0x7E, 0x3E, 0xFA, 0xDD
};

static unsigned char A5IT_BOOTLOADER[] = {
	0x57, 0xA7, 0x56, 0x1D, 0x9D, 0x06, 0xAC, 0x51, 0x66, 0xB3, // bootloader
	0x85, 0xC2, 0x1D, 0xED, 0x7C, 0x6A, 0x2F, 0x26, 0x4C, 0xF8,
	0xDA, 0x08, 0xC8, 0x25, 0x5B, 0x4A, 0x5F, 0x1F, 0x0D, 0x45,
	0x57, 0xDA, 0x31, 0x84, 0x55, 0x60, 0x94, 0x6E, 0x5D, 0x77,
	0x7F, 0xF8, 0xF8, 0x92, 0x6B, 0x07, 0xA5, 0x6B, 0x6C, 0x52,
	0x54, 0x8C, 0xB0, 0xE3, 0x47, 0x21, 0xDE, 0xD4, 0xF5, 0x8D,
	0xA8, 0x7D, 0x9A, 0x19, 0xA0, 0xAF, 0xDF, 0xC8, 0xF7, 0x85,
	0x25, 0xCC, 0x3C, 0x18, 0xA6, 0xA1, 0x58, 0xE3, 0xD5, 0x2D,
	0x23, 0xFA, 0xF5, 0x26, 0xA5, 0x2D, 0xF2, 0x03, 0x1D, 0xC8,
	0xC5, 0xF8, 0x14, 0x5E, 0x85, 0xA9, 0x2D, 0x80, 0xD8, 0x78,
	0x5A, 0xF2, 0x12, 0x50, 0xFC, 0xFF, 0x9F, 0x3A, 0x73, 0xFC,
	0xE4, 0xEF, 0x01, 0x9C, 0x10, 0x33, 0x8B, 0xFE, 0xEA, 0x5B,
	0x8C, 0x01, 0xCE, 0xC1, 0xC7, 0x30, 0x56, 0xB3,
	
	0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x23, 0x1A, 0xC6, 0x5C,
};

static unsigned char A5IT_RELMPK[] = {
	0x8B, 0xCB, 0x7F, 0x0C, 0x38, 0x29, 0x7A, 0x63, 0x22, 0x3F, // RelMPK
	0xB2, 0x61, 0x42, 0xBB, 0x05, 0x86, 0x72, 0x2D, 0xBE, 0x73,
	0x50, 0x55, 0xFD, 0x64, 0x1D, 0xCF, 0x67, 0x3B, 0x35, 0xDE,
	0xBC, 0xC9, 0x17, 0x2F, 0x63, 0xFA, 0x2A, 0x3E, 0xA2, 0x06,
	0x12, 0x45, 0x73, 0x89, 0x39, 0xF0, 0x08, 0x60, 0x6A, 0x29,
	0x6A, 0xB2, 0x7C, 0xFA, 0xAB, 0xC3, 0x9D, 0x82, 0xBD, 0x8B,
	0x64, 0xBF, 0x5C, 0x9B, 0xF3, 0xEE, 0xB5, 0xED, 0x8C, 0xF4,
	0x57, 0x7C, 0x95, 0xD1, 0x88, 0x40, 0xD4, 0x90, 0x1C, 0x70,
	0x0F, 0x91, 0x0D, 0x2B, 0xE9, 0x6E, 0xEF, 0x7C, 0x46, 0xE9,
	0xED, 0xBB, 0x5A, 0x61, 0x82, 0x84, 0xFA, 0x41, 0x4C, 0xA9,
	0x03, 0xEB, 0x96, 0xEC, 0x4E, 0x59, 0x69, 0x3B, 0xCF, 0xD4,
	0x73, 0xF3, 0xEF, 0x17, 0x2D, 0xDC, 0x55, 0x5A, 0x35, 0x2B,
	0xD6, 0x1F, 0xEF, 0xC1, 0x6E, 0xFB, 0xD5, 0xE3,
	
	0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xF8, 0xB7, 0x37, 0x7D,
};

static unsigned char A5IT_DEVMPK[] = {
	0x91, 0x6A, 0xE2, 0xD1, 0xD7, 0x51, 0x85, 0xBB, 0xE, 0xC, 0x24, // DevMPK
	0x99, 0xD, 0x46, 0xBF, 0x43, 0x2F, 0xAB, 0xA1, 0x89, 0xC2, 0x89,
	0x88, 0x67, 0x2C, 0x5B, 0x63, 0x3B, 0x6F, 0x82, 0x40, 0xBA,
	0xE7, 0x66,7, 0x21, 0xDF, 0x88, 0x7B, 0xFA, 0xBC, 0x36, 0x7C,
	0x2F, 0x51, 0x37, 0x8C, 0x40, 0xA2, 0x80, 0xC2, 0xD0, 0xF6,
	0x79, 0xDF, 0x99, 0xAE, 0x30, 0xB7, 0xDD,5, 0xAD, 0x14, 0x9A,
	0x80, 0x84, 0x46, 0xA, 0xA9, 0xD9, 0x54, 0x67, 0xC0, 0x64, 0x23,
	0x19, 0x52, 0x7E, 0x89, 0xC6, 0x6B, 0xA2, 0x3E, 0x65, 0x96,
	0x7B, 0x96, 0x61, 0x7C, 0x2D, 0xF2, 0xE8, 0xAA, 0x95, 0x1E,
	0x6B, 0x63, 0x9D, 0x78, 0xF7, 0xFF, 0x4F, 0x11, 0xAD, 0x35,
	0xA6, 0x20, 0xFE, 0x83, 0x6C, 0x4B, 0xCA, 0xD4, 0x43, 0x52,
	0x4A, 0x7B, 0xEC, 0xB6, 0xBD, 0xEC, 0xC2, 0x9F, 0x15, 0x7E,
	0x75, 0x40, 0xE7,1,0,3,0,0,0,0,0,0,0, 0x3F, 0x7A, 0x86, 0xA5,
};

static unsigned char A5IT_PLUGMPK[] = {
	0x9B, 0x63, 0xBC, 0xC6, 0x77, 0x36, 0xB5, 0xEA, 0xF0, 0xF, 0x78, // PlugMPK
	0x45, 0xD0,6, 0x83, 0xB1, 0x12, 0xD6, 0xB1, 0xA2, 0x8C, 0x3E,
	0xAA, 0xCF, 0x55, 0xDB, 0x11,5, 0x60, 0x12, 0x99, 0xCC, 0x92,
	9, 0xFA, 0xE3,5,2, 0x33, 0x7F, 0x6F, 0xD9, 0x5D, 0x8E, 0x32,
	0xB2, 0xE3, 0x9F, 0xD6, 0x86, 0xCB, 0xFB, 0x9D, 0x2A, 0x21,
	0x61, 0x2B, 0x98, 0x1D, 0xF1, 0x53, 0xB6, 0xD6, 0xC, 0xE0, 0x3E,
	0x57, 0x99, 0x52, 0x87, 0xFD, 0x99, 0x4B, 0x9E, 0x2D, 0xC2,
	0x66,5, 0xB1, 0xE7, 0xEE, 0x9C, 0x59, 0x1A, 0x79, 0xF3, 0x28,
	0x17, 0xF6, 0xCE, 0x80, 0x96, 0x12, 0xA2, 0xA7, 0xFB, 0xF0,
	0xF7, 0xB6, 0x36, 0xD0, 0x82, 0xC6, 0x32, 0x80, 0x62, 0x2D,
	0x5A, 0x46, 0x6D, 0x51, 0xB4, 0xE2, 0x8B, 0xA9, 0xEB, 0xFF,
	0xF8, 0x95, 0xDC, 0xD0, 0xBF, 0xCE, 0xE5, 0xFA, 0x99, 0x97,
	0xED,1,0,3,0,0,0,0,0,0,0, 0xC0, 0x44, 0xD6, 0xD0,
};

static unsigned char A5IT_HDDMPK[] = {
	0x4B, 0xE6, 0x2B, 0x67, 0x6E, 0xFE, 0xAA, 0xBA, 0xBE, 0xD1, // HDDMPK
	0xDD, 0xEA, 0xF4, 0x3E, 0x24, 0x4B, 0xD5, 0xBC, 0xED, 0xC5,
	0xB3, 0xFE, 0x8D, 0x8D, 0x76, 0x6A, 0xDB, 0x19, 0x15, 0x5D,
	0xCB, 0xE2, 0x70,8, 0x7E, 0xAA, 0x68, 0xE5, 0x56, 0x14, 0x65,
	0xF8, 0xB1, 0xBC, 0xEC, 0xB2, 0x38, 0xD1, 0xEB, 0x93,2, 0x43,
	0x83, 0x56, 0xE0, 0xF9, 0x90, 0xB2, 0x78, 0x6E, 0xAA, 0xB8,
	0x8A, 0xBD, 0x1E, 0x81, 0x48, 0x2A, 0x90, 0xE6, 0xCB, 0xBA,
	0x6C, 0x71, 0x7B, 0x9A, 0x56, 0x56, 0xA6, 0x6D, 0x5F, 0x52,
	0x37, 0x1E, 0x4D, 0x28, 0x15, 0xEC, 0xD4, 0xB5, 0x28, 0x15,
	0xC0,7, 0xC6, 0x85, 0x62, 0xE5, 0x3D, 0xDD, 0xE8, 0xD2, 0x8A,
	0x28, 0xE9, 0x3F, 0xFF, 0x5B, 0x28, 0x44, 0x34, 0xE7, 0x83,
	0x38, 0xC8, 0xF2, 0xE4, 0x20, 0x90, 0xE8, 0xCE, 0xA0, 0xDF,
	0x8B, 0x2A, 0x56, 0xC4, 0xA6,1,0,3,0,0,0,0,0,0,0, 0x29,
	0x63, 0x8A, 0xF8,
};

static char A5IT_GAMESMPK[] = {
	0x61, 0xED, 0xB2, 0xCB, 0xD6, 0x90, 0x3F, 0xC6, 0xE9, 0x52,
	0x8F, 0x88, 0xD9, 0x61, 0xC4, 0x8C, 0x69, 0x61, 0x35, 0x5C,
	0x28, 0xC1, 0xC6, 0xDA, 0x51, 0x78, 0x65, 0x43, 0x4C, 0x68,
	0x6D, 0xAB, 0x57, 0xF1, 0xC7, 0xC7, 0xC5, 0x48, 0xC9, 0x42,
	0xCC, 0x9C, 0x47, 0x54, 0x74, 0x35, 0x20, 0xD1, 0xE3, 0xB5,
	0xA6, 0x58, 0x97, 0x52, 0x6F, 0x13, 0x34, 0xEA, 0xAB, 0xC0,
	0x6C, 0xE4, 0x29, 0xC9, 0x2B, 0x7E, 0x81, 0x57, 0x30, 0x94,
	0xC2, 0x4C, 0xC3, 0x23, 0xCA, 0xC7, 0x85, 0xE6, 0xF3, 0xA0,
	0x50, 0x7C,0, 0xC5, 0xB, 0xEE, 0xD4, 0x98, 0x6D, 0x4E, 0x2B,
	0x52,6, 0xBF, 0xB0, 0xFE, 0x8D, 0xA5, 0x87, 0xCF, 0x19, 0x19,
	0xF1, 0xA7, 0x3E, 0xF4, 0x21, 0x27, 0xA6, 0xE8, 0x28, 0xA3,
	0x19, 0x10, 0x1B, 0x89, 0xE7, 0x33, 0x7A, 0xA3, 0x17, 0xF1,
	0x72, 0x7C, 0x1D, 0x51, 0xCA, 0xED,1,0,3,0,0,0,0,0,0,0,
	0,0,0,0
};

#endif /* __KEYS_H */
