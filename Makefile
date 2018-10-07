
SOURCE_HEADERS	=	image.h \
			phymodel.h \
			util.h
SOURCE_CODE	=	image.c \
			main.c \
			phymodel.c \
			util.c
SOURCE_COMPILE	=	Makefile
SOURCES		=	$(SOURCE_HEADERS) \
			$(SOURCE_CODE) \
			$(SOURCE_COMPILE)
LIBOBJECTS	=	image.o \
			phymodel.o \
			util.o
CMDOBJECTS	=	main.o
CC		=	gcc
CFLAGS		=	-g -Wall `pkg-config --cflags MagickWand`
LDFLAGS		=	`pkg-config --cflags --libs MagickWand`

all:	drop-tracer

drop-tracer:	$(LIBOBJECTS) \
		$(CMDOBJECTS)
	$(CC) -o drop-tracer $(CMDOBJECTS) $(LIBOBJECTS) $(LDFLAGS) -lm

install:	drop-tracer
	cp drop-tracer /usr/sbin/drop-tracer

clean:
	rm -f drop-tracer $(CMDOBJECTS) $(LIBOBJECTS)
	rm -f *~

wc:
	wc -l $(SOURCES)

