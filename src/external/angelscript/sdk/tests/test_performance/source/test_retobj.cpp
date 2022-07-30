//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "../../add_on/scriptmath/scriptmathcomplex.h"

namespace TestRetObj
{

#define TESTNAME "TestRetObj"

static const char *script =
"const int iterations = 2500000; \n"

// Return by value
// Two instances of the object is created in each iteration. The implicit temp object returned
// from the function, and the explicit object first used to initialize and then copied to the
// implicit object.
// TODO: optimize: Can the compiler be intelligent enough to detect that the explicit object
//                 is not stored anywhere and thus return it directly without making a copy to
//                 an implicit object?
// TODO: optimize: asBC_STOREOBJ + asBC_FREE  -> nothing (will this properly free the obj in register?)
// TODO: optimize: CMPIi + JS -> CmpJmpOnSIi (Compare immediate and jump on signed result)
// TODO: optimize: asBC_SUSPEND + asBC_SUSPEND -> remove second suspend at start of loop
// TODO: optimize: The VM should detect that the Test_Container constructor doesn't do anything and skip the execution of the Test_Container constructor, i.e. just allocate the memory
// TODO: optimize: Remove all SUSPEND instructions (it should be possible to suspend executions anyway.
//                 the debugger should be able to step through the code without these explicit instructions)
"void test1() \n"
"{ \n"
"  for (int i = 0; i < iterations; i++) { \n"
"    test_return_me(); \n"
"  } \n"
"} \n"
// TODO: optimize: asBC_CALL + asBC_STOREOBJ -> asBC_CallNStore
// TODO: optimize: Use copy constructor when creating the temp instance to return
// TODO: optimize: SetV1 + LoadRObjR + WRTV1 -> LoadRObjR + WrtI1 (Write immediate 8bit value to ref)
"Test_Container test_return_me() { \n"
"  Test_Container test; \n"
"  test.dog = false; \n"
"  test.bob = true; \n"
"  return test; \n"
"} \n"

// Take by ref
// One instance of the object is created in each iteration. It is passed by reference to the
// function where it is initialized.
// TODO: optimize: Can the compiler be intelligent enough to see that the test object is not 
//                 stored anywhere and thus reuse the same instance in each iteration?
//                 Maybe if the function instead used (Obj &out)
// TODO: optimize: VAR + GetObj -> PshV (except that GETOBJ clears the variable)
// TODO: optimize: The test variable is local and not a handle (i.e. cannot be reassigned), so the 
//                 compiler wouldn't have to create a handle to hold an extra reference 
//                 (except that the called function will release the handle, so for this reason it is necessary to copy it to a handle)
"void test2() \n"
"{ \n"
"  for (int i = 0; i < iterations; i++) { \n"
"    Test_Container test; \n"
"    test_take_by_ref(@test); \n"
"  } \n"
"} \n"
"void test_take_by_ref(Test_Container@ test) { \n"
"  test.dog = false; \n"
"  test.bob = true; \n"
"} \n"

// Take by ref 2
// One instance of the object is created outside the loop. The same instance is then passed
// into the function for initialization in each iteration.
"void test3() \n"
"{ \n"
"  Test_Container test; \n"
"  for (int i = 0; i < iterations; i++) { \n"
"    test_take_by_ref(@test); \n"
"  } \n"
"} \n"
"class Test_Container { \n"
"  bool dog; \n"
"  bool bob; \n"
"} \n";


void Test(double *testTimes)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	int r;

	// TODO: Disable GC to see how much it influences the result

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

#ifndef _DEBUG 
	double time;
	asIScriptContext *ctx = engine->CreateContext();

	ctx->Prepare(mod->GetFunctionByDecl("void test1()"));
	time = GetSystemTimer();
	r = ctx->Execute();
	time = GetSystemTimer() - time;
	testTimes[0] = time;

	ctx->Prepare(mod->GetFunctionByDecl("void test2()"));
	time = GetSystemTimer();
	r = ctx->Execute();
	time = GetSystemTimer() - time;
	testTimes[1] = time;

	ctx->Prepare(mod->GetFunctionByDecl("void test3()"));
	time = GetSystemTimer();
	r = ctx->Execute();
	time = GetSystemTimer() - time;
	testTimes[2] = time;

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

	ctx->Release();
#endif
	engine->Release();
}

} // namespace







