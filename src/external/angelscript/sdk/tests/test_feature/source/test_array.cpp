#include "utils.h"

namespace TestArray
{

static const char * const TESTNAME = "TestArray";

static const char *script1 =
"string[] b;                                     \n"
"int[] g_a(3);                                   \n"
"void TestArray()                                \n"
"{                                               \n"
"   string[] a(5);                               \n"
"   Assert(a.length() == 5);                     \n"
"   a.resize(10);                                \n"
"   a.resize(5);                                 \n"
"   a[0] = \"Hello\";                            \n"
"   Assert(a[0] == \"Hello\");                   \n"
"   uint n = 0;                                  \n"
"   Assert(a[n] == \"Hello\");                   \n"
"   n++;                                         \n"
"   Assert(a[n] == \"\");                        \n"
"   b = a;                                       \n"
"   Assert(b.length() == 5);                     \n"
"   Assert(b[0] == \"Hello\");                   \n"
"   b[0] = \"Goodbye\";                          \n"
"   Assert(a[0] != \"Goodbye\");                 \n"
"   int[] ia = TestArray4();                     \n"
"   TestArray2(ia);                              \n"
"   TestArray3(ia);                              \n"
"   ia = int[](3);                               \n"
"   Assert(ia.length() == 3);                    \n"
"   ia[0] = 1;                                   \n"
"   int[] ib = ia;                               \n"
"   Assert(ib.length() == ia.length());          \n"
"   Assert(ib[0] == ia[0]);                      \n"
"}                                               \n"
"void TestArray2(int[] &inout a)                 \n"
"{                                               \n"
"   Assert(a[0] == 1);                           \n"
"   Assert(a[1] == 2);                           \n"
"   Assert(a[2] == 3);                           \n"
"}                                               \n"
"void TestArray3(int[] a)                        \n"
"{                                               \n"
"   Assert(a[0] == 1);                           \n"
"   Assert(a[1] == 2);                           \n"
"   Assert(a[2] == 3);                           \n"
"}                                               \n"
"int[] TestArray4()                              \n"
"{                                               \n"
"   int[] ia(3);                                 \n"
"   ia[0] = 1;                                   \n"
"   ia[1] = 2;                                   \n"
"   ia[2] = 3;                                   \n"
"   return ia;                                   \n"
"}                                               \n";

static const char *script2 = 
"void TestArrayException()                       \n"
"{                                               \n"
"   string[] a;                                  \n"
"   a[0] == \"Hello\";                           \n"
"}                                               \n";

static const char *script3 = 
"void TestArrayMulti()                           \n"
"{                                               \n"
"   int[][] a(2);                                \n"
"   int[] b(2);                                  \n"
"   a[0] = b;                                    \n"
"   a[1] = b;                                    \n"
"                                                \n"
"   a[0][0] = 0;                                 \n"
"   a[0][1] = 1;                                 \n"
"   a[1][0] = 2;                                 \n"
"   a[1][1] = 3;                                 \n"
"                                                \n"
"   Assert(a[0][0] == 0);                        \n"
"   Assert(a[0][1] == 1);                        \n"
"   Assert(a[1][0] == 2);                        \n"
"   Assert(a[1][1] == 3);                        \n"
"}                                               \n";

static const char *script4 = 
"void TestArrayChar()                            \n"
"{                                               \n"
"   int8[] a(2);                                 \n"
"   a[0] = 13;                                   \n"
"   a[1] = 19;                                   \n"
"                                                \n"
"   int8 a0 = a[0];                              \n"
"   int8 a1 = a[1];                              \n"
"   Assert(a[0] == 13);                          \n"
"   Assert(a[1] == 19);                          \n"
"}                                               \n";

static const char *script5 = 
"int[] g = {1,2,3};                              \n"
"void TestArrayInitList()                        \n"
"{                                               \n"
"   Assert(g.length() == 3);                     \n"
"   Assert(g[2] == 3);                           \n"
"   int[] a = {,2,};                             \n"
"   Assert(a.length() == 3);                     \n"
"   Assert(a[1] == 2);                           \n"
"   string[] b = {\"test\", \"3\"};              \n"
"   Assert(b.length() == 2);                     \n"
"   Assert(b[0] == \"test\");                    \n"
"   Assert(b[1] == \"3\");                       \n"
"   int[][] c = {,{23},{23,4},};                 \n"
"   Assert(c.length() == 4);                     \n"
"   Assert(c[2].length() == 2);                  \n"
"   Assert(c[2][1] == 4);                        \n"
"   const int[] d = {0,1,2};                     \n"
"   Assert(d.length() == 3);                     \n"
"   Assert(d[2] == 2);                           \n"
"}                                               \n";

static const char *script6 =
"void Test()                                     \n"
"{                                               \n"
"   int[]@ e = {2,5};                            \n"
"   int[] f = {,{23}};                           \n"
"}                                               \n";

static const char *script7 =
"class TestC                                     \n"
"{                                               \n"
"  TestC() {count++; s = \"test\";}              \n"
"  string s;                                     \n"
"}                                               \n"
"int count = 0;                                  \n"
"void Test()                                     \n"
"{                                               \n"
"  TestC t;                                      \n"
"  Assert(count == 1);                           \n"
"  TestC[] arr(5);                               \n"
"  Assert(count == 6);                           \n"
"}                                               \n";

bool Test2();

bool Test()
{
	bool fail = Test2();
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptContext *ctx;
	
	// Test implicit conversion from anonymous init list to array arg with &inout
	// Reported by Max Waine
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void StartScript(array<int> & args) \n"
			"{ \n"
			"} \n"
			"void main_anonlist() \n"
			"{ \n"
			"	StartScript({1,2,3}); \n"
			"	StartScript(array<int> = {1,2,3}); \n" // same output as statement above, just more explicit
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		engine->ShutDownAndRelease();
	}

	// Test to make sure opIndex properly catches null pointer access
	// http://www.gamedev.net/topic/676729-crash-instead-of-null-pointer-exception-on-opindex/
	{
		asIScriptEngine *engine = asCreateScriptEngine();

		RegisterScriptArray(engine, true);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "array<array<int>@> a = {null}; a[0][0] = 1;", 0, ctx);
		if (r != asEXECUTION_EXCEPTION)
			TEST_FAILED;
		else if( GetExceptionInfo(ctx) != "func: void ExecuteString()\n"
										  "modl: \n"
										  "sect: ExecuteString\n"
										  "line: 1\n"
										  "desc: Null pointer access\n" )
		{
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
			TEST_FAILED;
		}
		ctx->Release();
		engine->ShutDownAndRelease();
	}

	// Test GetTypeDeclaration with arrays
	// http://www.gamedev.net/topic/663428-simplest-way-to-get-variable-type/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterScriptArray(engine, true);

		int typeId = engine->GetTypeIdByDecl("array<array<int>@>");
		if( typeId < 0 )
			TEST_FAILED;

		std::string typeDecl = engine->GetTypeDeclaration(typeId, true);
		if( typeDecl != "int[]@[]" )
		{
			PRINTF("%s\n", typeDecl.c_str());
			TEST_FAILED;
		}

		engine->SetDefaultNamespace("foo");
		engine->RegisterEnum("MyEnum");
		engine->SetDefaultNamespace("");

		typeId = engine->GetTypeIdByDecl("array<foo::MyEnum>");
		if( typeId < 0 )
			TEST_FAILED;

		typeDecl = engine->GetTypeDeclaration(typeId, true);
		if( typeDecl != "foo::MyEnum[]" )
		{
			PRINTF("%s\n", typeDecl.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		RegisterScriptString_Generic(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);


		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestArray()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			PRINTF("%s: Failed to execute script\n", TESTNAME);
			TEST_FAILED;
		}
		if( ctx ) ctx->Release();

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		r = ExecuteString(engine, "TestArrayException()", mod);
		if( r != asEXECUTION_EXCEPTION )
		{
			PRINTF("%s: No exception\n", TESTNAME);
			TEST_FAILED;
		}

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestArrayMulti()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			PRINTF("%s: Failure\n", TESTNAME);
			TEST_FAILED;
		}
		if( r == asEXECUTION_EXCEPTION )
		{
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}
		if( ctx ) ctx->Release();
		ctx = 0;

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script4, strlen(script4), 0);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestArrayChar()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			PRINTF("%s: Failure\n", TESTNAME);
			TEST_FAILED;
		}
		if( r == asEXECUTION_EXCEPTION )
		{
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}

		if( ctx ) ctx->Release();

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script5, strlen(script5), 0);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestArrayInitList()", mod, ctx);
		if( r != asEXECUTION_FINISHED ) TEST_FAILED;
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());

		if( ctx ) ctx->Release();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script6, strlen(script6), 0);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "TestArray (1, 1) : Info    : Compiling void Test()\n"
						   "TestArray (4, 16) : Error   : Initialization lists cannot be used with 'int'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Array object must call default constructor of the script classes
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script7, strlen(script7), 0);
		r = mod->Build();
		if( r < 0 ) 
			TEST_FAILED;
		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
			
		// Test bool[] on Mac OS X with PPC CPU
		// Submitted by Edward Rudd
		const char *script8 =
		"bool[] f(10);              \n"
		"for (int i=0; i<10; i++) { \n"
		"	f[i] = false;           \n"
		"}                          \n"
		"Assert(f[0] == false);     \n"
		"Assert(f[1] == false);     \n"
		"f[0] = true;               \n"
		"Assert(f[0] == true);      \n"
		"Assert(f[1] == false);     \n";
		
		r = ExecuteString(engine, script8);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Make sure it is possible to do multiple assignments with the array type
		r = ExecuteString(engine, "int[] a, b, c; a = b = c;");
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test circular reference between types
	{
		// Create the script engine
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);

		// Compile
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"class Hoge"
			"{"
			"    Hoge(){}"
			"    Hoge(HogeManager&){}"
			"};"
			"class HogeManager"
			"{"
			"    Hoge[] hoges;"
			"};"
			, 0);
		mod->Build();

		// Release engine
		engine->Release();
	}

	// Test multidimensional array initialization
	{
		// Create the script engine
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterScriptArray(engine, true);

		r = ExecuteString(engine, "int[][] a(2, int[](2)); assert(a[1].length() == 2);\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Release engine
		engine->Release();
	}

	// Test array of void
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptArray(engine, false);
		r = ExecuteString(engine, "array<void> a;");
		if( r != -1 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 7) : Error   : Attempting to instantiate invalid template type 'array<void>'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}

bool Test2()
{
	bool fail = false;

	const char *script =
	"class A                               \n"
	"{                                     \n"
	"	int x;                             \n"
	"}                                     \n"
	"int sum(const A[]& a)                 \n"
	"{                                     \n"
	"	int s = 0;                         \n"
	"	for (uint i=0; i<a.length(); i++)  \n"
	"		s+=a[i].x;                     \n"
	"	return s;                          \n"
	"}                                     \n";

	const char *exec =
	"A[] As;       \n"
    "As.resize(2); \n"
	"As[0].x = 1;  \n"
	"As[1].x = 2;  \n"
	"sum(As);      \n";

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);

	asIScriptModule *module = engine->GetModule("module", asGM_ALWAYS_CREATE);

	module->AddScriptSection("script", script);
	int r = module->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, exec, module);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

} // namespace

