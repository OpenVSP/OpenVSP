//
// Tests calling of a c-function from a script with two parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute2Args";

static int testVal = 0;
static bool called = false;

static void cfunction(int f1, int f2) 
{
	called = true;
	testVal = f1 + f2;
}

static void cfunction_gen(asIScriptGeneric *gen) 
{
	called = true;
	testVal = (int)gen->GetArgDWord(0) + (int)gen->GetArgDWord(1);
}

bool TestExecute2Args()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("void cfunction(int, int)", asFUNCTION(cfunction_gen), asCALL_GENERIC);
	else
		engine->RegisterGlobalFunction("void cfunction(int, int)", asFUNCTION(cfunction), asCALL_CDECL);

	ExecuteString(engine, "cfunction(5, 9)");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( testVal != (5 + 9) ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got %d, expected %d\n\n", TESTNAME, testVal, (5 + 9));
		TEST_FAILED;
	}

	engine->Release();
	
	// Success
	return fail;
}
