#include "utils.h"

namespace TestArrayIntf
{

static const char * const TESTNAME = "TestArrayIntf";

// Normal structure
static const char *script1 =
"void Test()                  \n"
"{                            \n"
"   float[] a(2);             \n"
"   a[0] = 1.1f;              \n"
"   a[1] = 1.2f;              \n"
"   @floatArray = a;          \n"
"   string[] b(1);            \n"
"   b[0] = \"test\";          \n"
"   @stringArray = b;         \n"
"}                            \n";


CScriptArray *floatArray = 0;
CScriptArray *stringArray = 0;

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptArray(engine, true);
	RegisterScriptString(engine);

	engine->RegisterGlobalProperty("float[] @floatArray", &floatArray);
	engine->RegisterGlobalProperty("string[] @stringArray", &stringArray);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED ) 
	{
		TEST_FAILED;
	}
	else
	{
		if( (floatArray->GetArrayTypeId() & asTYPEID_MASK_OBJECT) != asTYPEID_TEMPLATE )
			TEST_FAILED;

		if( floatArray->GetArrayTypeId() != engine->GetTypeIdByDecl("float[]") )
			TEST_FAILED;

		if( floatArray->GetElementTypeId() != engine->GetTypeIdByDecl("float") )
			TEST_FAILED;

		if( floatArray->GetSize() != 2 )
			TEST_FAILED;

		if( *(float*)floatArray->At(0) != 1.1f )
			TEST_FAILED;

		if( *(float*)floatArray->At(1) != 1.2f )
			TEST_FAILED;

		if( stringArray->GetArrayTypeId() != engine->GetTypeIdByDecl("string[]") )
			TEST_FAILED;

		if( stringArray->GetElementTypeId() != engine->GetTypeIdByDecl("string") )
			TEST_FAILED;

		if( stringArray->GetSize() != 1 )
			TEST_FAILED;

		if( ((CScriptString*)stringArray->At(0))->buffer != "test" )
			TEST_FAILED;

		stringArray->Resize(2);
	}

	if( floatArray )
		floatArray->Release();
	if( stringArray )
		stringArray->Release();

	engine->Release();

	// Success
	return fail;
}

} // namespace

