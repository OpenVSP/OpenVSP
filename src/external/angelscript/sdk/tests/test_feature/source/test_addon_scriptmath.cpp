#include "utils.h"
#include "../../../add_on/scriptmath/scriptmath.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"

namespace Test_Addon_ScriptMath
{

static const char * const TESTNAME = "TestAddonScriptMath";

static const char *script =
"complex TestComplex()  \n"
"{                      \n"
"  complex v;           \n"
"  v.r=1;               \n"
"  v.i=2;               \n"
"  return v;            \n"
"}                      \n"
"complex TestComplexVal(complex v)  \n"
"{                                  \n"
"  return v;                        \n"
"}                                  \n"
"void TestComplexRef(complex &out v)\n"
"{                                  \n"
"  v.r=1;                           \n"
"  v.i=2;                           \n"
"}                                  \n";

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	COutStream out;
	CBufferedOutStream bout;
	int r;
	asIScriptEngine *engine = 0;


	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptMath(engine);
	RegisterScriptMathComplex(engine);
	RegisterScriptArray(engine, false);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	r = ExecuteString(engine, "float a = 1; uint b = fpToIEEE(a); assert( b == 0x3f800000 );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	r = ExecuteString(engine, "uint64 a = 0x3ff0000000000000; double b = fpFromIEEE(a); assert( b == 1.0 );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test closeTo
	r = ExecuteString(engine, "float a = 0; for( int i = 0; i < 100; i++ ) a += 3.14f; \n"
		                      "assert( a != 314.0f ); \n"
							  "assert( closeTo(a, 314.0f) );");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "assert( closeTo(67329.242f, 67329.234f) ); \n"
	                          "assert( !closeTo(67329.f, 67331.f) ); \n");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ExecuteString(engine, "assert( closeTo(1e-6f, 0) ); \n"
	                          "assert( closeTo(0, 1e-11) ); \n");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test implicit conversion from float to complex
	r = ExecuteString(engine, "complex c = 3.14f; assert( c.r == 3.14f && c.i == 0 );");
	if (r != asEXECUTION_FINISHED)
		TEST_FAILED;

	// Test initialization list for value type in local variable
	r = ExecuteString(engine, 
		"complex a = {1, 2}; \n"
		"assert( a.r == 1 ); \n"
		"assert( a.i == 2 ); \n");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test initialization list for value type in global variable
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, 
		"complex g = {1,2}; \n");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	Complex *g = (Complex*)mod->GetAddressOfGlobalVar(0);
	if( g == 0 || g->r != 1 || g->i != 2 )
		TEST_FAILED;

	// Test initialization list for value type in class member
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME,
		"class T { complex m = {1,2}; } \n"
		"T g; \n");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	Complex *m = (Complex*)((asIScriptObject*)mod->GetAddressOfGlobalVar(0))->GetAddressOfProperty(0);
	if( m == 0 || m->r != 1 || m->i != 2 )
		TEST_FAILED;

	// Test initialization list for value type in initialization list
	r = ExecuteString(engine, "array<complex> a = {{1,2}, {3,4}}; \n"
							  "assert( a[0].r == 1 ); \n"
							  "assert( a[1].r == 3 ); \n");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	
	// Test the complex math add-on
	Complex v;
	engine->RegisterGlobalProperty("complex v", &v);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script);
	r = mod->Build();
	if( r < 0 )
	{
		PRINTF("%s: Failed to build\n", TESTNAME);
		TEST_FAILED;
	}
	else
	{
		// Internal return
		r = ExecuteString(engine, "v = TestComplex();", mod);
		if( r < 0 )
		{
			PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
			TEST_FAILED;
		}
		if( v.r != 1 || v.i != 2 )
		{
			PRINTF("%s: Failed to assign correct Complex\n", TESTNAME);
			TEST_FAILED;
		}

		// Manual return
		v.r = 0; v.i = 0;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByDecl("complex TestComplex()"));

		ctx->Execute();
		Complex *ret = (Complex*)ctx->GetReturnObject();
		if( ret->r != 1 || ret->i != 2 )
		{
			PRINTF("%s: Failed to assign correct Complex\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Prepare(mod->GetFunctionByDecl("complex TestComplexVal(complex)"));
		v.r = 3; v.i = 2;
		ctx->SetArgObject(0, &v);
		ctx->Execute();
		ret = (Complex*)ctx->GetReturnObject();
		if( ret->r != 3 || ret->i != 2 )
		{
			PRINTF("%s: Failed to pass Complex by val\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Prepare(mod->GetFunctionByDecl("void TestComplexRef(complex &out)"));
		ctx->SetArgObject(0, &v);
		ctx->Execute();
		if( v.r != 1 || v.i != 2 )
		{
			PRINTF("%s: Failed to pass Complex by ref\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Release();
	}

	// Assignment of temporary object
	r = ExecuteString(engine, "complex v; \n"
		                      "float r = (v = complex(10.0f,7)).r; \n"
							  "assert( r > 9.9999f && r < 10.0001f );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test some operator overloads
	r = ExecuteString(engine, "complex v(1,0); assert( (v*complex(2,0)).abs() == 2 );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "complex v(1,0); assert( (complex(2,0)*v).abs() == 2 );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "complex v(1,0); assert( (v+v).abs() == 2 );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "complex v(1,0); assert( v == complex(1,0) );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "complex v(1,0); assert( (v *= complex(2)).abs() == 2 );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that implicit conversion to the complex type works
	r = ExecuteString(engine, "complex v(1,1); v += 2; assert( v == complex(3,1) ); v = v - 3; assert( abs(v.r - 0) < 0.0001f );");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test error message when constructor is not found
	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "complex v = complex(4,3,2,1);");
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	// TODO: the function signature for the constructors/factories should carry the name of the object instead of $beh0
	if( bout.buffer != "ExecuteString (1, 13) : Error   : No matching signatures to 'complex(const int, const int, const int, const int)'\n"
					   "ExecuteString (1, 13) : Info    : Candidates are:\n"
					   "ExecuteString (1, 13) : Info    : complex::complex()\n"
				   	   "ExecuteString (1, 13) : Info    : complex::complex(const complex&in)\n"
					   "ExecuteString (1, 13) : Info    : complex::complex(float)\n"
					   "ExecuteString (1, 13) : Info    : complex::complex(float, float)\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// Test use of anonymous list construct
	{
		bout.buffer = "";
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptMathComplex(engine);
		RegisterStdString(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"addAttachment({1, 1});"
			"} \n"
			"void addAttachment(complex) {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	return fail;
}

}

