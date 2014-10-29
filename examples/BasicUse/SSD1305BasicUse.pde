#include <SPI.h>
#include <SSD1305.h>

SSD1305 *oled;

void setup() {
  // Chip select, data, and reset pins
  oled = new SSD1305(4, 3, 2);
  oled->drawLine(0, 0, 128, 32);
  oled->draw();
}

void loop() {

}