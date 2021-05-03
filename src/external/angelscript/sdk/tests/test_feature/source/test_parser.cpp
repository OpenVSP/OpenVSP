#include "utils.h"

namespace TestParser
{

static const char * const TESTNAME = "TestParser";

// Unfinished class
const char *script1 =
"class myclass \n"
"{             \n";

// Const with capital C
const char *script2 =
"class myclass                  \n"
"{                              \n"
"   void f(Const int&in) {}     \n"
"};                             \n";

bool Test()
{
	bool fail = false;
	int r;
	asIScriptEngine *engine;
	CBufferedOutStream bout;

	// Test parser errors
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "TestParser (3, 1) : Error   : Expected '}'\n"
			"TestParser (3, 1) : Error   : Instead found '<end of file>'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		// TODO: The } token isn't unexpected, the parser is just not understanding that it is still inside the object declaration
		if (bout.buffer != "TestParser (3, 17) : Error   : Expected ')' or ','\n"
			"TestParser (3, 17) : Error   : Instead found reserved keyword 'int'\n"
			"TestParser (4, 1) : Error   : Unexpected token '}'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
 	return fail;
}

} // namespace

