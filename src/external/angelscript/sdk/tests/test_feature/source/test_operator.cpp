#include "utils.h"

namespace TestOperator
{


class AppVal
{
public:
	AppVal() { value = 0; }
	int value;

	bool operator == (const AppVal &o) const
	{
		return value == o.value;
	}
};

void Construct(AppVal *a)
{
	new(a) AppVal();
}

AppVal g_AppVal;
AppVal &GetAppValRef(AppVal &a)
{
	a.value = 1;
	return g_AppVal;
}

bool Test()
{
	bool fail = false;
	CBufferedOutStream bout;
	COutStream out;
	int r;
	asIScriptEngine *engine = 0;
	asIScriptModule *mod = 0;

	// opCall on element returned from array
	// http://www.gamedev.net/topic/658983-opcall-access-violation/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"  array<foo> bar(1); \n"
			"  bar[0](''); \n"
			"}; \n"
			"class foo { \n"
			"  void opCall(string) {} \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// opCall for global variable
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void global()\n"
			"{\n"
			"    bool ok=globalF(0);\n"
			"}\n"
			"class Functor\n"
			"{\n"
			"    bool opCall(double d)\n"
			"    {\n"
			"        return d!=0;\n"
			"    }\n"
			"};\n"
			"Functor globalF;\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// opCall on property accessor
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int i=0;\n"
			"class Functor\n"
			"{\n"
			"    void opCall()\n"
			"    {\n"
		//	"        print('opCall'+i+'\n');\n"
			"        i++;\n"
			"    }\n"
			"    void callThis()\n"
			"    {\n"
			"        this();\n"
			"    }\n"
			"};\n"
			"class Hybrid\n"
			"{\n"
			"    Functor f;\n"
			"    Functor g\n"
			"    {\n"
			"        get const\n"
			"        {\n"
			"            return f;\n"
			"        }\n"
			"    }\n"
			"    void callMember()\n"
			"    {\n"
			"        f();\n"
			"    }\n"
			"    void callMemberProp()\n"
			"    {\n"
			"        g();\n"
			"    }\n"
			"}\n"
			"Hybrid glob;\n"
			"void member()\n"
			"{\n"
			"    Hybrid local;\n"
			"    local.f();\n"
			"    local.callMember();\n"
			"    local.g();\n"
			"    local.callMemberProp();\n"
			"    glob.f();\n"
			"    glob.get_g()();\n"
			"    glob.g();\n"
			"    glob.callMember();\n"
			"    glob.callMemberProp();\n"
			"    Functor@ fPtr=glob.g;\n"
			"    fPtr();\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "member()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// opCall
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"class C {\n"
			"  int opCall(int a, int b) { return a + b; } \n"
			"  int test() { return this(2,3); } \n"
			"} \n"
			"class D {\n"
			"  string &opCall(const string &in a, const string &in b, const string &in c = 'hello') { val = a + b + c; return val; } \n"
			"  string val; \n"
			"} \n"
			"class E {\n"
			"  int opCall() { return 42; } \n"
			"} \n"
			"class F { \n"
			"  C c; \n"
			"  int func() { return c(2,3); } \n"
			"  C @get_Functor() property { return c; } \n"
			"} \n");
		
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Test opIndex with multiple args
		r = ExecuteString(engine, "C c; assert( c(2,3) == 5 ); assert( c.opCall(2,3) == 5 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test with objects as arguments, and returning references too
		// Test with default arguments
		r = ExecuteString(engine, "D d; assert( d('a', 'b') == 'abhello' ); assert( d.opCall('a', 'b') == 'abhello' ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test calling as member method, i.e. assert( f.c(2,3) == 5 );
		r = ExecuteString(engine, "F f; assert( f.c(2,3) == 5 ); assert( f.c.opCall(2,3) == 5 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test calling as member from within class method, i.e. f::func() { return c(2,3); }
		r = ExecuteString(engine, "F f; assert( f.func() == 5 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test calling it as post op, i.e. assert( getFunctor()(2,3) == 5 ); 
		r = ExecuteString(engine, "F f; \n"
								  "assert( f.Functor(2,3) == 5 ); \n"
			                      "assert( f.get_Functor()(2,3) == 5 ); \n"
								  "assert( f.get_Functor().opCall(2,3) == 5 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test calling the opCall with this()
		r = ExecuteString(engine, "C c; assert( c.test() == 5 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// opIndex with multiple values
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Test",
			"class C {\n"
			"  int opIndex(int a, int b) { return a + b; } \n"
			"} \n"
			"class D {\n"
			"  string &opIndex(const string &in a, const string &in b, const string &in c = 'hello') { val = a + b + c; return val; } \n"
			"  string val; \n"
			"} \n"
			"class E {\n"
			"  int opIndex() { return 42; } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Test opIndex with multiple args
		r = ExecuteString(engine, "C c; assert( c[2,3] == 5 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test with objects as arguments, and returning references too
		// Test with default arguments
		r = ExecuteString(engine, "D d; assert( d['a', 'b'] == 'abhello' ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test with zero arguments
		r = ExecuteString(engine, "E e; assert( e[] == 42 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Operator overloads
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class C\n"
			"{\n"
			"    C & opAssign (const C &in other)\n" 
			"    {\n"
			"        return this;\n"
			"    }\n"
			"    C & opAssign (const C @ other)\n" 
			"    {\n"
			"        return this;\n"
			"    }\n"
			"    int opCmp (const C &in other)\n" 
			"    {\n"
			"        return 0;\n"
			"    }\n"
			"    int opCmp (const C @ other)\n" 
			"    {\n"
			"        return 0;\n"
			"    }\n"
			"    void opAdd (const C &in other)\n" 
			"    {\n"
			"    }\n"
			"    void opAdd (const C @ other)\n" 
			"    {\n"
			"    }\n"
			"}\n"
			"void main (void)\n"
			"{\n"
			"    array<C> a1;\n"
			"    a1.insertLast(C());\n" // TODO: The compiler should ask the template if the function is OK
			"    a1.insertLast(C());\n"
			"    a1.sortAsc();\n"
			"    a1[0] = C();\n"
			"    C() + C();\n"
			"    if( a1[0] < a1[1] ) {}\n"
			"}\n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (26, 1) : Info    : Compiling void main()\n"
		                   "test (32, 11) : Error   : Found more than one matching operator\n"
		                   "test (32, 11) : Info    : C& C::opAssign(const C&in)\n"
		                   "test (32, 11) : Info    : C& C::opAssign(const C@)\n"
		                   "test (33, 9) : Error   : Found more than one matching operator\n"
		                   "test (33, 9) : Info    : void C::opAdd(const C&in)\n"
		                   "test (33, 9) : Info    : void C::opAdd(const C@)\n"
		                   "test (34, 15) : Error   : Found more than one matching operator\n"
		                   "test (34, 15) : Info    : int C::opCmp(const C&in)\n"
		                   "test (34, 15) : Info    : int C::opCmp(const C@)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//----------------------------------------------
	// opEquals for script classes
	//
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"class Test                         \n"
			"{                                  \n"
			"  int value;                       \n"
			// Define the operator ==
			"  bool opEquals(const Test &in o) const \n"
			"  {                                     \n"
			"    return value == o.value;            \n"
			"  }                                     \n"
			// The operator can be overloaded for different types
			"  bool opEquals(int o)             \n"
			"  {                                \n"
			"    return value == o;             \n"
			"  }                                \n"
			// opEquals that don't return bool are ignored
			"  int opEquals(float o)            \n"
			"  {                                \n"
			"    return 0;                      \n"
			"  }                                \n"
			"}                                  \n"
			"Test func()                        \n"
			"{                                  \n"
			"  Test a;                          \n"
			"  a.value = 0;                     \n"
			"  return a;                        \n"
			"}                                  \n"
			"Test @funcH()                      \n"
			"{                                  \n"
			"  Test a;                          \n"
			"  a.value = 0;                     \n"
			"  return @a;                       \n"
			"}                                  \n"
			"void main()                        \n"
			"{                                  \n"
			"  Test a,b,c;                      \n"
			"  a.value = 0;                     \n"
			"  b.value = 0;                     \n"
			"  c.value = 1;                     \n"
			"  assert( a == b );                \n"  // a.opEquals(b)
			"  assert( a.opEquals(b) );         \n"  // Same as a == b
			"  assert( a == 0 );                \n"  // a.opEquals(0)
			"  assert( 0 == a );                \n"  // a.opEquals(0)
			"  assert( a == 0.1f );             \n"  // a.opEquals(int(0.1f))
			"  assert( a != c );                \n"  // !a.opEquals(c)
			"  assert( a != 1 );                \n"  // !a.opEquals(1)
			"  assert( 1 != a );                \n"  // !a.opEquals(1)
			"  assert( !a.opEquals(c) );        \n"  // Same as a != c
			"  assert( a == func() );           \n"
			"  assert( a == funcH() );          \n"
			"  assert( func() == a );           \n"
			"  assert( funcH() == a );          \n"
			"}                                  \n";

		mod->AddScriptSection("script", script);

		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "script (29, 1) : Info    : Compiling void main()\n"
		                   "script (39, 16) : Warning : Implicit conversion of value is not exact\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test const correctness. opEquals(int) isn't const so it must not be allowed
		bout.buffer = "";
		r = ExecuteString(engine, "Test a; const Test @h = a; assert( h == 0 );", mod);
		if( r >= 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "ExecuteString (1, 38) : Error   : No conversion from 'const Test@&' to 'int' available.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
		}

		engine->Release();
	}

	//--------------------------------------------
	// opEquals for application classes
	//
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = engine->RegisterObjectType("AppVal", sizeof(AppVal), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("AppVal", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(Construct, (AppVal*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("AppVal", "bool opEquals(const AppVal &in) const", asMETHODPR(AppVal, operator ==, (const AppVal &) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectProperty("AppVal", "int value", asOFFSET(AppVal, value)); assert( r >= 0 );

		r = engine->RegisterGlobalFunction("AppVal &GetAppValRef(AppVal &out)", asFUNCTIONPR(GetAppValRef, (AppVal &), AppVal &), asCALL_CDECL); assert( r >= 0 );
		g_AppVal.value = 0;

		const char *script = 			
			"void main()                        \n"
			"{                                  \n"
			"  AppVal a,b,c;                    \n"
			"  a.value = 0;                     \n"
			"  b.value = 0;                     \n"
			"  c.value = 1;                     \n"
			"  assert( a == b );                \n"  // a.opEquals(b)
			"  assert( a.opEquals(b) );         \n"  // Same as a == b
			"  assert( a != c );                \n"  // !a.opEquals(c)
			"  assert( !a.opEquals(c) );        \n"  // Same as a != c
			"  assert( a == GetAppValRef(b) );  \n"
			"  assert( b == c );                \n"
			"  b.value = 0;                     \n"
			"  assert( GetAppValRef(b) == a );  \n"
			"  assert( c == b );                \n"
			"  assert( AppVal() == a );         \n"
			"}                                  \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//----------------------------------------------
	// opCmp for script classes
	//
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"class Test                         \n"
			"{                                  \n"
			"  int value;                       \n"
			// Define the operators ==, !=, <, <=, >, >=
			"  int opCmp(const Test &in o) const  \n"
			"  {                                  \n"
			"    return value - o.value;          \n"
			"  }                                  \n"
			// The operator can be overloaded for different types
			"  int opCmp(int o)                 \n"
			"  {                                \n"
			"    return value - o;              \n"
			"  }                                \n"
			// opCmp that don't return int are ignored
			"  bool opCmp(float o)              \n"
			"  {                                \n"
			"    return false;                  \n"
			"  }                                \n"
			"}                                  \n"
			"Test func()                        \n"
			"{                                  \n"
			"  Test a;                          \n"
			"  a.value = 0;                     \n"
			"  return a;                        \n"
			"}                                  \n"
			"Test @funcH()                      \n"
			"{                                  \n"
			"  Test a;                          \n"
			"  a.value = 0;                     \n"
			"  return @a;                       \n"
			"}                                  \n"
			"void main()                        \n"
			"{                                  \n"
			"  Test a,b,c;                      \n"
			"  a.value = 0;                     \n"
			"  b.value = 0;                     \n"
			"  c.value = 1;                     \n"
			"  assert( a == b );                \n"  // a.opCmp(b) == 0
			"  assert( a.opCmp(b) == 0 );       \n"  // Same as a == b
			"  assert( a == 0 );                \n"  // a.opCmp(0) == 0
			"  assert( 0 == a );                \n"  // a.opCmp(0) == 0
			"  assert( a == 0.1f );             \n"  // a.opCmp(int(0.1f) == 0 )
			"  assert( a != c );                \n"  // a.opCmp(c) != 0
			"  assert( a != 1 );                \n"  // a.opCmp(1) != 0
			"  assert( 1 != a );                \n"  // a.opCmp(1) != 0
			"  assert( a.opCmp(c) != 0 );       \n"  // Same as a != c
			"  assert( a == func() );           \n"
			"  assert( a == funcH() );          \n"
			"  assert( func() == a );           \n"
			"  assert( funcH() == a );          \n"
			"  assert( a < 10 );                \n"
			"  assert( 10 > a );                \n"
			"  assert( c > 0 );                 \n"
			"  assert( 0 < c );                 \n"
			"}                                  \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		// Test const correctness. opCmp(int) isn't const so it must not be allowed
		bout.buffer = "";
		r = ExecuteString(engine, "Test a; const Test @h = a; assert( h == 0 );", mod);
		if( r >= 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "ExecuteString (1, 38) : Error   : No conversion from 'const Test@&' to 'int' available.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
		}

		engine->Release();
	}

	//----------------------------------------------
	// Other dual operators for script classes
	//
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"class Test                         \n"
			"{                                  \n"
			"  int value;                       \n"
			// Define the operators 
			"  Test opAdd(const Test &in o) const \n" // ordinary operator
			"  {                                  \n"
			"    Test t;                          \n"
			"    t.value = value + o.value;       \n"
			"    return t;                        \n"
			"  }                                  \n"
			"  Test opMul_r(int o) const          \n" // reversed order arguments
			"  {                                  \n"
			"    Test t;                          \n"
			"    t.value = o * value;             \n"
			"    return t;                        \n"
			"  }                                  \n"
			"  Test @opShl(int o)                 \n" // Implementing a stream operator << 
			"  {                                  \n"
			"    value += o;                      \n"
			"    return this;                     \n"
			"  }                                  \n"
			"}                                  \n"
			"void main()                        \n"
			"{                                  \n"
			"  Test c;                          \n"
			"  c.value = 1;                     \n"
			"  assert( (c + c).value == 2 );      \n"  // c.opAdd(c).value == 2
			"  assert( c.opAdd(c).value == 2 );   \n"
			"  assert( (3 * c).value == 3 );      \n"  // c.opMul_r(3).value == 3
			"  assert( c.opMul_r(3).value == 3 ); \n"
			"  c << 1 << 2 << 3;                \n"
			"  assert( c.value == 7 );          \n"
			"}                                  \n";

		bout.buffer = "";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//----------------------------------------------
	// Assignment operators for script classes
	//
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"class Test                         \n"
			"{                                  \n"
			"  int value;                       \n"
			// Define the operators 
			"  Test@ opAssign(const Test &in o)   \n" 
			"  {                                  \n"
			"    value = o.value;                 \n"
			"    return this;                     \n"
			"  }                                  \n"
			"  Test@ opMulAssign(int o)           \n" 
			"  {                                  \n"
			"    value *= o;                      \n"
			"    return this;                     \n"
			"  }                                  \n"
			"}                                  \n"
			"void main()                        \n"
			"{                                  \n"
			"  Test a,c;                        \n"
			"  a.value = 0;                     \n"
			"  c.value = 1;                     \n"
			"  a = c;                           \n"
			"  assert( a.value == 1 );          \n"
			"  a.value = 2;                     \n"
			"  a *= 2;                          \n"
			"  assert( a.value == 4 );          \n"
			"}                                  \n";

		bout.buffer = "";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	//----------------------------------------------
	// Unary operators for script classes
	//
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char *script = 
			"class Test                         \n"
			"{                                  \n"
			"  int value;                       \n"
			// Define the operators 
			"  Test opNeg() const               \n"
			"  {                                \n"
			"    Test t;                        \n"
			"    t.value = -value;              \n"
			"    return t;                      \n"
			"  }                                \n"
			"  Test opCom()                     \n"
			"  {                                \n"
			"    Test t;                        \n"
			"    t.value = ~value;              \n"
			"    return t;                      \n"
			"  }                                \n"
			"  void opPostInc()                 \n"
			"  {                                \n"
			"    value++;                       \n"
			"  }                                \n"
			"  void opPreDec()                  \n"
			"  {                                \n"
			"    --value;                       \n"
			"  }                                \n"
			"}                                  \n"
			"void main()                        \n"
			"{                                  \n"
			"  Test a;                          \n"
			"  a.value = 1;                     \n"
			"  assert( (-a).value == -1 );      \n"
			"  assert( (~a).value == int(~1) ); \n"
			"  a++;                             \n"
			"  assert( a.value == 2 );          \n"
			"  --a;                             \n"
			"  assert( a.value == 1 );          \n"
			"}                                  \n";

		bout.buffer = "";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
		}
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		// Test const correctness.
		bout.buffer = "";
		r = ExecuteString(engine, "Test a; const Test @h = a; assert( (~h).value == ~1 ); h++; --h;", mod);
		if( r >= 0 )
		{
			TEST_FAILED;
		}
		if( bout.buffer != "ExecuteString (1, 37) : Error   : Function 'opCom() const' not found\n"
			               "ExecuteString (1, 57) : Error   : Function 'opPostInc() const' not found\n"
		                   "ExecuteString (1, 61) : Error   : Function 'opPreDec() const' not found\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
		}

		engine->Release();
	}

	// Success
	return fail;
}

}
