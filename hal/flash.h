/*
 * flash.h - flash memory driver
 * 07-03-19 E. Brombaugh
 */

#ifndef __flash__
#define __flash__

#include "up5k_soc.h"

void flash_init(SPI_TypeDef *s);
void flash_read(SPI_TypeDef *s, uint8_t *dst, uint32_t addr, uint32_t len);
uint8_t flash_rdreg(SPI_TypeDef *s, uint8_t cmd);
uint8_t flash_status(SPI_TypeDef *s);
void flash_busy_wait(SPI_TypeDef *s);
void flash_eraseblk(SPI_TypeDef *s, uint32_t addr);
void flash_write(SPI_TypeDef *s, uint8_t *src, uint32_t addr, uint32_t len);
uint32_t flash_id(SPI_TypeDef *s);

#endif
