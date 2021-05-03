//
// Test to verify that modules are released correctly after use
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestModuleRef";
static const char *script = "int global; void Test() {global = 0;}";

bool TestModuleRef()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	
	asIScriptModule *mod = engine->GetModule("a", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: failed to build module a\n", TESTNAME);
		TEST_FAILED;
	}

	asIScriptFunction *func = engine->GetModule("a")->GetFunctionByDecl("void Test()");
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(func);

	if( engine->GetModule("a")->GetFunctionCount() < 0 )
	{
		PRINTF("%s: Failed to get function count\n", TESTNAME);
		TEST_FAILED;
	}

	engine->DiscardModule("a");
	if( engine->GetModule("a") )
	{
		PRINTF("%s: Module was not discarded\n", TESTNAME);
		TEST_FAILED;
	} 

	int r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: Execution failed\n", TESTNAME);
		TEST_FAILED;
	}

	ctx->Release();

	engine->Release();

	return fail;
}
