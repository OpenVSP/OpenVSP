//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "scriptstring.h"

namespace TestString2
{

#define TESTNAME "TestString2"

static const char *script =
"string BuildString2(string@ a, string@ b, string@ c)            \n"
"{                                                               \n"
"    return a + b + c;                                           \n"
"}                                                               \n"
"                                                                \n"
"void TestString2()                                              \n"
"{                                                               \n"
"    string a = \"Test\";                                        \n"
"    string b = \" \";                                           \n"
"    string c = \"string\";                                      \n"
"    string res;                                                 \n"
"    int i = 0;                                                  \n"
"                                                                \n"
"    for ( i = 0; i < 1000000; i++ )                             \n"
"    {                                                           \n"
"        res = BuildString2(a, b, c);                            \n"
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
	// TODO: optimize: BuildString2(a,b,c) is making unnecessary refcpy of local variables
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void TestString2()"));

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







