#!/usr/bin/make -f

CC= gcc
CXX= g++
RM= rm -f

PKGCONFIG= pkg-config
PACKAGES= 

# OS-Specfic Stuff
ifeq ($(OSTYPE),darwin)
	OS_COMPILE_FLAGS =  -DOSX
else
	# LINUX stuff
	OS_COMPILE_FLAGS =  -DLINUX
endif

CFLAGS= -O2 -g -Wall -std=c++17 \
	-fstack-protector-strong \
	-Wall \
	-Wformat \
	-Werror=format-security \
	-Wdate-time \
	-D_FORTIFY_SOURCE=2 \
	$(OS_COMPILE_FLAGS) \
	$(shell $(PKGCONFIG) --cflags $(PACKAGES) 2>/dev/null)

LDFLAGS= \
	-Wl,-z,defs,-z,relro,-z,now \
	-Wl,--as-needed \
	-Wl,--no-undefined

LIBS= \
	-lm -L../Pinocchio/ -lpinocchio \
	$(shell $(PKGCONFIG) --libs $(PACKAGES) 2>/dev/null)

SRCS= AttachWeights.cpp  stdafx.cpp
OBJS= $(subst .cpp,.o,$(SRCS))

BINARY= AttachWeights

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
