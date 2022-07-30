//
// Tests sending 8byte objects in a three levels deep function stack
//
// Sent in by: Adam Hoult (2004/07/02)
//

#include "utils.h"

static const char * const TESTNAME = "TestInt64";

static const char *script = 
"int Main()\n"
"{\n"
"    // Call test\n"
"    foo();\n"
"    cfunction();\n"
"    Int64 var;\n"
"    bar( var );\n"
"    // Some value we'll know when we return\n"
"    return 31337;\n"
"}\n"

"void foo( )\n"
"{\n"
"    Int64 var;\n"
"    bar( var );\n"
"}\n"

"void bar( Int64 )\n"
"{\n"
"    cfunction();\n"
"}\n";

static int called = 0;

static void cfunction(asIScriptGeneric *) {
	++called;
}

bool TestInt64()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterObjectType("Int64", 8, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterGlobalFunction("void cfunction()", asFUNCTION(cfunction), asCALL_GENERIC);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script);
	mod->Build();

	asIScriptFunction *f = engine->GetModule(0)->GetFunctionByDecl("int Main()");
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(f);
	int r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("\n%s: The execution didn't finish correctly (code %d)\n", TESTNAME, r);
		TEST_FAILED;

		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
	}
	
    if( called != 3 ) 
	{
		PRINTF("\n%s: cfunction called %d times. Expected 3 times\n", TESTNAME, called);
		TEST_FAILED;
	}

	ctx->Release();
	engine->Release();

	return fail;
}
