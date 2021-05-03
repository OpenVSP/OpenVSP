#include "utils.h"
#include "../../../add_on/scriptdictionary/scriptdictionary.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include "../../../add_on/scriptstdstring/scriptstdstring.h"
#include "../../../add_on/contextmgr/contextmgr.h"

namespace Test_Addon_ContextMgr
{

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine *engine;

	// Test co-routines
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterScriptArray(engine, false);
		RegisterStdString(engine);
		RegisterScriptDictionary(engine);

		CContextMgr ctxMgr;
		ctxMgr.RegisterCoRoutineSupport(engine);

		const char *script =
			"int value = 0; \n"
			"void MyCoRoutine(dictionary @args) { yield(); value = int(args['arg1']); } \n"
			"void main() { \n"
			"  createCoRoutine(MyCoRoutine, (dictionary = {{'arg1', 42}})); \n"
			"  assert( value == 0 ); \n" // the co-routine has not executed yet
			"  yield(); \n"
			"  assert( value == 0 ); \n" // the co-routine has executed but yielded before it set the value
			"  yield(); \n"
			"  assert( value == 42 ); \n" // the co-routine finished executing
			"} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("Failed to compile the script\n");
		}

		ctxMgr.AddContext(engine, mod->GetFunctionByName("main"));
		int count = 10;
		while (ctxMgr.ExecuteScripts() > 0 && count-- > 0);
		
		if (count != 6)
			TEST_FAILED;
		
		engine->Release();
	}

	// TODO: The context manager should have a context pool (shared between context managers)
	// TODO: It must be possible to debug the scripts when using the context manager too

	// Success
	return fail;
}


} // namespace

