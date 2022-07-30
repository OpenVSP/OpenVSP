//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestCall2
{

#define TESTNAME "TestCall2"

static const char *script =
"void TestCall2_A()                                               \n"
"{                                                                \n"
"}                                                                \n"
"void TestCall2_B()                                               \n"
"{                                                                \n"
"}                                                                \n";


                                         
void Test(double *testTime)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	asIScriptFunction *func_A = mod->GetFunctionByDecl("void TestCall2_A()");
	asIScriptFunction *func_B = mod->GetFunctionByDecl("void TestCall2_B()");

	double time = GetSystemTimer();
	int r;

	for( int n = 0; n < 5000000; n++ )
	{
		ctx->Prepare(func_A);
		r = ctx->Execute();
		if( r != 0 ) break;
		ctx->Prepare(func_B);
		r = ctx->Execute();
		if( r != 0 ) break;
	}

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







