//
// Tests the negate operator behaviour
//
// Test author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestNegateOperator";

static int testVal = 0;
static bool called = false;

static int negate(int *f1) 
{
	called = true;
	return -*f1;
}

static int minus(int *f1, int *f2)
{
	called = true;
	return *f1 - *f2;
}

static void negate_gen(asIScriptGeneric *gen) 
{
	int *f1 = (int*)gen->GetObject();
	called = true;
	int ret = -*f1;
	gen->SetReturnObject(&ret);
}

static void minus_gen(asIScriptGeneric *gen)
{
	int *f1 = (int*)gen->GetObject();
	int *f2 = (int*)gen->GetArgAddress(0);
	called = true;
	int ret = *f1 - *f2;
	gen->SetReturnObject(&ret);
}

bool TestNegateOperator()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterObjectType("obj", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		engine->RegisterObjectMethod("obj", "obj opNeg()", asFUNCTION(negate_gen), asCALL_GENERIC);
		engine->RegisterObjectMethod("obj", "obj opSub(obj &in)", asFUNCTION(minus_gen), asCALL_GENERIC);
	}
	else
	{
		engine->RegisterObjectMethod("obj", "obj opNeg()", asFUNCTION(negate), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("obj", "obj opSub(obj &in)", asFUNCTION(minus), asCALL_CDECL_OBJFIRST);
	}
	engine->RegisterGlobalProperty("obj testVal", &testVal);

	testVal = 1000;

	COutStream obj;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &obj, asCALL_THISCALL);
	ExecuteString(engine, "testVal = -testVal");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: behaviour function was not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( testVal != -1000 ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got %d, expected %d\n\n", TESTNAME, testVal, -1000);
		TEST_FAILED;
	}

	called = false;
	ExecuteString(engine, "testVal = testVal - testVal");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: behaviour function was not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( testVal != 0 ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got %d, expected %d\n\n", TESTNAME, testVal, 0);
		TEST_FAILED;
	}
	

	engine->Release();
	
	// Success
	return fail;
}
