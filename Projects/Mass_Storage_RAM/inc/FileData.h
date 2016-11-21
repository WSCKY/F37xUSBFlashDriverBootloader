#include "stm32f37x.h"

#define FAT_TABLE_ADDR						0x8003800                /* 14K */
#define SECTOR_SIZE							0x200
#define FLASH_PAGE_SIZE						((uint32_t)0x00000800)   /* FLASH Page Size */

#define FAT_USED_SIZE (SECTOR_SIZE * 9)

/* BOOT */
#define BOOT_TABLE_SIZE SECTOR_SIZE
#define BOOT_TABLE_OFFSET 0
#define BOOT_TABLE_USED_SIZE 62
static uint8_t BOOT_TABLE[62] = {
0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x57, 0x49, 0x4E, 0x34, 0x2E, 0x31, 0x00, 0x02, 0x01, 0x01, 0x00, 
0x02, 0x20, 0x00, 0xED, 0x01, 0xF0, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x74, 0x19, 0x02, 0x27,  'Y',  'U',  'N',  'E',  'E', 
 'C',  ' ',  'U',  'S',  'B', 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20
};

/* FAT */
#define FAT_TABLE_SIZE (SECTOR_SIZE * 3)
#define FAT1_TABLE_OFFSET 0x200
#define FAT2_TABLE_OFFSET 0x800
static uint8_t FATn_TABLE[FAT_TABLE_SIZE] = {0x00, 0x00, 0x00, 0x03, 0x40, 0x00, 0xFF, 0x0F};

/* ROOT */
#define ROOT_TABLE_SIZE (SECTOR_SIZE * 2)
#define ROOT_TABLE_OFFSET 0xE00
static uint8_t ROOT_TABLE[ROOT_TABLE_SIZE] = {
 'Y',  'U',  'N',  'E',  'E',  'C',  ' ',  'D',  'E',  'V',  ' ', 0x08, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAE, 0xA6, 0x9B, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xE5, 0xB0, 0x65, 0xFA, 0x5E, 0x87, 0x65, 0x2C, 0x67, 0x87, 0x65, 0x0F, 0x00, 0xD2, 0x63, 0x68, 
0x2E, 0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
0xE5, 0xC2, 0xBD, 0xA8, 0xCE, 0xC4, 0x7E, 0x31, 0x54, 0x58, 0x54, 0x20, 0x00, 0x00, 0xB0, 0xA6, 
0x9B, 0x47, 0x9B, 0x47, 0x00, 0x00, 0xB1, 0xA6, 0x9B, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 'R',  'E',  'A',  'D',  'M',  'E',  ' ',  ' ',  'T',  'X',  'T',  ' ', 0x18, 0x00, 0xB0, 0xA6, 
0x9B, 0x47, 0x9B, 0x47, 0x00, 0x00, 0xB5, 0xA6, 0x9B, 0x47, 0x02, 0x00, 0x14, 0x05, 0x00, 0x00
};

#define FileData_Offset 0
#define FileData_Size   1300
//const
static unsigned char FileData[1300] =
"This is the first of a pair of papers on the theory and implementation of a\r\n"
"direction-cosine-matrix (DCM) based inertial measurement unit for application\r\n"
"in model planes and helicopters. Actually, at this point, it is still a draft, there\r\n"
"is still a lot more work to be done. Several reviewers, especially Louis\r\n"
"LeGrand and UFO-man, have made good suggestions on additions and\r\n"
"revisions that we should make and prepared some figures that we have not\r\n"
"included yet. We will eventually incorporate their suggestions, but it may take\r\n"
"a long time to get there. In the meantime, we think there is an audience who\r\n"
"can benefit from what we have so far.\r\n"
"The motivation for DCM was to take the next step in stabilization and\r\n"
"control functions from an inherently stable aircraft with elevator and rudder\r\n"
"control, to an aerobatic aircraft with ailerons and elevator. One of the authors\r\n"
"(Premerlani) built a two axes board several years ago, and developed\r\n"
"rudimentary firmware to provide stabilization and return-to-launch (RTL)\r\n"
"functions for a Gentle Lady sailplane. The firmware worked well enough, and\r\n"
"the author came to rely on the RTL feature, but it never seemed to work as\r\n"
"well as the author would like. In particular, satisfactory solutions to the\r\n"
"following two issues were never found:  ";
