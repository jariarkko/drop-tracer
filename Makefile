
SOURCE_HEADERS	=	phymodel.h \
			util.h
SOURCE_CODE	=	main.c \
			phymodel.c \
			util.c
LIBOBJECTS	=	phymodel.o \
			util.o
CMDOBJECTS	=	main.o
CC		=	gcc
CFLAGS		=	-g -Wall

all:	drop-tracer

drop-tracer:	$(LIBOBJECTS) \
		$(CMDOBJECTS)
	$(CC) $(CFLAGS) -o drop-tracer $(CMDOBJECTS) $(LIBOBJECTS) -lm
