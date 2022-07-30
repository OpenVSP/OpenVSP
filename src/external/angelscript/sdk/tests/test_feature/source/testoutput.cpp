// 
// Test designed to verify the compiler message output
//
// Author: Andreas Jönsson
//

#include "utils.h"

namespace TestOutput
{
static const char * const TESTNAME = "TestOutput";


static const char *script1 =
"void test()                   \n"  // 1
"{                             \n"  // 2
"  // This is a semantic error \n"  // 3
"  a = a;                      \n"  // 4
"}                             \n"; // 5

static const char *script2 =
"// This is a syntax error     \n"  // 1
"a = a;                        \n"; // 2

bool Test()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	CBufferedOutStream out;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &out, asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1);
	mod->Build();

	mod->AddScriptSection(TESTNAME, script2);
	mod->Build();

	if( out.buffer != "TestOutput (1, 1) : Info    : Compiling void test()\n"
		              "TestOutput (4, 7) : Error   : No matching symbol 'a'\n"
		              "TestOutput (4, 3) : Error   : No matching symbol 'a'\n"
		              "TestOutput (2, 3) : Error   : Expected identifier\n"
					  "TestOutput (2, 3) : Error   : Instead found '='\n" )
	{
		PRINTF("%s", out.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

} // namespace
