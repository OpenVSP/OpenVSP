//
// This test was designed to test the engine property asEP_DISABLE_INTEGER_DIVISION
// 
// Author: Jason Goepel
// 

#include "utils.h"

namespace TestPropIntegerDivision
{

static const char* const TESTNAME = "TestPropIntegerDivision";

const char *script_on =
"void test_intdiv()                              \n"  // line 01
"{                                               \n"  // line 02
"   assert(1/2 == 0);                            \n"  // line 03
"                                                \n"  // line 04
"   int a = 3;                                   \n"  // line 05
"   int b = 4;                                   \n"  // line 06
"   int c = a / b;                               \n"  // line 07
"   double d = a / b;                            \n"  // line 08
"                                                \n"  // line 09
"   assert(c == 0);                              \n"  // line 10
"   assert(d == 0.0);                            \n"  // line 11
"}                                               \n"; // line 12

const char *script_off =
"void test_intdiv()                              \n"  // line 01
"{                                               \n"  // line 02
"   assert(1/2 == 1.0/2.0);                      \n"  // line 03
"                                                \n"  // line 04
"   int a = 3;                                   \n"  // line 05
"   int b = 4;                                   \n"  // line 06
"   int c = int(a / b);                          \n"  // line 07
"   double d = a / b;                            \n"  // line 08
"                                                \n"  // line 09
"   assert(3 / 4 == 3.0 / 4.0);                  \n"  // line 10
"   assert(c == 0);                              \n"  // line 11
"   assert(d == double(a) / double(b));          \n"  // line 12
"}                                               \n"; // line 13

bool Test()
{
	COutStream bout;
	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &bout, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// Default: engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, false);
	asIScriptModule *mod_on = engine->GetModule("intdiv_on", asGM_ALWAYS_CREATE);
	mod_on->AddScriptSection("script", script_on, strlen(script_on));
	r = mod_on->Build();
	if( r < 0 )
		TEST_FAILED;

	engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, true);
	asIScriptModule *mod_off = engine->GetModule("intdiv_off", asGM_ALWAYS_CREATE);
	mod_off->AddScriptSection("script", script_off, strlen(script_off));
	r = mod_off->Build();
	if( r < 0 )
		TEST_FAILED;

	asIScriptFunction *func_on, *func_off;
	func_on = mod_on->GetFunctionByName("test_intdiv");
	func_off = mod_off->GetFunctionByName("test_intdiv");

	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(func_on);
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	ctx->Prepare(func_off);
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	ctx->Release();
	engine->Release();

	return fail;
}

}  // namespace
