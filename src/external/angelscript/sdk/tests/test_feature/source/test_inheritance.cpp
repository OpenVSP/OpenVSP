#include "utils.h"

using namespace std;

namespace TestInheritance
{

std::string printResult;
void print(const std::string &s)
{
	printResult += s;
}

bool Test2();

bool TestModule(const char *module, asIScriptEngine *engine);

class FooScripted
{
public:
	void CallMe()
	{
		if( !m_isDead->Get() )
		{
			asIScriptEngine *engine = m_obj->GetEngine();
			asIScriptContext *ctx = engine->RequestContext();
			ctx->Prepare(m_obj->GetObjectType()->GetMethodByDecl("void CallMe()"));
			ctx->SetObject(m_obj);
			ctx->Execute();
			engine->ReturnContext(ctx);
		}
	}
	int m_value;

	static FooScripted *Factory()
	{
		asIScriptContext *ctx = asGetActiveContext();

		// Get the function that is calling the factory so we can be certain it is the FooScript class
		asIScriptFunction *func = ctx->GetFunction(0);
		if( func->GetObjectType() == 0 || std::string(func->GetObjectType()->GetName()) != "FooScripted" )
		{
			ctx->SetException("Invalid attempt to manually instantiate FooScript_t");
			return 0;
		}

		// Get the this pointer from the calling function
		asIScriptObject *obj = reinterpret_cast<asIScriptObject*>(ctx->GetThisPointer(0));

		return new FooScripted(obj);
	}

	void AddRef()
	{
		if( !m_isDead->Get() )
			m_obj->AddRef();
		m_refCount++;
	}

	void Release()
	{
		if( !m_isDead->Get() )
			m_obj->Release();
		if( --m_refCount == 0 )
			delete this;
	}

	static bool Test(FooScripted *obj)
	{
		if( obj == 0 ) return false;

		bool isValid = true;
		if( obj->m_obj == 0 ) 
			isValid = false;
		else if( string(obj->m_obj->GetObjectType()->GetName()) != "FooDerived" )
			isValid = false;

		obj->Release();

		return isValid;
	}
	
	FooScripted &operator=(const FooScripted &o)
	{
		// Copy only the content, not the script proxy class
		m_value = o.m_value;
		return *this;
	}

protected:
	FooScripted(asIScriptObject *obj) 
	{ 
		m_obj = 0;
		m_isDead = 0;
		m_value = 0;
		m_refCount = 1;

		m_isDead = obj->GetWeakRefFlag();
		m_isDead->AddRef();

		m_obj = obj;
	}
	~FooScripted()
	{
		m_isDead->Release();
	}

	int m_refCount;
	asILockableSharedBool *m_isDead;
	asIScriptObject *m_obj;
};

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

	asIScriptModule *mod = 0;
	COutStream out;
	CBufferedOutStream bout;
 	asIScriptEngine *engine = 0;

	// Script class inheriting from an application class through proxy and copy object
	// https://www.gamedev.net/forums/topic/704232-add-ref-proxy-class-causes-crash-in-asiscriptobjectoperator-due-to-early-destruction/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("FooScripted_t", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_FACTORY, "FooScripted_t @f()", asFUNCTION(FooScripted::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_ADDREF, "void f()", asMETHOD(FooScripted, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_RELEASE, "void f()", asMETHOD(FooScripted, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("FooScripted_t", "FooScripted_t &opAssign(const FooScripted_t &in)", asMETHOD(FooScripted, operator=), asCALL_THISCALL);
		engine->RegisterObjectMethod("FooScripted_t", "void CallMe()", asMETHOD(FooScripted, CallMe), asCALL_THISCALL);
		engine->RegisterObjectProperty("FooScripted_t", "int m_value", asOFFSET(FooScripted, m_value));

		mod = engine->GetModule("Foo", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Foo",
			"shared abstract class FooScripted { \n"
			"  FooScripted() { \n"
			"    @m_obj = FooScripted_t(); \n"
			"  } \n"
			"  FooScripted(const FooScripted &o) { \n"
			"    @m_obj = FooScripted_t(); \n" // Create a new C++ instance
			"    m_obj = o.m_obj; \n"          // copy content of C++ instance
			"  } \n"
			"  FooScripted &opAssign(const FooScripted &o) { \n"
			"    m_obj = o.m_obj; \n"  // copy content of C++ instance
			"    return this; \n"
			"  } \n"
			"  void CallMe() { m_obj.CallMe(); } \n"
			"  int m_value { \n"
			"    get { return m_obj.m_value; } \n"
			"    set { m_obj.m_value = value; } \n"
			"  } \n"
			"  FooScripted_t @opImplCast() { \n"
			"    return m_obj; \n"
			"  } \n"
			"  private FooScripted_t @m_obj; \n"
			"} \n");

		mod->AddScriptSection("Foo2",
			"class FooDerived : FooScripted { \n"
			"  void CallMe() { \n"
			"    m_value += 1; \n"
			"  } \n"
			"} \n"
			"void Test(FooDerived &in bar) { assert( bar.m_value == 1 ); } \n"); // without const to guarantee a copy is made

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "FooDerived f; \n"
								  "assert( f.m_value == 0 ); \n"
								  "f.CallMe(); \n"
								  "Test(f); \n" // makes a copy of f
								  "f.CallMe(); \n"
								  "assert( f.m_value == 2 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}
	
	
	// Test default value assignment operator for derived classes
	// Reported by Aaron Baker
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class foo \n"
			"{ \n"
			"	int type; \n"
			"	foo() \n"
			"	{ \n"
			"		type=0; \n"
			"	} \n"
			"} \n"
			"class bar:foo \n"
			"{ \n"
			"	bar() \n"
			"	{ \n"
			"		type=1; \n"
			"	} \n"
			"} \n"
			"foo@ make_foo(int type) \n"
			"{ \n"
			"	if(type==0) \n"
			"		return foo(); \n"
			"	else \n"
			"		return bar(); \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"	foo@ a = make_foo(1); \n"
			"	foo@ copy = make_foo(a.type); \n"
			"	copy = a; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}		
		ctx->Release();

		engine->ShutDownAndRelease();
	}
	
	// Test that the calling the parent's constructor through super works even when there is a get_super() property accessor
	// Reported by Patrick Jeeves
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int super { get { return 0; } } \n"
			"class Asuper { } \n"
			"class B : Asuper { \n"
			"  B() { super(); } \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Script class inheriting from an application class through proxy
	// http://www.gamedev.net/topic/658925-casting-and-inheritance/
	// http://www.gamedev.net/topic/535837-application-registered-classes/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("FooScripted_t", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_FACTORY, "FooScripted_t @f()", asFUNCTION(FooScripted::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_ADDREF, "void f()", asMETHOD(FooScripted, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_RELEASE, "void f()", asMETHOD(FooScripted, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("FooScripted_t", "void CallMe()", asMETHOD(FooScripted, CallMe), asCALL_THISCALL);
		engine->RegisterObjectProperty("FooScripted_t", "int m_value", asOFFSET(FooScripted, m_value));

		engine->RegisterGlobalFunction("bool Test(FooScripted_t @)", asFUNCTION(FooScripted::Test), asCALL_CDECL);

		mod = engine->GetModule("Foo", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Foo",
			"shared abstract class FooScripted { \n"
			"  FooScripted() { \n"
			"    @m_obj = FooScripted_t(); \n"
			"  } \n"
			"  void CallMe() { m_obj.CallMe(); } \n"
			"  int m_value { \n"
			"    get { return m_obj.m_value; } \n"
			"    set { m_obj.m_value = value; } \n"
			"  } \n"
			"  FooScripted_t @opImplCast() { \n"
			"    return m_obj; \n"
			"  } \n"
			"  private FooScripted_t @m_obj; \n"
			"} \n");

		mod->AddScriptSection("Foo2",
			"class FooDerived : FooScripted { \n"
			"  void CallMe() { \n"
			"    m_value += 1; \n"
			"  } \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "FooDerived f; \n"
								  "assert( f.m_value == 0 ); \n"
								  "f.CallMe(); \n"
								  "assert( f.m_value == 1 ); \n"
								  // The FooDerived should be implicitly cast to FooScripted_t
								  "assert( Test(f) ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		asIScriptObject *obj = reinterpret_cast<asIScriptObject*>(engine->CreateScriptObject(mod->GetTypeInfoByName("FooDerived")));
		FooScripted *obj2 = *reinterpret_cast<FooScripted**>(obj->GetAddressOfProperty(0));
		obj2->AddRef();
		obj->Release();
		engine->GarbageCollect();

		if( obj2->m_value != 0 )
			TEST_FAILED;
		obj2->CallMe();
		if( obj2->m_value != 1 )
			TEST_FAILED;

		obj2->Release();

		engine->Release();
	}

	// Value assignment on the base class where the operands are two different derived classes
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Base {} \n"
			"class Derived1 : Base { string a; } \n"
			"class Derived2 : Base { double a; } \n"
			"void main() \n"
			"{ \n"
			"  Derived1 d1; \n"
			"  Derived2 d2; \n"
			"  Base@ b1 = d1, b2 = d2; \n"
			"  b1 = b2; \n" // must not crash application. should raise script exception
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		if( std::string(ctx->GetExceptionString()) != "Mismatching types in value assignment" )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// A derived class must not be allowed to implement a function with the same 
	// name and parameter list as parent class, but with a different return type.
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class P { \n"
			"  int MyFunc(float) { return 0; } \n"
			"} \n"
			"class D : P { \n"
			"  float MyFunc(float) { return 0; } \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (4, 7) : Error   : The method in the derived class must have the same return type as in the base class: 'int P::MyFunc(float)'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Basic tests for inheritance
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);

		const char *script =
			"bool baseDestructorCalled = false;               \n"
			"bool baseConstructorCalled = false;              \n"
			"bool baseFloatConstructorCalled = false;         \n"
			"class Base : Intf                                \n"
			"{                                                \n"
			"  int a;                                         \n"
			"  void f1() { a = 1; }                           \n"
			"  void f2() { a = 0; }                           \n"
			"  void f3() { a = 3; }                           \n"
			"  Base() { baseConstructorCalled = true; }       \n"
			"  Base(float) { baseFloatConstructorCalled = true; } \n"
			"  ~Base() { baseDestructorCalled = true; }       \n"
			"}                                                \n"
			"bool derivedDestructorCalled = false;            \n"
			"bool derivedConstructorCalled = false;           \n"
			"class Derived : Base                             \n"
			"{                                                \n"
			   // overload f2()
			"  void f2() { a = 2; }                           \n"
			   // overload f3()
			"  void f3() { a = 2; }                           \n"
			"  void func()                                    \n"
			"  {                                              \n"
				 // call Base::f1()
			"    f1();                                        \n"
			"    assert(a == 1);                              \n"
				 // call Derived::f2()
			"    f2();                                           \n"
			"    assert(a == 2);                                 \n"
				 // call Base::f3() 
			"    Base::f3();                                     \n"
			"    assert(a == 3);                                 \n"
			"  }                                                 \n"
			"  Derived() {} \n"
			"  Derived(int) { derivedConstructorCalled = true; } \n"
			"  ~Derived() { derivedDestructorCalled = true; }    \n"
			"}                                                \n"
			"void foo( Base &in a )                           \n"
			"{                                                \n"
			"  assert( cast<Derived>(a) is null );            \n"
			"}                                                \n"
			// Must be possible to call the default constructor, even if not declared
			"class DerivedGC : BaseGC { DerivedGC() { super(); } }  \n"
			"class BaseGC { BaseGC @b; }                      \n"
			"class DerivedS : Base                            \n"
			"{                                                \n"
			"  DerivedS(float)                                \n"
			"  {                                              \n"
	  			 // Call Base::Base(float)
			"    if( true )                                   \n"
			"      super(1.4f);                               \n"
			"    else                                         \n"
			"      super();                                   \n"
			"  }                                              \n"
			"}                                                \n"
			// Must handle inheritance where the classes have been declared out of order
			"void func()                                      \n"
			"{                                                \n"
			"   Intf@ a = C();                                \n"
			"}                                                \n"
			"class C : B {}                                   \n"
			"interface Intf {}                                \n"
			"class B : Intf {}                                \n"
			// Several levels of inheritance
			"class C0                                         \n"
			"{                                                \n"
			"  void Dummy() {}                                \n"
			"}                                                \n"
			"class C1 : C0                                    \n"
			"{                                                \n"
			"  void Fun() { print('C1:Fun'); }                \n"
			"}                                                \n"
			"class C2 : C1                                    \n"
			"{                                                \n"
			"  void Fun() { print('C2:Fun'); }                \n"
			"}                                                \n"
			"class C3 : C2                                    \n"
			"{                                                \n"
			"  void Call() { Fun(); }                         \n"
			"}                                                \n";


		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}

		// Make sure the bytecode for the ref cast is correct
		asIScriptFunction *func = mod->GetFunctionByName("foo");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_PSF,asBC_Cast,asBC_STOREOBJ,asBC_ClrVPtr,asBC_CmpPtr,asBC_TZ,asBC_CpyRtoV4,asBC_FREE,asBC_FREE,asBC_PshV4,asBC_CALLSYS,
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		if( TestModule(0, engine) )
		{
			TEST_FAILED;
		}

		// Must make sure that the inheritance path is stored/restored with the saved byte code
		{ 
			CBytecodeStream stream(__FILE__"1");
			r = mod->SaveByteCode(&stream);
			if( r < 0 )
			{
				TEST_FAILED;
			}

			asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
			r = mod2->LoadByteCode(&stream);
			if( r < 0 )
			{
				TEST_FAILED;
			}

			// Both modules should have the same number of functions
			if( mod->GetFunctionCount() != mod2->GetFunctionCount() )
			{
				TEST_FAILED;

				asUINT n;
				PRINTF("First module's functions\n");
				for( n = 0; n < (asUINT)mod->GetFunctionCount(); n++ )
				{
					asIScriptFunction *f = mod->GetFunctionByIndex(n);
					PRINTF("%s\n", f->GetDeclaration());
				}
				PRINTF("\nSecond module's functions\n");
				for( n = 0; n < (asUINT)mod2->GetFunctionCount(); n++ )
				{
					asIScriptFunction *f = mod2->GetFunctionByIndex(n);
					PRINTF("%s\n", f->GetDeclaration());
				}
			}

			if( TestModule("2", engine) )
			{
				TEST_FAILED;
			}

			engine->DiscardModule("2");
		}

		engine->Release();
	}

	// Test final and override
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test", 
			"final class CFin1 {} \n" // Don't allow inheritance
			"shared final class CFin2 {} \n" // -"-
			"class CBase \n"
			"{ \n"
			"  void finalFunc() final {} \n" // don't allow override this func
			"  void overrideFunc() {} \n" 
			"} \n"
			"class CD1 : CFin1 {} \n" // Shouldn't work
			"class CD2 : CBase \n"
			"{ \n"
			"  void finalFunc() {} \n" // shouldn't work
			"  void overrideFunc(int) override {} \n" // must override
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (8, 13) : Error   : Can't inherit from class 'CFin1' marked as final\n"
					/*	   "test (9, 7) : Error   : Method 'void CBase::finalFunc()' declared as final and cannot be overridden\n"
						   "test (9, 7) : Error   : Method 'void CD2::overrideFunc(int)' marked as override but does not replace any base class or interface method\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	fail = Test2() || fail;

	// Success
	return fail;
}

bool TestModule(const char *module, asIScriptEngine *engine)
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;
	COutStream out;
	asIScriptModule *mod = engine->GetModule(module);

	// Test that it is possible to declare a class that inherits from another
	// Test that the inherited properties are available in the derived class
	// Test that the inherited methods are available in the derived class
	// Test that it is possible to override the inherited methods
	// Test that it is possible to call base class methods from within overridden methods in derived class 
	asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(mod->GetTypeInfoByName("Derived"));
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(obj->GetObjectType()->GetMethodByDecl("void func()"));
	ctx->SetObject(obj);
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}
	ctx->Release();
	obj->Release();

	// Test that implicit cast from derived to base is working
	r = ExecuteString(engine, "Derived d; Base @b = @d; assert( b !is null );", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that cast from base to derived require explicit cast
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = ExecuteString(engine, "Base b; Derived @d = @b;", mod);
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	if( bout.buffer != "ExecuteString (1, 22) : Error   : Can't implicitly convert from 'Base@&' to 'Derived@&'.\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is possible to explicitly cast to derived class
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	r = ExecuteString(engine, "Derived d; Base @b = @d; assert( cast<Derived>(b) !is null );", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test the explicit cast behaviour for a non-handle script object
	r = ExecuteString(engine, "Base b; assert( cast<Derived>(b) is null );", mod);
	if( r != asEXECUTION_FINISHED )
	{
		fail= true;
	}

	// Test that it is possible to implicitly assign derived class to base class
	r = ExecuteString(engine, "Derived d; Base b = d;", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that it is possible to pass a derived class to a function expecting a reference to the base class
	// This actually creates an instance of the Base class and assigns the Derived instance to it.
	// This is because the parameter is &in and not const &in
	// TODO: May be able to avoid this by having a specific behaviour for 
	//       duplicating objects, rather than using assignment
	r = ExecuteString(engine, "Derived d; foo(d);", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test polymorphing
	r = ExecuteString(engine, "Derived d; Base @b = @d; b.a = 3; b.f2(); assert( b.a == 2 );", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Base class' destructor must be called when object is destroyed
	r = ExecuteString(engine, "baseDestructorCalled = derivedDestructorCalled = false; { Derived d; }\n"
								      "assert( derivedDestructorCalled ); \n"
		                              "assert( baseDestructorCalled );\n", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// If the base class is garbage collected, then the derived class must also be garbage collected
	// This also tests that it is possible to call the default constructor of the base class, even though it is not declared
	engine->GarbageCollect();
	r = ExecuteString(engine, "DerivedGC b; @b.b = @b;", mod);
	if( r != asEXECUTION_FINISHED ) 
	{
		TEST_FAILED;
	}

	asUINT gcSize;
	engine->GetGCStatistics(&gcSize);
	assert( gcSize == 1 );
	engine->GarbageCollect();
	engine->GetGCStatistics(&gcSize);
	assert( gcSize == 0 );

	// Test that the derived class inherits the interfaces that the base class implements
	r = ExecuteString(engine, "Intf @a; Derived b; @a = @b;", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that an implemented constructor calls the base class' default constructor
	r = ExecuteString(engine, "baseConstructorCalled = derivedConstructorCalled = false; Derived d(1); \n"
		                              "assert( baseConstructorCalled ); \n"
									  "assert( derivedConstructorCalled ); \n", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that the default constructor calls the base class' default constructor
	r = ExecuteString(engine, "baseConstructorCalled = false; Derived d; \n"
		                              "assert( baseConstructorCalled ); \n", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that it is possible to manually call the base class' constructor
	// Test that the default constructor for the base class isn't called 
	//   when a manual call to another constructor is made
	r = ExecuteString(engine, "baseConstructorCalled = baseFloatConstructorCalled = false; DerivedS d(1.4f); \n"
		                              "assert( baseFloatConstructorCalled ); \n"
									  "assert( !baseConstructorCalled ); \n", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test that it is possible to determine base class from object type interface
	asITypeInfo *d = engine->GetTypeInfoById(mod->GetTypeIdByDecl("Derived"));
	if( d == 0 )
	{
		TEST_FAILED;
	}
	else if( d->GetBaseType() == 0 )
	{
		TEST_FAILED;
	}
	else if( strcmp(d->GetBaseType()->GetName(), "Base") != 0 )
	{
		TEST_FAILED;
	}

	// Test factory id
	if( d->GetFactoryCount() != 2 )
		TEST_FAILED;
	asIScriptFunction *fid = d->GetFactoryByDecl("Derived@ Derived(int)");
	if( fid < 0 )
		TEST_FAILED;
	if( fid != d->GetFactoryByIndex(1) )
		TEST_FAILED;

	// Test various levels of inheritance
	printResult = "";
	r = ExecuteString(engine, "C3 c; c.Call();", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}
	else if( printResult != "C2:Fun" )
	{
		PRINTF("%s\n", printResult.c_str());
		TEST_FAILED;
	}

	// TODO: not related to inheritance, but it should be possible to call another constructor from within a constructor. 
	//       We can follow D's design of using this(args) to call the constructor

	return fail;
}

bool Test2()
{
	bool fail = false;
	CBufferedOutStream bout;
	int r;
	asIScriptModule *mod;
	asIScriptEngine *engine;
	const char *script;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	RegisterScriptString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	// Test that it is not possible to inherit from application registered type
	script = "class A : string {} \n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 11) : Error   : Can't inherit from class 'string' marked as final\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Test that it is not possible to inherit from multiple script classes
	script = "class B {} class C {} class D {} class A : B, C, D {} \n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 47) : Error   : Can't inherit from multiple classes\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to inherit from a class that in turn inherits from this class
	script = "class A : C {} class B : A {} class C : B {}\n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 41) : Error   : Can't inherit from itself, or another class that inherits from this class\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to inherit from self
	script = "class A : A {}\n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 11) : Error   : Can't inherit from itself, or another class that inherits from this class\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that derived classes can't overload properties
	// TODO: In C++ it is possible to overload properties, in which case the base class property is hidden. Should we adopt this for AngelScript too?
	script = "class A { int a; } class B : A { double a; }\n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	// TODO: The error should explain that the original property is from the base class
	if( bout.buffer != "script (1, 41) : Error   : Name conflict. 'a' is an object property.\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super() when not deriving from any class
	script = "class A { A() { super(); } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	// TODO: The error message should explain that it is not possible to call super 
	//       because the class doesn't derived from another class
	if( bout.buffer != "script (1, 11) : Info    : Compiling A::A()\n"
					   "script (1, 17) : Error   : No matching symbol 'super'\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super() multiple times within the constructor
	script = "class A {} class B : A { B() { super(); super(); } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 ) 
		TEST_FAILED;
	if( bout.buffer != "script (1, 26) : Info    : Compiling B::B()\n"
					   "script (1, 41) : Error   : Can't call a constructor multiple times\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super() in a loop
	script = "class A {} class B : A { B() { while(true) { super(); } } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build(); 
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 26) : Info    : Compiling B::B()\n"
					   "script (1, 46) : Error   : Can't call a constructor in loops\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super() in a switch
	// TODO: Should allow call in switch, but all possibilities must call it once.
	script = "class A {} class B : A { B() { switch(2) { case 2: super(); } } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build(); 
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 26) : Info    : Compiling B::B()\n"
					   "script (1, 52) : Error   : Can't call a constructor in switch\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that all (or none) control paths must call super()
	script = "class A {} class B : A { \n"
		     "B(int) { if( true ) super(); } \n"
			 "B(float) { if( true ) {} else super(); } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build(); 
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (2, 1) : Info    : Compiling B::B(int)\n"
					   "script (2, 10) : Error   : Both conditions must call constructor\n"
					   "script (3, 1) : Info    : Compiling B::B(float)\n"
				   	   "script (3, 12) : Error   : Both conditions must call constructor\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super() outside of the constructor
	script = "class A {} class B : A { void mthd() { super(); } }";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build(); 
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 26) : Info    : Compiling void B::mthd()\n"
					   "script (1, 40) : Error   : No matching symbol 'super'\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that a base class can't have a derived class as member (except as handle)
	script = "class A { B b; } class B : A {}";
	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	// TODO: The message could be improved to mention which member
	if( bout.buffer != "script (1, 24) : Error   : Illegal member type\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that it is not possible to call super with any scope prefix
	script = "class A { } class B : A { B() { ::super(); } }";
	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 27) : Info    : Compiling B::B()\n"
					   "script (1, 33) : Error   : No matching symbol '::super'\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that the error message for calling missing method when scope is correct
	script = "class A { void method() { B::test(); A::method(2); A::method(); method(3.15); B::A::a(); } }";
	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 11) : Info    : Compiling void A::method()\n"
					   "script (1, 27) : Error   : No matching symbol 'B::test'\n"
					   "script (1, 38) : Error   : No matching signatures to 'A::method(const int)'\n"
					   "script (1, 38) : Info    : Candidates are:\n"
					   "script (1, 38) : Info    : void A::method()\n"
					   "script (1, 65) : Error   : No matching signatures to 'A::method(const double)'\n"
					   "script (1, 65) : Info    : Candidates are:\n"
					   "script (1, 65) : Info    : void A::method()\n"
					   "script (1, 79) : Error   : No matching symbol 'B::A::a'\n" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	// Test that calling the constructor from within the constructor 
	// using the class name will create a new object. 
	script = "A @a1, a2; class A { A() { @a1 = this; A(1); } A(int) { @a2 = this; } }";
	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}
	r = ExecuteString(engine, "A a; assert( a1 !is a2 ); assert( a1 !is null ); assert( a2 !is null );", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}


	engine->Release();

	return fail;
}

} // namespace

