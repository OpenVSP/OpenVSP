#include "utils.h"
using namespace std;

static const char * const TESTNAME = "TestNotInitialized";

static void cfunction(asIScriptGeneric *)
{

}

bool TestNotInitialized()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterGlobalFunction("void cfunction(int)", asFUNCTION(cfunction), asCALL_GENERIC);

	CBufferedOutStream out;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &out, asCALL_THISCALL);
	int r = ExecuteString(engine, "int a; cfunction(a);");
	if( r < 0 )
	{
	    TEST_FAILED;
	}

	if( out.buffer != "ExecuteString (1, 18) : Warning : 'a' is not initialized.\n" )
	{
		PRINTF("%s: Failed to catch use of uninitialized variable\n", TESTNAME);
		TEST_FAILED;
	}

	engine->Release();

	// Success
	return fail;
}
