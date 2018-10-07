
SOURCE_HEADERS	=	phymodel.h \
			util.h
SOURCE_CODE	=	main.c \
			phymodel.c \
			util.c
SOURCE_COMPILE	=	Makefile
SOURCES		=	$(SOURCE_HEADERS) \
			$(SOURCE_CODE) \
			$(SOURCE_COMPILE)
LIBOBJECTS	=	phymodel.o \
			util.o
CMDOBJECTS	=	main.o
CC		=	gcc
CFLAGS		=	-g -Wall

all:	drop-tracer

drop-tracer:	$(LIBOBJECTS) \
		$(CMDOBJECTS)
	$(CC) $(CFLAGS) -o drop-tracer $(CMDOBJECTS) $(LIBOBJECTS) -lm

clean:
	rm -f drop-tracer $(CMDOBJECTS) $(LIBOBJECTS)
	rm -f *~

wc:
	wc -l $(SOURCES)

