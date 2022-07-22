#include "utils.h"
#include "../../../add_on/scriptany/scriptany.h"
#include "../../../add_on/scripthandle/scripthandle.h"
#include "scriptmath3d.h"

namespace TestAny
{

// Normal functionality
static const char *script1 =
"string g_str = \"test\";               \n"
"any g_any(@g_str);                     \n"
"void TestAny()                         \n"
"{                                      \n"
"  any a, b;                            \n"
"  string str = \"test\";               \n"
"  b.store(@str);                       \n"
"  a = b;                               \n"
"  string @s;                           \n"
"  a.retrieve(@s);                      \n"
"  Assert(s == str);                    \n"
"  Assert(@s == @str);                  \n"
"  int[]@ c;                            \n"
"  a.retrieve(@c);                      \n"
"  Assert(@c == null);                  \n"
"  a = any(@str);                       \n"
"  a.retrieve(@s);                      \n"
"  Assert(s == str);                    \n"
"  any d(@str);                         \n"
"  d.retrieve(@s);                      \n"
"  Assert(s == str);                    \n"
"  g_any.retrieve(@s);                  \n"
"  Assert(@s == @g_str);                \n"
// If the container holds a handle to a const object, it must not copy this to a handle to a non-const object
"  const string @cs = str;              \n"
"  a.store(@cs);                        \n"
"  a.retrieve(@s);                      \n"
"  Assert(@s == null);                  \n"
"  @cs = null;                          \n"
"  a.retrieve(@cs);                     \n"
"  Assert(@cs == @str);                 \n"
// If the container holds a handle to a non-const object, it should be able to copy it to a handle to a const object
"  @s = str;                            \n"
"  a.store(@s);                         \n"
"  a.retrieve(@cs);                     \n"
"  Assert(@cs == @str);                 \n"
"}                                      \n";

// Test circular references with any
static const char *script2 =
"class s                                \n"
"{                                      \n"
"  any a;                               \n"
"};                                     \n"
"void TestAny()                         \n"
"{                                      \n"
"  any a;                               \n"
"  a.store(@a);                         \n"
"  any b,c;                             \n"
"  b.store(@c);                         \n"
"  c.store(@b);                         \n"
"  any[] d(1);                          \n"
"  d[0].store(@d);                      \n"
"  s e;                                 \n"
"  e.a.store(@e);                       \n"
"}                                      \n";

// Don't allow a ref to const in retrieve()
static const char *script3 =
"class s                   \n"
"{                         \n"
"  string @a;              \n"
"};                        \n"
"void TestAny()            \n"
"{                         \n"
"  const s a;              \n"
"  any c;                  \n"
"  c.retrieve(@a.a);       \n"
"}                         \n"; 

static const char *script4 =
"void TestAny()            \n"
"{                         \n"
"  string s = \"test\";    \n"
"  any a(@s);              \n"
"  SetMyAny(a);            \n"
"}                         \n";

static CScriptAny *myAny = 0;
void SetMyAny(asIScriptGeneric *gen)
{
	CScriptAny *a = (CScriptAny *)gen->GetArgObject(0);
	// Only call AddRef to hold the ref if the generic calling convention is using the old mode
	if( gen->GetEngine()->GetEngineProperty(asEP_GENERIC_CALL_MODE) == 0 )
		a->AddRef();
	if( myAny ) myAny->Release();
	myAny = a;
}

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	asIScriptContext* ctx;
	asIScriptEngine* engine;
	CBufferedOutStream bout;
	asIScriptModule* mod;

	// Test setting any with script function from c++
	// https://www.gamedev.net/forums/topic/707596-dynamic-import-of-native-functions/5429530/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "void func() {}");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		if (func == 0)
			TEST_FAILED;

		CScriptAny* any = new CScriptAny(engine);
		any->Store(&func, func->GetTypeId() | asTYPEID_OBJHANDLE);

		func = 0;
		any->Retrieve(&func, any->GetTypeId());

		if (func == 0 || std::string(func->GetName()) != "func")
			TEST_FAILED;
		if( func )
			func->Release();
		any->Release();

		engine->ShutDownAndRelease();

	}

	// Test setting content of any from C++
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);
		CScriptAny* any = new CScriptAny(engine);

		asINT64 someValue = 32;
		any->Store(&someValue, asTYPEID_INT64);

		someValue = 0;
		any->Retrieve(&someValue, asTYPEID_INT64);

		if (someValue != 32)
			TEST_FAILED;
		any->Release();

		engine->ShutDownAndRelease();
	}

	// Test circular reference between any and ref
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptHandle(engine);
		RegisterScriptAny(engine);

		// Create the circular reference
		r = ExecuteString(engine, "any a; ref @r = a; a.store(r);");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->GarbageCollect();

		asUINT currSize, totDestroy, totDetect;
		engine->GetGCStatistics(&currSize, &totDestroy, &totDetect);
		if (currSize != 0 || totDestroy != 1 || totDetect != 1)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test initialization from another any
	// http://www.gamedev.net/topic/678208-scriptany-addon-call-wrong-constructor-in-2310/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() \n"
			"{ \n"
			"  string testh = 'hello'; \n"
			"  string output; \n"
			"  any data; \n"
			"  data.store(testh); \n"
			"  any temp = data; \n"
			"  temp.retrieve(output); \n"
			"  assert( output == testh ); \n"
			"  temp = data; \n"
			"  temp.retrieve(output); \n"
			"  assert( output == testh ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (r == asEXECUTION_EXCEPTION)
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test bug where the any type destroyed asITypeInfo too early
	// http://www.gamedev.net/topic/676125-scriptany-addon-eats-typeinfo-references-when-nulls-are-stored/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"
			"void func() \n"
			"{ \n"
			"  A @a; \n"
			"  any c(@a); \n" // store a null handle
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "func()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (r == asEXECUTION_EXCEPTION)
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test bug
	// http://www.gamedev.net/topic/672536-patch-proposal-for-scriptany-add-on-issue/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, 
			"any myAny(10); \n"
			"int myValue; \n"
			"myAny.retrieve(myValue); \n"
			"assert( myValue == 10 ); \n");
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// ---------------------------------------------
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		RegisterScriptString_Generic(engine);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		r = engine->RegisterGlobalFunction("void SetMyAny(any@)", asFUNCTION(SetMyAny), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("TestAny", script1, strlen(script1), 0);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", "TestAny");
		}
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestAny()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			TEST_FAILED;
			PRINTF("%s: Execution failed\n", "TestAny");
		}
		if( ctx ) ctx->Release();
		engine->Release();
	}

	//--------------------------------------------------
	// Verify that the GC can handle circles with any structures
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		RegisterScriptString_Generic(engine);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		r = engine->RegisterGlobalFunction("void SetMyAny(any@)", asFUNCTION(SetMyAny), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("TestAny", script2, strlen(script2), 0);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile the script\n", "TestAny");
		}
		ctx = engine->CreateContext();
		r = ExecuteString(engine, "TestAny()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());

			TEST_FAILED;
			PRINTF("%s: Execution failed\n", "TestAny");
		}
		if( ctx ) ctx->Release();
		
		asUINT gcCurrentSize, gcTotalDestroyed, gcTotalDetected;
		engine->GetGCStatistics(&gcCurrentSize, &gcTotalDestroyed, &gcTotalDetected);
		engine->GarbageCollect();
		engine->GetGCStatistics(&gcCurrentSize, &gcTotalDestroyed, &gcTotalDetected);

		assert( fail || gcCurrentSize == 0 && gcTotalDestroyed == 7 && gcTotalDetected == 7 );

		engine->Release();
	}

	//-------------------------------------------------------
	// Don't allow const handle to retrieve()
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		RegisterScriptString_Generic(engine);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		r = engine->RegisterGlobalFunction("void SetMyAny(any@)", asFUNCTION(SetMyAny), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("TestAny", script3, strlen(script3), 0);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "TestAny (5, 1) : Info    : Compiling void TestAny()\n"
				   "TestAny (9, 15) : Error   : Output argument expression is not assignable\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	//--------------------------------------------------------
	// Make sure it is possible to pass any to the application
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		RegisterScriptString_Generic(engine);
		RegisterScriptAny(engine);
		engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		r = engine->RegisterGlobalFunction("void SetMyAny(any@)", asFUNCTION(SetMyAny), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("TestAny", script4, strlen(script4), 0);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		r = mod->Build();
		if( r < 0 )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to compile\n", "TestAny");
		}
		
		r = ExecuteString(engine, "TestAny()", mod);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
			PRINTF("%s: Failed to execute\n", "TestAny");
		}

		if( myAny )
		{
			int typeId = myAny->GetTypeId();

			if( !(typeId & asTYPEID_OBJHANDLE) )
				TEST_FAILED;
			if( (typeId & asTYPEID_MASK_OBJECT) != asTYPEID_APPOBJECT )
				TEST_FAILED;

			const char *decl = engine->GetTypeDeclaration(typeId, true);
			if( (decl == 0) || (strcmp(decl, "string@") != 0) )
			{
				TEST_FAILED;
				PRINTF("%s: Failed to return the correct type\n", "TestAny");
			}

			int typeId2 = engine->GetTypeIdByDecl("::string@");
			if( typeId != typeId2 )
			{
				TEST_FAILED;
				PRINTF("%s: Failed to return the correct type\n", "TestAny");
			}

			CScriptString *str = 0;
			myAny->Retrieve((void*)&str, typeId);

			if( str->buffer != "test" )
			{
				TEST_FAILED;
				PRINTF("%s: Failed to set the string correctly\n", "TestAny");
			}

			if( str ) str->Release();

			myAny->Release();
			myAny = 0;
		}
		else
			TEST_FAILED;

		//--------------------------------------
		// Make sure the any type can store primitives as well
		r = ExecuteString(engine, "any a; a.store(1); int b; a.retrieve(b); Assert(b == 1);");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test storing value type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptMath3D(engine);
		RegisterScriptAny(engine);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	
		const char *script =
			"void main() \n"
			"{ \n"
			" any storage; \n"
			" storage.store(vector3(1,1,1)); \n"
			"} \n";

		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION )
				PRINTF("%s", GetExceptionInfo(ctx).c_str());
			TEST_FAILED;
		}
		ctx->Release();

		engine->Release();
	}

	// Test storing null pointer
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptAny(engine);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, "any a; a.store(null);", 0);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Success
 	return fail;
}

} // namespace

