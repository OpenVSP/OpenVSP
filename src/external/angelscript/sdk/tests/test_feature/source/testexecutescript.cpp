//
// Unit test author: Fredrik Ehnbom
//
// Description:
//
// Tests calling a script-function from c.
// Based on the sample found on angelscripts
// homepage.
//

#include "utils.h"

#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include "../../../add_on/scriptbuilder/scriptbuilder.h"

static const char * const TESTNAME = "TestExecuteScript";

static bool ExecuteScript();

static asIScriptEngine *engine;

bool TestExecuteScript()
{
	bool fail = false;
	COutStream out;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	
	RegisterStdString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	CScriptBuilder builder;

	int r = builder.StartNewModule(engine, 0);
	if( r >= 0 )
		r = builder.AddSectionFromFile("scripts/TestExecuteScript.as");
	if( r >= 0 )
		r = builder.BuildModule();
	if( r >= 0 )
	{
		fail = ExecuteScript();
	}

	// Create a second engine before releasing the first
	asIScriptEngine *en2 = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->Release();
	engine = NULL;

	// Release the second engine after the first
	en2->Release();

	return fail;
}


static bool ExecuteScript()
{
	// Create a context in which the script will be executed.

	// Several contexts may exist in parallel, holding the execution
	// of various scripts in the same engine. At the moment contexts are not
	// thread safe though so you should make sure that only one executes
	// at a time. An execution can be suspended to allow another
	// context to execute.

	asIScriptContext *ctx = engine->CreateContext();
	if( ctx == 0 )
	{
		PRINTF("%s: Failed to create a context\n", TESTNAME);
		return true;
	}

	// Prepare the context for execution

	// When a context has finished executing the context can be reused by calling
	// PrepareContext on it again. If the same stack size is used as the last time
	// there will not be any new allocation thus saving some time.

	int r = ctx->Prepare(engine->GetModule(0)->GetFunctionByName("main"));
	if( r < 0 )
	{
		PRINTF("%s: Failed to prepare context\n", TESTNAME);
		return true;
	}

	// If the script function takes any parameters we need to
	// copy them to the context's stack by using SetArguments()

	// Execute script

	r = ctx->Execute();
	if( r < 0 )
	{
		PRINTF("%s: Unexpected error during script execution\n", TESTNAME);
		return true;
	}

	if( r == asEXECUTION_FINISHED )
	{
		// If the script function is returning any
		// data we can get it with GetReturnValue()
		float retVal = ctx->GetReturnFloat();

		if (retVal == 7.826446f)
			r = 0;
		else
			PRINTF("%s: Script didn't return the correct value. Returned: %f, expected: %f\n", TESTNAME, retVal, 7.826446f);
	}
	else if( r == asEXECUTION_SUSPENDED )
	{
		PRINTF("%s: Execution was suspended.\n", TESTNAME);

		// In this case we can call Execute again to continue
		// execution where it last stopped.

		const asIScriptFunction *func = ctx->GetFunction();
		PRINTF("func : %s\n", func->GetName());
		PRINTF("line : %d\n", ctx->GetLineNumber());
	}
	else if( r == asEXECUTION_ABORTED )
	{
		PRINTF("%s: Execution was aborted.\n", TESTNAME);
	}
	else if( r == asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: An exception occured during execution\n", TESTNAME);

		// Print exception description
		const asIScriptFunction *func = ctx->GetExceptionFunction();
		PRINTF("func : %s\n", func->GetName());
		PRINTF("line : %d\n", ctx->GetExceptionLineNumber());
		PRINTF("desc : %s\n", ctx->GetExceptionString());
	}

	// Don't forget to release the context when you are finished with it
	ctx->Release();

	return false;
}
