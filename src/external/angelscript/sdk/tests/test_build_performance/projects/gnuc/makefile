# Tutoral GCC Makefile
CXX = g++
#CXX = i686-pc-mingw32-g++.exe
CXXFLAGS = -O0 -g -I../../../../angelscript/include -I../../../../angelscript/source -D_LINUX_
CXXADDFLAGS = 
ifdef PROFILE
CXXFLAGS += -pg
CXXADDFLAGS = -pg
endif
#CXXFLAGS = -O0 -g -I../../../../angelscript/include 
SRCDIR = ../../source
ADDONDIR = ../../../../add_on
ADDONS = 
	 
OBJDIR = obj
SRCNAMES = \
  main.cpp \
  test_basic.cpp \
  test_big_arrays.cpp \
  test_complex.cpp \
  test_many_symbols.cpp \
  test_many_funcs.cpp \
  utils.cpp  
     
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o))) \
  obj/scriptstdstring.o \
  obj/scriptstdstring_utils.o \
  obj/scriptarray.o
	
BIN = ../../bin/asrun
DELETER = rm -f
all: $(BIN)
$(BIN): $(OBJ)
	$(CXX) -static $(CXXADDFLAGS) -o $(BIN) $(OBJ) -langelscript -lpthread -L ../../../../angelscript/lib
	@echo -------------------------------------------------------------------
	@echo Done.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
obj/scriptstdstring.o: ../../../../add_on/scriptstdstring/scriptstdstring.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
obj/scriptstdstring_utils.o: ../../../../add_on/scriptstdstring/scriptstdstring_utils.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
obj/scriptarray.o: ../../../../add_on/scriptarray/scriptarray.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BIN)
.PHONY: all clean
