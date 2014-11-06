#include "Arduino.h"
#include "SSD1305.h"

#include <SPI.h>

/**
 * SSD1305::SSD1305 - Initializes the SSD1305. Takes care of initial settings 
 *                    on the chip and
 * sets up SPI bus.
 * @cs      - chip select pin
 * @dc      - data/command pin
 * @reset   - reset pin
 */
SSD1305::SSD1305(int cs, int dc, int reset) {
    this->cs    = cs;
    this->dc    = dc;
    this->reset = reset;

    pinMode(dc, OUTPUT);
    pinMode(reset, OUTPUT);

    // Initialize the buffer for use
    for (int i = 0; i < numBuffers; i++) {
        buffer[i] = 0;
    }
    buffer_changed = 0;

    // Initialize the display
    SPI.begin(cs);
    SPI.setClockDivider(cs, 42);

    digitalWrite(reset, 0);
    delay(1);
    digitalWrite(reset, 1);
    digitalWrite(dc,    0);
    SPI.transfer(cs, 0xAE, SPI_CONTINUE);
    SPI.transfer(cs, 0xD5, SPI_CONTINUE);  // set display clock divider ratio
    SPI.transfer(cs, 0xA0, SPI_CONTINUE);
    SPI.transfer(cs, 0xA8, SPI_CONTINUE);  // set multiplex ratio
    SPI.transfer(cs, 0x3F, SPI_CONTINUE);
    SPI.transfer(cs, 0xD3, SPI_CONTINUE);  // set display offset
    SPI.transfer(cs, 0x00, SPI_CONTINUE);
    SPI.transfer(cs, 0x40, SPI_CONTINUE);  // set display start line
    SPI.transfer(cs, 0xAD, SPI_CONTINUE);  // set master configuration
    SPI.transfer(cs, 0x8E, SPI_CONTINUE);
    SPI.transfer(cs, 0xD8, SPI_CONTINUE);  // Set area color mode
    SPI.transfer(cs, 0x05, SPI_CONTINUE);
    SPI.transfer(cs, 0xA1, SPI_CONTINUE);  // Set segment re-map
    SPI.transfer(cs, 0xC8, SPI_CONTINUE);  // Set com output scan direction
    SPI.transfer(cs, 0xDA, SPI_CONTINUE);  // Set com pins hardware configuration
    SPI.transfer(cs, 0x12, SPI_CONTINUE);
    SPI.transfer(cs, 0x91, SPI_CONTINUE);  // Set lookup table
    SPI.transfer(cs, 0x3F, SPI_CONTINUE);
    SPI.transfer(cs, 0x3F, SPI_CONTINUE);
    SPI.transfer(cs, 0x3F, SPI_CONTINUE);
    SPI.transfer(cs, 0x3F, SPI_CONTINUE);
    SPI.transfer(cs, 0x81, SPI_CONTINUE);  // Set current control for bank 0
    SPI.transfer(cs, 0x8F, SPI_CONTINUE);
    SPI.transfer(cs, 0xD9, SPI_CONTINUE);  // Set pre-charge period
    SPI.transfer(cs, 0xD2, SPI_CONTINUE); 
    SPI.transfer(cs, 0xDB, SPI_CONTINUE);  // Set vcomh deselect level
    SPI.transfer(cs, 0x34, SPI_CONTINUE);
    SPI.transfer(cs, 0xA4, SPI_CONTINUE);  // Set entire display on/off
    SPI.transfer(cs, 0xA6, SPI_CONTINUE);  // set normal/inverse display
    SPI.transfer(cs, 0x20, SPI_CONTINUE);  // Page mode
    SPI.transfer(cs, 0X00, SPI_CONTINUE);

    // Clear content
    digitalWrite(dc, 1);

    for (int i = 0; i < total_buffers; i++) {
        SPI.transfer(cs, 0x00, SPI_CONTINUE);
    }

    digitalWrite(dc, 0);
    SPI.transfer(cs, 0xAF, SPI_LAST);
}

/**
 * SSD1305::~SSD1305 - Destroys the SSD1305. Really, does nothing.
 */
SSD1305::~SSD1305() {
    // no code here
}

/**
 * SSD1305::setPixel - sets a pixel on the display. Inline to save on
 *                     overhead on calling subroutines
 * @x       - X position of the pixel
 * @y       - Y position of the pixel
 * @val     - value of pixel (1 or 0)
 */
inline 
void SSD1305::setPixel(int x, int y, int val) {
    if (val)
        buffer[x + (width * (y / pix_in_page)) ] |= val << (y % pix_in_page);
    else
        buffer[x + (width * (y / pix_in_page)) ] &= 0xFF & (val << (y % pix_in_page));
}

/**
 * SSD1305::draw - Draws the screen. Will do nothing if nothing has changed on
 *                 the display.
 */
void SSD1305::draw() {
    if (buffer_changed) {
        digitalWrite(dc, 1);
        // transfer color data (we don't use this)
        for (int i = 0; i < junk_buffer; i++) {
            SPI.transfer(cs, 0, SPI_CONTINUE);
        }

        for (int i = 0; i < numBuffers; i++) {
            SPI.transfer(cs, buffer[i], (i == numBuffers - 1) ? SPI_LAST : SPI_CONTINUE);

            // Transfer the blank buffers
            if (i % width == width - 1 && i != numBuffers - 1) {
                for (int j = 0; j < blank; j++) {
                    SPI.transfer(cs, 255, SPI_CONTINUE);
                }
            }
        }
        buffer_changed = 0;
    }
}

/**
 * SSD1305::clear - Clears the display
 */
void SSD1305::clear() {
    for (int i = 0; i < numBuffers; i++) {
        buffer[i] = 0;
    }
    buffer_changed = 1;
}

/**
 * SSD1305::drawLine - draws a line using Bresenham's line drawing algorithm.
 * @x1      - First x coordinate
 * @y1      - First y coordinate
 * @x2      - Second x coordinate
 * @y2      - Second y coordinate
 */
void SSD1305::drawLine(int x1, int y1, int x2, int y2) {
    int dy = y2 - y1;
    int dx  = x2 - x1;
    
    if (dx == 0) {
        if (y2 < y1) {
            int tmp = y1;
            y1 = y2;
            y2 = tmp;
        } 
        for (int y  = y1; y < y2 + 1; y++) {
            setPixel(x1, y, 1); 
        }
    } else {
        float m = (float)dy / dx;
        float adjust = (m >= 0) ? 1 : -1;
        float offset = 0;
        float threshold = 0.5;
        
        if (m <= 1 and m >= -1) {
            float delta = abs(m); 
            int y = y1;
            if (x2 < x1) {
                int tmp = x1;
                x1 = x2;
                x2 = tmp;
                y = y2;
            }
            for (int x = x1; x < x2 + 1; x++) {
                setPixel(x, y, 1);
                offset += delta;
                if (offset >= threshold) {
                    y += adjust;
                    threshold += 1;
                }
            } 
        } 
        else {
            float delta = abs((float)dx / dy);
            int x = x1;
            if (y2 < y1) {
                int tmp = y1;
                y1 = y2;
                y2 = tmp;
                x = x2;
            }
            for (int y = y1; y < y2 + 1; y++) {
                setPixel(x, y, 1);
                offset += delta;
                if (offset >= threshold) {
                    x += adjust;
                   threshold += 1; 
                }
            }
        }
    }
    
    buffer_changed = 1;
}

/**
 * SSD1305::drawCircle - Draws a circle using Bresenham's circle algorithm
 * @x0      - Center of circle's X coordinate
 * @y0      - Center of circle's Y coordinate
 * @radius  - Radius of the circle in pixels
 */
void SSD1305::drawCircle(int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;
    
    while (x >= y) {
        setPixel( x + x0,  y + y0, 1);
        setPixel( y + x0,  x + y0, 1);
        setPixel(-x + x0,  y + y0, 1);
        setPixel(-y + x0,  x + y0, 1);
        setPixel(-x + x0, -y + y0, 1);
        setPixel(-y + x0, -x + y0, 1);
        setPixel( x + x0, -y + y0, 1);
        setPixel( y + x0, -x + y0, 1);
    
        y++;
        
        if (radiusError < 0) {
            radiusError += 2 * y + 1; 
        }
        else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
    buffer_changed = 1;
}