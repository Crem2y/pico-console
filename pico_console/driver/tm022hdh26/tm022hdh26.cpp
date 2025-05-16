// tm022hdh26 display lib

// Based in part on Adafruit ILI9340 display driver
// https://github.com/adafruit/Adafruit_ILI9340

/*
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution.
  Original source: https://github.com/adafruit/Adafruit_ILI9340
*/

// original source
// https://github.com/shawnhyam/pico/blob/main/ili9341/ili9341.c

#include "tm022hdh26.hpp"

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop");
}

/////////////////////////////////////////////////////////////////////

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
tm022hdh26::tm022hdh26(int pin_reset, int pin_dc, int pin_cs, int pin_led) : Adafruit_GFX(ILI9340_TFTWIDTH, ILI9340_TFTHEIGHT) {
  _pin_reset = pin_reset;
  _pin_dc = pin_dc;
  _pin_cs = pin_cs;
  _pin_led = pin_led;
}

void tm022hdh26::spiwrite(uint8_t cmd) {
  spi_write_blocking(DISPLAY_SPI_CH, &cmd, 1);
}

void tm022hdh26::writecommand(uint8_t cmd) {
  cs_select(_pin_cs);
  gpio_put(_pin_dc, 0);
  spi_write_blocking(DISPLAY_SPI_CH, &cmd, 1);
  gpio_put(_pin_dc, 1);
  cs_deselect(_pin_cs);
}

void tm022hdh26::writedata(uint8_t d) {
  cs_select(_pin_cs);
  spi_write_blocking(DISPLAY_SPI_CH, &d, 1);
  cs_deselect(_pin_cs);
} 

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void tm022hdh26::commandList(uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      sleep_ms(ms);
    }
  }
}


void tm022hdh26::begin(void) {
  gpio_init(_pin_reset);
  gpio_set_dir(_pin_reset, GPIO_OUT);
  gpio_put(_pin_reset, 1);

  gpio_init(_pin_dc);
  gpio_set_dir(_pin_dc, GPIO_OUT);

  gpio_init(_pin_cs);
  gpio_set_dir(_pin_cs, GPIO_OUT);
  gpio_put(_pin_cs, 1);

  spi_init(DISPLAY_SPI_CH, 25 * 1000000);
  gpio_set_function(DISPLAY_RX, GPIO_FUNC_SPI);
  gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
  gpio_set_function(DISPLAY_TX, GPIO_FUNC_SPI);

  // toggle RST low to reset
  gpio_put(_pin_reset, 0);
  sleep_ms(20);
  gpio_put(_pin_reset, 1);
  sleep_ms(150);

  /*
  uint8_t x = readcommand8(ILI9340_RDMODE);
  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9340_RDMADCTL);
  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9340_RDPIXFMT);
  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9340_RDIMGFMT);
  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9340_RDSELFDIAG);
  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
  */

  //if(cmdList) commandList(cmdList);
  
  writecommand(0xEF);
  writedata(0x03);
  writedata(0x80);
  writedata(0x02);

  writecommand(0xCF);  
  writedata(0x00); 
  writedata(0XC1); 
  writedata(0X30); 

  writecommand(0xED);  
  writedata(0x64); 
  writedata(0x03); 
  writedata(0X12); 
  writedata(0X81); 
 
  writecommand(0xE8);  
  writedata(0x85); 
  writedata(0x00); 
  writedata(0x78); 

  writecommand(0xCB);  
  writedata(0x39); 
  writedata(0x2C); 
  writedata(0x00); 
  writedata(0x34); 
  writedata(0x02); 
 
  writecommand(0xF7);  
  writedata(0x20); 

  writecommand(0xEA);  
  writedata(0x00); 
  writedata(0x00); 
 
  writecommand(ILI9340_PWCTR1);    //Power control 
  writedata(0x23);   //VRH[5:0] 
 
  writecommand(ILI9340_PWCTR2);    //Power control 
  writedata(0x10);   //SAP[2:0];BT[3:0] 
 
  writecommand(ILI9340_VMCTR1);    //VCM control 
  writedata(0x3e);
  writedata(0x28); 
  
  writecommand(ILI9340_VMCTR2);    //VCM control2 
  writedata(0x86);
 
  writecommand(ILI9340_MADCTL);    // Memory Access Control 
  writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

  writecommand(ILI9340_PIXFMT);    
  writedata(0x55); 
  
  writecommand(ILI9340_FRMCTR1);    
  writedata(0x00);  
  writedata(0x18); 
 
  writecommand(ILI9340_DFUNCTR);    // Display Function Control 
  writedata(0x08); 
  writedata(0x82);
  writedata(0x27);  
 
  writecommand(0xF2);    // 3Gamma Function Disable 
  writedata(0x00); 
 
  writecommand(ILI9340_GAMMASET);    //Gamma curve selected 
  writedata(0x01); 
 
  writecommand(ILI9340_GMCTRP1);    //Set Gamma 
  writedata(0x0F); 
  writedata(0x31); 
  writedata(0x2B); 
  writedata(0x0C); 
  writedata(0x0E); 
  writedata(0x08); 
  writedata(0x4E); 
  writedata(0xF1); 
  writedata(0x37); 
  writedata(0x07); 
  writedata(0x10); 
  writedata(0x03); 
  writedata(0x0E); 
  writedata(0x09); 
  writedata(0x00); 
  
  writecommand(ILI9340_GMCTRN1);    //Set Gamma 
  writedata(0x00); 
  writedata(0x0E); 
  writedata(0x14); 
  writedata(0x03); 
  writedata(0x11); 
  writedata(0x07); 
  writedata(0x31); 
  writedata(0xC1); 
  writedata(0x48); 
  writedata(0x08); 
  writedata(0x0F); 
  writedata(0x0C); 
  writedata(0x31); 
  writedata(0x36); 
  writedata(0x0F); 

  writecommand(ILI9340_SLPOUT);    //Exit Sleep 
  sleep_ms(120); 		
  writecommand(ILI9340_DISPON);    //Display on 

  setRotation(3);

  // pwm initalize
  gpio_set_function(_pin_led, GPIO_FUNC_PWM);
  slice_num = pwm_gpio_to_slice_num(_pin_led);
  led_pwm_ch = (_pin_led % 2) ? PWM_CHAN_B : PWM_CHAN_A;

  pwm_set_chan_level(slice_num, led_pwm_ch, 0);
  pwm_set_clkdiv(slice_num, SYS_CLK_KHZ * 1000/LCD_BACKLIGHT_MAX);
  pwm_set_wrap(slice_num, 1000);
  pwm_set_enabled(slice_num, true);
}


void tm022hdh26::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) {

  writecommand(ILI9340_CASET); // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);     // XSTART 
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);     // XEND

  writecommand(ILI9340_PASET); // Row addr set
  writedata(y0>>8);
  writedata(y0);     // YSTART
  writedata(y1>>8);
  writedata(y1);     // YEND

  writecommand(ILI9340_RAMWR); // write to RAM
}


void tm022hdh26::pushColor(uint16_t color) {

  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);

  spiwrite(color >> 8);
  spiwrite(color);

  cs_deselect(_pin_cs);
}

void tm022hdh26::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);

  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);

  spiwrite(color >> 8);
  spiwrite(color);

  cs_deselect(_pin_cs);
}


void tm022hdh26::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;

  if((y+h-1) >= _height) 
    h = _height-y;

  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);
  
  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }

  cs_deselect(_pin_cs);
}


void tm022hdh26::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;
  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);
  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }
  cs_deselect(_pin_cs);
}

void tm022hdh26::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void tm022hdh26::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
  cs_deselect(_pin_cs);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t tm022hdh26::Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


void tm022hdh26::setRotation(uint8_t m) {

  writecommand(ILI9340_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTWIDTH;
     _height = ILI9340_TFTHEIGHT;
     break;
   case 1:
     writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTHEIGHT;
     _height = ILI9340_TFTWIDTH;
     break;
  case 2:
    writedata(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTWIDTH;
     _height = ILI9340_TFTHEIGHT;
    break;
   case 3:
     writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTHEIGHT;
     _height = ILI9340_TFTWIDTH;
     break;
  }
}


void tm022hdh26::invertDisplay(bool i) {
  writecommand(i ? ILI9340_INVON : ILI9340_INVOFF);
}


////////// stuff not actively being used, but kept for posterity


uint8_t tm022hdh26::spiread(void) {
  uint8_t r = 0;

  cs_select(_pin_cs);
  spi_read_blocking(DISPLAY_SPI_CH, 0, &r, 1);
  cs_deselect(_pin_cs);

  return r;
}

uint8_t tm022hdh26::readdata(void) {

  cs_select(_pin_cs);
  gpio_put(_pin_dc, 1);

  uint8_t r = spiread();

  cs_deselect(_pin_cs);
   
  return r;
}

uint8_t tm022hdh26::readcommand8(uint8_t c) {

  cs_deselect(_pin_cs);
  gpio_put(_pin_dc, 0);
   
  spiwrite(c);

  gpio_put(_pin_dc, 1);
  uint8_t r = spiread();
  cs_deselect(_pin_cs);

  return r;
}

void tm022hdh26::set_bright(uint32_t bright) {
  _bright = bright;
  pwm_set_chan_level(slice_num, led_pwm_ch, bright);
}

uint32_t tm022hdh26::get_bright(void) {
  return _bright;
}