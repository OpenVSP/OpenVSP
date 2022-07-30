// 
// Test designed to verify functionality of the dynamically growing stack
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestStack";


bool TestStack()
{
	bool fail = false;
	COutStream out;

	// Test limiting the data stack size
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"void recursive(int n) \n"
			"{                     \n"
			"  if( n > 0 )         \n"
			"    recursive(n - 1); \n"
			"}                     \n");
		int r = mod->Build();
		if (r < 0)
		{
			PRINTF("%s: Failed to build script\n", TESTNAME);
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		engine->SetEngineProperty(asEP_INIT_STACK_SIZE, 256); // 256 byte initial size
		engine->SetEngineProperty(asEP_MAX_STACK_SIZE, 256); // 256 byte limit
		ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void recursive(int)"));
		ctx->SetArgDWord(0, 100);
		r = ctx->Execute();
		if (r != asEXECUTION_EXCEPTION)
		{
			PRINTF("%s: Execution didn't throw an exception as was expected\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Release();
		engine->Release();
	}

	// Test limiting the call stack size
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"void recursive(int n) \n"
			"{                     \n"
			"  if( n > 0 )         \n"
			"    recursive(n - 1); \n"
			"}                     \n");
		int r = mod->Build();
		if (r < 0)
		{
			PRINTF("%s: Failed to build script\n", TESTNAME);
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		engine->SetEngineProperty(asEP_INIT_CALL_STACK_SIZE, 10); // 10 nested calls
		engine->SetEngineProperty(asEP_MAX_CALL_STACK_SIZE, 10);
		ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void recursive(int)"));
		ctx->SetArgDWord(0, 100);
		r = ctx->Execute();
		if (r != asEXECUTION_EXCEPTION)
		{
			PRINTF("%s: Execution didn't throw an exception as was expected\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Release();
		engine->Release();
	}

	return fail;
}
