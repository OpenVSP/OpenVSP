#include "utils.h"

namespace TestRefArgument
{

static const char * const TESTNAME = "TestRefArgument";

static const char *script1 =
"void TestObjHandle(refclass &in ref)   \n"
"{                                      \n"
"   float r;                            \n"
"   test2(r);                           \n"
"   Assert(ref.id == int(0xdeadc0de));  \n"
"   test(ref);                          \n"
"   test3(r);                           \n"
"   Assert(r == 1.0f);                  \n"
"}                                      \n"
"void test(refclass &in ref)            \n"
"{                                      \n"
"   Assert(ref.id == int(0xdeadc0de));  \n"
"}                                      \n"
"void test2(float &out ref)             \n"
"{                                      \n"
"}                                      \n"
"void test3(float &out a)               \n"
"{                                      \n"
"   a = 1.0f;                           \n"
"}                                      \n";

static const char *script2 = 
"void Test()                            \n"
"{                                      \n"
"  float[] a(2);                        \n"
"  Testf(a[1]);                         \n"
"}                                      \n"
"void Testf(float &inout a)             \n"
"{                                      \n"
"}                                      \n";

static const char *script3 = 
"void Test()                            \n"
"{                                      \n"
"  string b;                            \n"
"  Testref(b);                          \n"
"  Assert(b == \"test\");               \n"
"  string[] a(1);                       \n"
"  Testref(a[0]);                       \n"
"  Assert(a[0] == \"test\");            \n"
"}                                      \n"
"void Testref(string &inout s)          \n"
"{                                      \n"
"  s = \"test\";                        \n"
"}                                      \n";

class CRefClass
{
public:
	CRefClass() 
	{
		id = 0xdeadc0de;
	}
	~CRefClass() 
	{
	}
	CRefClass &operator=(const CRefClass &other) {id = other.id; return *this;}
	int id;
};

static void Assign(asIScriptGeneric *gen)
{
	CRefClass *obj = (CRefClass*)gen->GetObject();
	*obj = *(CRefClass*)gen->GetArgAddress(0);
}

static const char *script4 = 
"void Test()                            \n"
"{                                      \n"
"  int a = 0;                           \n"
"  float b = 0.0f;                      \n"
"  int c = 45;                          \n"
"  float d = 4.0f;                      \n"
"  TestNativeRefArgOut(a,b,c,d);        \n"
"  Assert(a == 10);                     \n"
"  Assert(b > 3.13f && b < 3.15f);      \n"
"}                                      \n";

static bool NativeTestFail = false;
static void TestNativeRefArgOut( int *a, float *b, const int *c, const float *d )
{
	assert( a != NULL );
	assert( b != NULL );
	assert( c != NULL );
	assert( d != NULL );
	*a = 10;
	*b = 3.14f;
	if( (*c) != 45 ) NativeTestFail = true;
	if( !CompareFloat( *d, 4.0f ) ) NativeTestFail = true;
}

bool Test()
{
	bool fail = false;
	bool testNative = false;
	int r;
	COutStream out;

	if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		testNative = true;
	}

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	RegisterScriptString(engine);
	r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );


	r = engine->RegisterObjectType("refclass", sizeof(CRefClass), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA); assert(r >= 0);
	r = engine->RegisterObjectProperty("refclass", "int id", asOFFSET(CRefClass, id)); assert(r >= 0);
	r = engine->RegisterObjectMethod("refclass", "refclass &opAssign(refclass &in)", asFUNCTION(Assign), asCALL_GENERIC); assert( r >= 0 );


	if( testNative )
	{
		r = engine->RegisterGlobalFunction("void TestNativeRefArgOut(int &out,float &out,const int &in,const float &in)", asFUNCTION(TestNativeRefArgOut), asCALL_CDECL); assert( r >= 0 );
	}


	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}
	asIScriptContext *ctx = engine->CreateContext();
	asIScriptFunction *func = engine->GetModule(0)->GetFunctionByName("TestObjHandle");

	CRefClass cref;	
	r = ctx->Prepare(func); assert(r >= 0);
	ctx->SetArgObject(0, &cref);
	r = ctx->Execute();  assert(r >= 0);


	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
		PRINTF("%s: Execution failed: %d\n", TESTNAME, r);
	}
	if( ctx ) ctx->Release();

	//-------------------
	CBufferedOutStream bout;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	r = mod->Build();
	if( !engine->GetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES) )
	{
		if( r >= 0 ) TEST_FAILED;
		if( bout.buffer != "TestRefArgument (6, 18) : Error   : Only object types that support object handles can use &inout. Use &in or &out instead\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}
	else
		if( r != 0 ) TEST_FAILED;

	//----------------------
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
	r = mod->Build();
	if( r < 0 ) 
		TEST_FAILED;
	r = ExecuteString(engine, "Test()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	//-------------------
	if( testNative )
	{
		CBufferedOutStream dout;
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &dout, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script4, strlen(script4), 0);
		r = mod->Build();
		if( r < 0 ) TEST_FAILED;
		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED ) TEST_FAILED;
		if( NativeTestFail ) TEST_FAILED;
	}

	engine->Release();

	//-------------------------
	// Test error message when registering a value type as &inout (or just &)
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterGlobalFunction("void func(float &)", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "System function (1, 17) : Error   : Only object types that support object handles can use &inout. Use &in or &out instead\n" 
		                   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void func(float &)' (Code: asINVALID_DECLARATION, -10)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//---------------------
	// Test passing 0 to out ref
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		const char *script = 
			"void f(float &out d) {}";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "f(0);", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}	

	//------------------------
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		const char *script = 
			"void f(int & d) {d = 1;}";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "f(0);", mod);
		if( r > 0 )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 3) : Error   : Not a valid reference\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();	
	}

	// Success
	return fail;
}

} // namespace
