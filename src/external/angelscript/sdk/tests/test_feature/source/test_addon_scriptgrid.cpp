#include "utils.h"
#include "../../../add_on/scriptgrid/scriptgrid.h"
#include "../../../add_on/scriptany/scriptany.h"
#include "../../../add_on/scripthandle/scripthandle.h"

namespace Test_Addon_ScriptGrid
{

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
//	asIScriptContext *ctx;
	asIScriptEngine *engine;
//	asIScriptModule *mod;

	// Test circular reference between grid and ref
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptHandle(engine);
		RegisterScriptGrid(engine);

		// Create the circular reference
		r = ExecuteString(engine, "grid<ref> a; a.resize(1,1); @a[0,0] = a;");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->GarbageCollect();

		asUINT currSize, totDestroy, totDetect;
		engine->GetGCStatistics(&currSize, &totDestroy, &totDetect);
		if (currSize != 0 || totDestroy != 1 || totDetect != 1)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test empty initialization list
	// http://www.gamedev.net/topic/658849-empty-array-initialization/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptGrid(engine);

		r = ExecuteString(engine, "grid<int> a = {};"); // Valid 0x0 grid
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "grid<int> a = {{}};"); // Valid 0x1 grid
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "grid<int> a = {{},{}};"); // Valid 0x2 grid
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test grid object forcibly destroyed by garbage collector
	// http://www.gamedev.net/topic/657955-a-quite-specific-bug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptGrid(engine);
		RegisterScriptAny(engine);
		RegisterScriptArray(engine, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class B {} \n"
			"class A \n"
			"{ \n"
			"	any a; \n"
			"	grid<B@> t(10, 10); \n"
			"	A() \n"
			"	{ \n"
			"		a.store(@this); \n"
			"	} \n"
			"} \n"
			"array<A@> arr; \n"
			"void main() \n"
			"{ \n"
			"	arr.insertLast(@A()); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// The type B is not really garbage collected
		asITypeInfo *t = mod->GetTypeInfoByDecl("B");
		if( t == 0 || (t->GetFlags() & asOBJ_GC) )
			TEST_FAILED;

		// grid<B> is not garbage collected since B is not
		t = mod->GetTypeInfoByDecl("grid<B>");
		if( t == 0 || (t->GetFlags() & asOBJ_GC) )
			TEST_FAILED;

		// grid<B@> is however garbage collected because it is not possible to know 
		// that no class derived from B can't form a circular reference with it.
		t = mod->GetTypeInfoByDecl("grid<B@>");
		if( t == 0 || !(t->GetFlags() & asOBJ_GC) )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test resize
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptGrid(engine);
		RegisterStdString(engine);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine,
			"grid<string> g; \n"
			"g.resize(1,1); \n"
			"g[0,0] = 'hello'; \n"
			"g.resize(2,2); \n"
			"assert( g[0,0] == 'hello' ); \n"
			"g[1,1] = 'there'; \n"
			"g.resize(1,1); \n"
			"assert( g.width() == 1 && g.height() == 1 ); \n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test initialization lists
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptGrid(engine);
		RegisterStdString(engine);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, 
			"grid<int8> g = {{1,2,3},{4,5,6},{7,8,9}}; \n"
			"assert( g[0,0] == 1 ); \n"
			"assert( g[2,2] == 9 ); \n"
			"assert( g[0,2] == 7 ); \n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, 
			"grid<string> g = {{'1','2'},{'4','5'}}; \n"
			"assert( g[0,0] == '1' ); \n"
			"assert( g[1,1] == '5' ); \n"
			"assert( g[0,1] == '4' ); \n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine,
			"grid<grid<int>@> g = {{grid<int> = {{1}}, grid<int> = {{2}}}, {grid<int> = {{3}}, grid<int> = {{4}}}}; \n"
			"assert( g[0,0][0,0] == 1 ); \n"
			"assert( g[1,1][0,0] == 4 ); \n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	
		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

