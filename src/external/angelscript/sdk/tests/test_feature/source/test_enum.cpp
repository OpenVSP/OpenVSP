#include "utils.h"
#include <sstream>
#include <iostream>

namespace TestEnum
{

#define TESTMODULE		"TestEnum"



enum TEST_ENUM
{
	ENUM1 = 1,
	ENUM2 = ENUM1*10,
	ENUM3,
	ENUM4 = -1
};

void func(asIScriptGeneric *g)
{
	TEST_ENUM e = (TEST_ENUM)g->GetArgDWord(0);
	UNUSED_VAR(e);
}

std::string buffer;
static void scriptOutput(int val1)
{
	char buf[256];
#if _MSC_VER >= 1500
	sprintf_s(buf, 255, "%d\n", val1);
#else
	sprintf(buf, "%d\n", val1);
#endif
	buffer += buf;
}

class CTestObject
{
public:
	bool TestEnum(TEST_ENUM e) { val = e; if( e == ENUM2 ) return true; else return false; }

	TEST_ENUM val;
};

static bool TestEnum()
{
	RET_ON_MAX_PORT

	asIScriptEngine   *engine;
	COutStream         out;
	CBufferedOutStream bout;
	int                r;
	bool               fail = false;

	// Report warning on enum values that do not fit in 32bit
	// https://www.gamedev.net/forums/topic/687053-values-of-enum-constants/
	{
		engine = asCreateScriptEngine();
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum VAL { a = 12345678901, b = -9223372036854775809 }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 12) : Info    : Compiling VAL a\n"
						   "test (1, 16) : Warning : Value is too large for data type\n"
						   "test (1, 29) : Info    : Compiling VAL b\n"
						   "test (1, 33) : Warning : Value is too large for data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test that enums are properly converted during function overloading
	// http://www.gamedev.net/topic/678030-enum-type-conversion-to-int-cost-not-calculated-correctly/
	{
		engine = asCreateScriptEngine();
		r = engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum e {val} \n"
			"void f(int) {} \n"
			"void f(int8) {} \n"
			"void f(int16) {} \n"
			"void f(int64) {} \n"
			"void main() { \n"
			"	f(val); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test registering same enum in different namespaces
	// http://www.gamedev.net/topic/663391-enum-names-collisions/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		r = engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = engine->RegisterEnum("myenum");
		if( r < 0 )
			TEST_FAILED;

		// The returned value is the type id
		std::string str = engine->GetTypeDeclaration(r);
		if (str != "myenum")
			TEST_FAILED;

		r = engine->RegisterEnumValue("myenum", "value", 1);
		if( r < 0 )
			TEST_FAILED;

		engine->SetDefaultNamespace("foo");
		r = engine->RegisterEnum("myenum");
		if( r < 0 )
			TEST_FAILED;
		r = engine->RegisterEnumValue("myenum", "value", 1);
		if( r < 0 )
			TEST_FAILED;
		engine->SetDefaultNamespace("");

		engine->ShutDownAndRelease();
	}

	// Test ambiguos enum in comparison
	// http://www.gamedev.net/topic/660871-ambiguous-enum-comparison/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum MyEnum1\n"
			"{\n"
			"    cYes,\n"
			"    cNo\n"
			"}\n"
			"enum MyEnum2\n"
			"{\n"
			"    cNo,\n"
			"    cYes\n"
			"}\n"
			"bool main()\n"
			"{\n"
			"    MyEnum1 e = cNo;\n"
			"    return e == cYes;\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}


	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		// Register the enum value
		r = engine->RegisterEnum("TEST_ENUM"); assert(r >= 0);
		r = engine->RegisterEnumValue("TEST_ENUM", "ENUM1", ENUM1); assert(r >= 0);
		r = engine->RegisterEnumValue("TEST_ENUM", "ENUM2", ENUM2); assert(r >= 0);
		r = engine->RegisterEnumValue("TEST_ENUM", "ENUM3", ENUM3); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void funce(TEST_ENUM)", asFUNCTION(func), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("void output(int val1)", asFUNCTION(scriptOutput), asCALL_CDECL); assert(r >= 0);

		// Test calling generic function with enum value
		r = ExecuteString(engine, "funce(ENUM1);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		r = ExecuteString(engine, "funce(TEST_ENUM::ENUM3);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Test using the registered enum values
		r = ExecuteString(engine, "output(ENUM1); output(ENUM2)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "1\n10\n")
			TEST_FAILED;

		// Test script that declare an enum
		// enum value can be given as expression of constants
		// enum can be implicitly cast to number
		buffer = "";
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *const script =
			"enum TEST2_ENUM  									\n"
			"{													\n"
			"	TEST_1 = -1,									\n"
			"	TEST1 = 1,										\n"
			"	TEST2,											\n"
			"	TEST1200 = 300 * 4,								\n"
			"	TEST1201 = TEST1200 + 1,						\n"
			"	TEST1202,										\n"
			"	TEST1203,										\n"
			"	TEST1205 = TEST1201 + 4,						\n"
			"	TEST1_0 = TEST_1 + 1, 							\n"
			"	TEST1_1 = TEST_1 + 2 							\n"
			"}													\n"
			"													\n"
			"TEST2_ENUM Test1()									\n"
			"{													\n"
			"   output(TEST_1);                                 \n"
			"   output(TEST1);                                  \n"
			"   output(TEST2);                                  \n"
			"   output(TEST1200);                               \n"
			"   output(TEST1201);                               \n"
			"   output(TEST1202);                               \n"
			"   output(TEST1203);                               \n"
			"   output(TEST1205);                               \n"
			"   output(TEST1_0);                                \n"
			"   output(TEST1_1);                                \n"
			"   output(TEST2_ENUM::TEST2);                      \n"
			"	return TEST2;									\n"
			"}													\n";
		r = mod->AddScriptSection(NULL, script, strlen(script), 0);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "Test1()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "-1\n1\n2\n1200\n1201\n1202\n1203\n1205\n0\n1\n2\n")
		{
			TEST_FAILED;
			PRINTF("%s", buffer.c_str());
		}

		// Registered enums are literal constants
		// variable of enum type can be implictly cast to primitive
		buffer = "";
		r = ExecuteString(engine, "TEST_ENUM e = ENUM1; switch( e ) { case ENUM1: output(e); }");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "1\n")
			TEST_FAILED;

		// Script declared enums behave the same
		buffer = "";
		r = ExecuteString(engine, "TEST2_ENUM e = TEST_1; switch( e ) {case TEST_1: output(e); }", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "-1\n")
			TEST_FAILED;

		// Different enum types can declare the same enum value
		mod->AddScriptSection(NULL,
			"enum ENUMA { VALUE = 1 } \n"
			"enum ENUMB { VALUE = 2 } \n"
			"enum ENUMC { VAL = 3 } \n"
			"int a = VALUE; \n" // fails with ambiguity
			"int b = ENUMA::VALUE; \n" // ok
			"int c = ENUMB::VALUE; \n" // ok
			"ENUMC d = VALUE; \n"); // fails with ambiguity
		bout.buffer = "";
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != " (4, 5) : Info    : Compiling int a\n"
			" (4, 9) : Error   : Found multiple matching enum values\n"
			" (7, 7) : Info    : Compiling ENUMC d\n"
			" (7, 11) : Error   : Found multiple matching enum values\n"
			" (7, 11) : Error   : Can't implicitly convert from 'int' to 'ENUMC&'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Automatically resolving ambiguous enums if possible
		bout.buffer = "";
		mod->AddScriptSection(NULL,
			"enum ENUMA { VALUE = 1 } \n"
			"enum ENUMB { VALUE = 2 } \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		r = ExecuteString(engine, "ENUMA a = VALUE; assert( a == 1 );\n"
			"ENUMB b = VALUE; assert( b == 2 );\n", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// enum values can't be declared with expressions including subsequent values
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		const char *script2 = "enum TEST_ERR { ERR1 = ERR2, ERR2 }";
		mod = engine->GetModule("error", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("error", script2, strlen(script2));
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "error (1, 17) : Info    : Compiling TEST_ERR ERR1\n"
			"error (1, 24) : Error   : No matching symbol 'ERR2'\n"
			"error (1, 30) : Info    : Compiling TEST_ERR ERR2\n"
			"error (1, 30) : Error   : Use of uninitialized global variable 'ERR1'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// enum type name can't be overloaded with variable name in another scope
		bout.buffer = "";
		r = ExecuteString(engine, "int TEST_ENUM = 999");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 5) : Error   : Illegal variable name 'TEST_ENUM'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		// enum value name can be overloaded with variable name in another scope
		buffer = "";
		r = ExecuteString(engine, "int ENUM1 = 999; output(ENUM1)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "999\n")
			TEST_FAILED;

		// number cannot be implicitly cast to enum type
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "TEST_ENUM val = 1");
		if (r >= 0)
			TEST_FAILED;
		r = ExecuteString(engine, "float f = 1.2f; TEST_ENUM val = f");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 17) : Error   : Can't implicitly convert from 'int' to 'TEST_ENUM'.\n"
			"ExecuteString (1, 33) : Error   : Can't implicitly convert from 'float' to 'TEST_ENUM'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		// constant number can be explicitly cast to enum type
		r = ExecuteString(engine, "TEST_ENUM val = TEST_ENUM(1)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// primitive value can be explicitly cast to enum type
		r = ExecuteString(engine, "float f = 1.2f; TEST_ENUM val = TEST_ENUM(f)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// math operator with enums
		buffer = "";
		r = ExecuteString(engine, "int a = ENUM2 * 10; output(a); output(ENUM2 + ENUM1)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "100\n11\n")
			TEST_FAILED;

		// comparison operator with enums
		buffer = "";
		r = ExecuteString(engine, "if( ENUM2 > ENUM1 ) output(1);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "1\n")
			TEST_FAILED;

		// bitwise operators with enums
		buffer = "";
		r = ExecuteString(engine, "output( ENUM2 << ENUM1 )");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "20\n")
			TEST_FAILED;

		// circular reference between enum and global variable are
		// allowed if they can be resolved
		const char *script3 =
			"enum TEST_EN                \n"
			"{                           \n"
			"  EN1,                      \n"
			"  EN2 = gvar,               \n"
			"  EN3,                      \n"
			"}                           \n"
			"const int gvar = EN1 + 10;  \n";
		mod = engine->GetModule("en", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("en", script3, strlen(script3));
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		buffer = "";
		r = ExecuteString(engine, "output(EN2); output(EN3)", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "10\n11\n")
			TEST_FAILED;

		// functions can be overloaded for parameters with enum type
		bout.buffer = "";
		const char *script4 =
			"void func(TEST_ENUM) { output(1); } \n"
			"void func(int) { output(2); } \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script4, strlen(script4));
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		buffer = "";
		r = ExecuteString(engine, "func(1); func(1.0f); TEST_ENUM e = ENUM1; func(e)", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "2\n2\n1\n")
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Using registered enum type in a script
		engine->RegisterEnum("game_type_t");
		const char *script5 =
			"game_type_t random_game_type;\n"
			"void foo(game_type_t game_type)\n"
			"{\n"
			"   random_game_type = game_type;\n"
			"};\n";
		r = mod->AddScriptSection("script", script5, strlen(script5));
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		// enum with assignment without comma
		const char *script6 = "enum test_wo_comma { value = 0 }";
		r = mod->AddScriptSection("script", script6, strlen(script6));
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		// Enums are not object types
		asITypeInfo *ti = mod->GetEnumByIndex(0);
		if (ti == 0)
			TEST_FAILED;
		if (!(ti->GetFlags() & asOBJ_ENUM))
			TEST_FAILED;

		// enum must allow negate and binary complement operators
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "int a = -ENUM1; int b = ~ENUM1;");
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 25) : Warning : Implicit conversion changed sign of value\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test specifying an unknown enum type name
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		const char *script7 = "void f() { funce(UNKNOWN_ENUM::ENUM1); }";
		mod = engine->GetModule("error", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("error", script7, strlen(script7));
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "error (1, 1) : Info    : Compiling void f()\n"
			"error (1, 18) : Error   : No matching symbol 'UNKNOWN_ENUM::ENUM1'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test specifying a non enum type name before the scope
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		const char *script8 =
			"class SomeClass\n"
			"{\n"
			"  int SOMEVALUE;\n"
			"}\n"
			"void f() { funce(SomeClass::SOMEVALUE); }";
		mod = engine->GetModule("error", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("error", script8, strlen(script8));
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "error (5, 1) : Info    : Compiling void f()\n"
			"error (5, 18) : Error   : Cannot access non-static member 'SOMEVALUE' like this\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test engine property
		r = engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 1);
		if (r != 0)
			TEST_FAILED;

		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		const char *script9 = "void f() { funce(ENUM1); }";
		mod = engine->GetModule("error", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("error", script9, strlen(script9));
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "error (1, 1) : Info    : Compiling void f()\n"
			"error (1, 18) : Error   : No matching symbol 'ENUM1'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		buffer = "";
		r = ExecuteString(engine, "output(TEST_ENUM::ENUM1);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (buffer != "1\n")
			TEST_FAILED;

		// Test enum in param to class method
		assert(sizeof(TEST_ENUM) == 4);
		r = engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 0);
		r = engine->RegisterObjectType("Obj", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
		r = engine->RegisterObjectMethod("Obj", "bool TestEnum(TEST_ENUM)", asMETHOD(CTestObject, TestEnum), asCALL_THISCALL); assert(r >= 0);

		CTestObject obj;
		obj.val = ENUM1;

		r = engine->RegisterGlobalProperty("Obj obj", &obj); assert(r >= 0);

		bout.buffer = "";
		r = ExecuteString(engine, "if( !obj.TestEnum(ENUM2) ) assert(false); ");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		if (obj.val != ENUM2)
			TEST_FAILED;

		// Repeated enum values would enter an infinit loop
		bout.buffer = "";
		const char *script10 = "enum Infinite { inf, inf }";
		mod->AddScriptSection("test", script10);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test (1, 22) : Error   : Name conflict. 'inf' is already used.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", "enum waveformType \n"
										"{ \n"
										"  sawtoothWave = 1, \n"
										"  squareWave = 2, \n"
										"  sineWave = 3 \n"
										"} \n"
										"void main() \n"
										"{ \n"
										"  tone_synth synth; \n"
										"  synth.waveform_type = sineWave; \n"
										"  assert( '' + sineWave + '' == '3' ); \n"
										"  assert( synth.waveform_type == 3 ); \n"
										"} \n"
										"class tone_synth { void set_waveform_type(double v) property {prop = v;} double get_waveform_type() property {return prop;} double prop; }\n");

		r = mod->Build();
		if( r < 0 ) 
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//paste at the end of TestEnum()
	{    
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 1); assert(r >= 0);

		r = engine->RegisterEnum("RENUM_1"); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_1", "R_GLOBAL", 0); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_1", "RE1_1", 1); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_1", "RE1_2", 2); assert(r >= 0);

		r = engine->RegisterEnum("RENUM_2"); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_2", "R_GLOBAL", 13); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_2", "RE2_1", 1); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_2", "RE2_2", 2); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void output(int val1)", asFUNCTION(scriptOutput), asCALL_CDECL); assert(r >= 0);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"enum ENUM_1            \n"
			"{                      \n"
			"  TEST_GLOBAL = 0,     \n"
			"  E1_VAL1,             \n"
			"  E1_VAL2,             \n"
			"  RE1_2                \n"
			"}                      \n"
			"enum ENUM_2            \n"
			"{                      \n"
			"  TEST_GLOBAL = 0,     \n" 
			"  E2_VAL1,             \n"
			"  E2_VAL2              \n"
			"}                      \n"
			"ENUM_1 g_e1 = ENUM_1::E1_VAL1;         \n"
			"RENUM_1 rg_e1 = RENUM_1::RE1_2;        \n" 
			"                                       \n"
			"void main()                            \n"
			"{                                      \n"
			"   ENUM_1 l_e1 = ENUM_1::E1_VAL1;      \n"
			"   g_e1 = ENUM_1::E1_VAL1;             \n"
			"   rg_e1 = RENUM_1::R_GLOBAL;          \n"
			"   RENUM_2 rl_e2 = RENUM_2::R_GLOBAL;  \n"
			"   output(rg_e1);                      \n"
			"   output(rl_e2);                      \n"
			"}                                      \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		buffer = "";
		r = ExecuteString(engine, "main();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( buffer != "0\n13\n" )
			TEST_FAILED;
		engine->Release();
	} 

	// Some validations that must be done
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 1); assert(r >= 0);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"enum ENUM_1            \n"
			"{                      \n"
			"  E1_VAL1              \n"
			"};                     \n"   // Semi colon after enum declaration is allowed, but optional
			"ENUM_1 g_e1 = ENUM_1::E1_VAL1;         \n"
			"ENUM_1 g_e2 = E2_VAL1;                 \n"); // <- that shouldn't (?)
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (6, 8) : Info    : Compiling ENUM_1 g_e2\n"
		                   "script (6, 15) : Error   : No matching symbol 'E2_VAL1'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->RegisterEnum("RENUM_1"); assert(r >= 0);
		r = engine->RegisterEnumValue("RENUM_1", "@#$%", 777); // shouldn't work
		if( r >= 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem reported by Andrew Ackermann
	// The code crashed in ALLOC as the enum was copied as 8bytes on 64bit platforms
	{
		const char *script = 
		"enum TestEnum { \n"
		" TE_0, \n"
		" TE_1, \n"
		"}; \n"
		"class TestClass { \n"
		" TestClass(TestEnum en) { \n"
		" } \n"
		"}; \n"
		"void init() { \n"
		" TestClass@ cl = TestClass(TE_1); \n"
		"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	
		r = ExecuteString(engine, "init()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem reported by SadSingleton
	// http://www.gamedev.net/topic/622524-crash-using-the-identity-operator-with-enum-values/
	{
		const char *script = 
			"enum MyEnum { MyEnumValue = 1 } \n"
			"void Update() \n"
			"{ \n"
			"        MyEnum enumValue = MyEnumValue; \n"
			"        bool condition = true; \n"
			"        if (condition) \n"
			"        { \n"
			"                if(enumValue is MyEnumValue) \n"
			"                { \n"
			"                        int i = 0; \n"
			"                } \n"
			"        } \n"
			"        else \n"
			"        { \n"
			"                int j = 1; \n"
			"        } \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (2, 1) : Info    : Compiling void Update()\n"
		                   "script (8, 30) : Error   : Both operands must be handles when comparing identity\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// http://www.gamedev.net/topic/624715-bug-in-compiler/
	{
		const char *script = 
			"enum E \n"
			"{ \n"
			"  VALUE1 = 20somegarbage, \n"
			"  VALUE2 = 30moregarbage \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (3, 3) : Info    : Compiling E VALUE1\n"
						   "script (3, 14) : Error   : Unexpected token '<identifier>'\n"
						   "script (4, 3) : Info    : Compiling E VALUE2\n"
						   "script (4, 14) : Error   : Unexpected token '<identifier>'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// Success
	return fail;
}

namespace Nalin
{
// This test was provided by Nalin to reproduce a bug where the
// compiler thought the enum type was 8 bytes on 64bit platforms.

using namespace std;

enum TEST_ENUM
{
	ENUM1 = 1,
	ENUM2 = ENUM1*10,
	ENUM3,
	ENUM4 = -1
};

bool TestEnum(TEST_ENUM e)
{
	if (e == ENUM2)
		return true;
	return false;
}

bool TestEnum2(int e)
{
	if ((TEST_ENUM)e == ENUM2)
		return true;
	return false;
}

void print(const string& /*log*/)
{
//	cout << log << endl;
}

string script =
"void Update()\n"
"{\n"
"	int test1 = 1;\n"
"	int test2 = 2;\n"
"	bool ret1 = TestEnum(ENUM1);\n"
"	if (ret1 == true) print('True');\n"
"	else print('False');\n"
"\n"
"	if (TestEnum(ENUM2))\n"
"	{\n"
"		string s = 'Enum success!';\n"
"		print(s);\n"
"		return;\n"
"	}\n"
"	print('Failure');\n"
"}\n"
;

int TestNalin()
{
	RET_ON_MAX_PORT

	asIScriptEngine   *engine;
	int r;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterStdString(engine);

	// Register the enum value
	r = engine->RegisterEnum("TEST_ENUM"); assert(r >= 0);
	r = engine->RegisterEnumValue("TEST_ENUM", "ENUM1", (int)ENUM1); assert(r >= 0);
	r = engine->RegisterEnumValue("TEST_ENUM", "ENUM2", (int)ENUM2); assert(r >= 0);
	r = engine->RegisterEnumValue("TEST_ENUM", "ENUM3", (int)ENUM3); assert(r >= 0);

	// Register some functions.
	r = engine->RegisterGlobalFunction("bool TestEnum(TEST_ENUM)", asFUNCTION(TestEnum), asCALL_CDECL); assert(r >= 0);
	//r = engine->RegisterGlobalFunction("bool TestEnum(int)", asFUNCTION(TestEnum2), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert(r >= 0);

	// Build our module.
	asIScriptModule* module = engine->GetModule("Test", asGM_ALWAYS_CREATE);
	module->AddScriptSection("Section1", script.c_str());
	module->Build();

	// Get our function.
	asIScriptFunction *update = module->GetFunctionByDecl("void Update()");

	// Prepare and execute the context.
	asIScriptContext* context = engine->CreateContext();
	context->Prepare(update);
	context->Execute();

	// Clean up.
	context->Release();
	engine->DiscardModule("Test");
	engine->Release();

	return 0;
}
}


bool Test()
{
	bool fail = false;
	fail = TestEnum()         || fail;
	fail = Nalin::TestNalin() || fail;
	return fail;
}



} // namespace
