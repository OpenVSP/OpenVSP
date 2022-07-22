#include "utils.h"
#include "../../add_on/scriptmath/scriptmathcomplex.h"

using namespace std;

namespace TestNamedArgs
{

bool Test()
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;
	COutStream out;
	asIScriptModule *mod;
	asIScriptEngine *engine;

	// Test invalid use of named args
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int a = int(a:0);\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test (1, 5) : Info    : Compiling int a\n"
			"test (1, 12) : Error   : Invalid use of named arguments\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test when named args isn't in the function signature
	{
		const char *script =
			"void func(int a) {\n"
			"}\n"
			"void test() {"
			"  func(b:4);\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (3, 1) : Info    : Compiling void test()\n"
						   "test (3, 16) : Error   : No matching signatures to 'func(b: const int)'\n"
						   "test (3, 16) : Info    : Candidates are:\n"
						   "test (3, 16) : Info    : void func(int a)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();

	}

	// Test named args with registered object behaviours
	// http://www.gamedev.net/topic/672959-problem-with-named-parameters-and-object-behaviors/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, false);

		r = ExecuteString(engine, "array<int> a( length: 10, value: 1 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test old syntax
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 1); // warn

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func(int val) {} \n"
			"void main() { \n"
			"  int val; \n"
			"  func(val = 42); \n" // named argument. should warn
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "test (2, 1) : Info    : Compiling void main()\n"
		                   "test (4, 12) : Warning : Detected named argument with old syntax\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//Test named arguments for global functions
	{
		const char *script =
			"bool func(int a, int b, int c) {\n"
			"  return a == 4 && b == 6 && c == 8;\n"
			"}\n"
			"\n"
			"void test() {"
			"  assert(func(4, 6, 8));\n"
			"  assert(func(4, 6, c: 8));\n"
			"  assert(func(a: 4, b: 6, c: 8));\n"
			"  assert(func(a: 4, c: 8, b: 6));\n"
			"  assert(func(c: 8, b: 6, a: 4));\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//Test named arguments combined with default values
	{
		const char *script =
			"bool func(int a = 4, int b = 6, int c = 8) {\n"
			"  return a == 4 && b == 6 && c == 8;\n"
			"}\n"
			"\n"
			"void test() {"
			"  assert(func());\n"
			"  assert(func(4, 6));\n"
			"  assert(func(a:4));\n"
			"  assert(func(b:6));\n"
			"  assert(func(c:8));\n"
			"  assert(func(c:8, a:4));\n"
			"  assert(func(a:4, b:6, c:8));\n"
			"  assert(func(a:4, c:8, b:6));\n"
			"  assert(func(c:8, b:6, a:4));\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//Test named arguments in methods
	{
		const char *script =
			"class Base {"
			"  bool func(int a = 4, int b = 6, int c = 8) {\n"
			"    return a == 4 && b == 6 && c == 8;\n"
			"  }\n"
			"}\n"
			"\n"
			"void test() {"
			"  Base o;\n"
			"  assert(o.func());\n"
			"  assert(o.func(4, 6));\n"
			"  assert(o.func(a:4));\n"
			"  assert(o.func(b:6));\n"
			"  assert(o.func(c:8));\n"
			"  assert(o.func(c:8, a:4));\n"
			"  assert(o.func(a:4, b:6, c:8));\n"
			"  assert(o.func(a:4, c:8, b:6));\n"
			"  assert(o.func(c:8, b:6, a:4));\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//Test having differently named arguments in overridden methods
	{
		const char *script =
			"class Base {"
			"  bool func(int a = 3, int b = 5, int c = 7) {\n"
			"    return a == 3 && b == 5 && c == 7;\n"
			"  }\n"
			"}\n"
			"\n"
			"class Derived : Base {"
			"  bool func(int x = 3, int y = 5, int z = 7) override {\n"
			"    return x == 3 && y == 5 && z == 7;\n"
			"  }\n"
			"}\n"
			"\n"
			"void test() {"
			"  Derived o;\n"
			"  assert(o.func());\n"
			"  assert(o.func(3, 5));\n"
			"  assert(o.func(x:3));\n"
			"  assert(o.func(y:5));\n"
			"  assert(o.func(z:7));\n"
			"  assert(o.func(z:7, x:3));\n"
			"  assert(o.func(x:3, y:5, z:7));\n"
			"  assert(o.func(x:3, z:7, y:5));\n"
			"  assert(o.func(z:7, y:5, x:3));\n"
			"  Base@ asBase = o;\n"
			"  assert(asBase.func());\n"
			"  assert(asBase.func(3, 5));\n"
			"  assert(asBase.func(a:3));\n"
			"  assert(asBase.func(b:5));\n"
			"  assert(asBase.func(c:7));\n"
			"  assert(asBase.func(c:7, a:3));\n"
			"  assert(asBase.func(a:3, b:5, c:7));\n"
			"  assert(asBase.func(a:3, c:7, b:5));\n"
			"  assert(asBase.func(c:7, b:5, a:3));\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//Test error message for positional arguments after named ones
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool func(int a, int b, int c) {\n"
			"  return a == 4 && b == 6 && c == 8;\n"
			"}\n"
			"\n"
			"void test() {"
			"  func(b:6, 4, 8);\n"
			"}\n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (5, 1) : Info    : Compiling void test()\n"
						   "test (5, 20) : Error   : Positional arguments cannot be passed after named arguments\n"
						   )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//Test that re-specifying positional arguments fails
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool func(int a, int b, int c) {\n"
			"  return a == 4 && b == 6 && c == 8;\n"
			"}\n"
			"\n"
			"void test() {"
			"  func(4, a:6);\n"
			"}\n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		//This needs to fail as a 'no matching signatures', because
		//the name of the positional argument may vary between different
		//possible matches.
		if( bout.buffer != "test (5, 1) : Info    : Compiling void test()\n"
						   "test (5, 16) : Error   : No matching signatures to 'func(const int, a: const int)'\n"
						   "test (5, 16) : Info    : Candidates are:\n"
						   "test (5, 16) : Info    : bool func(int a, int b, int c)\n"
						   )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//Test that re-specifying named arguments fails
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool func(int a, int b, int c) {\n"
			"  return a == 4 && b == 6 && c == 8;\n"
			"}\n"
			"\n"
			"void test() {"
			"  func(a:4, a:6);\n"
			"}\n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (5, 1) : Info    : Compiling void test()\n"
						   "test (5, 23) : Error   : Duplicate named argument\n"
						   )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}



} // namespace

