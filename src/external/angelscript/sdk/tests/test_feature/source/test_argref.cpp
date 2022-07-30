#include "utils.h"

namespace TestArgRef
{

static const char * const TESTNAME = "TestArgRef";



static const char *script1 =
"int g;                                 \n"
"void TestArgRef()                      \n"
"{                                      \n"
"  int a = 0;                           \n"
"  int[] b;                             \n"
"  Obj o;                               \n"
"  TestArgRef1(a);                      \n"
"  TestArgRef1(g);                      \n"
"  TestArgRef1(b[0]);                   \n"
"  TestArgRef1(o.v);                    \n"
"  string s;                            \n"
"  TestArgRef2(s);                      \n"
"}                                      \n"
"void TestArgRef1(int &in arg)          \n"
"{                                      \n"
"}                                      \n"
"void TestArgRef2(string &in str)       \n"
"{                                      \n"
"}                                      \n";

struct Obj
{
	int v;
};

bool Test()
{
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	RegisterScriptString_Generic(engine);

	engine->RegisterObjectType("Obj", sizeof(Obj), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
	engine->RegisterObjectProperty("Obj", "int v", asOFFSET(Obj, v));

	COutStream out;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	engine->Release();

	// Success
	return fail;
}

} // namespace

