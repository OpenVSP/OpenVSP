//
// Tests compiling a module and then discarding it
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestDiscard
{

static const char * const TESTNAME = "TestDiscard";



static const char *script1 =
"void Test()                  \n"
"{                            \n"
"    uint8[] kk(10);          \n"
"    uint8[] kk2(10);         \n"
"    func(kk, kk2, 10, 100);  \n" 
"}                            \n";


static void func(asIScriptGeneric *)
{
}



bool Test()
{
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	r = engine->RegisterGlobalFunction("uint8 func(uint8[] &in, uint8[] &inout, uint8, uint32)", asFUNCTION(func), asCALL_GENERIC); assert( r >= 0 ); 

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void Test()"));
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Prepare the context again, but don't execute it to see if the cleanup is proper
	ctx->Prepare(mod->GetFunctionByDecl("void Test()"));
	ctx->Release();

	engine->DiscardModule(0);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	engine->Release();

	// Success
	return fail;
}

} // namespace


