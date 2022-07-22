#include "utils.h"
#include "../../../add_on/scriptany/scriptany.h"

using namespace std;

namespace TestDynamicConfig
{

static const char * const TESTNAME = "TestDynamicConfig";

static const char *script1 =
"void Test()           \n"
"{                     \n"
"  MyFunc();           \n"
"}                     \n";

static const char *script2 =
"void Test()           \n"
"{                     \n"
"  global = 1;         \n"
"}                     \n";

static const char *script3 =
"void Test()           \n"
"{                     \n"
"  mytype var;         \n"
"}                     \n";

static const char *script4 =
"void Test()           \n"
"{                     \n"
"  mytype var;         \n"
"  string a;           \n"
"  a = a + var;        \n"
"}                     \n";

static const char *script5 =
"void Test()           \n"
"{                     \n"
"  mytype var;         \n"
"  g_any.store(@var);  \n"
"}                     \n";

static const char *script6 =
"void Test()           \n"
"{                     \n"
"  int[] a;            \n"
"}                     \n";

static const char *script7 =
"class mystruct        \n"
"{                     \n"
"  mytype var;         \n"
"};                    \n";

static const char *script8 =
"void Test(mytype&in)  \n"
"{                     \n"
"}                     \n";

static const char *script9 =
"void Test()           \n"
"{                     \n"
"   mytype[] a;        \n"
"}                     \n";

static const char *script10 =
"void Test()           \n"
"{                     \n"
"  mytype[] var;       \n"
"  g_any.store(@var);  \n"
"}                     \n";

static void MyFunc(asIScriptGeneric *)
{
}

static void Factory(asIScriptGeneric *gen)
{
	*(int**)gen->GetAddressOfReturnLocation() = new int(1); // set ref count to 1
}

static void AddRef(asIScriptGeneric *gen)
{
	int *o = (int*)gen->GetObject();
	(*o)++;
}

static void Release(asIScriptGeneric *gen)
{
	int *o = (int*)gen->GetObject();
	(*o)--;
	if( *o == 0 ) delete o;
}

bool Test2();

class Type
{
public:
	Type() { refCount = 1; callback = 0; }
	~Type() { if(callback) callback->Release(); }
	void AddRef() { refCount++ ; }
	void Release() { if( --refCount == 0 ) delete this; }
	int refCount;
	asIScriptFunction *callback;
};

bool Test()
{
	bool fail = Test2();
	int r;
	COutStream out;
	CBufferedOutStream bout;

	// Test that removing a group doesn't remove other functions
	// http://www.gamedev.net/topic/657987-bug-new-functions-not-accessibly-by-getglobalfunctionbyindex-after-removing-different-configuration-group/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		// register 3 script functions a(), b() and c()
		r = engine->RegisterGlobalFunction("void a()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->RegisterGlobalFunction("void b()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->RegisterGlobalFunction("void c()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);

		asIScriptFunction *func = engine->GetGlobalFunctionByIndex(0);
		if( func == 0 || string(func->GetName()) != "a" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(1);
		if( func == 0 || string(func->GetName()) != "b" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(2);
		if( func == 0 || string(func->GetName()) != "c" )
			TEST_FAILED;

		// Add a dynamic group, then remove it
		r = engine->BeginConfigGroup("myconfig"); assert(r>=0);
		r = engine->RegisterGlobalFunction("void x()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->RegisterGlobalFunction("void y()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->EndConfigGroup(); assert(r>=0);

		r = engine->RemoveConfigGroup("myconfig"); assert(r>=0);

		// original functions should still be available
		func = engine->GetGlobalFunctionByIndex(0);
		if( func == 0 || string(func->GetName()) != "a" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(1);
		if( func == 0 || string(func->GetName()) != "b" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(2);
		if( func == 0 || string(func->GetName()) != "c" )
			TEST_FAILED;

		// add some more functions in the default group
		r = engine->RegisterGlobalFunction("void d()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->RegisterGlobalFunction("void e()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);
		r = engine->RegisterGlobalFunction("void f()", asFUNCTION(0), asCALL_GENERIC); assert(r>=0);

		// original functions should still be available
		func = engine->GetGlobalFunctionByIndex(0);
		if( func == 0 || string(func->GetName()) != "a" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(1);
		if( func == 0 || string(func->GetName()) != "b" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(2);
		if( func == 0 || string(func->GetName()) != "c" )
			TEST_FAILED;

		// new functions must also be available
		func = engine->GetGlobalFunctionByIndex(3);
		if( func == 0 || string(func->GetName()) != "d" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(4);
		if( func == 0 || string(func->GetName()) != "e" )
			TEST_FAILED;
		func = engine->GetGlobalFunctionByIndex(5);
		if( func == 0 || string(func->GetName()) != "f" )
			TEST_FAILED;

		engine->Release();
	}

	// Test dynamic config groups with function definitions used for callbacks
	// http://www.gamedev.net/topic/618909-assertion-failure-in-as-configgroupcpp/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->BeginConfigGroup("gr");
		engine->RegisterObjectType("type", 0, asOBJ_REF);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterObjectBehaviour("type", asBEHAVE_ADDREF, "void f()", asMETHOD(Type,AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("type", asBEHAVE_RELEASE, "void f()", asMETHOD(Type,Release), asCALL_THISCALL);
#else
		engine->RegisterObjectBehaviour("type", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Type,AddRef), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("type", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Type,Release), asCALL_GENERIC);
#endif
		engine->RegisterFuncdef("void fun(type @)");
		engine->RegisterObjectProperty("type", "fun @callback", asOFFSET(Type,callback));
		engine->EndConfigGroup();

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("s", 
			"void func(type @) {} \n"
			"void main(type @t) \n"
			"{ \n"
			"  @t.callback = func; \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		Type *t = new Type();

		// Call the function that sets the callback on the object
		asIScriptFunction *m = mod->GetFunctionByName("main");
		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(m);
		ctx->SetArgObject(0, t);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		// Release the engine, while the object holding the callback is still alive
		engine->Release();

		t->Release();

		// The engine will warn about the callback not being released before the engine
		if( bout.buffer != " (0, 0) : Warning : There is an external reference to an object in module 'mod', preventing it from being deleted\n"
		                   " (0, 0) : Info    : The function in previous message is named 'func'. The func type is 1\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	//------------
	// Test global function
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void MyFunc()", asFUNCTION(MyFunc), asCALL_GENERIC); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 1) : Info    : Compiling void Test()\n"
                                 "TestDynamicConfig (3, 3) : Error   : No matching symbol 'MyFunc'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	//----------------
	// Test global property
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterGlobalProperty("int global", (void*)1); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 1) : Info    : Compiling void Test()\n"
                                 "TestDynamicConfig (3, 3) : Error   : No matching symbol 'global'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Try registering the property again
	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterGlobalProperty("int global", (void*)1); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	engine->Release();

	//-------------
	// Test object types
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 1) : Info    : Compiling void Test()\n"
                                 "TestDynamicConfig (3, 3) : Error   : Identifier 'mytype' is not a data type\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	//------------------
	// Test global behaviours
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterScriptString_Generic(engine);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->RegisterObjectMethod("mytype", "string@ opAdd_r(const string &in)", asFUNCTION(MyFunc), asCALL_GENERIC); assert( r >= 0 );
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script4, strlen(script4), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	// Register the type again, but without the operator overload
	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script4, strlen(script4), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 1) : Info    : Compiling void Test()\n"
                                 "TestDynamicConfig (5, 9) : Error   : No matching operator that takes the types 'string&' and 'mytype' found\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	//------------------
	// Test object types held by external variable, i.e. any

	CScriptAny *any = 0;
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptAny(engine);

	engine->BeginConfigGroup("group1");
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_REF);
	r = engine->RegisterObjectBehaviour("mytype", asBEHAVE_FACTORY, "mytype @f()", asFUNCTION(Factory), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("mytype", asBEHAVE_ADDREF, "void f()", asFUNCTION(AddRef), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("mytype", asBEHAVE_RELEASE, "void f()", asFUNCTION(Release), asCALL_GENERIC);

	any = (CScriptAny*)engine->CreateScriptObject(engine->GetTypeInfoByName("any"));

	r = engine->RegisterGlobalProperty("any g_any", any);
	engine->EndConfigGroup();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(0, script5);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->DiscardModule(0);
	engine->GarbageCollect();

	int *o = 0;
	any->Retrieve(&o, engine->GetTypeIdByDecl("mytype@"));
	if( o == 0 )
		TEST_FAILED;
	if( --(*o) != 1 )
		TEST_FAILED;

	// The mytype variable is still stored in the any variable so we shouldn't be allowed to remove it's configuration group
	r = engine->RemoveConfigGroup("group1"); assert( r < 0 );

	any->Release();
	engine->GarbageCollect();

	// Now it should be possible to remove the group
	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	engine->Release();

	//-------------
	// Test array types
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("int[]", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	asITypeInfo *ot = engine->GetTypeInfoByDecl("int[]");
	if( ot->GetSubTypeId() != asTYPEID_INT32 )
		TEST_FAILED;

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script6, strlen(script6), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	engine->Release();

	//-------------
	// Test object types in struct members
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script7, strlen(script7), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script7, strlen(script7), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (3, 3) : Error   : Identifier 'mytype' is not a data type in global namespace\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	//-------------
	// Test object types in function declarations
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script8, strlen(script8), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script8, strlen(script8), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 11) : Error   : Identifier 'mytype' is not a data type in global namespace\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	//-------------
	// Test object types in script arrays
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);

	r = engine->BeginConfigGroup("group1"); assert( r >= 0 );
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script9, strlen(script9), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->DiscardModule(0);
	engine->GarbageCollect();

	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script9, strlen(script9), 0);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = mod->Build();
	if( r >= 0 || bout.buffer != "TestDynamicConfig (1, 1) : Info    : Compiling void Test()\n"
                                 "TestDynamicConfig (3, 4) : Error   : Identifier 'mytype' is not a data type\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();


	//------------------
	// Test object types held by external variable, i.e. any
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	RegisterScriptAny(engine);

	engine->BeginConfigGroup("group1");
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD);

	any = (CScriptAny*)engine->CreateScriptObject(engine->GetTypeInfoByName("any"));

	r = engine->RegisterGlobalProperty("any g_any", any);
	engine->EndConfigGroup();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script10);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->DiscardModule(0);
	engine->GarbageCollect();

	CScriptArray *array = 0;
	any->Retrieve(&array, engine->GetTypeIdByDecl("mytype[]@"));
	if( array == 0 )
	{
		TEST_FAILED;
	}
	else
		array->Release();

	engine->GarbageCollect();

	// The mytype variable is still stored in the any variable so we shouldn't be allowed to remove it's configuration group
	r = engine->RemoveConfigGroup("group1"); assert( r < 0 );

	any->Release();
	engine->GarbageCollect();

	// Now it should be possible to remove the group
	r = engine->RemoveConfigGroup("group1"); assert( r >= 0 );

	engine->Release();

	//-------------------
	// Test references between config groups
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->BeginConfigGroup("group1");
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	engine->EndConfigGroup();

	engine->BeginConfigGroup("group2");
	r = engine->RegisterGlobalFunction("void func(mytype)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
	engine->EndConfigGroup();

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	r = engine->RemoveConfigGroup("group2"); assert( r <= 0 );

	r = engine->RemoveConfigGroup("group1"); assert( r <= 0 );

	engine->Release();

	//--------------------
	// Test situation where the default group references a dynamic group. It will then be impossible
	// to remove the dynamic group, but the application must still be able to release the engine.
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->BeginConfigGroup("group1");
	r = engine->RegisterObjectType("mytype", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	engine->EndConfigGroup();

	r = engine->RegisterGlobalFunction("void func(mytype)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RemoveConfigGroup("group1"); assert( r == asCONFIG_GROUP_IS_IN_USE );

	engine->Release();

	//----------------------
	// Test that it is possible to repeat the registration of the config group
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("g1"); assert( r >= 0 );
	RegisterScriptString_Generic(engine);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	r = ExecuteString(engine, "string a = \"test\""); assert( r == asEXECUTION_FINISHED );

	r = engine->GarbageCollect(); assert( r >= 0 );

	r = engine->RemoveConfigGroup("g1"); assert( r >= 0 );

	// again..
	r = engine->BeginConfigGroup("g1"); assert( r >= 0 );
	RegisterScriptString_Generic(engine);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	r = ExecuteString(engine, "string a = \"test\""); assert( r == asEXECUTION_FINISHED );

	r = engine->GarbageCollect(); assert( r >= 0 );

	r = engine->RemoveConfigGroup("g1"); assert( r >= 0 );

	engine->Release();


	//-----------------------------
	// Test that it isn't possible to register the same property in two different groups
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->BeginConfigGroup("a");

	r = engine->RegisterGlobalProperty("int a", (void*)1); assert( r >= 0 );

	engine->EndConfigGroup();

	r = engine->RegisterGlobalProperty("int a", (void*)1); assert( r < 0 );

	engine->Release();

	//------------------------------
	// Test that ExecuteString doesn't lock dynamic config groups
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	r = engine->BeginConfigGroup("g1"); assert( r >= 0 );
	RegisterScriptString_Generic(engine);
	r = engine->EndConfigGroup(); assert( r >= 0 );

	r = ExecuteString(engine, "string a = \"test\""); assert( r == asEXECUTION_FINISHED );

	// Garbage collect and remove config group before discarding module
	r = engine->GarbageCollect(); assert( r >= 0 );
	r = engine->RemoveConfigGroup("g1"); assert( r >= 0 );

	engine->Release();

	//-----------------------
	// Make sure the clean-up works when there a groups using types in the default group
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterStdString(engine);

	engine->BeginConfigGroup("g");
	r = engine->RegisterGlobalFunction("void SaveLesson(const string &in)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
	engine->EndConfigGroup();

	engine->Release();

	//-------------------------
	// Test registering object members in a group
	// http://www.gamedev.net/topic/636396-config-groups-and-object-property-accessors/
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("type", 0, asOBJ_REF | asOBJ_NOCOUNT);

	// TODO: It should be possible to register methods and properties in different groups from where the type itself was registered
	engine->BeginConfigGroup("g");
	engine->RegisterObjectMethod("type", "void func()", asFUNCTION(0), asCALL_GENERIC);
	engine->EndConfigGroup();

	asITypeInfo *type = engine->GetTypeInfoByName("type");
	if( type->GetMethodCount() != 1 )
		TEST_FAILED;

	r = engine->RemoveConfigGroup("g");
	if( r < 0 )
		TEST_FAILED;

	// TODO: Currently the method is not removed as the method will be placed in the same group as the type. When this changes, the method should be removed
	if( type->GetMethodCount() != 1 )
		TEST_FAILED;

	engine->Release();


	// Success
	return fail;
}

// This test was reported by Zeu5 on 2009-03-24
// It used to crash on the second call to Build(), and also on ExecuteString()
bool Test2()
{
	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->BeginConfigGroup("MyGroup");
	engine->RegisterInterface("MyHostDefinedInterface");
	engine->RegisterInterfaceMethod("MyHostDefinedInterface", "void doSomething()");
	engine->EndConfigGroup();

	const char *script = "class MyScriptedClass : MyHostDefinedInterface\n"
						 "{\n"
						 "   void doSomething() { /* nothing */ }\n"
						 "} \n"
						 "void Test()  \n"
						 "{ \n"
						 "  MyScriptedClass @b = MyScriptedClass(); \n"
						 "} \n";

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->DiscardModule(0);
	r = engine->RemoveConfigGroup("MyGroup");
	if( r < 0 )
	{
		TEST_FAILED;
	}

	// Now do everything again
	engine->BeginConfigGroup("MyGroup");
	engine->RegisterInterface("MyHostDefinedInterface");
	engine->RegisterInterfaceMethod("MyHostDefinedInterface", "void doSomething()");
	engine->EndConfigGroup();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

}
