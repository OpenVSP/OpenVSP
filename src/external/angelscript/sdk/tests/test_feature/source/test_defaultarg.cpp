#include "utils.h"
#include "../../add_on/scriptmath/scriptmathcomplex.h"

using namespace std;

namespace TestDefaultArg
{

//This function is registered as void StringTest(const string& in szString1, const string& in szString2 = "")
bool ComplexTest( const Complex& c1, const Complex& c2 )
{
	if( c1 != Complex(2,4) ) return false;
	if( c2 != Complex(1,1) ) return false;
	return true;
}

void *factory(int value, int offset) 
{
	assert(value == 123);
	assert(offset == 5000);

	return NULL;
}

int get_opIndex(int obj, int index) 
{
	assert(index == 10);

	return 123;
}

int get_None(int obj) 
{
	return 123;
}

bool Test()
{
	bool fail = false;
	int r;
	CBufferedOutStream bout;
	COutStream out;
	asIScriptModule *mod;
	asIScriptEngine *engine;

	// Test default arg after expression with index property accessor
	// Reported by Phong Ba
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("vObj", sizeof(int), asOBJ_VALUE | asOBJ_POD); assert(r >= 0);
		r = engine->RegisterObjectMethod("vObj", "int get_opIndex(int) const property", asFUNCTION(get_opIndex), asCALL_CDECL_OBJFIRST); assert(r >= 0);
		r = engine->RegisterObjectMethod("vObj", "int get_Prop(int) const property", asFUNCTION(get_opIndex), asCALL_CDECL_OBJFIRST); assert(r >= 0);
		r = engine->RegisterObjectMethod("vObj", "int get_None() const property", asFUNCTION(get_None), asCALL_CDECL_OBJFIRST); assert(r >= 0);
		r = engine->RegisterObjectMethod("vObj", "int getValue(int) const", asFUNCTION(get_opIndex), asCALL_CDECL_OBJFIRST); assert(r >= 0);

		r = engine->RegisterObjectType("rObj", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("rObj", asBEHAVE_FACTORY, "rObj@ f(int value, int offset = 5000)", asFUNCTION(factory), asCALL_CDECL); assert(r >= 0);

		{
			asIScriptModule  *mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);
			asIScriptContext *ctx = engine->CreateContext(); assert(ctx != NULL);

			r = mod->AddScriptSection("main", "void main() {vObj plain; rObj@ obj = rObj(plain.Prop[10]);}"); assert(r >= 0);
			r = mod->Build(); assert(r >= 0);

			r = ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void main()")); assert(r >= 0);
			r = ctx->Execute(); assert(r == asEXECUTION_FINISHED);

			ctx->Release();
			mod->Discard();
		}

		{
			asIScriptModule  *mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);
			asIScriptContext *ctx = engine->CreateContext(); assert(ctx != NULL);

			r = mod->AddScriptSection("main", "void main() {vObj plain; rObj@ obj = rObj(plain[10]);}"); assert(r >= 0);
			r = mod->Build(); assert(r >= 0); 

			r = ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void main()")); assert(r >= 0);
			r = ctx->Execute(); assert(r == asEXECUTION_FINISHED);

			ctx->Release();
			mod->Discard();
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// default arg accessing member of global var
	// Reported by Aaron Baker
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class foo \n"
			"{ \n"
			"	int bar() \n"
			"	{ \n"
			"		return 5; \n"
			"	} \n"
			"} \n"
			"foo f; \n"
			"void my_func(int age=f.bar()) \n"
			"{ \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"	my_func(); \n"
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
	
	// default arg with funcdef referring to global function
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void callback_definition(int x); \n"
			"void my_function(callback_definition@ callback = @default_callback) \n"
			"{ \n"
			"} \n"
			"void default_callback(int x) \n"
			"{ \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"	my_function(); \n"
			"	my_function(@default_callback); \n"
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

	// problem default args and string concatenation
	// The problem was that the default arg used a temporary variable that was also used to evaluate the first argument
	// http://www.gamedev.net/topic/663922-problem-with-default-arguments/
	SKIP_ON_MAX_PORT
	{
		const char *script = 
			"void test() \n"
			"{ \n"
			"	bool r = ComplexTest( complex(1,2) + complex(1,2) ); \n"
			"   assert( r ); \n"
			"} \n";
		
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptMathComplex(engine);

		engine->RegisterGlobalFunction( "bool ComplexTest(const complex& in c1, const complex& in c2 = complex(1,1))", asFUNCTION( ComplexTest ), asCALL_CDECL );

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "test(); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// default args should be compiled in the same namespace that the function was declared in
	// http://www.gamedev.net/topic/657430-default-parameters-within-namespace/
	{
		const char *script = 
			"namespace foo { \n"
			"  int global = 42; \n"
			"  void func(int var = global) {} \n"
			"} \n"
			"void main() { \n"
			"  foo::func(); \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
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

	// Test that compiler properly detects when two functions will conflict if the first non-default arguments equal
	{
		const char *script = 
			"class T { \n"
			"  void test() {} \n"
			"  void test(float a = 0) {} \n"
			"} \n"
			"class Base { \n"
			"  void test() {} \n"
			"} \n"
			"class Derived : Base { \n"
			"  void test(float a = 0) {} \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "test (3, 3) : Warning : The overloaded functions are identical on initial parameters without default arguments\n"
						   "test (3, 3) : Info    : void T::test(float = 0)\n"
						   "test (3, 3) : Info    : void T::test()\n"
						   "test (8, 7) : Warning : The overloaded functions are identical on initial parameters without default arguments\n"
						   "test (8, 7) : Info    : void Base::test()\n"
						   "test (8, 7) : Info    : void Derived::test(float = 0)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test that default arg uses global var instead of local
	// TODO: Should the default arg access the local variable first?
	{
		const char *script = 
			"int myvar = 42; \n"
			"void Function(int a, int b = myvar) { assert( b == 42 ); } \n"
			"void main() \n"
			"{ \n"
			"	int myvar = 1; \n"
			"	Function(1); \n"   // This will use the global myvar and not the local myvar
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test default arg that references class member
	// TODO: Should perhaps allow default args in class methods to access the class' members
	{
		const char *script = 
			"class monster \n"
			"{ \n"
			" int level; \n"
			" void calculate_necessary_experience(int level_to_check=this.level) \n"
			" { \n"
			"  level_to_check=1; \n"
			" } \n"
			" void act() \n"
			" { \n"
			"  calculate_necessary_experience(); \n"
			" } \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
		bout.buffer = "";
		r = mod->Build();
		if( r > 0 )
			TEST_FAILED;

		if( bout.buffer != "test (8, 2) : Info    : Compiling void monster::act()\n"
						   "default arg (1, 1) : Error   : No matching symbol 'this'\n"
						   "test (10, 3) : Error   : Failed while compiling default arg for parameter 0 in function 'void monster::calculate_necessary_experience(int = this . level)'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test default arg in constructors
	// http://www.gamedev.net/topic/646936-default-constructors-vs-default-arguments/
	{
		// This works in C++, and should work in AngelScript too
		const char *script = 
			"class Foo { \n"
			"  Foo(string arg = '23') { val = arg; } \n"
			"  Foo func() { Foo bar; return bar; } \n"
			"  Foo &opAssign(const Foo &in o) { val = o.val; return this; } \n"
			"  string val; \n"
			"} \n"
			"Foo bar; \n"
			"Foo bar2('2'); \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE); 
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		int idx = mod->GetGlobalVarIndexByName("bar");
		asIScriptObject *obj = (asIScriptObject*)mod->GetAddressOfGlobalVar(idx);
		if( *(string*)obj->GetAddressOfProperty(0) != "23" )
			TEST_FAILED;

		engine->Release();
	}

	// Test default arg and derived classes
	// Reported by Aaron Baker
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func(int a, int b) { \n"
			"  assert( a == 56 ); \n"
			"  assert( b == 78 ); \n"
			"} \n"
			"class Base { \n"
			"  void method(int a = 12, int b = 34) { func(a,b); } \n"
			"} \n"
			"class Derived : Base { \n"
			"  void method(int a = 56, int b = 78) { func(a,b); } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Derived d; d.method();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test memory leak with shared functions and default args
	// http://www.gamedev.net/topic/646826-crash-on-exit-binding-wrong/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"shared void global(int a = 42, int b = 82) {} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"shared void global(int a = 42, int b = 82) {} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Allow default args for anonymous parameters too
	// http://www.gamedev.net/topic/645049-nameless-arguments-cannot-have-default-values/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void func(int = 42) {} \n"
			"void main() { func(); } \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test default args with expressions enclosed in parenthesis
	// Reported by Aaron Baker
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void func(int arg = (23+4)) {} \n"
			"class myclass \n"
			"{ \n"
			"  void calculate_number(int hp_stat=(3+2)) {} \n"
			"} \n"
			"void main() { func(); \n"
			"  myclass c; c.calculate_number(); \n"
	        "} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test to make sure the default arg evaluates to a type that matches the function parameter
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterStdString(engine);

		bout.buffer = "";

		mod = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class ud_bool \n"
			"{ \n"
			"  bool value; \n"
			"  ud_bool() \n"
			"  { \n"
			"    value = false; \n"
			"  } \n"
			"  void  opAssign(bool data) \n"
			"  { \n"
			"    this.value = data; \n"
			"  } \n"
			"  bool opEquals(bool data) \n"
			"  { \n"
			"    if(this.value == data) \n"
			"      return true; \n"
			"    return false; \n"
			"  } \n"
			"} \n"
			"ud_bool kill_object; \n"
			"ud_bool@ kill=@kill_object; \n"
			"void main() \n"
			"{ \n"
			"  kill_all(); \n"
			"} \n"
			"void kill_all(bool only_you=kill) \n"
			"{ \n"
			"}\n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (21, 1) : Info    : Compiling void main()\n"
		                   "default arg (1, 1) : Error   : The type of the default argument expression doesn't match the function parameter type\n"
		                   "test (23, 3) : Error   : Failed while compiling default arg for parameter 0 in function 'void kill_all(bool = kill)'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test calling a function with default arg where the expression uses a global var
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"string g = 'global';\n"
			"void func(string a, string b = 'pre' + g + 'pos') \n"
			"{ \n"
			"  array<string> items; \n"
			"  assert(a == 'afirst'); \n"
			"  assert(b == 'preglobalpos'); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "array<string> arr = {'first', 'second'}; func('a'+arr[0]);\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test calling a function with default argument
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptString(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"void func(int b, const string &in a = 'default') \n"
			"{ \n"
			"  if( b == 0 ) \n"
			"    assert( a == 'default' ); \n"
			"  else  \n"
			"    assert( a == 'test' ); \n"
			"} \n" 
			"void main() \n"
			"{ \n"
			"  func(0); \n"
			"  func(0, 'default'); \n"
			"  func(1, 'test'); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Must be possible to register functions with default args as well
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = engine->RegisterGlobalFunction("void defarg(bool, int = 34 + /* comments will be removed *""/ 45, int = 23)", asFUNCTION(0), asCALL_GENERIC);
		if( r < 0 )
			TEST_FAILED;
		asIScriptFunction *func = engine->GetFunctionById(r);
		string decl = func->GetDeclaration();
		if( decl != "void defarg(bool, int = 34 + 45, int = 23)" )
		{
			PRINTF("%s\n", decl.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// When default arg is used, all other args after that must have default args
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = engine->RegisterGlobalFunction("void defarg(bool, int a = 34+45, int)", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "System function (1, 1) : Error   : All subsequent parameters after the first default value must have default values in function 'void defarg(bool, int = 34 + 45, int)'\n"
			               " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void defarg(bool, int a = 34+45, int)' (Code: asINVALID_DECLARATION, -10)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// Shouldn't be possible to write default arg expressions that access local variables, globals are ok though
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"void func(int a = n) {} \n"
			"void main() \n"
			"{ \n"
			"  int n; \n"
			"  func(); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (2, 1) : Info    : Compiling void main()\n"
		                   "default arg (1, 1) : Error   : No matching symbol 'n'\n"
		                   "script (5, 3) : Error   : Failed while compiling default arg for parameter 0 in function 'void func(int = n)'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Default args in script class constructors
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"class T \n"
			"{ \n"
			"  T(int a, int b = 25) \n"
			"  { \n"
			"    assert(a == 10); \n"
			"    assert(b == 25); \n"
			"  } \n"
			"} \n" 
			"T g(10); \n"
			"void main() \n"
			"{ \n"
			"  T(10); \n"
			"  T l(10); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Default arg must not end up using variables that are used 
	// in previously compiled variables as temporaries
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"void func(uint8 a, string b = 'b') \n"
			"{ \n"
			"  assert( a == 97 ); \n"
			"  assert( b == 'b' ); \n"
			"} \n" 
			"void main() \n"
			"{ \n"
			"  uint8 a; \n"
			"  func((a = 'a'[0])); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Shouldn't crash if attempting to call incorrect function
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script =
			"void myFunc( float f, int a=0, int b ) {} \n"
			"void main() \n"
			"{ \n"
			"  int n; \n"
			"  myFunc( 1.2, 6 ); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (1, 1) : Error   : All subsequent parameters after the first default value must have default values in function 'void myFunc(float, int = 0, int)'\n"
					/*	   "script (2, 1) : Info    : Compiling void main()\n"
						   "script (5, 3) : Error   : No matching signatures to 'myFunc(const double, const int)'\n"
						   "script (5, 3) : Info    : Candidates are:\n"
						   "script (5, 3) : Info    : void myFunc(float f, int a = 0, int b)\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test that default argument expression can access index in global array
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptArray(engine, true);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"int[] my_array(5);\n"
			"int index=3;\n"
			"void my_function(int arg1, int arg2=my_array[index])\n"
			"{\n"
			"  assert( arg2 == 42 ); \n"
			"}\n"
			"void main()\n"
			"{\n"
			"  my_array[index] = 42;\n"
			"  my_function(1);\n"
			"}\n";

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


	// Test invalid default argument expression
	// Reported by Philip Bennefall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptArray(engine, true);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"void my_function(int arg1, int arg2=my_array[i[])\n"
			"{\n"
			"}\n"
			"void main()\n"
			"{\n"
			"  my_function(1);\n"
			"}\n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "script (4, 1) : Info    : Compiling void main()\n"
						   "default arg (1, 17) : Error   : Expected ']'\n"
						   "default arg (1, 17) : Error   : Instead found '<end of file>'\n"
						   "script (6, 3) : Error   : Failed while compiling default arg for parameter 1 in function 'void my_function(int, int = my_array [ i [ ])'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test use of class constructor in default argument
	// Reported by Thomas Grip
	SKIP_ON_MAX_PORT
	{
		const char *script = 
			"void MyFunc(const complex &in avX = complex(1,1)){}";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterScriptMathComplex(engine);

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

	// The test to make sure the saved bytecode keeps the default args is done in test_saveload.cpp
	// A test to make sure script class methods with default args work is done in test_saveload.cpp

	// TODO: The compilation of the default args must not add any LINE instructions in the byte code, because they wouldn't match the real script

	// Success
	return fail;
}



} // namespace

