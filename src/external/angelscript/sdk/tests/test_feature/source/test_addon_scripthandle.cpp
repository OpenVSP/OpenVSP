#include "utils.h"
#include "../../../add_on/scripthandle/scripthandle.h"
#include "../../../add_on/scriptarray/scriptarray.h"

namespace Test_Addon_ScriptHandle
{

static const char *TESTNAME = "Test_Addon_ScriptHandle";

void ReceiveRefByValue(CScriptHandle hndl)
{
	asITypeInfo *type = hndl.GetType();
	if( type )
		std::string str(type->GetName());
}

void ReceiveRefByRef(CScriptHandle &/*hndl*/)
{
}

CScriptHandle GetFunc1()
{
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();
	asIScriptModule *mod = engine->GetModule("test");

	asIScriptFunction *func1 = mod->GetFunctionByName("func1");

	CScriptHandle ref;
	ref.Set(func1, engine->GetTypeInfoById(func1->GetTypeId()));

	return ref;
}

CScriptHandle ReturnRef()
{
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();
	asIScriptModule *mod = engine->GetModule("test");
	asITypeInfo *type = mod->GetTypeInfoByName("CTest");

	asIScriptObject *obj = reinterpret_cast<asIScriptObject *>(engine->CreateScriptObject(type));

	CScriptHandle ref;
	ref.Set(obj, type);

	// Need to release our reference as the CScriptHandle counts its own, and we will not keep our reference
	obj->Release();

	return ref;
}

std::string g_buf;
void print(asIScriptGeneric *gen)
{
	const std::string &s = *(std::string*)gen->GetArgAddress(0);
	g_buf += s;
	g_buf += "\n";
}

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptContext *ctx;
	asIScriptEngine *engine;

	// Test compiler error with ref
	// https://www.gamedev.net/forums/topic/698645-version-2330-wip-crash-fix-included/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptHandle(engine);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { int j=0; \n"
			"ref@ myRef = j; \n"
			"@myRef = j; } \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
						   "test (2, 14) : Error   : Object handle is not supported for this type\n"
						   "test (3, 10) : Error   : Object handle is not supported for this type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();

	}

	// Test passing null to ref
	// https://www.gamedev.net/forums/topic/698628-cscripthandle-null/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptHandle(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {  }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "A@ obj; \n"
								  "ref x(@obj); \n"
								  "ref y(null); \n", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "A@ obj; \n"
			"ref x(obj); \n", mod);   // causes null pointer exception as it tries to do a value copy of obj
		if (r != asEXECUTION_EXCEPTION)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test circular reference involving ref
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptHandle(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class A { ref @m; }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		// Create the circular reference
		r = ExecuteString(engine, "A a; @a.m = a;", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->GarbageCollect();

		asUINT currSize, totDestroy, totDetect;
		engine->GetGCStatistics(&currSize, &totDestroy, &totDetect);
		if (currSize != 0 || totDestroy != 1 || totDetect != 1)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test that correct type is handled on inheritance
	// https://www.gamedev.net/forums/topic/694164-scripthandle-addon-doesnt-check-object-type/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptHandle(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Foo \n"
			"{ \n"
			"} \n"
			"class Bar : Foo \n"
			"{ \n"
			"} \n"
			"void BugTest() \n"
			"{ \n"
			"	ref@ x = Foo(); \n"
			"	Bar@ y = cast<Bar>(x); \n"
			"   assert( y is null ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		
		r = ExecuteString(engine, "BugTest()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		
		engine->ShutDownAndRelease();
	}

	// Test assigning directly to out reference
	// This also tests that derived script classes can be properly cast to
	// http://www.gamedev.net/topic/660025-inconsistent-behavior-with-ref-type-and-out-references-to-handle-params/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptHandle(engine);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);

		const char *script = 
			"class base {}\n"
			"class derived : base {}\n"
			"class unrelated {}\n"
			"void fillInProperType(ref@ obj, base@& out b, unrelated@& out u)\n"
			"{\n"
			"	if ( obj is null )\n"
			"	{\n"
			"		print('obj is null');\n"
			"		return;\n"
			"	}\n"
			"	@b = cast<base>(obj);\n"
			"	@u = cast<unrelated>(obj);\n"
			"	if ( b !is null )\n"
			"		print('b');\n"
			"	if ( u !is null )\n"
			"		print('u');\n"
			"}\n";

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		g_buf = "";
		r = ExecuteString(engine, 
			"ref@ r = null;\n"
			"base@ b = null;\n"
			"unrelated@ u = null;\n"
			"fillInProperType(r, b, u);\n"
			"assert( b is null && u is null );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( g_buf != "obj is null\n" )
		{
			PRINTF("%s", g_buf.c_str());
			TEST_FAILED;
		}

		g_buf = "";
		r = ExecuteString(engine, 
			"ref@ r = base();\n"
			"base@ b = null;\n"
			"unrelated@ u = null;\n"
			"fillInProperType(r, b, u);\n"
			"assert( b !is null && u is null );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( g_buf != "b\n" )
		{
			PRINTF("%s", g_buf.c_str());
			TEST_FAILED;
		}

		g_buf = "";
		r = ExecuteString(engine, 
			"ref@ r = unrelated();\n"
			"base@ b = null;\n"
			"unrelated@ u = null;\n"
			"fillInProperType(r, b, u);\n"
			"assert( b is null && u !is null );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( g_buf != "u\n" )
		{
			PRINTF("%s", g_buf.c_str());
			TEST_FAILED;
		}

		g_buf = "";
		r = ExecuteString(engine, 
			"ref@ r = derived();\n"
			"base@ b = null;\n"
			"unrelated@ u = null;\n"
			"fillInProperType(r, b, u);\n"
			"assert( b !is null && u is null );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( g_buf != "b\n" )
		{
			PRINTF("%s", g_buf.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test storing a script object in a handle, then release the engine
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		const char *script = 
			"class Test { Test@ t; } \n"; // garbage collected class
		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asITypeInfo *type = mod->GetTypeInfoByName("Test");
		asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(type);

		// Store the object in the handle
		CScriptHandle handle(obj, type);

		obj->Release();

		// Release engine. handle is still holding on to the object
		engine->Release();

		// The engine is really only destroyed after the handle has been destroyed
	}

	// Basic tests with the CScriptHandle type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptHandle(engine);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterGlobalFunction("void ReceiveRefByVal(ref@)", asFUNCTION(ReceiveRefByValue), asCALL_CDECL);
		engine->RegisterGlobalFunction("void ReceiveRefByRef(ref&in)", asFUNCTION(ReceiveRefByRef), asCALL_CDECL);
		engine->RegisterGlobalFunction("ref @ReturnRef()", asFUNCTION(ReturnRef), asCALL_CDECL);
		engine->RegisterGlobalFunction("ref @GetFunc1()", asFUNCTION(GetFunc1), asCALL_CDECL);
#else
		engine->RegisterGlobalFunction("void ReceiveRefByVal(ref@)", WRAP_FN(ReceiveRefByValue), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void ReceiveRefByRef(ref&in)", WRAP_FN(ReceiveRefByRef), asCALL_GENERIC);
		engine->RegisterGlobalFunction("ref @ReturnRef()", WRAP_FN(ReturnRef), asCALL_GENERIC);
		engine->RegisterGlobalFunction("ref @GetFunc1()", WRAP_FN(GetFunc1), asCALL_GENERIC);
#endif
		CScriptHandle handle;
		r = engine->RegisterGlobalProperty("ref @g_handle", &handle); assert( r >= 0 );


		// TODO: optimize: assert( ha !is null ); is producing code that unecessarily calls ClrVPtr and FREE for the null handle
		const char *script =
							 "class A {} \n"
							 "class B {} \n"
							 "void main() \n"
							 "{ \n"
							 "  ref@ ra, rb; \n"
							 "  A a; B b; \n"
							 // Assignment of reference
							 "  @ra = @a; \n"
							 "  assert( ra is a ); \n"
							 "  @rb = @b; \n"
							 // Casting to reference
							 "  A@ ha = cast<A>(ra); \n"
							 "  assert( ha !is null ); \n"
							 "  B@ hb = cast<B>(ra); \n"
							 "  assert( hb is null ); \n"
							 // Assigning null, and comparing with null
							 "  @ra = null; \n"
							 "  assert( ra is null ); \n"
							 "  func2(ra); \n"
							 // Handle assignment with explicit handle
							 "  @ra = @rb; \n"
							 "  assert( ra is b ); \n"
							 "  assert( rb is b ); \n"
							 "  assert( ra is rb ); \n"
							 // Handle assignment with implicit handle
							 "  @rb = rb; \n"
							 "  assert( rb is b ); \n"
							 "  assert( ra is rb ); \n"
							 // Function call and return value
							 "  @rb = func(rb); \n"
							 "  assert( rb is b ); \n"
							 "  assert( func(rb) is b ); \n"
							 // Arrays of handles
							 "  array<ref@> arr(2); \n"
							 "  assert( arr[0] is null ); \n"
							 "  @arr[0] = a; \n"
							 "  @arr[1] = a; \n"
							 "  assert( arr[0] is arr[1] ); \n"
							 "  assert( arr[0] is a ); \n"
							 // Implicit conv from type to ref
							 "  func2(null); \n"
							 "  func(a); \n"
							 "  assert( func(a) is a ); \n"
							 "} \n"
							 "ref@ func(ref@ r) { return r; } \n"
							 "void func2(ref@r) { assert( r is null ); } \n"
							 "interface ITest {} \n"
							 "class CBase {} \n"
							 "class CTest : ITest, CBase \n"
							 "{ \n"
							 "  int val; \n"
							 "  CTest() \n"
							 "  { \n"
							 "    val = 42; \n"
							 "  } \n"
							 "} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			PRINTF("%s: Failed to execute script\n", TESTNAME);
			TEST_FAILED;
		}
		if( ctx ) ctx->Release();

		r = ExecuteString(engine, "ref @r; ReceiveRefByVal(r);", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// This will cause an implicit cast to 'ref'. The object must be released properly afterwards
		r = ExecuteString(engine, "ReceiveRefByRef(A());", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test return by ref
		r = ExecuteString(engine, "ref @r = ReturnRef(); \n"
								  "assert( r !is null ); \n"
								  "CTest @t = cast<CTest>(r); \n"
								  "assert( t !is null ); \n"
								  "assert( t.val == 42 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test that the cast op can determine relationship between object types
		r = ExecuteString(engine, "ref @r(CTest()); \n"
								  "ITest @t = cast<ITest>(r); \n"
								  "assert( t !is null ); \n"
								  "CBase @b = cast<CBase>(r); \n"
								  "assert( b !is null ); \n"
								  "@r = null; \n" // Clear the content
								  "@r = b; \n" // Set the same object, but this time as the base type
								  "@t = cast<ITest>(r); \n"
								  "assert( t !is null ); \n"
								  "CTest @d = cast<CTest>(r); \n"
								  "assert( d !is null ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test that it is possible to cast the return value directly
		r = ExecuteString(engine, "CTest @t = cast<CTest>(ReturnRef()); \n"
                                  "assert( t !is null ); \n"
                                  "assert( t.val == 42 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test function handles in ref object
		mod->AddScriptSection("test",
			"funcdef void FUNC1(); \n"
			"funcdef void FUNC2(int); \n"
			"void func1() {} \n"
			"void func2(int) {} \n"
			"void func3(float) {} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "ref @r; \n"
								  "@r = @func1; ReceiveRefByVal(r); \n"
								  "@r = func2; ReceiveRefByVal(r); \n"
								  "@r = @func3; ReceiveRefByVal(r); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test proper casting of function pointers
		r = ExecuteString(engine, "ref @r; \n"
								  "@r = func1; \n"
								  "assert( cast<FUNC1>(r) !is null ); \n"
								  "@r = func2; \n"
								  "assert( cast<FUNC1>(r) is null ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test setting function pointer from application
		r = ExecuteString(engine, "ref @r = GetFunc1(); \n"
								  "assert( cast<FUNC1>(r) !is null ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// This will cause an implicit cast to 'ref'. The object must be released properly afterwards
		mod->AddScriptSection("test", 
			"class B { B(int) {} } \n"
			"class A { \n"
			"  B @a; \n"
			"  void test() { \n"
			"    ReceiveRefByVal(a); \n"
			"  } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "A a; a.test();", mod);
		if( r != asEXECUTION_FINISHED ) 
			TEST_FAILED;

		// Give appropriate error if not declared with @
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "ref r = array<int>(10);"); // no opAssign available
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 5) : Error   : No appropriate opAssign method found in 'ref' for value assignment\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// It's not allowed to do a value assign of a ref to another ref
		bout.buffer = "";
		r = ExecuteString(engine, "ref@ a, b; a = b;"); // value assign isn't available
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 14) : Error   : No appropriate opAssign method found in 'ref' for value assignment\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test calling script function that takes CScriptHandle
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptHandle(engine);
		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		
		const char *script =
			"class Test {} \n"
			"void func(ref @t) { \n"
			"  assert( t !is null ); \n"
			"} \n";
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CScriptHandle ref;
		asITypeInfo *type = mod->GetTypeInfoByName("Test");
		asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(type);
		ref.Set(obj, type);

		asIScriptFunction *func = mod->GetFunctionByName("func");
		ctx = engine->CreateContext();
		ctx->Prepare(func);
		ctx->SetArgObject(0, &ref);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		ref.Set(0,0);
		obj->Release();

		ctx->Release();
		
		engine->Release();
	}

	// Test setting and getting a function pointer in the CScriptHandle from the application side
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		asIScriptFunction *func = engine->GetFunctionById(r);

		CScriptHandle hndl;
		
		// Pass the function pointer to the script handle
		hndl.Set(func, engine->GetTypeInfoById(func->GetTypeId()));

		// Verify the type id
		int typeId = hndl.GetTypeId();
		if( (typeId & ~asTYPEID_OBJHANDLE) != func->GetTypeId() )
			TEST_FAILED;

		// Retrieve the function
		if( hndl.GetType()->GetFlags() & asOBJ_FUNCDEF )
		{
			hndl.Cast((void**)&func, hndl.GetTypeId());

			if( strcmp(func->GetName(), "assert") != 0 )
				TEST_FAILED;

			func->Release();
		}

		// Clear the handle to release the reference
		hndl.Set(0,0);

		engine->Release();
	}

	// Success
	return fail;
}


} // namespace

