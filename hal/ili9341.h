/*
 * ili9341.h - ILI9341 LCD driver
 * 07-03-19 E. Brombaugh
 * portions based on Adafruit ILI9341 driver for Arduino
 */

#ifndef __ili9341__
#define __ili9341__

#include "up5k_soc.h"

// color definitions
#define ILI9341_BLACK 0x0000
#define ILI9341_BLUE 0x001F
#define ILI9341_RED 0xF800
#define ILI9341_GREEN 0x07E0
#define ILI9341_CYAN 0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW 0xFFE0
#define ILI9341_WHITE 0xFFFF

#define ILI9341_TFTWIDTH 320
#define ILI9341_TFTHEIGHT 240

void ili9341_init(SPI_TypeDef *s);
void ili9341_draw_pixel(int16_t x, int16_t y, uint16_t color);
void ili9341_draw_vline_fast(int16_t x, int16_t y, int16_t h, uint16_t color);
void ili9341_draw_hline_fast(int16_t x, int16_t y, int16_t w, uint16_t color);
void ili9341_hsv2rgb(uint8_t rgb[], uint8_t hsv[]);
uint16_t ili9342_color565(uint8_t r, uint8_t g, uint8_t b);
void ili9341_empty_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ili9341_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ili9341_fill_screen(uint16_t color);
void ili9341_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void ili9341_draw_char(int16_t x, int16_t y, uint8_t chr, uint16_t fg, uint16_t bg);
void ili9341_draw_str(int16_t x, int16_t y, char *str, uint16_t fg, uint16_t bg);
void ili9341_blit(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *src);
void ili9341_blit_binary(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t scale, uint8_t *data);
#endif
