#include "utils.h"
#include "../../../add_on/scriptany/scriptany.h"

namespace TestScriptClassMethod
{

static const char * const TESTNAME = "TestScriptClassMethod";

// Normal structure
static const char *script1 =
"void Test()                                     \n"
"{                                               \n"
"   myclass a;                                   \n"
"   a.mthd(1);                                   \n"
"   Assert( a.c == 4 );                          \n"
"   mthd2(2);                                    \n"
"   @g = myclass();                              \n"
"   g.deleteGlobal();                            \n"
"}                                               \n"
"class myclass                                   \n"
"{                                               \n"
"   void deleteGlobal()                          \n"
"   {                                            \n"
"      @g = null;                                \n"
"      Analyze(any(@this));                      \n"
"   }                                            \n"
"   void mthd(int a)                             \n"
"   {                                            \n"
"      int b = 3;                                \n"
"      print(\"class:\"+a+\":\"+b);              \n"
"      myclass tmp;                              \n"
"      this = tmp;                               \n"
"      this.c = 4;                               \n"
"   }                                            \n"
"   void mthd2(int a)                            \n"
"   {                                            \n"
"      print(\"class:\"+a);                      \n"
"   }                                            \n"
"   int c;                                       \n"
"};                                              \n"
"void mthd2(int a) { print(\"global:\"+a); }     \n"
"myclass @g;                                     \n";

static const char *script2 =
"class myclass                                   \n"
"{                                               \n"
"  myclass()                                     \n"
"  {                                             \n"
"    print(\"Default constructor\");             \n"
"    this.value = 1;                             \n"
"  }                                             \n"
"  myclass(int a)                                \n"
"  {                                             \n"
"    print(\"Constructor(\"+a+\")\");            \n"
"    this.value = 2;                             \n"
"  }                                             \n"
"  void method()                                 \n"
"  {                                             \n"
"    this.value = 3;                             \n"
"  }                                             \n"
"  void method2()                                \n"
"  {                                             \n"
"    this.method();                              \n"
"  }                                             \n"
"  int value;                                    \n"
"};                                              \n"
"void Test()                                     \n"
"{                                               \n"
"  myclass c;                                    \n"
"  Assert(c.value == 1);                         \n"
"  myclass d(1);                                 \n"
"  Assert(d.value == 2);                         \n"
"  c = myclass(2);                               \n"
"  Assert(c.value == 2);                         \n"
"}                                               \n";

static const char *script3 = 
"class myclass                                   \n"
"{                                               \n"
"  myclass() {value = 42;}                       \n"
"  void func() {Assert(value == 42);}            \n"
"  void func(int x, int y) {Assert(value == 42);}\n"
"  int value;                                    \n"
"};                                              \n"
"myclass c;                                      \n";

static const char *script4 =
"class myclass        \n"
"{                    \n"
"  void func() {}     \n"
"}                    \n"
"void func() {}       \n";

static const char *script5 =
"int b;               \n"
"class myclass        \n"
"{                    \n"
"  void func()        \n"
"  {                  \n"
"     int a = 3;      \n"
"     this.a = a;     \n"
"     test();         \n"
"  }                  \n"
"  void test()        \n"
"  {                  \n"
"     b = a;          \n"
"  }                  \n"
"  int a;             \n"
"  int b;             \n"
"}                    \n"
"void test()          \n"
"{                    \n"
"   b = 9;            \n"
"   myclass m;        \n"
"   m.func();         \n"
"   Assert(b == 9);   \n"
"   Assert(m.a == 3); \n"
"   Assert(m.b == 3); \n"
"}                    \n";


static const char *script6 =
"class Set             \n"
"{                     \n"
"   Set(int a) {print(\"Set::Set\");}      \n"
"};                    \n"
"class Test            \n"
"{                     \n"
"   void Set(int a) {print(\"Test::Set\");} \n"
"   void Test2()       \n"
"   {                  \n"
"      int a = 0;      \n"
       // Call class method
"      Set(a);         \n"
       // Call Set constructor
"      ::Set(a);       \n"
"   }                  \n"
"}                     \n";

std::string outbuffer;
void print(asIScriptGeneric *gen)
{
	std::string s = ((CScriptString*)gen->GetArgAddress(0))->buffer;
//	PRINTF("%s\n", s.c_str());
	outbuffer += s + "\n";
}

void Analyze(asIScriptGeneric *gen)
{
	CScriptAny *a = (CScriptAny*)gen->GetArgAddress(0);
	int myclassId = a->GetTypeId();
	asIScriptObject *s = 0;
	a->Retrieve(&s, myclassId);
	s->Release();
}

bool Test()
{
	bool fail = false;
	int r;
	asIScriptEngine *engine;
	COutStream out;
	CBufferedOutStream bout;

	// Test conflicting method name and class name with scoping
	// https://www.gamedev.net/topic/679898-cant-call-base-function-if-the-function-name-is-the-same-as-a-class-name/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Cat \n"
			"{ \n"
			"} \n"
			"class Foo \n"
			"{ \n"
			"	int Cat() \n"				// Method has the same name as another class
			"	{ \n"
			"		return 0; \n"
			"	} \n"
			"	int Dog() \n"				// No name conflict
			"	{ \n"
			"		return 0; \n"
			"	} \n"
			"} \n"
			"class Bar : Foo \n"
			"{ \n"
			"	int Cat() override \n"
			"	{ \n"
			"		return Foo::Cat(); \n" // Calling method 'Cat' on parent class 'Foo'
			"	} \n"
			"	int Dog() override \n"
			"	{ \n"
			"		return Foo::Dog(); \n" // OK
			"	} \n"
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

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterScriptString_Generic(engine);
		RegisterScriptAny(engine);

		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void Analyze(any &inout)", asFUNCTION(Analyze), asCALL_GENERIC);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "Test()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION) PRINTF("%s", GetExceptionInfo(ctx).c_str());
			TEST_FAILED;
		}
		if (ctx) ctx->Release();

		// Make sure that the error message for wrong constructor name works
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, "class t{ s() {} \n ~d() {} };");
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "TestScriptClassMethod (1, 10) : Error   : Method 't::s' is missing the return type, nor is it the same name as object to be a constructor\n"
			"TestScriptClassMethod (2, 2) : Error   : The name of the destructor 't::~d' must be the same as the class\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Make sure the default constructor can be overloaded
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		r = ExecuteString(engine, "Test()", mod);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
		}

		asITypeInfo *type = engine->GetModule("test")->GetTypeInfoByName("myclass");
		asIScriptObject *s = (asIScriptObject*)engine->CreateScriptObject(type);
		if (s == 0)
			TEST_FAILED;
		else
		{
			// Validate the property
			int *v = 0;
			int n = s->GetPropertyCount();
			for (int c = 0; c < n; c++)
			{
				std::string str = "value";
				if (str == s->GetPropertyName(c))
				{
					v = (int*)s->GetAddressOfProperty(c);
					if (*v != 1) TEST_FAILED;
				}
			}

			// Call the script class method
			if (type->GetMethodCount() != 2)
				TEST_FAILED;
			asIScriptFunction *method = type->GetMethodByDecl("void method2()");
			if (method == 0)
				TEST_FAILED;
			else
			{
				ctx = engine->CreateContext();
				ctx->Prepare(method);
				ctx->SetObject(s);
				r = ctx->Execute();
				if (r != asEXECUTION_FINISHED)
					TEST_FAILED;

				if ((!v) || (*v != 3))
					TEST_FAILED;

				ctx->Release();
			}

			s->Release();
		}

		engine->Release();
	}

	//----------------------------------
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test3", script3, strlen(script3), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		int typeId = engine->GetModule(0)->GetTypeIdByDecl("myclass");
		asITypeInfo *type = engine->GetTypeInfoById(typeId);
		asIScriptFunction *mtd = type->GetMethodByDecl("void func()");
		asIScriptObject *obj = (asIScriptObject *)engine->GetModule(0)->GetAddressOfGlobalVar(engine->GetModule(0)->GetGlobalVarIndexByName("c"));

		if (mtd == 0 || obj == 0) TEST_FAILED;
		else
		{
			asIScriptContext *ctx = engine->CreateContext();
			ctx->Prepare(mtd);
			ctx->SetObject(obj);
			r = ctx->Execute();
			if (r != asEXECUTION_FINISHED) TEST_FAILED;
			ctx->Release();
		}

		type = engine->GetTypeInfoById(typeId);
		mtd = type->GetMethodByDecl("void func(int, int)");
		if (mtd == 0 || obj == 0) TEST_FAILED;
		else
		{
			asIScriptContext *ctx = engine->CreateContext();
			ctx->Prepare(mtd);
			ctx->SetObject(obj);
			ctx->SetArgDWord(0, 1);
			ctx->SetArgDWord(1, 1);
			r = ctx->Execute();
			if (r != asEXECUTION_FINISHED) TEST_FAILED;
			ctx->Release();
		}

		engine->Release();
	}

	//----------------------------
	// Verify that global functions and class methods with the same name doesn't conflict
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptAny(engine);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test4", script4, strlen(script4), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByDecl("void func()");
		if (func == 0) TEST_FAILED;

		engine->Release();
	}

	//----------------------------
	// Accessing member variables without this
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptAny(engine);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test5", script5, strlen(script5), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		r = ExecuteString(engine, "test()", mod);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//-----------------------------
	// Name conflict with class method and object type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptString(engine);
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test6", script6, strlen(script6), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		outbuffer = "";
		r = ExecuteString(engine, "Test t; t.Set(1); t.Test2();", mod);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
		}
		if (outbuffer != "Test::Set\nTest::Set\nSet::Set\n")
		{
			PRINTF("%s", outbuffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//------------------------------
	// The scope operator should permit calling global functions if the class has a method of the same name
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script =
			"class A { \n"
			"  void func() { \n"
			"    g = 0; \n"
			"    testScope(); \n"
			"    assert(g == 3); \n"
			"    ::testScope(); \n"
			"    assert(g == 2); \n"
			"  } \n"
			"  void testScope() { g = 3; } \n"
			"} \n"
			"void testScope() { g = 2; } \n"
			"int g; \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if (r < 0)
		{
			TEST_FAILED;
		}
		r = ExecuteString(engine, "A a; a.func(); assert( g == 2 );", mod);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
		}
		engine->Release();
	}

	//---------------------------
	// It should not be possible to declare a method with the same name as the class
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script = 
			"class A { \n"
			"  void A() {} \n"
			"} \n";
		mod->AddScriptSection("script", script);
		bout.buffer = "";
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (2, 3) : Error   : The method cannot be named with the class name\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// Test calling a virtual method on a class without setting the object
	// http://www.gamedev.net/topic/650268-getting-assert-upon-calling-script-function/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script = 
			"class A { \n"
			"  void Test() {} \n"
			"} \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(mod->GetTypeInfoByName("A"));
		if( obj == 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(obj->GetObjectType()->GetMethodByName("Test"));
		// "Forget" to call SetObject
		r = ctx->Execute();
		if( r == asEXECUTION_EXCEPTION )
		{
			if( strcmp(ctx->GetExceptionString(), "Null pointer access") != 0 )
			{
				PRINTF("%s", ctx->GetExceptionString());
				TEST_FAILED;
			}
		}
		else
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		ctx->Release();
		obj->Release();
		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

