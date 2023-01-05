<!-- Описание -->
## Описание

Библиотека предназначина для организации виртуальной <b>EEPROM</b> на базе внутренней <b>FLASH</b> памяти микроконтроллеров <b>STM32</b>

<!-- USAGE EXAMPLES -->
## Пример использования

Добавляем заголовочный файл библиотеки
``` c
#include "eeprom_on_flash.h"
```

Объявляем переменные для работы с EEPROM
``` c
eeprom_status_t ee_status;
eeprom_config_t eeprom;
```

Задаем адреса и размеры страниц для EEPROM
``` c
eeprom.page0_addr = 0x0801F000;
eeprom.page0_size = 0x800;

eeprom.page1_addr = 0x0801F800;
eeprom.page1_size = 0x800;
```

Инициализируем EEPROM, если EEPROM не создана то форматируем ее
``` c
ee_status = EE_Init(&eeprom);
if(ee_status != EEPROM_RET_OK)
{
  EE_Format(&eeprom);
}
```

Записываем данные по адресу 0x100
``` c
uint32_t data_to_flash = 0x11223344;
ee_status = EE_Write(&eeprom, 0x100, &data_to_flash);
```

Читаем данные из адреса 0x100
``` c
uint32_t data_from_flash;
ee_status = EE_Read(&eeprom, 0x100, &data_from_flash);
```
