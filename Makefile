.PHONY: all clean

CC=mpicc
LIBS=-lm -lX11
SRCDIR=src/
BINDIR=bin/

SRCFILES= $(wildcard src/*.c)
OBJFILES= $(patsubst src/%.c, %.o, $(SRCFILES))
_PROGS= $(patsubst src/%.c, %, $(SRCFILES))
PROGFILES=$(addprefix $(BINDIR),$(_PROGS))

all: $(PROGFILES)

$(BINDIR)%: $(SRCDIR)%.c
	@mkdir -p $(BINDIR)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

debug:
	$(CC) $(INC) src/$(DEBUG_PROG).c $(CFLAGS) -o bin/$(DEBUG_PROG) $(LIBS) -g

clean:
	rm -rf $(BINDIR)