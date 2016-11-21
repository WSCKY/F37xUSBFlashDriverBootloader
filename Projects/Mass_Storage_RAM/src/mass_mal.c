/**
  ******************************************************************************
  * @file    mass_mal.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Medium Access Layer interface
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "mass_mal.h"
#include "FileData.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[1];
uint32_t Mass_Block_Size[1];
uint32_t Mass_Block_Count[1];

uint32_t Page[FLASH_PAGE_SIZE/4] = {0};

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
		Mass_Block_Count[0] = 493;//242K + 9sectors;
		Mass_Block_Size[0] = SECTOR_SIZE;
		Mass_Memory_Size[0] = 252416;//242*1024+9*512;

/* write file data to flash memory */
		FLASH_Unlock();
		/* Clear pending flags (if any) */  
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
		
		for(EraseCounter = 0; EraseCounter < 1; EraseCounter ++)//2K = 1 Pages = 4 sectors for readme.txt
		{
			FLASH_ErasePage(FAT_TABLE_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
		}
		while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == SET){}
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
		FLASH_Lock();

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
	uint32_t Address = 0;
	uint32_t Pages = 0;
	uint8_t *pBuf = (uint8_t *)Writebuff;
	if(lun == 0)
	{
		if(Memory_Offset > 0)//½ûÖ¹¸ñÊ½»¯
		{
			if(Memory_Offset < FAT_USED_SIZE)
			{
				Address = Memory_Offset;
				for(Index = 0; Index < Transfer_Length; Index ++)
				{
					if(Address < BOOT_TABLE_USED_SIZE + BOOT_TABLE_OFFSET)
						BOOT_TABLE[Address] = pBuf[Index];
					else if(Address < BOOT_TABLE_SIZE + BOOT_TABLE_OFFSET)
						{}
					else if(Address < FAT_TABLE_SIZE + FAT1_TABLE_OFFSET)
						FATn_TABLE[Address - FAT1_TABLE_OFFSET] = pBuf[Index];
					else if(Address < FAT_TABLE_SIZE + FAT2_TABLE_OFFSET)
						FATn_TABLE[Address - FAT2_TABLE_OFFSET] = pBuf[Index];
					else if(Address < ROOT_TABLE_SIZE + ROOT_TABLE_OFFSET)
						ROOT_TABLE[Address - ROOT_TABLE_OFFSET] = pBuf[Index];
					Address ++;
				}
			}
			else
			{
				//compute address.
				Memory_Offset -= FAT_USED_SIZE;
				Pages = ((Memory_Offset >> 11) << 11);

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
				Address = Memory_Offset - Pages;
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
		else return MAL_FAIL;
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
	uint16_t Address = 0;
	uint8_t *pBuf = (uint8_t *)Readbuff;
	if(lun == 0)
	{
		if(Memory_Offset < FAT_USED_SIZE)
		{
			Address = Memory_Offset;
			for(Index = 0; Index < Transfer_Length; Index ++)
			{
				if(Address < BOOT_TABLE_USED_SIZE + BOOT_TABLE_OFFSET)
					pBuf[Index] = BOOT_TABLE[Address];
				else if(Address < BOOT_TABLE_SIZE + BOOT_TABLE_OFFSET - 2)
					pBuf[Index] = 0;
				else if(Address == BOOT_TABLE_SIZE + BOOT_TABLE_OFFSET - 2)
					pBuf[Index] = 0x55;
				else if(Address == BOOT_TABLE_SIZE + BOOT_TABLE_OFFSET - 1)
					pBuf[Index] = 0xAA;
				else if(Address < FAT_TABLE_SIZE + FAT1_TABLE_OFFSET)
					pBuf[Index] = FATn_TABLE[Address - FAT1_TABLE_OFFSET];
				else if(Address < FAT_TABLE_SIZE + FAT2_TABLE_OFFSET)
					pBuf[Index] = FATn_TABLE[Address - FAT2_TABLE_OFFSET];
				else if(Address < ROOT_TABLE_SIZE + ROOT_TABLE_OFFSET)
					pBuf[Index] = ROOT_TABLE[Address - ROOT_TABLE_OFFSET];
				
				Address ++;
			}
		}
		else
		{
			Memory_Offset -= FAT_USED_SIZE;
			for(Index = 0; Index < Transfer_Length; Index += 4)
			{
				Readbuff[Index>>2] = *((uint32_t *)(FAT_TABLE_ADDR + Memory_Offset + Index));
			}
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
