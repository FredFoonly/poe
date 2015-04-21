include Makefile.inc

DIRS = src test

all : ./bin/poe ./bin/poetest

exe : ./bin/poe

test : ./bin/poetest

./bin/poe: forcelook
	$(ECHO) building poe
	cd src; $(MAKE) all

./bin/poetest: ./bin/poe forcelook
	$(ECHO) running tests .
	-for d in $(DIRS); do (cd $$d; $(MAKE) test ); done

clean :
	$(ECHO) cleaning up in .
	-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

install :
	$(ECHO) installing to $(PREFIX) .
	-for d in $(DIRS); do (cd $$d; $(MAKE) install ); done

uninstall :
	$(ECHO) uninstalling from $(PREFIX) .
	-for d in $(DIRS); do (cd $$d; $(MAKE) uninstall ); done

forcelook :
	@true
