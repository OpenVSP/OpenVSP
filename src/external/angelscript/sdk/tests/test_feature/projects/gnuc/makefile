# Test Framework GNUC makefile

CXX ?= g++
CXXFLAGS += -ggdb -I../../../../angelscript/include -Wno-missing-field-initializers
SRCDIR = ../../source
OBJDIR = obj

ifdef DONT_WAIT
    CXXFLAGS += -DDONT_WAIT
endif


SRCNAMES = \
  bstr.cpp \
  main.cpp \
  test_2func.cpp \
  test_addon_contextmgr.cpp \
  test_addon_datetime.cpp \
  test_addon_scriptarray.cpp \
  test_addon_scriptbuilder.cpp \
  test_addon_scriptfile.cpp \
  test_addon_scriptgrid.cpp \
  test_addon_scripthandle.cpp \
  test_addon_scriptmath.cpp \
  test_addon_serializer.cpp \
  test_addon_dictionary.cpp \
  test_addon_debugger.cpp \
  test_addon_weakref.cpp \
  test_addon_stdstring.cpp \
  test_any.cpp \
  test_argref.cpp \
  test_array.cpp \
  test_arrayhandle.cpp \
  test_arrayintf.cpp \
  test_arrayobject.cpp \
  test_autohandle.cpp \
  test_assign.cpp \
  test_bits.cpp \
  test_bool.cpp \
  test_castop.cpp \
  test_circularimport.cpp \
  test_compiler.cpp \
  test_composition.cpp \
  test_config.cpp \
  test_configaccess.cpp \
  test_condition.cpp \
  test_constobject.cpp \
  test_constproperty.cpp \
  test_constructor.cpp \
  test_constructor2.cpp \
  test_conversion.cpp \
  test_custommem.cpp \
  test_debug.cpp \
  test_defaultarg.cpp \
  test_namedargs.cpp \
  test_destructor.cpp \
  test_dict.cpp \
  test_discard.cpp \
  test_dump.cpp \
  test_dynamicconfig.cpp \
  test_enum.cpp \
  test_exception.cpp \
  test_exceptionmemory.cpp \
  test_float.cpp \
  test_for.cpp \
  test_funcoverload.cpp \
  test_functionptr.cpp \
  test_garbagecollect.cpp \
  test_generic.cpp \
  test_getargptr.cpp \
  test_getset.cpp \
  test_implicitcast.cpp \
  test_implicithandle.cpp \
  test_auto.cpp \
  test_import.cpp \
  test_import2.cpp \
  test_inheritance.cpp \
  test_interface.cpp \
  test_mixin.cpp \
  test_module.cpp \
  test_multiassign.cpp \
  test_namespace.cpp \
  test_native_defaultfunc.cpp \
  test_nested.cpp \
  test_nevervisited.cpp \
  test_notinitialized.cpp \
  test_object.cpp \
  test_object2.cpp \
  test_object3.cpp \
  test_objhandle.cpp \
  test_objhandle2.cpp \
  test_objzerosize.cpp \
  test_operator.cpp \
  test_optimize.cpp \
  test_parser.cpp \
  test_pointer.cpp \
  test_postprocess.cpp \
  test_pow.cpp \
  test_propintegerdivision.cpp \
  test_refargument.cpp \
  test_refcast.cpp \
  test_returnstring.cpp \
  test_return_with_cdecl_objfirst.cpp \
  test_saveload.cpp \
  test_scriptclassmethod.cpp \
  test_scriptmath.cpp \
  test_scriptretref.cpp \
  test_scriptstring.cpp \
  test_scriptstruct.cpp \
  test_shared.cpp \
  test_shark.cpp \
  test_singleton.cpp \
  test_suspend.cpp \
  test_stack2.cpp \
  test_stdvector.cpp \
  test_stream.cpp \
  test_structintf.cpp \
  test_template.cpp \
  test_thiscall_asglobal.cpp \
  test_thiscall_class.cpp \
  test_thiscall_as_method.cpp \
  test_thiscall_as_method_config.cpp \
  test_typedef.cpp \
  test_unsaferef.cpp \
  test_vartype.cpp \
  test_vector3.cpp \
  test_vector3_2.cpp \
  test2modules.cpp \
  testbstr.cpp \
  testcdecl_class.cpp \
  testcdecl_class_a.cpp \
  testcdecl_class_c.cpp \
  testcdecl_class_d.cpp \
  testcdecl_class_k.cpp \
  testcreateengine.cpp \
  testenumglobvar.cpp \
  testexecute.cpp \
  testexecute1arg.cpp \
  testexecute2args.cpp \
  testexecute32args.cpp \
  testexecute32mixedargs.cpp \
  testexecute4args.cpp \
  testexecute4argsf.cpp \
  testexecutemixedargs.cpp \
  testexecutescript.cpp \
  testexecutestring.cpp \
  testexecutethis32mixedargs.cpp \
  test_factory.cpp \
  testglobalvar.cpp \
  test_int8.cpp \
  testint64.cpp \
  testlongtoken.cpp \
  testmoduleref.cpp \
  testmultipleinheritance.cpp \
  testnegateoperator.cpp \
  testnotcomplexstdcall.cpp \
  testnotcomplexthiscall.cpp \
  testoutput.cpp \
  test_registertype.cpp \
  test_cdecl_objlast.cpp \
  test_cdecl_return.cpp \
  test_rz.cpp \
  teststdcall4args.cpp \
  teststdstring.cpp \
  teststack.cpp \
  testswitch.cpp \
  testtempvar.cpp \
  testvirtualinheritance.cpp \
  testvirtualmethod.cpp \
  utils.cpp \
  scriptstring.cpp \
  scriptstring_utils.cpp \
  scriptmath3d.cpp \


OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o))) \
  obj/scriptarray.o \
  obj/scriptgrid.o \
  obj/scripthandle.o \
  obj/scripthelper.o \
  obj/scriptstdstring.o \
  obj/scriptstdstringutil.o \
  obj/scriptany.o \
  obj/scriptmath.o \
  obj/scriptmathcomplex.o \
  obj/scriptdictionary.o \
  obj/scriptfile.o \
  obj/scriptfilesystem.o \
  obj/scriptbuilder.o \
  obj/serializer.o \
  obj/debugger.o \
  obj/weakref.o \
  obj/contextmgr.o \
  obj/datetime.o 


BIN = ../../bin/testgnuc
DELETER = rm -f

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $(BIN) $(OBJ)  -Wl,-Bstatic -langelscript -Wl,-Bdynamic -lpthread -L../../../../angelscript/lib
	@echo -------------------------------------------------------------------
	@echo Done.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptarray.o: ../../../../add_on/scriptarray/scriptarray.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptgrid.o: ../../../../add_on/scriptgrid/scriptgrid.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scripthandle.o: ../../../../add_on/scripthandle/scripthandle.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scripthelper.o: ../../../../add_on/scripthelper/scripthelper.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptstdstring.o: ../../../../add_on/scriptstdstring/scriptstdstring.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptstdstringutil.o: ../../../../add_on/scriptstdstring/scriptstdstring_utils.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptdictionary.o: ../../../../add_on/scriptdictionary/scriptdictionary.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptmath.o: ../../../../add_on/scriptmath/scriptmath.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptmathcomplex.o: ../../../../add_on/scriptmath/scriptmathcomplex.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptany.o: ../../../../add_on/scriptany/scriptany.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptfile.o: ../../../../add_on/scriptfile/scriptfile.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptfilesystem.o: ../../../../add_on/scriptfile/scriptfilesystem.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/scriptbuilder.o: ../../../../add_on/scriptbuilder/scriptbuilder.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/serializer.o: ../../../../add_on/serializer/serializer.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/debugger.o: ../../../../add_on/debugger/debugger.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/weakref.o: ../../../../add_on/weakref/weakref.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/contextmgr.o: ../../../../add_on/contextmgr/contextmgr.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/datetime.o: ../../../../add_on/datetime/datetime.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BIN)

.PHONY: all clean
