/*
 * ili9341.h - ILI9341 LCD driver
 * 07-03-19 E. Brombaugh
 * portions based on Adafruit ILI9341 driver for Arduino
 */

#include "clkcnt.h"
#include "font_8x8.h"
#include "ili9341.h"
#include "spi.h"

#define ILI9341_DC_CMD() (gp_out &= ~(1 << 30))
#define ILI9341_DC_DATA() (gp_out |= (1 << 30))
#define ILI9341_RST_LOW() (gp_out &= ~(1 << 31))
#define ILI9341_RST_HIGH() (gp_out |= (1 << 31))

#define ILI9341_CMD 0x100
#define ILI9341_DLY 0x200
#define ILI9341_END 0x400

/* some flags for init */
#define INITR_GREENTAB 0x0
#define INITR_REDTAB 0x1

#define ILI9341_NOP 0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID 0x04
#define ILI9341_RDDST 0x09

#define ILI9341_SLPIN 0x10
#define ILI9341_SLPOUT 0x11
#define ILI9341_PTLON 0x12
#define ILI9341_NORON 0x13

#define ILI9341_INVOFF 0x20
#define ILI9341_INVON 0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON 0x29
#define ILI9341_CASET 0x2A
#define ILI9341_RASET 0x2B
#define ILI9341_RAMWR 0x2C
#define ILI9341_RAMRD 0x2E

#define ILI9341_PTLAR 0x30
#define ILI9341_PIXFMT 0x3A
#define ILI9341_MADCTL 0x36

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR 0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1 0xC0
#define ILI9341_PWCTR2 0xC1
#define ILI9341_PWCTR3 0xC2
#define ILI9341_PWCTR4 0xC3
#define ILI9341_PWCTR5 0xC4
#define ILI9341_VMCTR1 0xC5
#define ILI9341_VMCTR2 0xC7

#define ILI9341_RDID1 0xDA
#define ILI9341_RDID2 0xDB
#define ILI9341_RDID3 0xDC
#define ILI9341_RDID4 0xDD

#define ILI9341_PWCTR6 0xFC

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

/* these values look better - more saturated, less flicker */
const static uint16_t initlst[] = {
    0x11 | ILI9341_CMD, // Exit Sleep
    120 | ILI9341_DLY,  // 120 ms delay
    0xCF | ILI9341_CMD, // Power control B
    0x00, 0xC3, 0x30,
    0xED | ILI9341_CMD, // Power on sequence control
    0x64, 0x03, 0x12, 0x81,
    0xE8 | ILI9341_CMD, // Driver timing control A
    0x85, 0x10, 0x79,
    0xCB | ILI9341_CMD, // Power control B
    0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7 | ILI9341_CMD, // Pump ratio control
    0x20,
    0xEA | ILI9341_CMD, // Driver timing control B
    0x00, 0x00,
    ILI9341_PWCTR1 | ILI9341_CMD, // Power control 1
    0x22,
    ILI9341_PWCTR2 | ILI9341_CMD, // Power control 2
    0x11,
    ILI9341_VMCTR1 | ILI9341_CMD, // VCOM Control 1
    0x3d, 0x20,
    ILI9341_VMCTR2 | ILI9341_CMD, // VCOM Control 2
    0xAA,
    ILI9341_MADCTL | ILI9341_CMD, // Memory Access Control
    0xA8,                         // Inverted row address mode, row/col exchange & RGB
    ILI9341_PIXFMT | ILI9341_CMD, // Pixel Format
    0x55,                         // 16-bit
    ILI9341_FRMCTR1 | ILI9341_CMD,
    0x00, 0x13,
    ILI9341_DFUNCTR | ILI9341_CMD,
    0x0A, 0xA2,
    0xF6 | ILI9341_CMD,
    0x01, 0x30,
    0xF2 | ILI9341_CMD, // 3Gamma Function Disable
    0x00,
    ILI9341_GAMMASET | ILI9341_CMD, // Set Gamma
    0x01,
    ILI9341_GMCTRP1 | ILI9341_CMD, // Gamma
    0x0F, 0x3f, 0x2f, 0x0c,
    0x10, 0x0A, 0x53, 0xD5,
    0x40, 0x0A, 0x13, 0x03,
    0x08, 0x03, 0x00,
    ILI9341_GMCTRN1 | ILI9341_CMD, // Gamma
    0x00, 0x00, 0x10, 0x03,
    0x0f, 0x05, 0x2c, 0xa2,
    0x3f, 0x05, 0x0e, 0x0c,
    0x37, 0x3c, 0x0F,
    ILI9341_SLPOUT | ILI9341_CMD, // Exit Sleep
    120 | ILI9341_DLY,            // 120 ms delay
    ILI9341_DISPON | ILI9341_CMD, // display on
    50 | ILI9341_DLY,             // 50 ms delay
    ILI9341_END                   // END
};

/* pointer to SPI port */
SPI_TypeDef *ili9341_spi;

/*
 * send single byte via SPI - cmd or data depends on bit 8
 */
void ili9341_write(uint16_t dat)
{
    if ((dat & ILI9341_CMD) == ILI9341_CMD)
    {
        ILI9341_DC_CMD();
    }
    else
    {
        ILI9341_DC_DATA();
    }

    spi_tx_byte(ili9341_spi, dat & 0xff);
}

/*
 * initialize the LCD
 */
void ili9341_init(SPI_TypeDef *s)
{
    // save SPI port
    ili9341_spi = s;

    // Reset it
    ILI9341_RST_LOW();
    clkcnt_delayms(50);
    ILI9341_RST_HIGH();
    clkcnt_delayms(50);

    // Send init command list
    uint16_t *addr = (uint16_t *)initlst, ms;
    while (*addr != ILI9341_END)
    {
        if ((*addr & ILI9341_DLY) != ILI9341_DLY)
        {
            ili9341_write(*addr++);
        }
        else
        {
            ms = (*addr++) & 0x1ff; // strip delay time (ms)
            clkcnt_delayms(ms);
        }
    }
}

/*
 * opens a window into display mem for bitblt
 */
void ili9341_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    ili9341_write(ILI9341_CASET | ILI9341_CMD); // Column addr set
    ili9341_write(x0 >> 8);
    ili9341_write(x0 & 0xff); // XSTART
    ili9341_write(x1 >> 8);
    ili9341_write(x1 & 0xff); // XEND

    ili9341_write(ILI9341_RASET | ILI9341_CMD); // Row addr set
    ili9341_write(y0 >> 8);
    ili9341_write(y0 & 0xff); // YSTART
    ili9341_write(y1 >> 8);
    ili9341_write(y1 & 0xff); // YEND

    ili9341_write(ILI9341_RAMWR | ILI9341_CMD); // write to RAM
}

/*
 * convert HSV triple to RGB triple
 * source: http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
 */
void ili9341_hsv2rgb(uint8_t rgb[], uint8_t hsv[])
{
    uint16_t C;
    int16_t Hprime, Cscl;
    uint8_t hs, X, m;

    /* default */
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;

    /* calcs are easy if v = 0 */
    if (hsv[2] == 0)
    {
        return;
    }

    /* C is the chroma component */
    C = ((uint16_t)hsv[1] * (uint16_t)hsv[2]) >> 8;

    /* Hprime is fixed point with range 0-5.99 representing hue sector */
    Hprime = (int16_t)hsv[0] * 6;

    /* get intermediate value X */
    Cscl = (Hprime % 512) - 256;
    Cscl = Cscl < 0 ? -Cscl : Cscl;
    Cscl = 256 - Cscl;
    X = ((uint16_t)C * Cscl) >> 8;

    /* m is value offset */
    m = hsv[2] - C;

    /* get the hue sector (1 of 6) */
    hs = (Hprime) >> 8;

    /* map by sector */
    switch (hs)
    {
    case 0:
        /* Red -> Yellow sector */
        rgb[0] = C + m;
        rgb[1] = X + m;
        rgb[2] = m;
        break;

    case 1:
        /* Yellow -> Green sector */
        rgb[0] = X + m;
        rgb[1] = C + m;
        rgb[2] = m;
        break;

    case 2:
        /* Green -> Cyan sector */
        rgb[0] = m;
        rgb[1] = C + m;
        rgb[2] = X + m;
        break;

    case 3:
        /* Cyan -> Blue sector */
        rgb[0] = m;
        rgb[1] = X + m;
        rgb[2] = C + m;
        break;

    case 4:
        /* Blue -> Magenta sector */
        rgb[0] = X + m;
        rgb[1] = m;
        rgb[2] = C + m;
        break;

    case 5:
        /* Magenta -> Red sector */
        rgb[0] = C + m;
        rgb[1] = m;
        rgb[2] = X + m;
        break;
    }
}

/*
 * convert 8-bit (each) R,G,B to 16-bit rgb565 packed color
 */
uint16_t ili9342_color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/*
 * fast color fill
 */
void ili9342_fill_color(uint16_t color, uint32_t sz)
{
    uint8_t lo = color & 0xff, hi = color >> 8;

    while (sz--)
    {
        /* wait for tx ready */
        spi_tx_wait(ili9341_spi);

        /* transmit hi byte */
        ili9341_spi->SPITXDR = hi;

        /* wait for tx ready */
        spi_tx_wait(ili9341_spi);

        /* transmit hi byte */
        ili9341_spi->SPITXDR = lo;
    }
}

/*
 * draw single pixel
 */
void ili9341_draw_pixel(int16_t x, int16_t y, uint16_t color)
{

    if ((x < 0) || (x >= ILI9341_TFTWIDTH) || (y < 0) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }

    ili9341_set_addr_window(x, y, x + 1, y + 1);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);
    ili9342_fill_color(color, 1);
    spi_cs_high(ili9341_spi);
}

/*
 * abs() helper function for line drawing
 */
int16_t ili9341_abs(int16_t x)
{
    return (x < 0) ? -x : x;
}

/*
 * swap() helper function for line drawing
 */
void ili9341_swap(int16_t *z0, int16_t *z1)
{
    int16_t temp = *z0;
    *z0 = *z1;
    *z1 = temp;
}

/*
 * bresenham line draw routine swiped from Wikipedia
 */
void ili9341_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int8_t steep;
    int16_t deltax, deltay, error, ystep, x, y;

    /* flip sense 45deg to keep error calc in range */
    steep = (ili9341_abs(y1 - y0) > ili9341_abs(x1 - x0));

    if (steep)
    {
        ili9341_swap(&x0, &y0);
        ili9341_swap(&x1, &y1);
    }

    /* run low->high */
    if (x0 > x1)
    {
        ili9341_swap(&x0, &x1);
        ili9341_swap(&y0, &y1);
    }

    /* set up loop initial conditions */
    deltax = x1 - x0;
    deltay = ili9341_abs(y1 - y0);
    error = deltax / 2;
    y = y0;
    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    /* loop x */
    for (x = x0; x <= x1; x++)
    {
        /* plot point */
        if (steep)
        { /* flip point & plot */
            ili9341_draw_pixel(y, x, color);
        }
        else
        { /* just plot */
            ili9341_draw_pixel(x, y, color);
        }

        /* update error */
        error = error - deltay;

        /* update y */
        if (error < 0)
        {
            y = y + ystep;
            error = error + deltax;
        }
    }
}

/*
 * fast vert line
 */
void ili9341_draw_vline_fast(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // clipping
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }
    if ((y + h - 1) >= ILI9341_TFTHEIGHT)
    {
        h = ILI9341_TFTHEIGHT - y;
    }
    ili9341_set_addr_window(x, y, x, y + h - 1);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);
    ili9342_fill_color(color, h);
    spi_cs_high(ili9341_spi);
}

/*
 * fast horiz line
 */
void ili9341_draw_hline_fast(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // clipping
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }
    if ((x + w - 1) >= ILI9341_TFTWIDTH)
    {
        w = ILI9341_TFTWIDTH - x;
    }
    ili9341_set_addr_window(x, y, x + w - 1, y);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);
    ili9342_fill_color(color, w);
    spi_cs_high(ili9341_spi);
}

/*
 * empty rect
 */
void ili9341_empty_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    /* top & bottom */
    ili9341_draw_hline_fast(x, y, w, color);
    ili9341_draw_hline_fast(x, y + h - 1, w, color);

    /* left & right - don't redraw corners */
    ili9341_draw_vline_fast(x, y + 1, h - 2, color);
    ili9341_draw_vline_fast(x + w - 1, y + 1, h - 2, color);
}

/*
 * fill a rectangle
 */
void ili9341_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color)
{
    // clipping
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }
    if ((x + w - 1) >= ILI9341_TFTWIDTH)
    {
        w = ILI9341_TFTWIDTH - x;
    }
    if ((y + h - 1) >= ILI9341_TFTHEIGHT)
    {
        h = ILI9341_TFTHEIGHT - y;
    }

    ili9341_set_addr_window(x, y, x + w - 1, y + h - 1);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);
    ili9342_fill_color(color, h * w);
    spi_cs_high(ili9341_spi);
}

/*
 * fill screen w/ single color
 */
void ili9341_fill_screen(uint16_t color)
{
    ili9341_fill_rect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, color);
}

/*
 * draw character direct to the display
 */
void ili9341_draw_char(int16_t x, int16_t y, uint8_t chr,
                       uint16_t fg, uint16_t bg)
{
    uint16_t i, j, col;
    uint8_t d;

    ili9341_set_addr_window(x, y, x + 7, y + 7);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);
    for (i = 0; i < 8; i++)
    {
        d = fontdata[(chr << 3) + i];
        for (j = 0; j < 8; j++)
        {
            if (d & 0x80)
            {
                col = fg;
            }
            else
            {
                col = bg;
            }

            ili9342_fill_color(col, 1);

            // next bit
            d <<= 1;
        }
    }
    spi_cs_high(ili9341_spi);
}

// draw a string to the display
void ili9341_draw_str(int16_t x, int16_t y, char *str,
                      uint16_t fg, uint16_t bg)
{
    uint8_t c;

    while ((c = *str++))
    {
        ili9341_draw_char(x, y, c, fg, bg);
        x += 8;
        if (x > ILI9341_TFTWIDTH)
        {
            break;
        }
    }
}

/*
 * send a buffer to the LCD
 */
void ili9341_blit(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t scale, uint16_t *src)
{
    int32_t ws = w * scale - 1;
    int32_t hs = h * scale - 1;

    // clipping
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }
    if ((x + ws) >= ILI9341_TFTWIDTH)
    {
        w = ILI9341_TFTWIDTH - x;
    }
    if ((y + hs) >= ILI9341_TFTHEIGHT)
    {
        h = ILI9341_TFTHEIGHT - y;
    }

    ili9341_set_addr_window(x, y, x + ws, y + hs);

    int32_t i = 0;

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);

    for (int16_t y = 0; y < h; ++y)
    {
        for (uint8_t j = 0; j < scale; ++j)
        {
            uint8_t *src_8 = (uint8_t *)&src[i];

            for (int16_t x = 0; x < w; ++x, src_8 += 2)
            {
                for (uint8_t k = 0; k < scale; ++k)
                {
                    spi_tx_wait(ili9341_spi);
                    ili9341_spi->SPITXDR = *src_8;

                    spi_tx_wait(ili9341_spi);
                    ili9341_spi->SPITXDR = *(src_8 + 1);
                }
            }
        }

        i += w;
    }

    spi_rx_wait(ili9341_spi);
    spi_cs_high(ili9341_spi);
}

void ili9341_blit_binary(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t scale, uint8_t *src)
{
    int32_t ws = w * scale - 1;
    int32_t hs = h * scale - 1;

    // clipping
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
    {
        return;
    }
    if ((x + ws) >= ILI9341_TFTWIDTH)
    {
        w = ILI9341_TFTWIDTH - x;
    }
    if ((y + hs) >= ILI9341_TFTHEIGHT)
    {
        h = ILI9341_TFTHEIGHT - y;
    }

    ili9341_set_addr_window(x, y, x + ws, y + hs);

    ILI9341_DC_DATA();
    spi_cs_low(ili9341_spi);

    int16_t line_width = w >> 3;
    for (int16_t y = 0; y < h; ++y)
    {
        for (uint8_t i = 0; i < scale; ++i)
        {
            for (int16_t x = 0; x < line_width; ++x)
            {
                uint8_t bit_pack = src[x];

                uint8_t color;
                for (uint8_t mask = 128; mask > 0; mask >>= 1)
                {
                    for (uint8_t k = 0; k < scale; ++k)
                    {
                        color = -((bit_pack & mask) != 0);
                        spi_tx_wait(ili9341_spi);
                        ili9341_spi->SPITXDR = color;
                        spi_tx_wait(ili9341_spi);
                        ili9341_spi->SPITXDR = color;
                    }
                }
            }
        }

        src += line_width;
    }

    spi_rx_wait(ili9341_spi);
    spi_cs_high(ili9341_spi);
}
