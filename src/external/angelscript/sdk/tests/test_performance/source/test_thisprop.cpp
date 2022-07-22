//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestThisProp
{

#define TESTNAME "TestThisProp"

static const char *script =
"void TestThisProp()                    \n"
"{                                      \n"
"  Test t();                            \n"
"  t.Run();                             \n"
"}                                      \n"
"class Test                             \n"
"{                                      \n"
"  float a;                             \n"
"  float b;                             \n"
"  float c;                             \n"
"  int i;                               \n"
"  void Run()                           \n"
"  {                                    \n"
"    a = 1; b = 2; c = 3;               \n"
"    i = 0;                             \n"
"    for ( i = 0; i < 10000000; i++ )   \n"
"    {                                  \n"
"      a = a + b * c;                   \n"
"      if( a == 0 )                     \n"
"        a = 100.0f;                    \n"
"      if( b == 1 )                     \n"
"        b = 2;                         \n"
"    }                                  \n"
"  }                                    \n"
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
	ctx->Prepare(mod->GetFunctionByDecl("void TestThisProp()"));

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


