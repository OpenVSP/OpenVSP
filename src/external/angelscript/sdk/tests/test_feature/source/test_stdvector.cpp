#include <memory>

#include "stdvector.h"
#include "utils.h"


namespace TestStdVector
{

static const char * const TESTNAME = "TestStdVector";


static void print(std::string &str)
{
	PRINTF("%s", str.c_str());
}

static void print(int num)
{
	PRINTF("%d", num);
}

static const char *script1 =
"void Test()                         \n"
"{                                   \n"
"   TestInt();                       \n"
"   TestChar();                      \n"
"   Test2D();                        \n"
"}                                   \n"
"                                    \n"
"void TestInt()                      \n"
"{                                   \n"
"   int[] A(5);                      \n"
"   Assert(A.size() == 5);           \n"
"	A.push_back(6);                  \n"
"   Assert(A.size() == 6);           \n"
"	int[] B(A);                      \n"
"   Assert(B.size() == 6);           \n"
"	A.pop_back();                    \n"
"   Assert(B.size() == 6);           \n"
"   Assert(A.size() == 5);           \n"
"	A = B;                           \n"
"   Assert(A.size() == 6);           \n"
"	A.resize(8);                     \n"
"   Assert(A.size() == 8);           \n"
"	A[1] = 20;                       \n"
"	Assert(A[1] == 20);              \n"
"}                                   \n"
"                                    \n"
"void TestChar()                     \n"
"{                                   \n"
"   int8[] A(5);                     \n"
"   Assert(A.size() == 5);           \n"
"   A.push_back(6);                  \n"
"   Assert(A.size() == 6);           \n"
"   int8[] B(A);                     \n"
"   Assert(B.size() == 6);           \n"
"   A.pop_back();                    \n"
"   Assert(B.size() == 6);           \n"
"   Assert(A.size() == 5);           \n"
"   A = B;                           \n"
"   Assert(A.size() == 6);           \n"
"   A.resize(8);                     \n"
"   Assert(A.size() == 8);           \n"
"   A[1] = 20;                       \n"
"   Assert(A[1] == 20);              \n"
"}                                   \n"
"                                    \n"
"void Test2D()                       \n"
"{                                   \n"
"   int[][] A(2);                    \n"
"   int[] B(2);                      \n"
"   A[0] = B;                        \n"
"   A[1] = B;                        \n"
"                                    \n"
"   A[0][0] = 0;                     \n"
"   A[0][1] = 1;                     \n"
"   A[1][0] = 2;                     \n"
"   A[1][1] = 3;                     \n"
"                                    \n"
"   Assert(A[0][0] == 0);            \n"
"   Assert(A[0][1] == 1);            \n"
"   Assert(A[1][0] == 2);            \n"
"   Assert(A[1][1] == 3);            \n"
"}                                   \n";

static const char *script2 =
"void Test()                         \n"
"{                                   \n"
"  MyStruct[] A;                     \n"
"  MyClass B;                        \n"
"  A = B.myVec;                      \n"
"  Assert(A.size() == 2);            \n"
"}                                   \n";

static const char *script3 = 
"void Test(string[] v)               \n"
"{                                   \n"
"  Assert(v.size() == 1);            \n"
"  Assert(v[0] == \"test\");         \n"
"}                                   \n";

using namespace std;

class MyStruct
{
public:
	int v;
};

class MyClass
{
public:
	vector<MyStruct> myVec;
};

void MyClass_Construct(MyClass *o)
{
	new(o) MyClass();
	o->myVec.resize(2);
}

void MyClass_Destruct(MyClass *o)
{
	o->~MyClass();
}

bool Test()
{
	RET_ON_MAX_PORT

	CBufferedOutStream bout;
	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);
	RegisterVector<char>("int8[]", "int8", engine);
	RegisterVector<int>("int[]", "int", engine);
#if !defined(_MSC_VER) || _MSC_VER >= 1500 || defined(__BORLANDC__)
	RegisterVector<string>("string[]", "string", engine);
	RegisterVector< std::vector<int> >("int[][]", "int[]", engine);
#else
	// There is something going wrong when registering the following.
	// It looks like it is a linker problem, but I can't be sure.
	PRINTF("%s: MSVC6 can't register vector< vector<int> >\n", TESTNAME);

	// It seems that MSVC isn't able to differ between the template instances,
	// when registering the different array overloads, all function pointers 
	// are the same.

	// This is true at least for MSVC6. I don't know about other versions
#endif
	engine->RegisterGlobalFunction("void Print(string &in)", asFUNCTIONPR(print, (std::string&), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void Print(int)", asFUNCTIONPR(print, (int), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	r = engine->RegisterObjectType("MyStruct", sizeof(MyStruct), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS); assert( r >= 0 );
	r = engine->RegisterObjectProperty("MyStruct", "int v", asOFFSET(MyStruct, v)); assert( r >= 0 );

	RegisterVector<MyStruct>("MyStruct[]", "MyStruct", engine);

	r = engine->RegisterObjectType("MyClass", sizeof(MyClass), asOBJ_VALUE | asOBJ_APP_CLASS_CD); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MyClass", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(MyClass_Construct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("MyClass", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(MyClass_Destruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectProperty("MyClass", "MyStruct[] myVec", asOFFSET(MyClass,myVec)); assert( r >= 0 );
 
	{
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script = 
		"MyStruct[] TestInt() \n"
		"{ \n"
		"  MyStruct[] a; \n"
		"  MyStruct m; m.v=10; \n"
		"  a.push_back(m); \n"
		"  return a; \n"
		"} \n";
		mod->AddScriptSection("s", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// The object that was returned by value, must not be freed too early
		r = ExecuteString(engine, "Assert(TestInt()[0].v == 10)", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "Test()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: Failed to execute script\n", TESTNAME);

		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		
		TEST_FAILED;
	}
	
	if( ctx ) ctx->Release();

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	ctx = engine->CreateContext();
	r = ExecuteString(engine, "Test()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: Failed to execute script\n", TESTNAME);

		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		
		TEST_FAILED;
	}
	
	if( ctx ) ctx->Release();

	mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	ctx = engine->CreateContext();
	r = ctx->Prepare(mod->GetFunctionByDecl("void Test(string[] v)"));
	if( r < 0 ) TEST_FAILED;
	vector<string> local;
	local.push_back(string("test"));
	r = ctx->SetArgObject(0, &local);
	if( r < 0 ) TEST_FAILED;

	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		TEST_FAILED;
	}

	ctx->Release();

	engine->Release();

	// Success
	return fail;
}

} // namespace

