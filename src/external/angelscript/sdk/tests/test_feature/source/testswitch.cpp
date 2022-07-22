// 
// Test designed to verify functionality of the switch case
//
// Author: Andreas Jönsson
//

#include "utils.h"
using namespace std;


static const char * const TESTNAME = "TestSwitch";


static int sum = 0;

static void add(asIScriptGeneric *gen)
{
	sum += (int)gen->GetArgDWord(0);
}

static string _log;
static void Log(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString *)gen->GetArgObject(0);
	_log += str->buffer;
}

bool TestSwitch()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterScriptString(engine);
	engine->RegisterGlobalFunction("void Log(const string &in)", asFUNCTION(Log), asCALL_GENERIC);

	engine->RegisterGlobalFunction("void add(int)", asFUNCTION(add), asCALL_GENERIC);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	const char *script =
		"void _switch()                  \n"  // 1
		"{                               \n"  // 2
		"  for( int n = -1; n < 2; ++n ) \n"  // 3
		"    switch( n )                 \n"  // 4
		"    {                           \n"  // 5
		"    case 0:                     \n"  // 6
		"      add(0);                   \n"  // 7
		"      break;                    \n"  // 8
		"    case -1:                    \n"  // 9
		"      add(-1);                  \n"  // 10
		"      break;                    \n"  // 11
		"    case 0x5:                   \n"  // 12
		"      add(5);                   \n"  // 13
		"      break;                    \n"  // 14
		"    case 0xF:                   \n"  // 15
		"      add(15);                  \n"  // 16
		"      break;                    \n"  // 17
		"    default:                    \n"  // 18
		"      add(255);                 \n"  // 19
		"      break;                    \n"  // 20
		"    }                           \n"  // 21
		"}                               \n"; // 22

	mod->AddScriptSection("switch", script);
	int r = mod->Build();
	if( r < 0 )
	{
		PRINTF("%s: Failed to build script\n", TESTNAME);
		TEST_FAILED;
	}

	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void _switch()"));
	ctx->Execute();

	if( sum != 254 )
	{
		PRINTF("%s: Expected %d, got %d\n", TESTNAME, 254, sum);
		TEST_FAILED;
	}

	ctx->Release();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	const char *script2 =
		"const int a = 1;                           \n"
		"const int8 b = 2;                          \n"
		"void _switch2()                            \n"
		"{                                          \n"
		"  const uint c = 3;                        \n"
		"  for( uint8 n = 0; n <= 5; ++n )          \n"
		"  {                                        \n"
		"    switch( n )                            \n"
		"    {                                      \n"
		"    case 5: Log(\"5\"); break;             \n"
		"    case 4: Log(\"4\"); break;             \n"
		"    case c: Log(\"3\"); break;             \n"
		"    case b: Log(\"2\"); break;             \n"
		"    case a: Log(\"1\"); break;             \n"
		"    default: Log(\"d\"); break;            \n"
		"    }                                      \n"
		"  }                                        \n"
		"  Log(\"\\n\");                            \n"
		"  myFunc127(127);                          \n"
		"  myFunc128(128);                          \n"
		"}                                          \n"
		"const uint8 c127 = 127;                    \n"
		"void myFunc127(uint8 value)                \n"
		"{                                          \n"
		"  if(value == c127)                        \n"
		"    Log(\"It is the value we expect\\n\"); \n"
		"                                           \n"
		"  switch(value)                            \n"
		"  {                                        \n"
		"    case c127:                             \n"
		"      Log(\"The switch works\\n\");        \n"
		"      break;                               \n"
		"    default:                               \n"
		"      Log(\"I didnt work\\n\");            \n"
		"      break;                               \n"
		"  }                                        \n"
		"}                                          \n"
		"const uint8 c128 = 128;                    \n"
		"void myFunc128(uint8 value)                \n"
		"{                                          \n"
		"  if(value == c128)                        \n"
		"    Log(\"It is the value we expect\\n\"); \n"
		"                                           \n"
		"  switch(value)                            \n"
		"  {                                        \n"
		"    case c128:                             \n"
		"      Log(\"The switch works\\n\");        \n"
		"      break;                               \n"
		"    default:                               \n"
		"      Log(\"I didnt work\\n\");            \n"
		"      break;                               \n"
		"  }                                        \n"
		"}                                          \n";
	mod->AddScriptSection("switch", script2);
	mod->Build();

	ExecuteString(engine, "_switch2()", mod);

	if( _log != "d12345\n"
		        "It is the value we expect\n"
                "The switch works\n"
                "It is the value we expect\n"
                "The switch works\n" )
	{
		TEST_FAILED;
		PRINTF("%s: Switch failed. Got: %s\n", TESTNAME, _log.c_str());
	}
 
	CBufferedOutStream bout;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	ExecuteString(engine, "switch(1) {}", mod); 
	if( bout.buffer != "ExecuteString (1, 1) : Error   : Empty switch statement\n" )
		TEST_FAILED;

	// Default case in the middle
	{
		bout.buffer = "";
		script =
			"int func(int d) { \n"
			"  switch (d) \n"
			"  { \n"
			"	case 0: \n"
			"	default: \n"
			"		return (4 * 16); \n"
			"	case 1: \n"
			"		return (4 * 16); \n"
			"	case 2: \n"
			"		return (5 * 16); \n"
			"	case 3: \n"
			"		return (9 * 16); \n"
			"  } \n"
			"  return 0; \n"
			"} \n";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("name", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "name (1, 1) : Info    : Compiling int func(int)\n"
		                   "name (5, 2) : Error   : The default case must be the last one\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// A switch case must not have duplicate cases
	{
		bout.buffer = "";
		script = "switch( 1 ) { case 1: case 1: }";
		r = ExecuteString(engine, script, mod);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 28) : Error   : Duplicate switch case\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test to make sure assert is not failing
	{
		bout.buffer = "";
		script = 
			"class Test \n"
			"{ \n"
			"	int8 State; \n"
			"	void test() \n"
			"	{ \n"
			"		switch (State) \n"
			"		{ \n"
			"       case 0: \n"
			"		} \n"
			"	} \n"
			"}; \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test to make sure the error message is appropriate when declaring variable in case
	{
		bout.buffer = "";
		script = 
			"	void test() \n"
			"	{ \n"
			"		switch (0) \n"
			"		{ \n"
			"       case 0: \n"
			"         int n; \n"
			"		} \n"
			"	} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (1, 2) : Info    : Compiling void test()\n"
		                   "script (6, 10) : Error   : Variables cannot be declared in switch cases, except inside statement blocks\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test error message for switch cases where the cases are strings
	{
		bout.buffer = "";
		script = 
			"void func() { \n"
			"  int test; \n"
			"  switch (undeclaredString) \n"
			"  { \n"
			"  case 'a': \n"
			"    test = 1; \n"
			"  } \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "script (1, 1) : Info    : Compiling void func()\n"
						   "script (3, 11) : Error   : No matching symbol 'undeclaredString'\n"
						   "script (5, 8) : Error   : Switch expressions must be integral numbers\n"
						   "script (3, 3) : Error   : Empty switch statement\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	engine->Release();

	return fail;
}
