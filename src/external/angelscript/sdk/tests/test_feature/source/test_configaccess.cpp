#include "utils.h"

namespace TestConfigAccess
{

static void Func(asIScriptGeneric *)
{
}

static void TypeAdd(asIScriptGeneric *gen)
{
	int *a = (int*)gen->GetObject();
	int *b = (int*)gen->GetArgAddress(0);

	gen->SetReturnDWord(*a + *b);
}

bool Test()
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;
	COutStream out;
	asIScriptModule *mod;

	float val;

	// Test access masks for delegates without the first bit set
	// http://www.gamedev.net/topic/673050-bug-with-access-masks/
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, true);
		RegisterStdString(engine);

		engine->RegisterGlobalFunction("void Print(const string& in s)", asFUNCTION(0), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->SetAccessMask(2);
		mod->AddScriptSection("test",
			"funcdef void Foo();\n"
			"Foo@ g_pFoo = null;\n"
			"class Bar\n"
			"{\n"
			"	Bar()\n"
			"	{\n"
			"		@g_pFoo = Foo(this.bar);\n"
			"	}\n"
			"	void bar()\n"
			"	{\n"
			"	}\n"
			"}\n"
			"void test()\n"
			"{\n"
			"	Bar bar;\n"
			"	g_pFoo();\n"
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		
		engine->ShutDownAndRelease();
	}

	//------------
	// Test global properties
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetDefaultAccessMask(1);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	r = engine->BeginConfigGroup("group"); assert( r >= 0 );
	r = engine->RegisterGlobalProperty("float val", &val); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	// Make sure the default access is granted
	r = ExecuteString(engine, "val = 1.3f"); 
	if( r < 0 )
		TEST_FAILED;

	// Make sure the default access can be turned off
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	
	mod = engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);
	mod->SetAccessMask(2); 

	r = ExecuteString(engine, "val = 1.0f", mod);
	if( r >= 0 )
		TEST_FAILED;

	if( bout.buffer != "ExecuteString (1, 1) : Error   : No matching symbol 'val'\n")
		TEST_FAILED;

	// Make sure the default access can be overridden
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod->SetAccessMask(1); 

	r = ExecuteString(engine, "val = 1.0f", mod);
	if( r < 0 )
		TEST_FAILED;

	engine->Release();

	//----------
	// Test global functions
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	engine->SetDefaultAccessMask(2);

	r = engine->BeginConfigGroup("group"); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void Func()", asFUNCTION(Func), asCALL_GENERIC); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);
	mod->SetAccessMask(1); 

	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = ExecuteString(engine, "Func()", mod);
	if( r >= 0 )
		TEST_FAILED;

	if (bout.buffer != "ExecuteString (1, 1) : Error   : No matching symbol 'Func'\n")
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	mod->SetAccessMask(2);

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = ExecuteString(engine, "Func()", mod);
	if( r < 0 )
		TEST_FAILED;

	engine->Release();

	//------------
	// Test object types
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetDefaultAccessMask(1);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	r = engine->BeginConfigGroup("group"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);
	mod->SetAccessMask(2); 

	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = ExecuteString(engine, "mytype a", mod);
	if( r >= 0 )
		TEST_FAILED;

	if( bout.buffer != "ExecuteString (1, 1) : Error   : Type 'mytype' is not available for this module\n")
		TEST_FAILED;

	engine->Release();

	//------------
	// Test class members in different config groups
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );

	r = engine->SetDefaultAccessMask(2);
	r = engine->RegisterObjectMethod("mytype", "mytype opAdd(mytype &in)", asFUNCTION(TypeAdd), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectProperty("mytype", "int val", 0); assert( r >= 0 );
	
	mod = engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);
	mod->SetAccessMask(1); 

	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = ExecuteString(engine, "mytype a; a + a; a.val + a.val; a.opAdd(a);", mod);

	// It should be possible to disallow individual class methods
	if( r >= 0 )
		TEST_FAILED;

	if( bout.buffer != "ExecuteString (1, 13) : Error   : No matching operator that takes the types 'mytype' and 'mytype' found\n"
		               "ExecuteString (1, 19) : Error   : 'val' is not a member of 'mytype'\n"
					   "ExecuteString (1, 35) : Error   : No matching symbol 'opAdd'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// Success
	return fail;
}

}
