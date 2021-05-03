//
// Test compiling scripts in two named modules
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "Test2Modules";
static const char *script = "int global; void Test() {global = 0;} float Test2() {Test(); return 0;}";

bool Test2Modules()
{
	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	
	asIScriptModule *mod = engine->GetModule("a", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script), 0);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: failed to build module a\n", TESTNAME);
		TEST_FAILED;
	}

	mod = engine->GetModule("b", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script), 0);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: failed to build module b\n", TESTNAME);
		TEST_FAILED;
	}

	if( !fail )
	{
		asIScriptFunction *aFunc = engine->GetModule("a")->GetFunctionByName("Test");
		if( aFunc == 0 )
		{
			PRINTF("%s: failed to retrieve func ID for module a\n", TESTNAME);
			TEST_FAILED;
		}

		asIScriptFunction *bFunc = engine->GetModule("b")->GetFunctionByName("Test");
		if( bFunc == 0 )
		{
			PRINTF("%s: failed to retrieve func ID for module b\n", TESTNAME);
			TEST_FAILED;
		}
	}

	engine->Release();

	// Test using an object created in another module
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterInterface("ITest");
	engine->RegisterInterfaceMethod("ITest", "void test()");
	const char *scriptA = "ITest @obj;";
	const char *scriptB = 
	"class CTest : ITest        \n"
	"{                          \n"
	"  void test() {glob = 42;} \n"
	"}                          \n"
	"int glob = 0;              \n";

	mod = engine->GetModule("a", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("scriptA", scriptA, strlen(scriptA));
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	mod = engine->GetModule("b", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("scriptB", scriptB, strlen(scriptB));
	mod->Build();
	if( r < 0 ) TEST_FAILED;

	asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(engine->GetModule("b")->GetTypeInfoByName("CTest"));
	*((asIScriptObject**)engine->GetModule("a")->GetAddressOfGlobalVar(0)) = obj;
	r = ExecuteString(engine, "obj.test()", engine->GetModule("a"));
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	int val = *(int*)engine->GetModule("b")->GetAddressOfGlobalVar(engine->GetModule("b")->GetGlobalVarIndexByName("glob"));
	if( val != 42 ) TEST_FAILED;

	engine->Release();

	return fail;
}
