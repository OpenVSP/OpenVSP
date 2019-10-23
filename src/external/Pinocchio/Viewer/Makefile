#!/usr/bin/make -f

CC= gcc
CXX= g++
RM= rm -f

PKGCONFIG= pkg-config
PACKAGES= glew gl

CFLAGS= -O2 -g -Wall -std=c++17 \
	-fstack-protector-strong \
	-Wall \
	-Wformat \
	-Werror=format-security \
	-Wdate-time \
	-D_FORTIFY_SOURCE=2 \
	$(shell $(PKGCONFIG) --cflags $(PACKAGES))

LDFLAGS= \
	-Wl,-z,defs,-z,relro,-z,now \
	-Wl,--as-needed \
	-Wl,--no-undefined

LIBS= \
	-lm -lglut -L../Pinocchio/ -lpinocchio \
	$(shell $(PKGCONFIG) --libs $(PACKAGES))

SRCS= viewer.cpp model.cpp shader_utils.cpp DefMesh.cpp Motion.cpp MotionFilter.cpp
OBJS= $(subst .cpp,.o,$(SRCS))

BINARY= Viewer

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CFLAGS)

%.o: %.cc
	$(CXX) -o $@ -c $< $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(BINARY)

distclean cleanall: clean
	$(RM) *~ .depend core *.out *.bak *.o

SRC_FILES = $(shell find . -maxdepth 3 \
	-name '*.c' -o -name '*.cpp' -o -name '*.cc' -o \
	-name '*.h' -o -name '*.hpp' -o -name '*.hh')

bcpp:
	for FILE in $(SRC_FILES); do \
		cp -v "$${FILE}" "$${FILE}~" ; \
		bcpp -fnc bcpp.cfg -fi "$${FILE}~" -fo "$${FILE}" ; \
		dos2unix "$${FILE}" ; \
	done

include .depend

.PHONY: all depend clean distclean cleanall

