#include <sstream>

#include "utils.h"
#include "../../../add_on/autowrapper/aswrappedcall.h"

namespace TestImplicitCast
{

void Type_construct0(asIScriptGeneric *gen)
{
	int *a = (int*)gen->GetObject();
	*a = 0;
}

void Type_construct1(asIScriptGeneric *gen)
{
	int *a = (int*)gen->GetObject();
	*a = *(int*)gen->GetAddressOfArg(0);;
}

void Type_castInt(asIScriptGeneric *gen)
{
	int *a = (int*)gen->GetObject();
	*(int*)gen->GetAddressOfReturnLocation() = *a;
}

void Type_castVar(void *ptr, int typeId, int *obj)
{
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();

	asITypeInfo *type = engine->GetTypeInfoById(typeId);
	if( std::string(type->GetName()) == "string" )
	{
		std::stringstream strm;
		strm << *obj;
		*(std::string*)ptr = strm.str();
	}
}

bool Type_equal(int &a, int &b)
{
	return a == b;
}

// Class A is the base class
class A
{
public:
	virtual int test() 
	{
		return 1;
	}

	static A* factory() 
	{
		return new A;
	}
	virtual void addref() 
	{
		refCount++;
	}
	virtual void release() 
	{
		refCount--; 
		if( refCount == 0 ) 
			delete this;
	}
	virtual A& assign(const A &)
	{
		return *this;
	}
protected:
	A() {refCount = 1;}
	virtual ~A() {}
	int refCount;
};

// Class B is the sub class, derived from A
class B : public A
{
public:
	virtual int test() 
	{
		return 2;
	}

	static B* factory() 
	{
		return new B;
	}
	static A* castToA(B*b) 
	{
		A *a = dynamic_cast<A*>(b);
		return a;
	}
	static B* AcastToB(A*a) 
	{
		B *b = dynamic_cast<B*>(a);
		return b;
	}
protected:
	B() : A() {}
	~B() {}
};

template<class T>
class Param {
public:
  Param(const T _value) : value_(_value), ref_count_(1) { };

  T v() { return value_; } const

  int add_ref() { return ++ref_count_; }
  int release() { if( --ref_count_ == 0 ) { delete this; return 0; } return ref_count_;}

private:
  T value_;
  int ref_count_;
};

static Param<double> * Param_double_Factory(double v) {
  return new Param<double>(v);
}

double myFunction(const double d) {
	return d;
}

struct Value
{
	double castToDouble() { return dbl; }
	int castToInt() { return int(dbl); }
	double dbl;
};

static bool Test2();
static bool Test3();
static bool Test4();
static bool Test5();

bool Test()
{
	RET_ON_MAX_PORT


	bool fail = false;
	int r;
	asIScriptEngine *engine;

	CBufferedOutStream bout;
	COutStream out;

	// Test implicit cast in arg using the constructor
	// TODO: Add this support (only for value types for now)
	// http://www.gamedev.net/topic/660695-passing-strings-inline/
/*	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);

		r = engine->RegisterObjectType("EventArg", 4, asOBJ_VALUE); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("EventArg", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("EventArg", asBEHAVE_CONSTRUCT, "void f(const float)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("EventArg", asBEHAVE_CONSTRUCT, "void f(const EventArg &in)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("EventArg", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("EventArg", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterGlobalFunction("void sendEvent(const uint, EventArg = EventArg(), EventArg = EventArg(), EventArg = EventArg(), EventArg = EventArg())", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"  sendEvent(123, 'string', 1.234f); \n"
			"  string str = 'string'; \n"
			"  sendEvent(123, str, 1.234f); \n"
			"  EventArg a = 'string'; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	} */

	// Test implicit conv/cast when function is expecting &in  (ref arg, but as input only, thus allow object to be transformed)
	// problem reported by Thomas Grip
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class cIDSet {} \n"
			"class cID : cIDSet {} \n"
			"class cID2 { \n"
			"  cIDSet opImplConv() const { return cIDSet(); } \n"
			"} \n"
			"void func(const cIDSet &in ) {} \n"
			"void main() { \n"
			"  cID id; \n"
			"  func(id); \n"  // should be allowed
			"  cID2 id2; \n"
			"  func(id2); \n" // should be allowed
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test bug fix for implicit conv and math operations
	// http://www.gamedev.net/topic/661765-implicit-convert-math-bug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = 
			"int x = 1;\n"
			"class A\n"
			"{\n"
			"	int val;\n"
			"	A(int x)\n"
			"	{\n"
			"		val = x;\n"
			"	}\n"
			"	int opImplConv()\n"
			"	{\n"
			"		return val;\n"
			"	}\n"
			"}\n"
			"A myA(5);\n"
			"void main()\n"
			"{\n"
			"	assert(myA + (x + 1) == myA + x + 1);\n"
			"}\n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test global var with implicit cast
	// http://www.gamedev.net/topic/659415-implicit-downcast-not-works/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A \n"
			"{ \n"
			"} \n"
			"class B : A \n"
			"{ \n"
			"} \n"
			"A@ a = B(); \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem reported by Amer Koleci
	// It was causing assert failure as the compiler tried to release the same temporary variable twice
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("String", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
		engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT, "void f(uint)", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("Texture2D", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("Texture2D", "uint get_width() const property", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterGlobalFunction("void Print(const String &in)", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			" Texture2D@ texture; \n"
			" Print(texture.width); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	fail = Test2() || fail;
	fail = Test3() || fail;
	fail = Test4() || fail;
	fail = Test5() || fail;

	// Two forms of casts: value cast and ref cast
	// A value cast actually constructs a new object
	// A ref cast will only reinterpret a handle, without actually constructing any object

	// Should be possible to tell AngelScript if it may use the behaviour implicitly or not
	// Since care must be taken with implicit casts, it is not allowed by default,
	// i.e. opConv and opImplConv or
	//      asBEHAVE_REF_CAST and asBEHAVE_REF_CAST_IMPLICIT

	//----------------------------------------------------------------------------
	// VALUE_CAST

	// TODO: (Test) Cast from primitive to object is an object constructor/factory
	// TODO: (Test) Cast from object to object can be either object behaviour or object constructor/factory, 
	// depending on which object registers the cast

	// TODO: (Implement) It shall be possible to register cast operators as explicit casts. The constructor/factory 
	// is by default an explicit cast, but shall be possible to register as implicit cast.

	// TODO: (Implement) Type constructors should be made explicit cast only, or perhaps not permit casts at all

	// TODO: (Test) When compiling operators with non-primitives, the compiler should first look for 
	// compatible registered operator behaviours. If not found, the compiler should see if 
	// there is any cast behaviour that allow conversion of the type to a primitive type.

	// Test 1
	// A class can be implicitly cast to a primitive, if registered the VALUE_CAST behaviour
 	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	r = engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectType("type", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("type", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Type_construct0), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("type", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(Type_construct1), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("type", "int opImplConv()", asFUNCTION(Type_castInt), asCALL_GENERIC); assert( r >= 0 );

	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "type t(5); \n"
		                         "int a = t; \n"             // conversion to primitive in assignment
								 "assert( a == 5 ); \n"
								 "assert( a + t == 10 ); \n" // conversion to primitive with math operation
								 "a -= t; \n"                // conversion to primitive with math operation
								 "assert( a == 0 ); \n"
								 "assert( t == int(5) ); \n" // conversion to primitive with comparison 
								 "type b(t); \n"             // conversion to primitive with parameter
								 "assert( 32 == (1 << t) ); \n"   // conversion to primitive with bitwise operation 
	                             "assert( (int(5) & t) == 5 ); \n" // conversion to primitive with bitwise operation
								 , 0, ctx);
	if( r != 0 )
	{
		if( r == 3 )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	// Test 2
	// A class won't be converted to primitive if there is no obvious target type
	// ex: t << 1 - It is not known what type t should be converted to
	// ex: t + t - It is not known what type t should be converted to
	// ex: t < t - It is not known what type t should be converted to
	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "type t(5); t << 1; ");
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 14) : Error   : Illegal operation on 'type'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	bout.buffer = "";
	r = ExecuteString(engine, "type t(5); t + t; ");
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 14) : Error   : No matching operator that takes the types 'type' and 'type' found\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	bout.buffer = "";
	r = ExecuteString(engine, "type t(5); t < t; ");
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 14) : Error   : No matching operator that takes the types 'type' and 'type' found\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Test3
	// If an object has a cast to more than one matching primitive type, the cast to the 
	// closest matching type will be used, i.e. Obj has cast to int and to float. A type of 
	// int8 is requested, so the cast to int is used
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = ExecuteString(engine, "type t(2); assert( (1.0 / t) == (1.0 / 2.0) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	engine->Release();

	// Test4
	// It shall not be possible to register a cast behaviour from an object to a boolean type
/* 
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = engine->RegisterObjectType("type", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectMethod("type", "bool opImplConv()", asFUNCTION(Type_castInt), asCALL_GENERIC); 
	if( r != asNOT_SUPPORTED )
	{
		TEST_FAILED;
	}
	if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'type' and 'bool f()' (Code: -7)\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();
*/
	// Test5
	// Exclicit value cast
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		r = engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectType("type", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("type", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Type_construct0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("type", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(Type_construct1), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("type", "int opConv()", asFUNCTION(Type_castInt), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("type", "bool opEquals(const type &in) const", asFUNCTION(Type_equal), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
		r = engine->RegisterObjectProperty("type", "int v", 0);

		// explicit cast to int is allowed
		r = ExecuteString(engine, "type t; t.v = 5; int a = int(t); assert(a == 5);"); 
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// as cast to int is allowed, AngelScript also allows cast to float (using cast to int then implicit cast to int)
		r = ExecuteString(engine, "type t; t.v = 5; float a = float(t); assert(a == 5.0f);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// implicit cast to int is not allowed
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "type t; int a = t;");
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 17) : Error   : Can't implicitly convert from 'type' to 'int'.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Having an implicit constructor with an int param makes it possible to compare the type with int
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "type t(5); assert( t == 5 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Implicit cast to value type works for function arguments too
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool funcCalled = false; \n"
			"void func(const type &in a) \n"
			"{ \n"
			"  assert( a == 5 ); \n"
			"  funcCalled = true; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "func(5); assert( funcCalled );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test generic value cast
		RegisterStdString(engine);
		r = engine->RegisterObjectMethod("type", "void opConv(?&out)", asFUNCTION(Type_castVar), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = ExecuteString(engine, "type t; t.v = 5; string s = string(t); assert( s == '5' );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// The string(t), that actually calls t.opConv(string) should only create 1 temporary string value
		mod->AddScriptSection("test", "void func() { type t; string(t); }");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByDecl("void func()");
		asBYTE expect[] = 
			{
				asBC_SUSPEND, asBC_PSF, asBC_CALLSYS, 
				// TODO: optimize: The string object should be constructed before asBC_TYPEID, thus allow asBC_VAR and asBC_GETREF to be optimized to asBC_PSF
				asBC_SUSPEND, asBC_TYPEID, asBC_VAR, asBC_PSF, asBC_PSF, asBC_CALLSYS, asBC_GETREF, asBC_CALLSYS, asBC_PSF, asBC_CALLSYS, 
				asBC_SUSPEND, asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		engine->Release();
	}

	// Test6
	// Must be possible to return a const string variable from a function returning a non-const string
	// string registered as reference type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptString(engine);

		const char *script = "string test() { const string s = 'hello'; return s; }";
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test7
	// Must be possible to return a const string variable from a function returning a non-const string
	// string registered as value type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);

		const char *script = "string test() { const string s = 'hello'; return s; }";
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = 
			"class File { \n"
			"  int64 readInt(uint a) { int64 v = -1; return v*512; } \n"
			"} \n"
			"const int origVal = -512; \n"
			"void main() \n"
			"{ \n"
			"  File f; \n"
			"  assert( f.readInt(4) == origVal ); \n"
			"  assert( int(f.readInt(4)) == origVal ); \n"
			"  const int localVal = -512; \n"
			"  assert( f.readInt(4) == localVal ); \n"
			"  assert( int(f.readInt(4)) == localVal ); \n"
			"} \n";
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//-----------------------------------------------------------------
	// REFERENCE_CAST

	// Test comparison of objects at different hierarchy levels
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class P {} \n"
			"class C : P {} \n"
			"void main() { \n"
			"  P @p; \n"
			"  C @c; \n"
			"  if( p is c ) {} \n"
			"  if( c is p ) {} \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	SKIP_ON_MAX_PORT
	{

		// It must be possible to cast an object handle to another object handle, without 
		// losing the reference to the original object. This is what will allow applications
		// to register inheritance for registered types. This should be a special 
		// behaviour, i.e. REF_CAST. 
		
		// How to provide a cast from a base class to a derived class?
		// The base class may not know about the derived class, so it must
		// be the derived class that registers the behaviour. 
		
		// How to provide interface functionalities to registered types? I.e. a class implements 
		// various interfaces, and a handle to one of the interfaces may be converted to a handle
		// of another interface that is implemented by the class.

		// TODO: Can't register casts from primitive to primitive

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		// Class A is the base class
		engine->RegisterObjectType("A", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("A", asBEHAVE_FACTORY, "A@f()", asFUNCTION(A::factory), asCALL_CDECL); 
		engine->RegisterObjectBehaviour("A", asBEHAVE_RELEASE, "void f()", asMETHOD(A, release), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("A", asBEHAVE_ADDREF, "void f()", asMETHOD(A, addref), asCALL_THISCALL);
		engine->RegisterObjectMethod("A", "A& opAssign(const A &in)", asMETHOD(A, assign), asCALL_THISCALL);
		engine->RegisterObjectMethod("A", "int test()", asMETHOD(A, test), asCALL_THISCALL);

		// Class B inherits from class A
		engine->RegisterObjectType("B", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("B", asBEHAVE_FACTORY, "B@f()", asFUNCTION(B::factory), asCALL_CDECL); 
		engine->RegisterObjectBehaviour("B", asBEHAVE_RELEASE, "void f()", asMETHOD(B, release), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("B", asBEHAVE_ADDREF, "void f()", asMETHOD(B, addref), asCALL_THISCALL);
		engine->RegisterObjectMethod("B", "int test()", asMETHOD(B, test), asCALL_THISCALL);
		
		// Test the classes to make sure they work
		r = ExecuteString(engine, "A a; assert(a.test() == 1); B b; assert(b.test() == 2);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// It should be possible to register a REF_CAST to allow implicit cast
		// Test IMPLICIT_REF_CAST from subclass to baseclass
		r = engine->RegisterObjectMethod("B", "A@+ opImplCast()", asFUNCTION(B::castToA), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = ExecuteString(engine, "B b; A@ a = b; assert(a.test() == 2);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test explicit cast with registered IMPLICIT_REF_CAST
		r = ExecuteString(engine, "B b; A@ a = cast<A>(b); assert(a.test() == 2);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// It should be possible to assign a value of type B 
		// to and variable of type A due to the implicit ref cast
		r = ExecuteString(engine, "A a; B b; a = b;");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test REF_CAST from baseclass to subclass
		r = engine->RegisterObjectMethod("A", "B@+ opCast()", asFUNCTION(B::AcastToB), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = ExecuteString(engine, "B b; A@ a = cast<A>(b); B@ _b = cast<B>(a); assert(_b.test() == 2);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test REF_CAST from baseclass to subclass, where the cast is invalid
		r = ExecuteString(engine, "A a; B@ b = cast<B>(a); assert(@b == null);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Doing a ref cast on a null pointer must not throw an exception
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "A@ a;\n B@ b;\n @b = cast<B>(a);\n @a = @b;\n", 0, ctx);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		ctx->Release();

		// Various situations with ref casts on null pointers
		const char* script =
			"A@ global_a; \n"
			"B@ global_b; \n"
			"B@ global_b_nulled = null; \n"
			"B@ ret_null() { return null; } \n"
			"class C { B@ b; } \n"
			"void testFunc() { \n"
			"	A@ local_a = A(); \n"
			"	B@ local_b = B(); \n"
			"	@local_a = @local_b; \n"
			"	@global_a = @local_b; \n"
			"	@local_a = cast<A>(global_b); \n"
			"	@local_a = @global_b; \n"
			"	@local_a = @global_b_nulled; \n"
			"   @local_a = @ret_null(); \n"
			"   C c; \n"
			"   @local_a = @c.b; \n"
			"} \n";

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "testFunc();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Reported by ThyReaper
		{
			mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
			mod->AddScriptSection("script", 
				"class C \n"
				"{ \n"
				"  B b; \n"
				"  B@ get_a() property { return b; } \n"
				"  void set_a(B@ value) property { } \n"
				"} \n"
				"void func() \n"
				"{ \n"
				"  C c; \n"
				"  A @a; \n"
				"  @a = c.a; \n"
				"  assert( a is c.b ); \n"
				"} \n");
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;

			r = ExecuteString(engine, "func()", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}


		// TODO: This requires implicit value cast
		// Test passing a value of B to a function expecting its base class
		// the compiler will automatically create a copy
/*		script = 
			"void func(A a) {assert(a.test() == 1);}\n";
		r = mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "B b; func(b)");
		if( r < 0 )
			TEST_FAILED;
*/
		// TODO: A handle to A can not be implicitly cast to a handle to B since it was registered as explicit REF_CAST
		// TODO: It shouldn't be possible to cast away constness

		engine->Release();
	}

	// Test reported bug
	// http://www.gamedev.net/topic/639743-crash-using-asbehave-implicit-value-cast/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = engine->RegisterObjectType("Pdouble", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Pdouble", asBEHAVE_FACTORY, "Pdouble @f(double)", asFUNCTION(Param_double_Factory), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Pdouble", asBEHAVE_ADDREF,  "void f()", asMETHOD(Param<double>, add_ref), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Pdouble", asBEHAVE_RELEASE, "void f()", asMETHOD(Param<double>, release), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Pdouble", "double opImplConv() const", asMETHOD(Param<double>, v), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterGlobalFunction("double myFunction(const double)", asFUNCTIONPR(myFunction, (const double), double), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "Pdouble myValue( 0.3 ); \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "double temp = myValue; \n"
							      "assert( myFunction( temp ) == 0.3 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "assert( myFunction( myValue ) == 0.3 ); \n"
								  "Pdouble local( 0.3 ); \n"
								  "assert( myFunction( local ) == 0.3 );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test crash while returning from global handle
		// http://www.gamedev.net/topic/653857-implicit-value-cast-from-global-handle/
		mod->AddScriptSection("test",
			"Pdouble @dbl; \n"
			"double func1() { \n"
			"  @dbl = Pdouble(3.14); \n"
			"  return dbl; \n"
			"} \n"
			"double func2() { \n"
			"  Pdouble d(3.14); \n"
			"  return d; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "assert( func1() == 3.14 ); \n"
								  "assert( func2() == 3.14 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test implicit cast while assigning to global var
	// http://www.gamedev.net/topic/652108-implicit-value-cast-assigned-to-global-variable/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("type", sizeof(Value), asOBJ_VALUE|asOBJ_POD);
		engine->RegisterObjectProperty("type", "double dbl", asOFFSET(Value, dbl));
		engine->RegisterObjectMethod("type", "double opImplConv()", asMETHOD(Value, castToDouble), asCALL_THISCALL);
		
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"double g_Value; \n"
			"void main() \n"
			"{ \n"
			"    type x; \n"
			"    x.dbl = 42.2; \n"
			"    double Value; \n"
			"    Value = x; \n"
			"    g_Value = x; \n"
			"    assert( g_Value == 42.2 ); \n"
			"    assert( Value == 42.2 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED ) 
			TEST_FAILED;

		engine->Release();
	}

	// Test math operator with primitive and object type with implicit cast to primitive
	// http://www.gamedev.net/topic/653484-compile-math-operator-first-attempts-casting/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("type", sizeof(Value), asOBJ_VALUE|asOBJ_POD);
		engine->RegisterObjectProperty("type", "double dbl", asOFFSET(Value, dbl));
		engine->RegisterObjectMethod("type", "int opImplConv()", asMETHOD(Value, castToInt), asCALL_THISCALL);
		engine->RegisterObjectMethod("type", "double opImplConv()", asMETHOD(Value, castToDouble), asCALL_THISCALL);
		
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() \n"
			"{ \n"
			"  type x; \n"
			"  x.dbl = 3.5; \n"
			"  double y = x * 2; \n" // the implicit cast should choose double, since it has better precision
			"  assert( y == 7 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED ) 
			TEST_FAILED;

		engine->Release();
	}

	// Test opImplConv in script class
	// Test math operator with primitive and object type with implicit cast to primitive
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class type { \n"
			"  double dbl; \n"
			"  double opImplConv() { return dbl; } \n"
			"  int opImplConv() { return int(dbl); } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  type x; \n"
			"  x.dbl = 3.5; \n"
			"  double y = x * 2; \n" // the implicit cast should choose double, since it has better precision
			"  assert( y == 7 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED ) 
			TEST_FAILED;

		engine->Release();
	}

	// Test inheriting opConv and adding others
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class base { \n"
			"  double dbl; \n"
			"  double opConv() { return dbl; } \n"
			"} \n"
			"class derived : base { \n"
			"  int opConv() { return int(dbl); } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  derived x; \n"
			"  x.dbl = 3.5; \n"
			"  double y = double(x) * 2; \n"
			"  assert( y == 7 ); \n"
			"  int z = int(x) * 2; \n"
			"  assert( z == 6 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED ) 
			TEST_FAILED;

		engine->Release();
	}

	// Success
 	return fail;
}

struct Simple {
};

struct Complex {
};

void implicit(asIScriptGeneric *) {
}

static bool Test2()
{
	bool fail = false;
	COutStream out;
	int r;

	asIScriptEngine * engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	r = engine->RegisterObjectType("simple", sizeof(Simple), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert(r >= 0);
	r = engine->RegisterObjectType("complex", sizeof(Complex), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert(r >= 0);
	r = engine->RegisterObjectMethod("complex", "int opImplConv()", asFUNCTION(implicit), asCALL_GENERIC);
	r = engine->RegisterObjectMethod("complex", "double opImplConv()", asFUNCTION(implicit), asCALL_GENERIC);
	r = engine->RegisterObjectMethod("complex", "simple opImplConv()", asFUNCTION(implicit), asCALL_GENERIC);

	const char script[] =
	"void main() {\n"
	"  int i;\n"
	"  double d;\n"
	"  simple s;\n"
	"  complex c;\n"
	"  i = c;\n"
	"  d = c;\n"
	"  s = c;\n"
	"}";
	asIScriptModule * mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	r = mod->AddScriptSection("script", script, sizeof(script) - 1); assert(r >= 0);
	r = mod->Build(); 
	if( r < 0 )
	{
		TEST_FAILED;
	}
	else
	{
		asIScriptFunction *func = mod->GetFunctionByDecl("void main()"); assert(func);
		asIScriptContext * ctx = engine->CreateContext();

		r = ctx->Prepare(func); assert(r >= 0);
		r = ctx->Execute(); assert(r >= 0);

		ctx->Release();
	}

	// It must be possible to cast using an explicit construct cast
	r = ExecuteString(engine, "complex c; simple s = simple(c);");
	if( r < 0 )
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

//========================================================================================

class DisplayObject
{
public:
	DisplayObject() {refCount=1;}
	void AddRef() {refCount++;}
	void Release() {if(--refCount==0)delete this;}
	int refCount;
};

class MovieClip : public DisplayObject
{
public:
	MovieClip() : DisplayObject() {}
};

#if !defined(_MSC_VER) || _MSC_VER > 1200
// This doesn't seem to link well on MSVC6
template<class A, class B>
B* refCast(A* a)
{
    // If the handle already is a null handle, then just return the null handle
    if( !a ) return 0;

    // Now try to dynamically cast the pointer to the wanted type
    B* b = dynamic_cast<B*>(a);
    if( b != 0 )
    {
        // Since the cast was made, we need to increase the ref counter for the returned handle
        b->AddRef();
    }
    return b;
}
#endif

MovieClip *MovieClipFactory()
{
	return new MovieClip();
}

static bool Test3()
{
#if defined(_MSC_VER) && _MSC_VER <= 1200
	// This doesn't work on MSVC6
	return false;
#endif

	bool fail = false;
	asIScriptEngine *engine;
	COutStream out;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	engine->RegisterObjectType("DisplayObject", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("DisplayObject", asBEHAVE_ADDREF, "void f()", asMETHOD(DisplayObject, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("DisplayObject", asBEHAVE_RELEASE, "void f()", asMETHOD(DisplayObject, Release), asCALL_THISCALL);

	engine->RegisterObjectType("MovieClip", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("MovieClip", asBEHAVE_FACTORY, "MovieClip @f()", asFUNCTION(MovieClipFactory), asCALL_CDECL);
	engine->RegisterObjectBehaviour("MovieClip", asBEHAVE_ADDREF, "void f()", asMETHOD(MovieClip, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("MovieClip", asBEHAVE_RELEASE, "void f()", asMETHOD(MovieClip, Release), asCALL_THISCALL);
#if !defined(_MSC_VER) || _MSC_VER > 1200
 #ifdef __BORLANDC__
    // BCC cannot infer return values for function pointer types (QC #85378).
    // Use explicit cast via asFUNCTIONPR() instead.
	engine->RegisterObjectMethod("MovieClip", "DisplayObject @opImplCast()", asFUNCTIONPR((refCast<MovieClip, DisplayObject>),(MovieClip*),DisplayObject*), asCALL_CDECL_OBJLAST);
 #else
	engine->RegisterObjectMethod("MovieClip", "DisplayObject @opImplCast()", asFUNCTION((refCast<MovieClip, DisplayObject>)), asCALL_CDECL_OBJLAST);
//	engine->RegisterObjectMethod("MovieClip", "DisplayObject @opImplCast()", WRAP_OBJ_LAST((refCast<MovieClip, DisplayObject>)), asCALL_GENERIC);
 #endif
#endif

	const char *script = 
		"class TransitionManager { \n"
		"  void MoveTo(DisplayObject @o) {} \n"
		"} \n"
		"TransitionManager mgr; \n"
		"MovieClip movie; \n"
		"void OnLoad() \n"
		"{ \n"
		"  mgr.MoveTo(movie); \n"
		"  mgr.MoveTo(@movie); \n"
		"} \n";

	asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	int r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	r = ExecuteString(engine, "OnLoad()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();	

	return fail;
}

//==========================================================================================================
// Value cast didn't work when returning object by value
// http://www.gamedev.net/topic/614070-implicit-value-cast-and-explicit-value-cast-no-longer-working-with-2212/

struct Castee{
	Castee() {}
	Castee(int v) : m_v(v) {}
	Castee(const Castee &v) : m_v(v.m_v) {}
	Castee& operator=(const Castee& rhs) { m_v = rhs.m_v; return *this; } 
	int GetValue() const { return m_v; }
	int m_v;

	static void Construct(void *mem) { new(mem) Castee(); }
	static void Construct2(void *mem, const Castee &v) { new(mem) Castee(v); }
	static void Destruct(void * /*mem*/) {}
};
struct Caster{ 
	Caster() {}
	Caster(int v) : m_v(v) {}
	operator Castee() const { return Castee(m_v); } 
	int m_v;

	static void Construct(void *mem) { new(mem) Caster(); }
	static void Construct2(void *mem, int v) { new(mem) Caster(v); }
	static void Destruct(void * /*mem*/) {}
}; 

static bool Test4()
{
	bool fail = false;
	int r;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	r = engine->RegisterObjectType("Castee", sizeof(Castee), asOBJ_VALUE | asOBJ_APP_CLASS_CAK); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Castee", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Castee::Construct), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
//	r = engine->RegisterObjectBehaviour("Castee", asBEHAVE_CONSTRUCT, "void f(const Castee &in)", asFUNCTION(Castee::Construct2), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Castee", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Castee::Destruct), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Castee", "Castee &opAssign(const Castee &in)", asMETHOD(Castee, operator=), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Castee", "int GetValue() const", asMETHOD(Castee, GetValue), asCALL_THISCALL); assert( r >= 0 );

	r = engine->RegisterObjectType("Caster", sizeof(Caster), asOBJ_VALUE | asOBJ_APP_CLASS_C); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Caster", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Caster::Construct), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Caster", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(Caster::Construct2), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Caster", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Caster::Destruct), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Caster", "Castee opImplConv() const", asMETHOD(Caster, operator Castee), asCALL_THISCALL); assert( r >= 0 );

	asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

	mod->AddScriptSection("test", "int Run() \n"
							      "{ \n"
								  "  return GetValueFromCastee(Caster(5)); \n"
                                  "} \n"
								  "int GetValueFromCastee(Castee castee) \n"
	                              "{ \n"
								  "  return castee.GetValue(); \n"
                                  "} \n");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(mod->GetFunctionByDecl("int Run()"));
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ctx->GetReturnDWord();
	if( r != 5 )
		TEST_FAILED;

	mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", "int Run() \n"
								  "{ \n"
								  "  Caster caster(5); \n"
								  "  Castee castee = caster; \n"
								  "  return GetValueFromCastee(castee); \n"
								  "} \n"
								  "int GetValueFromCastee(const Castee &in castee) \n"
								  "{ \n"
								  "  return castee.GetValue(); \n"
								  "} \n");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	ctx->Prepare(mod->GetFunctionByDecl("int Run()"));
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	r = ctx->GetReturnDWord();
	if( r != 5 )
		TEST_FAILED;

	ctx->Release();
	engine->Release();

	return fail;
}

//----------------------------------------------------------------------------------
// http://www.gamedev.net/topic/654193-implicit-reference-cast-in-function-argument/

class RefTest5
{
public:
	RefTest5() : Refs(1) {}
	virtual ~RefTest5() {}
	virtual void AddRef() const
	{
		++Refs;
	}
	virtual void Release() const
	{
		if (--Refs == 0) delete this;
	}

	mutable int Refs;
};

class ATest5
{
public:
	virtual int GetValue() const=0;
};

class BTest5 : public RefTest5, public ATest5
{
public:
	static BTest5* Factory() { return new BTest5; }

	virtual int GetValue() const 
	{
		return 7;
	}

	ATest5* RefCastA() { return static_cast<ATest5*>(this); }
};

class CTest5
{
public:
	CTest5& opShl(const ATest5& x)
	{
		Value = x.GetValue();
		return *this;
	}

	int Value;
};

static const char* script =
"void main()                      \n"
"{                                \n"
"    B x;                         \n"
"    C y;                         \n"
"    y << x;                      \n"
"    assert( y.Value == 7 );      \n"
"}                                \n";


bool Test5()
{
	bool fail = false;
	int r;
	asIScriptEngine* engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	engine->RegisterObjectType("A", 0, asOBJ_REF | asOBJ_NOCOUNT);
	
	engine->RegisterObjectType("B", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("B", asBEHAVE_FACTORY,
		"B@ f()", asFUNCTION(BTest5::Factory), asCALL_CDECL);
	engine->RegisterObjectBehaviour("B", asBEHAVE_ADDREF,
		"void AddRef() const", asMETHODPR(BTest5, AddRef, () const, void), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("B", asBEHAVE_RELEASE,
		"void Release() const", asMETHODPR(BTest5, Release, () const, void), asCALL_THISCALL);
	engine->RegisterObjectMethod("B", 
		"A@ opImplCast()", asMETHOD(BTest5, RefCastA), asCALL_THISCALL);

	engine->RegisterObjectType("C", sizeof(CTest5), asOBJ_VALUE | asOBJ_POD);
	engine->RegisterObjectMethod("C", "C& opShl(const A&)",
	asMETHOD(CTest5, opShl), asCALL_THISCALL);
	engine->RegisterObjectProperty("C", "int Value", offsetof(CTest5, Value));

	asIScriptModule* mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("Script", script);

	//engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
	mod->Build();

	asIScriptContext* ctx = engine->CreateContext();
	r = ExecuteString(engine, "main()", mod, ctx);
	if (r != asEXECUTION_FINISHED) TEST_FAILED;

	ctx->Release();
	engine->Release();

	return fail;
}


} // namespace

