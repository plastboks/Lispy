CC=cc
TIGCC=tigcc
LFLAGS=-Wall -Iinc -std=c99
LNFLAGS=-Wall -Iinc -W -Os -g
TIFLAGS=-Wall -Iinc -W -Os -Wwrite-strings
EFLAGS=-lm

ODIR=obj

_LSPY = lispy.o
_LN = linenoise.o
_TI = ti.o
_OBJ = func.o mpc.o lenv.o lval.o builtins.o

OBJ_LIB = $(patsubst %,$(ODIR)/%,$(_OBJ))
OBJ_LN = $(patsubst %,$(ODIR)/%,$(_LN))
OBJ_LSPY = $(patsubst %,$(ODIR)/%,$(_LSPY))
OBJ_TI = $(patsubst %,$(ODIR)/%,$(_TI))

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(LFLAGS)

$(ODIR)/%.o: linenoise/%.c $(DEPS)
	$(CC) -c -o $@ $< $(LNFLAGS)

$(ODIR)/%.o: port/%.c $(DEPS)
	$(TIGCC) -c -o $@ $< $(TIFLAGS)

lispy: $(OBJ_LIB) $(OBJ_LN) $(OBJ_LSPY)
	$(CC) -o $@ $^ $(EFLAGS)

ti: $(OBJ_TI)
	$(TIGCC) -o $@ $^

# clean routine
.PHONY: clean

clean:
	rm -f $(ODIR)/*.o lispy ti
