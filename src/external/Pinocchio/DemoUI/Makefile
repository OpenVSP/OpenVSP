#!/usr/bin/make -f

CC= gcc
CXX= g++
RM= rm -f

PKGCONFIG= pkg-config
PACKAGES= gl

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
	-lm -L../Pinocchio/ -lpinocchio -lfltk -lfltk_gl \
	$(shell $(PKGCONFIG) --libs $(PACKAGES))

SRCS= DemoUI.cpp MyWindow.cpp defmesh.cpp processor.cpp motion.cpp filter.cpp
OBJS= $(subst .cpp,.o,$(SRCS))

BINARY= DemoUI

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

include .depend

.PHONY: all depend clean distclean cleanall
