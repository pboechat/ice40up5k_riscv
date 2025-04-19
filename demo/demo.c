/*
 * demo firmware
 */

#include "acia.h"
#include "clkcnt.h"
#include "flash.h"
#include "ili9341.h"
#include "i2c.h"
#include "spi.h"
#include "up5k_soc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "printf.h"

void main(void)
{
	uint32_t cnt, spi_id;

	init_printf(0, acia_printf_putc);

	printf("\n\n\r");
	printf(" (               *         )   \n\r");
	printf(" )\\ )          (  `     ( /(   \n\r");
	printf("(()/(    (     )\\))(    )\\())  \n\r");
	printf(" /(_))   )\\   ((_)()\\  ((_)\\   \n\r");
	printf("(_))_   ((_)  (_()((_)   ((_)  \n\r");
	printf(" |   \\  | __| |  \\/  |  / _ \\  \n\r");
	printf(" | |) | | _|  | |\\/| | | (_) | \n\r");
	printf(" |___/  |___| |_|  |_|  \\___/  \n\r");
	printf("\n\r");

#if 0
	/* test dynamic allocation */

	void *ptr = malloc(0x1000);  // allocate 1 KB
    if (ptr) 
	{
        printf("malloc() allocated at 0x%08X\n\r", (intptr_t)ptr);
    }
	else 
	{
        printf("malloc() failed!\n\r");
    }
#endif

	/* initialize both SPI ports */
	spi_init(SPI0);
	spi_init(SPI1);

	/* get SPI flash id */
	flash_init(SPI0); // wake up the flash chip
	spi_id = flash_id(SPI0);
	printf("SPI flash id: 0x%08X\n\r", spi_id);

#if 0
	/* read some data */
	{
		uint8_t read[256];
		flash_read(SPI0, read, 0, 256);
		for(uint32_t i = 0; i < 256; i += 8)
		{
			printf("0x%02X: ", i);
			for(uint32_t j = 0; j < 8; ++j)
			{
				printf("0x%02X ", read[i + j]);
			}
			printf("\n\r");
		}
	}
#endif

	/* initialize LCD */
	ili9341_init(SPI1);
	printf("LCD initialized\n\r");

#if 1
	/* color fill + text fonts */
	ili9341_fill_rect(20, 20, ILI9341_TFTWIDTH - 40, ILI9341_TFTHEIGHT - 40, ILI9341_MAGENTA);
	ili9341_draw_str((ILI9341_TFTWIDTH >> 1) - 44, 44, "Hello World", ILI9341_WHITE, ILI9341_MAGENTA);

	/* test font */
	for (uint32_t c = 0, p_y = ((ILI9341_TFTHEIGHT - 112) >> 1); c < 256; c += 16, p_y += 8)
	{
		for (uint32_t x = 0, p_x = ((ILI9341_TFTWIDTH - 128) >> 1); x < 16; ++x, p_x += 8)
		{
			ili9341_draw_char(p_x, p_y, (uint8_t)c + x, ILI9341_GREEN, ILI9341_BLACK);
		}
	}

	clkcnt_delayms(1000);
#endif

#if 0
	/* test colored lines */
	{
		uint8_t rgb[3], hsv[3];
		uint16_t color;
		ili9341_fill_screen(ILI9341_BLACK);
		hsv[1] = 255;
		hsv[2] = 255;
		uint32_t j = 256;
		while (j--)
		{
			for (uint32_t i = 0; i < ILI9341_TFTHEIGHT; ++i)
			{
				hsv[0] = (i + j);

				ili9341_hsv2rgb(rgb, hsv);
				color = ili9342_color565(rgb[0], rgb[1], rgb[2]);
#if 0
				ili9341_draw_line(i, 0, ILI9341_TFTWIDTH-1, i, color);
				ili9341_draw_line(ILI9341_TFTWIDTH-1, i, ILI9341_TFTWIDTH-1 - i, ILI9341_TFTWIDTH-1, color);
				ili9341_draw_line(ILI9341_TFTWIDTH-1 - i, ILI9341_TFTWIDTH-1, 0, ILI9341_TFTWIDTH-1 - i, color);
				ili9341_draw_line(0, ILI9341_TFTWIDTH-1 - i, i, 0, color);
#else
				ili9341_draw_hline_fast(0, i, ILI9341_TFTWIDTH, color);
#endif
			}
		}
	}
	clkcnt_delayms(1000);
#endif

#if 0
	/* test image blit from flash */
	{
		uint16_t blit[ILI9341_TFTWIDTH*4];
		uint32_t blitaddr, blitsz;
		blitaddr = 0x200000;
		blitsz = ILI9341_TFTWIDTH*4*sizeof(uint16_t);
		for(uint32_t i = 0; i < ILI9341_TFTHEIGHT; i += 4)
		{
			flash_read(SPI0, (uint8_t *)blit, blitaddr, blitsz);
			ili9341_blit(0, i, ILI9341_TFTWIDTH, 4, blit);
			blitaddr += blitsz;
		}
	}
#endif

	/* initialize I2C */
	i2c_init(I2C0);
	printf("I2C0 initialized\n\r");

	cnt = 0;
	while (1)
	{
		gp_out = (gp_out & ~(7 << 17)) | ((cnt & 7) << 17);

		if (i2c_tx(I2C0, 0x1A, (uint8_t *)&cnt, 2))
			acia_putc('x');
		else
			acia_putc('.');

		cnt++;

#if 0
		/* simple echo */
		if((c=acia_getc()) != EOF)
			acia_putc(c);
#endif
		clkcnt_delayms(1000);
	}
}
