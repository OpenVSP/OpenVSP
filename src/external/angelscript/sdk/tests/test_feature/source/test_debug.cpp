
#include <stdarg.h>
#include "utils.h"

namespace TestDebug
{

static const char *script1 =
"import void Test2() from \"Module2\";  \n"
"funcdef void func_t();                 \n"
"void main()                            \n"
"{                                      \n"
"  int a = 1;                           \n"
"  string s = \"text\";                 \n" // 6
"  c _c; _c.Test1();                    \n" // 7
"  func_t @t = main;                    \n"
"  Test2();                             \n"
"}                                      \n"
"class c                                \n" 
"{                                      \n"
"  void Test1()                         \n" // 13
"  {                                    \n"
"    int d = 4;                         \n"
"  }                                    \n"
"}                                      \n";

static const char *script2 =
"void Test2()           \n"
"{                      \n"
"  int b = 2;           \n"
"  Test3();             \n" // 4
"}                      \n"
"void Test3()           \n"
"{                      \n"
"  int c = 3;           \n"
"  int[] a;             \n" // 9
"  a[0] = 0;            \n" // 10
"}                      \n";


std::string printBuffer;

static const char *correct =
"Module1:void main():5,3\n"
"Module1:void main():5,3\n"
"Module1:void main():6,3\n"
"Module1:void main():7,3\n"
" Module1:c@ c():0,0\n"
"  Module1:c::c():0,0\n"
"Module1:void main():7,9\n"
" Module1:void c::Test1():15,5\n"
" Module1:void c::Test1():15,5\n"
" Module1:void c::Test1():16,4\n"
"Module1:void main():8,3\n"
"Module1:void main():9,3\n"
" Module2:void Test2():3,3\n"
" Module2:void Test2():3,3\n"
" Module2:void Test2():4,3\n"
"  Module2:void Test3():8,3\n"
"  Module2:void Test3():8,3\n"
"  Module2:void Test3():9,3\n"
"   (null):int[]@ $fact():0,0\n"
"  Module2:void Test3():10,3\n"
"--- exception ---\n"
"desc: Index out of bounds\n"
"func: void Test3()\n"
"modl: Module2\n"
"sect: :2\n"
"line: 10,3\n"
" int c = 3\n"
" int[] a = {...}\n"
"--- call stack ---\n"
"Module2:void Test2():4,3\n"
" int b = 2\n"
"Module1:void main():9,3\n"
" int a = 1\n"
" string s = 'text'\n"
" c _c = {...}\n"
" func_t@ t = {...}\n"
"--- exception ---\n"
"desc: Index out of bounds\n"
"func: void Test3()\n"
"modl: Module2\n"
"sect: :2\n"
"line: 10,3\n"
" int c = 3\n"
" int[] a = {...}\n"
"--- call stack ---\n"
"Module2:void Test2():4,3\n"
" int b = 2\n"
"Module1:void main():9,3\n"
" int a = 1\n"
" string s = 'text'\n"
" c _c = {...}\n"
" func_t@ t = {...}\n";

void print(const char *format, ...)
{
	char buf[256];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	printBuffer += buf;
}

void PrintVariables(asIScriptContext *ctx, asUINT stackLevel);

void LineCallback(asIScriptContext *ctx, void * /*param*/)
{
	if( ctx->GetState() != asEXECUTION_ACTIVE )
		return;

	int col;
	int line = ctx->GetLineNumber(0, &col);
	int indent = ctx->GetCallstackSize();
	for( int n = 1; n < indent; n++ )
		print(" ");
	const asIScriptFunction *function = ctx->GetFunction();
	print("%s:%s:%d,%d\n", function->GetModuleName(),
	                    function->GetDeclaration(),
	                    line, col);

//	PrintVariables(ctx, 0);
}

void PrintVariables(asIScriptContext *ctx, asUINT stackLevel)
{
	asIScriptEngine *engine = ctx->GetEngine();

	int typeId = ctx->GetThisTypeId(stackLevel);
	void *varPointer = ctx->GetThisPointer(stackLevel);
	if( typeId )
	{
		print(" this = 0x%x\n", varPointer);
	}

	int numVars = ctx->GetVarCount(stackLevel);
	for( int n = 0; n < numVars; n++ )
	{
		typeId = ctx->GetVarTypeId(n, stackLevel); 
		varPointer = ctx->GetAddressOfVar(n, stackLevel);
		if( typeId == engine->GetTypeIdByDecl("int") )
		{
			print(" %s = %d\n", ctx->GetVarDeclaration(n, stackLevel), *(int*)varPointer);
		}
		else if( typeId == engine->GetTypeIdByDecl("string") )
		{
			CScriptString *str = (CScriptString*)varPointer;
			if( str )
				print(" %s = '%s'\n", ctx->GetVarDeclaration(n, stackLevel), str->buffer.c_str());
			else
				print(" %s = <null>\n", ctx->GetVarDeclaration(n, stackLevel));
		}
		else
		{
			print(" %s = {...}\n", ctx->GetVarDeclaration(n, stackLevel));
		}
	}
}

void ExceptionCallback(asIScriptContext *ctx, void * /*param*/)
{
	const asIScriptFunction *function = ctx->GetExceptionFunction();
	print("--- exception ---\n");
	print("desc: %s\n", ctx->GetExceptionString());
	print("func: %s\n", function->GetDeclaration());
	print("modl: %s\n", function->GetModuleName());
	print("sect: %s\n", function->GetScriptSectionName());
	int col, line = ctx->GetExceptionLineNumber(&col);
	print("line: %d,%d\n", line, col);

	// Print the variables in the current function
	PrintVariables(ctx, 0);

	// Show the call stack with the variables
	print("--- call stack ---\n");
	for( asUINT n = 1; n < ctx->GetCallstackSize(); n++ )
	{
		const asIScriptFunction *func = ctx->GetFunction(n);
		line = ctx->GetLineNumber(n,&col);
		print("%s:%s:%d,%d\n", func->GetModuleName(),
		                       func->GetDeclaration(),
							   line, col);
		PrintVariables(ctx, n);
	}
}

class TestLN
{
public:
void *TestLineNumber()
{
    asIScriptContext *ctx = asGetActiveContext();
    const char *script_section;
    /*int line =*/ ctx->GetLineNumber(0, 0, &script_section);
	assert( std::string(script_section) == "a" );
    return 0;
}
};

bool Test2();

bool Test()
{
	bool fail = Test2();

	// GetTypeByDeclaration shouldn't write message on incorrect declaration
	// http://www.gamedev.net/topic/649322-getting-rid-of-angelscript-prints/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		CBufferedOutStream bout;
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		RegisterStdString(engine);

		asITypeInfo *t = engine->GetTypeInfoByName("wrong decl");
		if( t != 0 ) TEST_FAILED;
		int i = engine->GetTypeIdByDecl("wrong decl");
		if( i >= 0 ) TEST_FAILED;
		asIScriptFunction *func = engine->GetGlobalFunctionByDecl("wrong decl");
		if( func != 0 ) TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test FindNextLineWithCode
	// Reported by Scott Bean
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		asIScriptModule *mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"int Function() \n" // 1
			"{ \n"              // 2
			"    // comment \n" // 3
			"    return 1; \n"  // 4
			"} \n");            // 5
		int r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("Function");
		int line = func->FindNextLineWithCode(0);
		if( line != -1 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(1);
		if( line != 4 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(2);
		if( line != 4 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(3);
		if( line != 4 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(4);
		if( line != 4 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(5);
		if( line != -1 )
			TEST_FAILED;
		
		engine->Release();
	}

	// Test FindNextLineWithCode for class destructors
	// Reported by Scott Bean
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		RegisterStdString(engine);

		asIScriptModule *mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"class ssNode \n"
			"{ \n"
			"    ssNode()  \n"
			"    { \n"
			"        n = 42; \n"
			"    } \n"
			"    int n; \n"
			"    string str; \n"
			"} \n");
		int r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asITypeInfo *type = mod->GetTypeInfoByName("ssNode");
		asEBehaviours behave;
		asUINT behaveCount = type->GetBehaviourCount();
		if( behaveCount != 9 )
			TEST_FAILED;
		asIScriptFunction *func = type->GetBehaviourByIndex(8, &behave);
		if( behave != asBEHAVE_CONSTRUCT )
			TEST_FAILED;
		int line = func->FindNextLineWithCode(5);
		if( line != 5 )
			TEST_FAILED;
		line = func->FindNextLineWithCode(8);
		if( line != 8 )
			TEST_FAILED;

		engine->Release();
	}

    // Test crash in GetLineNumber
    // http://www.gamedev.net/topic/638656-crash-in-ctx-getlinenumber/
	SKIP_ON_MAX_PORT
    {
        asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        engine->RegisterInterface("foo");
        engine->RegisterObjectType("Test", 0, asOBJ_REF | asOBJ_NOCOUNT);
        engine->RegisterObjectMethod("Test", "foo @TestLineNumber()", asMETHOD(TestLN, TestLineNumber), asCALL_THISCALL);
 
        TestLN t;
        engine->RegisterGlobalProperty("Test test", &t);
 
        asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
        mod->AddScriptSection("b"," // nothing to compile");
        mod->AddScriptSection("a","foo @f = test.TestLineNumber();");
        int r = mod->Build();
        if( r < 0 )
            TEST_FAILED;

		CBytecodeStream stream("test");
		mod->SaveByteCode(&stream);

        engine->Release();

		// Load the bytecode
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        engine->RegisterInterface("foo");
        engine->RegisterObjectType("Test", 0, asOBJ_REF | asOBJ_NOCOUNT);
        engine->RegisterObjectMethod("Test", "foo @TestLineNumber()", asMETHOD(TestLN, TestLineNumber), asCALL_THISCALL);
        engine->RegisterGlobalProperty("Test test", &t);

        mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

        engine->Release();
    }

	int number = 0;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	RegisterScriptString_Generic(engine);

	// Test GetTypeIdByDecl
	if( engine->GetTypeIdByDecl("int") != engine->GetTypeIdByDecl("const int") )
		TEST_FAILED;
	if( engine->GetTypeIdByDecl("string") != engine->GetTypeIdByDecl("const string") )
		TEST_FAILED;

	// A handle to a const is different from a handle to a non-const
	if( engine->GetTypeIdByDecl("string@") == engine->GetTypeIdByDecl("const string@") )
		TEST_FAILED;

	// Test debugging
	engine->RegisterGlobalProperty("int number", &number);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule("Module1", asGM_ALWAYS_CREATE);
	mod->AddScriptSection(":1", script1, strlen(script1), 0);
	mod->Build();

	mod = engine->GetModule("Module2", asGM_ALWAYS_CREATE);
	mod->AddScriptSection(":2", script2, strlen(script2), 0);
	mod->Build();

	// Bind all functions that the module imports
	mod = engine->GetModule("Module1");
	mod->BindAllImportedFunctions();

	asIScriptContext *ctx =	engine->CreateContext();
	ctx->SetLineCallback(asFUNCTION(LineCallback), 0, asCALL_CDECL);
	ctx->SetExceptionCallback(asFUNCTION(ExceptionCallback), 0, asCALL_CDECL);
	ctx->Prepare(mod->GetFunctionByDecl("void main()"));
	int r = ctx->Execute();
	if( r == asEXECUTION_EXCEPTION )
	{
		// It is possible to examine the callstack even after the Execute() method has returned
		ExceptionCallback(ctx, 0);
	}
	ctx->Release();
	engine->Release();

	if( printBuffer != correct )
	{
		TEST_FAILED;
		PRINTF("%s", printBuffer.c_str());
	}

	// Success
	return fail;
}

//----------------------------------------------

// In this test we'll use the debug functions to update a script parameter directly on the stack

void DebugCall()
{
	asIScriptContext *ctx = asGetActiveContext();

	// Get the address of the output parameter
	void *varPointer = ctx->GetAddressOfVar(0, 0);

	// We got the address to the handle
	CScriptString **str = (CScriptString**)varPointer;

	// Set the handle to point to a new string
	*str = new CScriptString("test");

	// It is possible to determine the function that is being called too
	asIScriptFunction *func = ctx->GetSystemFunction();
	assert( std::string(func->GetDeclaration()) == "void debugCall()" );
}

bool Test2()
{
	RET_ON_MAX_PORT

	bool fail = false;
	COutStream out;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	RegisterScriptString(engine);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	engine->RegisterGlobalFunction("void debugCall()", asFUNCTION(DebugCall), asCALL_CDECL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	const char *script = 
		"void func(string@ &out output) \n"
		"{ \n"
		"  debugCall(); \n"
		"  assert( output == 'test' ); \n"
		"} \n";

	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "string @o; func(o); assert( o == 'test' );", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}

} // namespace

