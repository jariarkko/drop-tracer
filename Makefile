
SOURCE_HEADERS	=	image.h \
			phymodel.h \
			simul.h \
			util.h
SOURCE_CODE	=	image.c \
			main.c \
			phyatom.c \
			phymodel.c \
			simul.c \
			util.c
SOURCE_COMPILE	=	Makefile
SOURCES		=	$(SOURCE_HEADERS) \
			$(SOURCE_CODE) \
			$(SOURCE_COMPILE)
LIBOBJECTS	=	image.o \
			phyatom.o \
			phymodel.o \
			simul.o \
			util.o
CMDOBJECTS	=	main.o
TESTOBJECTS	=	test.o
CC		=	gcc
CFLAGS		=	-g -Wall `pkg-config --cflags MagickWand`
LDFLAGS		=	`pkg-config --cflags --libs MagickWand`

all:	drop-tracer runtest

$(LIBOBJECTS) $(CMDOBJECTS): $(SOURCE_HEADERS)

drop-tracer:	$(LIBOBJECTS) \
		$(CMDOBJECTS)
	$(CC) -o drop-tracer $(CMDOBJECTS) $(LIBOBJECTS) $(LDFLAGS) -lm

test-tracer:	$(LIBOBJECTS) \
		$(TESTOBJECTS)
	$(CC) -o test-tracer $(TESTOBJECTS) $(LIBOBJECTS) $(LDFLAGS) -lm

runtest:	test-tracer
	./test-tracer

install:	drop-tracer
	cp drop-tracer /usr/sbin/drop-tracer

clean:
	rm -f drop-tracer $(CMDOBJECTS) $(LIBOBJECTS)
	rm -f *~

wc:
	wc -l $(SOURCES)

