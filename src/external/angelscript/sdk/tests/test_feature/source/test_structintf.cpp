#include "utils.h"
#include "../../../add_on/scriptany/scriptany.h"

namespace TestStructIntf
{

static const char * const TESTNAME = "TestStructIntf";

// Normal structure
static const char *script1 =
"struct MyStruct              \n"
"{                            \n"
"   float a;                  \n"
"   string b;                 \n"
"   string @c;                \n"
"};                           \n"
"void Test()                  \n"
"{                            \n"
"   MyStruct s;               \n"
"   s.a = 3.141592f;          \n"
"   s.b = \"test\";           \n"
"   @s.c = \"test2\";         \n"
"   g_any.store(@s);          \n"
"}                            \n";



CScriptAny *any = 0;

bool Test()
{
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterScriptString(engine);

	any = (CScriptAny*)engine->CreateScriptObject(engine->GetTypeInfoByName("any"));
	engine->RegisterGlobalProperty("any g_any", any);

	COutStream out;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	// Try retrieving the type Id for the structure
	int typeId = mod->GetTypeIdByDecl("MyStruct");
	if( typeId < 0 )
	{
		PRINTF("%s: Failed to retrieve the type id for the script struct\n", TESTNAME);
		TEST_FAILED;
	}

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED ) 
		TEST_FAILED;
	else
	{		
		asIScriptObject *s = 0;
		typeId = any->GetTypeId();
		any->Retrieve(&s, typeId);

		if( (typeId & asTYPEID_MASK_OBJECT) != asTYPEID_SCRIPTOBJECT )
			TEST_FAILED;

		if( strcmp(engine->GetTypeDeclaration(typeId, true), "MyStruct@") )
			TEST_FAILED;

		typeId = s->GetTypeId();
		if( strcmp(engine->GetTypeDeclaration(typeId), "MyStruct") )
			TEST_FAILED;

		if( s->GetPropertyCount() != 3 )
			TEST_FAILED;

		if( strcmp(s->GetPropertyName(0), "a") )
			TEST_FAILED;

		if( s->GetPropertyTypeId(0) != engine->GetTypeIdByDecl("float") )
			TEST_FAILED;

		if( *(float*)s->GetAddressOfProperty(0) != 3.141592f )
			TEST_FAILED;

		if( strcmp(s->GetPropertyName(1), "b") )
			TEST_FAILED;

		if( s->GetPropertyTypeId(1) != engine->GetTypeIdByDecl("string") )
			TEST_FAILED;

		if( ((CScriptString*)s->GetAddressOfProperty(1))->buffer != "test" )
			TEST_FAILED;

		if( strcmp(s->GetPropertyName(2), "c") )
			TEST_FAILED;

		if( s->GetPropertyTypeId(2) != engine->GetTypeIdByDecl("string@") )
			TEST_FAILED;

		if( (*(CScriptString**)s->GetAddressOfProperty(2))->buffer != "test2" )
			TEST_FAILED;

		if( s )
			s->Release();
	}

	if( any )
		any->Release();

	// The type id is valid for as long as the type exists
	if( strcmp(engine->GetTypeDeclaration(typeId), "MyStruct") )
		TEST_FAILED;

	// Make sure the type is not used anywhere
	engine->DiscardModule(0);
	engine->GarbageCollect();

	// The type id is no longer valid
	if( engine->GetTypeDeclaration(typeId) != 0 )
		TEST_FAILED;

	engine->Release();

	// Success
	return fail;
}

} // namespace

