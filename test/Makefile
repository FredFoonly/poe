
include ../Makefile.inc

CFLAGS = $(PRJCFLAGS) -I../src/

EXE = ../bin/poetest
_POEOBJS = tabstops.o mark.o markstack.o utils.o trace.o vec.o cstr.o buffer.o margins.o editor_globals.o logging.o poe_err.o poe_exit.o
POEOBJS = ${_POEOBJS:S/^/..\/src\//}
OBJS = test.o testing.o test_tabstops.o test_mark.o test_markstack.o test_vec.o test_cstr.o test_buffer.o 
OBJLIBS = 
LIBS = -L. -lncurses

all: $(EXE)

test: all
	../bin/poetest

$(EXE) : $(OBJS) $(POEOBJS) $(OBJLIBS)
	$(LD) -o $(EXE) $(OBJS) $(POEOBJS) $(LIBS) 

clean :
	$(ECHO) cleaning up in .
	$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	$(RM) -f *.s
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

install :

uninstall :
	$(ECHO) uninstalling poe from $(PREFIX) .
	-$(ECHO) rm $(PREFIX)/bin/poe
	-rm $(PREFIX)/bin/poe

$(POEOBJS) : 
	@(cd ../src; $(MAKE) all)

