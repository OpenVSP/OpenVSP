//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestMthd
{

#define TESTNAME "TestMthd"

static const char *script =
"class cls                                                       \n"
"{                                                               \n"
"    void func0()                                    {}          \n"
"    void func1(int)                                 {}          \n"
"    void func2(int,int)                             {}          \n"
"    void func3(int,int,int)                         {}          \n"
"    void func4(int,int,int,int)                     {}          \n"
"    void func5(int,int,int,int,int)                 {}          \n"
"    void func6(int,int,int,int,int,int)             {}          \n"
"    void func7(int,int,int,int,int,int,int)         {}          \n"
"    void func8(int,int,int,int,int,int,int,int)     {}          \n"
"    void func9(int,int,int,int,int,int,int,int,int) {}          \n"
"}                                                               \n"
"                                                                \n"
"void TestMthd()                                                 \n"
"{                                                               \n"
"    cls obj;                                                    \n"
"                                                                \n"
"    for( int n = 0; n < 1000000; n++ )                          \n"
"    {                                                           \n"
"        obj.func0();                                            \n"
"        obj.func1(1);                                           \n"
"        obj.func2(1,2);                                         \n"
"        obj.func3(1,2,3);                                       \n"
"        obj.func4(1,2,3,4);                                     \n"
"        obj.func5(1,2,3,4,5);                                   \n"
"        obj.func6(1,2,3,4,5,6);                                 \n"
"        obj.func7(1,2,3,4,5,6,7);                               \n"
"        obj.func8(1,2,3,4,5,6,7,8);                             \n"
"        obj.func9(1,2,3,4,5,6,7,8,9);                           \n"
"    }                                                           \n"
"}                                                               \n";

                                         
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
	ctx->Prepare(mod->GetFunctionByDecl("void TestMthd()"));

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







