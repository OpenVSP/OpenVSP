#!/usr/bin/make -f

LIBRARY=libpinocchio

all: $(LIBRARY).a $(LIBRARY).so

MAJOR= 0
MINOR= 0

SOURCES= \
	attachment.cpp discretization.cpp indexer.cpp lsqSolver.cpp mesh.cpp \
	graphutils.cpp intersector.cpp matrix.cpp skeleton.cpp embedding.cpp \
	pinocchioApi.cpp refinement.cpp quatinterface.cpp

SHARED_OBJS = $(SOURCES:.cpp=.shared.o)
STATIC_OBJS = $(SOURCES:.cpp=.static.o)

CC= gcc
CXX= g++
AR= ar
RM= rm -f

PKGCONFIG= pkg-config
PACKAGES= 

CFLAGS= -O2 -g -Wall -std=c++17 \
	-Ishared \
	-fstack-protector-strong \
	-Wall \
	-Wformat \
	-Werror=format-security \
	-Wdate-time \
	-D_FORTIFY_SOURCE=2 \
	$(shell $(PKGCONFIG) --cflags $(PACKAGES) 2>/dev/null)

STATIC_CFLAGS= $(CFLAGS)
SHARED_CFLAGS= $(CFLAGS) -fPIC

LDFLAGS= \
	-Wl,-z,defs,-z,relro,-z,now \
	-Wl,--as-needed \
	-Wl,--no-undefined

EXTRA_LDFLAGS=

ARFLAGS= cr

LIBS= $(shell $(PKGCONFIG) --libs $(PACKAGES) 2>/dev/null)

$(LIBRARY).so.$(MAJOR).$(MINOR): $(SHARED_OBJS)
	$(CXX) $(LDFLAGS) $(EXTRA_LDFLAGS) -shared \
		-Wl,-soname,$(LIBRARY).so.$(MAJOR) \
		-o $(LIBRARY).so.$(MAJOR).$(MINOR) \
		$+ -o $@ $(LIBS)

$(LIBRARY).so: $(LIBRARY).so.$(MAJOR).$(MINOR)
	rm -f $@.$(MAJOR)
	ln -s $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	rm -f $@
	ln -s $@.$(MAJOR) $@

$(LIBRARY).a: $(STATIC_OBJS)
	$(AR) $(ARFLAGS) $@ $+

%.shared.o: %.cpp
	$(CXX) -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.cc
	$(CXX) -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.c
	$(CC) -o $@ -c $+ $(SHARED_CFLAGS)

%.so : %.o
	$(CXX) $(LDFLAGS) $(EXTRA_LDFLAGS) -shared $^ -o $@

%.static.o: %.cpp
	$(CXX) -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.cc
	$(CXX) -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.c
	$(CC) -o $@ -c $+ $(STATIC_CFLAGS)

clean:
	rm -f $(SHARED_OBJS)
	rm -f $(STATIC_OBJS)
	rm -f $(FIXED_OBJS)
	rm -f $(STATIC_FIXED_OBJS)
	rm -f $(LIBRARY).a $(LIBRARY).so
	rm -f *.so *.so* *.a *~

distclean cleanall: clean
	$(RM) *~ .depend core *.out *.bak *.o

depend: .depend

.depend: $(SOURCES)
	$(RM) ./.depend
	$(CXX) $(CFLAGS) -MM $^>>./.depend;

include .depend

.PHONY: all depend clean distclean cleanall
