#include "utils.h"
using std::string;

static const char * const TESTNAME = "TestGlobalVar";
static const char *script1 = "float global = func() * g_f * 2.0f;";
static const char *script2 = "float global = 1.0f;";

static void func(asIScriptGeneric *gen)
{
	gen->SetReturnFloat(3.0f);
}

static float cnst = 2.0f;
static CScriptString *g_str = 0;

static const char *script3 =
"float f = 2;                 \n"
"string str = 'test';         \n"
"void TestGlobalVar()         \n"
"{                            \n"
"  float a = f + g_f;         \n"
"  string s = str + g_str;    \n"
"  g_f = a;                   \n"
"  f = a;                     \n"
"  g_str = s;                 \n"
"  str = s;                   \n"
"}                            \n";

static const char *script4 =
"const double gca=12;   \n"
"const double gcb=5;    \n"
"const double gcc=35.2; \n"
"const double gcd=4;    \n"
"double a=12;   \n"
"double b=5;    \n"
"double c=35.2; \n"
"double d=4;    \n"
"void test()          \n"
"{                    \n"
"  print(gca+\"\\n\");  \n"
"  print(gcb+\"\\n\");  \n"
"  print(gcc+\"\\n\");  \n"
"  print(gcd+\"\\n\");  \n"
"  print(a+\"\\n\");  \n"
"  print(b+\"\\n\");  \n"
"  print(c+\"\\n\");  \n"
"  print(d+\"\\n\");  \n"
"const double lca=12;   \n"
"const double lcb=5;    \n"
"const double lcc=35.2; \n"
"const double lcd=4;    \n"
"double la=12;   \n"
"double lb=5;    \n"
"double lc=35.2; \n"
"double ld=4;    \n"
"  print(lca+\"\\n\");  \n"
"  print(lcb+\"\\n\");  \n"
"  print(lcc+\"\\n\");  \n"
"  print(lcd+\"\\n\");  \n"
"  print(la+\"\\n\");  \n"
"  print(lb+\"\\n\");  \n"
"  print(lc+\"\\n\");  \n"
"  print(ld+\"\\n\");  \n"
"}                    \n";

static const char *script5 =
"uint OFLAG_BSP = uint(1024);";

static const char *script6 = 
"string @handle = @object; \n"
"string  object = \"t\";   \n";

void print(asIScriptGeneric *gen)
{
	std::string s = ((CScriptString*)gen->GetArgAddress(0))->buffer;

	if( s != "12\n" && 
		s != "5\n" &&
		s != "35.2\n" &&
		s != "4\n" )
		PRINTF("Error....\n");

//	PRINTF(s.c_str());
}

bool TestGlobalVar()
{
	bool fail = false;
	asIScriptEngine *engine;
	COutStream out;
	int r;

	// Test removing a global variable from a module before discarding the module
	{
		engine = asCreateScriptEngine();

		RegisterScriptArray(engine, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "array<int> a(100000); \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = mod->RemoveGlobalVar(0);
		if (r < 0)
			TEST_FAILED;
		
		engine->ShutDownAndRelease();
	}

	g_str = new CScriptString("test");

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptString_Generic(engine);

	engine->RegisterGlobalFunction("float func()", asFUNCTION(func), asCALL_GENERIC);
	engine->RegisterGlobalProperty("float g_f", &cnst);
	engine->RegisterGlobalProperty("string g_str", g_str);
	engine->RegisterGlobalFunction("void print(string &in)", asFUNCTION(print), asCALL_GENERIC);

	asIScriptModule *mod = engine->GetModule("a", asGM_ALWAYS_CREATE);

	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: build failed\n", TESTNAME);
		TEST_FAILED;
	}

	mod->AddScriptSection("script", script2, strlen(script2), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: build failed\n", TESTNAME);
		TEST_FAILED;
	}

	mod->AddScriptSection("script", script3, strlen(script3), 0);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: build failed\n", TESTNAME);
		TEST_FAILED;
	}

	ExecuteString(engine, "TestGlobalVar()", mod);

	float *f = (float*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByDecl("float f"));
	string *str = (string*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByDecl("string str"));

	float fv = *f; UNUSED_VAR(fv);
	string strv = *str;

	mod->ResetGlobalVars();

	f = (float*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByDecl("float f"));
	str = (string*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByDecl("string str"));

	if( !CompareDouble(*f, 2) || *str != "test" )
	{
		PRINTF("%s: Failed to reset the module\n", TESTNAME);
		TEST_FAILED;
	}

	// Use another module so that we can test that the variable id is correct even for multiple modules
	mod = engine->GetModule("b", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script4);
	if( mod->Build() < 0 )
	{
		PRINTF("%s: build failed\n", TESTNAME);
		TEST_FAILED;
	}

	int c = engine->GetModule("b")->GetGlobalVarCount();
	if( c != 8 ) TEST_FAILED;
	double d;
	d = *(double*)engine->GetModule("b")->GetAddressOfGlobalVar(0); 
	if( !CompareDouble(d, 12) ) TEST_FAILED;
	d = *(double*)engine->GetModule("b")->GetAddressOfGlobalVar(engine->GetModule("b")->GetGlobalVarIndexByName("gcb")); 
	if( !CompareDouble(d, 5) ) TEST_FAILED;
	d = *(double*)engine->GetModule("b")->GetAddressOfGlobalVar(engine->GetModule("b")->GetGlobalVarIndexByDecl("const double gcc")); 
	if( !CompareDouble(d, 35.2) ) TEST_FAILED;
	d = *(double*)engine->GetModule("b")->GetAddressOfGlobalVar(3); 
	if( !CompareDouble(d, 4) ) TEST_FAILED;
	
	ExecuteString(engine, "test()", engine->GetModule("b"));

	engine->Release();

	//--------------------
	CScriptArray *gPacketData = 0;
	unsigned int gPacketLength = 0;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	r = engine->RegisterGlobalProperty("uint gPacketLength", &gPacketLength); assert( r >= 0 );
	r = engine->RegisterGlobalProperty("uint8[] @gPacketData", &gPacketData); assert( r >= 0 );
	engine->Release();

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	r = engine->RegisterGlobalProperty("uint8[] @gPacketData", &gPacketData); assert( r >= 0 );
	r = engine->RegisterGlobalProperty("uint gPacketLength", &gPacketLength); assert( r >= 0 );
	engine->Release();

	//-----------------------
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script5);
	r = mod->Build(); 
	if( r < 0 )
		TEST_FAILED;
	engine->Release();

	//--------------------------
	// Make sure GetGlobalVarPointer is able to handle objects and pointers correctly
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptString(engine);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script6);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	else
	{
		CScriptString *object = (CScriptString*)engine->GetModule(0)->GetAddressOfGlobalVar(engine->GetModule(0)->GetGlobalVarIndexByName("object"));
		CScriptString **handle = (CScriptString**)engine->GetModule(0)->GetAddressOfGlobalVar(engine->GetModule(0)->GetGlobalVarIndexByName("handle"));
		if( *handle != object )
			TEST_FAILED;
		if( object->buffer != "t" )
			TEST_FAILED;
	}
	engine->Release();

	// Test accessing global var in namespace
	// http://www.gamedev.net/topic/662385-calling-a-method-on-a-script-class-from-c/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"class foo \n"
			"{ \n"
			"  void test() {} \n"
			"} \n"
			"namespace ns \n"
			"{ \n"
			"	foo bar; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		int i = mod->GetGlobalVarIndexByName("bar");
		if( i >= 0 )
			TEST_FAILED;

		i = mod->GetGlobalVarIndexByDecl("foo ns::bar");
		if( i < 0 )
			TEST_FAILED;

		i = mod->GetGlobalVarIndexByDecl("ns::foo ns::bar");
		if( i < 0 )
			TEST_FAILED;

		mod->SetDefaultNamespace("ns");
		i = mod->GetGlobalVarIndexByName("bar");
		if( i < 0 )
			TEST_FAILED;

		i = mod->GetGlobalVarIndexByDecl("foo bar");
		if( i < 0 )
			TEST_FAILED;

		engine->Release();
	}

	//----------------------
	// Global object handles initialized with other global variables
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"class b {}         \n"
			"b a;               \n"
			"b @h = @a;         \n"
			"b@[] v = {@a, @h}; \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		else
		{
			r = ExecuteString(engine, "assert(@a == @h); assert(v.length() == 2); assert(@v[0] == @v[1]);", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}

		engine->Release();
	}

	//-----------
	// It's not valid to attempt registering global properties with values
	{
		CBufferedOutStream bout;
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = engine->RegisterGlobalProperty("const int value = 3345;", (void*)1);
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "Property (1, 17) : Error   : Expected '<end of file>'\n"
			               "Property (1, 17) : Error   : Instead found '='\n"
			               " (0, 0) : Error   : Failed in call to function 'RegisterGlobalProperty' with 'const int value = 3345;' (Code: asINVALID_DECLARATION, -10)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	//-------------
	// It is possible to access global variables through scope operator if local variables have the same name
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); 
		const char *script = 
			"float g = 0; \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		mod->Build();
		r = ExecuteString(engine, "float g = 0; g = 1; ::g = 2; assert( g == 1 ); assert( ::g == 2 );", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}
		engine->Release();
	}

	//-----------------
	// It is possible to turn off initialization of global variables
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterObjectType("object", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("object", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);

		// Without this, the application would crash as the engine tries
		// to initialize the variable with the invalid function pointer
		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		const char *script = "object o;";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	//-----------------------
	// variables of primitive type should be initialized before variables of complex type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = "class MyObj { MyObj() { a = g_a; b = g_b; } int a; int b; } \n"
			                 "int g_a = 314 + g_b; \n" // This forces g_a to be compiled after g_b
							 "MyObj obj(); \n"         // obj should be compiled last
							 "int g_b = 42; \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "assert( obj.a == 314+42 ); \n"
			                      "assert( obj.b == 42 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//-----------------------
	// variables of object type that access other global objects in constructor will throw null-pointer exception
	{
		CBufferedOutStream bout;
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = "class A { void Access() {} } \n"
			                 "class B { B() { g_a.Access(); g_c.Access(); } } \n"
			                 "A g_a; \n"
							 "B g_b; \n" // g_b accesses both g_a and g_c
							 "A g_c; \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (4, 3) : Error   : Failed to initialize global variable 'g_b'\n"
		                   " (2, 0) : Info    : Exception 'Null pointer access' in 'B::B()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Global vars can be disabled
	{
		CBufferedOutStream bout;
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		
		engine->SetEngineProperty(asEP_DISALLOW_GLOBAL_VARS, true);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("", "int var;");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (1, 1) : Error   : Global variables have been disabled by the application\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	g_str->Release();
	g_str = 0;

	// Test - Philip Bennefall
	// Global vars initialized in the wrong order
	{
		const char *script = 
			"test stuff; \n"
			"string whatever; \n"
			"class test \n"
			"{ \n"
			" int whatever; \n"
			" test() \n"
			" { \n"
			"  whatever=create_whatever(); \n"
			" } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"} \n"
			"int create_whatever() \n"
			"{ \n"
			" whatever='HelloStrangeErrors'; \n"
			" return whatever.length(); \n"
			"} \n";

		CBufferedOutStream bout;
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);
		
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod->ResetGlobalVars();

		if( bout.buffer != " (1, 6) : Error   : Failed to initialize global variable 'stuff'\n"
                           " (16, 0) : Info    : Exception 'Null pointer access' in 'int create_whatever()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		bout.buffer = "";

		CBytecodeStream stream(__FILE__"1");
		mod->SaveByteCode(&stream);

		asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		mod2->ResetGlobalVars();

		if( bout.buffer != " (1, 6) : Error   : Failed to initialize global variable 'stuff'\n"
                           " (16, 0) : Info    : Exception 'Null pointer access' in 'int create_whatever()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

