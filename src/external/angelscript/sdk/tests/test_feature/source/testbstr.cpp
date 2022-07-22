// 
// Test designed to verify functionality of the bstr type
//
// Written by Andreas Jönsson 
//

#include "utils.h"
#include "bstr.h"

static const char * const TESTNAME = "TestBStr";

static asBSTR NewString(int length)
{
	return asBStrAlloc(length);
}

bool TestBStr()
{
	RET_ON_MAX_PORT

	bool fail = false;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	RegisterBStr(engine);

	engine->RegisterGlobalFunction("bstr NewString(int)", asFUNCTION(NewString), asCALL_CDECL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	int r = ExecuteString(engine, "bstr s = NewString(10)");
	if( r < 0 ) 
	{
		PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		TEST_FAILED;
	}
	else if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: ExecuteString() returned %d\n", TESTNAME, r);
		TEST_FAILED;
	}

	// Test passing bstr strings to a script function
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	const char *script =
		"void test(bstr a, bstr b)  \n"
		"{                          \n"
		"  assert(a == \"a\");      \n"
		"  assert(b == \"b\");      \n"
		"}                          \n";
	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	asIScriptFunction *func = mod->GetFunctionByIndex(0);
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(func);

	// Create the object and initialize it, then give 
	// the pointer directly to the script engine. 
	// The script engine will free the object.
	asBSTR *a = (asBSTR*)engine->CreateScriptObject(engine->GetTypeInfoByName("bstr"));
	*a = asBStrAlloc(1);
	strcpy((char*)*a, "a");
	*(asBSTR**)ctx->GetAddressOfArg(0) = a;

	// Create a local instance and have the script engine copy it.
	// The application must free its copy of the object.
	asBSTR b = asBStrAlloc(1);
	strcpy((char*)b, "b");
	ctx->SetArgObject(1, &b);
	asBStrFree(b);

	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	if( ctx ) ctx->Release();

	engine->Release();

	return fail;
}
