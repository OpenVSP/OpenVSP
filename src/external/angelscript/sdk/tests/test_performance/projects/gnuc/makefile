# Test Framework GNUC makefile



CXX = g++

CXXFLAGS = -ggdb -I../../../../angelscript/include -Wno-missing-field-initializers
SRCDIR = ../../source
OBJDIR = obj


SRCNAMES = \
  main.cpp \

  test_basic2.cpp \
  test_basic.cpp \
  test_call2.cpp \
  test_call.cpp \
  test_fib.cpp \

  test_int.cpp \
  test_intf.cpp \

  test_mthd.cpp \
  test_string2.cpp \
  test_string.cpp \

  test_string_pooled.cpp \
  test_thisprop.cpp \
  test_vector3.cpp \
  utils.cpp

OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o))) \
  obj/scriptstring.o



BIN = ../../bin/testgnuc
DELETER = rm -f



all: $(BIN)


$(BIN): $(OBJ)

	$(CXX) -o $(BIN) $(OBJ) -langelscript -lpthread -L../../../../angelscript/lib
	@echo -------------------------------------------------------------------
	@echo Done.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptstring.o: ../../../../add_on/scriptstring/scriptstring.cpp

	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BIN)

.PHONY: all clean