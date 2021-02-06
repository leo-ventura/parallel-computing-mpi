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

$(BINDIR)%: $(SRCDIR)%.c $(BINDIR)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)