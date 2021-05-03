//
// Tests the ternary operator ?:
//
// Author: Andreas Jonsson
//

#include "utils.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include "../../../add_on/scriptmath/scriptmath.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"

static const char * const TESTNAME = "TestCondition";

using std::string;
static CScriptString *a = 0;

static void formatf(asIScriptGeneric *gen)
{
	float f = gen->GetArgFloat(0);
	char buffer[25];
	sprintf(buffer, "%f", f);
	gen->SetReturnAddress(new CScriptString(buffer));
}

static void formatUI(asIScriptGeneric *gen)
{
	asUINT ui = gen->GetArgDWord(0);
	char buffer[25];
	sprintf(buffer, "%d", ui);
	gen->SetReturnAddress(new CScriptString(buffer));
}

static void print(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgObject(0);
	UNUSED_VAR(str);
//	PRINTF((str + "\n").c_str());
}

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
  string *tmp = static_cast<string *>(gen->GetArgObject(0));
  new (gen->GetObject()) string(*tmp);
}

static void DestructStringGeneric(asIScriptGeneric * gen) {
  string * ptr = static_cast<string *>(gen->GetObject());
  ptr->~string();
}

static void AssignStringGeneric(asIScriptGeneric *gen) {
  string * tmp = static_cast<string *>(gen->GetArgObject(0));
  string * self = static_cast<string *>(gen->GetObject());
  *self = *tmp;
  gen->SetReturnAddress(self);
}

bool StringEquals(const std::string& lhs, const std::string& rhs)
{
    return lhs == rhs;
}

static int newId_ = 0;
static int countValueDestruct = 0;

bool TestCondition()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine* engine;

	// Test condition with enum and int as operands. Should convert the enum to int
	// ref: for c++ https://stackoverflow.com/questions/32251419/c-ternary-operator-conditional-operator-and-its-implicit-type-conversion-r
	// https://www.gamedev.net/forums/topic/708198-implicit-conversion-from-enum-to-int-in-ternary-op/5431934/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule* mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum A { VALUE = 1 } \n"
			"void test() \n"
			"{ \n"
			"	int val = true ? VALUE : 0; \n"
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

	// Test condition with float and int as operands. Ambiguous case, shouldn't implicitly convert
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule* mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"float a = 3.1; \n"
			"int b = 3; \n"
			"void test() \n"
			"{ \n"
			"	int val = true ? a : b; \n"
			"} \n");
 		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (3, 1) : Info    : Compiling void test()\n"
			               "test (5, 12) : Error   : Can't find unambiguous implicit conversion to make both expressions have the same type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test that temporary objects allocated as arguments that are then deferred in ternary operands are properly handled
	// Reported by Polyak Istvan
	{
		class Value
		{
		public:
			Value(void) : id_(++newId_)
			{
			//	printf("Value (void)\n");
			}
			~Value()
			{
				countValueDestruct++;
			//	printf("~Value ():%d\n", id_);
			}
			static void constructor(Value* val) {
				new(val) Value();
			}
			static void destructor(Value* val) {
				val->~Value();
			}
		private:
			int id_;
		};

		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
		engine->RegisterObjectType("Value", sizeof(Value), asOBJ_VALUE | asOBJ_APP_CLASS);
		engine->RegisterObjectBehaviour("Value", asBEHAVE_CONSTRUCT, "void f ()", asFUNCTIONPR(Value::constructor, (Value*), void), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectBehaviour("Value", asBEHAVE_DESTRUCT, "void f ()", asFUNCTIONPR(Value::destructor, (Value*), void), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("Value", "Value & opAssign (const Value &in)", asMETHODPR(Value, operator=, (const Value&), Value&), asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int gi = 0; \n"
			"int& f3(const Value & in v) \n"
			"{ \n"
			"	return gi; \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"	int i3 = false ? f3(Value()) : f3(Value()); \n"  // The temporary Value() must be destroyed within the operand itself
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (countValueDestruct != 1)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test anonymous initialization lists in conditional operator
	// Reported by doctorgester
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"array<int> let_the_dogs_out(bool flag) { \n"
			"    return flag ? { 5 } : array<int>(); \n"  // compiler sets the type of list the the same as second condition
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"array<int> let_the_dogs_out(bool flag) { \n"
			"    return flag ? array<int>() : {5}; \n"  // compiler sets the type of list the the same as second condition
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"array<int> let_the_dogs_out(bool flag) { \n"
			"    return flag ? {5} : {0}; \n"  // error, compiler cannot know the type
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 1) : Info    : Compiling array<int> let_the_dogs_out(bool)\n"
						   "test (2, 19) : Error   : Unable to resolve auto type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"array<int> let_the_dogs_out(bool flag) { \n"
			"    return flag ? array<int> = { 5 } : array<int>(); \n"  // explicitly informing the type works
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

	// Test condition with std string
	// http://www.gamedev.net/topic/684124-weird-string-behavior-when-using/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "string foo = 'foo' + (true ? string('bar') : 'bar'); assert( foo == 'foobar' );", 0, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "string foo = 'foo' + (false ? 'bar' : string('bar')); assert( foo == 'foobar' );", 0, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "string foo = 'foo' + (true ? 'bar' : 'bar'); assert( foo == 'foobar' );", 0, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test condition where both cases return null
	// http://www.gamedev.net/topic/677273-various-unexpected-behaviors-of-angelscript-2310/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

//		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() {\n"
			"	foo@ bar = true ? null : null;\n"
			"}\n"
			"class foo {}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] =
		{
			asBC_SUSPEND,asBC_FREE,asBC_RET
		};
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;


		engine->ShutDownAndRelease();
	}

	// Test nested conditions returning global values
	// Problem reported by Jonathan Sandusky
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterScriptMathComplex(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"complex a(1,0), b(2,0), c(1,1); \n"
			"void Test() { \n"
			"   complex d; \n"
			"   bool editable = true; \n"
			"   bool modified = true; \n"
			"   d = (editable ? (modified ? a : b) : c); \n"
			"   assert( d == a ); \n"
			"   editable = false; \n"
			"   d = (editable ? (modified ? a : b) : c); \n"
			"   assert( d == c ); \n"
			"   editable = true; \n"
			"   modified = false; \n"
			"   d = (editable ? (modified ? a : b) : c); \n"
			"   assert( d == b ); \n"
			"} \n");
		// TODO: run-time optimize: The produced bytecode can be improved by placing the RDSPtr in both expressions so the bytecode optimization can exchange the instructions to PshGPtr
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test condition used as lvalue
	// If both expressions are lvalues of the same type and neither have
	// deferred arguments the condition itself can be an lvalue
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);
		
		bout.buffer = "";
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class TestClass{ int a; int b; \n"
			"  int &func(bool c) { return c ? a : b; } \n"
			"  void func2(bool c, int val) { (c ? a : b) = val; } \n"
			"} \n"
			"void Test() { \n"
			"   TestClass t; \n"
			"   t.func(true) = 1; \n"
			"   t.func(false) = 2; \n"
			"   assert( t.a == 1 ); \n"
			"   assert( t.b == 2 ); \n"
			"   t.func2(true, 3); \n"
			"   t.func2(false, 4); \n"
			"   assert( t.a == 3 ); \n"
			"   assert( t.b == 4 ); \n"
			"   int a = 0, b = 0; \n"
			"   (true?a:b) = 1; \n"
			"   (false?a:b) = 2; \n"
			"   assert( a == 1 ); \n"
			"   assert( b == 2 ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Must not allow returning reference to local variable
		bout.buffer = "";
		mod->AddScriptSection("test",
			"int &func(bool c) { int a, b; return c ? a : b; } \n"
			"string &func2(bool c) { string a, b; return c ? a : b; } \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if(	bout.buffer != "test (1, 1) : Info    : Compiling int& func(bool)\n"
						   "test (1, 31) : Error   : Resulting reference cannot be returned. Returned references must not refer to local variables.\n"
						   "test (2, 1) : Info    : Compiling string& func2(bool)\n"
			               "test (2, 38) : Error   : Resulting reference cannot be returned. The expression uses objects that during cleanup may invalidate it.\n" )
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
#ifndef AS_MAX_PORTABILITY
		r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTIONPR(StringEquals, (const string &, const string &), bool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
#else
		r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", WRAP_OBJ_FIRST_PR(StringEquals, (const string &, const string &), bool), asCALL_GENERIC); assert( r >= 0 );
#endif
		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

		// Condition was failing due to the string factory returning a const reference
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"void func(int value)\n"
			"{\n"
			"  string result= (value<5) ? 'less than 5' : (value>8) ? 'Greater than 8' : 'greater than 5';\n"
			"  assert( result == 'greater than 5' ); \n"
			"}\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "func(5)", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test condition with null handle
	// http://www.gamedev.net/topic/652528-cant-implicitly-convert-from-const-testclass-to-testclass/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterScriptArray(engine, true);

		bout.buffer = "";
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class TestClass{} \n"
			"void Test() { \n"
			"   array<TestClass@> test; \n"
			"   TestClass @s = test.length() > 0 ? test[0] : null; \n"
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

	// Test condition where the two operands different only in const
	// http://www.gamedev.net/topic/651245-factory-for-const-string/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";
		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Test {} \n"
			"void func() { \n"
			"  const Test @ct; \n"
			"  Test @t; \n"
			"  const Test @a = true ? t : ct; \n"
			"  const Test @b = true ? ct : t; \n"
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

	// Test condition operator and implicit cast
	// Observe that AngelScript does NOT follow the same rules as C++ for this operator
	// http://www.gamedev.net/topic/648406-implicit-conversion-of-value-is-not-exact/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterScriptMath(engine);

		bout.buffer = "";
		r = ExecuteString(engine, "float ret = false ? 0 : 0.1; \n"       // 0 is implicitly converted to the other type
			                      "assert( abs(ret - 0.1) < 0.0001 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		r = ExecuteString(engine, "float ret = true ? 0.1 : 0; \n"       // 0 is implicitly converted to the other type
			"assert( abs(ret - 0.1) < 0.0001 );");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		r = ExecuteString(engine, "float ret = false ? 1 : 0.1; \n"       // 1 is not implicitly converted
			                      "assert( abs(ret - 0) < 0.0001 );");
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 13) : Error   : Can't find unambiguous implicit conversion to make both expressions have the same type\n"
						   "ExecuteString (2, 17) : Warning : 'ret' is not initialized.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		r = ExecuteString(engine, "float f = 0.1;\n"
								  "float ret = 0.2;\n"
								  "ret += false ? 0 : f * 15;\n"           // 0 is implicitly converted to the other type
								  "assert( abs(ret - 1.7) < 0.0001 );\n");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		r = ExecuteString(engine, "float f = 0.1;\n"
								  "float ret = 0.2;\n"
								  "ret += false ? 1 : f * 15;\n"           // 1 is not implicitly converted to the other type
								  "assert( abs(ret - 1.2) < 0.0001 );\n");
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (3, 8) : Error   : Can't find unambiguous implicit conversion to make both expressions have the same type\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		a = new CScriptString();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterScriptString_Generic(engine);
		engine->RegisterGlobalProperty("string a", a);

	//	engine->RegisterObjectType("Data", 0, asOBJ_REF | asOBJ_NOHANDLE);

		engine->RegisterGlobalFunction("string@ format(float)", asFUNCTION(formatf), asCALL_GENERIC);
		engine->RegisterGlobalFunction("string@ format(uint)", asFUNCTION(formatUI), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void print(string &in)", asFUNCTION(print), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "print(a == \"a\" ? \"t\" : \"f\")");
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script1 =
			"void Test(string strA, string strB)   \n"
			"{                                     \n"
			"  a = true ? strA : strB;             \n"
			"  a = false ? \"t\" : \"f\";          \n"
			"  SetAttrib(true ? strA : strB);      \n"
			"  SetAttrib(false ? \"f\" : \"t\");   \n"
			"}                                     \n"
			"void SetAttrib(string str) { assert( str == 't' ); }   \n";
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		mod->Build();

		r = ExecuteString(engine, "Test(\"t\", \"f\")", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

	/*	
		// Pointers are no longer supported since version 2.0.0
		const char *script2 =
			"void Test()                     \n"
			"{                               \n"
			"  int a = 0;                    \n"
			"  Data *v = 0;                  \n"
			"  Data *p;                      \n"
			"  p = a != 0 ? v : 0;           \n"
			"  p = a == 0 ? 0 : v;           \n"
			"}                               \n";

		mod->AddScriptSection(0, TESTNAME, script2, strlen(script2), 0);
		mod->Build(0);

		r = ExecuteString(engine, "Test()");
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}
	*/

		const char *script3 =
			"void Test()                                  \n"
			"{                                            \n"
			"  int test = 5;                              \n"
			"  int test2 = int((test == 5) ? 23 : 12);    \n"
			"}                                            \n";
		mod->AddScriptSection(TESTNAME, script3, strlen(script3), 0);
		mod->Build();

		r = ExecuteString(engine, "Test()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

		r = ExecuteString(engine, "bool b = true; print(\"Test: \" + format(float(b ? 15 : 0)));");
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

		r = ExecuteString(engine, "bool b = true; print(\"Test: \" + format(b ? 15 : 0));");
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

		r = ExecuteString(engine, "(true) ? print(\"true\") : print(\"false\")");
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: ExecuteString() failed\n", TESTNAME);
		}

		const char *script = "double get_gameTime() property { return 100; } \n"
							 "void advance(bool full) { \n"
							 "  nextThink = gameTime + ( 30.0 * (full ? 10.0 : 1.0) ); \n"
							 "} \n"
							 "double nextThink; \n";
		mod->AddScriptSection("script", script);
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "nextThink = 0; advance(true); assert( nextThink == 100 + 300 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		r = ExecuteString(engine, "nextThink = 0; advance(false); assert( nextThink == 100 + 30 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
		a->Release();
	}

	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		const char *script = "class T \n"
		                     "{ \n"
			                 "  T@ Get() \n"
		                     "  { \n"
							 "    T@ r; \n"
							 "    return (false ? null : r); \n"
	                         "  } \n"
	                         "} \n";
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Having classes with the same name as the template subtype was causing problems
		script = "class T \n"
		    "{ \n"
			"  array<T@> Ts;\n"
			"  T@ Get(uint n)\n"
		    "  {\n"
			"    return (n>=Ts.length()) ? null : Ts[n];  \n"
			"  } \n"
			"} \n";

		bout.buffer = "";
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

	// Success
	return fail;
}
