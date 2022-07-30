//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "scriptstring.h"

namespace TestString
{

#define TESTNAME "TestString"

static const char *script =
"string BuildString(string &in a, string &in b, string &in c)    \n"
"{                                                               \n"
"    return a + b + c;                                           \n"
"}                                                               \n"
"                                                                \n"
"void TestString()                                               \n"
"{                                                               \n"
"    string a = \"Test\";                                        \n"
"    string b = \"string\";                                      \n"
"    int i = 0;                                                  \n"
"                                                                \n"
"    for ( i = 0; i < 1000000; i++ )                             \n"
"    {                                                           \n"
"        string res = BuildString(a, \" \", b);                  \n"
"    }                                                           \n"
"}                                                               \n";


void Test(double *testTime)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString(engine);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void TestString()"));

	double time = GetSystemTimer();

	int r = ctx->Execute();

	time = GetSystemTimer() - time;

	if( r != 0 )
	{
		printf("Execution didn't terminate with asEXECUTION_FINISHED\n");
		if( r == asEXECUTION_EXCEPTION )
		{
			printf("Script exception\n");
			asIScriptFunction *func = ctx->GetExceptionFunction();
			printf("Func: %s\n", func->GetName());
			printf("Line: %d\n", ctx->GetExceptionLineNumber());
			printf("Desc: %s\n", ctx->GetExceptionString());
		}
	}
	else
		*testTime = time;

	ctx->Release();
#endif
	engine->Release();
}

} // namespace







