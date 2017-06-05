CC = g++
CFLAGS = -std=c++11 
INCLUDES =
LDFLAGS = -lGL -lglut -lGLU
DEBUGF = $(CFLAGS) -ggdb
SOURCES = *.cpp
OUTF = build/

build: $(SOURCES) $(OUTF)sph

$(OUTF)sph:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTF)sph $(SOURCES) $(LDFLAGS)

debug: $(SOURCES) $(OUTF)sph_d

$(OUTF)sph_d:
	$(CC) -o $(OUTF)sph_d $(SOURCES) $(INCLUDES) $(LDFLAGS) $(DEBUGF)

clean: 
	rm build/*

rebuild: clean build