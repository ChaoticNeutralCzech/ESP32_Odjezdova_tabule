const uint8_t ElektrocasGeaThinHalf-9_Bitmap[] PROGMEM = {
	// Bitmap Data:
	0b11111001, 0b10011001, 0b10011001, 0b10011001, 0b11110000, // zero
	0b00100110, 0b10100010, 0b00100010, 0b00100010, 0b01110000, // one
	0b11111001, 0b00010010, 0b00100100, 0b01001000, 0b11110000, // two
	0b11111001, 0b00010111, 0b00010001, 0b00011001, 0b11110000, // three
	0b10001001, 0b10011001, 0b10011111, 0b00010001, 0b00010000, // four
	0b11111000, 0b10001111, 0b00010001, 0b00011001, 0b11110000, // five
	0b11111000, 0b10001111, 0b10011001, 0b10011001, 0b11110000, // six
	0b11110001, 0b00010010, 0b00100100, 0b01001000, 0b10000000, // seven
	0b11111001, 0b10011001, 0b11111001, 0b10011001, 0b11110000, // eight
	0b11111001, 0b10011001, 0b10011111, 0b00010001, 0b11110000, // nine
	0b10010000, // colon
	0b00000000, // semicolon
};

const GFXglyphElektrocasGeaThinHalf-9Glyphs[] PROGMEM = {
	// bitmap  wid  hei  x    x    y
  // Offset, th , ght, Adv, Ofs, Ofs
	{     0,   4,   9,   5,   0,  -9,  }, // zero
	{     5,   4,   9,   5,   0,  -9,  }, // one
	{    10,   4,   9,   5,   0,  -9,  }, // two
	{    15,   4,   9,   5,   0,  -9,  }, // three
	{    20,   4,   9,   5,   0,  -9,  }, // four
	{    25,   4,   9,   5,   0,  -9,  }, // five
	{    30,   4,   9,   5,   0,  -9,  }, // six
	{    35,   4,   9,   5,   0,  -9,  }, // seven
	{    40,   4,   9,   5,   0,  -9,  }, // eight
	{    45,   4,   9,   5,   0,  -9,  }, // nine
	{    50,   1,   4,   5,   2,  -6,  }, // colon
	{    51,   1,   1,   5,   4,  -1,  }, // semicolon (space)
};
const GFXfont ElektrocasGeaThinHalf-9 PROGMEM = {
(uint8_t  *)ElektrocasGeaThinHalf-9_Bitmap,(GFXglyph *)ElektrocasGeaThinHalf-9Glyphs, 48, 59, 9};