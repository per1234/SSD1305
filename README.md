#SSD1305 Library#

SSD1305 allows you to interface with SSD1305-based displays via Arduino Due's SPI port. Sorry, this is Due-only, although you could use this as a base to make one work on your Uno or Mega ;)

You must include &lt;SPI.h&gt; in your sketch file. For optimum performance, attach a timer interrupt
to a function that calls clear() and draw() in it.

SSD1305 was created by Michael Hawthorne (Jigglebizz)