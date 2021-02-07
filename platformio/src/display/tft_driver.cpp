

#include "tft_driver.h"

#include <arduino.h>

#include "bssr_tables.h"
#include "hal/gpio.h"

// // Assuming landscape mode per memory access command 0x36.
#define WIDTH 480
#define HEIGHT 320


// #define ILI9488_TFTWIDTH 480
// #define ILI9488_TFTHEIGHT 320

// #define ILI9488_NOP 0x00
// #define ILI9488_SWRESET 0x01
// #define ILI9488_RDDID 0x04
// #define ILI9488_RDDST 0x09

// #define ILI9488_SLPIN 0x10
#define ILI9488_SLPOUT 0x11
// #define ILI9488_PTLON 0x12
// #define ILI9488_NORON 0x13

// #define ILI9488_RDMODE 0x0A
// #define ILI9488_RDMADCTL 0x0B
// #define ILI9488_RDPIXFMT 0x0C
// #define ILI9488_RDIMGFMT 0x0D
// #define ILI9488_RDSELFDIAG 0x0F

// #define ILI9488_INVOFF 0x20
// #define ILI9488_INVON 0x21
// #define ILI9488_GAMMASET 0x26
// #define ILI9488_DISPOFF 0x28
#define ILI9488_DISPON 0x29

#define ILI9488_CASET 0x2A
#define ILI9488_PASET 0x2B
#define ILI9488_RAMWR 0x2C
#define ILI9488_RAMRD 0x2E

// #define ILI9488_PTLAR 0x30
// #define ILI9488_MADCTL 0x36
// #define ILI9488_PIXFMT 0x3A

// #define ILI9488_FRMCTR1 0xB1
// #define ILI9488_FRMCTR2 0xB2
// #define ILI9488_FRMCTR3 0xB3
// #define ILI9488_INVCTR 0xB4
// #define ILI9488_DFUNCTR 0xB6

// #define ILI9488_PWCTR1 0xC0
// #define ILI9488_PWCTR2 0xC1
// #define ILI9488_PWCTR3 0xC2
// #define ILI9488_PWCTR4 0xC3
// #define ILI9488_PWCTR5 0xC4
// #define ILI9488_VMCTR1 0xC5
// #define ILI9488_VMCTR2 0xC7

// #define ILI9488_RDID1 0xDA
// #define ILI9488_RDID2 0xDB
// #define ILI9488_RDID3 0xDC
// #define ILI9488_RDID4 0xDD

// #define ILI9488_GMCTRP1 0xE0
// #define ILI9488_GMCTRN1 0xE1

// Color definitions
// #define ILI9488_BLACK 0x0000       /*   0,   0,   0 */
// #define ILI9488_NAVY 0x000F        /*   0,   0, 128 */
// #define ILI9488_DARKGREEN 0x03E0   /*   0, 128,   0 */
// #define ILI9488_DARKCYAN 0x03EF    /*   0, 128, 128 */
// #define ILI9488_MAROON 0x7800      /* 128,   0,   0 */
// #define ILI9488_PURPLE 0x780F      /* 128,   0, 128 */
// #define ILI9488_OLIVE 0x7BE0       /* 128, 128,   0 */
// #define ILI9488_LIGHTGREY 0xC618   /* 192, 192, 192 */
// #define ILI9488_DARKGREY 0x7BEF    /* 128, 128, 128 */
// #define ILI9488_BLUE 0x001F        /*   0,   0, 255 */
// #define ILI9488_GREEN 0x07E0       /*   0, 255,   0 */
// #define ILI9488_CYAN 0x07FF        /*   0, 255, 255 */
// #define ILI9488_RED 0xF800         /* 255,   0,   0 */
// #define ILI9488_MAGENTA 0xF81F     /* 255,   0, 255 */
// #define ILI9488_YELLOW 0xFFE0      /* 255, 255,   0 */
// #define ILI9488_WHITE 0xFFFF       /* 255, 255, 255 */
// #define ILI9488_ORANGE 0xFD20      /* 255, 165,   0 */
// #define ILI9488_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
// #define ILI9488_PINK 0xF81F



namespace tft_driver {

  // It seems that the CPU has better access to RAM vs Flash. Copying
// the bssr table to ram increase the transfer rate from ~8.5M
// pixels/sec to ~10M pixels/sec, at the cost of 2KB of RAM.
static uint64_t ram_color_bssr_table[256];

// Sending a byte using the 8 LSB bits of the 16 bit
// data path. This is how commands and their arguments
// are sent. Only colors are sent using 16 bits parallel
// transfer.
//
// TFT_WR is high before and after this function.
static inline void send_byte(uint8_t c) {
  // The TFT D15:D0 are divided across port A and port B. We
  // use a lookup table to set those pins to the desired value.
  // The port A bssr mask also resets the WR bit.
  const uint64_t bssr64 = bssr_tables::direct_bssr_table[c];
  // This also set TFT_WR to 0.
  GPIOA->BSRR = (uint32_t)bssr64;
  GPIOB->BSRR = (uint32_t)(bssr64 >> 32);
  TFT_WR_HIGH;
}

// Colors are sent using 16 bit parallel transfers.
static void send_color8(uint8_t color) {
  // The TFT D15:D0 are divided across port A and port B. We
  // use a lookup table to set those pins to the desired value.
  // The port A bssr mask also resets the WR bit.
  const uint64_t bssr64 = ram_color_bssr_table[color];
  // This also set TFT_WR to 0.
  GPIOA->BSRR = (uint32_t)bssr64;
  GPIOB->BSRR = (uint32_t)(bssr64 >> 32);
  TFT_WR_HIGH;
}

void writecommand(uint8_t c) {
  TFT_DC_LOW;  // Indicates a command
  send_byte(c);
}

void writedata(uint8_t c) {
  TFT_DC_HIGH;  // Indicates data
  send_byte(c);
}

void begin() {
  // Cache the BSSR color table in RAM for faster access.
  for (int i = 0; i < 256; i++) {
    ram_color_bssr_table[i] = bssr_tables::color_bssr_table[i];
  }

  // NOTE: we enable TFT_BL (backlight) later in main after completing
  // the initialization and filling the screen.
  TFT_WR_HIGH;
  TFT_RST_HIGH;

  delay(5);
  TFT_RST_LOW;
  delay(20);
  TFT_RST_HIGH;
  delay(150);

  writecommand(0xE0);
  writedata(0x00);
  writedata(0x03);
  writedata(0x09);
  writedata(0x08);
  writedata(0x16);
  writedata(0x0A);
  writedata(0x3F);
  writedata(0x78);
  writedata(0x4C);
  writedata(0x09);
  writedata(0x0A);
  writedata(0x08);
  writedata(0x16);
  writedata(0x1A);
  writedata(0x0F);

  writecommand(0XE1);
  writedata(0x00);
  writedata(0x16);
  writedata(0x19);
  writedata(0x03);
  writedata(0x0F);
  writedata(0x05);
  writedata(0x32);
  writedata(0x45);
  writedata(0x46);
  writedata(0x04);
  writedata(0x0E);
  writedata(0x0D);
  writedata(0x35);
  writedata(0x37);
  writedata(0x0F);

  writecommand(0XC0);  // Power Control 1
  writedata(0x17);     // Vreg1out
  writedata(0x15);     // Verg2out

  writecommand(0xC1);  // Power Control 2
  writedata(0x41);     // VGH,VGL

  writecommand(0xC5);  // Power Control 3
  writedata(0x00);
  writedata(0x12);  // Vcom
  writedata(0x80);

  writecommand(0x36);  // Memory Access
  writedata(0xe8);     // landscape mode. Swappin and mirroring x, y.

  // NOTE: For 16bit parallel transfer, IM jumbers need to be set
  // as 010.
  writecommand(0x3A);  // Interface Pixel Format
  writedata(0x55);     // 16 bit

  writecommand(0XB0);  // Interface Mode Control
  writedata(0x80);     // SDO NOT USE

  writecommand(0xB1);  // Frame rate
  writedata(0xA0);     // 60Hz

  writecommand(0xB4);  // Display Inversion Control
  writedata(0x02);     // 2-dot

  writecommand(0XB6);  // Display Function Control  RGB/MCU Interface Control

  writedata(0x02);  // MCU
  writedata(0x02);  // Source,Gate scan dieection

  writecommand(0XE9);  // Set Image Functio
  writedata(0x00);     // Disable 24 bit data

  writecommand(0xF7);  // Adjust Control
  writedata(0xA9);
  writedata(0x51);
  writedata(0x2C);
  writedata(0x82);  // D7 stream, loose

  writecommand(ILI9488_SLPOUT);  // Exit Sleep
  delay(120);
  writecommand(ILI9488_DISPON);  // Display on
}

// This is followed by a stream of pixels to render in this
// rectangle.
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  writecommand(ILI9488_CASET);  // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);  // XSTART
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);  // XEND

  writecommand(ILI9488_PASET);  // Row addr set
  writedata(y0 >> 8);
  writedata(y0 & 0xff);  // YSTART
  writedata(y1 >> 8);
  writedata(y1 & 0xff);  // YEND

  // Should follow by data bytes.
  writecommand(ILI9488_RAMWR);  // write to RAM

  // Next send the data bytes.
  TFT_DC_HIGH;
}

//  This is used to init the screen.
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= WIDTH) || (y >= HEIGHT)) return;
  if ((x + w - 1) >= WIDTH) w = WIDTH - x;
  if ((y + h - 1) >= HEIGHT) h = HEIGHT - y;

  setAddrWindow(x, y, x + w - 1, y + h - 1);

  for (int32_t i = w * h; i > 0; i--) {
    send_color8(color);
  }
}

//  This is used to init the screen.
void fillScreen(uint8_t color) { fillRect(0, 0, WIDTH, HEIGHT, color); }

// NOTE: The value written to GPIOA->BSRR also resets the TFT_WR output.
#define RENDER_NEXT_PIXEL \
  { \
    const uint64_t bssr64 =  ram_color_bssr_table[*p++]; \
    GPIOA->BSRR = (uint32_t)bssr64; \
    GPIOB->BSRR = (uint32_t)(bssr64 >> 32); \
    TFT_WR_HIGH; \
  }

// This function is time critical since it dominates the screen update time.
// LVGL writes to the screen via this function.
extern void render_buffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint8_t* color8_p) {
  setAddrWindow(x1, y1, x2, y2);

  const int32_t w_pixels = x2 - x1 + 1;
  const int32_t h_pixels = y2 - y1 + 1;

  uint8_t* p = color8_p;
  uint32_t pixels_left = w_pixels * h_pixels;

  // The code below was optimized for fast rendering, since it
  // handles all the LVGL display updates.
  //
  // First do inlined chunks of 10 per iteration. This
  // increase the transfer rate from ~5M pixels/sec to
  // ~9M.
  while (pixels_left >= 10) {
    pixels_left -= 10;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;

    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
    RENDER_NEXT_PIXEL;
  }

  // Do the rest of the pixels, one at a time, up to
  // 9 iterations.
  while (pixels_left > 0) {
    pixels_left--;
    RENDER_NEXT_PIXEL;
  }
}

}  // namespace tft_driver
