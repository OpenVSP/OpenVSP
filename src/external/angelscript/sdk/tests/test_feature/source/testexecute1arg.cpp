//
// Tests calling of a c-function from a script with one parameter
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute1Arg";

static int testVal = 0;
static bool called = false;

static void cfunction(int f1) 
{
	called = true;
	testVal = f1;
}

static void cfunction_gen(asIScriptGeneric*gen)
{
	called = true;
	testVal = (int)gen->GetArgDWord(0);
}

bool TestExecute1Arg()
{
	bool fail = false;
	int funcId;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		funcId = engine->RegisterGlobalFunction("void cfunction(int)", asFUNCTION(cfunction_gen), asCALL_GENERIC);
	else
		funcId = engine->RegisterGlobalFunction("void cfunction(int)", asFUNCTION(cfunction), asCALL_CDECL);

	ExecuteString(engine, "cfunction(5)");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( testVal != 5 ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got %d, expected %d\n\n", TESTNAME, testVal, 5);
		TEST_FAILED;
	}

	// Now try to call the function directly via a context
	testVal = 0;
	called = false;
	asIScriptContext *ctx = engine->CreateContext();

	r = ctx->Prepare(engine->GetFunctionById(funcId));
	if( r < 0 )
	{
		TEST_FAILED;
	}
	else
	{
		ctx->SetArgDWord(0, 5);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}
		else
		{
			if( !called ) 
			{
				// failure
				PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
				TEST_FAILED;
			} 
			else if( testVal != 5 ) 
			{
				// failure
				PRINTF("\n%s: testVal is not of expected value. Got %d, expected %d\n\n", TESTNAME, testVal, 5);
				TEST_FAILED;
			}
		}
	}

	ctx->Release();

	engine->Release();
	
	// Success
	return fail;
}
