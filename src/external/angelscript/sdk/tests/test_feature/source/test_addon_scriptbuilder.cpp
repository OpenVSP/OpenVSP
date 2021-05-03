#include "utils.h"

#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include "../../../add_on/scriptbuilder/scriptbuilder.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"

namespace Test_Addon_ScriptBuilder
{

const char *script =
// shebang directive is removed by preprocessor
"#!asrun\n"
// Global functions can have meta data
"[ my meta data test ] /* separated by comment */ [second data] void func1() {} \n"
// meta data strings can contain any tokens, and can use nested []
"[ test['hello'] ] void func2() {} \n"
// global variables can have meta data
"[ init ] int g_var = 0; \n"
// Parts of the code can be excluded through conditional compilation
"#if DONTCOMPILE                                      \n"
"  This code should be excluded by the CScriptBuilder \n"
"  #if NESTED                                         \n"
"    Nested blocks are also possible                  \n"
"  #endif                                             \n"
"  Nested block ended                                 \n"
"#endif                                               \n"
// global object variable
"[ var of type myclass ] MyClass g_obj(); \n"
// class declarations can have meta data
"#if COMPILE \n"
"[ myclass ] class MyClass {} \n"
" #if NESTED \n"
"   dont compile this nested block \n"
" #endif \n"
"#endif \n"
// class properties can also have meta data
"[ myclass2 ] \n"
"class MyClass2 { \n"
" [ edit ] \n"
" int a; \n"
" int func() { \n"
"   return 0; \n"
" } \n"
" [ noedit ] int b; \n"
" [ edit,c ] \n"
" complex c; \n"
" [ prop ] \n" // It's possible to inform meta data for virtual properties too
" complex prop { get {return c;} set {c = value;} } \n"
"} \n"
// interface declarations can have meta data
"[ myintf ] interface MyIntf {} \n"
// arrays must still work
"int[] arr = {1, 2, 3}; \n"
"int[] @arrayfunc(int[] @a) { a.resize(1); return a; } \n"
// directives in comments should be ignored
"/* \n"
"#include \"dont_include\" \n"
"*/ \n"
// namespaces can also contain entities with metadata
"namespace NS { \n"
" [func] void func() {} \n"
" [class] class Class {} \n"
"} \n"
;

using namespace std;



bool Test()
{
	bool fail = false;
	int r = 0;
	COutStream out;
	CBufferedOutStream bout;

	// TODO: Preprocessor directives should be alone on the line

	// Test gathering metadata for class members when class has been declared with decorators
	// https://www.gamedev.net/forums/topic/709284-property-accessors-in-mixins/5436118/?page=2
#if AS_PROCESS_METADATA == 1
	{
		asIScriptEngine* engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		CScriptBuilder builder;
		builder.StartNewModule(engine, 0);
		builder.AddSectionFromMemory("test",
			"shared class Foo \n"
			"{ \n"
			"	[replication] \n"
			"	int bar; \n"
			"} \n");
		r = builder.BuildModule();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		int typeId = engine->GetModule(0)->GetTypeIdByDecl("Foo");
		vector<string> metadata = builder.GetMetadataForTypeProperty(typeId, 0);
		if (metadata[0] != "replication")
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}
#endif

	// Test gathering metadata for class methods with decorators
	// (reported by Patrick Jeeves)
#if AS_PROCESS_METADATA == 1
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		CScriptBuilder builder;
		builder.StartNewModule(engine, 0);
		builder.AddSectionFromMemory("test",
			"class GameObject { \n"
			"   [metadata] \n"
			"   private void onActivate1(int id, GameObject@ from) {} \n"
			"   [metadata] \n"
			"   void onActivate2(int id, GameObject@ from) final {} \n"
			"} \n");
		r = builder.BuildModule();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}
#endif

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	{
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		if (!strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
			RegisterScriptMathComplex(engine);
		else
			engine->RegisterObjectType("complex", 4, asOBJ_VALUE | asOBJ_POD);

		// Test the parse token method
		asETokenClass t = engine->ParseToken("!is");
		if (t != asTC_KEYWORD)
			TEST_FAILED;

		// Compile a script with meta data strings
		CScriptBuilder builder;
		builder.DefineWord("COMPILE");
		r = builder.StartNewModule(engine, 0);
		r = builder.AddSectionFromMemory("", script);
		r = builder.BuildModule();
#if AS_PROCESS_METADATA == 1
		if (r < 0)
			TEST_FAILED;

		asIScriptFunction *func = engine->GetModule(0)->GetFunctionByName("func1");
		vector<string> metadata = builder.GetMetadataForFunc(func);
		if (metadata[0] != " my meta data test ")
			TEST_FAILED;
		if (metadata[1] != "second data")
			TEST_FAILED;

		func = engine->GetModule(0)->GetFunctionByName("func2");
		metadata = builder.GetMetadataForFunc(func);
		if (metadata[0] != " test['hello'] ")
			TEST_FAILED;

		engine->GetModule(0)->SetDefaultNamespace("NS");
		func = engine->GetModule(0)->GetFunctionByName("func");
		metadata = builder.GetMetadataForFunc(func);
		if (metadata[0] != "func")
			TEST_FAILED;
		engine->GetModule(0)->SetDefaultNamespace("");

		int typeId = engine->GetModule(0)->GetTypeIdByDecl("MyClass");
		metadata = builder.GetMetadataForType(typeId);
		if (metadata[0] != " myclass ")
			TEST_FAILED;

		typeId = engine->GetModule(0)->GetTypeIdByDecl("NS::Class");
		metadata = builder.GetMetadataForType(typeId);
		if (metadata[0] != "class")
			TEST_FAILED;

		typeId = engine->GetModule(0)->GetTypeIdByDecl("MyClass2");
		metadata = builder.GetMetadataForTypeProperty(typeId, 0);
		if (metadata[0] != " edit ")
			TEST_FAILED;
		metadata = builder.GetMetadataForTypeProperty(typeId, 1);
		if (metadata[0] != " noedit ")
			TEST_FAILED;
		metadata = builder.GetMetadataForTypeProperty(typeId, 2);
		if (metadata[0] != " edit,c ")
			TEST_FAILED;

		asITypeInfo *type = engine->GetTypeInfoById(typeId);
		if (type == 0)
			TEST_FAILED;
		else
		{
			metadata = builder.GetMetadataForTypeMethod(typeId, type->GetMethodByName("get_prop"));
			if (metadata[0] != " prop ")
				TEST_FAILED;
			metadata = builder.GetMetadataForTypeMethod(typeId, type->GetMethodByName("set_prop"));
			if (metadata[0] != " prop ")
				TEST_FAILED;
		}

		typeId = engine->GetModule(0)->GetTypeIdByDecl("MyIntf");
		metadata = builder.GetMetadataForType(typeId);
		if (metadata[0] != " myintf ")
			TEST_FAILED;

		int varIdx = engine->GetModule(0)->GetGlobalVarIndexByName("g_var");
		metadata = builder.GetMetadataForVar(varIdx);
		if (metadata[0] != " init ")
			TEST_FAILED;

		varIdx = engine->GetModule(0)->GetGlobalVarIndexByName("g_obj");
		metadata = builder.GetMetadataForVar(varIdx);
		if (metadata[0] != " var of type myclass ")
			TEST_FAILED;
#endif
	}

#if AS_PROCESS_METADATA == 1
	// Test metadata on external shared entities
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"shared class test {} \n"
			"shared interface intf {} \n"
			"shared enum boo {} \n"
			"shared void func() {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		
		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("test1",
			"[external test] \n"
			"external shared class test; \n"
			"[external intf] \n"
			"external shared interface intf; \n"
			"[external boo] \n"
			"external shared enum boo; \n"
			"[external func] \n"
			"external shared void func(); \n");

		r = builder.BuildModule();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asITypeInfo *type = engine->GetModule("mod")->GetTypeInfoByName("test");
		if (type == 0)
			TEST_FAILED;
		else
		{
			int typeId = type->GetTypeId();
			vector<string> metadata = builder.GetMetadataForType(typeId);
			if (metadata[0] != "external test")
				TEST_FAILED;
		}

		vector<string> metadata = builder.GetMetadataForFunc(engine->GetModule("mod")->GetFunctionByName("func"));
		if (metadata[0] != "external func")
			TEST_FAILED;

		metadata = builder.GetMetadataForType(engine->GetModule("mod")->GetTypeInfoByName("boo")->GetTypeId());
		if (metadata[0] != "external boo")
			TEST_FAILED;

		metadata = builder.GetMetadataForType(engine->GetModule("mod")->GetTypeInfoByName("intf")->GetTypeId());
		if (metadata[0] != "external intf")
			TEST_FAILED;
	}
#endif

#if AS_PROCESS_METADATA == 1
	// Test metadata on class methods with default arguments
	// Bug reported by Thomas Grip
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("test1",
			"class test { \n"
			"  [some meta data for func] \n"
			"  void func(int a, int b = 0) {} \n"
			"} \n"
			"[some meta data for global] \n"
			"void glob(int a, int b = 0) {} \n");

		r = builder.BuildModule();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asITypeInfo *type = engine->GetModule("mod")->GetTypeInfoByName("test");
		if (type == 0)
			TEST_FAILED;
		else
		{
			int typeId = type->GetTypeId();
			vector<string> metadata = builder.GetMetadataForTypeMethod(typeId, type->GetMethodByName("func"));
			if (metadata[0] != "some meta data for func")
				TEST_FAILED;
		}

		vector<string> metadata = builder.GetMetadataForFunc(engine->GetModule("mod")->GetFunctionByName("glob"));
		if (metadata[0] != "some meta data for global")
			TEST_FAILED;
	}
#endif

#ifdef _WIN32
	// On Windows the file names are case insensitive so the script builder 
	// must do caseless comparison for duplicate included files
	// http://www.gamedev.net/topic/669353-script-builder-addon-does-not-detect-duplicate-scripts-on-windows/
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("test1","#include 'blah.as'\n");
		builder.AddSectionFromMemory("test2","#include 'BLAH.AS'\n");
		r = builder.BuildModule();
		if( r >= 0 )
			TEST_FAILED;

		// Should only get error for the first include
		string error = GetCurrentDir() + "/blah.as (0, 0) : Error   : Failed to open script file '" + GetCurrentDir() + "/blah.as'\n"
			           " (0, 0) : Error   : Nothing was built in the module\n";

		if( bout.buffer != error )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}
#endif

	// Test proper error for missing include files
	// http://www.gamedev.net/topic/661064-include-in-scriptbuilder-addon-may-report-wrong-path-on-error/
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("test1", "#include 'rel_dir/missing_include.as'\n");
		builder.AddSectionFromMemory("test2", "#include '/abs_dir/missing_inc.as'\n");
		builder.AddSectionFromMemory("test3", "#include 'c:/disk_path/missing_inc.as'\n");
		builder.AddSectionFromMemory("test4", "#include '../bin/scripts/include.as'\n");
		r = builder.BuildModule();
		if( r < 0 )
			PRINTF("The build failed. Are you running the test from the correct path?\n");
		// TODO: The error message should be shown as being from the file that included the other file. Line number should be where the #include directive was found
		string error = GetCurrentDir() + "/rel_dir/missing_include.as (0, 0) : Error   : Failed to open script file '" + GetCurrentDir() + "/rel_dir/missing_include.as'\n"
					   "/abs_dir/missing_inc.as (0, 0) : Error   : Failed to open script file '/abs_dir/missing_inc.as'\n"
					   "c:/disk_path/missing_inc.as (0, 0) : Error   : Failed to open script file 'c:/disk_path/missing_inc.as'\n";
		if( bout.buffer != error )
			PRINTF("%s", bout.buffer.c_str());
	}

	// Test that ../ and ./ are properly resolved
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("test5", "#include '../bin/./scripts/include.as'\n");
		r = builder.BuildModule();
		if (r < 0)
			PRINTF("The build failed. Are you running the test from the correct path?\n");
		string sections;
		for (asUINT n = 0; n < builder.GetSectionCount(); n++)
			sections += builder.GetSectionName(n) + "\n";

		string expected = GetCurrentDir() + "/scripts/include.as\n"
			              "test5\n";
		if (sections != expected)
		{
			PRINTF("%s", sections.c_str());
			TEST_FAILED;
		}
	}


	// http://www.gamedev.net/topic/624445-cscriptbuilder-asset-string-subscript-out-of-range/
	{
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("", "#", 0, 2); // Add a line offset for error reporting
		r = builder.BuildModule();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != " (3, 1) : Error   : Unexpected token '<unrecognized token>'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Add a script section from memory with length
	{
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &bout, asCALL_THISCALL);
		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("", "void func() {} $#", 14);
		r = builder.BuildModule();
		if( r < 0 )
			TEST_FAILED;
	}

	// http://www.gamedev.net/topic/631848-cscriptbuilder-bug/
	{
		bout.buffer = "";
		CScriptBuilder builder;
		builder.StartNewModule(engine, "mod");
		builder.AddSectionFromMemory("", "class ");
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = builder.BuildModule();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != " (1, 7) : Error   : Expected identifier\n"
		                   " (1, 7) : Error   : Instead found '<end of file>'\n"
						   " (1, 7) : Error   : Expected '{'\n"
						   " (1, 7) : Error   : Instead found '<end of file>'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	engine->Release();

	return fail;
}

} // namespace

