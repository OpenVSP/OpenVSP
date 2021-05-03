//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestBasic2
{

#define TESTNAME "TestBasic2"

static const char *script =
"void TestBasic2()                      \n"
"{                                      \n"
"    float a = 1, b = 2, c = 3;         \n"
"    int i = 0;                         \n"
"                                       \n"
"    for ( i = 0; i < 10000000; i++ )   \n"
"    {                                  \n"
"       a = a + b * c;                  \n"
"       if( a == 0 )                    \n"
"         a = 100.0f;                   \n"
"       if( b == 1 )                    \n"
"         b = 2;                        \n"
"    }                                  \n"
"}                                      \n";

void Test(double *testTime)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void TestBasic2()"));

	double time = GetSystemTimer();

	int r = ctx->Execute();

	time = GetSystemTimer() - time;

	if( r != asEXECUTION_FINISHED )
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


