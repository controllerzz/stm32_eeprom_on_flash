#include "eeprom_on_flash.h"


eeprom_status_t EE_ErasePage(eeprom_config_t * eeprom, eeprom_page page)
{
	HAL_StatusTypeDef ret;
	uint32_t page_error;

	FLASH_EraseInitTypeDef erase_config;
	erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_config.NbPages = 1;
	erase_config.PageAddress = (page == EEPROM_PAGE_0 ? eeprom->page0_addr : eeprom->page1_addr);

	HAL_FLASH_Unlock();
	ret = HAL_FLASHEx_Erase(&erase_config, &page_error);
	HAL_FLASH_Lock();

	if(ret == HAL_OK)
		return EEPROM_RET_OK;

	return EEPROM_RET_ERROR;
}


eeprom_status_t EE_Format(eeprom_config_t * eeprom)
{
	if(EE_ErasePage(eeprom, EEPROM_PAGE_0) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;

	if(EE_ErasePage(eeprom, EEPROM_PAGE_1) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;


	EE_Write32bit(eeprom->page0_addr, EEPROM_WORK);
	EE_Write32bit(eeprom->page0_addr + 4, 0);
	eeprom->page0_count = 0;

	EE_Write32bit(eeprom->page1_addr, EEPROM_WORK);
	EE_Write32bit(eeprom->page1_addr + 4, 1);
	eeprom->page1_count = 1;

	return EEPROM_RET_ERROR;
}


eeprom_status_t EE_Init(eeprom_config_t * eeprom)
{
	uint32_t value;

	value = EE_Read32bit(eeprom->page0_addr);
	if(value != EEPROM_WORK)
		return EEPROM_RET_ERROR;
	eeprom->page0_type = (eeprom_page_t)value;
	eeprom->page0_count = EE_Read32bit(eeprom->page0_addr + 4);

	value = EE_Read32bit(eeprom->page1_addr);
	if(value != EEPROM_WORK)
		return EEPROM_RET_ERROR;
	eeprom->page1_type = (eeprom_page_t)value;
	eeprom->page1_count = EE_Read32bit(eeprom->page1_addr + 4);

	if(eeprom->page0_type != EEPROM_WORK && eeprom->page1_type != EEPROM_WORK)
		return EEPROM_NEED_HELP;


	if((eeprom->page0_count == 0xFFFFFFFF || eeprom->page1_count == 0xFFFFFFFF) && (eeprom->page0_count == 0 || eeprom->page1_count == 0))
	{
		if(eeprom->page0_count > eeprom->page1_count)
		{
			eeprom->page_work = EEPROM_PAGE_0;
			return EEPROM_RET_OK;
		}
		else
		{
			eeprom->page_work = EEPROM_PAGE_1;
			return EEPROM_RET_OK;
		}
	}
	else
	{
		if(eeprom->page0_count < eeprom->page1_count)
		{
			eeprom->page_work = EEPROM_PAGE_0;
			return EEPROM_RET_OK;
		}
		else
		{
			eeprom->page_work = EEPROM_PAGE_1;
			return EEPROM_RET_OK;
		}
	}

	return EEPROM_RET_ERROR;
}


uint32_t EE_GetNextAddr(eeprom_config_t * eeprom)
{
	uint32_t addr = (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;
	uint32_t addr_end = addr;
	addr_end += (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_size : eeprom->page1_size;

	for(addr += 8; addr < addr_end; addr += 8)
	{
		if(EE_Read32bit(addr) == 0xFFFFFFFF)
			return addr;
	}

	return 0xFFFFFFFF;
}


uint32_t EE_GetPageNextAddr(eeprom_config_t * eeprom, eeprom_page page)
{
	uint32_t addr = (page == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;
	uint32_t addr_end = addr;
	addr_end += (page == EEPROM_PAGE_0) ? eeprom->page0_size : eeprom->page1_size;

	for(addr += 8; addr < addr_end; addr += 8)
	{
		if(EE_Read32bit(addr) == 0xFFFFFFFF)
			return addr;
	}

	return 0;
}


uint8_t is_present(uint32_t * p_addr, uint32_t count, uint32_t value)
{
	for(; count > 0; count--)
	{
		if(*p_addr == value)
			return 1;
		p_addr++;
	}

	return 0;
}


eeprom_status_t EE_GetAllVirtualAddress(eeprom_config_t * eeprom, uint32_t * p_addr, uint32_t * count)
{
	uint32_t read_value;
	*count = 0;

	uint32_t addr = (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;
	uint32_t addr_end = addr;
	addr_end += (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_size : eeprom->page1_size;

	for(addr += 8; addr < addr_end; addr += 8)
	{
		read_value = EE_Read32bit(addr);

		if(read_value == 0xFFFFFFFF)
			return EEPROM_RET_OK;

		if(is_present(p_addr, *count, read_value) == 0)
		{
			p_addr[*count] = read_value;
			*count = *count + 1;
		}
	}

	return EEPROM_RET_OK;
}


eeprom_status_t EE_Write(eeprom_config_t * eeprom, uint32_t virual_addr, uint32_t value)
{
	uint32_t write_addr;
	uint32_t value_current;

	EE_Read(eeprom, virual_addr, &value_current);
	if(value_current == value)
		return EEPROM_RET_OK;

	write_addr = EE_GetNextAddr(eeprom);

	if(write_addr == 0xFFFFFFFF)
	{
		eeprom_page page_new, page = (eeprom->page_work == EEPROM_PAGE_0 ? EEPROM_PAGE_1 : EEPROM_PAGE_0);
		page_new = page;
		uint32_t page_addr = (page == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;
		uint32_t page_addr_end = page_addr;
		page_addr_end += (page == EEPROM_PAGE_0) ? eeprom->page0_size : eeprom->page1_size;

		uint32_t value;
		uint8_t is_need_erase = 0;

		uint32_t addr[EEPROM_MAX_CELL];
		uint32_t count;

		for(uint32_t addr_test = page_addr + 8; addr_test < page_addr_end; addr_test += 4)
		{
			if(EE_Read32bit(addr_test) != 0xFFFFFFFF)
				is_need_erase = 1;
		}


		if(is_need_erase == 1)
		{
			EE_ErasePage(eeprom, page);
			if(page == EEPROM_PAGE_1)
				count = eeprom->page0_count;
			else
				count = eeprom->page1_count;
			count++;

			EE_Write32bit(page_addr + 4, count);
			EE_Write32bit(page_addr, EEPROM_WORK);
		}

		EE_GetAllVirtualAddress(eeprom, addr, &count);

		for(uint32_t idx = 0; idx < count; idx++)
		{
			EE_Read(eeprom, addr[idx], &value);
			EE_WriteToPage(eeprom, page, addr[idx], value);
		}

		page = eeprom->page_work;
		EE_ErasePage(eeprom, page);
		page_addr = (page == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;

		count = (eeprom->page0_count > eeprom->page1_count) ? eeprom->page0_count : eeprom->page1_count;
		count++;

		if(page == EEPROM_PAGE_0)
			eeprom->page0_count = count;
		else
			eeprom->page1_count = count;

		EE_Write32bit(page_addr + 4, count);
		EE_Write32bit(page_addr, EEPROM_WORK);

		eeprom->page_work = page_new;
		write_addr = EE_GetNextAddr(eeprom);
	}

	if(EE_Write32bit(write_addr, virual_addr) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;

	if(EE_Write32bit(write_addr + 4, value) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;

	return EEPROM_RET_OK;
}


eeprom_status_t EE_WriteToPage(eeprom_config_t * eeprom, eeprom_page page, uint32_t virual_addr, uint32_t value)
{
	uint32_t write_addr;
	write_addr = EE_GetPageNextAddr(eeprom, page);

	if(write_addr == 0xFFFFFFFF)
	{
		return EEPROM_RET_ERROR;
	}

	if(EE_Write32bit(write_addr, virual_addr) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;

	if(EE_Write32bit(write_addr + 4, value) != EEPROM_RET_OK)
		return EEPROM_RET_ERROR;

	return EEPROM_RET_OK;
}


eeprom_status_t EE_Read(eeprom_config_t * eeprom, uint32_t virual_addr, uint32_t * value)
{
	uint32_t value_addr = 0xFFFFFFFF;

	uint32_t addr = (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_addr : eeprom->page1_addr;
	uint32_t addr_end = addr;
	addr_end += (eeprom->page_work == EEPROM_PAGE_0) ? eeprom->page0_size : eeprom->page1_size;

	for(addr += 8; addr < addr_end; addr += 8)
	{
		if(EE_Read32bit(addr) == virual_addr)
			value_addr = addr + 4;

		if(EE_Read32bit(addr) == 0xFFFFFFFF)
			break;

	}

	if(value_addr == 0xFFFFFFFF)
	{
		*value = EEPROM_DEFAULT_VALUE;
		return EEPROM_RET_ERROR;
	}

	*value = EE_Read32bit(value_addr);

	return EEPROM_RET_OK;
}


eeprom_status_t EE_Write32bit(uint32_t addr, uint32_t value)
{
	HAL_StatusTypeDef ret;

	if(HAL_FLASH_Unlock() != HAL_OK)
		return EEPROM_RET_ERROR;

	ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);

	if(HAL_FLASH_Lock() != HAL_OK)
		return EEPROM_RET_ERROR;

	if(ret == HAL_OK)
		return EEPROM_RET_OK;

	return EEPROM_RET_ERROR;
}


uint32_t EE_Read32bit(uint32_t addr)
{
	return *((uint32_t *)addr);
}
