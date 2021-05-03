//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include "../../../add_on/scriptarray/scriptarray.h"

namespace TestArray
{

#define TESTNAME "TestArray"

static const char *script =
"void TestArray()                              \n"
"{                                             \n"
"    for( uint i = 0; i < 1000000; i++ )       \n"
"    {                                         \n"
"        array<int> a = {0,1,2,3,4,5,6,7,8,0}; \n"
"        for( uint p = 0; p < 10; p++ )        \n"
"            a[p]++;                           \n"
"    }                                         \n"
"}                                             \n"
"void TestArray2()                             \n"
"{                                             \n"
"    array<int> a = {0};                       \n"
"    for( uint i = 0; i < 2000000; i++ )       \n"
"    {                                         \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"        a[0]++;                               \n"
"    }                                         \n"
"}                                             \n";

// The same function in C++ for comparison
void TestArray2(asIScriptEngine *engine)
{
	int val = 0;
	CScriptArray *a = CScriptArray::Create(engine->GetTypeInfoByDecl("array<int>"), 1, &val);
	for( asUINT i = 0; i < 2000000; i++ )
	{
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
		(*(int*)a->At(0))++;
	}
	a->Release();
}

void Test(double *testTimes)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, false);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void TestArray()"));

	double time = GetSystemTimer();
	
	int r = 0;

	// Test mixed creation of array object and access of elements
	r = ctx->Execute();

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
		testTimes[0] = time;

	ctx->Prepare(mod->GetFunctionByDecl("void TestArray2()"));

	time = GetSystemTimer();

	// Test pure access of elements
	// http://www.gamedev.net/topic/661177-qtscript-vs-angelscript/

	// TODO: run-time optimize: By allowing the application to tell the compiler how a call can be inlined
	//                          it would be possible to optimize this further. The inlined version of opIndex
	//                          would look something like:
	//
	//                            ChkIdx   index, length        // Check if the index is valid
	//                            ADDSi    v24                  // Move the this pointer to the internal buffer
	//                            RDSPtr                        // Dereference pointer
	//                            ADDSi    v8                   // Move the this pointer to the start of the array
	//                            ADDSi    index, element size  // Move the this pointer to the correct element
	//                            PopRPtr                       // Load the address of the reference into the register
	//                            Pop      4                    // Remove the index argument from the stack
	//
	//                          Without JIT the above would be pretty much the same as the quick Thiscall1 instruction,
	//                          but with JIT it would quite possibly be even faster.

	r = ctx->Execute();

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
		testTimes[1] = time;

	ctx->Release();
#endif
	engine->Release();
}

} // namespace



//---------------------------------------------------
// This is the same test in LUA script
//

/*

function func5()
    n = n + zfx.average( n, n )
end

function func4()
    n = n + 2 * zfx.average( n+1, n+2 )
end

function func3()
    n = n * 2.1 * n
end

function func2()
    n = n / 3.5
end

function recursion( rec )
    if rec >= 1 then
        recursion( rec - 1 )
    end

    if rec == 5 then func5()
        else if rec==4 then func4()
                else if rec==3 then func3()
                        else if rec==2 then func2()
                                else n = n * 1.5 
                                end
                        end
                end
        end
end

n = 0
i = 0

for i = 0, 249999, 0.25 do
    zfx.average( i, i + 1 ) 
    recursion( 5 )
    if n > 100 then n = 0 end
end

*/



