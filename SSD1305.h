#ifndef SSD1305_h
#define SSD1305_h

#include "Arduino.h"

/**
 * SSD1305 contains basic utilities for interacting with the SSD1305 display
 * controller. This class currently only works on Arduino Due, because of the
 * SPI library it references.
 */
class SSD1305 {
  private:
    static const int junk_buffer    =  532;  // buffers beyond display
    static const int blank          =    4;  // Offscreen buffers
    static const int pix_in_page    =    8;  // Pixels in a buffer page
    static const int total_buffers  = 1056;

    static const int numBuffers     =  512;  // visible buffers
                                             // (width * height) / pix_in_page

    int cs;        // chip select pin
    int dc;        //data/command pin
    int reset;     // reset pin

    uint8_t buffer[numBuffers];
    int buffer_changed;

  public:
    static const int width          =  128;
    static const int height         =   32;

    SSD1305(int, int, int);
    ~SSD1305();

    inline void setPixel(int x, int y, int val);
    void draw();
    void clear();
    void drawLine(int, int, int, int);
    void drawCircle(int, int, int);
};

#endif