#include "utils.h"

namespace TestObjHandle2
{

static const char * const TESTNAME = "TestObjHandle2";

static const char *script1 =
"void TestObjHandle()                   \n"
"{                                      \n"
"   refclass@ b = @getRefClass();       \n"
"   Assert(b.id == int(0xdeadc0de));    \n"
// Pass argument with explicit handle
"   refclass@ c = @getRefClass(@b);     \n"
"   Assert(@c == @b);                   \n"
// Pass argument with implicit handle
"   @c = @getRefClass(b);               \n"
"   Assert(@c == @b);                   \n"
// Pass argument with implicit in reference to handle
"   t(b);                               \n"
// Pass argument with explicit in reference to handle
"   t(@b);                              \n"
// Pass argument with implicit out reference to handle
"   s(b);                               \n"
// Pass argument with explicit out reference to handle
"   s(@b);                              \n"
// Handle assignment
"   @c = @b;                            \n"
"   @c = b;                             \n"
// Handle comparison        
"   @c == @b;                           \n"
"}                                      \n"
"void t(refclass@ &in a)                \n"
"{                                      \n"
"}                                      \n"
"void s(refclass@ &out a)               \n"
"{                                      \n"
"}                                      \n";

static const char *script2 =
"class A                                \n"
"{ int a; };                            \n"
"void Test()                            \n"
"{                                      \n"
"  A a, b;                              \n"
"  @a = b;                              \n"
"}                                      \n";

class CRefClass
{
public:
	CRefClass() 
	{
		id = 0xdeadc0de;
//		asIScriptContext *ctx = asGetActiveContext(); 
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber()); 
//		PRINTF("Construct(%X)\n",this); 
		refCount = 1;
	}
	~CRefClass() 
	{
//		asIScriptContext *ctx = asGetActiveContext(); 
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber()); 
//		PRINTF("Destruct(%X)\n",this);
	}
	int AddRef() 
	{
//		asIScriptContext *ctx = asGetActiveContext(); 
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber()); 
//		PRINTF("AddRef(%X)\n",this); 
		return ++refCount;
	}
	int Release() 
	{
//		asIScriptContext *ctx = asGetActiveContext(); 
//		PRINTF("ln:%d ", ctx->GetCurrentLineNumber()); 
//		PRINTF("Release(%X)\n",this); 
		int r = --refCount; 
		if( refCount == 0 ) delete this; 
		return r;
	}
	void Method()
	{
		// Some method
	}
	CRefClass &operator=(const CRefClass & /*o*/) {return *this;}
	int refCount;
	int id;
};

CRefClass c;
CRefClass *getRefClass() 
{
//	asIScriptContext *ctx = asGetActiveContext(); 
//	PRINTF("ln:%d ", ctx->GetCurrentLineNumber()); 
//	PRINTF("getRefClass() = %X\n", &c); 

	// Must add the reference before returning it
	c.AddRef();
	return &c;
}

CRefClass *getRefClass(CRefClass *obj)
{
	assert(obj != 0);
	return obj;
}

bool TestHandleMemberCalling(void);

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

	if( !TestHandleMemberCalling() )
		return true;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString_Generic(engine);

	r = engine->RegisterObjectType("refclass", sizeof(CRefClass), asOBJ_REF); assert(r >= 0);
	r = engine->RegisterObjectProperty("refclass", "int id", asOFFSET(CRefClass, id));
	r = engine->RegisterObjectBehaviour("refclass", asBEHAVE_ADDREF, "void f()", asMETHOD(CRefClass, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("refclass", asBEHAVE_RELEASE, "void f()", asMETHOD(CRefClass, Release), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("refclass", "refclass &opAssign(const refclass &in)", asMETHOD(CRefClass, operator=), asCALL_THISCALL); assert(r >= 0);
	
	r = engine->RegisterObjectMethod("refclass", "void Method()", asMETHOD(CRefClass, Method), asCALL_THISCALL); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("refclass @getRefClass()", asFUNCTIONPR(getRefClass,(),CRefClass*), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("refclass @getRefClass(refclass@)", asFUNCTIONPR(getRefClass,(CRefClass*),CRefClass*), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );


	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}
	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "TestObjHandle()", mod, ctx);

	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
		{
			int col;
			int row = ctx->GetExceptionLineNumber(&col);
			PRINTF("Exception\n");
			PRINTF("line: %d, %d\n", row, col);
			PRINTF("desc: %s\n", ctx->GetExceptionString());
		}

		TEST_FAILED;
	}
	if( ctx ) ctx->Release();

	// Verify that the compiler doesn't implicitly convert the lvalue in an assignment to a handle, 
	// nor convert the explicit handle on the rvalue to a reference
	CBufferedOutStream bout;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "refclass @a; a = @a;");
	// TODO: The message should mention that the conversion is not allowed due to the explicit handle
	if( r >= 0 || bout.buffer != "ExecuteString (1, 18) : Error   : Can't implicitly convert from 'refclass@' to 'refclass&'.\n" ) 
	{
		TEST_FAILED;
		PRINTF("%s", bout.buffer.c_str());
	}

	ctx = engine->CreateContext();
	r = ExecuteString(engine, "refclass@ a; a.Method();", 0, ctx);
	if( r != asEXECUTION_EXCEPTION )
	{
		TEST_FAILED;
	}
	if( ctx ) ctx->Release();


	engine->Release();

	// Verify that the compiler doesn't allow the use of handles if addref/release aren't registered
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->RegisterObjectType("type", 0, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	engine->RegisterGlobalFunction("type @func()", asFUNCTION(0), asCALL_CDECL);
	engine->Release();

	// Verify that it's not possible to do handle assignment to non object handles
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	r = mod->Build();
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "TestObjHandle2 (3, 1) : Info    : Compiling void Test()\n"
                       "TestObjHandle2 (6, 3) : Error   : Expression is not an l-value\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Success
	return fail;
}


//////////////////////////

const char *script3 = 
"class TestClass                            \n"
"{                                          \n"
"	ArgClass @m_arg;                        \n"
"   CallerClass @m_caller;                  \n"
"	TestClass()                             \n"
"	{                                       \n"
"		ArgClass _arg;                      \n"
"		_arg.SetWeight( 2.0f );             \n"
"		@m_arg = _arg;                      \n"
"		m_arg.SetWeight( 3.0f );            \n"
"		CallerClass _caller;                \n"
"		@m_caller = _caller;                \n"
"	}                                       \n"
"	void Test()                             \n"
"	{                                       \n"
"		Point pos(0.0f,0.0f,0.0f);          \n"
//////////////////////////////////////////////////
// UNCOMMENT THE NEXT TWO LINES TO MAKE IT WORK
//"		ArgClass @ap = m_arg;               \n"
//"		m_caller.DoSomething( ap, pos );    \n"
//////////////////////////////////////////////////
// THIS LINE DOESN'T WORK
"		m_caller.DoSomething( m_arg, pos ); \n"
//////////////////////////////////////////////////
"	}                                       \n"
"}                                          \n"
"void TestScript()                          \n"
"{                                          \n"
"	TestClass t;                            \n"
"	t.Test();                               \n"
"}                                          \n";

class ArgClass
{
public:
	ArgClass()
	{
		m_weight = 1.0f;
		m_refcount = 1;
	}
	void AddRef()
	{
		++m_refcount;
	}
	void Release()
	{
		--m_refcount;
		if( m_refcount > 0 )
			return;
		delete this;
	}

	static ArgClass *Factory()
	{
		return new ArgClass();
	}
	
	float GetWeight(void) const
	{
		return m_weight;
	}

	void SetWeight(float t)
	{
		m_weight = t;
	}

	float m_weight;
	int m_refcount;
};

class Point
{
public:
	Point()
	{
		m_x[0] = m_x[1] = m_x[2] = m_x[3] = -1.0f;
	}

	Point(float x,float y,float z)
	{
		m_x[0] = x;
		m_x[1] = y;
		m_x[2] = z;
		m_x[3] = 0.0f;
	}

	static void Construct(Point *self)
	{
		new(self) Point();
	}

	static void Construct2(float x,float y,float z,Point *self)
	{
		new(self) Point(x,y,z);
	}

	~Point()
	{
	}

	float m_x[4];
};

class CallerClass
{
public:
	CallerClass()
	{
		m_refcount = 1;
	}

	void AddRef()
	{
		++m_refcount;
	}

	void Release()
	{
		--m_refcount;
		if( m_refcount > 0 )
			return;
		delete this;
	}

	static CallerClass *Factory()
	{
		return new CallerClass();
	}

	void DoSomething(const ArgClass &arg, Point &)
	{
		float weight = arg.GetWeight();
		if( (weight > 2.9f) && (weight < 3.1f) )
		{
			m_success = true;
		}
	}

	int m_refcount;
	static bool m_success;
};

bool CallerClass::m_success = false;

bool TestHandleMemberCalling(void)
{
	int r;

	// initialize the test
	CallerClass::m_success = false;

	// create AngelScript
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	// setup output stream
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString_Generic(engine);
	r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

	// register Point
	r = engine->RegisterObjectType("Point", sizeof(Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Point", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Point::Construct), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Point", asBEHAVE_CONSTRUCT, "void f(float,float,float)", asFUNCTION(Point::Construct2), asCALL_CDECL_OBJLAST); assert(r >= 0);

	// register ArgClass
	r = engine->RegisterObjectType("ArgClass", sizeof(ArgClass), asOBJ_REF); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ArgClass", asBEHAVE_ADDREF, "void f()", asMETHOD(ArgClass, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ArgClass", asBEHAVE_RELEASE, "void f()", asMETHOD(ArgClass, Release), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ArgClass", asBEHAVE_FACTORY, "ArgClass@ f()", asFUNCTION(ArgClass::Factory), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterObjectMethod("ArgClass", "float GetWeight() const", asMETHOD(ArgClass,GetWeight), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("ArgClass", "void SetWeight(float)", asMETHOD(ArgClass,SetWeight), asCALL_THISCALL); assert(r >= 0);

	// register CallerClass
	r = engine->RegisterObjectType("CallerClass", sizeof(CallerClass), asOBJ_REF); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("CallerClass", asBEHAVE_ADDREF, "void f()", asMETHOD(CallerClass, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("CallerClass", asBEHAVE_RELEASE, "void f()", asMETHOD(CallerClass, Release), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("CallerClass", asBEHAVE_FACTORY, "CallerClass@ f()", asFUNCTION(CallerClass::Factory), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterObjectMethod("CallerClass", "void DoSomething(const ArgClass &inout, Point &out)", asMETHOD(CallerClass,DoSomething), asCALL_THISCALL); assert(r >= 0);

	// add our script
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	r = mod->AddScriptSection("script", script3, strlen(script3), 0); assert( r >= 0 );
	r = mod->Build();
	if( r < 0 )
	{
		return false;
	}

	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "TestScript()", mod, ctx);

	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
		{
			int col;
			int row = ctx->GetExceptionLineNumber(&col);
			PRINTF("Exception\n");
			PRINTF("line: %d, %d\n", row, col);
			PRINTF("desc: %s\n", ctx->GetExceptionString());
		}		
		PRINTF("%s: Execution failed\n", TESTNAME);
		return false;
	}

	if( ctx )
	{
		ctx->Release();
	}

	engine->Release();

	return CallerClass::m_success;
}

} // namespace

