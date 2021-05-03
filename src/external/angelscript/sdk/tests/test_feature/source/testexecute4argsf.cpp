//
// Tests calling of a c-function from a script with four float parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute4Argsf";

static bool testVal = false;
static bool called  = false;

static float  t1 = 0;
static float  t2 = 0;
static double t3 = 0;
static float  t4 = 0;

static void cfunction(float f1, float f2, double f3, float f4)
{
	called = true;
	t1 = f1;
	t2 = f2;
	t3 = f3;
	t4 = f4;
	testVal = (f1 == 9.2f) && (f2 == 13.3f) && (f3 == 18.8) && (f4 == 3.1415f);
}

static void cfunction2(double f1, double f2, double f3, double f4)
{
	called = true;
	assert(f1 == 1337.0);
	assert(f2 == 1338.0);
	assert(f3 == 1339.0);
	assert(f4 == 1340.0);
}

static void cfunction_gen(asIScriptGeneric *gen)
{
	called = true;
	t1 = gen->GetArgFloat(0);
	t2 = gen->GetArgFloat(1);
	t3 = gen->GetArgDouble(2);
	t4 = gen->GetArgFloat(3);
	testVal = (t1 == 9.2f) && (t2 == 13.3f) && (t3 == 18.8) && (t4 == 3.1415f);
}


bool TestExecute4Argsf()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("void cfunction(float, float, double, float)", asFUNCTION(cfunction_gen), asCALL_GENERIC);
	else
	{
		int r;
		r = engine->RegisterGlobalFunction("void cfunction(float, float, double, float)", asFUNCTION(cfunction), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterGlobalFunction("void cfunction2(double, double, double, double)", asFUNCTION(cfunction2), asCALL_CDECL); assert( r >= 0 );
	}

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	ExecuteString(engine, "cfunction(9.2f, 13.3f, 18.8, 3.1415f)");

	if( !called )
	{
		// failure
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	}
	else if( !testVal )
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got (%f, %f, %f, %f), expected (%f, %f, %f, %f)\n\n", TESTNAME, t1, t2, t3, t4, 9.2f, 13.3f, 18.8, 3.1415f);
		TEST_FAILED;
	}

	SKIP_ON_MAX_PORT
	{
		called = false;
		testVal = false;
		ExecuteString(engine, "cfunction2(1337.0, 1338.0, 1339.0, 1340.0)");
		if( !called )
		{
			// failure
			PRINTF("\n%s: cfunction2 not called from script\n\n", TESTNAME);
			TEST_FAILED;
		}
	}

	engine->Release();

	// Success
	return fail;
}
