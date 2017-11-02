CC = g++
CFLAGS = -std=c++11 -Ofast
INCLUDES =
LDFLAGS = -lGL -lglut -lGLU
DEBUGF = $(CFLAGS) -ggdb
SOURCES = *.cpp
OUTF = build/
MKDIR_P = mkdir -p

all: build

directory: $(OUTF)

build: directory $(SOURCES) $(OUTF)sph

$(OUTF):
	$(MKDIR_P) $(OUTF)

$(OUTF)sph: $(OUTF)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTF)sph $(SOURCES) $(LDFLAGS)

debug: directory $(SOURCES) $(OUTF)sph_d

$(OUTF)sph_d:
	$(CC) -o $(OUTF)sph_d $(SOURCES) $(INCLUDES) $(LDFLAGS) $(DEBUGF)

clean:
	@[ -f $(OUTF)sph ] && rm $(OUTF)sph || true
	@[ -f $(OUTF)sph_d ] && rm $(OUTF)sph_d || true

rebuild: clean build