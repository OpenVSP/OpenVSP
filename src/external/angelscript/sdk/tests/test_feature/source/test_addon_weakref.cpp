#include "utils.h"
#include "../../../add_on/weakref/weakref.h"

namespace Test_Addon_WeakRef
{

static const char *TESTNAME = "Test_Addon_WeakRef";

class MyClass
{
public:
	MyClass() { refCount = 1; weakRefFlag = 0; }
	~MyClass()
	{
		if( weakRefFlag )
			weakRefFlag->Release();
	}
	void AddRef() { refCount++; }
	void Release() 
	{
		// If the weak ref flag exists it is because someone held a weak ref
		// and that someone may add a reference to the object at any time. It
		// is ok to check the existance of the weakRefFlag without locking here
		// because if the refCount is 1 then no other thread is currently 
		// creating the weakRefFlag.
		if( refCount == 1 && weakRefFlag )
		{
			// Set the flag to tell others that the object is no longer alive
			// We must do this before decreasing the refCount to 0 so we don't
			// end up with a race condition between this thread attempting to 
			// destroy the object and the other that temporary added a strong
			// ref from the weak ref.
			weakRefFlag->Set(true);
		}

		if( --refCount == 0 ) 
			delete this; 
	}
	asILockableSharedBool *GetWeakRefFlag()
	{
		if( !weakRefFlag )
		{
			// Lock globally so no other thread can attempt
			// to create a shared bool at the same time
			asAcquireExclusiveLock();

			// Make sure another thread didn't create the 
			// flag while we waited for the lock
			if( !weakRefFlag )
				weakRefFlag = asCreateLockableSharedBool();

			asReleaseExclusiveLock();
		}

		return weakRefFlag;
	}

	static MyClass *Factory() { return new MyClass(); }

protected:
	int refCount;
	asILockableSharedBool *weakRefFlag;
};

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine *engine;

	// Ordinary tests
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptWeakRef(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"class Test {} \n"
			"void main() { \n"
			"  Test @t = Test(); \n"
			"  weakref<Test> r(t); \n"
			"  assert( r.get() !is null ); \n"
			"  const_weakref<Test> c; \n"
			"  @c = r; \n"
			"  assert( c.get() !is null ); \n"
			"  @t = null; \n"
			"  assert( r.get() is null ); \n"
			"  assert( c.get() is null ); \n"
			"  @t = Test(); \n"
			"  @c = t; \n"
			"  assert( c.get() !is null ); \n"
			"  const Test @ct = c; \n"
			"  assert( ct !is null ); \n"
			"  assert( c !is null ); \n"
			"  assert( c is ct ); \n"
			"  @c = null; \n"
			"  assert( c is null ); \n"
			"} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if (r < 0)
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->Release();
	}

	// value assignment for weakref
	// http://www.gamedev.net/topic/680611-passing-this-as-argument-in/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptWeakRef(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Foo {} \n"
			"void main() \n"
			"{ \n"
			"  weakref<Foo> a = Foo(); \n"
			"  weakref<Foo> b; \n"
			"  b = a; \n"
			"  assert( a.get() is b.get() ); \n"
			"  weakref<Foo> c(b); \n"
			"  assert( c.get() is b.get() ); \n"
			"  func(a); \n"
			"} \n"
			"void func(weakref<Foo> f) {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// CreateScriptObject with Weakref
	// http://www.gamedev.net/topic/680788-crash-when-trying-to-store-weakref-in-an-array/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptWeakRef(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "class Foo {}");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asITypeInfo *type = mod->GetTypeInfoByDecl("weakref<Foo>");
		CScriptWeakRef *weakRef = (CScriptWeakRef*)engine->CreateScriptObject(type);
		if (weakRef == 0 || weakRef->GetRefType() == 0 || std::string(weakRef->GetRefType()->GetName()) != "Foo")
			TEST_FAILED;

		CScriptWeakRef *weakRef2 = (CScriptWeakRef*)engine->CreateScriptObjectCopy(weakRef, type);
		if (weakRef2 == 0 || weakRef2->GetRefType() == 0 || std::string(weakRef2->GetRefType()->GetName()) != "Foo")
			TEST_FAILED;

		engine->ReleaseScriptObject(weakRef, type);
		engine->ReleaseScriptObject(weakRef2, type);

		engine->ShutDownAndRelease();
	}

	// Weakref as member of script class
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptWeakRef(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"class Test { weakref<Test> friend(null); } \n"
			"void main() { \n"
			"  Test t; \n"
			"  assert( t.friend is null ); \n"
			"  @t.friend = t; \n"
			"  assert( t.friend is t ); \n"
			"  weakref<Test> f; \n"  // http://www.gamedev.net/topic/671908-bug-in-ascscriptenginegeneratetemplatefactorystub/
			"} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
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

		CBytecodeStream stream("test");
		r = mod->SaveByteCode(&stream);
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&stream);
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod2);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// It shouldn't be possible to instantiate the weakref for types that do not support it
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptWeakRef(engine);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"class Test {} \n"
			"void main() { \n"
			"  weakref<int> a; \n"         // fail
			"  weakref<string> b; \n"      // fail
			"  weakref<Test@> c; \n"       // fail
			"  weakref<array<Test>> d; \n" // fail
			"} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "Test_Addon_WeakRef (2, 1) : Info    : Compiling void main()\n"
			"Test_Addon_WeakRef (3, 11) : Error   : Attempting to instantiate invalid template type 'weakref<int>'\n"
			"Test_Addon_WeakRef (4, 11) : Error   : Attempting to instantiate invalid template type 'weakref<string>'\n"
			"Test_Addon_WeakRef (5, 11) : Error   : Attempting to instantiate invalid template type 'weakref<Test@>'\n"
			"weakref (0, 0) : Error   : The subtype doesn't support weak references\n"
			"Test_Addon_WeakRef (6, 11) : Error   : Attempting to instantiate invalid template type 'weakref<array<Test>>'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Chained assignment with weak-ref
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptWeakRef(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("MyClass", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_FACTORY, "MyClass @f()", asFUNCTION(MyClass::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_ADDREF, "void f()", asMETHOD(MyClass, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_RELEASE, "void f()", asMETHOD(MyClass, Release), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asMETHOD(MyClass, GetWeakRefFlag), asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"weakref<MyClass> sm_fireDB; \n"
			"MyClass@ GetFireDB() \n"
			"{ \n"
			"	MyClass@ db; \n"
			"	@db = sm_fireDB.get(); \n"
			"	if (db !is null) return db; \n"
			"	@sm_fireDB = @db = MyClass(); \n"   // this produces the correct result, since the db holds the strong reference to the temporary object
			"	assert( db !is null && db is sm_fireDB.get() ); \n"
			"	@db = @sm_fireDB = MyClass(); \n"   // this produced a null pointer, since the temporary object is destroyed as soon as the weakref has finished assigning it (since it doesn't hold a strong ref)
			"	assert( db is null && db is sm_fireDB.get() ); \n"
			"	return db; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "GetFireDB()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test registering app type with weak ref
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("MyClass", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_FACTORY, "MyClass @f()", asFUNCTION(MyClass::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_ADDREF, "void f()", asMETHOD(MyClass, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_RELEASE, "void f()", asMETHOD(MyClass, Release), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("MyClass", asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asMETHOD(MyClass, GetWeakRefFlag), asCALL_THISCALL);

		RegisterScriptWeakRef(engine);

		const char *script =
			"void main() { \n"
			"  MyClass @t = MyClass(); \n"
			"  weakref<MyClass> r(t); \n"
			"  assert( r.get() !is null ); \n"
			"  @t = null; \n"
			"  assert( r.get() is null ); \n"
			"} \n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME, script);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", TESTNAME);
		}

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// TODO: weak: It should be possible to declare a script class to not allow weak references, and as such save the memory for the internal pointer
	// TODO: weak: add engine property to turn off automatic support for weak references to all script classes

	// Success
	return fail;
}


} // namespace

