# Tutoral GCC Makefile

CXX = g++
CXXFLAGS = -std=c++11 -ggdb -I../../../../angelscript/include
SRCDIR = ../../source
OBJDIR = obj

SRCNAMES =   main.cpp

OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o))) obj/scriptstdstring.o obj/contextmgr.o obj/scriptany.o obj/scriptdictionary.o obj/scriptarray.o
BIN = ../../bin/coroutine
DELETER = rm -f

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) -o $(BIN) $(OBJ) -static -langelscript -dynamic -L ../../../../angelscript/lib -lpthread
	@echo -------------------------------------------------------------------
	@echo Done.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptarray.o: ../../../../add_on/scriptarray/scriptarray.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptdictionary.o: ../../../../add_on/scriptdictionary/scriptdictionary.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptstdstring.o: ../../../../add_on/scriptstdstring/scriptstdstring.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/contextmgr.o: ../../../../add_on/contextmgr/contextmgr.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptany.o: ../../../../add_on/scriptany/scriptany.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BIN)

.PHONY: all clean
