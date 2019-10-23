#!/usr/bin/make -f

dirs = Pinocchio AttachWeights DemoUI

# Define a standard makePerDir command, which goes into
# each dir, and runs make $(makerule)
define makePerDir
for dir in $(dirs); \
do \
	cd $$dir && { $(MAKE) $(makerule); cd ..; }; \
done
endef

nullstring :=

all: makerule = $(nullstring)
depend clean distclean cleanall: makerule = distclean

all depend clean:
	$(makePerDir)

distclean cleanall:
	$(makePerDir)
	rm -f *.o */*.o *.a */*.a *~ */*~

SRC_FILES = $(shell find . -maxdepth 3 \
	-name '*.c' -o -name '*.cpp' -o -name '*.cc' -o \
	-name '*.h' -o -name '*.hpp' -o -name '*.hh')

bcpp:
	for FILE in $(SRC_FILES); do \
		cp -v "$${FILE}" "$${FILE}~" ; \
		bcpp -fnc bcpp.cfg -fi "$${FILE}~" -fo "$${FILE}" ; \
		dos2unix "$${FILE}" ; \
	done

test:all
	LD_LIBRARY_PATH=Pinocchio DemoUI/DemoUI DemoUI/data/test.obj -motion DemoUI/data/walk.txt -algo DQS

.PHONY: all depend clean distclean cleanall bcpp test
