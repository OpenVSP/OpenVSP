//
// Tests releasing a suspended context
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestSuspend
{

static const char * const TESTNAME = "TestSuspend";

static int loopCount = 0;

static const char *script1 =
"string g_str = \"test\";    \n" // variable that must be released when module is released
"void TestSuspend()          \n"
"{                           \n"
"  string str = \"hello\";   \n" // variable that must be released before exiting the function
"  while( true )             \n" // never ending loop
"  {                         \n"
"    string a = str + g_str; \n" // variable inside the loop
"    Suspend();              \n"
"    loopCount++;            \n"
"  }                         \n"
"}                           \n";

static const char *script2 = 
"void TestSuspend2()         \n"
"{                           \n"
"  loopCount++;              \n"
"  loopCount++;              \n"
"  loopCount++;              \n"
"}                           \n";

bool doSuspend = false;
void Suspend(asIScriptGeneric * /*gen*/)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->Suspend();
	doSuspend = true;
}

bool doAbort = true;
void Abort(asIScriptGeneric *)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->Abort();
	doAbort = true;
}

void STDCALL LineCallback(asIScriptContext *ctx, void * /*param*/)
{
	// Suspend immediately
	ctx->Suspend();
}

bool Test()
{ 
	bool fail = false;

	//---
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	
	// Verify that the function doesn't crash when the stack is empty
	asIScriptContext *ctx = asGetActiveContext();
	assert( ctx == 0 );

	RegisterScriptString_Generic(engine);
	
	engine->RegisterGlobalFunction("void Suspend()", asFUNCTION(Suspend), asCALL_GENERIC);
	engine->RegisterGlobalFunction("void Abort()", asFUNCTION(Abort), asCALL_GENERIC);
	engine->RegisterGlobalProperty("int loopCount", &loopCount);

	COutStream out;
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(":1", script1);

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod->Build();

	ctx = engine->CreateContext();
	ctx->SetLineCallback(asFUNCTION(LineCallback), 0, asCALL_STDCALL);
	if( ctx->Prepare(mod->GetFunctionByDecl("void TestSuspend()")) >= 0 )
	{
		while( loopCount < 5 && !doSuspend )
			ctx->Execute();
	}
	else
		TEST_FAILED;

	// Make sure the Execute method returns proper status on abort
	int r = ExecuteString(engine, "Abort()", 0, 0);
	if( r != asEXECUTION_ABORTED )
	{
		TEST_FAILED;
	}

	// Release the engine first
	engine->Release();

	// Now release the context
	ctx->Release();
	//---
	// If the library was built with the flag BUILD_WITH_LINE_CUES the script
	// will return after each increment of the loopCount variable.
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterGlobalProperty("int loopCount", &loopCount);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":2", script2);
		mod->Build();

		ctx = engine->CreateContext();
		ctx->SetLineCallback(asFUNCTION(LineCallback), 0, asCALL_STDCALL);
		asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void TestSuspend2()");
		ctx->Prepare(func);
		loopCount = 0;
		while( ctx->GetState() != asEXECUTION_FINISHED )
			ctx->Execute();
		if( loopCount != 3 )
		{
			PRINTF("%s: failed\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Prepare(func);
		loopCount = 0;
		while( ctx->GetState() != asEXECUTION_FINISHED )
			ctx->Execute();
		if( loopCount != 3 )
		{
			PRINTF("%s: failed\n", TESTNAME);
			TEST_FAILED;
		}

		ctx->Release();
		engine->Release();
	}

	// Test cleaning up while context is suspended
	// http://www.gamedev.net/topic/618821-crash-while-closing-application-with-the-suspended-context/
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void Suspend()", asFUNCTION(Suspend), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":1", 
			"class Test { \n"
			"  void method() { \n"
			"    string str = 'test'; \n"
			"    Suspend(); \n"
			"  } \n"
			"} \n");

		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		mod->Build();

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "Test t; t.method();", mod, ctx);
		if( r != asEXECUTION_SUSPENDED )
			TEST_FAILED;

		// Try freeing up as much memory as possible
		engine->GarbageCollect();
		engine->Release();

		// Free the suspended context last
		ctx->Release();
	}

	// Test cleaning up while context is suspended
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void Suspend()", asFUNCTION(Suspend), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":1", 
			"void func() \n"
			"{ \n"
			"  string s; \n"
			"  s = method(); \n"
			"} \n"
			"string method() { \n"
			"  Suspend(); \n"
			"  return 'test'; \n"
			"} \n"
			);

		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		mod->Build();

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "func();", mod, ctx);
		if( r != asEXECUTION_SUSPENDED )
			TEST_FAILED;

		// Free the context while still suspended
		ctx->Release();

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

