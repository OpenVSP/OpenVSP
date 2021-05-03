#include "utils.h"
#include "scriptmath3d.h"

BEGIN_AS_NAMESPACE
void RegisterStdString_Generic(asIScriptEngine *engine);
END_AS_NAMESPACE

namespace TestUnsafeRef
{

static const char * const TESTNAME = "TestUnsafeRef";

struct Str
{
public:
	Str() {};
	Str(const Str &o) { str = o.str; }

	static void StringConstruct(Str *p) { new(p) Str(); }
	static void StringCopyConstruct(const Str &o, Str *p) { new(p) Str(o); }
	static void StringDestruct(Str *p) { p->~Str(); }

	bool opEquals(const Str &o) { return str == o.str; }
	Str &opAssign(const Str &o) { str = o.str; return *this; }
	Str &opShl(int val) { std::stringstream s; s << str << val; str = s.str(); return *this; }
	Str &opShl(const Str &val) { std::stringstream s; s << str << val.str; str = s.str(); return *this; }

	std::string str;
};

class CStrFactory : public asIStringFactory
{
public:
	const void *GetStringConstant(const char *data, asUINT length)
	{
		Str *str = new Str();
		str->str.assign(data, length);
		return str;
	}
	int ReleaseStringConstant(const void *str)
	{
		delete reinterpret_cast<const Str*>(str);
		return 0;
	}
	int GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (length) *length = (asUINT) reinterpret_cast<const Str*>(str)->str.length();
		if (data) memcpy(data, reinterpret_cast<const Str*>(str)->str.c_str(), reinterpret_cast<const Str*>(str)->str.length());
		return 0;
	}
};

CStrFactory strFactory;

template<typename Type> void constructor(Type* mem)
{
	new (mem) Type;
}
template<typename P, typename Type> void constructor(Type* mem, P p)
{
	new (mem) Type(p);
}
template<typename Type> void copyConstructor(Type* mem, const Type& other)
{
	new (mem) Type(other);
}
template<typename Type> void destructor(Type* obj)
{
	obj->~Type();
}

class C1
{
public:
	C1(void) : m1_(0)
	{
	}
	C1(int m) : m1_(m)
	{
	}
	~C1()
	{
	}

	C1& operator= (const C1&)
	{
		return *this;
	}
	bool operator== (const C1&) const
	{
		return true;
	}

	bool f(void) const
	{
		return true;
	}

private:
	int m1_;
};

bool Test()
{
	bool fail = false;
	int r;

	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine *engine;

	// Test problem with small types and unsafe references
	// Reported by Polyak Istvan
	{
		engine = asCreateScriptEngine();
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		const char* typeName = "C1";

		r = engine->RegisterObjectType(typeName, sizeof(C1), asOBJ_VALUE | asOBJ_APP_CLASS); assert(r >= 0 && "RegisterObjectType");
		r = engine->RegisterObjectBehaviour(typeName, asBEHAVE_CONSTRUCT, "void f ()", asFUNCTIONPR(constructor, (C1*), void), asCALL_CDECL_OBJFIRST); assert(r >= 0 && "asBEHAVE_CONSTRUCT");
		r = engine->RegisterObjectBehaviour(typeName, asBEHAVE_CONSTRUCT, "void f (int)", asFUNCTIONPR(constructor, (C1*, int), void), asCALL_CDECL_OBJFIRST); assert(r >= 0 && "asBEHAVE_CONSTRUCT");
		r = engine->RegisterObjectBehaviour(typeName, asBEHAVE_DESTRUCT, "void f ()", asFUNCTIONPR(destructor, (C1*), void), asCALL_CDECL_OBJFIRST); assert(r >= 0 && "asBEHAVE_DESTRUCT");
		r = engine->RegisterObjectMethod(typeName, "C1 & opAssign (const C1 &in)", asMETHODPR(C1, operator=, (const C1&), C1&), asCALL_THISCALL); assert(r >= 0 && "opAssign");
		r = engine->RegisterObjectMethod(typeName, "bool opEquals (const C1 &in) const", asMETHODPR(C1, operator==, (const C1&) const, bool), asCALL_THISCALL); assert(r >= 0 && "opAssign");
		r = engine->RegisterObjectMethod(typeName, "bool f () const", asMETHODPR(C1, f, (void) const, bool), asCALL_THISCALL); assert(r >= 0 && "opAssign");

		asIScriptModule* mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class C2\n"
			"{\n"
			"	C2(const C1& in c1)\n"
			"	{\n"
			"		c1_ = c1; \n"
			"	}\n"
			"	bool opEquals(const C2& in other) const\n"
			"	{ \n"
			"		c1_ == other.c1_; \n"  // before the fix this code produced a refcpy that overwrote the this pointer, thus causing null pointer access in the next statement
			"		c1_.f(); \n"
			"		return true; \n"
			"	}\n"
			"	private C1 c1_; \n"
			"}\n"
			"void main()\n"
			"{ \n"
			"	C2 a(C1(1)); \n"
			"	C2 b(C1(1)); \n"
			"	a == b; \n"
			"}\n");
 		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptContext* ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByName("main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s\n", GetExceptionInfo(ctx, true).c_str());
			TEST_FAILED;
		}
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test chained operations with global variables and with unsafe ref turned on
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString_Generic(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"const string \n"
			"  sep_g = ';', \n"
			"  row2_g = sep_g + sep_g + sep_g + sep_g; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "assert( row2_g == ';;;;' );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test stream operator on value type with unsafe ref turned on
	// Observe, without unsafe references, the stream operator won't work due to the need to make copies of the value for each expression in order to guarantee safe references
	// https://www.gamedev.net/forums/topic/685927-tepm-object-on-multiple-sub-expression/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 0);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = engine->RegisterObjectType("string", sizeof(Str), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);
		r = engine->RegisterStringFactory("string", &strFactory); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Str::StringConstruct), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(Str::StringCopyConstruct), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Str::StringDestruct), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in)", asMETHOD(Str, opEquals), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("string", "string &opShl(const string &in)", asMETHODPR(Str, opShl, (const Str &), Str&), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("string", "string &opShl(int)", asMETHODPR(Str, opShl, (int), Str&), asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void test(string& lhs) \n"
			"{ \n"
			"	lhs << 'abc' << 456; \n" // chained operators, all working on the same original object
			"} \n");
 		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "string s = '123'; test(s); assert( s == '123abc456' );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Basic tests
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterScriptString(engine);

		r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script1 =
			"void Test()                            \n"
			"{                                      \n"
			"   int[] arr = {0};                    \n"
			"   TestRefInt(arr[0]);                 \n"
			"   Assert(arr[0] == 23);               \n"
			"   int a = 0;                          \n"
			"   TestRefInt(a);                      \n"
			"   Assert(a == 23);                    \n"
			"   string[] sa = {\"\"};               \n"
			"   TestRefString(sa[0]);               \n"
			"   Assert(sa[0] == \"ref\");           \n"
			"   string s = \"\";                    \n"
			"   TestRefString(s);                   \n"
			"   Assert(s == \"ref\");               \n"
			"}                                      \n"
			"void TestRefInt(int &ref)              \n"
			"{                                      \n"
			"   ref = 23;                           \n"
			"}                                      \n"
			"void TestRefString(string &ref)        \n"
			"{                                      \n"
			"   ref = \"ref\";                      \n"
			"}                                      \n";
		mod->AddScriptSection(TESTNAME, script1);
		r = mod->Build();
		if (r < 0)
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "Test()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
			PRINTF("%s: Execution failed: %d\n", TESTNAME, r);
		}

		if (ctx) ctx->Release();

		engine->Release();
	}

	// Passing a const value object to a function expecting a non-const ref
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
		RegisterStdString(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func(string &) {} \n"
			"void main() { \n"
			"  const string foo = 'bar'; \n"
			"  func(foo); \n" // shouldn't be allowed since foo is const and the function expects a non-const reference
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (2, 1) : Info    : Compiling void main()\n"
						   "test (4, 3) : Error   : No matching signatures to 'func(const string)'\n"
						   "test (4, 3) : Info    : Candidates are:\n"
						   "test (4, 3) : Info    : void func(string&inout)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}


	// Test bug
	// http://www.gamedev.net/topic/657960-tempvariables-assertion-with-indexed-unsafe-reference/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetEngineProperty(asEP_USE_CHARACTER_LITERALS, true);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);
		engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);
		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("ShortStringHash", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectBehaviour("ShortStringHash", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("Variant", 4, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
		engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f(const Variant&in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Variant", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Variant", "Variant& opAssign(const Variant&in)", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("VariantMap", 4, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
		engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_CONSTRUCT, "void f(const VariantMap&in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("VariantMap", "Variant& opIndex(ShortStringHash)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("VariantMap", "const Variant& opIndex(ShortStringHash) const", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("UIElement", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("UIElement", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("UIElement", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("UIElement", "VariantMap& get_vars() property", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule* module = engine->GetModule("Test", asGM_ALWAYS_CREATE);

		const char *script =
			"UIElement@ element;\n"
			"VariantMap internalVars;\n"
			"void Test()\n"
			"{\n"
			"    ShortStringHash key; \n"
			"    internalVars[key] = element.vars[key];\n"
			"}\n\n";

		module->AddScriptSection("Test", script);
		r = module->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test value class with unsafe ref
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptMath3D(engine);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"class Good \n"
			"{ \n"
			"  vector3 _val; \n"
			"  Good(const vector3& in val) \n"
			"  { \n"
			"    _val = val; \n"
			"  }  \n"
			"};  \n"
			"class Bad  \n"
			"{  \n"
			"  vector3 _val;  \n"
			"  Bad(const vector3& val)  \n"
			"  {  \n"
			"    _val = val;  \n"
			"  }  \n"
			"}; \n"
			"void test()  \n"
			"{ \n"
			"  // runs fine  \n"
			"  for (int i = 0; i < 2; i++)  \n"
			"    Good(vector3(1, 2, 3));  \n"
			"  // causes vm stack corruption  \n"
			"  for (int i = 0; i < 2; i++)  \n"
			"    Bad(vector3(1, 2, 3));  \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test ref to primitives
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"void func(){ \n"
			"  float a; \n"
			"  uint8 b; \n"
			"  int c; \n"
			"  funcA(c, a, b); \n"
			"} \n"
			"void funcA(float& a, uint8& b, int& c) {} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "TestUnsafeRef (1, 1) : Info    : Compiling void func()\n"
		                   "TestUnsafeRef (5, 3) : Error   : No matching signatures to 'funcA(int, float, uint8)'\n"
		                   "TestUnsafeRef (5, 3) : Info    : Candidates are:\n"
		                   "TestUnsafeRef (5, 3) : Info    : void funcA(float&inout a, uint8&inout b, int&inout c)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test problem found by TheAtom
	// Passing an inout reference to a handle to a function wasn't working properly
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"class T { int a; } \n"
			"void f(T@& p) { \n"
			"  T t; \n"
			"  t.a = 42; \n"
			"  @p = t; \n" // or p=t; in which case t is copied
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "T @t; f(t); assert( t.a == 42 );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();		
	}

	// http://www.gamedev.net/topic/624722-bug-with/
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"class T { T() { val = 123; } int val; } \n"
			"T g_t; \n"
			"T &GetTest() { return g_t; } \n"
			"void f(T@& t) { \n"
			"  assert( t.val == 123 ); \n"
			"} \n"
			"void func() { \n"
			"  f(GetTest()); \n"
			"  f(@GetTest()); \n"
			"  T @t = GetTest(); \n"
			"  f(t); \n"
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "TestUnsafeRef (7, 1) : Info    : Compiling void func()\n"
						   "TestUnsafeRef (8, 3) : Error   : No matching signatures to 'f(T)'\n"
						   "TestUnsafeRef (8, 3) : Info    : Candidates are:\n"
						   "TestUnsafeRef (8, 3) : Info    : void f(T@&inout t)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// http://www.gamedev.net/topic/624722-bug-with/
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"class T { T() { val = 123; } int val; } \n"
			"T g_t; \n"
			"T &GetTest() { return g_t; } \n"
			"void f(T@& t) { \n"
			"  assert( t.val == 123 ); \n"
			"} \n"
			"void func() { \n"
			"  f(cast<T>(GetTest())); \n"
			"  f(@GetTest()); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "func()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}
		ctx->Release();

		engine->Release();
	}

	// http://www.gamedev.net/topic/636443-there-is-no-copy-operator-for-the-type-val-available/
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("Val", sizeof(int), asOBJ_VALUE | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("Val", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		// With unsafe references the copy constructor doesn't have to be in, it can be inout too
		engine->RegisterObjectBehaviour("Val", asBEHAVE_CONSTRUCT, "void f(const Val &)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Val", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"Val GetVal() \n"
			"{ \n"
			"    Val ret; \n"
			"    return ret; \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();		
	}

	// Test passing literal constant to parameter reference
	// http://www.gamedev.net/topic/653394-global-references/
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterGlobalFunction("void func(const int &)", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"const int value = 42; \n"
			"void main() { \n"
			"    func(value); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test with copy constructor that takes unsafe reference
	// http://www.gamedev.net/topic/638613-asassert-in-file-as-compillercpp-line-675/
	SKIP_ON_MAX_PORT
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = engine->RegisterObjectType("string", sizeof(Str), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
		r = engine->RegisterStringFactory("string", &strFactory); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Str::StringConstruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		// Copy constructor takes an unsafe reference
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f(const string &)", asFUNCTION(Str::StringCopyConstruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Str::StringDestruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in)", asMETHOD(Str, opEquals), asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"void SetTexture( string txt ) { assert( txt == 'test' ); } \n"
			"void startGame( ) \n"
			"{ \n"
			"   SetTexture('test'); \n"
			"} \n"
			// It must be possible to pass a const string to a function expecting a string by value
			"void startGame2( ) \n"
			"{ \n"
			"   const string a = 'test'; \n"
			"   SetTexture(a); \n"
			"} \n"
			);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "startGame();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test with assignment operator that takes unsafe reference
	SKIP_ON_MAX_PORT
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = engine->RegisterObjectType("string", sizeof(Str), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
		r = engine->RegisterStringFactory("string", &strFactory); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Str::StringConstruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Str::StringDestruct), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		// Assignment operator takes an unsafe reference
		r = engine->RegisterObjectMethod("string", "string &opAssign(const string &)", asMETHOD(Str, opAssign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in)", asMETHOD(Str, opEquals), asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, 
			"void SetTexture( string txt ) { assert( txt == 'test' ); } \n"
			"void startGame( ) \n"
			"{ \n"
			"   SetTexture('test'); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "startGame();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace
