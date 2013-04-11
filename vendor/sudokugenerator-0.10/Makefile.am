## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" 

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = sudokugenerator sign_encode

sudokugenerator_SOURCES = \
	main.c sudokumchain.c holepatterns.c generator.h

sudokugenerator_LDFLAGS = -lm

sudokugenerator_LDADD = 

sign_encode_SOURCES = \
	sign_encode.cc

sign_encode_LDFLAGS = -lm

sign_encode_LDADD = 

EXTRA_DIST = scripts

dist-hook:
	rm -rf `find $(distdir)/ -name .svn`

