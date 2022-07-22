# Tutoral GCC Makefile

CXX = g++
CXXFLAGS = -std=c++11 -ggdb -I../../../../angelscript/include
SRCDIR = ../../source
OBJDIR = obj

SRCNAMES = \
  main.cpp

OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o))) obj/scriptarray.o obj/scriptstdstring.o obj/scripthelper.o
BIN = ../../bin/console
DELETER = rm -f

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) -o $(BIN) $(OBJ) -static -langelscript -dynamic -lpthread -L ../../../../angelscript/lib
	@echo -------------------------------------------------------------------
	@echo Done.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptarray.o: ../../../../add_on/scriptarray/scriptarray.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptstdstring.o: ../../../../add_on/scriptstdstring/scriptstdstring.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scripthelper.o: ../../../../add_on/scripthelper/scripthelper.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BIN)

.PHONY: all clean
