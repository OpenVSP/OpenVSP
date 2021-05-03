#include "utils.h"
#include <exception>
using namespace std;

namespace TestException
{

std::string printOutput;
static void print(asIScriptGeneric *gen)
{
	std::string *s = (std::string*)gen->GetArgAddress(0);
	printOutput += *s;
}

class ExceptionHandler
{
public:
	ExceptionHandler() : ok(false) {}

	void Callback(asIScriptContext *ctx)
	{
		// Callback was called
		ok = true;

		// Type of exception is what was expected?
		if( string(ctx->GetExceptionString()) != "Null pointer access" )
			ok = false;
	}

	bool ok;
};

void TranslateException(asIScriptContext *ctx, void * /*param*/)
{
	try
	{
		// Retrow the original exception so we can catch it again
		throw;
	}
	catch (const std::exception &e)
	{
		ctx->SetException(e.what());
	}
	catch (...)
	{
		ctx->SetException("Some exception occurred");
	}
}

void ThrowException()
{
	throw std::exception();
}

class foo
{
	public:
	int *opIndex_ThrowException(int) { throw std::exception(); return 0; }
};

class Dummy
{
public:
	Dummy() {refCount = 1;}
	~Dummy() {};
	void AddRef() {refCount++;}
	void Release() { if( --refCount == 0 ) delete this; }
	int refCount;
	static Dummy *Factory() { asGetActiveContext()->SetException("...", true); return new Dummy(); }
};

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;

	// Test script exception in registered factory function
	// https://www.gamedev.net/forums/topic/704577-throwing-script-exception-from-factory-function/
	SKIP_ON_MAX_PORT
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		engine->RegisterObjectType("Dummy", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("Dummy", asBEHAVE_FACTORY, "Dummy @f()", asFUNCTION(Dummy::Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("Dummy", asBEHAVE_ADDREF, "void f()", asMETHOD(Dummy, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("Dummy", asBEHAVE_RELEASE, "void f()", asMETHOD(Dummy, Release), asCALL_THISCALL);

		r = ExecuteString(engine, "Dummy D();");
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}	
		
		engine->ShutDownAndRelease();
	}

	// Test to make sure catching two exceptions in a row works
	// https://www.gamedev.net/forums/topic/700622-catching-two-in-a-row/
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterStdString(engine);
		RegisterExceptionRoutines(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		printOutput = "";
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"void main() { \n"
			"    try { throw ('1'); } \n"
			"    catch { print ('Exception 1\\n'); } \n"
			"    try { throw ('2'); } \n"
			"    catch { print ('Exception 2\\n'); } \n"
			"    try { throw ('3'); } \n"
			"    catch { print ('Exception 3\\n'); } \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		// Test again after saving/loading the bytecode
		bout.buffer = "";
		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1);
		assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();
		
		if( printOutput != "Exception 1\n"  // first test
						   "Exception 2\n"
						   "Exception 3\n"
						   "Exception 1\n"  // second test
						   "Exception 2\n"
						   "Exception 3\n" )
		{
			PRINTF("%s", printOutput.c_str());
			TEST_FAILED;
		}
		
		engine->ShutDownAndRelease();
	}
	
	// Test to make sure stack unwind to catch block doesn't clean-up objects created before the try block
	// Identified by Polyak Istvan
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterStdString(engine);
		RegisterExceptionRoutines(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		printOutput = "";
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class C \n"
			"{ \n"
			"    C (const string &in t) \n"
			"    { \n"
			"        t_ = t; \n"
			"        print('C: ' + t_ + '\\n'); \n"
			"    } \n"
			"    ~C () \n"
			"    { \n"
			"        print('~C: ' + t_ + '\\n'); \n"
			"    } \n"
			"    string t_; \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"    C c1('1'); \n"
			"    print('before try\\n'); \n"
			"	try \n"
			"	{ \n"
			"		C c2('2'); \n"
			"		throw('exception'); \n"
			"	} \n"
			"	catch \n"
			"	{ \n"
			"		print('Caught \\'' + getExceptionInfo() + '\\'\\n'); \n"
			"	} \n"
			"    print('after catch\\n'); \n"
			"	print('Still here: ' + c1.t_ + '\\n'); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		// Test again after saving/loading the bytecode
		bout.buffer = "";
		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1);
		assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();
		
		if( printOutput != "C: 1\n"  // first test
						   "before try\n"
						   "C: 2\n"
						   "~C: 2\n"
						   "Caught 'exception'\n"
						   "after catch\n"
						   "Still here: 1\n"
						   "~C: 1\n"
						   "C: 1\n"  // second test
						   "before try\n"
						   "C: 2\n"
						   "~C: 2\n"
						   "Caught 'exception'\n"
						   "after catch\n"
						   "Still here: 1\n"
						   "~C: 1\n" )
		{
			PRINTF("%s", printOutput.c_str());
			TEST_FAILED;
		}
		
		engine->ShutDownAndRelease();
	}
	
	// Test that the compiler can identify that both try and catch block returns so nothing afterwards can be executed
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"void Test()                    \n"
			"{                              \n"
			"  try {                        \n"
			"   return;                     \n"
			"  } catch {                    \n"
			"   return;                     \n"
			"  }                            \n"
			"  int a;                       \n" // this cannot be reached
			"}                              \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "script (1, 1) : Info    : Compiling void Test()\n"
						   "script (8, 3) : Warning : Unreachable code\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test catching an exception
	// TODO: Test rethrowing exception in catch block
	// TODO: Test use of try/catch in constructor to call base class' constructor
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterStdString(engine);
		RegisterExceptionRoutines(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"bool exceptionCaught = false;  \n"
			"void Test()                    \n"
			"{                              \n"
			"  try {                        \n"
			"   string test = 'hello';      \n" // this shouldn't be cleaned up by exception
			"   try {                       \n"
			"    string cleanup = 'clean';  \n" // this must be cleaned up
			"    throw('test');             \n" // an exception is thrown here
			"    test = 'blah';             \n" // this is never executed
			"   } catch {                   \n"
			"    assert( getExceptionInfo() == 'test' ); \n"
			"    exceptionCaught = true;    \n" // signal that the exception was caught
			"    assert( test == 'hello' ); \n" // original value
			"   }                           \n"
			"  }                            \n"
			"  catch {                      \n"
			"    assert( false );           \n" // never executed
			"  }                            \n"
			"}                              \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		asIScriptContext *ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("Test"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		if (*(bool*)mod->GetAddressOfGlobalVar(0) != true)
			TEST_FAILED;
		
		// Test again after saving/loading the bytecode
		bout.buffer = "";
		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1);
		assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		ctx = engine->CreateContext();
		r = ctx->Prepare(mod->GetFunctionByName("Test"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		if (*(bool*)mod->GetAddressOfGlobalVar(0) != true)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test exception translation
	SKIP_ON_MAX_PORT
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetTranslateAppExceptionCallback(asFUNCTION(TranslateException), 0, asCALL_CDECL);

		engine->RegisterGlobalFunction("void ThrowException()", asFUNCTION(ThrowException), asCALL_CDECL);

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "ThrowException()", 0, ctx);
		if (r != asEXECUTION_EXCEPTION)
		{
			TEST_FAILED;
		}
		if (ctx->GetExceptionString() == 0 || 
			(string(ctx->GetExceptionString()) != "Unknown exception" && // msvc
			 string(ctx->GetExceptionString()) != "std::exception"))      // gnuc
		{
			PRINTF("Exception: '%s'\n", ctx->GetExceptionString());
			TEST_FAILED;
		}
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Test exception translation in opIndex(int) (asBC_Thiscall1)
	// Reported by Quentin Cosendey
	SKIP_ON_MAX_PORT
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetTranslateAppExceptionCallback(asFUNCTION(TranslateException), 0, asCALL_CDECL);

		engine->RegisterObjectType("foo", sizeof(foo), asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectMethod("foo", "int &opIndex(int)", asMETHOD(foo, opIndex_ThrowException), asCALL_THISCALL);

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "foo bar; bar[0]", 0, ctx);
		if (r != asEXECUTION_EXCEPTION)
		{
			TEST_FAILED;
		}
		if (ctx->GetExceptionString() == 0 || 
			(string(ctx->GetExceptionString()) != "Unknown exception" && // msvc
			 string(ctx->GetExceptionString()) != "std::exception"))      // gnuc
		{
			PRINTF("Exception: '%s'\n", ctx->GetExceptionString());
			TEST_FAILED;
		}
		ctx->Release();

		engine->ShutDownAndRelease();
	}

	// Basic tests
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptString(engine);
		printOutput = "";
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_GENERIC);


		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "int a = 0;\na = 10/a;", 0, ctx); // Throws an exception
		if (r == asEXECUTION_EXCEPTION)
		{
			int line = ctx->GetExceptionLineNumber();
			const char *desc = ctx->GetExceptionString();

			const asIScriptFunction *function = ctx->GetExceptionFunction();
			if (strcmp(function->GetName(), "ExecuteString") != 0)
			{
				PRINTF("Exception function name is wrong\n");
				TEST_FAILED;
			}
			if (strcmp(function->GetDeclaration(), "void ExecuteString()") != 0)
			{
				PRINTF("Exception function declaration is wrong\n");
				TEST_FAILED;
			}

			if (line != 2)
			{
				PRINTF("Exception line number is wrong\n");
				TEST_FAILED;
			}
			if (strcmp(desc, "Divide by zero") != 0)
			{
				PRINTF("Exception string is wrong\n");
				TEST_FAILED;
			}
		}
		else
		{
			PRINTF("Failed to raise exception\n");
			TEST_FAILED;
		}

		ctx->Release();

		// This script will cause an exception inside a class method
		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"class A               \n"
			"{                     \n"
			"  void Test(string c) \n"
			"  {                   \n"
			"    int a = 0, b = 0; \n"
			"    a = a/b;          \n"
			"  }                   \n"
			"}                     \n");
		mod->Build();
		r = ExecuteString(engine, "A a; a.Test(\"test\");", mod);
		if (r != asEXECUTION_EXCEPTION)
		{
			TEST_FAILED;
		}

		// A test to validate Unprepare without execution
		{
			asITypeInfo *type = mod->GetObjectTypeByIndex(0);
			asIScriptFunction *func = type->GetMethodByDecl("void Test(string c)");
			ctx = engine->CreateContext();
			ctx->Prepare(func);
			asIScriptContext *obj = (asIScriptContext*)engine->CreateScriptObject(type);
			ctx->SetObject(obj); // Just sets the address
			CScriptString *str = new CScriptString();
			ctx->SetArgObject(0, str); // Makes a copy of the object
			str->Release();
			ctx->Unprepare(); // Must release the string argument, but not the object
			ctx->Release();
			obj->Release();
		}

		// Another test to validate Unprepare without execution
		{
			asITypeInfo *type = mod->GetObjectTypeByIndex(0);
			// Get the real method, not the virtual method
			asIScriptFunction *func = type->GetMethodByDecl("void Test(string c)", false);
			ctx = engine->CreateContext();
			ctx->Prepare(func);
			// Don't set the object, nor the arguments
			ctx->Unprepare();
			ctx->Release();
		}

		// A test to verify behaviour when exception occurs in script class constructor
		const char *script2 = "class SomeClassA \n"
			"{ \n"
			"	int A; \n"
			" \n"
			"	~SomeClassA() \n"
			"	{ \n"
			"		print('destruct'); \n"
			"	} \n"
			"} \n"
			"class SomeClassB \n"
			"{ \n"
			"	SomeClassA@ nullptr; \n"
			"	SomeClassB(SomeClassA@ aPtr) \n"
			"	{ \n"
			"		this.nullptr.A=100; // Null pointer access. After this class a is destroyed. \n"
			"	} \n"
			"} \n"
			"void test() \n"
			"{ \n"
			"	SomeClassA a; \n"
			"	SomeClassB(a); \n"
			"} \n";
		mod->AddScriptSection("script2", script2);
		r = mod->Build();
		if (r < 0) TEST_FAILED;
		r = ExecuteString(engine, "test()", mod);
		if (r != asEXECUTION_EXCEPTION)
		{
			TEST_FAILED;
		}

		engine->GarbageCollect();

		engine->Release();
	}

	// Problem reported by Philip Bennefall
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);
		RegisterScriptArray(engine, true);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test",
			"string post_score(string url, string channel, string channel_password, int score, string name, string email, string country) \n"
			"{ \n"
			"  string[] list={'something'}; \n"
			"  return list[1]; \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  string result=post_score('hello', 'palacepunchup', 'anka', -1, 'Philip', 'philip@blastbay.com', 'Sweden'); \n"
			"}\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		if( string(ctx->GetExceptionString()) != "Index out of bounds" )
			TEST_FAILED;
		if( string(ctx->GetExceptionFunction()->GetName()) != "post_score" )
			TEST_FAILED;

		ctx->Release();
		engine->Release();
	}

	// Test exception within default constructor
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);

		mod->AddScriptSection("test",
			"class Test { \n"
			"  string mem = 'hello'; \n"
			"  int a = 0; \n"
			"  int b = 10/a; \n"
			"}\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "Test t;", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		if( string(ctx->GetExceptionString()) != "Divide by zero" )
		{
			PRINTF("%s\n", ctx->GetExceptionString());
			TEST_FAILED;
		}
		if( string(ctx->GetExceptionFunction()->GetName()) != "Test" )
			TEST_FAILED;

		ctx->Release();
		engine->Release();
	}

	// Test exception in for-condition
	// http://www.gamedev.net/topic/638128-bug-with-show-code-line-after-null-pointer-exception-and-for/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A\n"
			"{\n"
			"    int GetCount(){ return 10; }\n"
			"}\n"
			"void startGame()\n"
			"{\n"
			"    A @a = null;\n"
			"    for( int i=0; i < a.GetCount(); i++ )\n"
			"    {\n"
			"        int some_val;\n"
			"    }\n"
			"}\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "startGame();", mod, ctx);
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		if( string(ctx->GetExceptionString()) != "Null pointer access" )
		{
			PRINTF("%s\n", ctx->GetExceptionString());
			TEST_FAILED;
		}
		if( string(ctx->GetExceptionFunction()->GetName()) != "startGame" )
			TEST_FAILED;
		if( ctx->GetExceptionLineNumber() != 8 )
			TEST_FAILED;

		ctx->Release();
		engine->Release();
	}

	// Test exception handler with thiscall
	// http://www.gamedev.net/topic/665587-angelscript-ios-x64/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class SomeClass \n"
			"{ \n"
			"	float some_value; \n"
			"} \n"
			"SomeClass @obj; \n"
			"void test() \n"
			"{\n"
			"	obj.some_value = 3.14f; \n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		ExceptionHandler handler;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->SetExceptionCallback(asMETHOD(ExceptionHandler, Callback), &handler, asCALL_THISCALL);
		ctx->Prepare(mod->GetFunctionByName("test"));
		r = ctx->Execute();
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		ctx->Release();
		
		if( handler.ok == false )
			TEST_FAILED;

		engine->Release();
	}

	// Success
	return fail;
}

}
