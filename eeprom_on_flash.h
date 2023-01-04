#ifndef INC_EEPROM_ON_FLASH_H_
#define INC_EEPROM_ON_FLASH_H_


#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"


#define EEPROM_DEFAULT_VALUE	0 //0xFFFFFFFF
#define EEPROM_MAX_CELL			128

typedef enum
{
	EEPROM_PAGE_ERROR = 0,
	EEPROM_PAGE_0,
	EEPROM_PAGE_1
} eeprom_page;


typedef enum
{
	EEPROM_WORK 	= 0xAAAAAAA0U,
	EEPROM_ERASHED 	= 0xAAAAAAAAU
} eeprom_page_t;


typedef struct
{
	eeprom_page page_work;

	uint32_t page0_addr;
	uint32_t page0_size;
	eeprom_page_t page0_type;
	uint32_t page0_count;
	uint32_t page0_last_addr;

	uint32_t page1_addr;
	uint32_t page1_size;
	eeprom_page_t page1_type;
	uint32_t page1_count;
	uint32_t page1_last_addr;
} eeprom_config_t;


typedef enum
{
	EEPROM_RET_OK 		= 0x00U,
	EEPROM_RET_ERROR 	= 0x01U,
	EEPROM_NEED_HELP
} eeprom_status_t;


// ----------------------- EEPROM ON FLASH -----------------------

eeprom_status_t EE_Init(eeprom_config_t * eeprom);
eeprom_status_t EE_Format(eeprom_config_t * eeprom);

eeprom_status_t EE_Write32bit(uint32_t addr, uint32_t value);
uint32_t 		EE_Read32bit(uint32_t addr);

eeprom_status_t EE_ReadVariable(uint32_t addr, uint32_t * value);
eeprom_status_t EE_WriteVariable(uint32_t addr, uint32_t value);

eeprom_status_t EE_ChangeWorkPage(eeprom_config_t * eeprom);
eeprom_page EE_GetWorkPage(eeprom_config_t * eeprom);

eeprom_page_t EE_GetPageType(eeprom_config_t * eeprom, eeprom_page page);

uint32_t EE_GetRecordsCount(eeprom_config_t * eeprom);
uint32_t EE_GetRecordsFree(eeprom_config_t * eeprom);

uint32_t EE_GetNextAddr(eeprom_config_t * eeprom);
uint32_t EE_GetPageNextAddr(eeprom_config_t * eeprom, eeprom_page page);

eeprom_status_t EE_WriteToPage(eeprom_config_t * eeprom, eeprom_page page, uint32_t virual_addr, uint32_t value);
eeprom_status_t EE_Write(eeprom_config_t * eeprom, uint32_t virual_addr, uint32_t value);
eeprom_status_t EE_Read(eeprom_config_t * eeprom, uint32_t virual_addr, uint32_t * value);

eeprom_status_t EE_GetAllVirtualAddress(eeprom_config_t * eeprom, uint32_t * p_addr, uint32_t * count);

// ---------------------------------------------------------------

#endif
