#include "utils.h"
using std::string;
#include "../../../add_on/scriptmath/scriptmath.h"

namespace TestScriptMath
{

const char *script =
"void test()                        \n"
"{                                  \n"
"   assert(abs(-1) == 1);           \n"
"   assert(sin(0) == 0);            \n"
"   assert(cos(0) == 1);            \n"
"   assert(tan(0) == 0);            \n"
"   assert(asin(0) == 0);           \n"
"   assert(acos(1) == 0);           \n"
"   assert(atan(0) == 0);           \n"
"   atan2(1,1);                     \n"
"   sinh(0);                        \n"
"   cosh(0);                        \n"
"   tanh(0);                        \n"
"   assert(fraction(1.1f) >= 0.000009f && \n"
"          fraction(1.1f) <= 0.100001f);  \n"
"   log(0);                         \n"
"   log10(0);                       \n"
"   pow(1,1);                       \n"
"   sqrt(1);                        \n"
"   ceil(1.1f);                      \n"
"   floor(1.1f);                     \n"
"}                                  \n";


bool Test()
{
	bool fail = false;
	COutStream out;
	int r;

	//----------------------
	// Make sure the native interface works (where supported)
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptMath(engine);

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "test()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	//------------------
	// Make sure the generic interface works as well
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptMath_Generic(engine);

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "test()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}

} // namespace

