/**
  ******************************************************************************
  * @file    mass_mal.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Medium Access Layer interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "mass_mal.h"
#include "FileData.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define FAT_TABLE_ADDR						0x8004000                /* 16K */
#define SECTOR_SIZE							0x200
#define FLASH_PAGE_SIZE						((uint32_t)0x00000800)   /* FLASH Page Size */

static const uint8_t FAT_DBR_TABLE[64] = {
	0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x57, 0x49, 0x4E, 0x34, 0x2E, 0x31, 0x00, 0x02, 0x01, 0x01, 0x00, 
	0x02, 0x20, 0x00, 0xE0, 0x01, 0xF0, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x74, 0x19, 0x02, 0x27,  'Y',  'U',  'N',  'E',  'E', 
	 'C',  ' ',  'U',  'S',  'B', 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00
};

#define FAT_TAB_SIZE (SECTOR_SIZE * 3)
#define FAT1_Offset 0x200
#define FAT2_Offset 0x800
static const uint8_t FAT_Tab[8] = {0x00, 0x00, 0x00, 0x03, 0xF0, 0xFF, 0x00, 0x00};

uint32_t Page[FLASH_PAGE_SIZE/4] = {0};

/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[1];
uint32_t Mass_Block_Size[1];
uint32_t Mass_Block_Count[1];
__IO uint32_t Status = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
	uint16_t Index = 0, EraseCounter;
	uint32_t Data = 0, Address = 0;

	if(lun == 0)
	{
		Mass_Block_Count[0] = 240 * 1024 / SECTOR_SIZE;
		Mass_Block_Size[0] = SECTOR_SIZE;
		Mass_Memory_Size[0] = 240 * 1024;
		Data = *(__IO uint16_t *)(FAT_TABLE_ADDR + 0x1FE);
		if(Data != 0xAA55)//No FAT
		{
			FLASH_Unlock();
			/* Clear pending flags (if any) */  
			FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
			
			for(EraseCounter = 0; EraseCounter < 3; EraseCounter ++)//6K = 3 Pages = 12 sectors
			{
				FLASH_ErasePage(FAT_TABLE_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
			}
			while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
/* boot sector */
			Address = FAT_TABLE_ADDR;
			for(Index = 0; Index < sizeof(FAT_DBR_TABLE); Index += 4)//write BPB Table
			{
				Data = FAT_DBR_TABLE[Index + 3]; Data <<= 8;
				Data |= FAT_DBR_TABLE[Index + 2]; Data <<= 8;
				Data |= FAT_DBR_TABLE[Index + 1]; Data <<= 8;
				Data |= FAT_DBR_TABLE[Index];

				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
			FLASH_ProgramHalfWord((FAT_TABLE_ADDR + 0x1FE), 0xAA55);//write endian
			while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			Data = 0; Address = FAT_TABLE_ADDR + sizeof(FAT_DBR_TABLE);
			for(Index = 0; Index < SECTOR_SIZE - sizeof(FAT_DBR_TABLE) - 2; Index += 2)// clear boot sector
			{
				FLASH_ProgramHalfWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
/* FAT1 & FAT2 sector */
			//FAT1
			Address = FAT_TABLE_ADDR + FAT1_Offset;
			for(Index = 0; Index < sizeof(FAT_Tab); Index += 4)
			{
				Data = FAT_Tab[Index + 3]; Data <<= 8;
				Data |= FAT_Tab[Index + 2]; Data <<= 8;
				Data |= FAT_Tab[Index + 1]; Data <<= 8;
				Data |= FAT_Tab[Index];

				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
			Data = 0; Address = FAT_TABLE_ADDR + FAT1_Offset + sizeof(FAT_Tab);
			for(Index = 0; Index < FAT_TAB_SIZE - sizeof(FAT_Tab); Index += 4)//flush sectors for FAT1             
			{
				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
            //FAT2
			Address = FAT_TABLE_ADDR + FAT2_Offset;
			for(Index = 0; Index < sizeof(FAT_Tab); Index += 4)
			{
				Data = FAT_Tab[Index + 3]; Data <<= 8;
				Data |= FAT_Tab[Index + 2]; Data <<= 8;
				Data |= FAT_Tab[Index + 1]; Data <<= 8;
				Data |= FAT_Tab[Index];

				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
			Data = 0; Address = FAT_TABLE_ADDR + FAT2_Offset + sizeof(FAT_Tab);
			for(Index = 0; Index < FAT_TAB_SIZE - sizeof(FAT_Tab); Index += 4)//flush sectors for FAT2     
			{
				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
/* root directory */
            Address = FAT_TABLE_ADDR + FileTab_Offset;
			for(Index = 0; Index < sizeof(FileTAB); Index += 4)//write file prop data
			{
				Data = FileTAB[Index + 3]; Data <<= 8;
				Data |= FileTAB[Index + 2]; Data <<= 8;
				Data |= FileTAB[Index + 1]; Data <<= 8;
				Data |= FileTAB[Index];

				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
			Data = 0; Address = FAT_TABLE_ADDR + FileTab_Offset + sizeof(FileTAB);
			for(Index = 0; Index < SECTOR_SIZE * 2 - sizeof(FileTAB); Index += 4)//flush 2 sectors for root directory   
			{
				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
/* file data area */
			Address = FAT_TABLE_ADDR + FileData_Offset;
			for(Index = 0; Index < sizeof(FileData); Index += 4)//write file data
			{
				Data = FileData[Index + 3]; Data <<= 8;
				Data |= FileData[Index + 2]; Data <<= 8;
				Data |= FileData[Index + 1]; Data <<= 8;
				Data |= FileData[Index];

				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
			Data = 0; Address = FAT_TABLE_ADDR + FileData_Offset + FileData_Size;
			for(Index = 0; Index < SECTOR_SIZE - FileData_Size % SECTOR_SIZE; Index += 4)//flush end of sector
			{
				FLASH_ProgramWord((Address + Index), Data);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
			}
/* -------- */
			FLASH_Lock();
		}
		return MAL_OK;
	}
	return MAL_FAIL;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
	uint16_t Index = 0;
	uint32_t Offset = Memory_Offset;
	uint32_t Address = 0;
	uint32_t Pages = ((Offset >> 11) << 11);
	if(lun == 0)
	{
		if(Memory_Offset > 0)
		{
			Address = FAT_TABLE_ADDR + Pages;
			for(Index = 0; Index < FLASH_PAGE_SIZE; Index += 4)//Copy all page Data from flash memory.
			{
				Page[Index >> 2] = *(__IO uint32_t *)(Address + Index);
			}
			FLASH_Unlock();
			while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==SET){}
			/* Clear pending flags (if any) */  
			FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
			FLASH_ErasePage(Address);
			while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==SET){}
			Address = Offset - Pages;
			Address >>= 2;
			for(Index = 0; Index < Transfer_Length; Index += 4)
			{
				Page[Address + (Index >> 2)] = Writebuff[Index >> 2];
			}
			Address = FAT_TABLE_ADDR + Pages;
			for(Index = 0; Index < FLASH_PAGE_SIZE; Index += 4)
			{
				FLASH_ProgramWord((Address + Index), Page[Index >> 2]);
				while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==SET){}
			}
			FLASH_Lock();
		}
			return MAL_OK;
	}
	return MAL_FAIL;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
	uint16_t Index = 0;
	if(lun == 0)
	{
		for(Index = 0; Index < Transfer_Length; Index += 4)
		{
			Readbuff[Index>>2] = *((uint32_t *)(FAT_TABLE_ADDR + Memory_Offset + Index));
		}
		return MAL_OK;
	}
	return MAL_FAIL;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{
	if (lun == 0)
	{
		return MAL_OK;
	}
	return MAL_FAIL;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
