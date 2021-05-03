//
// Tests calling of a c-function from a script with four parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute4Args";

static bool testVal = false;
static bool called  = false;

static int	 t1 = 0;
static short t2 = 0;
static char	 t3 = 0;
static int	 t4 = 0;

static void cfunction(int f1, short f2, char f3, int f4)
{
	called = true;
	t1 = f1;
	t2 = f2;
	t3 = f3;
	t4 = f4;
	testVal = (f1 == 5) && (f2 == 9) && (f3 == 1) && (f4 == 3);
}

static void cfunction_gen(asIScriptGeneric *gen)
{
	called = true;
	t1 = gen->GetArgDWord(0);
	t2 = *(short*)gen->GetAddressOfArg(1);
	t3 = *(char*)gen->GetAddressOfArg(2);
	t4 = gen->GetArgDWord(3);
	testVal = (t1 == 5) && (t2 == 9) && (t3 == 1) && (t4 == 3);
}

bool TestExecute4Args()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("void cfunction(int, int16, int8, int)", asFUNCTION(cfunction_gen), asCALL_GENERIC);
	else
		engine->RegisterGlobalFunction("void cfunction(int, int16, int8, int)", asFUNCTION(cfunction), asCALL_CDECL);

	ExecuteString(engine, "cfunction(5, 9, 1, 3)");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( !testVal ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got (%d, %d, %d, %d), expected (%d, %d, %d, %d)\n\n", TESTNAME, t1, t2, t3, t4, 5, 9, 1, 3);
		TEST_FAILED;
	}
	
	engine->Release();
	
	// Success
	return fail;
}
