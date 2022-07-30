#include "utils.h"
#include "../../../add_on/scriptdictionary/scriptdictionary.h"
#include "../../../add_on/scriptany/scriptany.h"

namespace TestCastOp
{

const char *script = "\
interface intf1            \n\
{                          \n\
  void Test1();            \n\
}                          \n\
interface intf2            \n\
{                          \n\
  void Test2();            \n\
}                          \n\
interface intf3            \n\
{                          \n\
  void Test3();            \n\
}                          \n\
class clss : intf1, intf2  \n\
{                          \n\
  void Test1() {}          \n\
  void Test2() {}          \n\
}                          \n";


// In this test must be possible to call Func both
// with an uint and a double. The path via TestObj2
// must not be considered by the compiler, as that 
// would make: Func(TestObj(TestObj2(2)));
/*
const char *script2 = "\
class TestObj                                     \n\
{                                                 \n\
    TestObj(int a) {this.a = a;}                  \n\
	TestObj(TestObj2 a) {this.a = a.a;}           \n\
	int a;                                        \n\
}                                                 \n\
// This object must not be used to get to TestObj \n\
class TestObj2                                    \n\
{                                                 \n\
    TestObj2(int a) {assert(false);}              \n\
	int a;                                        \n\
}                                                 \n\
void Func(TestObj obj)                            \n\
{                                                 \n\
    assert(obj.a == 2);                           \n\
}                                                 \n\
void Test()                                       \n\
{                                                 \n\
	Func(2);                                      \n\
	Func(2.1);                                    \n\
}                                                 \n";
*/

// In this test it must not be possible to implicitly convert using 
// a path that requires multiple object constructions, e.g.
// Func(TestObj1(TestObj2(2)));
const char *script3 = 
"class TestObj1                 \n"
"{                              \n"
"  TestObj1(TestObj2 a) {}      \n"
"}                              \n"
"class TestObj2                 \n"
"{                              \n"
"  TestObj2(int a) {}           \n"
"}                              \n"
"void Func(TestObj1 obj) {}     \n"
"void Test()                    \n"
"{                              \n"
"  Func(2);                     \n"
"}                              \n";

void TypeToString(asIScriptGeneric *gen)
{
//	int *i = (int*)gen->GetArgPointer(0);
	*(CScriptString**)gen->GetAddressOfReturnLocation() = new CScriptString("type");
}

class A
{
public:
	operator const char * ( ) const { return 0; }
};

class EventSource
{
public:
	EventSource() {refCount = 1; value = 42;};
	virtual ~EventSource() {}
	virtual void AddRef() {refCount++;}
	virtual void Release() {if( --refCount == 0 ) delete this;}
	int refCount;

	int value;
};

class ASConsole : public EventSource
{
public:
	static ASConsole *factory() { return new ASConsole(); }
//	EventSource *opCast() { return this; }
};

ASConsole* c= 0;
bool g_fail = false;
template<class A, class B> B* ASRefCast(A* a)
{
	if( a != c )
		g_fail = true;
		
	// If the handle already is a null handle, then just return the null handle
	if (a==NULL) return NULL;
	// Now try to dynamically cast the pointer to the wanted type
	B* b = dynamic_cast<B*>(a);
	if (b!=NULL) {
			// Since the cast was made, we need to increase the ref counter for the returned handle
			b->AddRef();
	}
//	PRINTF("ASRefCast: returning %p\n", b);
	return b;
}

void addListener(EventSource* source, int /*mask*/) 
{
	if( source != c )
		g_fail = true;

//    PRINTF("addListener: source = %p\n", source);
//    PRINTF("addListener: source.value = %d\n", (int)source->value);
//	PRINTF("\n");
}

class base
{
public:
	base() { refCount = 1; }
	static void *factory() { return new base(); }
	void addref() { /*printf("addref\n");*/ refCount++; }
	void release() { /*printf("release\n");*/ if (--refCount == 0) delete this; }
	void opCast(void **ref, int typeId) {
		*ref = 0; 
		asIScriptContext *ctx = asGetActiveContext();
		asIScriptEngine *engine = ctx->GetEngine();
		asITypeInfo *type = engine->GetTypeInfoById(typeId);
		if (std::string(type->GetName()) == "CControlBase")
		{
			addref();
			*ref = this;
		}
	}

	int refCount;
};

bool Test()
{
	bool fail = false;
	int r;
	asIScriptEngine *engine;

	CBufferedOutStream bout;
	COutStream out;

	// TODO: What should the compiler do when the class has both a valid opCast method 
	//       and a related class in a class hierarchy? Should prefer calling opCast, right?
	//       How does C++ do it?

	// Test opCast(?&out) with CScriptAny
	// https://www.gamedev.net/forums/topic/697067-refcounting-in-opcast/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		RegisterScriptAny(engine);
		r = engine->RegisterObjectMethod("any", "void opCast(?&out)", asFUNCTION(0), asCALL_GENERIC);
		r = engine->RegisterGlobalFunction("any@ GetA()", asFUNCTION(0), asCALL_GENERIC);
		
		asIScriptModule *mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"shared class A {}"
			"A@ a = cast<A>(GetA());"
		);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test void opCast(?&out) in complex expression
	// https://www.gamedev.net/forums/topic/697067-refcounting-in-opcast/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)\n", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("base", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("base", asBEHAVE_FACTORY, "base @f()", asFUNCTION(base::factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("base", asBEHAVE_ADDREF, "void f()", asMETHOD(base, addref), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("base", asBEHAVE_RELEASE, "void f()", asMETHOD(base, release), asCALL_THISCALL);
		engine->RegisterObjectMethod("base", "void opCast(?&out)", asMETHOD(base, opCast), asCALL_THISCALL);

		engine->RegisterObjectType("CControlBase", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("CControlBase", asBEHAVE_ADDREF, "void f()", asMETHOD(base, addref), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("CControlBase", asBEHAVE_RELEASE, "void f()", asMETHOD(base, release), asCALL_THISCALL);

		RegisterScriptArray(engine, false);
		
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class scene { \n"
			"  array<base@> Mobils = { base() }; \n"
			"} \n"
			"scene g_scene; \n"
			"void test() { \n"
			"  auto b = cast<CControlBase>(g_scene.Mobils[0]); \n"
			//"  auto a = g_scene.Mobils[0]; \n"
			//"  auto b = cast<CControlBase>(a); \n"
			"  assert( b !is null ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "test()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test both opCast and opImplCast
	// https://www.gamedev.net/forums/topic/696449-implicitly-assign-handle-by-overloading-opimplconv-operator/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)\n", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int op = 0; \n"
			"class A { B @opCast() { op = 1; return null; } \n"
			" B @opImplCast() { op = 2; return null; } \n"
			" const B @opCast() const { op = 3; return null; } \n"
			" const B @opImplCast() const { op = 4; return null; } } \n"
			"class B {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "A a; B @b = a; assert( op == 2 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "A a; B @b = cast<B>(a); assert( op == 1 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "const A a; const B @b = cast<B>(a); assert( op == 3 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "const A a; const B @b = a; assert( op == 4 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test both opConv and opImplConv
	// https://www.gamedev.net/forums/topic/696449-implicitly-assign-handle-by-overloading-opimplconv-operator/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)\n", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int op = 0; \n"
			"class A { B opConv() { op = 1; return B(); } \n"
			" B opImplConv() { op = 2; return B(); } \n"
			" B opConv() const { op = 3; return B(); } \n"
			" B opImplConv() const { op = 4; return B(); } } \n"
			"class B {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "A a; B b = a; assert( op == 2 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "A a; B b = B(a); assert( op == 1 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "const A a; B b = B(a); assert( op == 3 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "const A a; B b = a; assert( op == 4 );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test opCast(?&out) on null handle (should be allowed)
	// http://www.gamedev.net/topic/683804-void-opcastout-on-null-handle/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("test", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("test", "void opCast(?&out) const", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n");
		mod->Build();

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "test @t; A @a = cast<A>(t);", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test using opImplConv on ref type stored in dictionary
	// http://www.gamedev.net/topic/668972-getting-dictionary-addon-to-work-with-ref-counted-strings/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine,
			"dictionary dict; \n"
			"@dict['foo'] = 'bar';\n"
			"string t; \n"
			"dict.get('foo', t); \n"
			"assert( t == 'bar' ); \n"
			"dictionaryValue val = dict['foo']; \n"
			"t = cast<const string>(val); \n"
			"assert( t == 'bar' ); \n"
			"t = string(val); \n"
			"assert( t == 'bar' ); \n"
			"string s = string(dict['foo']);\n"
			"assert( s == 'bar' ); \n" 
			);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// The cast op can be overloaded in script classes by implementing opCast and opImplCast
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A { \n"
			"  B @opCast() { return b; } \n"
			"  B @b; \n"
			"} \n"
			"class B { \n"
			"  A @opImplCast() { return a; } \n"
			"  A @a; \n"
			"} \n" 
			"class C { \n"
			"  A @opCast() { return a; } \n"
			"  const A @opCast() const { return a; } \n"
			"  B @opCast() { return b; } \n"
			"  const B @opCast() const { return b; } \n"
			"  A @a; \n"
			"  B @b; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// cast<B>(a) should be translated to a.opCast() and return B@ 
		r = ExecuteString(engine, "A a; @a.b = B(); \n"
								  "B @b = cast<B>(a); \n"
								  "assert( b is a.b ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// @a = b should be translated to b.opImplCast() and return A@ 
		r = ExecuteString(engine, "B b; @b.a = A(); \n"
								  "A @a = b; \n"
								  "assert( a is b.a ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// cast should be able to properly handle const overloads too
		r = ExecuteString(engine, "C c; @c.a = A(); @c.b = B(); \n"
			                      "const C @cc = c; \n"
								  "A @a = cast<A>(c); \n"
								  "const A @ca = cast<A>(cc); \n"
								  "const B @cb = cast<B>(cc); \n"
								  "assert( a is ca && a !is null ); \n"
								  "assert( cb is c.b ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// http://www.gamedev.net/topic/636163-segfault-when-casting-directly/
	SKIP_ON_MAX_PORT
	{
		c = new ASConsole();

#if defined(_MSC_VER) && _MSC_VER <= 1200
		// MSVC6 was complaining about not finding this symbol because 
		// it didn't see the function being called anywhere
		ASRefCast<ASConsole, EventSource>(c);
		c->Release();
#endif

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		engine->RegisterObjectType("EventSource", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("EventSource", asBEHAVE_ADDREF, "void f()", asMETHOD(EventSource, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("EventSource", asBEHAVE_RELEASE, "void f()", asMETHOD(EventSource, Release), asCALL_THISCALL);
		engine->RegisterObjectProperty("EventSource", "int value", asOFFSET(EventSource, value));

		engine->RegisterObjectType("ASConsole", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("ASConsole", asBEHAVE_FACTORY, "ASConsole @f()", asFUNCTION(ASConsole::factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("ASConsole", asBEHAVE_ADDREF, "void f()", asMETHOD(ASConsole, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("ASConsole", asBEHAVE_RELEASE, "void f()", asMETHOD(ASConsole, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("ASConsole", "EventSource@ opImplCast()", asFUNCTION((ASRefCast<ASConsole, EventSource>)), asCALL_CDECL_OBJLAST);

		engine->RegisterGlobalFunction("void addListener(EventSource &inout, const int &in)", asFUNCTION(addListener), asCALL_CDECL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum E { ET_READLINE = 24 } \n"
			"ASConsole @console; \n"
			"void main() \n"
			"{ \n"
			"  addListener(cast<EventSource>(console), ET_READLINE); \n"
			"  EventSource @s = cast<EventSource>(console); \n"
			"  addListener(s, ET_READLINE); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		void** consoleVarAddr = (void**)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByName("console"));
        *consoleVarAddr = c;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( g_fail )
			TEST_FAILED;

		engine->Release();
	}

  	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	int res = 0;
	engine->RegisterGlobalProperty("int res", &res);

	ExecuteString(engine, "res = int(2342.4)");
	if( res != 2342 ) 
		TEST_FAILED;

	ExecuteString(engine, "double tmp = 3452.4; res = int(tmp)");
	if( res != 3452 ) 
		TEST_FAILED;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	mod->Build();

	r = ExecuteString(engine, "clss c; cast<intf1>(c); cast<intf2>(c);", mod);
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "intf1 @a = clss(); cast<clss>(a).Test2(); cast<intf2>(a).Test2();", mod);
	if( r < 0 )
		TEST_FAILED;

	// Test use of handle after invalid cast (should throw a script exception)
	r = ExecuteString(engine, "intf1 @a = clss(); cast<intf3>(a).Test3();", mod);
	if( r != asEXECUTION_EXCEPTION )
		TEST_FAILED;

	// Don't permit cast operator to remove constness
	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "const intf1 @a = clss(); cast<intf2>(a).Test2();", mod);
	if( r >= 0 )
		TEST_FAILED;

	if( bout.buffer != "ExecuteString (1, 41) : Error   : No matching signatures to 'intf2::Test2() const'\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// It should be allowed to cast null to an interface
	bout.buffer = "";
	r = ExecuteString(engine, "intf1 @a = cast<intf1>(null);", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	//--------------
	// Using constructor as implicit cast operator
	// TODO: Script classes should perhaps allow implicit casts to be implemented as well
/*	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	engine->AddScriptSection(0, "Test2", script2, strlen(script2));
	r = mod->Build(0);
	if( r < 0 )
		TEST_FAILED;
	r = ExecuteString(engine, "Test()");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
*/
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("Test3", script3, strlen(script3));
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;

	//-------------
	// "test" + string(type) + "\n"
	// "test" + type + "\n" 
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	r = engine->RegisterObjectType("type", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	RegisterScriptString(engine);
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY, "string@ f(const type &in)", asFUNCTION(TypeToString), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "type t; string a = \"a\" + string(t) + \"b\";"); 
	if( r < 0 )
		TEST_FAILED;
		
	// Since the constructor is not 'explicit' it can be used implicitly too
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = ExecuteString(engine, "type t; string a = \"a\" + t + \"b\";"); 
	if( r < 0 )
		TEST_FAILED;
	if (bout.buffer != "")
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Try using the asMETHOD macro with a cast operator
	// The first option fail to compile on MSVC2005 (Thanks Jeff Slutter)
//	engine->RegisterObjectMethod("obj", "void f()", asMETHOD(A, operator const char *), asCALL_THISCALL);
	engine->RegisterObjectMethod("obj", "void f()", asMETHODPR(A, operator const char *, () const, const char *), asCALL_THISCALL);

	engine->Release();

	//-------------------
	// Illegal cast statement
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";

	r = ExecuteString(engine, "uint8 a=0x80; int j=int();");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 24) : Error   : A cast operator has one argument\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// Success
 	return fail;
}

} // namespace

