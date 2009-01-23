#
#	@file Makefile		bdf2c
#
#	Copyright (c) 2009 by Johns.  All Rights Reserved.
#
#	Contributor(s): 
#
#	License: AGPLv3
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU Affero General Public License as
#	published by the Free Software Foundation, either version 3 of the
#	License.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU Affero General Public License for more details.
#
#	$Id: $

CC	=	gcc
LIBS	=	
CFLAGS	=	-Os -Werror -W -Wall
LDFLAGS	=

OBJS	=	bdf2c.o
HDRS	=	
MISC	=	font.h Makefile agpl-3.0.txt readme.txt

all:	bdf2c

$(OBJS):	$(HDRS)

utv:	$(OBJS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LIBS) 

clean:
	-rm *.o *~

#----------------------------------------------------------------------------
#	Developer tools

indent:
	for i in $(OBJS:.o=.c) $(HDRS); do \
		indent $$i; unexpand -a $$i > $$i.up; mv $$i.up $$i; \
	done

commit:
	git commit $(OBJS:.o=.c) $(HDRS) $(MISC)

