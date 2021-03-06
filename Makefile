#
# ##########################################################################
# ############################                     #########################
# ###########################  D R O P T R A C E R  ########################
# ############################                     #########################
# ##########################################################################
# #######      #####    #####  ##    #############      ###    ####  #######
# ####           ###      ###   ##      #########        ###     ##     ####
# #               #        #    ##        #####          ##      ##        #
#                          #     O         ###           ##       #
#                          o               ###           ##       #
#                                           #             #
#                                           o             #
#                                                         o
#                                           o
#
#
#                          Cave Forms Simulation Software
#                                Jari Arkko, 2018
#
#                      https://github.com/jariarkko/drop-tracer
#                              License: BSD 3-Clause
#
#

SOURCE_HEADERS	=	image.h \
			phymodel.h \
			rock.h \
			coords.h \
			drop.h \
			droptable.h \
			simul.h \
			util.h
SOURCE_CODE	=	image.c \
			main.c \
			phyatom.c \
			phymodel.c \
			rockcave.c \
			rockcrack.c \
			rockutil.c \
			coords.c \
			drop.c \
			droptable.c \
			simul.c \
			util.c
SOURCE_COMPILE	=	Makefile
SOURCES		=	$(SOURCE_HEADERS) \
			$(SOURCE_CODE) \
			$(SOURCE_COMPILE)
LIBOBJECTS	=	image.o \
			phyatom.o \
			phymodel.o \
			rockcave.o \
			rockcrack.o \
			rockutil.o \
			coords.o \
			drop.o \
			droptable.o \
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

rockcave.o:	rockcave.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

rockcrack.o:	rockcrack.c $(SOURCE_HEADERS)
	$(CC) -c $(CFLAGS) $<

rockutil.o:	rockutil.c $(SOURCE_HEADERS)
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
BASETESTSETTINGSSIMULATOR=	--xsize 32 --ysize 5 --zsize 32
BASETESTSETTINGSLARGE	=	--xsize 512 --ysize 512 --zsize 512
BASETESTSETTINGSLARGEWIDE=	--xsize 1024 --ysize 512 --zsize 512

runtest:	test-tracer rununittest runbasiccreationtest runbasicsimulationtest runlargecreationtest

rununittest:	test-tracer
	./test-tracer

runbasiccreationtest:	drop-tracer
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
	./drop-tracer --create-rock --simple-crack --cave \
		      $(BASETESTSETTINGS) \
		      --crack-width 10 --uniform --output test4.mod
	./drop-tracer --image --input test4.mod --output test4.z.jpg
	./drop-tracer --image --imagey 32 --input test4.mod --output test4.y.jpg
	./drop-tracer --image --imagex 32 --input test4.mod --output test4.x.jpg

runbasicsimulationtest:	drop-tracer
	./drop-tracer --create-rock --simple-crack --cave \
		      $(BASETESTSETTINGSSIMULATOR) \
		      --seed 1000 \
		      --crack-width 10 --non-uniform --output test7.mod
	./drop-tracer --image --input test7.mod --output test7.z.jpg
	./drop-tracer --image --imagey 2 --input test7.mod --output test7.y.jpg
	./drop-tracer --simulate \
		      --rounds 1 --drop-frequency 1 --drop-size 10 \
		      --seed 3008 \
		      --input test7.mod --output test7s1.mod
	./drop-tracer --image --imagey 2 --input test7s1.mod --output test7s1.y.jpg
	./drop-tracer --image --imagey 2 --input test7s1.mod --output test7s1.y.txt
	./drop-tracer --simulate \
		      --seed 3008 \
		      --rounds 10 --drop-frequency 10 --drop-size 10 --progress-images test7s2.progress.%.jpg \
		      --input test7.mod --output test7s2.mod
	./drop-tracer --image --imagey 2 --input test7s2.mod --output test7s2.y.jpg
	./drop-tracer --image --imagey 2 --input test7s2.mod --output test7s2.y.txt
	./drop-tracer --simulate \
		      --rounds 10 --drop-frequency 1 --drop-size 10 \
		      --seed 3008 \
		      --input test7.mod --output test7s3.mod
	./drop-tracer --image --imagey 2 --input test7s3.mod --output test7s3.y.jpg
	./drop-tracer --image --imagey 2 --input test7s3.mod --output test7s3.y.txt

runlargecreationtest:	drop-tracer
	./drop-tracer --create-rock --fractal-crack --cave \
		      $(BASETESTSETTINGSLARGE) \
		      --crack-width 10 --non-uniform --output test5.mod
	./drop-tracer --image --input test5.mod --output test5.z.jpg
	./drop-tracer --image --imagey 256 --input test5.mod --output test5.y.jpg
	./drop-tracer --image --imagex 256 --input test5.mod --output test5.x.jpg
	./drop-tracer --create-rock --fractal-crack --cave \
		      $(BASETESTSETTINGSLARGEWIDE) \
		      --crack-width 10 --non-uniform --output test6.mod
	./drop-tracer --image --input test6.mod --output test6.z.jpg
	./drop-tracer --image --imagey 32 --input test6.mod --output test6.y.jpg
	./drop-tracer --image --imagex 32 --input test6.mod --output test6.x.jpg

install:	drop-tracer
	cp drop-tracer /usr/sbin/drop-tracer

clean:
	rm -f drop-tracer $(CMDOBJECTS) $(LIBOBJECTS)
	rm -f *~
	rm -f debug.*
	rm -f test.mod
	rm -f test.jpg
	rm -f test.tmp
	rm -f test[1-9]*.mod
	rm -f test[1-9]*.jpg

wc:
	wc -l $(SOURCES)

