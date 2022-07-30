//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "../../add_on/scriptarray/scriptarray.h"

namespace TestClassProp
{

#define TESTNAME "TestClassProp"

static const char *script =
"class TestClass {                                               \n"
"	array<float> mvScriptArray(1000);                            \n"
"	float mfTotal;                                               \n"
"	void DebugSpeedTestMember() {                                \n"
"		const int lIterations = 10000;                           \n"
"		const int lEntityNum = 1000;                             \n"
"		for (int j = 0; j<lIterations; ++j) {                    \n"
"			mfTotal = 0;                                         \n"
"			for (int i = 1; i<lEntityNum; ++i)                   \n"
"				mfTotal += mvScriptArray[i];                     \n"
"		}                                                        \n"
"	}                                                            \n"
"	void DebugSpeedTestLocalHandle() {                           \n"
"		const int lIterations = 10000;                           \n"
"		const int lEntityNum = 1000;                             \n"
"		array<float>@ pScriptArray = @mvScriptArray;             \n"
"		for (int j = 0; j<lIterations; ++j) {                    \n"
"			mfTotal = 0;                                         \n"
"			for (int i = 1; i<lEntityNum; ++i)                   \n"
"				mfTotal += pScriptArray[i];                      \n"
"		}                                                        \n"
"	}                                                            \n"
"}                                                               \n";

void Test(double *testTime)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, false);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);
	mod->Build();

#ifndef _DEBUG

	asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(mod->GetTypeInfoByName("TestClass"));


	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(obj->GetObjectType()->GetMethodByDecl("void DebugSpeedTestMember()"));
//	ctx->Prepare(obj->GetObjectType()->GetMethodByDecl("void DebugSpeedTestLocalHandle()"));
	ctx->SetObject(obj);

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
	obj->Release();
#endif
	engine->Release();
}

} // namespace


