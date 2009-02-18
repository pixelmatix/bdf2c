///
///	@file bdf2c.c		 BDF Font to C source convertor
///
///	Copyright (c) 2009 by Johns.  All Rights Reserved.
///
///	Contributor(s): 
///
///	License: AGPLv3
///
///	This program is free software: you can redistribute it and/or modify
///	it under the terms of the GNU Affero General Public License as
///	published by the Free Software Foundation, either version 3 of the
///	License.
///
///	This program is distributed in the hope that it will be useful,
///	but WITHOUT ANY WARRANTY; without even the implied warranty of
///	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///	GNU Affero General Public License for more details.
///
///	$Id$
//////////////////////////////////////////////////////////////////////////////

///
///	@mainpage
///		bdf2c - converts bdf font files into C include files.
///
///		The Bitmap Distribution Format (BDF) is a file format for
///		storing bitmap fonts. The content is presented as a text file
///		that is intended to be human and computer readable.
///

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define VERSION "1"

//////////////////////////////////////////////////////////////////////////////

//
//	Create our header file.
//
void CreateFontHeaderFile(FILE * out)
{
    register int i;

    fprintf(out, "// (c) 2009 Johns, License: AGPLv3\n\n");
    fprintf(out,
	"\t/// bitmap font structure\n" "struct bitmap_font {\n"
	"\tunsigned char Width;\t\t///< max. character width\n"
	"\tunsigned char Height;\t\t///< character height\n"
	"\tunsigned short Chars;\t\t///< number of characters in font\n"
	"\tconst unsigned char *Widths;\t///< width of each character\n"
	"\tconst unsigned short *Index;\t///< encoding to character index\n"
	"\tconst unsigned char *Bitmap;\t///< bitmap of each character\n"
	"};\n\n");

    fprintf(out, "\t/// @{ defines to have human readable font files\n");
    for (i = 0; i < 256; ++i) {
	fprintf(out, "#define %c%c%c%c%c%c%c%c 0x%02X\n",
	    (i & 0x80) ? 'X' : '_', (i & 0x40) ? 'X' : '_',
	    (i & 0x20) ? 'X' : '_', (i & 0x10) ? 'X' : '_',
	    (i & 0x08) ? 'X' : '_', (i & 0x04) ? 'X' : '_',
	    (i & 0x02) ? 'X' : '_', (i & 0x01) ? 'X' : '_', i);
    }
    fprintf(out, "\t/// @}\n");
}

//////////////////////////////////////////////////////////////////////////////

//
//	Print header for c file.
//
void Header(FILE * out, const char *name)
{
    fprintf(out,
	"// Created from bdf2c Version %s, (c) 2009 by Johns\n"
	"//\tLicense AGPLv3: GNU Affero General Public License version 3\n"
	"\n#include \"font.h\"\n\n", VERSION);

    fprintf(out,
	"\t/// character bitmap for each encoding\n"
	"static const unsigned char __%s_bitmap__[] = {\n", name);
}

//
//	Print width table for c file
//
void WidthTable(FILE * out, const char *name, const unsigned *width_table,
    int chars)
{
    fprintf(out, "};\n\n");

    fprintf(out,
	"\t/// character width for each encoding\n"
	"static const unsigned char __%s_widths__[] = {\n", name);
    while (chars--) {
	printf("\t%u,\n", *width_table++);
    }
}

//
//	Print encoding table for c file
//
void EncodingTable(FILE * out, const char *name,
    const unsigned *encoding_table, int chars)
{
    fprintf(out, "};\n\n");

    fprintf(out,
	"\t/// character encoding for each index entry\n"
	"static const unsigned short __%s_index__[] = {\n", name);
    while (chars--) {
	printf("\t%u,\n", *encoding_table++);
    }
}

//
//	Print footer for c file.
//
void Footer(FILE * out, const char *name, int width, int height, int chars)
{
    fprintf(out, "};\n\n");
    fprintf(out,
	"\t/// bitmap font structure\n" "const struct bitmap_font %s = {\n",
	name);
    fprintf(out, "\t.Width = %d, .Height = %d,\n", width, height);
    fprintf(out, "\t.Chars = %d,\n", chars);
    fprintf(out, "\t.Widths = __%s_widths__,\n", name);
    fprintf(out, "\t.Index = __%s_index__,\n", name);
    fprintf(out, "\t.Bitmap = __%s_bitmap__,\n", name);
    fprintf(out, "};\n\n");
}

//
//	Read BDF font file.
//
void ReadBdf(FILE * bdf, const char *name)
{
    char linebuf[1024];
    char *s;
    char *p;
    int fontboundingbox_width;
    int fontboundingbox_height;
    int chars;
    int i;
    int n;
    int scanline;
    char charname[1024];
    int encoding;
    int bbx;
    int bby;
    int bbw;
    int bbh;
    int width;
    unsigned *width_table;
    unsigned *encoding_table;

    fontboundingbox_width = 0;
    fontboundingbox_height = 0;
    chars = 0;
    for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
	    break;
	}
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
	    break;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "FONTBOUNDINGBOX")) {
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_width = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_height = atoi(p);
	} else if (!strcasecmp(s, "CHARS")) {
	    p = strtok(NULL, " \t\n\r");
	    chars = atoi(p);
	    break;
	}
    }
    /*
       printf("%d * %dx%d\n", chars, fontboundingbox_width,
       fontboundingbox_height);
     */

    if (chars <= 0) {
	fprintf(stderr, "Need to know the number of characters\n");
	exit(-1);
    }
    width_table = malloc(chars * sizeof(*width_table));
    if (!width_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }
    encoding_table = malloc(chars * sizeof(*encoding_table));
    if (!encoding_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }

    Header(stdout, name);

    scanline = -1;
    n = 0;
    encoding = -1;
    bbx = 0;
    bby = 0;
    bbw = 0;
    bbh = 0;
    width = INT_MIN;
    strcpy(charname, "unknown character");
    for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
	    break;
	}
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
	    break;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "STARTCHAR")) {
	    p = strtok(NULL, " \t\n\r");
	    strcpy(charname, p);
	} else if (!strcasecmp(s, "ENCODING")) {
	    p = strtok(NULL, " \t\n\r");
	    encoding = atoi(p);
	} else if (!strcasecmp(s, "DWIDTH")) {
	    p = strtok(NULL, " \t\n\r");
	    width = atoi(p);
	} else if (!strcasecmp(s, "BBX")) {
	    p = strtok(NULL, " \t\n\r");
	    bbw = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bbh = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bbx = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bby = atoi(p);
	} else if (!strcasecmp(s, "BITMAP")) {
	    fprintf(stdout, "// %3d $%02x '%s'\n", encoding, encoding,
		charname);
	    fprintf(stdout, "//\twidth %d, bbx %d, bby %d, bbw %d, bbh %d\n",
		width, bbx, bby, bbw, bbh);

	    if (n == chars) {
		fprintf(stderr, "Too many bitmaps for characters\n");
		exit(-1);
	    }
	    width_table[n] = width;
	    encoding_table[n] = encoding;
	    ++n;
	    scanline = 0;
	} else if (!strcasecmp(s, "ENDCHAR")) {
	    scanline = -1;
	} else {
	    if (scanline >= 0) {
		p = s;
		fprintf(stdout, "\t");
		while (*p) {
		    if (*p <= '9') {
			i = *p - '0';
		    } else if (*p <= 'F') {
			i = *p - 'A' + 10;
		    } else {
			i = *p - 'a' + 10;
		    }
		    fprintf(stdout, "%c%c%c%c", (i & 0x08) ? 'X' : '_',
			(i & 0x04) ? 'X' : '_', (i & 0x02) ? 'X' : '_',
			(i & 0x01) ? 'X' : '_');
		    ++p;
		    if (~(p - s) & 1) {
			fprintf(stdout, ",");
		    }
		}
		if ((p - s) & 1) {	// fill last nibble
		    fprintf(stdout, "____,");
		}
		fprintf(stdout, "\n");
		++scanline;
	    }
	}
    }

    // Output width table for proportional font.
    WidthTable(stdout, name, width_table, chars);
    // Output encoding table for utf-8 support
    EncodingTable(stdout, name, encoding_table, chars);

    Footer(stdout, name, fontboundingbox_width, fontboundingbox_height, chars);
}

//////////////////////////////////////////////////////////////////////////////

//
//	Print version
//
void PrintVersion(void)
{
    printf("bdf2c Version %s, (c) 2009 by Johns\n"
	"\tLicense AGPLv3: GNU Affero General Public License version 3\n",
	VERSION);
}

//
//	Print usage
//
void PrintUsage(void)
{
    printf("Usage: bdf2c [OPTIONs]\n" "\t-b\tRead bdf file from stdin\n"
	"\t-c\tCreate font header on stdout\n"
	"\t-C file\tCreate font header file\n"
	"\t-n name\tName of c font variable (place it before -b)\n");
    printf("\tOnly idiots print usage on stderr\n");
}

//
//	Main test program for bdf2c.
//
int main(int argc, char *const argv[])
{
    const char *name;

    name = "font";
    //
    //	Parse arguments.
    //
    for (;;) {
	switch (getopt(argc, argv, "bcC:n:h?-")) {
	    case 'b':
		ReadBdf(stdin, name);
		continue;
	    case 'c':
		CreateFontHeaderFile(stdout);
		break;
	    case 'C':
	    {
		FILE *out;

		if (!(out = fopen(optarg, "w"))) {
		    fprintf(stderr, "Can't open file '%s': %s\n", optarg,
			strerror(errno));
		    exit(-1);
		}
		CreateFontHeaderFile(out);
		fclose(out);
	    }
		continue;
	    case 'n':
		name = optarg;
		continue;

	    case EOF:
		break;
	    case '?':
	    case 'h':			// help usage
		PrintVersion();
		PrintUsage();
		exit(0);
	    case '-':
		fprintf(stderr, "We need no long options\n");
		PrintUsage();
		exit(-1);
	    case ':':
		PrintVersion();
		fprintf(stderr, "Missing argument for option '%c'\n", optopt);
		exit(-1);
	    default:
		PrintVersion();
		fprintf(stderr, "Unkown option '%c'\n", optopt);
		exit(-1);
	}
	break;
    }
    while (optind < argc) {
	fprintf(stderr, "Unhandled argument '%s'\n", argv[optind++]);
    }

    return 0;
}
