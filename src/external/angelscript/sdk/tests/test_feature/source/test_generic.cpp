
#include <stdarg.h>
#include "utils.h"
#include <sstream>
#include <../../add_on/autowrapper/aswrappedcall.h>

// From the scriptstdstring add-on
BEGIN_AS_NAMESPACE
extern void RegisterStdString_Generic(asIScriptEngine *engine);
END_AS_NAMESPACE

using std::string;

namespace TestGeneric
{


int obj;

void GenFunc1(asIScriptGeneric *gen)
{
	assert(gen->GetObject() == 0);

//	PRINTF("GenFunc1\n");

	int arg1 = (int)gen->GetArgDWord(0);
	double arg2 = gen->GetArgDouble(1);
	string arg3 = *(string*)gen->GetArgObject(2);

	assert(arg1 == 23);
	assert(arg2 == 23);
	assert(arg3 == "test");

	gen->SetReturnDouble(23);
}

void GenMethod1(asIScriptGeneric *gen)
{
	assert(gen->GetObject() == &obj);

//	PRINTF("GenMethod1\n");

	int arg1 = (int)gen->GetArgDWord(0);
	double arg2 = gen->GetArgDouble(1);

	assert(arg1 == 23);
	assert(arg2 == 23);

	string s("Hello");
	gen->SetReturnObject(&s);
}

void GenAssign(asIScriptGeneric *gen)
{
//	assert(gen->GetObject() == &obj);

	int *obj2 = (int*)gen->GetArgObject(0);
	UNUSED_VAR(obj2);

//	assert(obj2 == &obj);

	gen->SetReturnObject(&obj);
}

void TestDouble(asIScriptGeneric *gen)
{
	double d = gen->GetArgDouble(0);

	assert(d == 23);
}

void TestString(asIScriptGeneric *gen)
{
	string s = *(string*)gen->GetArgObject(0);

	assert(s == "Hello");
}

void GenericString_Construct(asIScriptGeneric *gen)
{
	string *s = (string*)gen->GetObject();

	new(s) string;
}

void GenericString_Destruct(asIScriptGeneric *gen)
{
	string *s = (string*)gen->GetObject();

	s->~string();
}

void GenericString_Assignment(asIScriptGeneric *gen)
{
	string *other = (string*)gen->GetArgObject(0);
	string *self = (string*)gen->GetObject();

	*self = *other;

	gen->SetReturnObject(self);
}

void GenericString_Factory(asIScriptGeneric *gen)
{
	asUINT length = gen->GetArgDWord(0);
	UNUSED_VAR(length);
	const char *s = (const char *)gen->GetArgAddress(1);

	string str(s);

	gen->SetReturnObject(&str);
}

void nullPtr(asIScriptGeneric *gen)
{
	asIScriptObject **intf = (asIScriptObject**)gen->GetAddressOfArg(0);
	assert( *intf == 0 );

	assert(gen->GetArgCount() == 1);

	*(asIScriptObject **)gen->GetAddressOfReturnLocation() = *intf;

	assert(gen->GetReturnTypeId() == gen->GetEngine()->GetTypeIdByDecl("intf@"));
}

int extraValue = 42;
void Generic_Functor(asIScriptGeneric *gen)
{
	// The auxiliary was informed by application when registering the function
	int *value = reinterpret_cast<int*>(gen->GetAuxiliary());
	assert(*value == 42);
	*value = 24;
}

bool Test2();

int counter = 0;
void DoNothingTest(asIScriptGeneric *gen)
{
	if (size_t(gen->GetAuxiliary()) == 123)
		counter++;
}

void ConstructorTest(asIScriptGeneric *gen)
{
	if (size_t(gen->GetAuxiliary()) == 123)
		counter++;
}

void DestructorTest(asIScriptGeneric *gen)
{
	if (size_t(gen->GetAuxiliary()) == 123)
		counter++;
}

asIScriptFunction *callback = NULL;
void SetCallback(asIScriptGeneric *gen)
{
	callback = (asIScriptFunction *)gen->GetArgAddress(0);
	callback->AddRef();

	asIScriptFunction *f1 = (asIScriptFunction*)gen->GetArgObject(0);
	assert(f1 == callback);

	asIScriptFunction *f2 = *((asIScriptFunction**)gen->GetAddressOfArg(0));
	assert(f2 == callback);
}

void GetCallback(asIScriptGeneric *gen)
{
	gen->SetReturnAddress(callback);                           // <== If using this line it will work as expected
	assert(*(void **)gen->GetAddressOfReturnLocation() == callback);
	*(void **)gen->GetAddressOfReturnLocation() = 0;

	gen->SetReturnObject(callback);
	assert(*(void **)gen->GetAddressOfReturnLocation() == callback);
	*(void **)gen->GetAddressOfReturnLocation() = 0;
	callback->Release(); // SetReturnObject will increase the reference counter

	*(void **)gen->GetAddressOfReturnLocation() = callback;      // <== I expect this line would work same as the above line, but it's not.
	callback->AddRef();
}

int idCount = 0;
std::stringstream log;
class RefCounted
{
public:
	RefCounted() { id = idCount++; refCount = 1; log << "created " << id << std::endl; }
	void AddRef() { refCount++; log << "increased ref " << id << ", refCount = " << refCount << std::endl; }
	void Release() 
	{ 
		refCount--; log << "decreased ref " << id << ", refCount = " << refCount << std::endl; 
		if (refCount == 0)
		{
			log << "deleted " << id << std::endl;
			delete this;
		}
	}
	static RefCounted *Factory() { return new RefCounted(); }
	int refCount;
	int id;
};

void ReceiveHandle(RefCounted *t)
{
	// Release the handle that was received
	if( t ) t->Release();
}

void ReceiveAutoHandle(RefCounted * /*t*/)
{
	// Do not release the handle, as AngelScript will do that afterwards
}

RefCounted *g_t = 0;
RefCounted *ReturnHandle()
{
	// increase the refcount for the returned handle
	g_t->AddRef();
	return g_t;
}

RefCounted *ReturnAutoHandle()
{
	// Do not increase ref count since angelscript will do that afterwards
	return g_t;
}





bool Test()
{
	bool fail = Test2();

	int r;
	asIScriptEngine *engine;
	CBufferedOutStream bout;

	// Comparing generic to native functions
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->SetEngineProperty(asEP_GENERIC_CALL_MODE, 1);

		engine->RegisterObjectType("type", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("type", asBEHAVE_FACTORY, "type @f()", asFUNCTION(RefCounted::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("type", asBEHAVE_ADDREF, "void f()", asMETHOD(RefCounted, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("type", asBEHAVE_RELEASE, "void f()", asMETHOD(RefCounted, Release), asCALL_THISCALL);

		engine->RegisterGlobalFunction("void ReceiveHandle(type @)", asFUNCTION(ReceiveHandle), asCALL_CDECL);
		engine->RegisterGlobalFunction("void ReceiveAutoHandle(type @+)", asFUNCTION(ReceiveAutoHandle), asCALL_CDECL);
		engine->RegisterGlobalFunction("type @ ReturnHandle()", asFUNCTION(ReturnHandle), asCALL_CDECL);
		engine->RegisterGlobalFunction("type @+ ReturnAutoHandle()", asFUNCTION(ReturnAutoHandle), asCALL_CDECL);

		engine->RegisterGlobalFunction("void ReceiveHandle_gen(type @)", WRAP_FN(ReceiveHandle), asCALL_GENERIC);
		engine->RegisterGlobalFunction("void ReceiveAutoHandle_gen(type @+)", WRAP_FN(ReceiveAutoHandle), asCALL_GENERIC);
		engine->RegisterGlobalFunction("type @ ReturnHandle_gen()", WRAP_FN(ReturnHandle), asCALL_GENERIC);
		engine->RegisterGlobalFunction("type @+ ReturnAutoHandle_gen()", WRAP_FN(ReturnAutoHandle), asCALL_GENERIC);

		g_t = new RefCounted();

		log << "ReceiveHandle" << std::endl;
		r = ExecuteString(engine, "type t; ReceiveHandle(t);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReceiveAutoHandle" << std::endl;
		r = ExecuteString(engine, "type t; ReceiveAutoHandle(t);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReturnHandle" << std::endl;
		r = ExecuteString(engine, "type @t = ReturnHandle();");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReturnAutoHandle" << std::endl;
		r = ExecuteString(engine, "type @t = ReturnAutoHandle();");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReceiveHandle_gen" << std::endl;
		r = ExecuteString(engine, "type t; ReceiveHandle_gen(t);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReceiveAutoHandle_gen" << std::endl;
		r = ExecuteString(engine, "type t; ReceiveAutoHandle_gen(t);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReturnHandle_gen" << std::endl;
		r = ExecuteString(engine, "type @t = ReturnHandle_gen();");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		log << "ReturnAutoHandle_gen" << std::endl;
		r = ExecuteString(engine, "type @t = ReturnAutoHandle_gen();");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		// Release the global instance
		g_t->Release();
		g_t = 0;

		if (log.str() != "created 0\n"
			"ReceiveHandle\n"
			"created 1\n"
			"increased ref 1, refCount = 2\n"
			"decreased ref 1, refCount = 1\n"
			"decreased ref 1, refCount = 0\n"
			"deleted 1\n"
			"ReceiveAutoHandle\n"
			"created 2\n"
			"increased ref 2, refCount = 2\n"
			"decreased ref 2, refCount = 1\n"
			"decreased ref 2, refCount = 0\n"
			"deleted 2\n"
			"ReturnHandle\n"
			"increased ref 0, refCount = 2\n"
			"decreased ref 0, refCount = 1\n"
			"ReturnAutoHandle\n"
			"increased ref 0, refCount = 2\n"
			"decreased ref 0, refCount = 1\n"
			"ReceiveHandle_gen\n"
			"created 3\n"
			"increased ref 3, refCount = 2\n"
			"decreased ref 3, refCount = 1\n"
			"decreased ref 3, refCount = 0\n"
			"deleted 3\n"
			"ReceiveAutoHandle_gen\n"
			"created 4\n"
			"increased ref 4, refCount = 2\n"
			"decreased ref 4, refCount = 1\n"
			"decreased ref 4, refCount = 0\n"
			"deleted 4\n"
			"ReturnHandle_gen\n"
			"increased ref 0, refCount = 2\n"
			"decreased ref 0, refCount = 1\n"
			"ReturnAutoHandle_gen\n"
			"increased ref 0, refCount = 2\n"
			"decreased ref 0, refCount = 1\n"
			"decreased ref 0, refCount = 0\n"
			"deleted 0\n")
		{
			PRINTF("%s", log.str().c_str());
			TEST_FAILED;
		}

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test passing and returning funcdefs in generic functions
	{
		engine = asCreateScriptEngine();

		asIScriptModule *module = engine->GetModule("testCallback", asGM_ALWAYS_CREATE);
		asIScriptContext *context = engine->CreateContext();

		r = engine->RegisterFuncdef("int Callback()");assert(r >= 0);
		r = engine->RegisterGlobalFunction("void SetCallback(Callback@+ cb)", asFUNCTION(SetCallback), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("Callback@ GetCallback()", asFUNCTION(GetCallback), asCALL_GENERIC); assert(r >= 0);

		r = module->AddScriptSection("test", "int main(){ \n"
											 "  SetCallback(@testCB); \n"
											 "  Callback@ cb = GetCallback(); \n"
											 "  return cb(); \n"
											 "} \n"
											 "int testCB(){ \n"
											 "  return 123; \n"
											 "}");assert(r >= 0);
		r = module->Build();assert(r >= 0);

		r = context->Prepare(module->GetFunctionByName("main"));assert(r >= 0);

		if (context->Execute() != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (context->GetReturnDWord() != 123)
			TEST_FAILED;

		context->Release();
		module->Discard();

		if (callback)
		{
			callback->Release();
			callback = 0;
		}
		else
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test auxiliary pointer with generic functions
	{
		engine = asCreateScriptEngine();

		asIScriptModule *module = engine->GetModule("testAuxiliary", asGM_ALWAYS_CREATE);
		asIScriptContext *context = engine->CreateContext();

		r = engine->RegisterGlobalFunction("void DoNothing()", asFUNCTION(DoNothingTest), asCALL_GENERIC, (void *)123); assert(r >= 0);
		r = engine->RegisterObjectType("dummy", 4, asOBJ_VALUE | asOBJ_APP_CLASS_CD); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("dummy", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructorTest), asCALL_GENERIC, (void *)123); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("dummy", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructorTest), asCALL_GENERIC, (void *)123); assert(r >= 0);

		counter = 0;
		r = module->AddScriptSection("test", "void main(){   DoNothing();  dummy d;    }");
		r = module->Build();

		r = context->Prepare(module->GetFunctionByName("main"));
		r = context->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (counter != 3)
			TEST_FAILED;

		context->Release();
		module->Discard();

		engine->ShutDownAndRelease();
	}
	
	// Standard test
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterStdString_Generic(engine);

		r = engine->RegisterGlobalFunction("void test(double)", asFUNCTION(TestDouble), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("void test(string)", asFUNCTION(TestString), asCALL_GENERIC); assert(r >= 0);

		r = engine->RegisterGlobalFunction("double func1(int, double, string)", asFUNCTION(GenFunc1), asCALL_GENERIC); assert(r >= 0);

		r = engine->RegisterObjectType("obj", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "string mthd1(int, double)", asFUNCTION(GenMethod1), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "obj &opAssign(obj &in)", asFUNCTION(GenAssign), asCALL_GENERIC); assert(r >= 0);

		r = engine->RegisterGlobalProperty("obj o", &obj);

		r = engine->RegisterInterface("intf");
		r = engine->RegisterGlobalFunction("intf @nullPtr(intf @+)", asFUNCTION(nullPtr), asCALL_GENERIC); assert(r >= 0);

		r = engine->RegisterGlobalFunction("void functor()", asFUNCTION(Generic_Functor), asCALL_GENERIC, &extraValue); assert(r >= 0);

		COutStream out;
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "test(func1(23, 23, \"test\"))");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "test(o.mthd1(23, 23))");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "o = o");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "nullPtr(null)");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		r = ExecuteString(engine, "functor()");
		if (r != asEXECUTION_FINISHED || extraValue != 24)
			TEST_FAILED;

		engine->Release();
	}

	// Success
	return fail;
}

//--------------------------------------------------------
// This part is going to test the auto-generated wrappers
//--------------------------------------------------------

// This doesn't work on MSVC6. The template implementation isn't good enough.
// It also doesn't work on MSVC2005, it gets confused on const methods that return void. Reported by Jeff Slutter.
// TODO: Need to fix implementation for MSVC2005.
#if !defined(_MSC_VER) || (_MSC_VER > 1200 && _MSC_VER != 1400) 

}
namespace TestGeneric
{

void TestNoArg() {}

void TestStringByVal(std::string val) {
	assert(val == "test");
}

void TestStringByRef(std::string &ref) {
	assert(ref == "test");
}

void TestIntByVal(int val) {
	assert(val == 42);
}

void TestIntByRef(int &ref) {
	assert(ref == 42);
}

int TestRetIntByVal() {
	return 42;
}

int &TestRetIntByRef() {
	static int val = 42;
	return val;
}

std::string TestRetStringByVal() {
	return "test";
}

std::string &TestRetStringByRef() {
	static std::string val = "test";
	return val;
}

void TestOverload(int) {}

void TestOverload(float) {}

class A
{
public:
	A() {id = 0;}
	virtual void a() const {assert(id == 2);}
	int id;
};

class B
{
public:
	B() {}
	virtual void b() {}
};

class C : public A, B
{
public:
	C() {id = 2;}
	~C() {}
	virtual void c(int) {assert(id == 2);}
	virtual void c(float) const {assert(id == 2);}
};

// http://www.gamedev.net/topic/639902-premature-destruction-of-object-in-android/
std::stringstream buf;

class RefCountable
{
public:
	RefCountable() { refCount = 1; buf << "init\n"; }
	virtual ~RefCountable() { buf << "destroy\n"; }
	void addReference() { refCount++; buf << "add (" << refCount << ")\n"; }
	void removeReference() { buf << "rem (" << refCount-1 << ")\n"; if( --refCount == 0 ) delete this; }
	int refCount;
};

class Animable
{
public:
	Animable() {}
};

class Trackable
{
public:
	Trackable() {}
};

class UIControl : public Animable, public Trackable, public RefCountable
{
public:
	UIControl() : Animable(), Trackable(), RefCountable() { test = "hello"; }

	string test;
};

class UIButton : public UIControl
{
public:
	UIButton() : UIControl() {};
};

template<typename T>
T* genericFactory()
{
	return new T();
}

bool Test2()
{
	bool fail = false;
	COutStream out;

	int r;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	RegisterStdString(engine);

	r = engine->RegisterGlobalFunction("void TestNoArg()", WRAP_FN(TestNoArg), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "TestNoArg()");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("void TestStringByVal(string val)", WRAP_FN(TestStringByVal), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "TestStringByVal('test')");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("void TestStringByRef(const string &in ref)", WRAP_FN(TestStringByRef), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "TestStringByRef('test')");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("void TestIntByVal(int val)", WRAP_FN(TestIntByVal), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "TestIntByVal(42)");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("void TestIntByRef(int &in ref)", WRAP_FN(TestIntByRef), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "TestIntByRef(42)");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("int TestRetIntByVal()", WRAP_FN(TestRetIntByVal), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "assert(TestRetIntByVal() == 42)");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("int &TestRetIntByRef()", WRAP_FN(TestRetIntByRef), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "assert(TestRetIntByRef() == 42)");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("string TestRetStringByVal()", WRAP_FN(TestRetStringByVal), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "assert(TestRetStringByVal() == 'test')");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterGlobalFunction("string &TestRetStringByRef()", WRAP_FN(TestRetStringByRef), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "assert(TestRetStringByRef() == 'test')");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	r = engine->RegisterObjectType("C", sizeof(C), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("C", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON(C, ()), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("C", asBEHAVE_DESTRUCT, "void f()", WRAP_DES(C), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("C", "void a() const", WRAP_MFN(A, a), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("C", "void b()", WRAP_MFN(B, b), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("C", "void c(int)", WRAP_MFN_PR(C, c, (int), void), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("C", "void c(float) const", WRAP_MFN_PR(C, c, (float) const, void), asCALL_GENERIC); assert( r >= 0 );

	r = ExecuteString(engine, "C c; c.a(); c.b(); c.c(1); c.c(1.1f);");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();

	// http://www.gamedev.net/topic/639902-premature-destruction-of-object-in-android/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("UIButton", 0, asOBJ_REF);
		r = engine->RegisterObjectBehaviour("UIButton", asBEHAVE_FACTORY, "UIButton @f()", WRAP_FN(genericFactory<UIButton>), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("UIButton", asBEHAVE_ADDREF, "void f()", WRAP_MFN(UIButton, addReference), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("UIButton", asBEHAVE_RELEASE, "void f()", WRAP_MFN(UIButton, removeReference), asCALL_GENERIC); assert( r >= 0 );

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "UIButton button;");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		UIButton *but = reinterpret_cast<UIButton*>(mod->GetAddressOfGlobalVar(0));
		if( but == 0 )
			TEST_FAILED;

		if( but->test != "hello" )
			TEST_FAILED;

		buf << "reset\n";
		mod->ResetGlobalVars();

		but = reinterpret_cast<UIButton*>(mod->GetAddressOfGlobalVar(0));
		if( but == 0 )
			TEST_FAILED;

		if( but->test != "hello" )
			TEST_FAILED;

		if( buf.str() != "init\n"
						 "add (2)\n"
						 "rem (1)\n"
						 "reset\n"
						 "rem (0)\n"
						 "destroy\n"
						 "init\n"
						 "add (2)\n"
						 "rem (1)\n" )
		{
			PRINTF("%s", buf.str().c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}
#else
bool Test2()
{
	PRINTF("The test of the autowrapper was skipped due to lack of proper template support\n");
	return false;
}
#endif

} // namespace

