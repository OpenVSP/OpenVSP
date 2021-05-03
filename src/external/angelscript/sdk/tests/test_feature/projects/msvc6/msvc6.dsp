# Microsoft Developer Studio Project File - Name="msvc6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=msvc6 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msvc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msvc6.mak" CFG="msvc6 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msvc6 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "msvc6 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msvc6 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /I "../../../../angelscript/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../../angelscript/lib/angelscript.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/msvc6.exe"

!ELSEIF  "$(CFG)" == "msvc6 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../../angelscript/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../../angelscript/lib/angelscriptd.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/msvc6.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "msvc6 - Win32 Release"
# Name "msvc6 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\source\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test2modules.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_2func.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_debugger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_dictionary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_scriptarray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_scriptbuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_scripthandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_scriptmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_serializer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_addon_weakref.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_any.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_argref.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_array.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_arrayhandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_arrayintf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_arrayobject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_assign.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_autohandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_bits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_bool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_castop.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_cdecl_objlast.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_cdecl_return.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_circularimport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_compiler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_condition.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_config.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_configaccess.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_constobject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_constproperty.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_constructor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_constructor2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_conversion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_cstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_custommem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_defaultarg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_destructor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_dict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_discard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_dump.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_dynamicconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_enum.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_exceptionmemory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_factory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_file.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_float.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_for.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_funcoverload.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_functionptr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_garbagecollect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_generic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_getargptr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_getset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_implicitcast.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_implicithandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_import.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_import2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_inheritance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_int8.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_interface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_mixin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_module.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_multiassign.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_namespace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_nested.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_nevervisited.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_notinitialized.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_object2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_object3.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_objhandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_objhandle2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_objzerosize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_operator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_optimize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_pointer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_postprocess.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_refargument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_refcast.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_registertype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_return_with_cdecl_objfirst.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_returnstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_rz.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_saveload.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_scriptclassmethod.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_scriptmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_scriptretref.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_scriptstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_scriptstruct.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_shared.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_shark.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_singleton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_stack2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_stdvector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_structintf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_suspend.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_template.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_thiscall_asglobal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_thiscall_class.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_typedef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_unsaferef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_vartype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_vector3.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\test_vector3_2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testbstr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcdecl_class.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcdecl_class_a.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcdecl_class_c.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcdecl_class_d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcdecl_class_k.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testcreateengine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testenumglobvar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute1arg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute2args.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute32args.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute32mixedargs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute4args.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecute4argsf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecutemixedargs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecutescript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecutestring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testexecutethis32mixedargs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testglobalvar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testint64.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testlongtoken.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testmoduleref.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testmultipleinheritance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testnegateoperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testnotcomplexstdcall.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testnotcomplexthiscall.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testoutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\teststack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\teststdcall4args.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\teststdstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testswitch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testtempvar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testvirtualinheritance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\testvirtualmethod.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\angelscript\include\angelscript.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\add_on\autowrapper\aswrappedcall.h
# End Source File
# Begin Source File

SOURCE=..\..\source\bstr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\bstr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\contextmgr\contextmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\contextmgr\contextmgr.h
# End Source File
# Begin Source File

SOURCE=..\..\source\cstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\debugger\debugger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\debugger\debugger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptany\scriptany.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptany\scriptany.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptarray\scriptarray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptarray\scriptarray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptbuilder\scriptbuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptbuilder\scriptbuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptdictionary\scriptdictionary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptdictionary\scriptdictionary.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptfile\scriptfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptfile\scriptfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scripthandle\scripthandle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scripthandle\scripthandle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scripthelper\scripthelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scripthelper\scripthelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptmath\scriptmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptmath\scriptmath.h
# End Source File
# Begin Source File

SOURCE=..\..\source\scriptmath3d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\scriptmath3d.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptmath\scriptmathcomplex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptmath\scriptmathcomplex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptstdstring\scriptstdstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptstdstring\scriptstdstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\scriptstdstring\scriptstdstring_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\scriptstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\scriptstring.h
# End Source File
# Begin Source File

SOURCE=..\..\source\scriptstring_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\serializer\serializer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\serializer\serializer.h
# End Source File
# Begin Source File

SOURCE=..\..\source\stdvector.h
# End Source File
# Begin Source File

SOURCE=..\..\source\utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\source\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\weakref\weakref.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\add_on\weakref\weakref.h
# End Source File
# End Group
# End Target
# End Project
