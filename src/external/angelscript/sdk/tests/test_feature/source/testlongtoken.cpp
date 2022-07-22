//
// Tests to verify that long tokens doesn't crash the library
//

#include "utils.h"

static const char * const TESTNAME = "TestLongToken";


bool TestLongToken()
{
	bool fail = false;
	CBufferedOutStream bout;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	

	std::string str;

	str.resize(400);
	memset(&str[0], 'a', 400);
	str += " = 1";

	ExecuteString(engine, str.c_str());

	engine->Release();

	if (bout.buffer != "ExecuteString (1, 1) : Error   : No matching symbol 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'\n")
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Success
	return fail;
}
