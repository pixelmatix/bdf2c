
My version of BDF fonts convertor to C includes, which can be used to
embed fonts into the executable.

Usage:
	./bdf2c -c

	Print #defines for font files to stdout.

	./bdf2c -C font.h

	Create font.h, which contains #defines for fonts.

	./bdf2c -b < font.bdf > font.c

	Create font.c which contains the converted bdf font.

The C file contains:

	Bitmap data for the characters.
	Character width table for proportional font
	Character codes table for utf-8 font

TODO:
	Proportional fonts (f.e. generated from ttf2bdf) aren't yet supported.
