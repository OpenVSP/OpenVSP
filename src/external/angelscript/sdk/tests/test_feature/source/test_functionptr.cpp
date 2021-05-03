#include "utils.h"
#include <sstream>
#include "../../../add_on/scripthelper/scripthelper.h"
#include "../../../add_on/scriptstdstring/scriptstdstring.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include "../../../add_on/scriptdictionary/scriptdictionary.h"

namespace TestFunctionPtr
{

bool receivedFuncPtrIsOK = false;

void ReceiveFuncPtr(asIScriptFunction *funcPtr)
{
	if( funcPtr == 0 ) return;

	if( strcmp(funcPtr->GetName(), "test") == 0 ) 
		receivedFuncPtrIsOK = true;

	funcPtr->Release();
}

class Obj
{
public:
	Obj() { func = 0; }
	~Obj() { if (func) func->Release(); }
	asIScriptFunction *opCast() 
	{
		if (func) func->AddRef();
		return func;
	}

	asIScriptFunction *func;
};

asIScriptFunction *cb = 0;
void *cbo = 0;
asITypeInfo *cbot = 0;
void SetCB(asIScriptGeneric *gen)
{
	if( cb )
		cb->Release();
	if( cbo )
	{
		gen->GetEngine()->ReleaseScriptObject(cbo, cbot);
		cbo = cbot = 0;
	}
	asIScriptFunction *f = (asIScriptFunction*)gen->GetArgAddress(0);
	
	// For 2.32.2 and earlier, the generic calling convention automatically released all 
	// handles received, so to compensate for that it would be necessary to add a ref if 
	// the handle should be kept
	if( gen->GetEngine()->GetEngineProperty(asEP_GENERIC_CALL_MODE) == 0 )
		f->AddRef();

	if( f && f->GetFuncType() == asFUNC_DELEGATE )
	{
		cbo = f->GetDelegateObject();
		cbot = f->GetDelegateObjectType();
		cb = f->GetDelegateFunction();
		
		// Hold on to the objects
		gen->GetEngine()->AddRefScriptObject(cbo, cbot);
		cb->AddRef();
		
		// Release the delegate, which we don't need
		f->Release();
	}
	else
	{
		cb = f;
	}
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;
	asIScriptEngine *engine;
	asIScriptModule *mod;
	asIScriptContext *ctx;
	CBufferedOutStream bout;

	// Test anonymous functions in delegate
	// Reported by Phong Ba
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		asIScriptModule* shareMod = NULL;

		//Module 1 
		shareMod = engine->GetModule("shared", asGM_ALWAYS_CREATE); assert(shareMod != NULL);

		r = shareMod->AddScriptSection("main",
			"funcdef void SimpleCallback(); \n"

			"//shared void Simple() {} \n"

			"void InvokeSimple() { \n"
			"	//SimpleCallback@ cb = SimpleCallback(Simple); \n" //No error
			"	SimpleCallback@ cb = SimpleCallback(function() {}); \n" //Error caused by the anonymous function

			"	cb(); \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"	InvokeSimple(); \n"
			"} \n"); assert(r >= 0);
		r = shareMod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptContext* ctx = engine->CreateContext();
		r = ctx->Prepare(shareMod->GetFunctionByDecl("void main()")); assert(r >= 0);
		r = ctx->Execute(); assert(r == asEXECUTION_FINISHED);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		shareMod->Discard();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test member funcdefs
	// Reported by Polyak Istvan
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class C \n"
			"{ \n"
			"    funcdef void fd (); \n"
			"    void f1 () \n"
			"    { \n"
			"    } \n"
			"    void f2 () \n"
			"    { \n"
			"        fd @ fd1 = C::fd(f1); \n"
			"        fd @ fd2 = fd(f1);  \n" // <- bug: without C:: the compiler don't find the funcdef
			"    } \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}
	
	// Test delegates as callbacks
	// https://www.gamedev.net/forums/topic/705573-asassert-abort-on-delegate-early-destruction-after-release/5420521/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_GENERIC_CALL_MODE, 1);
		
		engine->RegisterFuncdef("void CB()");
		engine->RegisterGlobalFunction("void SetCB(CB @)", asFUNCTION(SetCB), asCALL_GENERIC);
		
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
		"class Test { \n"
		"  void cb() { \n"
		"  } \n"
		"  int cbCount = 0; \n"
		"  Test() { \n"
		"    SetCB(CB(cb)); \n"
		"  } \n"
		"} \n"
		"Test t; \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		if( cb == 0 || cbo == 0 || cbot == 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		ctx->Prepare(cb);
		ctx->SetObject(cbo);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		if( cb )
		{
			cb->Release();
			cb = 0;
		}
		
		if( cbo )
		{
			engine->ReleaseScriptObject(cbo, cbot);
			cbo = cbot = 0;
		}

		engine->ShutDownAndRelease();
	}

	// Test funcdef and shared entitites
	// http://www.gamedev.net/topic/681021-crash-on-closing-modules-with-shared-and-funcdef/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"funcdef void functype();\n"
			"shared class classname {}\n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"funcdef void functype();\n"
			"shared class classname {}\n"
			"classname classinstance;\n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		mod1->Discard();
		mod2->Discard();

		engine->ShutDownAndRelease();
	}

	// Test passing function pointer to script function, and returning function pointer from script function
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterFuncdef("void FUNC()");
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() {} \n"
			"FUNC @foo(int a, FUNC @b) \n"
			"{ \n"
			"  assert( a == 42 ); \n"
			"  assert( b is func ); \n"
			"  return b; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByName("foo"));

		asIScriptFunction *func = mod->GetFunctionByName("func");

		// ctx->SetArgDWord should fail, since the argument is a funcdef
		r = ctx->SetArgDWord(1, 42);
		if (r != asINVALID_TYPE)
			TEST_FAILED;

		ctx->Prepare(mod->GetFunctionByName("foo"));

		// SetArgAddress should fail when the argument is not an object or funcdef
		r = ctx->SetArgAddress(0, func);
		if (r != asINVALID_TYPE)
			TEST_FAILED;

		ctx->Prepare(mod->GetFunctionByName("foo"));

		r = ctx->SetArgDWord(0, 42);
		if (r != 0)
			TEST_FAILED;

		// SetArgAddress doesn't increment the refcount
		r = ctx->SetArgAddress(1, func);
		if (r != 0)
			TEST_FAILED;

		// Make sure GetAddressOfArg works, and that SetArgAddress really set the value
		if (*((asIScriptFunction**)ctx->GetAddressOfArg(1)) != func)
			TEST_FAILED;

		// Clear the value to test SetArgObject
		*((asIScriptFunction**)ctx->GetAddressOfArg(1)) = 0;

		// SetArgObject increments the refcount
		r = ctx->SetArgObject(1, func);
		if (r != 0)
			TEST_FAILED;

		if (*((asIScriptFunction**)ctx->GetAddressOfArg(1)) != func)
			TEST_FAILED;

		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// ctx->GetAddressOfReturnValue should work
		if (*((asIScriptFunction**)ctx->GetAddressOfReturnValue()) != func)
			TEST_FAILED;

		// ctx->GetReturnDWord should fail, since the return value is a funcdef
		r = (int)ctx->GetReturnDWord();
		if (r != 0)
			TEST_FAILED;

		// ctx->GetReturnAddress should work
		if (((asIScriptFunction*)ctx->GetReturnAddress()) != func)
			TEST_FAILED;

		// ctx->GetReturnObject should work
		if (((asIScriptFunction*)ctx->GetReturnObject()) != func)
			TEST_FAILED;
		
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Allow a type registered with asOBJ_NOHANDLE to register opCast methods 
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterFuncdef("void func()");
		engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectMethod("Object", "func @opCast()", asMETHOD(Obj, opCast), asCALL_THISCALL);
		engine->RegisterObjectProperty("Object", "func @f", asOFFSET(Obj, func));

		Obj o;
		engine->RegisterGlobalProperty("Object obj", &o);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
		"void main() { \n"
		"	assert( cast<func@>(obj) is null ); \n"
		"   @obj.f = main; \n"
		"	assert( cast<func@>(obj) !is null ); \n"
		"} \n");
		r = mod->Build();
		if (r < 0)
		TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
		TEST_FAILED;

		// Release the function before the engine to avoid complaints from GC
		if (o.func)
		{
			o.func->Release();
			o.func = 0;
		}

		engine->ShutDownAndRelease();
	}

	// Test function pointers in initialization lists
	// http://www.gamedev.net/topic/678333-access-violation-when-passing-function-to-generic-initialization-list/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDictionary(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef bool Callback(int, int); \n"
			"bool myGreaterThan(int a, int b) { \n"
			"	return a > b; \n"
			"} \n"
			"bool myEquals(int a, int b) { \n"
			"	return a == b; \n"
			"} \n"
			"bool myLessThan(int a, int b) { \n"
			"	return a < b; \n"
			"} \n"
			"dictionary ops = { \n"
			"  { 'gt', @myGreaterThan}, \n"
			"  { 'lt', @myLessThan}, \n"
			"  { 'eq', @myEquals} \n"
			"}; \n"
			"dictionary ops2 = { \n"
			"  { 'gt', cast<Callback>(myGreaterThan) }, \n"
			"  { 'lt', cast<Callback>(myLessThan) }, \n"
			"  { 'eq', cast<Callback>(myEquals) } \n"
			"}; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "assert( cast<Callback>(ops['gt']) is myGreaterThan );", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test returning function pointer from registered class method
	// http://www.gamedev.net/topic/678317-incorrect-results-from-functions-returning-function-handles/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterFuncdef("void func()");
		engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("Object", "func @opCast()", asMETHOD(Obj, opCast), asCALL_THISCALL);
		engine->RegisterObjectProperty("Object", "func @f", asOFFSET(Obj, func));

		Obj o;
		engine->RegisterGlobalProperty("Object obj", &o);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"	assert( cast<func@>(obj) is null ); \n"
			"   @obj.f = main; \n"
			"	assert( cast<func@>(obj) !is null ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Release the function before the engine to avoid complaints from GC
		if (o.func)
		{
			o.func->Release();
			o.func = 0;
		}

		engine->ShutDownAndRelease();
	}

	// Create anonymous function from within class method
	// This caused error in asCByteCode::DebugOutput
	// Problem reported by Phong Ba
	{
		engine = asCreateScriptEngine();

		asIScriptModule *module = engine->GetModule("testCallback", asGM_ALWAYS_CREATE);
		asIScriptContext *context = engine->CreateContext();

		r = module->AddScriptSection("test",
			"funcdef void CALLBACK();"

			"class Test {"
			"	void set_Callback(CALLBACK@ handler) property"
			"	{"
			"	}"
			"}"

			"class ErrHere {"
			"	Test@ obj = null;"
			"	ErrHere()"
			"	{"
			"		@obj = Test();"
			"		@obj.Callback = function() {};" //<== Assertion failed: file, file ..\..\source\as_bytecode.cpp, line 2082
			"	}"
			"}"

			"void main(){ErrHere@ err = ErrHere();}"
			);
		assert(r >= 0);

		r = module->Build(); 
		if (r < 0)
			TEST_FAILED;
		r = context->Prepare(module->GetFunctionByName("main")); assert(r >= 0);
		r = context->Execute(); 
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		context->Release();
		module->Discard();

		engine->ShutDownAndRelease();
	}

	// Test passing function pointer by reference
	// http://www.gamedev.net/topic/676566-assert-failure-when-passing-function-handle-by-reference/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void f(); \n"
			"class foo { \n"
			"	void opAssign(f@ &in) { correct = true; } \n"
			"   bool correct = false; \n"
			"} \n"
			"void main() { \n"
			"	foo bar = main; \n"
			"   assert( bar.correct ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test function overloads and function pointers
	// http://www.gamedev.net/topic/676565-assert-failure-during-overload-resolution/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void f();\n"
			"class foo {\n"
			"  void opAssign(f@) {}\n"
			"  void opAssign(int) { correct = true; }\n"
			"  bool correct = false; \n"
			"}\n"
			"void main() {\n"
			"  foo bar = 1;\n"
			"  assert( bar.correct ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test function pointers in array
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void FD (void);\n"
			"void f(void)\n"
			"{\n"
			"}\n"
			"void main1(void)\n"
			"{\n"
			"	array<FD @> a;\n"
			"	a.insertLast(f);\n"
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main1()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef void FD (void);\n"
			"void f(void)\n"
			"{\n"
			"}\n"
			"void main2(void)\n"
			"{\n"
			"	array<FD> a;\n" // shouldn't be allowed as it would mean value copies of functions
			"	a.insertLast(f);\n"
			"}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test (5, 1) : Info    : Compiling void main2()\n"
						   "array (0, 0) : Error   : The subtype has no default factory\n"
						   "test (7, 8) : Error   : Attempting to instantiate invalid template type 'array<FD>'\n"
						   "test (8, 3) : Warning : 'a' is not initialized.\n"
						   "test (8, 3) : Error   : Illegal operation on 'int'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test function pointers in ternary conditions
	// http://www.gamedev.net/topic/672565-conditional-operator-is-unusable-with-functions/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef fd@ fd();\n"
			"fd@ f() {\n"
			"	return true ? f : f;\n" // should work
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"int g(int) {\n"
			"return 0;\n"
			"}\n"
			"void f() {\n"
			"	true ? f : g;\n" // must detect that the signature differs
			"}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (4, 1) : Info    : Compiling void f()\n"
						   "test (5, 2) : Error   : Can't find unambiguous implicit conversion to make both expressions have the same type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"int f(int) {\n"
			"return 0;\n"
			"}\n"
			"void f() {\n"
			"	true ? f : f;\n" // must detect that there are multiple matches for f
			"}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (4, 1) : Info    : Compiling void f()\n"
						   "test (5, 9) : Error   : Multiple matching signatures to '::f'\n"
						   "test (5, 13) : Error   : Multiple matching signatures to '::f'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test declaring funcdefs as members of classes
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class MyObj { \n"
			"  funcdef void Callback(); \n"            // Allow declaring funcdef as member
			"  void Method(Callback@ cb) { \n"         // The class should see its own funcdef without need for scope
			"    cb(); \n"
			"    Callback @cb2 = cb; \n"               // The class should see its own funcdef without need for scope
			"  } \n"
			"  Callback @cb; \n"                       // The class should see its own funcdef without need for scope
			"} \n"
			"bool called = false; \n"
			"void main() { \n"
			"  MyObj test; \n"
			"  MyObj::Callback @cb = Function; \n"     // Using the class name as scope can be used to identify the funcdef
			"  test.Method(cb); \n"
			"  assert( called ); \n"
			"} \n"
			"void Function() { called = true; } \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Saving and loading should also work
		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if (r < 0)
			TEST_FAILED;

		r = mod->LoadByteCode(&stream);
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Test name conflict within class (funcdef vs funcdef, funcdef vs property, funcdef vs method)
		bout.buffer = "";
		mod->AddScriptSection("test",
			"class MyObj { \n"
			"  funcdef void a(); \n"
			"  int a; \n"              // conflicts with first funcdef
			"  void b() {} \n"         // conflicts with next funcdef
			"  funcdef void b(); \n"
			"  funcdef void a(); \n"   // conflicts with first funcdef
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test (6, 11) : Error   : Name conflict. 'a' is a funcdef.\n"
			"test (4, 3) : Error   : Name conflict. 'b' is a funcdef.\n"
		/*	"test (3, 7) : Error   : Name conflict. 'a' is a funcdef.\n"*/)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test shared class with child funcdef. The funcdef must be shared automatically too
		bout.buffer = "";
		mod->AddScriptSection("test1",
			"shared class MyObj { funcdef void CB(); } \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test2",
			"shared class MyObj { } \n"  // The shared class should automatically get the funcdef
			"MyObj::CB @c; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test2",
			"shared class MyObj { funcdef int CB(int); } \n"); // TODO: The compiler should detect that this is different
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->DiscardModule("test1");
		engine->DiscardModule("test2");

		// Test inheriting from class with child funcdef
		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test2",
			"class Base { funcdef void A(); } \n"
			"class Derived : Base { \n"
			"  A @GetCallback() { return a; } \n" // should see the funcdef declared in base class
			"  A @a; \n"
			"} \n"
			"void main() { \n"
			"  Derived::A @a; \n" // should see the funcdef declared in base class
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test enumerating child types of MyObj (it must be possible to find the Callback. The declaration of the funcdef must be void MyObj::Callback())
		asITypeInfo *ot = mod->GetTypeInfoByName("Base");
		if (ot == 0 || ot->GetChildFuncdefCount() != 1 ||
			ot->GetChildFuncdef(0) == 0 ||
			std::string(ot->GetChildFuncdef(0)->GetFuncdefSignature()->GetDeclaration()) != "void Base::A()")
			TEST_FAILED;
		ot = mod->GetTypeInfoByName("Derived");
		if (ot == 0 || ot->GetChildFuncdefCount() != 0)
			TEST_FAILED;

		// Test appropriate error when the child type doesn't exist
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"class MyObj  { \n"
			"  funcdef void A(); \n"
			"} \n"
			"void main() { \n"
			"  MyObj::B @a; \n" // wrong child type
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test2 (4, 1) : Info    : Compiling void main()\n"
			"test2 (5, 10) : Error   : Identifier 'B' is not a data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test private and protected funcdefs (currently not supported)
		// TODO: The error message could be better
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"class MyObj  { \n"
			"  private funcdef void A(); \n"
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test2 (2, 3) : Error   : Expected method or property\n"
			"test2 (2, 3) : Error   : Instead found reserved keyword 'private'\n"
			"test2 (3, 1) : Error   : Unexpected token '}'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test that it is possible to find the type MyObj::Callback when MyObj is not declared in global namespace
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"namespace Boo { \n"
			"  class MyObj  { \n"
			"    funcdef void A(); \n"
			"  } \n"
			"  MyObj::A @b; \n"
			"} \n"
			"Boo::MyObj::A @a; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test that the child funcdef can use as returntype or parameter other child funcdefs of the same class without informing scope
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"class MyObj  { \n"
			"  funcdef A@ B(); \n"  // This should be able to see the A funcdef too
			"  funcdef void A(); \n"
			"} \n"
			"MyObj::B @b; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test funcdef in mixin class (currently not supported)
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"mixin class MyMix  { \n"
			"  funcdef void A(); \n"
			"} \n"
			"class MyObj : MyMix {}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test2 (2, 11) : Error   : Mixin classes cannot have child types\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test funcdef in interface (currently not supported)
		// TODO: Error message should be improved
		bout.buffer = "";
		mod->AddScriptSection("test2",
			"interface MyMix  { \n"
			"  funcdef void A(); \n"
			"} \n"
			"class MyObj : MyMix {}\n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test2 (2, 3) : Error   : Expected data type\n"
			"test2 (2, 3) : Error   : Instead found reserved keyword 'funcdef'\n"
			"test2 (3, 1) : Error   : Unexpected token '}'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test name conflict when derived class declares same funcdef as present in base class
		bout.buffer = "";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test2",
			"class Base { \n"
			"  A @GetCalback() { return null; } \n"
			"  funcdef void A(); \n"
			"} \n"
			"class Derived : Base { \n"
			"  A @GetCallback() override { return a; } \n"
			"  funcdef int A(int); \n"
			"  A @a; \n"
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "test2 (5, 7) : Error   : Method 'Derived::A@ Derived::GetCallback()' marked as override but does not replace any base class or interface method\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test registering funcdef as child of application type: RegisterFuncdef("void MyObj::Callback()")
		bout.buffer = "";
		engine->RegisterObjectType("MyType", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterFuncdef("void MyType::Callback()");
		engine->RegisterObjectMethod("MyType", "void SetCallback(Callback @+)", asFUNCTION(0), asCALL_GENERIC);
		mod->AddScriptSection("test", "MyType::Callback @cb;\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test GetTypeIdByDecl
		int typeId = engine->GetTypeIdByDecl("MyType::Callback");
		if (typeId < 0)
			TEST_FAILED;
		if (std::string(engine->GetTypeDeclaration(typeId)) != "MyType::Callback")
			TEST_FAILED;

		// Test WriteConfigToStream (try configurations with depency between types)
		bout.buffer = "";
		engine->RegisterObjectType("MyType2", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectType("MyType3", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterFuncdef("void MyType2::Callback(MyType3 @)");
		engine->RegisterFuncdef("void MyType3::F(MyType2::Callback @)");
		std::stringstream s;
		WriteConfigToStream(engine, s);

		asIScriptEngine *engine2 = asCreateScriptEngine();
		engine2->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ConfigEngineFromStream(engine2, s);
		if (r < 0)
			TEST_FAILED;
		asITypeInfo *type = engine->GetTypeInfoByDecl("MyType3::F");
		asIScriptFunction *f = type->GetFuncdefSignature();
		if (std::string(f->GetDeclaration()) != "void MyType3::F(MyType2::Callback@)")
			TEST_FAILED;
		if (type->GetParentType() == 0 || std::string(type->GetParentType()->GetName()) != "MyType3")
			TEST_FAILED;
		engine2->ShutDownAndRelease();
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test registering funcdef as child of template type: RegisterFuncdef("T Array<T>::Callback(T)")
		// The template instance must create new funcdefs
		bout.buffer = "";
		RegisterScriptArray(engine, false);
		r = engine->RegisterFuncdef("T &array<T>::MyCallback(const T&in)");
		if (r < 0)
			TEST_FAILED;
		mod->AddScriptSection("name",
			"int retval; \n"
			"int &func(const int &in a) { retval = a; return retval; } \n"
			"void main() \n"
			"{ \n"
			"  array<int>::MyCallback @cb = func; \n"
			"  int val = 34; \n"
			"  assert( 34 == cb(val) ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		r = ExecuteString(engine, "main();", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Test 'array<int>::MyCallback @cb;' should give appropriate error when MyCallback is not child of array type
		bout.buffer = "";
		r = ExecuteString(engine, "array<int>::MyCallback2 @cb;");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 13) : Error   : Identifier 'MyCallback2' is not a data type\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test child funcdef in template where funcdef takes template subtype by @. Should fail when trying to instance template with value type
		bout.buffer = "";
		r = engine->RegisterFuncdef("T @array<T>::MyCallback2(T@)");
		if (r < 0)
			TEST_FAILED;
		r = ExecuteString(engine, "array<float>::MyCallback2 @cb;");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 7) : Error   : Attempting to instantiate invalid template type 'array<float>'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test RegisterFuncdef("void array<@>::CB()"); should give appropriate parser error
		bout.buffer = "";
		r = engine->RegisterFuncdef("void array<@>::CB()");
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "System function (1, 12) : Error   : Expected data type\n"
						   "System function (1, 12) : Error   : Instead found '@'\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterFuncdef' with 'void array<@>::CB()' (Code: asINVALID_DECLARATION, -10)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test that funcdefs from other namespaces are not visible
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"namespace foo { funcdef void bar(); } \n"
			"void main() { \n"
			"  bar @b; \n"        // not visible
			"  foo::bar @c; \n"   // visible
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (2, 1) : Info    : Compiling void main()\n"
						   "test (3, 3) : Error   : Identifier 'bar' is not a data type in global namespace\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test lambdas
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		bout.buffer = "";

		// Success scenarios
		//---------------------
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("name",
			"funcdef void CB0(); \n"
			"funcdef void CB1(bool); \n"
			"funcdef void CB2(int, int); \n"
			"bool called = false; \n"
			"void func() { \n"
			"   CB0 @cb0 = function() {}; \n"          // The lambda takes on the signature of the funcdef it is assigned to
			"   CB1 @cb1 = function(a) {}; \n"
			"   CB2 @cb2 = function(a,b) {}; \n"
			"   CB0 @a0 = cast<CB0>(function(){}); \n" // or if a cast to a funcdef
			"   call(function(a) { called = a; }); \n" // or directly passed to a function expecting funcdef
			"   assert( called ); \n"
			"   call(function(a) { called = !a; }); \n"
			"   assert( !called ); \n"
			"} \n"
			"void call(CB1@a) { a(true); } \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Test calling lambda function
		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Saving and loading should also work
		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test using lambda functions in asIScriptModule::CompileFunction()
		asUINT funcCount = mod->GetFunctionCount();
		r = ExecuteString(engine, "called = false; \n call(function(a) { called = a; }); \n assert( called );\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( funcCount != mod->GetFunctionCount() )
			TEST_FAILED;

		// The keyword 'function' should not be reserved. Test "int c = function(a,b);". Should work normally
		mod->AddScriptSection("test",
			"int function(int,int) {return 0;}\n"
			"void func() { \n"
			"  int a = 0, b = 0; \n"
			"  int c = function(a, b); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Test using lambda functions in shared functions (the lambda's should be shared too). 
		// The shared lambda should be included in the module that compiles the shared function that declared the lambda
		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"funcdef int CB1(int); \n"
			"shared int sfunc(int a) { \n"
			"  CB1 @c = function(a) {return a*a;}; \n"
			"  return c(a); \n"
			"} \n");
		r = mod2->Build();
		if( r < 0 ) 
			TEST_FAILED;
		mod->AddScriptSection("test",
			"funcdef int CB1(int); \n"        // TODO: It shouldn't be necessary to declare this, but without it, it will not be saved with the bytecode, and thus the LoadByteCode will not be able to link the sfunc that uses it
			"shared int sfunc(int a) {} \n"); // no need to implement it again
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "assert( sfunc(4) == 16 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Saving and loading should also work
		CBytecodeStream stream2(__FILE__"1");
		r = mod->SaveByteCode(&stream2);
		if( r < 0 )
			TEST_FAILED;
		engine->DiscardModule("test2");
		engine->DiscardModule("test");

		mod = engine->GetModule("test3", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream2);
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "assert( sfunc(4) == 16 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Test lambda function in asIScriptModule::CompileGlobalVar()
		r = mod->CompileGlobalVar("glob", "CB1 @g = function(a) {return a;};", 0);
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "assert( g(4) == 4 );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// No messages should have been generated until now
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// The global variable is holding a reference to the anonymous function,
		// which is causing the module to believe it is still in use. So it is
		// necessary to explicitly discard the module before reusing it
		mod->Discard();
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		// Error scenarios
		//-------------------
		// Test assigning lambda to a funcdef that hasn't been declared
		bout.buffer = "";
		r = mod->CompileGlobalVar("glob", "NotDeclared @nd = function(a) {return a;};", 0);
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "glob (1, 1) : Error   : Identifier 'NotDeclared' is not a data type in global namespace\n"
						   "glob (1, 14) : Info    : Compiling int nd\n"
						   "glob (1, 28) : Error   : Can't implicitly convert from '$func@const' to 'int&'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test compiler error within lambda
		bout.buffer = "";
		mod->AddScriptSection("name", 
			"funcdef void CB0(); \n"
			"void func() { \n"
			"  CB0 @c = function() { error }; \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "name (3, 23) : Info    : Compiling void $void func()$0()\n"
						   "name (3, 31) : Error   : Expected ';'\n"
						   "name (3, 31) : Error   : Instead found '}'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Stand-alone lambda should generate appropriate error
		bout.buffer = "";
		r = ExecuteString(engine, "function(){};");
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "ExecuteString (1, 11) : Error   : Invalid expression: stand-alone anonymous function\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test error when lambda has wrong number of parameters for funcdef
		// Test error when attempting to call lambda through opCall post operator
		bout.buffer = "";
		mod->AddScriptSection("name", 
			"funcdef void CB1(int); \n"
			"void func() { \n"
			"  CB1 @c = function() {}; \n"    // too few arguments
			"  CB1 @d = function(a,b) {}; \n" // too many arguments
			"  function(){}(); \n"            // directly calling the lambda is not allowed
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		// TODO: The error messages should be more more explicit
		if( bout.buffer != "name (2, 1) : Info    : Compiling void func()\n"
						   "name (3, 23) : Error   : Can't implicitly convert from '$func@const' to 'CB1@&'.\n"
						   "name (4, 21) : Error   : Can't implicitly convert from '$func@const' to 'CB1@&'.\n"
						   "name (5, 15) : Error   : No matching signatures to '$func::opCall()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test multiple options for matching lambda
		// https://www.gamedev.net/forums/topic/672780-lambda-overload-problem/
		bout.buffer = "";
		mod->AddScriptSection("name",
			"funcdef void A(int); \n"
			"funcdef void B(float); \n"
			"void func(A@) {} \n"
			"void func(B@) {} \n"
			"void main() { \n"
			"  func(function(a){}); \n"          // compiler cannot decide
			"  func(cast<B>(function(a){})); \n" // this one is known
			"  func(function(float a){}); \n"    // this one is also known
			"} \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;
		if (bout.buffer != "name (5, 1) : Info    : Compiling void main()\n"
						   "name (6, 3) : Error   : Multiple matching signatures to 'func($func@const)'\n"
						   "name (6, 3) : Info    : void func(A@)\n"
						   "name (6, 3) : Info    : void func(B@)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test to make sure it is not possible to convert class method to primitive
	// http://www.gamedev.net/topic/669352-can-cast-object-method-to-uint/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("name",
			"class T { uint length() { return 42; } } \n"
			"void func() { \n"
			"   T t; \n"
			"   uint a = uint( t.length ); \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "name (2, 1) : Info    : Compiling void func()\n"
						   "name (4, 13) : Error   : Invalid operation on method\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test assert failure with taking address of method on temporary object
	// http://www.gamedev.net/topic/667853-assertion-failure-in-compiler-when-using-delegate/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("name", 
			"funcdef void Callback(); \n"
			"class Foo \n"
			"{ \n"
			"	void Bar() \n"
			"	{ \n"
			"	} \n"
			"} \n"
			"void test() \n"
			"{ \n"
			"	Callback@ cb = Callback( Foo().Bar ); \n"
			"} \n");
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

	// Test proper error when taking address of method by mistake
	// Reported by Polyak Istvan
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		const char *script =
			"class C1\n"
			"{\n"
			"    C1 ()\n"
			"    {\n"
			//"        write('C1\\n');\n"
			"    }\n"
			"    int m1 ()\n"
			"    {\n"
			//"        write('m1\\n');\n"
			"        return 2;\n"
			"	}\n"
			"}\n"
			"class C2\n"
			"{\n"
			"    C2 (int )\n"
			"    {\n"
			//"        write('C2 int\\n');\n"
			"    }\n"
			"    C2 (const C1 &in c1)\n"
			"    {\n"
			//"        write('C2 C1\\n');\n"
			"    } \n"
			"} \n"
			"void main () \n"
			"{ \n"
			"    C1 c1;            \n"  // C1
			"    C2 c2_4(c1.m1);   \n"  // problem: C2 C1 was called instead of error
			"} \n";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (20, 1) : Info    : Compiling void main()\n"
						   "test (23, 12) : Error   : No matching signatures to 'C2(C1::m1)'\n"
						   "test (23, 12) : Error   : Can't pass class method as arg directly. Use a delegate object instead\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test calling function pointer retrieved from property accessor
	// http://www.gamedev.net/topic/664944-stack-corruption-when-using-funcdef-and-class-property/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = 
			"funcdef void Callback( array<int>@ pArray );\n"
			"class Class\n"
			"{\n"
			"	private Callback@ m_pCallback;\n"
			"	Callback@ Callback\n"
			"	{\n"
			"		get const { return m_pCallback; }\n"
			"	}\n"
			"	Class( Callback@ pCallback )\n"
			"	{\n"
			"		@m_pCallback = @pCallback;\n"
			"	}\n"
			"}\n"
			"void CallbackFn( array<int>@ pArray )\n"
			"{\n"
			"	uint uiLength = pArray.length(); \n"//Crash occurs here
			"   g_length = uiLength; \n"
			"}\n"
			"uint g_length; \n"
			"void test()\n"
			"{\n"
			"	Class instance( @CallbackFn );\n"
			"	array<int> arr = {1,2,3};\n"
			"	g_length = 0; \n"
			"	instance.Callback( @arr );\n"
			"	assert( g_length == 3 ); \n"
			"	g_length = 0; \n"
			"	Callback@ pCallback = instance.Callback; \n"
			"	pCallback( @arr ); \n"
			"	assert( g_length == 3 ); \n"
			"}\n";

		mod = engine->GetModule("TEst", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "test()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test invalid use of function pointer
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func2() { \n"
			"  func; \n"
			"} \n"
			"void func() {} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void func2()\n"
						   "test (2, 3) : Error   : Invalid expression: ambiguous name\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test value comparison for function pointers
/*
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void CALLBACK(); \n"
			"void func1() {} \n"
			"void func2() {} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "CALLBACK@ c1 = func1, c2 = func1; \n"
								  "assert( c1 == c2 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// TODO: Test that two different function pointers give false
		// TODO: Test for delegate objects

		engine->Release();
	}
*/

	// Proper error message when trying to pass class method as function pointer directly
	// http://www.gamedev.net/topic/655390-is-there-a-bug-with-function-callbacks/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		
		bout.buffer = "";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void CALLBACK(int); \n"
			"class Test { \n"
			"  void Init() { \n"
			"    SetCallback(MyCallback); \n" // This should fail, since delegate is necessary
			"  } \n"
			"  void MyCallback(int) {} \n"
			"} \n"
			"void SetCallback(CALLBACK @) {} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (3, 3) : Info    : Compiling void Test::Init()\n"
						   "test (4, 5) : Error   : No matching signatures to 'SetCallback(Test::MyCallback)'\n"
						   "test (4, 5) : Error   : Can't pass class method as arg directly. Use a delegate object instead\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// opEquals with funcdef
	// http://www.gamedev.net/topic/647797-difference-between-xopequalsy-and-xy-with-funcdefs/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void CALLBACK(); \n"
			"class Test { \n"
			"  bool opEquals(CALLBACK @f) { \n"
			"    return f is func; \n"
			"  } \n"
			"  CALLBACK @func; \n"
			"} \n"
			"void func() {} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Test t; \n"
			                      "@t.func = func; \n"
								  "assert( t == func );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void CALLBACK(); \n"
			"class Test { \n"
			"  bool opEquals(CALLBACK @f) { \n"
			"    return f is func; \n"
			"  } \n"
			"  CALLBACK @func; \n"
			"} \n"
			"namespace ns { \n"
			"void func() {} \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Test t; \n"
								  "@t.func = ns::func; \n"
								  "assert( t == ns::func );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test funcdefs and namespaces
	// http://www.gamedev.net/topic/644586-application-function-returning-a-funcdef-handle-crashes-when-called-in-as/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"bool called = false; \n"
			"funcdef void simpleFuncDef(); \n"
			"namespace foo { \n"
			"  void simpleFunction() { called = true; } \n"
			"} \n"
			"void takeSimpleFuncDef(simpleFuncDef@ f) { f(); } \n"
			"void main() { \n"
			"  takeSimpleFuncDef(foo::simpleFunction); \n"
			"  assert( called ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		mod->AddScriptSection("test",
			"bool called = false; \n"
			"funcdef void simpleFuncDef();\n"
			"namespace foo {\n"
			"  void simpleFunction() { called = true; }\n"
			"}\n"
			"void main() {\n"
			"  simpleFuncDef@ bar = foo::simpleFunction;\n"
			"  bar(); \n"
			"  assert( called ); \n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		engine->Release();
	}

	// Test registering global property of funcdef type
	// http://www.gamedev.net/topic/644586-application-function-returning-a-funcdef-handle-crashes-when-called-in-as/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptFunction *f = 0;
		engine->RegisterFuncdef("void myfunc()");
		r = engine->RegisterGlobalProperty("myfunc @f", &f);
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() {} \n");
		mod->Build();

		r = ExecuteString(engine, "@f = func; \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( f == 0 )
			TEST_FAILED;
		if( strcmp(f->GetName(), "func") != 0 )
			TEST_FAILED;

		f->Release();
		f = 0;

		engine->Release();
	}

	// Test casting with funcdefs
	// http://www.gamedev.net/topic/644586-application-function-returning-a-funcdef-handle-crashes-when-called-in-as/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void myfunc1(); \n"
			"funcdef void myfunc2(); \n"
			"funcdef void myfunc3(); \n"
			"bool called = false; \n"
			"void func() { called = true; } \n"
			"void main() \n"
			"{ \n"
			"  myfunc1 @f1 = func; \n"
			"  myfunc2 @f2 = cast<myfunc2>(f1); \n" // explicit cast
			"  myfunc3 @f3 = f2; \n"                // implicit cast
			"  assert( f1 is f2 ); \n"
			"  assert( f2 is f3 ); \n"
			"  assert( f3 is func ); \n"
			"  f3(); \n"
			"  assert( called ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Don't allow application to register additional behaviours to funcdefs
	// http://www.gamedev.net/topic/644586-application-function-returning-a-funcdef-handle-crashes-when-called-in-as/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterObjectType("jjOBJ", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterFuncdef("void jjBEHAVIOR(jjOBJ@)");
		engine->RegisterFuncdef("void DifferentFunctionPointer()");
		r = engine->RegisterObjectMethod("jjBEHAVIOR", "DifferentFunctionPointer@ opImplCast()", asFUNCTION(0), asCALL_CDECL_OBJLAST);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectMethod' with 'jjBEHAVIOR' and 'DifferentFunctionPointer@ opImplCast()' (Code: asINVALID_ARG, -5)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test delegate function pointers for object methods
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Test { \n"
			"  void method() {} \n"
			"  int func(int) { return 0; } \n"
			"  void func() { called = true; } \n" // The compiler should pick the right overload
			"  bool called = false; \n"
			"} \n"
			"funcdef void CALLBACK(); \n"
			"void main() { \n"
			"  Test t; \n"
			"  CALLBACK @cb = CALLBACK(t.func); \n" // instanciate a delegate
			"  cb(); \n" // invoke the delegate
			"  assert( t.called ); \n"
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

		// Must be possible to save/load bytecode
		CBytecodeStream stream("test");
		mod->SaveByteCode(&stream);

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Must be possible to create delegate from within class method, i.e. implicit this.method
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef void CALL(); \n"
			"class Test { \n"
			"  bool called = false; \n"
			"  void callMe() { called = true; } \n"
			"  CALL @GetCallback() { return CALL(callMe); } \n"
			"} \n"
			"void main() { \n"
			"  Test t; \n"
			"  CALL @cb = t.GetCallback(); \n"
			"  cb(); \n"
			"  assert( t.called ); \n"
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

		// A delegate to own method held as member of class must be properly resolved by gc
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef void CALL(); \n"
			"class Test { \n"
			"  void call() {}; \n"
			"  CALL @c; \n"
			"} \n"
			"void main() { \n"
			"  Test t; \n"
			"  @t.c = CALL(t.call); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->GarbageCollect();

		asUINT currSize, totalDestr, totalDetect;
		engine->GetGCStatistics(&currSize, &totalDestr, &totalDetect);

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->GarbageCollect();

		asUINT currSize2, totalDestr2, totalDetect2;
		engine->GetGCStatistics(&currSize2, &totalDestr2, &totalDetect2);

		if( totalDetect2 == totalDetect )
			TEST_FAILED;

		// Must be possible to call delegate from application
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef void CALL(); \n"
			"class Test { \n"
			"  bool called = false; \n"
			"  void call() { called = true; } \n"
			"} \n"
			"Test t; \n"
			"CALL @callback = CALL(t.call); \n");
		r = mod->Build();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		int idx = mod->GetGlobalVarIndexByDecl("CALL @callback");
		if( idx < 0 )
			TEST_FAILED;

		asIScriptFunction *callback = *(asIScriptFunction**)mod->GetAddressOfGlobalVar(idx);
		if( callback == 0 )
			TEST_FAILED;
		if( callback->GetFuncType() != asFUNC_DELEGATE )
			TEST_FAILED;
		if( callback->GetDelegateObject() == 0 )
			TEST_FAILED;
		if( std::string(callback->GetDelegateFunction()->GetDeclaration()) != "void Test::call()" )
			TEST_FAILED;

		ctx = engine->CreateContext();
		ctx->Prepare(callback);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		r = ExecuteString(engine, "assert( t.called );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Must be possible to create the delegate from the application
		asIScriptObject *obj = (asIScriptObject*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByDecl("Test t"));
		asIScriptFunction *func = obj->GetObjectType()->GetMethodByName("call");
		asIScriptFunction *delegate = engine->CreateDelegate(func, obj);
		if( delegate == 0 )
			TEST_FAILED;
		delegate->Release();

		// Must be possible to create delegate for registered type too
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef bool EMPTY(); \n"
			"void main() { \n"
			"  array<int> a; \n"
			"  EMPTY @empty = EMPTY(a.isEmpty); \n"
			"  assert( empty() == true ); \n"
			"  a.insertLast(42); \n"
			"  assert( empty() == false ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Must not be possible to create delegate with const object and non-const method
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef void F(); \n"
			"class Test { \n"
			"  void f() {} \n"
			"} \n"
			"void main() { \n"
			" const Test @t; \n"
			" F @f = F(t.f); \n" // t is read-only, so this delegate must not be allowed
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		// TODO: Error message should be better, so it is understood that the error is because of const object
		if( bout.buffer != "test (5, 1) : Info    : Compiling void main()\n"
		                   "test (7, 9) : Error   : No matching signatures to 'void F()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Must not be possible to create delegates for non-reference types
		bout.buffer = "";
		mod->AddScriptSection("test",
			"funcdef bool CB(); \n"
			"string s; \n"
			"CB @cb = CB(s.isEmpty); \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (3, 5) : Info    : Compiling CB@ cb\n"
		                   "test (3, 10) : Error   : Can't create delegate for types that do not support handles\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test ordinary function pointers for global functions
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		// Test the declaration of new function signatures
		const char *script = "funcdef void functype();\n"
		// It must be possible to declare variables of the funcdef type
				 "functype @myFunc = null;\n"
		// It must be possible to initialize the function pointer directly
				 "functype @myFunc1 = @func;\n"
		 		 "void func() { called = true; }\n"
				 "bool called = false;\n"
		// It must be possible to compare the function pointer with another
				 "void main() { \n"
				 "  assert( myFunc1 !is null ); \n"
				 "  assert( myFunc1 is func ); \n"
		// It must be possible to call a function through the function pointer
	    		 "  myFunc1(); \n"
				 "  assert( called ); \n"
		// Local function pointer variables are also possible
				 "  functype @myFunc2 = @func;\n"
				 "} \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r != 0 )
			TEST_FAILED;
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// It must be possible to save the byte code with function handles
		CBytecodeStream bytecode(__FILE__"1");
		mod->SaveByteCode(&bytecode);
		{
			asIScriptModule *mod2 = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
			mod2->LoadByteCode(&bytecode);
			r = ExecuteString(engine, "main()", mod2);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}

		// Test function pointers as members of classes. It should be possible to call the function
		// from within a class method. It should also be possible to call it from outside through the . operator.
		script = "funcdef void FUNC();       \n"
				 "class CMyObj               \n"
				 "{                          \n"
				 "  CMyObj() { @f = @func; } \n"
				 "  FUNC@ f;                 \n"
				 "  void test()              \n"
				 "  {                        \n"
				 "    this.f();              \n"
				 "    f();                   \n"
				 "    CMyObj o;              \n"
				 "    o.f();                 \n"
				 "    main();                \n"
				 "    assert( called == 4 ); \n"
				 "  }                        \n"
				 "}                          \n"
				 "void main()                \n"
				 "{                          \n"
				 "  CMyObj o;                \n"
				 "  o.f();                   \n"
				 "}                          \n"
				 "int called = 0;            \n"
				 "void func() { called++; }  \n";
		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "CMyObj o; o.test();", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
		}
		ctx->Release();

		// funcdefs cannot be instantiated without being a delegate
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		script = "funcdef void functype();\n"
				 "functype myFunc;\n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (2, 1) : Error   : Data type can't be 'functype'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// It must not be possible to invoke the funcdef
		bout.buffer = "";
		script = "funcdef void functype();\n"
				 "void func() { functype(); } \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (2, 1) : Info    : Compiling void func()\n"
						   "script (2, 15) : Error   : No matching signatures to 'functype()'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Test that a funcdef can't have the same name as other global entities
		bout.buffer = "";
		script = "funcdef void test();  \n"
				 "int test; \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "script (2, 5) : Error   : Name conflict. 'test' is a funcdef.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// It is possible to take the address of class methods, but not to assign to funcdef variable
		bout.buffer = "";
		script = 
			"funcdef void F(); \n"
			"class t { \n"
			"  void func() { \n"
			"    @func; \n"
			"    F @f = @func; \n"
			"    } \n"
			"} \n";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		// TODO: The error message should be better
		if( bout.buffer != "script (3, 3) : Info    : Compiling void t::func()\n"
			               "script (4, 5) : Error   : Invalid expression: ambiguous name\n"
		                   "script (5, 12) : Error   : Can't implicitly convert from 't' to 'F@&'.\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// A more complex sample
		bout.buffer = "";
		script = 
			"funcdef bool CALLBACK(int, int); \n"
			"funcdef bool CALLBACK2(CALLBACK @); \n"
			"void main() \n"
			"{ \n"
			"	CALLBACK @func = @myCompare; \n"
			"	CALLBACK2 @func2 = @test; \n"
			"	func2(func); \n"
			"} \n"
			"bool test(CALLBACK @func) \n"
			"{ \n"
			"	return func(1, 2); \n"
			"} \n"
			"bool myCompare(int a, int b) \n"
			"{ \n"
			"	return a > b; \n"
			"} \n";
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

		// It must be possible to register the function signature from the application
		r = engine->RegisterFuncdef("void AppCallback()");
		if( r < 0 )
			TEST_FAILED;

		r = engine->RegisterGlobalFunction("void ReceiveFuncPtr(AppCallback @)", asFUNCTION(ReceiveFuncPtr), asCALL_CDECL); assert( r >= 0 );

		// It must be possible to use the registered funcdef
		// It must be possible to receive a function pointer for a registered func def
		bout.buffer = "";
		script = 
			"void main() \n"
			"{ \n"
			"	AppCallback @func = @test; \n"
			"   func(); \n"
			"   ReceiveFuncPtr(func); \n"
			"} \n"
			"void test() \n"
			"{ \n"
			"} \n";
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

		if( !receivedFuncPtrIsOK )
			TEST_FAILED;

		CBytecodeStream bytecode2(__FILE__"2");
		mod->SaveByteCode(&bytecode2);
		{
			receivedFuncPtrIsOK = false;
			asIScriptModule *mod2 = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
			mod2->LoadByteCode(&bytecode2);
			r = ExecuteString(engine, "main()", mod2);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;

			if( !receivedFuncPtrIsOK )
				TEST_FAILED;
		}

		// The compiler should be able to determine the right function overload
		// by the destination of the function pointer
		bout.buffer = "";
		mod->AddScriptSection("test",
		         "funcdef void f(); \n"
				 "f @fp = @func;  \n"
				 "bool called = false; \n"
				 "void func() { called = true; }    \n"
				 "void func(int) {} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		r = ExecuteString(engine, "fp(); assert( called );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		//----------------------------------------------------------
		// TODO: Future improvements below

		// If the function referred to when taking a function pointer is removed from the module,
		// the code must not be invalidated. After removing func() from the module, it must still 
		// be possible to execute func2()
		script = "funcdef void FUNC(); \n"
				 "void func() {} \n";
				 "void func2() { FUNC@ f = @func; f(); } \n";

		// Test that the function in a function pointer isn't released while the function 
		// is being executed, even though the function pointer itself is cleared
		script = "DYNFUNC@ funcPtr;        \n"
				 "funcdef void DYNFUNC(); \n"
				 "@funcPtr = @CompileDynFunc('void func() { @funcPtr = null; }'); \n";

		// Test that it is possible to declare the function signatures out of order
		// This also tests the circular reference between the function signatures
		script = "funcdef void f1(f2@) \n"
				 "funcdef void f2(f1@) \n";

		// It must be possible to identify a function handle type from the type id

		// It must be possible enumerate the function definitions in the module, 
		// and to enumerate the parameters the function accepts

		// A funcdef defined in multiple modules must share the id and signature so that a function implemented 
		// in one module can be called from another module by storing the handle in the funcdef variable

		// An interface that takes a funcdef as parameter must still have its typeid shared if the funcdef can also be shared
		// If the funcdef takes an interface as parameter, it must still be shared

		// Must have a generic function pointer that can store any signature. The function pointer
		// can then be dynamically cast to the correct function signature so that the function it points
		// to can be invoked.

		engine->Release();
	}

	// Test function pointers with virtual property accessors
	// http://www.gamedev.net/topic/639243-funcdef-inside-shared-interface-interface-already-implement-warning/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void funcdef1( ifuncdef1_1& i ); \n"
			"shared interface ifuncdef1_1 \n"
			"{ \n"
			"    ifuncdef1_2@ events { get; set; } \n"
			"    void crashme(); \n"
			"} \n"
			"shared interface ifuncdef1_2 \n"
			"{ \n"
			"    funcdef1@ f { get; set; } \n"
			"} \n"
			"class cfuncdef1_1 : ifuncdef1_1 \n"
			"{ \n"
			"    ifuncdef1_2@ _events_; \n"
			"    cfuncdef1_1() { @this._events_ = cfuncdef1_2(); } \n"
			"    ifuncdef1_2@ get_events() property { return( this._events_ ); } \n"
			"    void set_events( ifuncdef1_2@ events ) property { @this._events_ = events; } \n"
			"    void crashme() \n"
			"    { \n"
			"         if( this._events_ !is null && this._events_.f !is null ) \n"
			"         { \n"
			"            this.events.f( this ); \n"
//			"            this.get_events().get_f()( this ); \n" // This should produce the same bytecode as the above
			"         } \n"
			"    } \n"
			"} \n"
			"class cfuncdef1_2 : ifuncdef1_2 \n"
			"{ \n"
			"    funcdef1@ ff; \n"
			"    cfuncdef1_2() { @ff = null; } \n"
			"    funcdef1@ get_f() property { return( @this.ff ); } \n"
			"    void set_f( funcdef1@ _f ) property { @this.ff = _f; } \n"
			"} \n"
			"void start() \n"
			"{ \n"
			"    ifuncdef1_1@ i = cfuncdef1_1(); \n"
			"    @i.events.f = end; \n" // TODO: Shouldn't this give an error? It's attempting to do an value assignment to a function pointer
			"    i.crashme(); \n"
			"} \n"
			"bool called = false; \n"
			"void end( ifuncdef1_1& i  ) \n"
			"{ \n"
			"    called = true; \n"
			"} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start(); assert( called );", mod, ctx);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		ctx->Release();

		CBytecodeStream stream(__FILE__"1");
		
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		
		engine->Release();

		// Load the bytecode 
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		stream.Restart();
		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start(); assert( called );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		stream.Restart();
		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "start(); assert( called );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// Test clean up with registered function definitions
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptString(engine);
		r = engine->RegisterFuncdef("void MSG_NOTIFY_CB(const string& strCommand, const string& strTarget)"); assert(r>=0);

		engine->Release();
	}

	// Test registering function pointer as property
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		r = engine->RegisterFuncdef("void fptr()");
		r = engine->RegisterGlobalProperty("fptr f", 0);
		if( r >= 0 ) TEST_FAILED;
		engine->RegisterObjectType("obj", 0, asOBJ_REF);
		r = engine->RegisterObjectProperty("obj", "fptr f", 0);
		if( r >= 0 ) TEST_FAILED;

		engine->Release();
	}

	// Test passing handle to function pointer
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		mod->AddScriptSection("script",
			"class CTempObj             \n"
			"{                          \n"
			"  int Temp;                \n"
			"}                          \n"
			"funcdef void FUNC2(CTempObj@);\n"
			"class CMyObj               \n"
			"{                          \n"
			"  CMyObj() { @f2= @func2; }\n"
			"  FUNC2@ f2;               \n"
			"}                          \n"
			"void main()                \n"
			"{                          \n"
			"  CMyObj o;                \n"
			"  CTempObj t;              \n"
			"  o.f2(t);                 \n"
			"  assert( called == 1 );   \n"
			"}                          \n"
			"int called = 0;            \n"
			"void func2(CTempObj@ Obj)  \n"
			"{ called++; }              \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test out of order declaration with function pointers
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		mod->AddScriptSection("script",
			"funcdef void FUNC2(CTempObj@);\n"
			"class CTempObj {}             \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// It must be possible calling system functions through pointers too
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterFuncdef("bool fun(bool)");

		// The returned value is the type id for the funcdef
		std::string str = engine->GetTypeDeclaration(r);
		if (str != "fun")
			TEST_FAILED;

		engine->RegisterGlobalFunction("bool assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, "fun @f = assert; f(true);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// It should be possible to call functions through function pointers returned by an expression
	// http://www.gamedev.net/topic/627386-bug-with-parsing-of-callable-expressions/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		mod->AddScriptSection("Test",
			"funcdef void F(int); \n"
			"array<F@> arr(1); \n"
			"F@ g()            \n"
			"{                 \n"
			"  return test;    \n"
			"}                 \n"
			"void test(int a)  \n"
			"{                 \n"
			"  assert(a == 42); \n"
			"  called++;       \n"
			"}                 \n"
			"int called = 0;   \n"
			"void f()          \n"
			"{                 \n"
			"  @arr[0] = test; \n"
			"  arr[0](42);     \n"
			"  g()(42);        \n"
			"  F@ p; \n"
			"  (@p = arr[0])(42);     \n"
			"  (@p = g())(42);        \n"
			"}                        \n");

	//	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "f()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		int idx = mod->GetGlobalVarIndexByName("called");
		int *called = (int*)mod->GetAddressOfGlobalVar(idx);
		if( *called != 4 )
			TEST_FAILED;

		engine->Release();
	}

	// Global function pointers must not overload local class methods
	// Local variables take precedence over class methods
	// http://www.gamedev.net/topic/626746-function-call-operators-in-the-future/
	{
		const char *script = 
			"funcdef void FUNC(); \n"
			"FUNC @func; \n"
			"class Class \n"
			"{ \n"
			"  void func() {} \n"
			"  void method() \n"
			"  { \n"
			"    func(); \n"       // Should call Class::func()
			"  } \n"
			"  void func2() {} \n"
			"  void method2() \n"
			"  { \n"
			"    FUNC @func2; \n"
			"    func2(); \n"      // Should call variable func2
			"  } \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Class c; c.method();", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		r = ExecuteString(engine, "Class c; c.method2();", mod);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		engine->Release();
	}

	// Success
 	return fail;
}

} // namespace

