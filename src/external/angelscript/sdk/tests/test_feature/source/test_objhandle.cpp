#include "utils.h"

namespace TestObjHandle
{

static const char * const TESTNAME = "TestObjHandle";



static const char *script1 =
"refclass@ g;                           \n"
"refclass@ c = @g;                      \n"
"class t                                \n"
"{                                      \n"
"  refclass @m;                         \n"
"  void test() {Assert(@m != null);}    \n"
"}                                      \n"
"void TestObjHandle()                   \n"
"{                                      \n"
"   refclass@ b = @refclass();          \n"
// Should generate an exception
// as g isn't initialized yet.
//"   g = b;                              \n"
//"   b = g;                              \n"
// Do a handle assignment
"   @g = @b;                            \n"
// Now an assignment to g is possible
"   g = b;                              \n"
// Compare with null
"   if( @g != null ) {}                  \n"
"   if( null != @g ) {}                   \n"
// Compare with another object
"   if( @g == @b ) {}                       \n"
"   if( @b == @g ) {}                     \n"
// Value comparison
//"   if( g == b );                       \n"
//"   if( b == g );                       \n"
// Assign null to release the object
"   @g = null;                          \n"
"   @g = @b;                            \n"
// Operators
"   b = g + b;                          \n"
// parameter references
"   @g = null;                          \n"
"   TestObjHandleRef(b, @g);            \n"
"   Assert(@g == @b);                   \n"
// return handles
"   @g = null;                          \n"
"   @g = @TestObjReturnHandle(b);       \n"
"   Assert(@g == @b);                   \n"
"   Assert(@TestReturnNull() == null);  \n"
"   Assert(@TestObjReturnHandle(b) != null); \n"
// Test for class members
"   t cl;                               \n"
"   @cl.m = @TestObjReturnHandle(b);    \n"
"   Assert(@cl.m != null);              \n"
"   cl.test();                          \n"
"}                                      \n"
"void TestObjHandleRef(refclass@ i, refclass@ &out o)  \n"
"{                                                     \n"
"   @o = @i;                                           \n"
"}                                                     \n"
"refclass@ TestObjReturnHandle(refclass@ i)            \n"
"{                                                     \n"
"   return i;                                          \n"
"}                                                     \n"
"refclass@ TestReturnNull()                            \n"
"{                                                     \n"
"   return null;                                       \n"
"}                                                     \n";

// Make sure the handle can be explicitly taken for class properties, array members, and global variables
static const char *script5 =
"class C {int val; C() {val = 0;}}      \n"
"class D {C c;}                         \n"
"C g;                                   \n"
"void Test()                            \n"
"{                                      \n"
"   Func(@g);                           \n"
"   Assert(g.val == 1);                 \n"
"   D d;                                \n"
"   Func(@d.c);                         \n"
"   C[] a1(1);                          \n"
"   Func(@a1[0]);                       \n"
"   Assert(a1[0].val == 1);             \n"
"   C@[] a2(1);                         \n"
"   @a2[0] = @C();                      \n"
"   Func(@a2[0]);                       \n"
"   Assert(a2[0].val == 1);             \n"
"}                                      \n"
"void Func(C@ c) {c.val = 1;}           \n";

class CRefClass
{
public:
	CRefClass()
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("Construct(%X)\n",this);
		refCount = 1;
	}
	~CRefClass()
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("Destruct(%X)\n",this);
	}
	CRefClass &operator=(const CRefClass & /*o*/)
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("Assign(%X, %X)\n", this, &o);
		return *this;
	}
	int AddRef()
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("AddRef(%X)\n",this);
		return ++refCount;
	}
	int Release()
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("Release(%X)\n",this);
		int r = --refCount;
		if( refCount == 0 ) delete this;
		return r;
	}
	static CRefClass &Add(CRefClass &self, CRefClass & /*other*/)
	{
//		asIScriptContext *ctx = asGetActiveContext();
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber());
//		PRINTF("Add(%X, %X)\n", &self, &other);
		return self;
	}
	CRefClass &Do()
	{
		return *this;
	}
	int refCount;
};

CRefClass *Factory()
{
	return new CRefClass;
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine *engine;
	asIScriptModule *mod;
	asIScriptContext *ctx;

	// Test const handles
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"
			"void test(A @const a, const A @const b, const A @const &in c) { \n"
			"  @a = null; \n" // error. a is read-only
			"  a = A(); \n" // ok. a refers to modifiable object
			"  @b = null; \n" // error. b is read-only
			"  b = A(); \n" // error. b refers to read-only object
			"  @c = null; \n" // error. c is read-only
			"  A@const d = a; \n" // ok. both are referring to modifiable objects
			"  A@const e = b; \n" // error. e refers to modifiable object, but b to read-only object
			"  const A@ f = a; \n" // ok. f will have a read-only view of the object
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (2, 1) : Info    : Compiling void test(A@const, const A@const, const A@const&in)\n"
						   "test (3, 3) : Error   : Expression is not an l-value\n"
						   "test (5, 3) : Error   : Expression is not an l-value\n"
						   "test (6, 5) : Error   : Reference is read-only\n"
						   "test (7, 3) : Error   : Expression is not an l-value\n"
						   "test (9, 15) : Error   : Can't implicitly convert from 'const A@' to 'A@&'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}	
		
		engine->ShutDownAndRelease();
	}
	
	// Make sure unnecessary use of addref and release is minimized
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class TestClass { \n"
			"  array<int> a(10); \n"
			"  array<int> @b; \n"
			"  void TestAccessToMemberOfMember() { \n"
			"    int r = a[0]; \n"  // no need to call addref/release on a, since 'this' is already guaranteed to stay alive
			"    r += b[0]; \n"     // must call addref/release on b to guarantee that the object will stay alive
			"  } \n"
			"} \n"
			"void TestGlobalAccessToMemberOfMember() { \n"
			"  TestClass c; \n"
			"  int r = c.a[0]; \n"  // no need to call addref/release on a, since c is already guaranteed to stay alive
			"  r += c.b[0]; \n"     // must call addref/release on b to guarantee that the object will stay alive
			"} \n"
			"void TestHandleAccessToMemberOfMember() { \n"
			"  TestClass @c; \n"
			"  int r = c.a[0]; \n"  // must call addref/release on a, since c is not guaranteed to stay alive
			"  r += c.b[0]; \n"     // must call addref/release on b to guarantee that the object will stay alive
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptFunction *func = mod->GetTypeInfoByName("TestClass")->GetMethodByName("TestAccessToMemberOfMember", false);
		asBYTE expect[] =
		{
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_Thiscall1,asBC_RDR4,asBC_CpyVtoV4,
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_RefCpyV,asBC_Thiscall1,asBC_RDR4,asBC_ADDi,asBC_FREE,
			asBC_SUSPEND,asBC_RET
		};
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;

		func = mod->GetFunctionByName("TestGlobalAccessToMemberOfMember");
		asBYTE expect2[] =
		{
			asBC_SUSPEND,asBC_CALL,asBC_STOREOBJ,
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_Thiscall1,asBC_RDR4,asBC_CpyVtoV4,
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_RefCpyV,asBC_Thiscall1,asBC_RDR4,asBC_ADDi,asBC_FREE,
			asBC_SUSPEND,asBC_FREE,asBC_RET
		};
		if (!ValidateByteCode(func, expect2))
			TEST_FAILED;

		func = mod->GetFunctionByName("TestHandleAccessToMemberOfMember");
		asBYTE expect3[] =
		{
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_RefCpyV,asBC_Thiscall1,asBC_RDR4,asBC_CpyVtoV4,asBC_FREE,
			asBC_SUSPEND,asBC_PshC4,asBC_PshVPtr,asBC_ADDSi,asBC_RDSPtr,asBC_RefCpyV,asBC_Thiscall1,asBC_RDR4,asBC_ADDi,asBC_FREE,
			asBC_SUSPEND,asBC_FREE,asBC_RET
		};
		if (!ValidateByteCode(func, expect3))
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test passing @& argument to a function
	// http://www.gamedev.net/topic/681790-problem-in-arrayinterface/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"interface IItem {\n"
			"}\n"
			"class CItem : IItem {\n"
			"}\n"
			"array<IItem@> items;\n"
			"int32 getIndexOf(const IItem@ &in hItem) {\n"
			"	return items.findByRef(@hItem);\n"
			"}\n"
			"int32 getIndexOf2(const IItem@ hItem) {\n"
			"	return items.findByRef(@hItem);\n"
			"}\n"
			"void check() {\n"
			"	CItem@ hItem = CItem();\n"
			"	items.insertLast(@hItem);\n"
			"	assert( items.findByRef(@hItem) == 0);\n"
			"	assert( getIndexOf2(@hItem) == 0);\n"
			"	assert( getIndexOf(@hItem) == 0);\n"
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		bout.buffer = "";

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "check()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
			TEST_FAILED;
		}
		ctx->Release();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterScriptString(engine);

		r = engine->RegisterObjectType("refclass", sizeof(CRefClass), asOBJ_REF); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("refclass", asBEHAVE_FACTORY, "refclass@ f()", asFUNCTION(Factory), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("refclass", asBEHAVE_ADDREF, "void f()", asMETHOD(CRefClass, AddRef), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("refclass", asBEHAVE_RELEASE, "void f()", asMETHOD(CRefClass, Release), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("refclass", "refclass &opAssign(refclass &in)", asMETHOD(CRefClass, operator=), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("refclass", "refclass &Do()", asMETHOD(CRefClass, Do), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("refclass", "refclass &opAdd(refclass &in)", asFUNCTION(CRefClass::Add), asCALL_CDECL_OBJFIRST); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = mod->Build();
		if (r < 0)
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestObjHandle()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			TEST_FAILED;
			PRINTF("%s: Execution failed\n", TESTNAME);
		}
		if (ctx) ctx->Release();

		// Call TestObjReturnHandle() from the application to verify that references are updated as necessary
		ctx = engine->CreateContext();
		ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("refclass@ TestObjReturnHandle(refclass@)"));
		CRefClass *refclass = new CRefClass();

		ctx->SetArgObject(0, refclass);

		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			TEST_FAILED;
			PRINTF("%s: Execution failed\n", TESTNAME);
		}
		if (refclass->refCount != 2)
		{
			TEST_FAILED;
			PRINTF("%s: Ref count is wrong\n", TESTNAME);
		}

		refclass->Release();
		if (ctx) ctx->Release();

		// Test returning a reference to the object from an object method
		r = engine->GarbageCollect();
		asUINT gcCurrentSize;
		engine->GetGCStatistics(&gcCurrentSize, 0, 0);
		assert(gcCurrentSize == 0);

		r = ExecuteString(engine, "refclass ref; ref.Do()");
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//--------------------
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script5, strlen(script5), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		r = ExecuteString(engine, "Test()", mod);
		if (r != asEXECUTION_FINISHED) TEST_FAILED;
		engine->Release();
	}

	//----------------------
	// It should be allowed to have a global function return a handle to a const object
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		const char *scriptC = "class T {} const T@ func() {return T();}";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", scriptC, strlen(scriptC));
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		engine->Release();
	}

	//---------------------
	// These tests are designed to make sure ambiguities with handles is avoided
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		r = engine->RegisterObjectType("A", sizeof(CRefClass), asOBJ_REF); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("A", asBEHAVE_FACTORY, "A@ f()", asFUNCTION(Factory), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("A", asBEHAVE_ADDREF, "void f()", asMETHOD(CRefClass, AddRef), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("A", asBEHAVE_RELEASE, "void f()", asMETHOD(CRefClass, Release), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("A", "A &opAssign(const A &in)", asMETHOD(CRefClass, operator=), asCALL_THISCALL); assert(r >= 0);

		bout.buffer = "";
		r = ExecuteString(engine, "A a; a == null;");    // Should give warning
		if (r < 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; null == a;");    // Should give warning
		if (r < 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; @a == null;");   // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; null == @a;");   // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; @a == a;");      // Should give warning
		if (r < 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; a == @a;");      // Should give warning
		if (r < 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; @a == @a;");     // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A @a = null;");       // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; A @b = a;");     // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; A @b = @a;");    // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; A b = @b;");     // Should give error
		if (r >= 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A @a, b; @a = @b;");  // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A @a, b; @a = b;");   // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A @a, b; a = @b;");   // Should give error
		if (r >= 0 || bout.buffer == "")
		{
			TEST_FAILED;
		}

		bout.buffer = "";
		r = ExecuteString(engine, "A a; null is a;");    // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}
		bout.buffer = "";
		r = ExecuteString(engine, "A a; a !is null;");    // OK
		if (r < 0 || bout.buffer != "")
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

