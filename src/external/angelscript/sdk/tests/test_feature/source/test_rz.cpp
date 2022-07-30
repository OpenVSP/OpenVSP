#include "utils.h"
#include "../../../add_on/scriptany/scriptany.h"


namespace TestRZ
{

const char *script1 = "\n"
"MyGame @global;       \n"
"class MyGame          \n"
"{                     \n"
// Cause GC to keep a reference (for testing purposes)
"  MyGame@ ref;        \n"
"  MyGame@[] array;    \n"
"}                     \n"
"any@ CreateInstance() \n"
"{                     \n"
"  any res;            \n"
"  MyGame obj;         \n"
"  @global = @obj;     \n"
"  res.store(@obj);    \n"
"  return res;         \n"
"}                     \n";


bool Test1()
{
	bool fail = false;
	int r = 0;
	COutStream out;
 	asIScriptEngine *engine;
	int refCount;

	asIScriptObject *myGame = 0;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	RegisterScriptAny(engine);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script1, strlen(script1));
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	// Calling the garbage collector mustn't free the object types, even though they are not used yet
	int tid1 = engine->GetModule(0)->GetTypeIdByDecl("MyGame@[]");
	engine->GarbageCollect();
	int tid2 = engine->GetModule(0)->GetTypeIdByDecl("MyGame@[]");

	if( tid1 != tid2 )
	{
		PRINTF("Object type was released incorrectly by GC\n");
		TEST_FAILED;
	}

	// Make sure ref count is properly updated
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(engine->GetModule(0)->GetFunctionByName("CreateInstance"));
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("execution failed\n");
		TEST_FAILED;
	}
	else
	{
		CScriptAny *any = *(CScriptAny**)ctx->GetAddressOfReturnValue();
		int typeId = any->GetTypeId();
		if( !(typeId & asTYPEID_OBJHANDLE) )
		{
			PRINTF("not a handle\n");
			TEST_FAILED;
		}

		// Retrieve will increment the reference count for us
		any->Retrieve(&myGame, typeId);

		// What is the refcount?
		myGame->AddRef();
		refCount = myGame->Release();

		// GC, any, global, application
		if( refCount != 4 )
		{
			PRINTF("ref count is wrong\n");
			TEST_FAILED;
		}

		// Clear the reference that the any object holds (this is not necessary)
		double zero = 0.0;
		any->Store(zero);

		// What is the refcount?
		myGame->AddRef();
		refCount = myGame->Release();

		// GC, global, application
		if( refCount != 3 )
		{
			PRINTF("ref count is wrong\n");
			TEST_FAILED;
		}
	}

	// Call abort on the context to free up resources (this is not necessary)
	ctx->Abort();

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// GC, global, application
	if( refCount != 3 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release the context
	ctx->Release();
	ctx = 0;

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// GC, global, application
	if( refCount != 3 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Call garbage collection
	engine->GarbageCollect();

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// GC, global, application
	if( refCount != 3 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Discard the module, freeing the global variable
	engine->DiscardModule(0);

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// GC, application
	if( refCount != 2 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release the game object
	refCount = myGame->Release();

	// GC
	if( refCount != 1 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release engine
	engine->Release();
	engine = 0;

	// Success
 	return fail;
}




bool Test2()
{
	bool fail = false;
	int r = 0;
	COutStream out;
 	asIScriptEngine *engine;
	int refCount;

	asIScriptObject *myGame = 0;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	RegisterScriptAny(engine);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	const char *script2 = "\n"
		"MyGame @global;       \n"
		"class MyGame          \n"
		"{                     \n"
		"}                     \n"
		"any@ CreateInstance() \n"
		"{                     \n"
		"  any res;            \n"
		"  MyGame obj;         \n"
		"  @global = @obj;     \n"
		"  res.store(@obj);    \n"
		"  return res;         \n"
		"}                     \n";
	mod->AddScriptSection("script", script2, strlen(script2));
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	// Make sure ref count is properly updated
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(engine->GetModule(0)->GetFunctionByName("CreateInstance"));
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("execution failed\n");
		TEST_FAILED;
	}
	else
	{
		CScriptAny *any = *(CScriptAny**)ctx->GetAddressOfReturnValue();
		int typeId = any->GetTypeId();
		if( !(typeId & asTYPEID_OBJHANDLE) )
		{
			PRINTF("not a handle\n");
			TEST_FAILED;
		}

		// Retrieve will increment the reference count for us
		any->Retrieve(&myGame, typeId);

		// What is the refcount?
		myGame->AddRef();
		refCount = myGame->Release();

		// any, global, application
		if( refCount != 3 )
		{
			PRINTF("ref count is wrong\n");
			TEST_FAILED;
		}

		// Clear the reference that the any object holds (this is not necessary)
		double zero = 0.0;
		any->Store(zero);

		// What is the refcount?
		myGame->AddRef();
		refCount = myGame->Release();

		// global, application
		if( refCount != 2 )
		{
			PRINTF("ref count is wrong\n");
			TEST_FAILED;
		}
	}

	// Call abort on the context to free up resources (this is not necessary)
	ctx->Abort();

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// global, application
	if( refCount != 2 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release the context
	ctx->Release();
	ctx = 0;

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// global, application
	if( refCount != 2 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Call garbage collection
	engine->GarbageCollect();

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// global, application
	if( refCount != 2 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Discard the module, freeing the global variable
	engine->DiscardModule(0);

	// What is the refcount?
	myGame->AddRef();
	refCount = myGame->Release();

	// application
	if( refCount != 1 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release the game object
	refCount = myGame->Release();

	// nobody
	if( refCount != 0 )
	{
		PRINTF("ref count is wrong\n");
		TEST_FAILED;
	}

	// Release engine
	engine->Release();
	engine = 0;

	// Success
 	return fail;
}

static int g_printCount = 0;
void Print()
{
	g_printCount++;
}

void GetClassInstance(asIScriptEngine *engine, asIScriptFunction *func, asIScriptObject* &retObj, int& retTypeId)
{
	int r;
	asIScriptContext* ctxt = engine->CreateContext();	
	r = ctxt->Prepare( func );
	r = ctxt->Execute();
	
	CScriptAny *anyResult = *(CScriptAny **)ctxt->GetAddressOfReturnValue();
	retTypeId = anyResult->GetTypeId();		

	retObj = NULL;
	r = anyResult->Retrieve( (void*)&retObj, retTypeId );

	// replace it in the any to clear it out
	asINT64 dummy = 0;
	anyResult->Store( dummy );
	
	// and clean out the return object (just as a precaution)
	ctxt->Abort();
	ctxt->Release();
}

bool Test3()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;

	const char *script =
		"interface IMyInterface { void SomeFunc(); } \n"
		"class MyBaseClass : IMyInterface { ~MyBaseClass(){ Print(); } void SomeFunc(){} } \n"
		"class MyDerivedClass : MyBaseClass \n"
		"{ \n"
		"   IMyInterface@ m_obj; \n"
		"	MyDerivedClass(){} \n"
		"	void SetObj( IMyInterface@ obj ) { @m_obj = obj; } \n"
		"	void ClearObj(){ @m_obj = null; } \n"
		"} \n"
		"void SomeOtherFunction(){}\n"
		"any@ GetClass(){ \n"
		"  MyDerivedClass x; \n"
		"  any a( @x ); \n"
		"  return a;\n"
		"} \n";

	const char *script2 = 
		"class AClass { void Blah(){} void Blah2(){} void Blah3(){} void Blah4(){} void Blah5(){} }\n"
		"void SomeBlahFunc(){ }\n";

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	RegisterStdString(engine);
	RegisterScriptAny(engine);

	engine->RegisterGlobalFunction( "void Print()", asFUNCTION(Print), asCALL_CDECL );

	asIScriptModule *mod = engine->GetModule( "test", asGM_ALWAYS_CREATE );
	mod->AddScriptSection("script", script);
	r = mod->Build();

	// create two instances of our classes
	asIScriptFunction *func = mod->GetFunctionByDecl( "any@ GetClass()" );
	
	asIScriptObject* objA;
	int objATypeId;
	GetClassInstance( engine, func, objA, objATypeId );

	asIScriptObject* objB;
	int objBTypeId;
	GetClassInstance( engine, func, objB, objBTypeId );

	// resolve method functions we want to call
	asITypeInfo* typeA = engine->GetTypeInfoById( objATypeId );
	asIScriptFunction *setFunc = typeA->GetMethodByDecl( "void SetObj( IMyInterface@ obj )" );
	asIScriptFunction *clearFunc = typeA->GetMethodByDecl( "void ClearObj()" );

	// set our objB into objA
	{
		asIScriptContext* ctxt = engine->CreateContext();
		r = ctxt->Prepare( setFunc );
		r = ctxt->SetObject( objA );
		r = ctxt->SetArgObject( 0, objB );
		r = ctxt->Execute();
		ctxt->Release();
	}

	// release objB...
	objB->Release();
	objB = NULL;
	objBTypeId = 0;

	// clear objB from objA
	{
		asIScriptContext* ctxt = engine->CreateContext();
		r = ctxt->Prepare( clearFunc );
		r = ctxt->SetObject( objA );
		r = ctxt->Execute();
		ctxt->Release();
	}

	// release objA
	objA->Release();
	objA = NULL;
	objATypeId = 0;

	// There are still objects held alive in the GC
	unsigned int gcCount;
	engine->GetGCStatistics(&gcCount);
	assert( gcCount != 0 ); // The script class types and functions are also in the gc

	// discard the module - no longer in use
	// The module will be discarded, but the functions that the live objects use will remain
	r = engine->DiscardModule("test");	
	if( r < 0 )
		TEST_FAILED;

	// Do a couple of more builds, so that the memory freed by DiscardModule is reused otherwise 
	// the problem may not occur, as the memory is still there, even though it was freed

	// create a module
	mod = engine->GetModule( "test2", asGM_ALWAYS_CREATE );
	mod->AddScriptSection( "script", script2 );
	r = mod->Build();

	// recreate the module
	mod = engine->GetModule( "test", asGM_ALWAYS_CREATE );
	mod->AddScriptSection("script", script);
	r = mod->Build();

	// run the garbage collector to 'clean things up'
	r = engine->GarbageCollect(asGC_FULL_CYCLE);

	// Print is called by each script class' destructor, even though the module has already been discarded
	if( g_printCount != 2 )
		TEST_FAILED;

	// we're done
	engine->Release();
		

	return fail;
}

bool Test()
{
	if( Test1() ) return true;

	if( Test2() ) return true;

	// This problem was reported by Jeff Slutter. Apparently the garbage collector is trying  
	// to execute the destructor of an object whose module has already been discarded.
	if( Test3() ) return true;

	return false;
}

} // namespace

