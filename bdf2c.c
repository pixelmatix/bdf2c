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
///	$Id: $
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define VERSION "1"

//////////////////////////////////////////////////////////////////////////////

//
//	Create our header file.
//
void CreateFontHeaderFile(FILE * out)
{
    register int i;

    fprintf(out, "// (c) 2009 Johns, License: AGPLv3\n");
    for (i = 0; i < 256; ++i) {
	fprintf(out, "#define %c%c%c%c%c%c%c%c 0x%02X\n",
	    (i & 0x80) ? 'X' : '_', (i & 0x40) ? 'X' : '_',
	    (i & 0x20) ? 'X' : '_', (i & 0x10) ? 'X' : '_',
	    (i & 0x08) ? 'X' : '_', (i & 0x04) ? 'X' : '_',
	    (i & 0x02) ? 'X' : '_', (i & 0x01) ? 'X' : '_', i);
    }
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
    printf("Usage: bdf2c [OPTIONs]\n");
    printf("\tOnly idiots print usage on stderr\n");
}

//
//	Main test program for bdf2c.
//
int main(int argc, char *const argv[])
{
    //
    //	Parse arguments.
    //
    for (;;) {
	switch (getopt(argc, argv, "cC:h?-")) {
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
		break;

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
