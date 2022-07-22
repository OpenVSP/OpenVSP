#include "utils.h"

using namespace std;

namespace TestSingleton
{

static const char * const TESTNAME = "TestSingleton";

int GameMgr;
int SoundMgr;

const char *script =
"void TestSingleton() \n"
"{                    \n"
"  Game.Test();       \n"
"}                    \n";

void TestMethod(asIScriptGeneric *gen)
{
	int *obj = (int*)gen->GetObject();
	assert(obj == &GameMgr || obj == &SoundMgr);
}

bool Test()
{
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString(engine);

	r = engine->RegisterObjectType("GameMgr", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
	r = engine->RegisterObjectMethod("GameMgr", "void Test()", asFUNCTION(TestMethod), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalProperty("GameMgr Game", (void*)&GameMgr); assert(r >= 0);

	// Test registering another object globabl property after 
	// compiling script that uses previous global property
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script), 0);
	mod->Build();

	r = engine->RegisterObjectType("SoundMgr", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
	r = engine->RegisterObjectMethod("SoundMgr", "void Test()", asFUNCTION(TestMethod), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalProperty("SoundMgr SMgr", (void*)&SoundMgr); assert(r >= 0);

	ExecuteString(engine, "Game.Test()", mod);
	ExecuteString(engine, "SMgr.Test()", mod);
	
	ExecuteString(engine, "TestSingleton()", mod);

	engine->Release();

	// Success
	return fail;
}

} // namespace

