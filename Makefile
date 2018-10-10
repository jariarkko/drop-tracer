
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
CFLAGS		=	-g -Wall -Wpedantic
CFLAGS_IMG	=	$(CFLAGS) `pkg-config --cflags MagickWand`
LDFLAGS		=	
LDFLAGS_IMG	=	`pkg-config --cflags --libs MagickWand`

all:	drop-tracer runtest

$(LIBOBJECTS) $(CMDOBJECTS): $(SOURCE_HEADERS)

image.o:	image.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS_IMG) $<

main.o:		main.c 	$(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

phyatom.o:	phyatom.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

phymodel.o:	phymodel.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

simul.o:	simul.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

test.o:	test.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

util.o:	util.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

drop-tracer:	$(LIBOBJECTS) \
		$(CMDOBJECTS)
	$(CC) -o drop-tracer $(CMDOBJECTS) $(LIBOBJECTS) $(LDFLAGS_IMG) -lm

test-tracer:	$(LIBOBJECTS) \
		$(TESTOBJECTS)
	$(CC) -o test-tracer $(TESTOBJECTS) $(LIBOBJECTS) $(LDFLAGS_IMG) -lm

BASETESTSETTINGS	=	--xsize 64 --ysize 64 --zsize 64

runtest:	test-tracer
	./test-tracer
	./drop-tracer --create-rock --simple-crack \
		      $(BASETESTSETTINGS) \
		      --crack-width 10 --uniform --output test1.mod
	./drop-tracer --image --input test1.mod --output test1.z.jpg
	./drop-tracer --image --imagey 32 --input test1.mod --output test1.y.jpg
	./drop-tracer --create-rock --simple-crack \
		      $(BASETESTSETTINGS) \
		      --crack-width 10 --non-uniform --output test2.mod
	./drop-tracer --image --input test2.mod --output test2.z.jpg
	./drop-tracer --image --imagey 32 --input test2.mod --output test2.y.jpg
	./drop-tracer --create-rock --fractal-crack \
		      $(BASETESTSETTINGS) \
		      --crack-width 10 --fractal-shrink 0.7 --fractal-levels 3 --fractal-cardinality 8 \
		      --output test3.mod
	./drop-tracer --image --input test3.mod --output test3.z.jpg
	./drop-tracer --image --imagey 32 --input test3.mod --output test3.y.jpg

install:	drop-tracer
	cp drop-tracer /usr/sbin/drop-tracer

clean:
	rm -f drop-tracer $(CMDOBJECTS) $(LIBOBJECTS)
	rm -f *~
	rm -f test.mod
	rm -f test.jpg
	rm -f test1*.mod
	rm -f test1*.jpg
	rm -f test2*.mod
	rm -f test2*.jpg
	rm -f test3*.mod
	rm -f test3*.jpg

wc:
	wc -l $(SOURCES)

