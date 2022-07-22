//
// This test verifies enumeration of global script variables
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestEnumGlobVar";

static const char script[] = "int a; float b; double c; uint d = 0xC0DE; string e = \"test\"; obj @f = @o;";

void AddRef_Release_dummy(asIScriptGeneric *)
{
}


bool TestEnumGlobVar()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptString_Generic(engine);

	int r;
	r = engine->RegisterObjectType("obj", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("obj", asBEHAVE_ADDREF, "void f()", asFUNCTION(AddRef_Release_dummy), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("obj", asBEHAVE_RELEASE, "void f()", asFUNCTION(AddRef_Release_dummy), asCALL_GENERIC); assert( r >= 0 );
	int o = 0xBAADF00D;
	r = engine->RegisterGlobalProperty("obj o", &o);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script, sizeof(script)-1, 0);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod->Build();

	int count = mod->GetGlobalVarCount();
	if( count != 6 )
	{
		PRINTF("%s: GetGlobalVarCount() returned %d, expected 6.\n", TESTNAME, count);
		TEST_FAILED;
	}

	const char *buffer = 0;
	if( (buffer = mod->GetGlobalVarDeclaration(0)) == 0 )
	{
		PRINTF("%s: GetGlobalVarDeclaration() failed\n", TESTNAME);
		TEST_FAILED;
	}
	else if( strcmp(buffer, "int a") != 0 )
	{
		PRINTF("%s: GetGlobalVarDeclaration() returned %s\n", TESTNAME, buffer);
		TEST_FAILED;
	}

	int idx = mod->GetGlobalVarIndexByName("b");
	if( idx < 0 )
	{
		PRINTF("%s: GetGlobalVarIndexByName() returned %d\n", TESTNAME, idx);
		TEST_FAILED;
	}

	idx = mod->GetGlobalVarIndexByDecl("double c");
	if( idx < 0 )
	{
		PRINTF("%s: GetGlobalVarIndexByDecl() returned %d\n", TESTNAME, idx);
		TEST_FAILED;
	}

	buffer = 0;
	mod->GetGlobalVar(3, &buffer);
	if( buffer == 0 )
	{
		PRINTF("%s: GetGlobalVar() failed\n", TESTNAME);
		TEST_FAILED;
	}
	else if( strcmp(buffer, "d") != 0 )
	{
		PRINTF("%s: GetGlobalVar() returned %s\n", TESTNAME, buffer);
		TEST_FAILED;
	}

	asUINT *d;
	d = (asUINT *)mod->GetAddressOfGlobalVar(3);
	if( d == 0 )
	{
		PRINTF("%s: GetAddressOfGlobalVar() returned %d\n", TESTNAME, r);
		TEST_FAILED;
	}
	if( *d != 0xC0DE )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	std::string *e;
	e = (std::string*)mod->GetAddressOfGlobalVar(4);
	if( e == 0 )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	if( *e != "test" )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	int *f;
	f = *(int**)mod->GetAddressOfGlobalVar(5); // We're getting a pointer to the handle
	if( f == 0 )
	{
		PRINTF("%s: failed\n", TESTNAME);
		TEST_FAILED;
	}

	if( *f != (int)0xBAADF00D )
	{
		PRINTF("%s: failed\n", TESTNAME);
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

