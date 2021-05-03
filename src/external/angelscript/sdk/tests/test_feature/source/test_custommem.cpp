
#include <stdarg.h>
#include "utils.h"

namespace TestCustomMem
{

static const char * const TESTNAME = "TestCustomMem";

int objectsAllocated = 0;
void *MyAlloc(size_t size)
{
	objectsAllocated++;

	void *mem = new asBYTE[size];
//	PRINTF("MyAlloc(%d) %X\n", size, mem);
	return mem;
}

void MyFree(void *mem)
{
	objectsAllocated--;

//	PRINTF("MyFree(%X)\n", mem);
	delete[] (asBYTE*)mem;
}

int ReturnObj()
{
	return 0;
}

void ReturnObjGeneric(asIScriptGeneric *gen)
{
	int v = 0;
	gen->SetReturnObject(&v);
}


static const char *script =
"void test(obj o) { }";

bool Test()
{
	bool fail = false;

	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	r = engine->RegisterObjectType("obj", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
//  r = engine->RegisterObjectBehaviour("obj", asBEHAVE_ALLOC, "obj &f(uint)", asFUNCTION(MyAlloc), asCALL_CDECL); assert( r >= 0 );
//  r = engine->RegisterObjectBehaviour("obj", asBEHAVE_FREE, "void f(obj &in)", asFUNCTION(MyFree), asCALL_CDECL); assert( r >= 0 );
	if( !strstr(asGetLibraryOptions(),"AS_MAX_PORTABILITY") )
	{
		r = engine->RegisterGlobalFunction("obj retObj()", asFUNCTION(ReturnObj), asCALL_CDECL); assert( r >= 0 );
	}
	r = engine->RegisterGlobalFunction("obj retObj2(obj)", asFUNCTION(ReturnObjGeneric), asCALL_GENERIC); assert( r >= 0 );

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	ExecuteString(engine, "obj o");

	if( !strstr(asGetLibraryOptions(),"AS_MAX_PORTABILITY") )
		ExecuteString(engine, "retObj()");

	ExecuteString(engine, "obj o; retObj2(o)");

	ExecuteString(engine, "obj[] o(2)");

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(0, script, strlen(script));
	mod->Build();
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(engine->GetModule(0)->GetFunctionByName("test"));
	int v = 0;
	ctx->SetArgObject(0, &v);
	ctx->Execute();
	ctx->Release();

	engine->Release();

	if( objectsAllocated )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// Success
	return fail;
}

} // namespace

