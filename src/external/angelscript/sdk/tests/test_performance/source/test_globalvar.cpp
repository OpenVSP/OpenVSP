//
// http://www.gamedev.net/topic/663007-yet-another-performance-comparison-as-vs-small/
//

#include "utils.h"

namespace TestGlobalVar
{

#define TESTNAME "TestGlobalVar"

static const char *script =
"int TestNum;                              \n"
"void TestGlobalVar(void)                  \n"
"{                                         \n"
"  int count = 0;                          \n"
"  for(int col = 0; col < 10000000; col++) \n"
"  {                                       \n"
"    count++;                              \n"
"    ExtVar = count;                       \n" // Line A: Makes one call to external environment
"    TestNum = count;                      \n" // Line B: Only set a script variable
"  }                                       \n"
"}                                         \n";


int &Type_opAssign(int val, int &obj)
{
	obj = val;
	return obj;
}

void Type_opAssign_generic(asIScriptGeneric *gen)
{
	int *obj = reinterpret_cast<int*>(gen->GetObject());
	int val = int(gen->GetArgDWord(0));
	gen->SetReturnAddress(&Type_opAssign(val, *obj));
}

static int ExtVar;

void Test(double *testTime)
{
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);

	// Register an object type with an opAssign
	engine->RegisterObjectType("Type", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectMethod("Type", "Type &opAssign(int)", asFUNCTION(Type_opAssign), asCALL_CDECL_OBJLAST);
	// The generic calling convention is slightly faster on 64bit
	//engine->RegisterObjectMethod("Type", "Type &opAssign(int)", asFUNCTION(Type_opAssign_generic), asCALL_GENERIC);
	engine->RegisterGlobalProperty("Type ExtVar", &ExtVar);

/*
	// If the external variable was an int directly the execution become 
	// much fast, showing that the overhead is in the function call
	engine->RegisterGlobalProperty("int ExtVar", &ExtVar);
*/
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	mod->Build();

#ifndef _DEBUG
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("void TestGlobalVar()"));

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

	if( ExtVar != 10000000 )
		printf("Error in execution\n");
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



