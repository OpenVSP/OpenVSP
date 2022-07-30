//
// Tests calling of a c-function from a script
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute";

static bool called = false;

static void cfunction() {
	called = true;
}

static void cfunction_generic(asIScriptGeneric *) {
	cfunction();
}

static void cleanContext(asIScriptContext *ctx)
{
	assert(ctx->GetUserData() == (void*)(size_t)0xDEADF00D);
	called = true;
}

bool TestExecute()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(),"AS_MAX_PORTABILITY") )
	{
		int r = engine->RegisterGlobalFunction("void cfunction()", asFUNCTION(cfunction_generic), asCALL_GENERIC); assert( r >= 0 );
	}
	else
	{
		int r = engine->RegisterGlobalFunction("void cfunction()", asFUNCTION(cfunction), asCALL_CDECL); assert( r >= 0 );
	}
	ExecuteString(engine, "cfunction()");

	if (!called) {
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	}

	
	asIScriptContext *ctx = engine->CreateContext();
	void *p = ctx->SetUserData((void*)(size_t)0xDEADF00D); assert(p == 0);
	assert(ctx->GetUserData() == (void*)(size_t)0xDEADF00D);
	p = ctx->SetUserData(0); assert(p == (void*)(size_t)0xDEADF00D);
	p = ctx->SetUserData((void*)(size_t)0xDEADF00D); assert(p == 0);
	engine->SetContextUserDataCleanupCallback(cleanContext);
	called = false;
	// Prepare the context, but don't execute the function. The clean-up should work properly
	ctx->Prepare(engine->GetGlobalFunctionByDecl("void cfunction()"));
	ctx->Release();
	if( !called )
		TEST_FAILED;

	engine->Release();
	engine = NULL;

	return fail;
}
