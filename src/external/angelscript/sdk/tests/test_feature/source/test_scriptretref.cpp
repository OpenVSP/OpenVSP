#include "utils.h"

namespace TestScriptRetRef
{

bool Test()
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;
	asIScriptEngine *engine;
	asIScriptModule *mod;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	RegisterScriptString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	// Must be allowed to return 'this' from a class method
	{
		bout.buffer = "";
		const char *script = 
			"class C \n"
			"{ \n"
			"  C &opAssign(const C &in o) \n"
			"  { \n"
			"    return this; \n"
			"  } \n"
			"  int n; \n"
			"} \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "C a, b; (a = b).n = 42; assert( a.n == 42 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	}

	// Test returning reference to a global variable
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"int g;\n"
			"int &Test()\n"
			"{\n"
			"  return g;\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "g = 0; Test() = 42; assert( g == 42 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to a global variable
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"string g;\n"
			"string &Test()\n"
			"{\n"
			"  return g;\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "Test() = '42'; assert( g == '42' );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	}

	// Test returning reference to a global variable
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"string @g;\n"
			"string@ &Test()\n"
			"{\n"
			"  return g;\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;

		// Test by making a explicit copy
		r = ExecuteString(engine, "const string s = '42'; @Test() = @string(s); assert( g == '42' );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Test by making a explicit copy
		r = ExecuteString(engine, "@Test() = @string('42'); assert( g == '42' );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Implicit handle
		r = ExecuteString(engine, "@Test() = '42'; assert( g == '42' );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Explicit handle. Once the explicit handle has been taken the string is const, so it cannot be assigned
		bout.buffer = "";
		r = ExecuteString(engine, "@Test() = @'42'; assert( g == '42' );", mod);
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "ExecuteString (1, 11) : Error   : Can't implicitly convert from 'const string@' to 'string@'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to a class member
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"class A { \n"
			"  int g;\n"
			"  int &Test()\n"
			"  {\n"
			"    return g;\n"
			"  }\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "A a; a.g = 0; a.Test() = 42; assert( a.g == 42 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	}

	// Test returning reference to a class member
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"class A { \n"
			"  string g;\n"
			"  string &Test()\n"
			"  {\n"
			"    return g;\n"
			"  }\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;

		r = ExecuteString(engine, "A a; a.g = ''; a.Test() = '42'; assert( a.g == '42' );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to a class member
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"class A { \n"
			"  string @g;\n"
			"  string @&Test()\n"
			"  {\n"
			"    return g;\n"
			"  }\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;

		r = ExecuteString(engine, "A a; @a.Test() = '42'; assert( a.g == '42' );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to a global variable
	// This should work, as the global variable is guaranteed to be there even after the function returns
	{
		bout.buffer = "";
		const char *script =
			"string g;\n"
			"string &Test()\n"
			"{\n"
			"  return g;\n"
			"}\n"
			"string &Test2()\n"
			"{\n"
			"  return Test();\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "Test2() = '42'; assert( g == '42' );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	}

	// This should work as the parameter is a primitive and doesn't need cleanup after the expression.
	{
		bout.buffer = "";
		const char *script =
			"string g;\n"
			"string &Test(int s)\n"
			"{\n"
			"  return g;\n"
			"}\n"
			"string &Test2()\n"
			"{\n"
			"  return Test(1);\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Make sure a reference to the object is held by the caller so that the callee doesn't destroy
	// itself before returning the reference. For example if the object is stored in a global variable
	// and the class resets the global variable.
	{
		bout.buffer = "";
		const char *script =
			"class Test\n"
			"{ \n"
			"  string &t()\n"
			"  { \n"
			"    @g = null; \n"  
			"    return m; \n"
			"  } \n"
			"  string m; \n"
			"} \n"
			"Test @g; \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "@g = Test(); g.m = 'test'; assert( g.t() == 'test' );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	}

	// Objects in the function must be cleaned up before the return expression 
	// is evaluated to make sure they do not invalidate the reference.
	{
		bout.buffer = "";
		const char *script =
			"Val @g; \n"
			"class Cleanup \n"
			"{ \n"
			"  ~Cleanup() \n"
			"  { \n"
			"    @g = null; \n"
			"  } \n"
			"} \n"
			"class Val { string v; } \n"
			"string &Test()\n"
			"{\n"
			"  Cleanup c(); \n"
			"  @g = Val(); \n"
			"  g.v = 'test'; \n"
			"  return g.v; \n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// This should give a null pointer exception, since the Cleanup is supposed   
		// to clear the global variable before the return expression is evaluated
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "assert( Test() == 'test' );", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
		{
			TEST_FAILED;
		}
		else if( std::string(ctx->GetExceptionString()) != "Null pointer access" )
		{
			PRINTF("%s", ctx->GetExceptionString());
			TEST_FAILED;
		}
		ctx->Release();
	}

	// The return expression must not use any local variables or temporaries that will be 
	// destroyed after the expression is completed.
	{
		bout.buffer = "";
		const char *script =
			"uint8 &Test2()\n"
			"{\n"
			"  string s;\n"
			"  return s[1];\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "script (1, 1) : Info    : Compiling uint8& Test2()\n"
		                   "script (4, 3) : Error   : Resulting reference cannot be returned. The expression uses objects that during cleanup may invalidate it.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// This should not work, as the output parameter is evaluated after the reference is 
	// is returned from Test, but before it is returned from Test2.
	{
		bout.buffer = "";
		const char *script =
			"string g;\n"
			"string &Test(int &out s)\n"
			"{\n"
			"  return g;\n"
			"}\n"
			"string &Test2()\n"
			"{\n"
			"  int s;\n"
			"  return Test(s);\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "script (6, 1) : Info    : Compiling string& Test2()\n"
		                   "script (9, 3) : Error   : Resulting reference cannot be returned. There are deferred arguments that may invalidate it.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// This should not work, as the input parameter is cleaned up after the 
	// reference is returned from Test, which may invalidate the reference
	{
		bout.buffer = "";
		const char *script =
			"string g;\n"
			"string &Test(const string &in s)\n"
			"{\n"
			"  return g;\n"
			"}\n"
			"string &Test2()\n"
			"{\n"
			"  string s;\n"
			"  return Test(s);\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "script (6, 1) : Info    : Compiling string& Test2()\n"
		                   "script (9, 3) : Error   : Resulting reference cannot be returned. There are deferred arguments that may invalidate it.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning a constant
	// This should fail to compile because the expression is not a reference
	{
		bout.buffer = "";
		const char *script =
			"int &Test()\n"
			"{\n"
			"  return 42;\n"
			"}\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "script (1, 1) : Info    : Compiling int& Test()\n"
						   "script (3, 3) : Error   : Not a valid reference\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to a parameter
	// This should fail to compile because the origin cannot be guaranteed 
	{
		bout.buffer = "";
		const char *script19 =
			"class Object {}\n"
			"Object &Test(Object &in object)\n"
			"{\n"
			"  return object;\n"
			"}\n";
		mod->AddScriptSection("script19", script19, strlen(script19));
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "script19 (2, 1) : Info    : Compiling Object& Test(Object&in)\n"
						   "script19 (4, 3) : Error   : Can't return reference to local value.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to local variable with pre-increment
	// This should not work, because the resulting reference is still pointing to the local variable
	{
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		bout.buffer = "";
		const char *script25 = "int &SomeFunc() { int a = 0; return ++a; }";
		r = mod->AddScriptSection("25", script25);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "25 (1, 1) : Info    : Compiling int& SomeFunc()\n"
						   "25 (1, 30) : Error   : Can't return reference to local value.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning reference to temporary object
	// This should fail to compile because the reference is to a temporary variable
	{
		bout.buffer = "";
		const char *script25 = "string &SomeFunc() { return ''; }";
		r = mod->AddScriptSection("25", script25);
		r = mod->Build();
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "25 (1, 1) : Info    : Compiling string& SomeFunc()\n"
						   "25 (1, 22) : Error   : Can't implicitly convert from 'const string' to 'string&'.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test returning const reference to temporary object
	// This should be allowed because the string constant is global
	{
		bout.buffer = "";
		const char *script = "const string &SomeFunc() { return ''; }";
		r = mod->AddScriptSection("25", script);
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		if (bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	engine->Release();

	// Success
	return fail;
}

} // namespace

