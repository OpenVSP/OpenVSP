#include "utils.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"

namespace TestExceptionMemory
{

static const char * const TESTNAME = "TestExceptionMemory";

static const char *script1 =
"void Test1()                   \n"
"{                              \n"
"  Object a;                    \n"
"  RaiseException();            \n"
"}                              \n"
"void Test2()                   \n"
"{                              \n"
"  RaiseException();            \n"
"  Object a;                    \n"
"}                              \n"
"void Test3()                   \n"
"{                              \n"
"  int a;                       \n"
"  Func(Object());              \n"
"}                              \n"
"void Func(Object a)            \n"
"{                              \n"
"  Object b;                    \n"
"  RaiseException();            \n"
"}                              \n"
"void Test4()                   \n"
"{                              \n"
"  Object a = SuspendObj();     \n"
"}                              \n"
"void Test5()                   \n"
"{                              \n"
"  Object a = ExceptionObj();   \n"
"}                              \n"
"void Test6()                   \n"
"{                              \n"
"  Object a(1);                 \n"
"}                              \n";
static const char *script2 =
"void Test7()                      \n"
"{                                 \n"
"  RefObj @a = @ExceptionHandle(); \n"
"}                                 \n";

static const char *script3 =
"class Pie                             \n"
"{                                     \n"
"	void foo() {}                      \n"
"}                                     \n"
"void calc()                           \n"
"{                                     \n"
"    Pie@ thing = null;                \n"
"    thing.foo(); // Null dereference  \n"
"}                                     \n";

static int CObject_constructCount = 0;
static int CObject_destructCount = 0;
class CObject
{
public:
	CObject() 
	{
		val = ('C' | ('O'<<8) | ('b'<<16) | ('j'<<24)); 
		mem = new int[1]; 
		*mem = ('M' | ('e'<<8) | ('m'<<16) | (' '<<24)); 
		//PRINTF("C: %x\n", this);
		CObject_constructCount++;
	}
	~CObject() 
	{
		delete[] mem; 
		//PRINTF("D: %x\n", this);
		CObject_destructCount++;
	}
	int val;
	int *mem;
};

void Assign_gen(asIScriptGeneric *)
{
	// Don't do anything
}

class CRefObject
{
public:
	CRefObject() {refCount = 1;}
	int AddRef() {return ++refCount;}
	int Release() {int r = --refCount; if( r == 0 ) delete this; return r;}
	int refCount;
};

void AddRef_gen(asIScriptGeneric*gen)
{
	CRefObject *o = (CRefObject*)gen->GetObject();
	o->AddRef();
}

void Release_gen(asIScriptGeneric *gen)
{
	CRefObject *o = (CRefObject*)gen->GetObject();
	o->Release();
}

CRefObject *RefObjFactory()
{
	return new CRefObject();
}

void RefObjFactory_gen(asIScriptGeneric *gen)
{
	*(CRefObject**)gen->GetAddressOfReturnLocation() = new CRefObject();
}

void Construct(CObject *o)
{
	new(o) CObject();
}

void CopyConstruct(const CObject &, CObject *o)
{
	new(o) CObject();
}

void Construct_gen(asIScriptGeneric *gen)
{
	CObject *o = (CObject*)gen->GetObject();
	new(o) CObject();
}

void Construct2(asIScriptGeneric *)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");
}

void Destruct(CObject *o)
{
	o->~CObject();
}

void Destruct_gen(asIScriptGeneric *gen)
{
	CObject *o = (CObject*)gen->GetObject();
	o->~CObject();
}

void RaiseException()
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");
}

CObject SuspendObj()
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->Suspend();

	return CObject();
}

void SuspendObj_gen(asIScriptGeneric*gen)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->Suspend();

	CObject obj;
	gen->SetReturnObject(&obj);
}

CObject ExceptionObj()
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");

	return CObject();
}

void ExceptionObj_gen(asIScriptGeneric *gen)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");

	CObject obj;
	gen->SetReturnObject(&obj);
}

CRefObject *ExceptionHandle()
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");

	return 0;
}

void ExceptionHandle_gen(asIScriptGeneric *gen)
{
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx ) ctx->SetException("application exception");

	gen->SetReturnObject(0);
}

std::string ReturnStringButException()
{
	if( !strstr(asGetLibraryOptions(), "AS_NO_EXCEPTIONS") )
		throw std::exception(); // random exception. AngelScript will catch all the same way
	else
	{
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx ) ctx->SetException("Caught an exception from the application");
	}

	return ""; // This is never returned so AngelScript has to properly handle the situation
}

void ReturnStringButException_generic(asIScriptGeneric *gen)
{
	// This call will throw an exception (unless library has been compiled with AS_NO_EXCEPTIONS)
	std::string str = ReturnStringButException();

	// Initialize the returned object
	new(gen->GetAddressOfReturnLocation()) std::string(str);
}

bool Test()
{
	bool fail = false;
	int r;
	int suspendId, exceptionId;

	// Test calling a function that throws an exception and has been registered with generic calling convention
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("string RetStrButExcept()", asFUNCTION(ReturnStringButException_generic), asCALL_GENERIC);

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "string str = RetStrButExcept()", 0, ctx);
		if (r != asEXECUTION_EXCEPTION)
			TEST_FAILED;
		else if (std::string(ctx->GetExceptionString()) != "Caught an exception from the application")
		{
			PRINTF("Got exception : %s\n", ctx->GetExceptionString());
			TEST_FAILED;
		}

		ctx->Release();
		engine->Release();
	}

	RET_ON_MAX_PORT

	// Test calling a function that throws an exception
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterStdString(engine);
		engine->RegisterGlobalFunction("string RetStrButExcept()", asFUNCTION(ReturnStringButException), asCALL_CDECL);

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "string str = RetStrButExcept()", 0, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		else if( std::string(ctx->GetExceptionString()) != "Caught an exception from the application" )
		{
			PRINTF("Got exception : %s\n", ctx->GetExceptionString());
			TEST_FAILED;
		}
	
		ctx->Release();
		engine->Release();
	}
	

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("Object", sizeof(CObject), asOBJ_VALUE | asOBJ_APP_CLASS_CD);	
	r = engine->RegisterObjectType("RefObj", sizeof(CRefObject), asOBJ_REF); assert(r >= 0);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		engine->RegisterObjectBehaviour("Object", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(Construct2), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Object", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Construct_gen), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Object", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destruct_gen), asCALL_GENERIC);
		engine->RegisterObjectMethod("Object", "Object &opAssign(const Object &in)", asFUNCTION(Assign_gen), asCALL_GENERIC);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_FACTORY, "RefObj@ f()", asFUNCTION(RefObjFactory_gen), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_ADDREF, "void f()", asFUNCTION(AddRef_gen), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_RELEASE, "void f()", asFUNCTION(Release_gen), asCALL_GENERIC); assert(r >= 0);
		engine->RegisterGlobalFunction("void RaiseException()", asFUNCTION(RaiseException), asCALL_GENERIC);
		suspendId   = engine->RegisterGlobalFunction("Object SuspendObj()", asFUNCTION(SuspendObj_gen), asCALL_GENERIC);
		exceptionId = engine->RegisterGlobalFunction("Object ExceptionObj()", asFUNCTION(ExceptionObj_gen), asCALL_GENERIC);
		engine->RegisterGlobalFunction("RefObj@ ExceptionHandle()", asFUNCTION(ExceptionHandle_gen), asCALL_GENERIC);
	}
	else
	{
		engine->RegisterObjectBehaviour("Object", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTION(Construct2), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("Object", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Construct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("Object", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("Object", "Object &opAssign(const Object &in)", asFUNCTION(Assign_gen), asCALL_GENERIC);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_FACTORY, "RefObj@ f()", asFUNCTION(RefObjFactory), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_ADDREF, "void f()", asMETHOD(CRefObject, AddRef), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("RefObj", asBEHAVE_RELEASE, "void f()", asMETHOD(CRefObject, Release), asCALL_THISCALL); assert(r >= 0);
		engine->RegisterGlobalFunction("void RaiseException()", asFUNCTION(RaiseException), asCALL_CDECL);
		suspendId   = engine->RegisterGlobalFunction("Object SuspendObj()", asFUNCTION(SuspendObj), asCALL_CDECL);
		exceptionId = engine->RegisterGlobalFunction("Object ExceptionObj()", asFUNCTION(ExceptionObj), asCALL_CDECL);
		engine->RegisterGlobalFunction("RefObj@ ExceptionHandle()", asFUNCTION(ExceptionHandle), asCALL_CDECL);
	}



	COutStream out;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	mod->AddScriptSection(TESTNAME, script2, strlen(script2), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}

	// The object has been initialized
	r = ExecuteString(engine, "Test1()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// The object has not yet been initialized
	r = ExecuteString(engine, "Test2()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// An object has been initialized and passed by value to function that throws exception
	r = ExecuteString(engine, "Test3()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// An object has been initialized and passed by value to a function, but 
	// the function cannot be called due to the stack being full
	if( strstr(asGetLibraryOptions(), "WIP_16BYTE_ALIGN") )
		engine->SetEngineProperty(asEP_MAX_STACK_SIZE, 16);
	else
		engine->SetEngineProperty(asEP_MAX_STACK_SIZE, sizeof(void*));
	r = ExecuteString(engine, "Test3()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// An object is allocated and initialized with a call to 
	// a function that returns an object by value. The function 
	// suspends the thread. The context is then aborted.
	asIScriptContext *ctx = engine->CreateContext();
	engine->SetEngineProperty(asEP_MAX_STACK_SIZE, 0);
	r = ExecuteString(engine, "Test4()", mod, ctx);
	if( r != asEXECUTION_SUSPENDED )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}
	ctx->Abort();
	ctx->Release();

	// An object is allocated and initialized with a call to 
	// a function that returns an object by value. The function 
	// sets a script exception.
	r = ExecuteString(engine, "Test5()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// The object constructor sets the exception
	r = ExecuteString(engine, "Test6()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// A function that is supposed to return a handle sets an exception
	r = ExecuteString(engine, "Test7()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

    // Attempt to call method on null class pointer
	mod->AddScriptSection("script", script3, strlen(script3));
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;
	r = ExecuteString(engine, "calc()", mod);
	if( r != asEXECUTION_EXCEPTION )
	{
		PRINTF("%s: Failed\n", TESTNAME);
		TEST_FAILED;
	}

	// Exception happens after value object has already been destroyed
	r = ExecuteString(engine, "{\n"
		                      "  Object o;\n"
                              "}\n"
							  "RaiseException();");
	if( r != asEXECUTION_EXCEPTION )
		TEST_FAILED;

	// Exception happens after the value object has been destroyed and, 
	// the same position would also be used again after the exception
	r = ExecuteString(engine, "{ Object o; } \n"
		                      "RaiseException(); \n"
							  "Object o; \n");
	if( r != asEXECUTION_EXCEPTION )
		TEST_FAILED;

	// The code has two places where the object is destroyed, one in the if case, and 
	// and one at the end of the function. If the code doesn't go in to the if case,
	// and the exception happens afterwards, the exception handler must not think the
	// object was already destroyed.
	r = ExecuteString(engine, "Object o; bool a = false; \n"
		                      "if( a ) return; \n"
							  "RaiseException(); \n");
	if( r != asEXECUTION_EXCEPTION )
		TEST_FAILED;

	// Calling a function that returns an object directly must release the object upon releasing the context
	ctx = engine->CreateContext();
	ctx->Prepare(engine->GetFunctionById(suspendId));
	ctx->Execute();
	ctx->Release();

	// Calling a function that returns an object but raised an exception shouldn't try to destroy the object
	ctx = engine->CreateContext();
	ctx->Prepare(engine->GetFunctionById(exceptionId));
	ctx->Execute();
	ctx->Release();

 	engine->Release();

	// Test proper release of function pointers on exception
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void RaiseException()", asFUNCTION(RaiseException), asCALL_CDECL);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"funcdef void func_t(); \n"
			"void main() \n"
			"{ \n"
			"  func_t @f = main; \n"
			"  RaiseException(); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		
		engine->Release();
	}

	// Test problem reported by FDsagizi
	// http://www.gamedev.net/topic/631801-pod-type-and-null-pointer-exception-bug-with-call-destructor/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("TestLink", sizeof(CObject), asOBJ_VALUE | asOBJ_APP_CLASS_CD);	
		engine->RegisterObjectBehaviour("TestLink", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Construct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("TestLink", asBEHAVE_CONSTRUCT, "void f(const TestLink &in)", asFUNCTION(CopyConstruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("TestLink", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destruct), asCALL_CDECL_OBJLAST);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Object3 \n"
			"{ \n"
			"    Object3( TestLink str ) \n"
			"    { \n"
			"        Object3 @null_object = null; \n"
			"        null_object.Do(); \n"
			"    } \n"
			"    void Do() {} \n"
			"} \n"
			"void Main() \n"
			"{ \n"
			"   Object3 @oo = Object3( TestLink() ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CObject_constructCount = 0;
		CObject_destructCount = 0;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "Main()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		if( CObject_constructCount != 2 ||
			CObject_destructCount != 1 )
			TEST_FAILED;

		ctx->Release();

		if( CObject_constructCount != 2 ||
			CObject_destructCount != 2 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		
		r = mod->LoadByteCode(&stream); 
		if( r < 0 )
			TEST_FAILED;

		CObject_constructCount = 0;
		CObject_destructCount = 0;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "Main()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		if( CObject_constructCount != 2 ||
			CObject_destructCount != 1 )
			TEST_FAILED;

		ctx->Release();

		engine->Release();
	}

	// Test failure when allocating very large array
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, true);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class test\n"
			"{\n"
			"  double x;\n"
			"  double y;\n"
			"  double z;\n"
			"  double other;\n"
			"}\n"
			"void main()\n"
			"{\n"
			"  test[] list(2000000000);\n"
			"}\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_EXCEPTION )
		{
			PRINTF("Return code is %d\n", r);
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test memory leak on exception
	// http://www.gamedev.net/topic/639243-funcdef-inside-shared-interface-interface-already-implement-warning/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void funcdef1( ); \n"
			"void end( ) {} \n"
			"interface ifuncdef1_2 \n"
			"{ \n"
			"    funcdef1@ f { get; set; } \n"
			"} \n"
			"void start() \n"
			"{ \n"
			"    ifuncdef1_2@ i; \n"
			"    @i.f = end; \n" // exception while calling set_f, as i is null
			"} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start();", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// Test memory leak on exception
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterScriptMathComplex(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void func( complex ); \n"
			"void start() \n"
			"{ \n"
			"    func @f; \n"
			"    f( complex(1,1) ); \n" // exception because f is null
			"} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start();", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// Test to make sure the exception handlers knows to clean up the memory in the list buffers
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class T { T @t; } \n"
			"void start() \n"
			"{ \n"
			"    T @nullVar; \n"
			"    array<T@> t = {T(), T(), T(), T(), nullVar.t}; \n"
			"} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start();", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

