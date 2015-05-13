//
// Copyright (c) 2015, RF Digital Corp.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must on an RF Digital part and reproduce the 
//       above copyright notice, this list of conditions and the following disclaimer 
//       in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// Very simple library for driving an OLED display connected via SSD1306 driver chip.
// One such display is: http://www.amazon.com/gp/product/B00O2KDQBE/ref=s9_simh_gw_p147_d0_i2?pf_rd_m=ATVPDKIKX0DER&pf_rd_s=desktop-1&pf_rd_r=1ABTWJR1SYABM57H2H7X&pf_rd_t=36701&pf_rd_p=1970559082&pf_rd_i=desktop
// but there are many examples on Amazon and eBay such as http://www.ebay.com/itm/170842973484
//
// The code keeps a buffer of the display memory in PROGMEM. Each pixel on the display is one bit
// in the buffer which can be "on" or "off" (1 or 0). "Drawing" on the display consists
// of turning the appropriate bits on and off in the buffer, and then when ready, copying the entire
// buffer to the display. You may wish to store an initial image or "splash screen" in the buffer
// to simplify startup.
//
// Adafruit makes a complete line of similar displays and has a richer library available
// if more functionality is required. See: http://www.adafruit.com/category/63_98
//
// On RFduino I2C defaults to pins:
//	  SCL: GPIO 5
//	  SDA: GPIO 6
//
#ifndef __OLEDDisplay_H__
#define __OLEDDisplay_H__

#include "Arduino.h"

#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 64

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define swap(a, b) { int16_t t = a; a = b; b = t; }

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

class OLEDDisplay : public Print
{

public:
	OLEDDisplay(uint8_t address);
	void update(void);
	void on(void);
	void off(void);
	void clear(void);
	void fillScreen(uint16_t color);
	void setLocation(uint x, uint y);
	void setTextSize(uint size);

	void drawPixel(int16_t x, int16_t y, uint16_t color);
	void drawLine(int16_t x0, int16_t y0,
		int16_t x1, int16_t y1,
		uint16_t color);

	void drawFastVLine(int16_t x, int16_t y,
		int16_t h, uint16_t color);

	void drawFastHLine(int16_t x, int16_t y,
		int16_t w, uint16_t color);

	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
		uint16_t color);

	void drawChar(int16_t x, int16_t y, unsigned char c,
		uint16_t color, uint16_t bg, uint8_t size);
	void drawCharFont2(int16_t x, int16_t y, unsigned char c,
		uint16_t color, uint16_t bg, uint8_t size);
	virtual size_t write(uint8_t);

	void drawBitmap(uint x, uint y,
		const uint8_t *bitmap, uint width, uint height,
		uint16_t color, uint16_t bg);

protected:
	// The coordinates of where to draw next
	uint _x;
	uint _y;
	uint _textSize;
	bool _wordWrap;

	uint getCharacterWidth();
	uint getCharacterHeight();

private:
	// The I2C bus address for the display
	uint8_t _address;

	// Write a command to the display
	void writeCommand(uint8_t command);
};

#endif
