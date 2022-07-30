#include "utils.h"

static const char * const TESTNAME = "TestExecuteString";

struct Obj
{
	bool a;
	bool b;
} g_Obj;


bool TestExecuteString()
{
	bool fail = false;
	asIScriptEngine *engine;
	COutStream out;

	// Test ExecuteString with return values
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);

		int val1;
		ExecuteString(engine, "return 42", &val1, asTYPEID_INT32);
		if( val1 != 42 )
			TEST_FAILED;

		std::string val2;
		ExecuteString(engine, "return 'test'", &val2, engine->GetTypeIdByDecl("string"));
		if( val2 != "test" )
			TEST_FAILED;

		CScriptArray *val3 = 0;
		ExecuteString(engine, "return array<int>(1,42)", &val3, engine->GetTypeIdByDecl("array<int>@"));
		if( val3 == 0 || val3->GetSize() != 1 || *(int*)val3->At(0) != 42 )
			TEST_FAILED;
		val3->Release();

		engine->Release();
	}

	// Tests ExecuteString() with multiple lines of code
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->RegisterObjectType("Obj", sizeof(Obj), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
		engine->RegisterObjectProperty("Obj", "bool a", asOFFSET(Obj,a));
		engine->RegisterObjectProperty("Obj", "bool b", asOFFSET(Obj,b));

		engine->RegisterGlobalProperty("Obj g_Obj", &g_Obj);

		g_Obj.a = false;
		g_Obj.b = true;

		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		ExecuteString(engine, "g_Obj.a = true;\n"
							  "g_Obj.b = false;\n");

		engine->Release();

		if( !g_Obj.a || g_Obj.b )
		{
			PRINTF("%s: ExecuteString() didn't execute correctly\n", TESTNAME);
			TEST_FAILED;
		}
	}
	
	// Success
	return fail;
}
