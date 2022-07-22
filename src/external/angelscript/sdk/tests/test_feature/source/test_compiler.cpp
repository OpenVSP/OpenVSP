#include "utils.h"
#include <sstream>
#include "../../../add_on/scriptdictionary/scriptdictionary.h"
#include "../../../add_on/scriptany/scriptany.h"
#include "../../../add_on/scriptmath/scriptmath.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"
#include <iostream>

using namespace std;

namespace TestCompiler
{

static const char * const TESTNAME = "TestCompiler";


bool Test2();
bool Test3();
bool Test4();
bool Test5();
bool Test6();
bool Test7();
bool Test8();
bool Test9();
bool TestRetRef();

struct A {
    A() { text = "hello"; }
    static void Constructor(A *self) {new(self) A();}
    static void Destructor(A *memory) {memory->~A();}
    std::string getText() {return this->text;}
    std::string text;
    A getA() {return A();}
};

// For test Philip Bennefall
class CSound
{
public:
	CSound() { refCount = 1; }
	void AddRef() { refCount++; }
	void Release() { if( --refCount == 0 ) delete this; }
	double get_pan() const {return 0;}
	void set_pan(double &) {}
	int refCount;

	static CSound *CSound_fact() {return new CSound();}
};

class CStringFactory : public asIStringFactory
{
public:
	const void *GetStringConstant(const char *data, asUINT len)
	{
		return new std::string(data, len);
	}
	int ReleaseStringConstant(const void *str)
	{
		delete reinterpret_cast<const std::string*>(str);
		return 0;
	}
	int GetRawStringData(const void *str, char *data, asUINT *len) const
	{
		if (len) *len = (asUINT)reinterpret_cast<const std::string*>(str)->length();
		if (data) memcpy(data, reinterpret_cast<const std::string *>(str)->c_str(), reinterpret_cast<const std::string*>(str)->length());
		return 0;
	}
} stringFactory;

static void ConstructStringGeneric(asIScriptGeneric * gen) {
  new (gen->GetObject()) string();
}

static void CopyConstructStringGeneric(asIScriptGeneric * gen) {
  string * a = static_cast<string *>(gen->GetArgObject(0));
  new (gen->GetObject()) string(*a);
}

static void DestructStringGeneric(asIScriptGeneric * gen) {
  string * ptr = static_cast<string *>(gen->GetObject());
  ptr->~string();
}

static void AssignStringGeneric(asIScriptGeneric *gen) {
  string * a = static_cast<string *>(gen->GetArgObject(0));
  string * self = static_cast<string *>(gen->GetObject());
  *self = *a;
  gen->SetReturnAddress(self);
}

static void StringAddGeneric(asIScriptGeneric * gen) {
  string * a = static_cast<string *>(gen->GetObject());
  string * b = static_cast<string *>(gen->GetArgAddress(0));
  string ret_val = *a + *b;
  gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric * gen) {
  string * self = static_cast<string *>(gen->GetObject());
  *static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) = (asUINT)self->length();
}

static void AddString2IntGeneric(asIScriptGeneric * gen) {
  string * a = static_cast<string *>(gen->GetObject());
  int * b = static_cast<int *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static string alert_buf;
static void AlertGeneric(asIScriptGeneric * gen) {
	string *a = static_cast<string*>(gen->GetArgAddress(0));
	string *b = static_cast<string*>(gen->GetArgAddress(1));
	alert_buf += *a;
	alert_buf += *b;
	alert_buf += "\n";
}

static void String_get_opIndexGeneric(asIScriptGeneric *gen) {
  string * a = static_cast<string *>(gen->GetObject());
  asUINT i = gen->GetArgDWord(0);
  string ret_val = a->substr(i, 1);
  gen->SetReturnObject(&ret_val);
}

static void String_set_opIndexGeneric(asIScriptGeneric *gen) {
  string * a = static_cast<string *>(gen->GetObject());
  asUINT i = gen->GetArgDWord(0);
  string *str = static_cast<string*>(gen->GetArgAddress(1));
  (*a)[i] = (*str)[0];
}

string g_printbuf;
void Print(const string &s)
{
	g_printbuf += s;
}

// For the test with chained method calls
class ChainMe
{
public:
	int X;

	ChainMe() :
		X(0)
	{
	}

	ChainMe &Increase(const int &v)
	{
		X += v;
		return *this;
	}

	static void Construct(void *p)
	{
		new(p) ChainMe();
	}
};

void DoNothing(asIScriptGeneric * /*gen*/)
{
}

void *NullFactory()
{
	return 0;
}

bool Test()
{
	bool fail = false;
	int r;

	asIScriptEngine *engine;
	CBufferedOutStream bout;
	COutStream out;
	asIScriptModule *mod;

	// Test bytecode sequence on calling method on explicitly created temporary objects
	// https://www.gamedev.net/forums/topic/708821-crash-on-temp-string-objects/5434393/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);

		r = ExecuteString(engine, "string('aaaa').findFirst('v'); \n"
		                          "string aaaa('aaaa'); \n"
		                          "aaaa.findFirst('v'); \n");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test use of virtual property without get accessor causing assert failure
	// Reported by Patrick Jeeves
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptMathComplex(engine);
		r = engine->RegisterGlobalFunction("float dot(const complex &in, const complex &in)", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);
	
		r = engine->RegisterObjectType("foo", sizeof(int), asOBJ_VALUE|asOBJ_POD);
		r = engine->RegisterObjectProperty("foo", "complex velocity", 0);
		r = engine->RegisterObjectMethod("foo", "void set_velocity(const complex &in v) property", asFUNCTION(0), asCALL_GENERIC);
		
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class TestClass\n"
			"{\n"
			"  foo m; \n"
			"  void onCollision(complex &in col) { \n"
			"    if( dot(m.velocity, col) > 20 ) {} \n"
			"  } \n"
			"}\n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		
		if( bout.buffer != "test (4, 3) : Info    : Compiling void TestClass::onCollision(complex&in)\n"
						   "test (5, 14) : Error   : The property has no get accessor\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}	
		
		engine->ShutDownAndRelease();
	}
	
	// Test problem with class having a single contructor taking 1 argument
	// https://www.gamedev.net/forums/topic/702543-object-handle-and-constructor-with-array-argument-triggers-assert/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		
		RegisterScriptArray(engine, false);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class TestClass\n"
			"{\n"
			"  TestClass(array<int> arr)\n"
			"  {\n"
			"    this.arr = arr;\n"
			"  }\n"
			"  private array<int> arr;\n"
			"}\n"
			"void main()\n"
			"{\n"
			"  TestClass @t = TestClass({});\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		engine->ShutDownAndRelease();
	}
	
	// Test what happens when a registered factory return null without raising an exception
	// This is an undefined behaviour, and it is invalid for a factory function to do this
	// https://www.gamedev.net/forums/topic/701081-question-about-nullptrs/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterObjectType("foo", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectBehaviour("foo", asBEHAVE_FACTORY, "foo @f()", asFUNCTION(NullFactory), asCALL_CDECL);
		
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "foo f(); foo @f2 = f;", 0, ctx);
		if( r != 0 )
		{
			TEST_FAILED;
			if( r == asEXECUTION_EXCEPTION && strcmp(ctx->GetExceptionString(), "Null pointer access") != 0 )
				PRINTF("exception caught: %s\n", ctx->GetExceptionString());
		}
		ctx->Release();
		
		engine->ShutDownAndRelease();
	}
	
	
	// Proper error handling on duplicate class methods
	// https://www.gamedev.net/forums/topic/700394-compiler-crash-on-double-function-compiler-error/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Foo \n"
			"{ \n"
			"	void a() {} \n"
			"	void a() {} \n"
			"} \n"
			"class Bar : Foo \n"
			"{ \n"
			"	void a() override {} \n"
			"} \n"
			"class FooBar : Bar \n"
			"{ \n"
			"} \n"
			"void Test() \n"
			"{ \n"
			"	FooBar@ b = FooBar(); \n"
			"	b.a(); \n"
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (4, 2) : Error   : A function with the same name and parameters already exists\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test to make sure no crash occurs when anonymous list is matched against function taking funcdef
	// https://www.gamedev.net/forums/topic/699951-crash-in-argument-matching/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);
		
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void cb(); \n"
			"void f(cb @t) {} \n"
			"void f(array<int> a) {} \n"
			"void main() { \n"
			"	f({1}); \n"    // this crashed the compiler before the fix
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
	
	// Test invalid code
	// https://www.gamedev.net/forums/topic/696243-strange-code-works-normally/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A \n"
			"{ \n"
			"} \n"
			"class B \n"
			"{ \n"
			"	B() { } \n"
			"} \n"
			"int main() \n"
			"{ \n"
			"	A a; \n"
			"	a.B(); \n"
			"	return 0; \n"
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (8, 1) : Info    : Compiling int main()\n"
						   "test (11, 4) : Error   : No matching symbol 'B'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test passing object by value where the object is initialized with anonymous list
	// Reported by Patrick Jeeves
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterObjectType("Point2F", 1, asOBJ_VALUE);
		engine->RegisterObjectBehaviour("Point2F", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Point2F", asBEHAVE_CONSTRUCT, "void f(const Point2F &in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Point2F", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float}", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Point2F", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Point2F", "Point2F &opAssign(const Point2F &in)", asFUNCTION(0), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool setPosition(Point2F) {return true;} \n"
			"void main() \n"
			"{ \n"
			"  setPosition({ 2800, 5000 }); \n"
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] =
		{
			asBC_SUSPEND,asBC_PSF,
			// Setup the initialization list
			asBC_AllocMem,asBC_SetV4,asBC_PshListElmnt,asBC_PopRPtr,asBC_WRTV4,asBC_SetV4,asBC_PshListElmnt,asBC_PopRPtr,asBC_WRTV4,
			// Allocate the Point2F with the initialization list
			asBC_PshVPtr,asBC_PSF,asBC_CALLSYS,
			// Free the initialization list
			asBC_FREE,
			// Copy the Point2F to a temp Point2F allocated on the heap
			// TODO: optimize: Avoid this copy by allocating the first object on the heap directly
			asBC_PSF,asBC_ALLOC,
			// Free the first Point2F
			asBC_PSF,asBC_CALLSYS,
			// Call setPosition with the Point2F allocated on the heap
			asBC_VAR,asBC_GETOBJ,asBC_CALL,
			// Exit
			asBC_SUSPEND,asBC_RET
		};
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test assert failure on implicit conversion of primitive types
	// https://www.gamedev.net/forums/topic/692729-assert-failure-in-ascexprvaluegetconstantdw/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"const uint16 FREQUENCY = 2000; \n"
			"void onTick(CBlob@ blob) \n"
			"{ \n"
			"	if (blob.getTickSinceCreated() < FREQUENCY) return; \n"
			"} \n"
			"class CBlob { int getTickSinceCreated() { return 0; } } \n"
			"const uint8 COIN_COST = 60; \n"
			"void blah(CBlob @player) \n"
			"{ \n"
			"	if ( player.getTickSinceCreated() < COIN_COST) { } \n"
			"}\n"
			"const uint8 constant = 5; \n"
			"void foo() \n"
			"{ \n"
			"	-constant; \n"
			"}\n");
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

	// Test parser error message
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void Test(() {}");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 11) : Error   : Expected data type\n"
						   "test (1, 11) : Error   : Instead found '('\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test string operators
	// http://www.gamedev.net/topic/684124-weird-string-behavior-when-using/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Button { string m_func; string GetScenario() { return 'bar'; } } \n");

		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, 
			"Button but; but.m_func = 'scenario bar'; int result = 0; \n"
			"if( but.m_func == 'scenario ' + but.GetScenario() ) result = 1; \n"
			"assert( result == 1 ); \n", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine,
			"Button but; but.m_func = 'scenario bar'; string id = 'bar'; int result = 0; \n"
			"if( but.m_func == 'scenario ' + id ) result = 1; \n"
			"assert( result == 1 ); \n", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine,
			"Button but; but.m_func = 'scenario bar'; string id = 'scenario ' + but.GetScenario(); int result = 0; \n"
			"if( but.m_func == id ) result = 1; \n"
			"assert( result == 1 ); \n", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test order of evaluation for operands
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool funcAIsCalled = false; \n"
			"bool funcBIsCalled = false; \n"
			"string funcA() { \n"
			"  funcAIsCalled = true; \n"
			"  assert( funcBIsCalled == false ); \n"
			"  return 'A'; \n"
			"} \n"
			"string funcB() { \n"
			"  funcBIsCalled = true; \n"
			"  assert( funcAIsCalled == true ); \n"
			"  return 'B'; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "string r = funcA() + funcB(); assert( r == 'AB' );", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
			if (r == asEXECUTION_EXCEPTION)
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}
		ctx->Release();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test constant values
	// http://www.gamedev.net/topic/682120-warning-value-is-too-large-for-data-type-incorrect/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"int8 x1 = -1; \n"
			"int16 y1 = -1; \n"
			"int8 x2 = 255; \n" // too big for 8bit signed
			"int16 y2 = 65535; \n" // too big for 16bit signed
			"int8 x3 = 4294967297; \n" // too big for 8bit signed, too big for 32bit unsigned
			"int16 y3 = 4294967297; \n" // too big for 16bit signed
			"int x4 = 4294967295; \n" // too big for 32bit signed, but not for 32bit unsigned
			"int64 y4 = 18446744073709551616; \n" // overflow 64bit unsigned
			"uint64 z4 = 18446744073709551616; \n" // overflow 64bit unsigned biggest signed 9223372036854775807, biggest unsigned 18446744073709551615
			"uint8 x5 = 18446744073709551616; \n" // overflow 64bit unsigned
			"uint16 y5 = 18446744073709551616; \n" // overflow 64bit unsigned
			"uint z5 = 0x10000000000000000; \n" // overflow 64bit unsigned
			"uint x6 = 18446744073709551615; \n" // too big for 32bit unsigned, but not for 64bit
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
						   "test (4, 11) : Warning : Value is too large for data type\n"
						   "test (5, 12) : Warning : Value is too large for data type\n"
						   "test (6, 11) : Warning : Value is too large for data type\n"
						   "test (7, 12) : Warning : Value is too large for data type\n"
						   "test (8, 10) : Warning : Value is too large for data type\n"
						   "test (9, 12) : Error   : Value is too large for data type\n"
						   "test (10, 13) : Error   : Value is too large for data type\n"
						   "test (11, 12) : Error   : Value is too large for data type\n"
						   "test (12, 13) : Error   : Value is too large for data type\n"
						   "test (13, 11) : Error   : Value is too large for data type\n"
						   "test (14, 11) : Warning : Value is too large for data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test 8bit and 16bit uint constants 
	// http://www.gamedev.net/topic/683658-uint8-and-uint16-constants-cause-compile-time-assertion-failure-on-type-conversion-to-signed-types/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"const uint8 foo = 0; \n"
			"int bar = foo;  \n");
		mod->AddScriptSection("test2",
			"const uint16 foo2 = 0; \n"
			"void bar2(int16) {} \n"
			"void main() { \n"
			"	bar2(foo2); \n"
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

	// Test 8bit and 16bit integers
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"const int8 INT8_MIN = -1; \n"
			"const int16 INT16_MIN = -32768; \n");
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

	// Test boolean constants
	// http://www.gamedev.net/topic/682759-assert-failure-when-evaluating-const-bool-as-false-in-if-statement/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"	const bool value = true; \n"
			"	if (!value) {} \n"
			"} \n"
			"void Init() { \n"
			"  const bool show_invisible = false; \n"
			"  if (show_invisible == false) {} \n"
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

	// Test attempt to use null as a function
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() \n"
			"{ \n"
			"  null(); \n" // give a proper error message
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
						   "test (3, 7) : Error   : Expression doesn't evaluate to a function\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test anonymous array objects in expressions
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		RegisterScriptArray(engine, true);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() \n"
			"{ \n"
			" array<int> arr; \n"
			" if(arr == array<int> = { 1, 2 }) \n"
			"   arr.resize(0);\n"
		// TODO: This should work with the old style of array types too
		//	" if(arr == int[] = {1, 2}) \n"
		//	"   arr.resize(1);\n"
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


	// Test void &, which should fail with appropriate error message
	// http://www.gamedev.net/topic/677273-various-unexpected-behaviors-of-angelscript-2310/
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void &func1() {} \n"
			"void func2(void &, void &in, void &out) {} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 6) : Error   : Type 'void' cannot be a reference\n"
						   "test (2, 17) : Error   : Type 'void' cannot be a reference\n"
						   "test (2, 25) : Error   : Type 'void' cannot be a reference\n"
						   "test (2, 35) : Error   : Type 'void' cannot be a reference\n"
						/*   "test (2, 1) : Info    : Compiling void func2(void, void, void)\n"
						   "test (2, 1) : Error   : Parameter type can't be 'void', because the type cannot be instantiated.\n"
						   "test (2, 1) : Error   : Parameter type can't be 'void', because the type cannot be instantiated.\n"
						   "test (2, 1) : Error   : Parameter type can't be 'void', because the type cannot be instantiated.\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test funcdefs, property accessors, anonymous functions, and wrong syntax
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		asIScriptModule *module = engine->GetModule("testCallback", asGM_ALWAYS_CREATE);
		asIScriptContext *context = engine->CreateContext();

		r = engine->RegisterFuncdef("int Callback()"); assert(r >= 0);
		r = engine->RegisterGlobalFunction("void set_TestCallback(Callback@+ cb) property", asFUNCTION(DoNothing), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("Callback@ get_TestCallback() property", asFUNCTION(DoNothing), asCALL_GENERIC); assert(r >= 0);

		r = module->AddScriptSection("test", "void main1(){ Callback@ cb = function() {return 123;}; TestCallback = cb; }"); assert(r >= 0);
		r = module->Build(); // <== Crash Here
		if (r >= 0)
			TEST_FAILED;

		r = module->AddScriptSection("test", "void main2(){ Callback@ cb = test; TestCallback = cb; } int test() {return 123;}"); assert(r >= 0);
		r = module->Build(); // <== Crash Here
		if (r >= 0)
			TEST_FAILED;

		r = module->AddScriptSection("test", "void main3(){ TestCallback = function() {return 123;}; }"); assert(r >= 0);
		r = module->Build(); // <== Crash Here
		if (r >= 0)
			TEST_FAILED;

		r = module->AddScriptSection("test", "void main4(){ Callback@ cb = function() {return 123;}; @TestCallback = cb; }"); assert(r >= 0);
		r = module->Build(); 
		if (r < 0)
			TEST_FAILED;

		r = module->AddScriptSection("test", "void main5(){ @TestCallback = function(){return 123;}; }"); assert(r >= 0);
		r = module->Build(); 
		if (r < 0)
			TEST_FAILED;

		r = context->Prepare(module->GetFunctionByName("main5")); 
		if (r < 0)
			TEST_FAILED;

		if (context->Execute() != asEXECUTION_FINISHED)
			TEST_FAILED;

		context->Release();
		module->Discard();

		if (bout.buffer != "test (1, 1) : Info    : Compiling void main1()\n"
						   "test (1, 69) : Error   : No appropriate opAssign method found in 'Callback' for value assignment\n"
						   "test (1, 1) : Info    : Compiling void main2()\n"
						   "test (1, 49) : Error   : No appropriate opAssign method found in 'Callback' for value assignment\n"
						   "test (1, 1) : Info    : Compiling void main3()\n"
						   "test (1, 41) : Error   : Can't implicitly convert from 'const Callback@&' to 'Callback&'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test assert failure in compiler
	// http://www.gamedev.net/topic/676120-compiler-assert-hit-in-deallocatevariable/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A { \n"
			"  A() { value = 42; } \n"
			"  A(A@ other) { value = other.value*2; } \n"
			"  int value; \n"
			"}; \n"
			"void func(A@ a) {\n"
			"  A x(a); \n" // make a copy of the object
			"  assert( a.value == 42 ); \n" // original object must still be available through handle
			"  assert( x.value == 84 ); \n"
			"}; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "A a; func(a)", mod, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (r == asEXECUTION_EXCEPTION)
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		ctx->Release();
		
		engine->ShutDownAndRelease();
	}

	// Test implicit conversion to handle
	// Make sure no incorrect warning message is given
	// http://www.gamedev.net/topic/661910-template-containers-angelscript-addon-library-release/page-2#entry5273466
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A { \n"
			"  A@&opIndex(A@&in) { return g_a; } \n"
			"} \n"
			"A @g_a; \n"
			"void main() { \n"
			"  if( @g_a[@g_a] == null ) {} \n"
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

	// Test const correctness with parameters
	// http://www.gamedev.net/topic/673892-const-broken/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"
			"void foo(const A& a) { foo2(a); } \n" // foo2 call must fail as foo2 is expecting a const reference
			"void foo2(A& a) {}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (2, 1) : Info    : Compiling void foo(const A&inout)\n"
						   "test (2, 24) : Error   : No matching signatures to 'foo2(const A&)'\n"
						   "test (2, 24) : Info    : Candidates are:\n"
						   "test (2, 24) : Info    : void foo2(A&inout a)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test unicode identifiers
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, true);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Jönsson {} \n"
			"Jönsson jönsson; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test that script class isn't marked as garbage collected needlessly
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		// class B is declared between A and C, since generally the classes are built in the order they are declared
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"							  // don't need garbage collection
			"class B { array<A> a; array<C@> c; } \n" // don't need garbage collection
			"final class C {} \n"					  // don't need garbage collection
			);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asITypeInfo *type = mod->GetTypeInfoByName("A");
		if( type == 0 || (type->GetFlags() & asOBJ_GC) )
			TEST_FAILED;
		type = mod->GetTypeInfoByName("B");
		if( type == 0 || (type->GetFlags() & asOBJ_GC) )
			TEST_FAILED;
		type = mod->GetTypeInfoByName("C");
		if( type == 0 || (type->GetFlags() & asOBJ_GC) )
			TEST_FAILED;

		engine->Release();
	}

	// Warn if inner scope re-declares variable from outer scope
	// http://www.gamedev.net/topic/660746-problem-with-random-float-value-on-android/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptArray(engine, false);

		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"  array<uint> arr = {0,1,2,3,4,5,6,7,8,9}; \n"
			"  for( uint i = 0; i < 10; i++ ) { \n"
			"    for( uint i = 0; i < arr[i]; i++ ) { \n"
			"    } \n"
			"  } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
						   "test (4, 15) : Warning : Variable 'i' hides another variable of same name in outer scope\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Give error if &out arg is called with non-lvalue expr
	// http://www.gamedev.net/topic/660363-retrieving-an-array-of-strings-from-a-dictionary/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);

		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"  dictionary d = { {'arr', array<string>(1, 'something')} }; \n"
			"  array<string> arr4; \n"
			"  bool found4 = d.get('arr', @arr4); \n" // This is not valid, because arr4 is not a handle and cannot be reassigned
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
		                   "test (4, 31) : Error   : Output argument expression is not assignable\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test identity comparison with output handle
	// http://www.gamedev.net/topic/660025-inconsistent-behavior-with-ref-type-and-out-references-to-handle-params/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func(A@&out a) \n"
			"{ \n"
			"  assert( a is null ); \n"
			"  @a = A(); \n"
			"  assert( a !is null ); \n"
			"} \n"
			"class A{} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "A @a; func(a); assert( a !is null );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test invalid syntax
	// http://www.gamedev.net/topic/659153-crash-when-instantiating-handle-with-weird-syntax/
	{
		const char *script = 
			"array<string> foo = { 'a', 'b', 'c' };\n"
			"dictionary d1 = { {'arr', foo} };\n"
			"array<string>@ s1 = array<string>@(d1['arr']);\n";

		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (3, 16) : Info    : Compiling array<string>@ s1\n"
		                   "test (3, 21) : Error   : Can't construct handle 'array<string>@'. Use ref cast instead\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Give proper error when declaring variable as only statement of an if
	// http://www.gamedev.net/topic/653474-compile-error-in-if-statement-without-braces/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func(int a) \n"
			"{ \n"
			"  if( true ) \n"
			"    int a; \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void func(int)\n"
		                   "test (4, 5) : Error   : Unexpected variable declaration\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test appropriate error when attempting to declare variable as reference
	// http://www.gamedev.net/topic/657196-problem-returning-reference-to-internal-members/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {}; \n"
			"void func(int a) { \n"
			"  A &a; \n"      // Must detect this as a declaration and give error 
			"  A &b = a; \n"  // instead of thinking it is a bitwise and-operation
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (2, 1) : Info    : Compiling void func(int)\n"
		                   "test (3, 5) : Error   : Expected identifier\n"
		                   "test (3, 5) : Error   : Instead found '&'\n"
		                   "test (4, 5) : Error   : Expected identifier\n"
		                   "test (4, 5) : Error   : Instead found '&'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		bout.buffer = "";
		mod->AddScriptSection("test",
			"int &glob; \n");  // Must give error for invalid reference
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 5) : Error   : Expected identifier\n"
		                   "test (1, 5) : Error   : Instead found '&'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test proper error when attempting to use a type from an unrelated namespace
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"namespace A { class C {} } \n"
			"namespace B { void func( C @c ) {} } \n"
			"void func2( C @c ) {} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (2, 26) : Error   : Identifier 'C' is not a data type in namespace 'B' or parent\n"
			               "test (3, 13) : Error   : Identifier 'C' is not a data type in global namespace\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test proper error in case of double handle in variable decl
	// http://www.gamedev.net/topic/657480-double-handle/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);

		r = ExecuteString(engine, "dictionary @@dict;");
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 13) : Error   : Handle to handle is not allowed\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test name conflict with global variables
	// http://www.gamedev.net/topic/656569-global-variable-redeclaration/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"double x; \n"
			"double x; \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "test (2, 8) : Error   : Name conflict. 'x' is a global property.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test memory leak issue
	// http://www.gamedev.net/topic/655054-garbage-collection-bug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int g_int; \n"
			"class Leaker \n"
			"{ \n"
			"  Leaker@ Foo()      { return this;  } \n"
			"  Leaker@ Bar()      { return this;  } \n"
			"  Leaker@ Goo()      { return this;  } \n"
			"  Leaker@ opDiv(int) { return this;  } \n"
			"  int& Car()         { return g_int; } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"    Leaker x; \n"
			"    x.Foo().Goo() / x.Foo().Bar().Goo().Car(); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem with opAssign reported by loboWu
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptAny(engine);
		RegisterScriptArray(engine, true);
		RegisterStdString(engine);
		RegisterScriptDictionary(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class VARIANT \n"
			"{ \n"
			"    VARIANT@ opAssign(const VARIANT &in v) \n"
			"    { \n"
			"        return this; \n"
			"    } \n"
			"}; \n"
			"dictionary variant_code_map; \n"
			"void main() \n"
			"{ \n"
			"    VARIANT[] variant_code(1); \n"
			"    any a(variant_code[0]); \n"
			"    variant_code_map.set('current_variant_code', variant_code[0]); \n" // Crash!!!!!
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test crash with global variables of non-pod types and reference arguments
	// http://www.gamedev.net/topic/653919-global-variables-and-const-argument-by-reference/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		r = engine->RegisterObjectType("Value", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS);

		r = engine->RegisterObjectType("Vec2f", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDAK);
		r = engine->RegisterObjectBehaviour("Vec2f", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		// There is no copy constructor so the compiler must use the opAssign
		r = engine->RegisterObjectBehaviour("Vec2f", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		// The opAssign is registered to return a value type by value
		r = engine->RegisterObjectMethod("Vec2f", "Value opAssign(const Vec2f &in)", asFUNCTION(0), asCALL_GENERIC);



		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"Vec2f t; \n"
			"void main(){ \n"
			"    test( t ); \n"
			"} \n"
			"void test( const Vec2f &in v ){}\n");
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

	// Make sure tokenizer doesn't split tokens whose initial characters match keyword
	// http://www.gamedev.net/topic/653337-uint-token-parse-error/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		asUINT len = 0;
		if( engine->ParseToken("uintHello", 0, &len) != asTC_IDENTIFIER || len != 9 )
			TEST_FAILED;

		if( engine->ParseToken("uint8Hello", 0, &len) != asTC_IDENTIFIER || len != 10 )
			TEST_FAILED;

		if( engine->ParseToken("int32Hello", 0, &len) != asTC_IDENTIFIER || len != 10 )
			TEST_FAILED;

		if( engine->ParseToken("uint32Hello", 0, &len) != asTC_IDENTIFIER || len != 11 )
			TEST_FAILED;

		engine->Release();
	}

	// Test compiler warning with implicit conversion of enums
	// http://www.gamedev.net/topic/652867-implicit-conversion-changed-sign-of-value/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void main(uint flags) { \n"
			"  if( (func() & VAL) != 0 ) {} \n"
			"  if( (flags & VAL2) == VAL2 ) {} \n"
			"} \n"
			"int func() { return 1; } \n"
			"enum E { VAL = -1, VAL2 = 2 } \n");
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

	// Test the logic for JIT compilation
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		class JitCompiler : public asIJITCompiler
		{
		public:
			JitCompiler() : invokeCount(0) {}
			virtual int  CompileFunction(asIScriptFunction* /*function*/, asJITFunction* /*output*/) { invokeCount++; return 0; }
			virtual void ReleaseJITFunction(asJITFunction /*func*/) { }
			int invokeCount;
		} jit;

		engine->SetJITCompiler(&jit);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "void func() {}");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Warning : Function 'void func()' appears to have been compiled without JIT entry points\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = mod->CompileFunction("test2", "void func2() {}", 0, asCOMP_ADD_TO_MODULE, 0);
		if (r < 0)
			TEST_FAILED;

		if (jit.invokeCount != 2)
			TEST_FAILED;

		engine->Release();
	}

	// Test string with implicit cast to primitive and dictionary
	// http://www.gamedev.net/topic/652681-bug-problem-with-dictionary-addonimplicit-casts/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);
		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

		// Register additional behaviour to the CScriptString to allow it to be implicitly converted to/from double
		struct helper
		{
			static CScriptString *objectString_FactoryFromDouble(double v) 
			{ 
				CScriptString *str = new CScriptString(); 
				stringstream s; 
				s << v; 
				str->buffer = s.str(); 
				return str; 
			}
			static double objectString_CastToDouble(CScriptString *str) 
			{ 
				stringstream s(str->buffer); 
				double v; 
				s >> v; 
				return v; 
			}
		};
		r = engine->RegisterObjectBehaviour( "string", asBEHAVE_FACTORY, "string @f(double)", asFUNCTION( helper::objectString_FactoryFromDouble ), asCALL_CDECL ); assert( r >= 0 );
		r = engine->RegisterObjectMethod( "string", "double opImplConv() const", asFUNCTION( helper::objectString_CastToDouble ), asCALL_CDECL_OBJLAST ); assert( r >= 0 );

		//r = engine->RegisterObjectMethod("dictionary", "void set(const string &in, const string &in)", asFUNCTION(0), asCALL_GENERIC);
		//r = engine->RegisterObjectMethod("dictionary", "void get(const string &in, string &out)", asFUNCTION(0), asCALL_GENERIC);

		const char *str = 
			"string gs;                      \n"
			"void Test()                     \n"
			"{                               \n"
			"  dictionary dict;              \n"
			"  dict.set('a', '3.14');        \n" // calls dict.set(string, double &in) since the string has an implicit cast to double
			"  string c;                     \n"
			"  dict.get('a', c);             \n" // calls dict.get(string, double &out) since the string has implicit factory from double
			"  assert(c == double('3.14'));  \n"
			"  dict.get('a', gs);            \n" // calls dict.get(string, double &out) since the string has implicit factory from double
			"  assert(gs == double('3.14')); \n"
			"}                               \n";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", str);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test clean up
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, false);
		engine->RegisterObjectType("ScriptConsoleLine", sizeof(asDWORD), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
		engine->RegisterGlobalProperty("array<ScriptConsoleLine> @m_ScriptConsoleLineArray", (void*)1);

		engine->Release();
	}

	// Test handle assign on class member as array
	// http://www.gamedev.net/topic/652656-problem-with-arrays-when-upgraded-to-228/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, false);
		RegisterStdString(engine);

		engine->RegisterObjectType("ScriptConsoleLine", sizeof(asDWORD), asOBJ_VALUE | asOBJ_APP_CLASS);
		engine->RegisterObjectProperty("ScriptConsoleLine", "array<string>@ m_SA_Strings", 0);
		engine->RegisterObjectMethod("ScriptConsoleLine", "ScriptConsoleLine &opAssign(const ScriptConsoleLine& in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("ScriptConsoleLine", asBEHAVE_CONSTRUCT, "void XEAS_ScriptConsoleLineConstructor()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("ScriptConsoleLine", asBEHAVE_DESTRUCT, "void XEAS_ScriptConsoleLineDestructor()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterGlobalProperty("array<ScriptConsoleLine> @m_ScriptConsoleLineArray", (void*)1);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"void main() { \n"
			"  array<ScriptConsoleLine> crlArr; \n"
			"  array<string> line; \n"
			"  line.insertLast('blah'); \n"
			"  ScriptConsoleLine crl; \n"
			"  @crl.m_SA_Strings = line; \n"
			"  crlArr.insertLast(crl); \n"
			"  @m_ScriptConsoleLineArray = crlArr; \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
			
		engine->Release();
	}

	// Test compiler error with explicit type cast
	// http://www.gamedev.net/topic/649644-assert-when-casting-void-return-value-to-an-object-handle/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterStdString(engine);
		
		bout.buffer = "";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"void main() { \n"
			"	ParticleEmitter@ em = cast<ParticleEmitter>(CreateComponent('ParticleEmitter')); \n"
			"} \n"
			"class ParticleEmitter {} \n"
			"void CreateComponent(const string&in componentName) {} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		
		if( bout.buffer != "Test (1, 1) : Info    : Compiling void main()\n"
						   "Test (2, 24) : Error   : No conversion from 'void' to 'ParticleEmitter@' available.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test warning in implicit constructor call
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->RegisterObjectType("vec2i", 4, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectBehaviour("vec2i", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("vec2i", asBEHAVE_CONSTRUCT, "void f(int,int)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("vec2i", "vec2i opSub(const vec2i &in) const", asFUNCTION(0), asCALL_GENERIC);
		
		bout.buffer = "";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"vec2i f(){ \n"
			"    double f = 3.14; \n"
			"    return vec2i(1,2) - f; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		if( bout.buffer != "Test (1, 1) : Info    : Compiling vec2i f()\n"
						   "Test (3, 25) : Warning : Float value truncated in implicit conversion to integer\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test chained assigned with handles
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"class TestClass{} \n"
			"TestClass @t1; \n"
			"TestClass @t2; \n"
			"void startGame( ){ \n"
			"    TestClass @t = TestClass(); \n"
			"    @t1 = @t2 = t; \n"
			"    assert( t1 is t && t2 is t ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "startGame()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test large integers
	// http://www.gamedev.net/topic/648192-unsigned-int64-wokring-wrong/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		// Don't complain about changing sign, and don't sign extend the value to 64bit
		r = ExecuteString(engine, "uint64 ui64b = 4294967295; assert( (ui64b & 0xFFFFFFFF00000000) == 0 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "uint64 ui64b = 4294967296; assert( ui64b == 0x100000000 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Don't complain about changing signs or losing data
		r = ExecuteString(engine, "uint32 ui32 = 4294967295; assert( ui32 == 0xFFFFFFFF ); ");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "uint64 ui64 = 9223372036854775808; assert( ui64 == 0x8000000000000000 ); ");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "uint64 ui64 = 18446744073709551615; assert( ui64 == 0xFFFFFFFFFFFFFFFF ); ");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";

		// Warn about losing data
		r = ExecuteString(engine, "uint32 ui32b = 4294967296; assert( ui32b == 0 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Warn about change of sign
		r = ExecuteString(engine, "int64 i64 = 9223372036854775808; assert( uint64(i64) == 0x8000000000000000 ); ");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 16) : Warning : Value is too large for data type\n"
			               "ExecuteString (1, 13) : Warning : Implicit conversion changed sign of value\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Don't warn 
		bout.buffer = "";
		r = ExecuteString(engine, "int64 i64 = -9223372036854775808; assert( uint64(i64) == 0x8000000000000000 ); ");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Don't warn 
		bout.buffer = "";
		r = ExecuteString(engine, "const int64 i64 = -9223372036854775808; assert( uint64(i64) == 0x8000000000000000 ); ");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// void is a legal expression and can be used to ignore output parameters
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"void main() { \n"
			"  func(void); \n"
			"  void; \n"
			"} \n"
			"void func(int &out a, int &out b = void) { a = 42; b = 24; } \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// void can't be used with operators
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod->AddScriptSection("Test",
			"void main() { \n"
			"  void + void; \n"
			"  void == void; \n"
			"  void = void; \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "Test (1, 1) : Info    : Compiling void main()\n"
						   "Test (2, 8) : Error   : Void cannot be an operand in expressions\n"
						   "Test (3, 8) : Error   : Void cannot be an operand in expressions\n"
						   "Test (4, 3) : Error   : Expression is not an l-value\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// @null is a legal expression
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = ExecuteString(engine, "@null");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Reported by neorej16
	// Classes with both opAssign and opAddAssign wasn't compiled 
	// properly when returning handle instead of reference
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class gameScript\n"
			"{\n"
			"	gameScript@ opAssign(const gameScript &in other)\n"
			"	{\n"
			"		return this;\n"
			"	}\n"
			"	gameScript@ opAddAssign(const gameScript &in other)\n"
			"	{\n"
			"		return this;\n"
			"	}\n"
			"}\n"
			"gameScript buffer;\n"
			"gameScript cmd;\n"
			"void playerAdded()\n"
			"{\n"
			"	buffer += cmd;\n"
			"}\n");
			
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
			
		engine->Release();
	}
	
	// Reported by Hermann Noll
	// POD types that were not of even 4 bytes wasn't allocated enough size on stack and heap
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		struct Bytes10
		{
			bool a0;
			bool a1;
			bool a2;
			bool a3;
			bool a4;
			bool a5;
			bool a6;
			bool a7;
			bool a8;
			bool a9;
		}; 

		engine->RegisterObjectType ("Bytes10",sizeof(Bytes10),asOBJ_VALUE|asOBJ_POD);
		engine->RegisterObjectProperty ("Bytes10","bool a0",asOFFSET(Bytes10,a0));
		engine->RegisterObjectProperty ("Bytes10","bool a9",asOFFSET(Bytes10,a9));

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"uint before = 0; \n"
			"Bytes10 bytes; \n"
			"uint after = 0; \n"
			"bytes.a0 = true; \n"
			"bytes.a9 = true; \n"
			"assert( after == before ); \n"
			"assert( after == 0 ); \n"
			"after = before = 0xFFFFFFFF; \n"
			"Check(bytes); \n"
			"} \n"
			"void Check(Bytes10 a) { \n"
			"  assert( a.a0 == true ); \n"
			"  assert( a.a9 == true ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class dummy \n"
			"{ \n"
			"  ~dummy() \n"
			"  { \n"
			"  } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  dummy temp; \n"
			// TODO: optimize: The temp string 'Return' is copied twice when calling alert. 
			//                 This is because the function takes the string by value which currently 
			//                 requires the object to be on the heap so it can be destroyed by the 
			//                 called function. This will be modified in the future
			"  alert('Return', test(temp)); \n"
			"} \n"
			"bool test(dummy x) \n"
			"{ \n"
			"  return false; \n" // The return value is put in the register, then the dummy is destroyed
			"} \n"
			"void alert(string txt, bool b) \n"
			"{ \n"
			"  assert( b == false ); \n"
			"  assert( txt == 'Return' ); \n"
			"} \n");
		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();
		engine->Release();
	}

	// Test passing argument to function that returns object
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptMath(engine);
		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class C1 \n"
			"{ \n"
			"    C1 () \n"
			"    { \n"
			"        d_ = 0.0; \n"
			"    } \n"
			"    double d_; \n"
			"} \n"
			"C1 t1; \n"
			"C1 & getC1 (const string &in s) \n"
			"{ \n"
			"    return t1; \n"
			"} \n"
			"void main () \n"
			"{ \n"
			"   for ( int i = 0; i < 3; ++i ) \n"
			"   { \n"
			"       string s;           \n" // case 1.
			"       getC1(s).d_ += 10;  \n" // case 1.
			"	    getC1(string()).d_ += 10; \n" // case 2.
			"   } \n"
			"	assert( abs(t1.d_ - 60.0) <= 0.0001 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();
		engine->Release();
	}

	// Test assignment operator
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class C1\n"
			"{\n"
			"    C1 () {} \n"
			"    C1 & opAssign (const C1 &in other)\n"
			"    {\n"
			"        value = other.value; \n"
			"        return this; \n"
			"    }\n"
			"    int value = 0; \n"
			"}\n"
			"void main (void)\n"
			"{\n"
			"    array<C1> t; \n"
			"    t.insertLast(C1()); \n"
			"    t[0].value = 42; \n"
			"    t.insertLast(t[0]); \n" // should make a copy
			"    assert( t[0].value == t[1].value ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();
		engine->Release();
	}

	// Test that the object variable scopes in the function bytecode is correctly set
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"   bool TRUE = true, FALSE = false; \n"
			"   if( FALSE ) \n"
			"   { \n"
			"	  string temp=''; \n"
			"	  while( TRUE ) \n"
			"	  { \n"
			"		if( FALSE ) \n"
			"		{ \n"
			"			continue; \n"
			"		} \n"
			"  		break; \n"
			"  	  } \n"
			"   } \n"
			"   int a = 0; a = a/a; \n" // provoke exception by dividing with 0
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "func()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		ctx->Release();
		engine->Release();
	}

	// Test a null pointer exception reported by Robert Weitzel
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "int global = 1;");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		engine->DiscardModule("test");

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "enum A { a = 2 }");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, true);
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		// It shall not be possible to do a value assign
		bout.buffer = "";
		mod->AddScriptSection("test",
			"class T { T() {} T &opAssign(const T &in) { return this; } T &opAddAssign(const T &in) { return this; } } \n"
			"void main() { \n"
			"  T t; \n"
			"  t = T(); \n" // fail
			"  t += T(); \n" // fail
			"  @t = T(); \n" // fail (because the variable is not a handle)
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (2, 1) : Info    : Compiling void main()\n"
						   "test (4, 5) : Error   : Value assignment on reference types is not allowed. Did you mean to do a handle assignment?\n"
						   "test (5, 5) : Error   : Compound assignment on reference types is not allowed\n"
						   "test (6, 3) : Error   : Expression is not an l-value\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// It shall not be possible to declare function that take ref type by value
		bout.buffer = "";
		mod->AddScriptSection("test",
			"class T { T() {} T &opAssign(const T &in) { return this; } } \n"
			"void func1(T t) {} \n" // fail
			"T func2() { return T(); } \n" // fail
			"void func3(T &t) {} \n" // ok
			"T g; \n"
			"T &func4() { return g; } \n" // ok
			"void func5(T @t) {} \n" // ok
			"T @func6() { return null; } \n" // ok
			);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (2, 1) : Error   : Reference types cannot be passed by value in function parameters\n"
                           "test (3, 1) : Error   : Reference types cannot be returned by value from functions\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// The array add-on should only accept handles for the sub type. This should be controlled by the template callback
		RegisterScriptArray(engine, false);
		bout.buffer = "";
		mod->AddScriptSection("Test",
			"class C {} \n"
			"array<C> arr1; \n" // fail
			"array<C@> arr2; \n" // ok
			);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "array (0, 0) : Error   : The subtype has no default factory\n"
						   "Test (2, 7) : Error   : Attempting to instantiate invalid template type 'array<C>'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// When passing a ref type to a ? parameter, the compiler should assume it was meant to send the handle
		RegisterScriptAny(engine);
		bout.buffer = "";

		CScriptAny *any = new CScriptAny(engine);
		engine->RegisterGlobalProperty("any a", any);

		mod->AddScriptSection("Test", "class C {} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		r = ExecuteString(engine, "a.store(C())", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( (any->GetTypeId() & asTYPEID_OBJHANDLE) == 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "a.store(@C())", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( (any->GetTypeId() & asTYPEID_OBJHANDLE) == 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "C c; a.store(c)", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( (any->GetTypeId() & asTYPEID_OBJHANDLE) == 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		any->Release();
		engine->Release();
	}

	// Test opAssign that returns void
	// It was popping a word too many. Reported by Andrew Ackermann
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class T { \n"
			"  void opAssign(const T &in v) {} \n"
			"} \n"
			"void main() { \n"
			"  T t = T(); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_CALL,asBC_STOREOBJ,asBC_ChkNullV,asBC_VAR,asBC_CALL,asBC_STOREOBJ,asBC_PshVPtr,asBC_GETOBJREF,asBC_CALLINTF,asBC_FREE,
				asBC_SUSPEND,asBC_FREE,asBC_RET
			};
		asIScriptFunction *func = mod->GetFunctionByName("main");
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;

		engine->Release();
	}

	// Test
	// http://www.gamedev.net/topic/640966-returning-text-crashes-as-with-mingw-471-but-not-with-441/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

#ifdef AS_CAN_USE_CPP11
        if( asGetTypeTraits<A>() != asOBJ_APP_CLASS_CDAK )
            TEST_FAILED;
#endif

		r = engine->RegisterObjectType("A", sizeof(A), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("A", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(A::Constructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("A", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(A::Destructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("A", "string getText()", asMETHOD(A,getText), asCALL_THISCALL);assert( r >= 0 );
		r = engine->RegisterObjectMethod("A", "A getA()", asMETHOD(A,getA), asCALL_THISCALL);assert( r >= 0 );

		r = ExecuteString(engine, "A a; \n"
			                      "string text = a.getA().getText(); \n"
								  "assert( text == 'hello' ); \n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test that integer constants are signed by default
	// http://www.gamedev.net/topic/625735-bizarre-errata-with-ternaries-and-integer-literals/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		g_printbuf = "";
		r = ExecuteString(engine, "float a = ((false?1:0)-(true?1:0)); print('' + a); a += 1; assert( a < 0.005 && a > -0.005 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test warnings as error
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 2);

		bout.buffer = "";

		r = ExecuteString(engine, "uint a; a = -12;");
		if( r >= 0  )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 13) : Warning : Implicit conversion changed sign of value\n"
		                   " (0, 0) : Error   : Warnings are treated as errors by the application\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test check for division -2147483648 by -1
	// http://www.gamedev.net/topic/639703-crash-in-divmod-implementations/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		r = ExecuteString(engine, "int int_min = -2147483648;\n"
								  "int neg_one = -1;\n"
								  "int never_computed = (int_min / neg_one);\n");
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		r = ExecuteString(engine, "int int_min = -2147483648;\n"
								  "int neg_one = -1;\n"
								  "int the_same_error = (int_min % neg_one);\n");
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		r = ExecuteString(engine, "int int_min = -2147483648;\n"
								  "int neg_one = -1;\n"
								  "uint the_same_error = (uint(int_min) / uint(neg_one));\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "int never_computed = (-2147483648 / -1);\n"); // constants will be treated as 64bit so the division is fine
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 22) : Warning : Value is too large for data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		bout.buffer = "";

		r = ExecuteString(engine, "int never_computed = (-2147483648 % -1);\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "int64 int_min = -9223372036854775808;\n"
								  "int64 neg_one = -1;\n"
								  "int64 never_computed = (int_min / neg_one);\n");
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		r = ExecuteString(engine, "int64 int_min = -9223372036854775808;\n"
								  "int64 neg_one = -1;\n"
								  "int64 the_same_error = (int_min % neg_one);\n");
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		r = ExecuteString(engine, "int64 never_computed = (-9223372036854775808 / -1);\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 0);
		r = ExecuteString(engine, "int64 never_computed = (-9223372036854775808 % -1);\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test crash in compiler
	// http://www.gamedev.net/topic/639248-compilation-crash-possibly-on-error-output/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"import void g(bool dummy, int x = -1) from 'mod2'; \n"
			"void f(bool dummy, int x) \n"
			"{ \n"
			"} \n"
			"void run() \n"
			"{ \n"
			"    f(true, 0); \n"
			"    f(true, 0); \n"
			"    g(false); \n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"import void run() from 'mod1'; \n"
			"void g(bool dummy, int x = -1) \n"
			"{ \n"
			"} \n"
			"class T \n"
			"{ \n"
			"    T() \n"
			"    { \n"
			"        run(); \n"
			"    } \n"
			"}; \n"
			"T Dummy; \n");
		r = mod->Build();
		if( r != asINIT_GLOBAL_VARS_FAILED )
			TEST_FAILED;

		if( bout.buffer != "test (12, 3) : Error   : Failed to initialize global variable 'Dummy'\n"
		                   "test (10, 0) : Info    : Exception 'Unbound function called' in 'T::T()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test compile error
	// http://www.gamedev.net/topic/637772-small-compiller-bug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("test", "int f() {return 0;;}");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		r = mod->AddScriptSection("test", "class A\n"
			"{ \n"
			"    int a; ; \n"
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

	// Test compiler error
	// http://www.gamedev.net/topic/638128-bug-with-show-code-line-after-null-pointer-exception-and-for/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("test",
			"void F()\n"
			"{\n"
			"    int a1;\n"
			"    int a2;\n"
			"    int a3;\n"
			"    int a4;\n"
			"    int a5;\n"
			"    F()   '  ;\n"
			"}\n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (8, 11) : Error   : Non-terminated string literal\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test compiling an empty script
	// Reported by Damien French
	{
		asResetGlobalMemoryFunctions();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("test", "");
		if( r < 0 )
			TEST_FAILED;
		r = mod->AddScriptSection("test2", 0);
		if( r != asINVALID_ARG )
			TEST_FAILED;
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Nothing was built in the module\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();

		InstallMemoryManager();
	}

	// Problem reported by Paril101
	// http://www.gamedev.net/topic/636336-member-function-chaining/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->RegisterObjectType("ChainMe", sizeof(ChainMe), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C);
		engine->RegisterObjectBehaviour("ChainMe", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ChainMe::Construct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("ChainMe", "ChainMe &Increase(const int &in)", asMETHOD(ChainMe, Increase), asCALL_THISCALL);
		engine->RegisterObjectProperty("ChainMe", "int x", asOFFSET(ChainMe, X));

		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"  func2(ChainMe().Increase(5).Increase(15).Increase(25)); \n"
			"} \n"
			"void func2(const ChainMe &in a) { \n"
			"  assert( a.x == 45 ); \n"
			"} \n");


		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Problem reported by zerochen
	// http://www.gamedev.net/topic/634768-after-unreachable-code-wrong-error-msg/
	{
		const char *script =
			"void dum() {} \n"
			"int dummy() \n"
			"{ \n"
			"  return 0; \n"
			"  dum(); \n"
			"  //return 1; \n"  // Compiler shouldn't complain about paths that don't return
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);

		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "test (2, 1) : Info    : Compiling int dummy()\n"
		                   "test (5, 3) : Warning : Unreachable code\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by _Engine_
	// http://www.gamedev.net/topic/632922-huge-problems-with-precompilde-byte-code/
	{
		const char *script =
			"class SBuilding \n"
			"{ \n"
			"	void ReleasePeople() \n"
			"	{ \n"
			"		SPoint cellij; \n"
			"		if( GetRoadOrFreeCellInAround(cellij) ) {} \n"
			"	} \n"
			"	bool GetRoadOrFreeCellInAround(SPoint&out cellij) \n"
			"	{ \n"
			"		return false; \n"
			"	} \n"
			"} \n"
			"shared class SPoint \n"
			"{ \n"
			"	SPoint@ opAssign(const SPoint&in assign) \n"
			"	{ \n"
			"		return this; \n"
			"	}  \n"
			"} \n";


		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Problem reported by FDsagizi
	// http://www.gamedev.net/topic/632813-compiller-bug/
	{
		const char *script =
			"Test @cur_test; \n"
			"class Test { \n"
			"  void Do() { \n"
			"    cur_test.DoFail(); \n"
			"  } \n"
			"} \n"
			"void DoFail() {} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (3, 3) : Info    : Compiling void Test::Do()\n"
		                   "test (4, 14) : Error   : No matching symbol 'DoFail'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by FDsagizi
	// http://www.gamedev.net/topic/632123-compiler-assertion/
	{
		const char *script =
			"void startGame() \n"
			"{ \n"
			"		 array<int> arr; \n"
			"		 string s; \n"
			"		 s +- \n"
			"		 arr.insertLast( 1 ); \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, false);
		RegisterStdString(engine);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void startGame()\n"
		                   "test (5, 7) : Error   : Illegal operation on this datatype\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by Polyak Istvan
	{
		const char *script =
			"class X1 {} \n"
			"class X2 \n"
			"{ \n"
			"    const X1 @ f1 (void) \n"
			"    { \n"
			"        return x1_; \n" // ok
			"    } \n"
			"    const X1 & f2 (void) const \n"
			"    { \n"
			"        return x1_; \n" // ok
			"    } \n"
			"    const X1 & f3 (void) \n"
			"    { \n"
			"        return x1_; \n" // ok
			"    } \n"
			"    const int & f4 (void) \n"
			"    { \n"
			"        return i1_; \n" // ok
			"    } \n"
			"    int & f5 (void) const \n"
			"    { \n"
			"        return i1_; \n" // should fail
			"    } \n"
			"	 X1 & f6 (void) const \n"
			"    { \n"
			"        return x1_; \n" // should fail
			"    } \n"
			"    private X1 x1_; \n"
			"    private int i1_; \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (20, 5) : Info    : Compiling int& X2::f5() const\n"
						   "test (22, 9) : Error   : Can't implicitly convert from 'const int&' to 'int&'.\n"
						   "test (24, 3) : Info    : Compiling X1& X2::f6() const\n"
						   "test (26, 9) : Error   : Can't implicitly convert from 'const X1' to 'X1&'.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by Andrew Ackermann
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterStdString(engine);
		RegisterScriptArray(engine, true);

#ifndef AS_MAX_PORTABILITY
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
#else
		engine->RegisterGlobalFunction("void print(const string &in)", WRAP_FN(Print), asCALL_GENERIC);
#endif

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test",
			"class Data {\n"
			"	int x;\n"
			"	Data() {\n"
			"		print('create Data()\\n');\n"
			"	}\n"
			"	~Data() {\n"
			"		print('delete Data()\\n');\n"
			"	}\n"
			"	Data& opAssign(const Data&in other) {\n"
			"		x = other.x;\n"
			"		return this;\n"
			"	}\n"
			"};\n"
			"Data a;\n"
			"Data b;\n"
			"void TestCopyGlobals() {\n"
			"	Data c;\n"
			"	print('--a = b--\\n');\n"
			"	a = b; //Implicitly creates and then deletes a temporary copy\n"
			"	print('--a = c--\\n');\n"
			"	a = c; //Does not create a temporary copy\n"
			"	print('--end--\\n');\n"
			"}\n" );

		g_printbuf = "";

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "TestCopyGlobals()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( g_printbuf != "create Data()\n"
		                  "create Data()\n"
		                  "create Data()\n"
		                  "--a = b--\n"
		                  "--a = c--\n"
		                  "--end--\n"
		                  "delete Data()\n" )
		{
			PRINTF("%s", g_printbuf.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterStdString(engine);
		RegisterScriptArray(engine, true);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test",
			"int DATE_YEAR { get { return 2012; } } \n"
			"void alert( string t, string v ) { assert( v == '2012' ); } \n"
			"void main() \n"
			"{ \n"
			"  int[] dates(5); \n"
			"  alert('Year', '' + DATE_YEAR);   \n"
			"  dates[3]=DATE_YEAR; \n" // This was storing 3 in the array
			"  alert('Assigned year', '' + dates[3]); \n"
			"} \n" );

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Problem reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterStdString(engine);
		RegisterScriptArray(engine, true);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test",
			"string[] get_list() \n"
			"{ \n"
			"  string[]@ null_handle; \n"
			"  return null_handle; \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  string[] result=get_list(); \n"
			"} \n");

//		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		if( string(ctx->GetExceptionString()) != "Null pointer access" )
			TEST_FAILED;
		if( string(ctx->GetExceptionFunction()->GetName()) != "get_list" )
			TEST_FAILED;

		ctx->Release();
		engine->Release();
	}

	// Problem reported by _Vicious_
	// http://www.gamedev.net/topic/625747-multiple-matching-signatures-to/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptString(engine);

		bout.buffer = "";

		engine->RegisterObjectType("Cvar", sizeof(int), asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectBehaviour("Cvar", asBEHAVE_CONSTRUCT, "void f(const string &in, const string &in, const uint)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Cvar", "void set(string&in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Cvar", "void set(float)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Cvar", "void set(int)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Cvar", "void set(double)", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("ElementFormControl", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("ElementFormControl", "string@ get_value() const property", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("ElementFormControl", "void set_value(const string&in) property", asFUNCTION(0), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() \n"
			"{ \n"
			"  ElementFormControl @login_form_username; \n"
			"  Cvar mm_user( 'cl_mm_user', '', 0 ); \n"
			"  mm_user.set( login_form_username.value ); \n"
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

	// Problem reported by Ricky C
	// http://www.gamedev.net/topic/625484-c99-hexfloats/#entry4943881
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		// Parsing a C99 hex float constant doesn't give error
		// TODO: Maybe one day I'll implement support for this form of float constants
		r = ExecuteString(engine, "float v = 0x219AEFp-24;\n"
								  "v = 0x219AEFp-24;\n"
								  "if( v == 0x219AEFp-24 ) {}\n");
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "ExecuteString (1, 19) : Error   : Expected ',' or ';'\n"
						   "ExecuteString (1, 19) : Error   : Instead found identifier 'p'\n"
						   "ExecuteString (2, 13) : Error   : Expected ';'\n"
						   "ExecuteString (2, 13) : Error   : Instead found identifier 'p'\n"
						   "ExecuteString (3, 18) : Error   : Expected ')'\n"
						   "ExecuteString (3, 18) : Error   : Instead found identifier 'p'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}


	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		bout.buffer = "";

		const char *script1 =
			"void testFunction ()                          \n"
			"{                                             \n"
			" Assert@ assertReached = tryToAvoidMeLeak();  \n"
			"}                                             \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (1, 1) : Info    : Compiling void testFunction()\n"
						   "TestCompiler (3, 2) : Error   : Identifier 'Assert' is not a data type\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by ekimr
	{
		const char *script =
			"class END_MenuItem : Widget \n"
			"{ \n"
			"	END_MenuItem() \n"
			"	{ \n"
			"		//super(null); \n"
			"	} \n"
			"}; \n"
			"class Widget \n"
			"{ \n"
			"	Widget( Widget@ parent = null ) \n"
			"	{ \n"
			"	} \n"
			"}; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptArray(engine, true);
		RegisterScriptString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "TestCompiler (3, 2) : Info    : Compiling END_MenuItem::END_MenuItem()\n"
						   "TestCompiler (4, 2) : Error   : Base class doesn't have default constructor. Make explicit call to base constructor\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Problem reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		// Special string class
		r = engine->RegisterObjectType("string", sizeof(std::string), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
		r = engine->RegisterStringFactory("string", &stringFactory);
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructStringGeneric),  asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asFUNCTION(AssignStringGeneric),    asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "string get_opIndex(uint) const property", asFUNCTION(String_get_opIndexGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "void set_opIndex(uint, const string &in) property", asFUNCTION(String_set_opIndexGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "string opAdd(int) const", asFUNCTION(AddString2IntGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTION(StringAddGeneric), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterGlobalFunction("void alert(string &in, string &in)", asFUNCTION(AlertGeneric), asCALL_GENERIC); assert( r >= 0 );

		// This script should not compile, because true cannot be passed to int& in
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"void string_contains_bulk(string the_string, string the_bulk)\n"
			"{\n"
			"  string_contains(the_bulk[0], true);\n"
			"}\n"
			"void string_contains(string& in, int& in) {} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (1, 1) : Info    : Compiling void string_contains_bulk(string, string)\n"
						   "TestCompiler (3, 3) : Error   : No matching signatures to 'string_contains(const string&, const bool)'\n"
						   "TestCompiler (3, 3) : Info    : Candidates are:\n"
						   "TestCompiler (3, 3) : Info    : void string_contains(string&in, int&in)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// This script should correctly return the strings
		mod->AddScriptSection(TESTNAME,
			"void main() \n"
			"{ \n"
			"	string test='food'; \n"
			"	test[0]='g'; \n"
			"	for(uint i=0;i<test.length();i++) \n"
			"	  alert('Character ' + (i+1), test[i]); \n"
			"} \n");

		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( alert_buf != "Character 1g\n"
			             "Character 2o\n"
						 "Character 3o\n"
						 "Character 4d\n" )
		{
			PRINTF("%s", alert_buf.c_str());
			TEST_FAILED;
		}

		// This script must show a compiler error
		// Reported by Aaron Baker
		bout.buffer = "";
		mod->AddScriptSection("test",
			"void main() \n"
			"{ \n"
			"	string str; \n"
			"	string t = str[]; \n"
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test (1, 1) : Info    : Compiling void main()\n"
						   "test (4, 16) : Error   : Property accessor with index must have 1 and only 1 index argument\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// http://www.gamedev.net/topic/623846-asccompiler-with-out-asasserts-in-debug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		const char *script =
			"class AAA \n"
			"{ \n"
			"  Car @car; \n"
			"  void Update() \n"
			"  { \n"
			"    if( car !is null ) \n"
			"      car.Update(); \n"
			"  } \n"
			"} \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (3, 3) : Error   : Identifier 'Car' is not a data type in global namespace\n"
						/*   "TestCompiler (4, 3) : Info    : Compiling void AAA::Update()\n"
						   "TestCompiler (6, 13) : Error   : Both operands must be handles when comparing identity\n"
						   "TestCompiler (7, 10) : Error   : Illegal operation on 'int&'\n" */ )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		mod->AddScriptSection(TESTNAME,
			"class A{} \n"
			"class SomeClass \n"
			"{ \n"
			"         A @a; \n"
			"         void Create() \n"
			"         { \n"
			"                  int some_val = 15; + \n"
			"                  @a = A();\n"
			"         }\n"
			"}\n");

		bout.buffer = "";
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (5, 10) : Info    : Compiling void SomeClass::Create()\n"
						   "TestCompiler (7, 38) : Error   : Illegal operation on this datatype\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// http://www.gamedev.net/topic/623880-crash-after-get-property-of-null-class-in-function/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		const char *script =
			"class BugClass \n"
			"{ \n"
			"         int ID; \n"
			"} \n"
			"void CallBug( BugClass @bc ) \n"
			"{ \n"
			"         int id = bc.ID; \n"
			"} \n"
			"void startGame() \n"
			"{ \n"
			"         CallBug( null ); \n"
			"} \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "startGame()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION || string(ctx->GetExceptionString()) != "Null pointer access" || string(ctx->GetExceptionFunction()->GetName()) != "CallBug" )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, "void f(){\n  int a;\n  a(0)=0;}");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (1, 1) : Info    : Compiling void f()\n"
		                   "TestCompiler (3, 3) : Error   : Expression doesn't form a function call. 'a' evaluates to the non-function type 'int'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		// When passing 'null' or 'void' to an output parameter the compiler shouldn't warn
		const char *script = "class C {} void func(C @&out) {} \n"
			                 "void main() { \n"
							 "  bool f = true; \n"
							 "  if( f ) \n"
							 "  { \n"
							 "    func(void); \n" // ok. output will be discarded
							 "    func(null); \n" // ok. output will be discarded
							 "  } \n"
							 "  else \n"
							 "    func(C()); \n" // error. not assignable
	                         "}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestCompiler (2, 1) : Info    : Compiling void main()\n"
		                   "TestCompiler (10, 10) : Error   : Output argument expression is not assignable\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		// When passing a double constant to a float arg the compiler shouldn't warn if the value doesn't loose precision
		const char *script = "void func(float) {} \n"
			                 "void main() { \n"
							 "  func(0.3); \n"
	                         "}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
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


	{
		const char *script = "class Mind {} \n"
							 "class TA_VehicleInfo {} \n"
							 "class TA_Mind : Mind \n"
		                     "{ \n"
							 "  TA_Mind(TA_VehicleInfo@ vi) \n"
		                     "  { \n"
							 "    VehicleInfo = vi; \n" // script writer did a value assign by mistake
							 "  } \n"
							 "  TA_VehicleInfo@ get_VehicleInfo() const property { return m_VehicleInfo; } \n"
							 "  void set_VehicleInfo(TA_VehicleInfo@ info) property { @m_VehicleInfo = @info; } \n"
							 "  private TA_VehicleInfo@ m_VehicleInfo; \n"
	                         "}; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "TA_VehicleInfo vi; TA_Mind m(vi); \n", mod);
		if( r != asEXECUTION_EXCEPTION )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		// test 2
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		const char *script2 =
			"void CompilerAssert()\n"
			"{\n"
			"   bool x = 0x0000000000000000;\n"
			"   bool y = 1;\n"
			"   x+y;\n"
			"}";

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "TestCompiler (1, 1) : Info    : Compiling void CompilerAssert()\n"
			"TestCompiler (3, 13) : Error   : Can't implicitly convert from 'uint' to 'bool'.\n"
			"TestCompiler (4, 13) : Error   : Can't implicitly convert from 'int' to 'bool'.\n"
			"TestCompiler (5, 5) : Error   : No conversion from 'bool' to math type available.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// test 3
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script3 = "void CompilerAssert(uint8[]@ &in b) { b[0] == 1; }";
		mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		// test 4
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script4 = "class C : I {};";
		mod->AddScriptSection(TESTNAME, script4, strlen(script4), 0);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "TestCompiler (1, 11) : Error   : Identifier 'I' is not a data type in global namespace\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script5 =
			"void t() {} \n"
			"void crash() { bool b = t(); } \n";
		mod->AddScriptSection(TESTNAME, script5, strlen(script5), 0);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "TestCompiler (2, 1) : Info    : Compiling void crash()\n"
			"TestCompiler (2, 25) : Error   : Can't implicitly convert from 'void' to 'bool'.\n")
			TEST_FAILED;

		// test 6
		// Verify that script class methods can have the same signature as
		// globally registered functions since they are in different scope
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("bool Test(bool, float)", asFUNCTION(0), asCALL_GENERIC);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script6 = "class t { bool Test(bool, float) {return false;} }";
		mod->AddScriptSection(TESTNAME, script6, strlen(script6), 0);
		r = mod->Build();
		if (r < 0)
		{
			PRINTF("failed on 6\n");
			TEST_FAILED;
		}

		// test 7
		// Verify that declaring a void variable in script causes a compiler error, not an assert failure
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		ExecuteString(engine, "void m;");
		if (bout.buffer != "ExecuteString (1, 6) : Error   : Data type can't be 'void'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// test 8
		// Don't assert on implicit conversion to object when a compile error has occurred
		const char *script7 =
			"class Ship                           \n"
			"{                                    \n"
			"	Sprite		_sprite;              \n"
			"									  \n"
			"	string GetName() {                \n"
			"		return _sprite.GetName();     \n"
			"	}								  \n"
			"} \n";

		bout.buffer = "";
		RegisterStdString(engine);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script7, strlen(script7));
		r = mod->Build();
		if (r >= 0)
		{
			TEST_FAILED;
		}
		if (bout.buffer != "script (3, 2) : Error   : Identifier 'Sprite' is not a data type in global namespace\n"
			/*"script (5, 2) : Info    : Compiling string Ship::GetName()\n"
			"script (6, 17) : Error   : Illegal operation on 'int&'\n"*/)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// test 9
		// Don't hang on script with non-terminated string
		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script8 =
			"void Print(const string &in) {} \n"
			"float calc(float x, float y) { Print(\"GOT THESE NUMBERS: \" + x + \", \" + y + \"\n\"); return x*y; }";
		mod->AddScriptSection("script", script8, strlen(script8));
		r = mod->Build();
		if (r >= 0)
		{
			TEST_FAILED;
		}
		if (bout.buffer != "script (2, 1) : Info    : Compiling float calc(float, float)\n"
			"script (2, 77) : Error   : Multiline strings are not allowed in this application\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// test 10
		// Properly handle error with returning a void expression
		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script9 =
			"void noop() {}\n"
			"int fuzzy() {\n"
			"  return noop();\n"
			"}\n";
		mod->AddScriptSection("script", script9, strlen(script9));
		r = mod->Build();
		if (r >= 0)
		{
			TEST_FAILED;
		}
		if (bout.buffer != "script (2, 1) : Info    : Compiling int fuzzy()\n"
			"script (3, 3) : Error   : No conversion from 'void' to 'int' available.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// test 11
		// Properly handle error when assigning a void expression to a variable
		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script10 =
			"void func() {}\n"
			"void test() { int v; v = func(); }\n";
		mod->AddScriptSection("script", script10, strlen(script10));
		r = mod->Build();
		if (r >= 0)
		{
			TEST_FAILED;
		}
		if (bout.buffer != "script (2, 1) : Info    : Compiling void test()\n"
			"script (2, 26) : Error   : Can't implicitly convert from 'void' to 'int'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 12
		// Handle errors after use of undefined objects
		bout.buffer = "";
		engine->RegisterObjectType("type", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script11 =
			"class c                                       \n"
			"{                                             \n"
			"  object @obj;                                \n"
			"  void func()                                 \n"
			"  {type str = obj.GetTypeHandle();}           \n"
			"}                                             \n";
		mod->AddScriptSection("script", script11, strlen(script11));
		r = mod->Build();
		if (r >= 0)
		{
			TEST_FAILED;
		}
		if (bout.buffer != "script (3, 3) : Error   : Identifier 'object' is not a data type in global namespace\n"
			/*"script (4, 3) : Info    : Compiling void c::func()\n"
			"script (5, 18) : Error   : Illegal operation on 'int&'\n"*/)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 13
		// Don't permit implicit conversion of integer to obj even though obj(int) is a possible constructor
		bout.buffer = "";
		r = ExecuteString(engine, "uint32[] a = 0;");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 14) : Error   : Can't implicitly convert from 'const int' to 'uint[]&'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 14
		// Calling void function in ternary operator ?:
		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script12 =
			"void f()       \n"
			"{}             \n"
			"               \n"
			"void assert()  \n"
			"{              \n"
			"   2<3?f():1;  \n"
			"}              \n";
		r = mod->AddScriptSection("script", script12, strlen(script12));
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "script (4, 1) : Info    : Compiling void assert()\n"
			"script (6, 4) : Error   : Can't find unambiguous implicit conversion to make both expressions have the same type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 15
		// Declaring a class inside a function
		bout.buffer = "";
		r = ExecuteString(engine, "class XXX { int a; }; XXX b;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 1) : Error   : Expected expression value\n"
			"ExecuteString (1, 1) : Error   : Instead found reserved keyword 'class'\n"
			"ExecuteString (1, 23) : Error   : Identifier 'XXX' is not a data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 16
		// Compiler should warn if uninitialized variable is used to index an array
		bout.buffer = "";
		const char *script_16 = "void func() { int[] a(1); int b; a[b] = 0; }";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script_16, strlen(script_16));
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		if (bout.buffer != "script (1, 1) : Info    : Compiling void func()\n"
			"script (1, 36) : Warning : 'b' is not initialized.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 17
		// Compiler should warn if uninitialized variable is used with post increment operator
		bout.buffer = "";
		const char *script_17 = "void func() { int a; a++; }";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script_17, strlen(script_17));
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		if (bout.buffer != "script (1, 1) : Info    : Compiling void func()\n"
			"script (1, 23) : Warning : 'a' is not initialized.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 18
		// Properly notify the error of comparing boolean operands
		bout.buffer = "";
		r = ExecuteString(engine, "bool b1,b2; if( b1 <= b2 ) {}");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 20) : Warning : 'b1' is not initialized.\n"
			"ExecuteString (1, 20) : Warning : 'b2' is not initialized.\n"
			"ExecuteString (1, 20) : Error   : Illegal operation on this datatype\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 19 - moved to test_scriptretref

		// Test 20
		// Don't crash on invalid script code
		bout.buffer = "";
		const char *script20 =
			"class A { A @b; } \n"
			"void test()       \n"
			"{ A a; if( @a.b == a.GetClient() ) {} } \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script20", script20, strlen(script20));
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "script20 (2, 1) : Info    : Compiling void test()\n"
			"script20 (3, 22) : Error   : No matching symbol 'GetClient'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 21
		// Don't crash on undefined variable
		bout.buffer = "";
		const char *script21 =
			"bool MyCFunction() {return true;} \n"
			"void main() \n"
			"{ \n"
			"	if (true and MyCFunction( SomethingUndefined )) \n"
			"	{ \n"
			"	} \n"
			"} \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script21", script21, strlen(script21));
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "script21 (2, 1) : Info    : Compiling void main()\n"
			"script21 (4, 28) : Error   : No matching symbol 'SomethingUndefined'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 22
		bout.buffer = "";
		const char *script22 =
			"class Some{} \n"
			"void Func(Some@ some) \n"
			"{ \n"
			"if( some is null) return; \n"
			"Func_(null); \n"
			"} \n"
			"void Func_(uint i) \n"
			"{ \n"
			"} \n";

		r = mod->AddScriptSection("22", script22);
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "22 (2, 1) : Info    : Compiling void Func(Some@)\n"
			"22 (5, 1) : Error   : No matching signatures to 'Func_(<null handle>)'\n"
			"22 (5, 1) : Info    : Candidates are:\n"
			"22 (5, 1) : Info    : void Func_(uint i)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 23 - don't assert on invalid condition expression
		bout.buffer = "";
		const char *script23 = "openHandle.IsValid() ? 1 : 0\n";

		r = ExecuteString(engine, script23);
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 1) : Error   : No matching symbol 'openHandle'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 24 - don't assert on invalid return statement
		bout.buffer = "";
		const char *script24 = "string SomeFunc() { return null; }";
		r = mod->AddScriptSection("24", script24);
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "24 (1, 1) : Info    : Compiling string SomeFunc()\n"
			"24 (1, 28) : Error   : Can't implicitly convert from '<null handle>' to 'string'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test 25 - moved to test_scriptretref

		// Test 26 - don't crash on invalid script
		bout.buffer = "";
		const char *script26 = "void main() { main(anyWord)+main(anyWord); }";
		r = mod->AddScriptSection("26", script26);
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "26 (1, 1) : Info    : Compiling void main()\n"
			"26 (1, 20) : Error   : No matching symbol 'anyWord'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test 27 - don't crash on missing behaviours
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		// We don't register the assignment behaviour
		r = engine->RegisterObjectType("derp", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("derp", asBEHAVE_FACTORY,    "derp@ f()",                 asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("derp", asBEHAVE_FACTORY,    "derp@ f(int &in)",          asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("derp", asBEHAVE_FACTORY,    "derp@ f(const derp &in)",   asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("derp", asBEHAVE_ADDREF,     "void f()",                  asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("derp", asBEHAVE_RELEASE,    "void f()",                  asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "derp wtf = 32;");
		r = mod->Build();
		if( r >= 0 || bout.buffer != "test (1, 6) : Info    : Compiling derp wtf\n"
		                             "test (1, 6) : Error   : No appropriate opAssign method found in 'derp' for value assignment\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test 28 - if with empty statement should give error
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		r = ExecuteString(engine, "if(true); if(true) {} else;");
		if( r >= 0 )
		{
			TEST_FAILED;
		}

		if( bout.buffer != "ExecuteString (1, 9) : Error   : If with empty statement\n"
			               "ExecuteString (1, 27) : Error   : Else with empty statement\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test 29 - function overloads with multiple matches must display matches
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		const char *script = "int func() { return 0; }\n"
			                 "float func() { return 0; }\n"
							 "void main() { func(); }\n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("s", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "s (2, 1) : Error   : A function with the same name and parameters already exists\n"
		                 /*  "s (3, 1) : Info    : Compiling void main()\n"
		                   "s (3, 15) : Error   : Multiple matching signatures to 'func()'\n"
		                   "s (3, 15) : Info    : int func()\n"
		                   "s (3, 15) : Info    : float func()\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test 30 - proper handling of incorrect script
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptString(engine);

		const char *script = "void main() \n"
							 "{ \n"
							 "  string t = string(ti); \n" //ti is undefined
							 "} \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("s", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "s (1, 1) : Info    : Compiling void main()\n"
						   "s (3, 21) : Error   : No matching symbol 'ti'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test to make sure compilation error is properly handled
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptString_Generic(engine);

		r = ExecuteString(engine, "MissingFunction('test')");
		if( r >= 0 )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() succeeded even though it shouldn't\n", TESTNAME);
		}

		engine->Release();
	}

	// Give useful error message when no matching function is found
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"void test(int a) { }        \n"
			"void test(float a) { }      \n"
			"void test(bool c) { }       \n"
			"class Test {                \n"
			"    void test(int a) { }    \n"
			"    void test(float a) { }  \n"
			"    void test(bool c) { }   \n"
			"}                           \n"
			"void main() {               \n"
			"    test();                 \n"
			"    Test test;              \n"
			"    test.test();            \n"
			"}                           \n";

		mod->AddScriptSection(0, script);

		bout.buffer = "";
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (9, 1) : Info    : Compiling void main()\n"
						   " (10, 5) : Error   : No matching signatures to 'test()'\n"
						   " (10, 5) : Info    : Candidates are:\n"
						   " (10, 5) : Info    : void test(int a)\n"
						   " (10, 5) : Info    : void test(float a)\n"
					   	   " (10, 5) : Info    : void test(bool c)\n"
						   " (12, 10) : Error   : No matching signatures to 'Test::test()'\n"
						   " (12, 10) : Info    : Candidates are:\n"
						   " (12, 10) : Info    : void Test::test(int a)\n"
						   " (12, 10) : Info    : void Test::test(float a)\n"
						   " (12, 10) : Info    : void Test::test(bool c)\n" )
		{
			TEST_FAILED;
			PRINTF("%s", bout.buffer.c_str());
		}

		engine->Release();
	}

	//
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		RegisterScriptString(engine);

		engine->RegisterObjectType("sound", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_FACTORY, "sound @f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("sound", "bool get_playing() property", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("sound", "int get_count() property", asFUNCTION(0), asCALL_GENERIC);

		const char *script = "void main() \n"
							 "{ \n"
							 "  sound s; \n"
							 "  for(;s.playing;) {}\n"
							 "  while(s.playing) {} \n"
							 "  do {} while (s.playing); \n"
							 "  if(s.playing) {} \n"
							 "  s.playing ? 0 : 1; \n"
							 "  switch(s.count) {case 0:} \n"
							 "}\n";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	//
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptString(engine);
		const char *scriptMain =
		"void error()"
		"{"
		"\"\" + (a.a() - b);"
		"}";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("scriptMain", scriptMain, strlen(scriptMain));
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "scriptMain (1, 1) : Info    : Compiling void error()\n"
						   "scriptMain (1, 20) : Error   : No matching symbol 'a'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	//
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		const char *script =
			"class Hoge \n"
			"{ \n"
			"  int mValue; \n"
			"  Hoge() \n"
			"  { \n"
			"    mValue = 0; \n"
			"  } \n"
			"  Hoge@ opAssign(const Hoge &in aObj) \n"
			"  { \n"
			"    mValue = aObj.mValue; \n"
			"    return @this; \n"
			"  } \n"
			"}; \n"
			"void main() \n"
			"{ \n"
			"  Hoge a = Hoge(); \n"
			"} \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
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

	//
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptString(engine);
		bout.buffer = "";

		const char *script =
			"class Test \n"
			"{ \n"
			"  const string @get_id() property \n"
			"  { \n"
			"    return @'test'; \n"
			"  } \n"
			"} \n"
			"void getClauseDesc(const string &in s) \n"
			"{ \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  Test t; \n"
			"  getClauseDesc(t.id); \n"
			"} \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterObjectType("Entity", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("Entity", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Entity", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("EntityArray", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("EntityArray", asBEHAVE_FACTORY, "EntityArray @f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("EntityArray", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("EntityArray", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("EntityArray", "Entity@ &opIndex(const uint)", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterGlobalFunction("Entity @DeleteEntity(Entity &in)", asFUNCTION(0), asCALL_GENERIC);

		// Because the DeleteEntity is taking &in, the object must be copied to a variable
		// to make sure the original object is not modified by the function. Because the
		// Entity doesn't have a default factory, this is supposed to fail
		const char *script =
			"void func() { \n"
			"EntityArray arr; \n"
			"Entity @temp = @arr[0]; \n"
			"DeleteEntity(temp); \n"
			"DeleteEntity(arr[0]); \n"
			"}; \n";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (1, 1) : Info    : Compiling void func()\n"
						   "script (4, 14) : Error   : No default constructor for object of type 'Entity'.\n"
						   "script (4, 14) : Error   : Previous error occurred while attempting to create a temporary copy of object\n"
						   "script (5, 14) : Error   : No default constructor for object of type 'Entity'.\n"
						   "script (5, 14) : Error   : Previous error occurred while attempting to create a temporary copy of object\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test", "class C { int x; int get_x() property {return x;} }\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);

		bout.buffer = "";
		mod->AddScriptSection("test", "interface ITest {}\n class Test {ITest t;}\n class Test2 : Test {}\n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "test (2, 20) : Error   : Interface 'ITest' cannot be instantiated\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Avoid assert failure on undeclared variables
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		const char *script =
			"void my_method() \n"
			"{ \n"
			"    int[] arr; \n"
			"    if(arr[unexisting_var-1]==1) \n"
			"    { \n"
			"    } \n"
			"} \n";

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (1, 1) : Info    : Compiling void my_method()\n"
		                   "script (4, 12) : Error   : No matching symbol 'unexisting_var'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	/////////////////
	{
		const char *script =
			"void main()\n"
			"{\n"
			"  while(turn()) {}\n"
			"}\n"

			"void turn()\n"
			"{\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterStdString(engine);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (1, 1) : Info    : Compiling void main()\n"
		                   "script (3, 9) : Error   : Expression must be of boolean type\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//////////////
	{
		const char *script =
			"class irc_event\n"
			"{\n"
			"	irc_event() \n"
			"	{\n"
			"       // apparently the following code will make AngelScript segfault rather than throw an error\n"
			"		command=params='NULL';\n"
			"	}\n"
			"	void set_command(string@[] i) property  {command=i;}\n"
			"	void set_params(string@ i) property     {params=i;}\n"
			"	string@[] get_command() property {return command;    }\n"
			"	string@ get_params() property    {return params;     }\n"
			"	string@[] command;\n"
			"	string params;\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterScriptString(engine);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (3, 2) : Info    : Compiling irc_event::irc_event()\n"
		                   "script (6, 10) : Error   : No matching signatures to 'irc_event::set_command(string)'\n"
		                   "script (6, 10) : Info    : Candidates are:\n"
		                   "script (6, 10) : Info    : void irc_event::set_command(string@[] i)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//////////////
	{
		const char *script =
			"enum wf_type \n"
			"{ \n"
			"  sawtooth=1, \n"
			"  square=2, \n"
			"  sine=3 \n"
			"} \n"
			"class tone_synth \n"
			"{ \n"
			"  void set_waveform_type(wf_type i) property {} \n"
			"} \n"
			"void main () \n"
			"{ \n"
			"  tone_synth t; \n"
			"  t.waveform_type = sine; \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void sine()", asFUNCTION(0), asCALL_GENERIC);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (11, 1) : Info    : Compiling void main()\n"
			               "script (14, 19) : Error   : No matching signatures to 'tone_synth::set_waveform_type(::sine)'\n"
		                   "script (14, 19) : Info    : Candidates are:\n"
		                   "script (14, 19) : Info    : void tone_synth::set_waveform_type(wf_type i)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//////////////
	{
		const char *script =
			"class Obj {}; \n"
			"class Hoge \n"
			"{ \n"
			"    const Obj obj()const { return Obj(); } \n"
			"} \n"
			"class Foo \n"
			"{ \n"
			"    Foo() \n"
			"    { \n"
			"        Hoge h; \n"
			"        Obj tmpObj = h.obj(); \n"
			"        mObj = h.obj(); \n" // this should work
			"    } \n"
			"    Obj mObj; \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void sine()", asFUNCTION(0), asCALL_GENERIC);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
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

	//////////////
	{
		const char *script =
			"class dummy \n"
			"{ \n"
			"int x; \n"
			"dummy(int new_x) \n"
			"{ \n"
			"x=new_x; \n"
			"} \n"
			"} \n"
			"void alert(const string &in, const string &in) {}\n"
			"void main() \n"
			"{ \n"
			"alert('Result', '' + bad.x + ''); \n"
			"dummy bad(15); \n"
			"alert('Result', '' + bad.x + ''); \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptString(engine);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (10, 1) : Info    : Compiling void main()\n"
		                   "script (12, 22) : Error   : No matching symbol 'bad'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//////////////
	{
		const char *script =
			"void main() { \n"
			"  int r = 2; \n"
			"  while(r-- > 0) {}; } \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
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

	/////////////////////
	// This test validates that the temporary variable used to store the return
	// value while the output parameter is evaluated isn't overwritten
	{
		const char *script =
			"class obj {} \n"
			"bool getPendingMats(obj@&out TL) \n"
			"{ \n"
			"  return false; \n"
			"} \n"
			"void paintFloor() \n"
			"{ \n"
			"  obj@[] center(4); \n"
			"  bool bb = getPendingMats(center[3]); \n"
			"  assert( bb == false ); \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "paintFloor()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test the parsing of doubles
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		double d;
		engine->RegisterGlobalProperty("double d", &d);

		ExecuteString(engine, "d = 0.1234567890123456789");

		if( !CompareDouble(d, 0.1234567890123456789) )
		{
			cout << "Got: d = " << d << endl;
			TEST_FAILED;
		}

		ExecuteString(engine, "d = 1.0e300");

		if( !CompareDouble(d/1.0e300, 1.0) )
		{
			cout << "Got: d = " << d << ", d/1e300 = " << (d/1.0e300) << " d/1e300 - 1 = " << (d/1.0e300-1.0) << endl;
			TEST_FAILED;
		}

		engine->Release();
	}

	// Make sure the deferred parameters are processed in the switch condition
	{
		const char *script =
			"int[] level1(9); \n"
			"void move_x() \n"
			"{ \n"
			"    switch(level1[1]) \n"
			"    { \n"
			"    case 2: \n"
			"      break; \n"
			"    } \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		bout.buffer = "";
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
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

	// Test float numbers starting with .
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, "assert( .42 == 0.42 ); assert( .42f == 0.42f )");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test return of handle to array
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class Test \n"
			"{ \n"
			"  array<int> @retArray() { return array<int>(); } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test complex expression
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class vec { \n"
			"  float angleAt(float) {return 0;} \n"
			"} \n"
			"float easeIn(float) {return 0;} \n"
			"class t { \n"
			"  float rotation; \n"
			"  vec path; \n"
			"  float alpha; \n"
			"} \n"
			"class a { \n"
			"  t _feuilleRonce00; \n"
			"  void main() \n"
			"  { \n"
			"    t[] plant; \n"
			"    if( true ) \n"
			"      _feuilleRonce00.rotation = _feuilleRonce00.rotation + (((plant[0].path.angleAt(easeIn(plant[0].alpha) - 0.1f)) - 0.5f) - _feuilleRonce00.rotation) * 0.1f; \n"
			"    else \n"
			"      _feuilleRonce00.rotation = _feuilleRonce00.rotation + (((plant[0].path.angleAt(easeIn(plant[0].alpha) - 0.1f))) - _feuilleRonce00.rotation) * 0.1f; \n"
			"}} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test parser error
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"void main() \n"
			"{ \n"
			"  int[] _countKill; \n"
			"  _countKill[12)++; \n"
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (1, 1) : Info    : Compiling void main()\n"
						   "script (4, 16) : Error   : Expected ']'\n"
						   "script (4, 16) : Error   : Instead found ')'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test - Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterStdString(engine);

		engine->RegisterObjectType("sound", 0, asOBJ_REF);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterObjectBehaviour("sound", asBEHAVE_FACTORY, "sound @f()", asFUNCTIONPR(CSound::CSound_fact, (), CSound *), asCALL_CDECL);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_ADDREF, "void f()", asMETHODPR(CSound, AddRef, (), void), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_RELEASE, "void f()", asMETHODPR(CSound, Release, (), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("sound", "double get_pan() const property", asMETHODPR(CSound, get_pan, () const, double), asCALL_THISCALL);
		engine->RegisterObjectMethod("sound", "void set_pan(double &in) property", asMETHODPR(CSound, set_pan, (double &), void), asCALL_THISCALL);
#else
		engine->RegisterObjectBehaviour("sound", asBEHAVE_FACTORY, "sound @f()", WRAP_FN_PR(CSound::CSound_fact, (), CSound *), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_ADDREF, "void f()", WRAP_MFN_PR(CSound, AddRef, (), void), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("sound", asBEHAVE_RELEASE, "void f()", WRAP_MFN_PR(CSound, Release, (), void), asCALL_GENERIC);
		engine->RegisterObjectMethod("sound", "double get_pan() const property", WRAP_MFN_PR(CSound, get_pan, () const, double), asCALL_GENERIC);
		engine->RegisterObjectMethod("sound", "void set_pan(double &in) property", WRAP_MFN_PR(CSound, set_pan, (double &), void), asCALL_GENERIC);
#endif

		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"void main() \n"
			"{ \n"
			" sound b; \n"
			" int hits = 0; \n"
			" if(b.pan >= -5) \n"
			" { \n"
			" } \n"
			" switch(hits) \n"
			" { \n"
			" case 10:  \n"
			"  break; \n"
			" case 15:  \n"
			"  break; \n"
			" case 20:\n"
			"  break; \n"
			" } \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test - Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"bool freq_ms(string &in freqs, double &in length) \n"
			"{ \n"
			"  assert( freqs == '524' ); \n"
			"  assert( length == 1000 ); \n"
			"  return false; \n"
			"} \n"
			"class tone_player \n"
			"{ \n"
			"  double freq; \n"
			"  double ms; \n"
			"  tone_player() \n"
			"  { \n"
			"    freq=524; \n"
			"    ms=1000; \n"
			"  } \n"
			"  void play_tone() \n"
			"  { \n"
			"    freq_ms(''+freq,ms); \n"
			"  } \n"
			"} \n"
			);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "tone_player tp; tp.play_tone();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test - Philip Bennefall
	{
		const char *script =
			"class Technique {\n"
			"  string hitsound;\n"
			"}\n"
			"Technique@ getTechnique() {return @Technique();}\n"
			"void main() {\n"
			"  string t = getTechnique().hitsound;\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterStdString(engine);
		RegisterScriptDictionary(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test - reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
#else
		engine->RegisterGlobalFunction("void print(const string &in)", WRAP_FN(Print), asCALL_GENERIC);
#endif

		const char *script =
			"double round(double d, int i) {return double(int64(d*1000+0.5))/1000;}\n"
	//		"string input_box(string &in, string &in) {return '';}\n"
	//		"int string_to_number(string) {return 0;}\n"
			"void main()\n"
			"{\n"
			//"  show_game_window('Golden Ratio');\n"
			"  \n"
			"  double numberA=0, numberB=1;\n"
			"  double sum_of_ratios=0;\n"
			"  int sequence_length=1475; \n" // 1475 is the largest we can go without breaking the limits of what a double can hold
			"  \n"
			"  for(int i=0; i<sequence_length; i++)\n"
			"  { \n"
			"    double temp=numberB;\n"
			"    numberB=numberA+numberB;\n"
			"    numberA=temp;\n"
			"    \n"
	//		"    print('A:'+numberA+', B:'+numberB+'\\n'); \n"
			"    sum_of_ratios+=round(numberB/numberA, 3); \n"
	//		"    print(sum_of_ratios+'\\n'); \n"
			"  } // end for. \n"
			"  \n"
			"  double average_of_ratios=sum_of_ratios/sequence_length; \n"
			"  average_of_ratios=round(average_of_ratios, 3); \n"
//			"  assert(average_of_ratios == 1.618); \n"
			"  print('The average of first '+sequence_length+' ratios of Fibonacci number is: '+average_of_ratios+'.\\n'); \n"
			//"  alert('Average', 'The average of first '+sequence_length+' ratios of Fibonacci number is: '+average_of_ratios+'.'); \n"
			//"  exit(); \n"
			"}\n";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		g_printbuf = "";

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( g_printbuf != "The average of first 1475 ratios of Fibonacci number is: 1.618.\n" )
		{
			PRINTF("%s", g_printbuf.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test invalid script
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class B {}\n"
			"void func() \n"
			"{ \n"
			"  B @b = cast<B>( typo.createInstance() ); //typo is obviously not an object.  \n"
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (2, 1) : Info    : Compiling void func()\n"
		                   "script (4, 19) : Error   : No matching symbol 'typo'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test ambigious names
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"void name(uint8 a) { } \n"
			"void main() \n"
			"{ \n"
			"  uint8 name; \n"
			"  name(7); \n"
			"  ::name(7); \n"
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (2, 1) : Info    : Compiling void main()\n"
                           "script (5, 3) : Error   : Expression doesn't form a function call. 'name' evaluates to the non-function type 'uint8'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test function overloading
	// This was failing because the code didn't filter out the method with only one parameter before starting
	// the match making. As it's first parameter was a better match it then made the code ignore the correct
	// method.
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class T { \n"
			"  uint mthd(int a, bool b) {return 0;} \n"
			"  bool mthd(uint a) {return false;} \n"
			"} \n"
			"void main() { \n"
			"  T t; \n"
			"  t.mthd(1, true); \n"
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

	// Test value copy during decl
	// http://www.gamedev.net/topic/618412-memory-leak-when-doing-an-assign-operation-with-a-handle/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class T { \n"
			"  T() {} \n"
		//	"  T(int v) {} \n"
			"  T &opAssign(const T&in o) {return this;} \n"
			"} \n"
			"T @Get() { return T(); } \n"
			"void main() { \n"
			"  T @t1 = Get(); \n"
			"  T t2 = t1; \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test extremely long expressions
	// Previously this was failing due to the recursiveness in the compiler
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		string script;
		script.reserve(20000);
		script += "int a = 1; \na = ";
		for( asUINT n = 0; n < 9500; n++ )
			script += "a+";
		script += "a; \nassert( a == 9501 ); \n";

		r = ExecuteString(engine, script.c_str());
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}


	// This test caught a problem when the script code allocated a script class,
	// which in turn used nested contexts to initialize some members. The VM
	// hadn't updated the members with the stack pointer/program pointer before
	// the nested call so the memory was overwritten.
	// Reported by Andrew Ackermann
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"shared class Alignment { \n"
			"  Alignment(int lt, float lp, int lx, \n"
			" 		     int tt, float tp, int tx, \n"
			"		     int rt, float rp, int rx, \n"
			"		     int bt, float bp, int bx) \n"
			"  { \n"
			"    assert(lt == AS_Left); \n"
			"    assert(tt == AS_Bottom); \n"
			"    assert(rt == AS_Right); \n"
			"    assert(bt == AS_Bottom); \n"
			"    assert(lp == 3.14f); \n"
			"    assert(tp == 1.43f); \n"
			"    assert(rp == 4.13f); \n"
			"    assert(bp == 4.34f); \n"
			"    assert(lx == 42); \n"
			"    assert(tx == 53); \n"
			"    assert(rx == 64); \n"
			"    assert(bx == 75); \n"
			"  }\n"
			"  AlignedPoint left; \n"
		//	"  AlignedPoint right; \n"
		//	"  AlignedPoint top; \n"
		//	"  AlignedPoint bottom; \n"
		//	"  double aspectRatio; \n"
		//	"  double aspectHorizAlign; \n"
		//	"  double aspectVertAlign; \n"
			"} \n"
			"shared class AlignedPoint { \n"
		//	"  int type; \n"
		//	"  float percent; \n"
		//	"  int pixels; \n"
		//	"  int size; \n"
			"  \n"
			"  AlignedPoint() { \n"
		//	"    type = AS_Left; \n"
		//	"    pixels = 0; \n"
		//	"    percent = 0; \n"
		//	"    size = 0; \n"
			"  } \n"
			"} \n"
			"shared enum AlignmentSide \n"
			"{ \n"
			"  AS_Left, AS_Right, AS_Top = AS_Left, AS_Bottom = AS_Right \n"
			"} \n"
			"class Fault { \n"
			"  Alignment @get_alignment() property {return A;} \n"
			"  void set_alignment(Alignment@ value) property {@A = value;} \n"
			"  Fault() { \n"
			"    a = 3.14f; \n"
			"    b = 1.43f; \n"
			"    c = 4.13f; \n"
			"    d = 4.34f; \n"
			"    @alignment = Alignment( \n"
			"                    AS_Left,   a + 0.0f, 42, \n"
			"                    AS_Bottom, b + 0.0f, 53, \n"
			"                    AS_Right,  c + 0.0f, 64, \n"
			"                    AS_Bottom, d + 0.0f, 75); \n"
			"  } \n"
			"  Alignment @A; \n"
			"  float a; float b; float c; float d; \n"
			"} \n";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Fault f()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	fail = Test2() || fail;
	fail = Test3() || fail;
	fail = Test4() || fail;
	fail = Test5() || fail;
	fail = Test6() || fail;
	fail = Test7() || fail;
	fail = Test8() || fail;
	fail = Test9() || fail;
	fail = TestRetRef() || fail;

	// Success
 	return fail;
}


//------------------------------------
// Test 2 was reported by Dentoid
float add(float &a, float &b)
{
	return a+b;
}

void doStuff(float, float)
{
}

bool Test2()
{
	RET_ON_MAX_PORT

#if defined(__GNUC__) && defined(__amd64__)
	// TODO: Add this support
	// Passing non-complex objects by value is not yet supported, because
	// it means moving each property of the object into different registers
	return false;
#endif

	bool fail = false;
	int r;
	COutStream out;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	engine->RegisterObjectType( "Test", sizeof(float), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_FLOAT );
	engine->RegisterObjectMethod( "Test", "Test opAdd(Test &in) const", asFUNCTION(add), asCALL_CDECL_OBJFIRST);
	engine->RegisterGlobalFunction("void doStuff(Test, Test)", asFUNCTION(doStuff), asCALL_CDECL);

	const char *script =
	"Test test1, test2;                \n"
	"doStuff( test1, test1 + test2 );  \n"  // This one will work
	"doStuff( test1 + test2, test1 );  \n"; // This one will blow

	r = ExecuteString(engine, script);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();
	return fail;
}

//-----------------------------------------
// Test 3 was reported by loboWu
bool Test3()
{
	bool fail = false;
	COutStream out;
	int r;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	const char *script =
	"uint8 search_no(uint8[]@ cmd, uint16 len, uint8[] @rcv)    \n" //mbno is nx5 array, static
	"{														    \n"
	"	if (@rcv == null) assert(false);						\n"
	"		return(255);										\n"
	"}															\n"
	"void main()												\n"
	"{															\n"
	"	uint8[] cmd = { 0x02, 0x95, 0x45, 0x42, 0x32 };			\n"
	"	uint8[] rcv;											\n"
	"	uint16 len = 8;											\n"
	"	search_no(cmd, cmd.length(), rcv);						\n" //This is OK! @rcv won't be null
	"	search_no(cmd, GET_LEN2(cmd), rcv);						\n" //This is OK!
	"	len = GET_LEN(cmd);										\n"
	"	search_no(cmd, len, rcv);								\n" //This is OK!
	"															\n"//but
	"	search_no(cmd, GET_LEN(cmd), rcv);						\n" //@rcv is null
	"}															\n"
	"uint16 GET_LEN(uint8[]@ cmd)								\n"
	"{															\n"
	"	return cmd[0]+3;										\n"
	"}															\n"
	"uint16 GET_LEN2(uint8[] cmd)								\n"
	"{															\n"
	"	return cmd[0]+3;										\n"
	"}															\n";

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "main()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();
	return fail;
}

//----------------------------------------
// Test 4 reported by dxj19831029
bool Test4()
{
	bool fail = false;
	COutStream out;
	int r = 0;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	engine->RegisterObjectType("Chars", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("Chars", asBEHAVE_FACTORY, "Chars@ f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Chars", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Chars", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectMethod("Chars", "Chars &opAssign(const Chars &in)", asFUNCTION(0), asCALL_GENERIC);

	engine->RegisterObjectType("_Save", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectProperty("_Save", "Chars FieldName", 0);

	engine->RegisterObjectType("Struct", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("Struct", asBEHAVE_FACTORY, "Struct@ f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Struct", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Struct", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
	engine->RegisterObjectProperty("Struct", "_Save Save", 0);

	engine->RegisterObjectType("ScriptObject", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterObjectMethod("ScriptObject", "Struct @f()", asFUNCTION(0), asCALL_GENERIC);

	engine->RegisterGlobalProperty("ScriptObject current", (void*)1);

	engine->RegisterGlobalFunction("void print(Chars&)", asFUNCTION(0), asCALL_GENERIC);

	const char *script1 = "void main() { print(current.f().Save.FieldName); }";
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script1, strlen(script1));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	const char *script2 = "void main() { Chars a = current.f().Save.FieldName; print(a); }";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script2, strlen(script2));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	engine->Release();
	return fail;
}

//-----------------------------------------------
// Test5 reported by jal
bool Test5()
{
	// This script caused an assert failure during compilation
	const char *script =
		"class cFlagBase {} \n"
		"void CTF_getBaseForOwner( )   \n"
		"{  \n"
		"   for ( cFlagBase @flagBase; ; @flagBase = null ) \n"
		"   {  \n"
		"	}  \n"
		"}   ";

	bool fail = false;
	COutStream out;
	int r = 0;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 0);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	engine->Release();
	return fail;
}

//-------------------------------------------------
// Test6 reported by SiCrane
bool Test6()
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

	// The following script would enter an infinite loop while building
	const char *script1 =
		"class Foo { \n"
		"const int foo(int a) { return a; } \n"
		"} \n";

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script1, strlen(script1));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// This should also work
	const char *script2 =
		"interface IFoo { \n"
		"	const int foo(int a); \n"
		"} \n";

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script2, strlen(script2));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// This would cause an assert failure
	const char *script3 =
		"class MyClass { \n"
		"    MyClass(int a) {} \n"
		"} \n"
		"const MyClass foo(int (a) ,bar); \n";

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script3, strlen(script3));
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (4, 15) : Info    : Compiling const MyClass foo\n"
					   "script (4, 28) : Error   : No matching symbol 'bar'\n"
					   "script (4, 24) : Error   : No matching symbol 'a'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// This would also cause an assert failure
	const char *script4 =
		"void main() { \n"
		"  for (;i < 10;); \n"
		"} \n";

	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script4, strlen(script4));
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 1) : Info    : Compiling void main()\n"
					   "script (2, 9) : Error   : No matching symbol 'i'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

//---------------------------------------
// Test7 reported by Vicious
// http://www.gamedev.net/community/forums/topic.asp?topic_id=525467
bool Test7()
{
	RET_ON_MAX_PORT

	bool fail = false;

	const char *script =
	"void GENERIC_CommandDropItem( cClient @client )	\n"
	"{													\n"
	"	client.getEnt().health -= 1;					\n"
	"}													\n";

	// 1. tmp1 = client.getEnt()
	// 2. tmp2 = tmp1.health
	// 3. tmp3 = tmp2 - 1
	// 4. free tmp2
	// 5. tmp1.health = tmp3
	// 6. free tmp3
	// 7. free tmp1

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString(engine);

	engine->RegisterObjectType("cEntity", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("cEntity", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("cEntity", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("cEntity", "cEntity @getEnt()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectProperty("cEntity", "int health", 0);
	engine->RegisterObjectType("cClient", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("cClient", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("cClient", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("cClient", "cEntity @getEnt()", asFUNCTION(0), asCALL_CDECL_OBJLAST);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	int r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

bool Test8()
{
	bool fail = false;

	CBufferedOutStream bout;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	RegisterScriptString(engine);

	// Must allow returning a const string
	const char *script = "const string func() { return ''; }";

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	int r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	r = ExecuteString(engine, "string str = func(); assert( str == '' );", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}

bool Test9()
{
	bool fail = false;
	CBufferedOutStream bout;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	const char *script = "void Func() \n"
						 "{ \n"
						 "	(aaa.AnyName())==0?1:0; \n"
						 "} \n";

	mod->AddScriptSection("sc", script);
	int r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;

	if( bout.buffer != "sc (1, 1) : Info    : Compiling void Func()\n"
					   "sc (3, 3) : Error   : No matching symbol 'aaa'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

//////////////////////////////
// AgentC reported problems
// http://www.gamedev.net/topic/614727-crash-with-temporary-value-types-and-unsafe-references/
// http://www.gamedev.net/topic/615762-assert-failures-of-unfreed-temp-variables/page__gopid__4887763#entry4887763

class Variant
{
public:
	Variant() {val = "test";}
	Variant(const Variant &other) {val = other.val;}
	~Variant() {val = "deleted";}
	Variant &operator=(const Variant &) {return *this;}
	Variant &operator=(int) {return *this;}
	const std::string &GetString() const {return val;}
	std::string val;
};

void ConstructVariant(Variant *self)
{
	new(self) Variant();
}

void ConstructVariantCopy(Variant &other, Variant *self)
{
	new(self) Variant(other);
}

void DestructVariant(Variant *self)
{
	self->~Variant();
}

class VariantMap
{
public:
	Variant var;
	Variant &opIndex(const string &) { return var; }
};

void ConstructVariantMap(VariantMap *self)
{
	new(self) VariantMap();
}

void DestructVariantMap(VariantMap *self)
{
	self->~VariantMap();
}

class Node
{
public:
	Node() {refCount = 1;}
	void AddRef() {refCount++;}
	void Release() {if( --refCount == 0 ) delete this;}
	Variant GetAttribute() {return Variant();}
	int refCount;
	VariantMap vars;
};

Node *NodeFactory()
{
	return new Node();
}

static Node *g_node = 0;
Node *GetGlobalNode()
{
	return g_node;
}

bool TestRetRef()
{
	bool fail = false;
	CBufferedOutStream bout;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

	RegisterStdString(engine);

	engine->RegisterObjectType("Variant", sizeof(Variant), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
	engine->RegisterObjectType("VariantMap", sizeof(VariantMap), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
	engine->RegisterObjectType("Node", 0, asOBJ_REF);
#ifndef AS_MAX_PORTABILITY
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructVariant), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f(const Variant&in)", asFUNCTION(ConstructVariantCopy), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructVariant), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Variant", "const string& GetString() const", asMETHOD(Variant, GetString), asCALL_THISCALL);
	engine->RegisterObjectMethod("Variant", "Variant& opAssign(const Variant&in)", asMETHODPR(Variant, operator =, (const Variant&), Variant&), asCALL_THISCALL);
	engine->RegisterObjectMethod("Variant", "Variant& opAssign(int)", asMETHODPR(Variant, operator =, (int), Variant&), asCALL_THISCALL);

	engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructVariantMap), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructVariantMap), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("VariantMap", "Variant &opIndex(const string &in)", asMETHODPR(VariantMap, opIndex, (const string &), Variant&), asCALL_THISCALL);

	engine->RegisterObjectBehaviour("Node", asBEHAVE_FACTORY, "Node @f()", asFUNCTION(NodeFactory), asCALL_CDECL);
	engine->RegisterObjectBehaviour("Node", asBEHAVE_ADDREF, "void f()", asMETHOD(Node, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("Node", asBEHAVE_RELEASE, "void f()", asMETHOD(Node, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod("Node", "Variant GetAttribute() const", asMETHODPR(Node, GetAttribute, (), Variant), asCALL_THISCALL);

	engine->RegisterGlobalFunction("Node@+ get_node() property", asFUNCTION(GetGlobalNode), asCALL_CDECL);
#else
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f()", WRAP_OBJ_LAST(ConstructVariant), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_CONSTRUCT, "void f(const Variant&in)", WRAP_OBJ_LAST(ConstructVariantCopy), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Variant", asBEHAVE_DESTRUCT, "void f()", WRAP_OBJ_LAST(DestructVariant), asCALL_GENERIC);
	engine->RegisterObjectMethod("Variant", "const string& GetString() const", WRAP_MFN(Variant, GetString), asCALL_GENERIC);
	engine->RegisterObjectMethod("Variant", "Variant& opAssign(const Variant&in)", WRAP_MFN_PR(Variant, operator =, (const Variant&), Variant&), asCALL_GENERIC);
	engine->RegisterObjectMethod("Variant", "Variant& opAssign(int)", WRAP_MFN_PR(Variant, operator =, (int), Variant&), asCALL_GENERIC);

	engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_CONSTRUCT, "void f()", WRAP_OBJ_LAST(ConstructVariantMap), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("VariantMap", asBEHAVE_DESTRUCT, "void f()", WRAP_OBJ_LAST(DestructVariantMap), asCALL_GENERIC);
	engine->RegisterObjectMethod("VariantMap", "Variant &opIndex(const string &in)", WRAP_MFN_PR(VariantMap, opIndex, (const string &), Variant&), asCALL_GENERIC);

	engine->RegisterObjectBehaviour("Node", asBEHAVE_FACTORY, "Node @f()", WRAP_FN(NodeFactory), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Node", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Node, AddRef), asCALL_GENERIC);
	engine->RegisterObjectBehaviour("Node", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Node, Release), asCALL_GENERIC);
	engine->RegisterObjectMethod("Node", "Variant GetAttribute() const", WRAP_MFN_PR(Node, GetAttribute, (), Variant), asCALL_GENERIC);

	engine->RegisterGlobalFunction("Node@+ get_node() property", WRAP_FN(GetGlobalNode), asCALL_GENERIC);
#endif
	engine->RegisterObjectProperty("Node", "VariantMap vars", asOFFSET(Node, vars));

	g_node = NodeFactory();

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", "void f() { \n"
								  "  string str = node.GetAttribute().GetString(); // Get its first attribute as a string \n"
								  "  assert( str == 'test' ); \n"
								  "} \n");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	if( bout.buffer != "" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	r = ExecuteString(engine, "f();", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	// Test for assert failures
	mod->AddScriptSection("test", "class GameObject { \n"
		                          "  int health; \n"
								  "  GameObject() { \n"
								  "    health = 10; \n"
                                  "  } \n"
								  "  void Update(float deltaTime) { \n"
								  "    node.vars['Health'] = health; \n" // This will cause unfreed temp variable of type Node@
                                  "  } \n"
                                  "} \n");
	bout.buffer = "";
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	if( bout.buffer != "" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->ShutDownAndRelease();
	g_node->Release();

	return fail;
}

} // namespace

