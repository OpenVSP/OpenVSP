//
// Tests compiling with 2 equal functions
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace Test2Func
{

static const char * const TESTNAME = "Test2Func";


static const char *script1 =
"void Test() { } \n"
"void Test() { } \n";

static const char *script2 = 
"void Test(void) { } \n";

bool Test()
{
	bool fail = false;
	int r;
	CBufferedOutStream out;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &out, asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	mod->Build();

	if( out.buffer != "Test2Func (2, 1) : Error   : A function with the same name and parameters already exists\n" )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to identify the error with two equal functions\n", TESTNAME);
	}

	out.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;
	if( out.buffer != "" )
		TEST_FAILED;





	engine->Release();

	if( fail )
	{
		PRINTF("%s: failed\n", TESTNAME);
	}

	// Success
	return fail;
}

} // namespace

