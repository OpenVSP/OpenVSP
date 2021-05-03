#include "utils.h"

namespace TestShared
{

bool Test()
{
	bool fail = false;
	CBufferedOutStream bout;
	asIScriptEngine* engine;
	int r;

	// Test discarding a shared class with default factory
	// https://www.gamedev.net/forums/topic/709671-shared-class-invalidates-when-1-of-the-modules-is-discarded/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule* mod1 = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"shared class Foo\n"
			"{\n"
			"	void MethodInFoo(int b) { bar = b; }\n"
			"	int bar;\n"
			"}\n"
			"void Main()\n"
			"{\n"
			"	Foo@ f = Foo(); \n"
			"}\n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule* mod2 = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"shared class Foo\n"
			"{\n"
			"	void MethodInFoo(int b) { bar = b; }\n"
			"	int bar;\n"
			"}\n"
			"void Main()\n"
			"{\n"
			"}\n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		mod1->Discard();
		mod1 = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"shared class Foo\n"
			"{\n"
			"	void MethodInFoo(int b) { bar = b; }\n"
			"	int bar;\n"
			"}\n"
			"void Main()\n"
			"{\n"
			"	Foo@ f = Foo(); \n"
			"}\n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test using shared class after the original module has been discarded
	// https://www.gamedev.net/forums/topic/709671-shared-class-invalidates-when-1-of-the-modules-is-discarded/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule* mod1 = engine->GetModule("mod1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"shared class Foo\n"
			"{\n"
			"	void MethodInFoo(int b) { bar = b; }\n"
			"	int bar;\n"
			"}\n"
			"void Main()\n"
			"{\n"
			"	Foo@ f = Foo(); \n"
			"}\n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule* mod2 = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"shared class Foo\n"
			"{\n"
			"	void MethodInFoo(int b) { bar = b; }\n"
			"	int bar;\n"
			"}\n"
			"void Main()\n"
			"{\n"
			"	while (true) {\n"
			"		Foo@ f = Foo();\n"
			"		break;\n"
			"	}\n"
			"}\n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		mod1->Discard();

		asIScriptContext* ctx = engine->CreateContext();
		ctx->Prepare(mod2->GetFunctionByName("Main"));
		r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		ctx->Release();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test anonymous functions within shared functions
	// Reported by Phong Ba
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		asIScriptModule* shareMod = NULL;

		{ //Module 1 (Build this module as shared module)
			shareMod = engine->GetModule("shared", asGM_ALWAYS_CREATE); assert(shareMod != NULL);

			r = shareMod->AddScriptSection("main",
				"shared funcdef void SimpleCallback(); \n"

				"//shared void Simple() {} \n"

				"shared void InvokeSimple() { \n"
				"	//SimpleCallback@ cb = Simple; \n" //No error
				"	SimpleCallback@ cb = function() {}; \n" //Error caused by the anonymous function

				"	cb(); \n"
				"} \n"); assert(r >= 0);
			r = shareMod->Build();
			if (r < 0)
				TEST_FAILED;
		}

		{ //Module 2 (Build this module and discard) <== No error
			asIScriptModule* mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->AddScriptSection("main",
				"external shared void InvokeSimple(); \n"

				"void main() \n"
				"{ \n"
				"	InvokeSimple(); \n"
				"} \n"); assert(r >= 0);
			r = mod->Build();
			if (r < 0)
				TEST_FAILED;

			mod->Discard();
		}

		{ //Module 3 (Build this module and execute the main function) <== Error here
			asIScriptModule* mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);
			asIScriptContext* ctx = engine->CreateContext(); assert(ctx != NULL);

			r = mod->AddScriptSection("main",
				"external shared void InvokeSimple(); \n"

				"void main() \n"
				"{ \n"
				"	InvokeSimple(); \n"
				"} \n"); assert(r >= 0);
			r = mod->Build();
			if (r < 0)
				TEST_FAILED;

			r = ctx->Prepare(engine->GetModule(0)->GetFunctionByDecl("void main()")); assert(r >= 0);
			r = ctx->Execute();
			if (r != asEXECUTION_FINISHED)
				TEST_FAILED;

			ctx->Release();
			mod->Discard();
		}

		shareMod->Discard();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test with external shared interface and inhering from shared interface
	// https://www.gamedev.net/forums/topic/707753-bug-when-importing-an-external-interface/5430255/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule* mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"shared interface ITest {} \n"
			"shared interface ITest2 : ITest {} \n"
			"external shared interface ITest2; \n");
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (3, 27) : Error   : Name conflict. 'ITest2' is an interface.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test inheriting from shared base class, and then discard the module that compiled the base class
	// https://www.gamedev.net/forums/topic/706321-angelscript-crash-using-disposed-script-function-that-was-loaded-from-bytecode/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

const char* file1 = "					\
	class Test1 : Test0 {						\
		int function1() { return Test0::function1(); }	\
	}									\
	shared class Test0 {				\
		int function1() { return 1; }	\
	}									\
	";

const char* file2 = "					\
	class Test2 : Test0 {				\
	}									\
	shared class Test0 {				\
		int function1() { return 1; }	\
	}									\
	";
	
		asIScriptModule* mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		r = mod1->AddScriptSection("test1", file1, strlen(file1));
		r = mod1->Build();
		if( r < 0 )
			TEST_FAILED;
	
		asIScriptModule* mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->AddScriptSection("test2", file2, strlen(file2));
		r = mod2->Build();
		if( r < 0 )
			TEST_FAILED;

		mod1->Discard();
		engine->GarbageCollect();

		auto testTypeInfo = mod2->GetTypeInfoByName("Test2");
		auto test1Func = testTypeInfo->GetMethodByDecl("int function1()");
		auto factoryFunc = testTypeInfo->GetFactoryByIndex(0);

		engine->GarbageCollect();

		auto context = engine->RequestContext();
		context->Prepare(factoryFunc);
		context->Execute();
		asIScriptObject* scriptObj = (asIScriptObject*)context->GetReturnObject();
		scriptObj->AddRef();

		context->Prepare(test1Func);
		context->SetObject(scriptObj);
		int funcCall = context->Execute();
		if( funcCall != asEXECUTION_FINISHED )
			TEST_FAILED;

		scriptObj->Release();
		engine->ReturnContext(context);

		mod2->Discard();
		engine->GarbageCollect();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}
	
	// Test multiple modules with shared objects and inheritance
	// https://www.gamedev.net/forums/topic/706321-angelscript-crash-using-disposed-script-function-that-was-loaded-from-bytecode/5424084/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		CBytecodeStream stream1("1");
		CBytecodeStream stream2("2");
		CBytecodeStream stream3("3");
		CBytecodeStream stream4("4");

			//file for loading in some functions and disposing them to make room
			//in the front of the script engine's function array
			const char* file4 = " \
			shared class Test4 { \
				int function1() { return 0; }	\
				int function2() { return 0; }	\
				int function3() { return 0; }	\
				int function4() { return 0; }	\
				int function5() { return 0; }	\
				int function6() { return 0; }	\
				int function7() { return 0; }	\
				int function8() { return 0; }	\
				int function9() { return 0; }	\
				int function10() { return 0; }	\
			} \
			";
			asIScriptModule* mod4 = engine->GetModule("test4", asGM_ALWAYS_CREATE);
			r = mod4->AddScriptSection("test4", file4, strlen(file4));
			assert(r >= 0);
			r = mod4->Build();
			assert(r >= 0);

			r = mod4->SaveByteCode(&stream4);
			assert(r >= 0);

			mod4->Discard();
			engine->GarbageCollect();

			//file for the first derived class
			const char* file2 = " \
				shared class Test2 : Test0 { \
				} \
				shared class Test0 { \
					int function1() { return 0; } \
				} \
				";
		
			asIScriptModule* mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
			r = mod2->AddScriptSection("test2", file2, strlen(file2));
			assert(r >= 0);
			r = mod2->Build();
			assert(r >= 0);

			r = mod2->SaveByteCode(&stream2);
			assert(r >= 0);

			mod2->Discard();
			engine->GarbageCollect();

			//file for the base class
			const char* file1 = " \
			shared class Test0 { \
			int function1() { return 0; } \
			} \
			";
			
			asIScriptModule* mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
			r = mod1->AddScriptSection("test1", file1, strlen(file1));
			assert(r >= 0);
			r = mod1->Build();
			assert(r >= 0);
			r = mod1->SaveByteCode(&stream1);
			assert(r >= 0);

			//! module1 is not discarded before saving bytecode for module3 to produce a different bytecode !

			//file for the second derived class
			const char* file3 = " \
			shared class Test3 : Test0 { \
				int function1() { Test0::function1(); return 1; } \
			} \
			shared class Test0 { \
				int function1() { return 0; } \
			} \
			";
			
			asIScriptModule* mod3 = engine->GetModule("test3", asGM_ALWAYS_CREATE);
			r = mod3->AddScriptSection("test3", file3, strlen(file3));
			assert(r >= 0);
			r = mod3->Build(); // 4 m_scriptFunctions, 2 m_classTypes (int Test3::function1(), constructor Test3::Test3(), factory Test3 @Test3(), virtual Test0::function1())
			assert(r >= 0);

			r = mod3->SaveByteCode(&stream3);
			assert(r >= 0);


			mod1->Discard();
			mod3->Discard();
			engine->GarbageCollect();
		

		//load some functions to reserve thr front spaces of the script engine function array
		mod4 = engine->GetModule("test4", asGM_ALWAYS_CREATE);
		r = mod4->LoadByteCode(&stream4);
		assert(r == 0);

		//load a module with the base script class that should be the owning module of the shared base class functions
		mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		r = mod1->LoadByteCode(&stream1);
		assert(r == 0);

		//make the front of the script engine function array available for use
		mod4->Discard();
		engine->GarbageCollect();

		//load a derived class module with the base class definition being included afterthe derived class (important!)
		//this will force new base class shared function creation
		mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		//mod2->AddScriptSection("test2", file2, strlen(file2));
		//r = mod2->Build();
		r = mod2->LoadByteCode(&stream2);
		assert( r >= 0 );

		//load a derived class module that was compiled while another module that owned the base class existed (important!)
		//this will result in slightly different bytecode that will do a function search in the script engine's function array
		//(rather than the module's function array) and locate the new duplicate base class shared function introduced by module2
		mod3 = engine->GetModule("test3", asGM_ALWAYS_CREATE);
		r = mod3->LoadByteCode(&stream3);
		assert(r == 0);

		//get rid of module2. This will dispose the duplicate shared function because a new owner could not be located
		mod2->Discard();
		engine->GarbageCollect();

		//try executing a function in module3 that calls a base class virtual function.
		//this will crash doe to trying to use the function pointer from module2 rather than module1
		auto test1TypeInfo = mod3->GetTypeInfoByName("Test3");
		auto test1Func = test1TypeInfo->GetMethodByDecl("int function1()");
		auto factoryFunc = test1TypeInfo->GetFactoryByIndex(0);
		
		auto context = engine->RequestContext();
		context->Prepare(factoryFunc);
		context->Execute();

		asIScriptObject* scriptObj = (asIScriptObject*)context->GetReturnObject();
		scriptObj->AddRef();
		context->Prepare(test1Func);
		context->SetObject(scriptObj);
		context->Execute();   // <-- crashes here when calling function1 which is a shared function with id 37. 
		                      // It should have objType set to Test0 as well as scriptData, but does not

		scriptObj->Release();
		engine->ReturnContext(context);


		mod3->Discard();
		engine->GarbageCollect();




		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared interface with inheritance
	// https://www.gamedev.net/forums/topic/700203-asccontextcallscriptfunction-called-with-null/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("Module1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Script1", 
			"shared interface Interface1 \n"
			"{ \n"
			"    void test(); \n"
			"} \n"
			"shared interface Interface2 : Interface1 \n"
			"{ \n"
			"} \n"
			"class Object : Interface2 \n"
			"{ \n"
			"    void test() \n"
			"    { \n"
			"    } \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"    Object@ object = Object(); \n"
			"    object.test(); \n"
			"} \n");
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		mod = engine->GetModule("Module2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Script2",
			"external shared interface Interface2; \n");
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();		
	}
	
	// Test memory clean up
	// https://www.gamedev.net/forums/topic/696396-leak-occurs-when-shared-class-exists/
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("Module1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Script1", 
			"shared class X { } \n"
			"X@ x = X();        \n");
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		mod = engine->GetModule("Module2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("Script2",
			"shared class X { };  \n"
			"void test() {}       \n");
		r = mod->Build();
		if (r < 0) TEST_FAILED;

		int currAlloc = GetAllocedMem();

		// discard `Module2`
		mod->Discard();

		if (currAlloc <= GetAllocedMem())
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// external shared entities should be saved specifically as external in bytecode to avoid increase in file size
	// TODO: as_restore should only populate externalTypes and externalFunctions if not def AS_NO_COMPILER
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod1 = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"shared funcdef void A(); \n"
			"shared void B() {} \n"
			"shared interface C { void foo(); } \n"
			"shared enum D { a, b, c } \n"
			"shared class E : C { void foo() {} } \n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test2",
			"external shared funcdef void A(); \n"
			"external shared void B(); \n"
			"external shared interface C; \n"
			"external shared enum D; \n"
			"external shared class E; \n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1(__FILE__"1");
		CBytecodeStream bc2(__FILE__"2");
		r = mod1->SaveByteCode(&bc1); assert(r >= 0);
		r = mod2->SaveByteCode(&bc2); assert(r >= 0);

		if (bc1.buffer.size() <= bc2.buffer.size())
			TEST_FAILED;

		asDWORD crc32 = ComputeCRC32(&bc1.buffer[0], asUINT(bc1.buffer.size()));
		if (crc32 != 0xA617CEE5)
		{
			PRINTF("The saved byte code has different checksum than the expected. Got 0x%X\n", crc32);
			TEST_FAILED;
		}

		crc32 = ComputeCRC32(&bc2.buffer[0], asUINT(bc2.buffer.size()));
		if (crc32 != 0xC1DD769E)
		{
			PRINTF("The saved byte code has different checksum than the expected. Got 0x%X\n", crc32);
			TEST_FAILED;
		}

		asIScriptModule *mod3 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod3->LoadByteCode(&bc2);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared funcdef
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void A(); \n" // ok
			"shared funcdef void B(); \n" // ok
			"external shared funcdef void C(); \n" // builder should report error due to not finding C
			"shared external funcdef void D(); \n" // builder should report error due to not finding D
			);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (3, 1) : Error   : External shared entity 'C' not found\n"
						   "test (4, 1) : Error   : External shared entity 'D' not found\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"shared funcdef void TEST();");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"external shared funcdef void TEST(); \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared function
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void A(); \n" // looks like a variable declaration
			"shared void B(); \n" // missing definition
			"external shared void C(); \n" // builder should report error due to not finding C
			"shared external void D() {  }; \n" // builder should report error due to not finding C. Cannot redefine original 
			);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 1) : Error   : Data type can't be 'void'\n"
						   "test (2, 1) : Error   : Missing definition of 'B'\n"
						   "test (3, 1) : Error   : External shared entity 'C' not found\n"
						   "test (4, 1) : Error   : External shared entity 'D' not found\n"
						   "test (4, 1) : Error   : External shared entity 'D' cannot redefine the original entity\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"shared void TEST() {  }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"external shared void TEST(); \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared class
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A; \n" // missing definition
			"shared class B; \n" // missing definition
			"external shared class C; \n" // builder should report error due to not finding C
			"shared external class D { void func() {} }; \n" // builder should report error due to not finding C. Cannot redefine original 
			);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 7) : Error   : Missing definition of 'A'\n"
			"test (2, 14) : Error   : Missing definition of 'B'\n"
			"test (3, 23) : Error   : External shared entity 'C' not found\n"
			"test (4, 23) : Error   : External shared entity 'D' cannot redefine the original entity\n"
			"test (4, 23) : Error   : External shared entity 'D' not found\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"shared class TEST { void func() {} }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"external shared class TEST; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared interface
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"interface A; \n" // missing definition
			"shared interface B; \n" // missing definition
			"external shared interface C; \n" // builder should report error due to not finding C
			"shared external interface D { int func(); }; \n" // builder should report error due to not finding C. Cannot redefine original 
			);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 11) : Error   : Missing definition of 'A'\n"
						   "test (2, 18) : Error   : Missing definition of 'B'\n"
						   "test (3, 27) : Error   : External shared entity 'C' not found\n"
						   "test (4, 27) : Error   : External shared entity 'D' cannot redefine the original entity\n"
						   "test (4, 27) : Error   : External shared entity 'D' not found\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"shared interface TEST { int func(); }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"external shared interface TEST; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared enums
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"enum TEST1; \n" // missing definition
			"shared enum TEST2; \n" // missing definition
			"external shared enum TEST3; \n" // builder should report error due to not finding TEST3
			"shared external enum TEST4 { val }; \n" // builder should report error due to not finding TEST4. Cannot redefine original 
			);
		r = mod->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (1, 6) : Error   : Missing definition of 'TEST1'\n"
						   "test (2, 13) : Error   : Missing definition of 'TEST2'\n"
						   "test (3, 22) : Error   : External shared entity 'TEST3' not found\n"
						   "test (4, 22) : Error   : External shared entity 'TEST4' not found\n"
						   "test (4, 22) : Error   : External shared entity 'TEST4' cannot redefine the original entity\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		bout.buffer = "";
		mod->AddScriptSection("test",
			"shared enum TEST { val = 1 }");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"external shared enum TEST; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test declaring a shared funcdef that use a non-shared type
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test",
			"class A {} \n"
			"shared funcdef void F(A@); \n");
		r = mod1->Build();
		if (r >= 0)
			TEST_FAILED;

		if (bout.buffer != "test (2, 1) : Error   : Shared code cannot use non-shared type 'A'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test using a registered funcdef in a shared function
	// https://www.gamedev.net/topic/685120-application-registered-funcdefs-arent-shared/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->RegisterFuncdef("void voidFunc()");

		asIScriptModule *mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test", 
			"shared void call(voidFunc@ f) { \n"
			"  f(); \n"
			"} \n");
		r = mod1->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test memory management with shared functions calling shared functions
	// http://www.gamedev.net/topic/638334-assertion-failed-on-exit-with-shared-func/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		const char *script = 
			"shared void func1() { func2(); }\n"
			"shared void func2() { func1(); }\n";

		const char *script2 =
			"shared void func3() { func1(); } \n";

		asIScriptModule *mod1 = engine->GetModule("test1", asGM_ALWAYS_CREATE);
		mod1->AddScriptSection("test", script);
		r = mod1->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod1->GetFunctionByName("func1");

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test", script);
		mod2->AddScriptSection("test2", script2);
		r = mod2->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func2 = mod2->GetFunctionByName("func1");
		if( func != func2 )
			TEST_FAILED;

		engine->DiscardModule("test1");

		engine->Release();
	}

	// Test funcdefs in shared interfaces
	// http://www.gamedev.net/topic/639243-funcdef-inside-shared-interface-interface-already-implement-warning/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", 
			"funcdef void Func();\n"
			"shared interface ielement\n"
			"{\n"
			"    Func@ f { get; set; }\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("B", 
			"funcdef void Func();\n"
			"shared interface ielement\n"
			"{\n"
			"    Func@ f { get; set; }\n"
			"}\n"
			"class celement : ielement\n"
			"{\n"
			"    Func@ fdef;\n"
			"    Func@ get_f()\n"
			"    {\n"
			"       return( this.fdef ); \n"
			"    }\n"
			"    void set_f( Func@  newF )\n"
			"    {\n"
			"       @this.fdef = newF;\n"
			"    }\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		CBytecodeStream stream("B");
		mod->SaveByteCode(&stream);

		bout.buffer = "";
		mod = engine->GetModule("C", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r != 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test different types of virtual properties in shared interfaces
	// http://www.gamedev.net/topic/639243-funcdef-inside-shared-interface-interface-already-implement-warning/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", 
			"shared interface ielement\n"
			"{\n"
			"    float f { get; set; }\n"
			"}\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("B", 
			"shared interface ielement\n"
			"{\n"
			"    int f { get; set; }\n"
			"}\n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "B (3, 13) : Error   : Shared type 'ielement' doesn't match the original declaration in other module\n"
		                   "B (3, 18) : Error   : Shared type 'ielement' doesn't match the original declaration in other module\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test problem reported by Andrew Ackermann
	{
		const char *script =
			"shared interface I {} \n"
			"shared class C : I {} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Make sure the shared type didn't get the interface duplicated
		asITypeInfo *type = mod->GetTypeInfoByName("C");
		if( type->GetInterfaceCount() != 1 )
			TEST_FAILED;

		engine->Release();
	}

	// Compiling a script with a shared class that refers to other non declared entities must give
	// error even if the shared class is already existing in a previous module
	// http://www.gamedev.net/topic/632922-huge-problems-with-precompilde-byte-code/
	{
		const char *script1 = 
			"shared class A { \n"
			"  B @b; \n"
			"  void setB(B@) {} \n"
			"  void func() {B@ l;} \n" // TODO: The method isn't compiled so this error isn't seen. Should it be?
			"  string c; \n"
			"} \n";

		const char *script2 =
			"shared class B {} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterStdString(engine);
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", script1);
		mod->AddScriptSection("B", script2);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("A", script1);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != "A (3, 13) : Error   : Identifier 'B' is not a data type in global namespace\n"
		                   "A (3, 3) : Error   : Shared type 'A' doesn't match the original declaration in other module\n"
		                 /*  "A (2, 3) : Error   : Identifier 'B' is not a data type in global namespace\n"
		                   "A (2, 6) : Error   : Shared type 'A' doesn't match the original declaration in other module\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		int reg;
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		engine->RegisterGlobalProperty("int reg", &reg);

		RegisterScriptArray(engine, true);

		engine->RegisterEnum("ESHARED");
		engine->RegisterEnumValue("ESHARED", "ES1", 1);

		asIScriptModule *mod = engine->GetModule("", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("a", 
			"interface badIntf {} \n"
			"shared interface sintf {} \n"
			"shared class T : sintf, badIntf \n" // Allow shared interface, but not non-shared interface
			"{ \n"
			"  void test() \n"
			"  { \n"
			"    var = 0; \n" // Don't allow accessing non-shared global variables
			"    gfunc(); \n" // Don't allow calling non-shared global functions
			"    reg = 1; \n" // Allow accessing registered variables
			"    assert( reg == 1 ); \n" // Allow calling registered functions
			"    badIntf @intf; \n" // Do not allow use of non-shared types in parameters/return type
			"    int cnst = g_cnst; \n" // Allow using literal consts, even if they are declared in global scope
			"    ESHARED es = ES1; \n" // Allow
			"    ENOTSHARED ens = ENS1; \n" // Do not allow. The actual value ENS1 is allowed though, as it is a literal constant
			"    cast<badIntf>(null); \n" // do not allow casting to non-shared types
			"    assert !is null; \n" // Allow taking address of registered functions
			"    gfunc !is null; \n" // Do not allow taking address of non-shared script functions
			"    nonShared(); \n" // Do not allow constructing objects of non-shared type
			"    impfunc(); \n" // Do not allow calling imported function
			"    sfunc(); \n" // Allow calling a shared global function
			"  } \n"
			"  T @dup() const \n" // It must be possible for the shared class to use its own type
			"  { \n"
			"    T d; \n" // Calling the global factory as a shared function
			"    return d; \n" 
			"  } \n"
			"  T() {} \n"
			"  T(int a) \n"
			"  { \n"
			"     var = a; \n" // Constructor of shared class must not access non-shared code
			"  } \n"
			"  void f(badIntf @) {} \n" // Don't allow use of non-shared types in parameters/return type
			"  ESHARED _es; \n" // allow
			"  ESHARED2 _es2; \n" // allow
			"  ENOTSHARED _ens; \n" // Don't allow
			"  void f() \n"
			"  { \n"
			"    array<int> a; \n"
			"  } \n"
			"} \n"
			"int var; \n"
			"void gfunc() {} \n"
			"shared void sfunc() \n"
			"{ \n"
			"  gfunc(); \n" // don't allow
			"  T t; \n" // allow
			"  ESHARED2 s; \n" // allow
			"} \n"
			"enum ENOTSHARED { ENS1 = 1 } \n"
			"shared enum ESHARED2 { ES21 = 0 } \n"
			"const int g_cnst = 42; \n"
			"class nonShared {} \n"
			"import void impfunc() from 'mod'; \n"
			);
		bout.buffer = "";
		r = mod->Build();
		if( r >= 0 ) 
			TEST_FAILED;
		if( bout.buffer != "a (3, 25) : Error   : Shared type cannot implement non-shared interface 'badIntf'\n"
						   "a (32, 3) : Error   : Shared code cannot use non-shared type 'badIntf'\n"
					/*	   "a (35, 3) : Error   : Shared code cannot use non-shared type 'ENOTSHARED'\n"
						   "a (43, 1) : Info    : Compiling void sfunc()\n"
						   "a (45, 3) : Error   : Shared code cannot call non-shared function 'void gfunc()'\n"
						   "a (5, 3) : Info    : Compiling void T::test()\n"
						   "a (7, 5) : Error   : Shared code cannot access non-shared global variable 'var'\n"
						   "a (8, 5) : Error   : Shared code cannot call non-shared function 'void gfunc()'\n"
						   "a (11, 5) : Error   : Shared code cannot use non-shared type 'badIntf'\n"
						   "a (14, 5) : Error   : Shared code cannot use non-shared type 'ENOTSHARED'\n"
						   "a (15, 5) : Error   : Shared code cannot use non-shared type 'badIntf'\n"
						   "a (17, 11) : Error   : Shared code cannot call non-shared function 'void gfunc()'\n"
						   "a (18, 5) : Error   : Shared code cannot use non-shared type 'nonShared'\n"
						   "a (19, 5) : Error   : Shared code cannot call non-shared function 'void impfunc()'\n"
						   "a (28, 3) : Info    : Compiling T::T(int)\n"
						   "a (30, 6) : Error   : Shared code cannot access non-shared global variable 'var'\n" */)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->DiscardModule("");
	
		const char *validCode =
			"shared interface I {} \n"
			"shared class T : I \n"
			"{ \n"
			"  void func() {} \n"
			"  int i; \n"
			"} \n"
			"shared void func() {} \n"
			"shared enum eshare { e1, e2 } \n";
		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("a", validCode);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		int t1 = mod->GetTypeIdByDecl("T");
		if( t1 < 0 )
			TEST_FAILED;

		asIScriptFunction *f1 = mod->GetFunctionByDecl("void func()");
		if( f1 == 0 )
			TEST_FAILED;

		if( t1 >= 0 )
		{
			asIScriptFunction *fact1 = engine->GetTypeInfoById(t1)->GetFactoryByIndex(0);
			if( fact1 < 0 )
				TEST_FAILED;
		
			asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
			mod2->AddScriptSection("b", validCode);
			r = mod2->Build();
			if( r < 0 )
				TEST_FAILED;

			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}

			int t2 = mod2->GetTypeIdByDecl("T");
			if( t1 != t2 )
				TEST_FAILED;

			asIScriptFunction *f2 = mod2->GetFunctionByDecl("void func()");
			if( f1 != f2 )
				TEST_FAILED;

			asIScriptFunction *fact2 = engine->GetTypeInfoById(t2)->GetFactoryByIndex(0);
			if( fact1 != fact2 )
				TEST_FAILED;

			CBytecodeStream stream(__FILE__"1");
			mod->SaveByteCode(&stream);

			bout.buffer = "";
			asIScriptModule *mod3 = engine->GetModule("3", asGM_ALWAYS_CREATE);
			r = mod3->LoadByteCode(&stream);
			if( r < 0 )
				TEST_FAILED;

			int t3 = mod3->GetTypeIdByDecl("T");
			if( t1 != t3 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}

			asIScriptFunction *f3 = mod3->GetFunctionByDecl("void func()");
			if( f1 != f3 )
				TEST_FAILED;

			asIScriptFunction *fact3 = engine->GetTypeInfoById(t3)->GetFactoryByIndex(0);
			if( fact1 != fact3 )
				TEST_FAILED;

			bout.buffer = "";
			r = ExecuteString(engine, "T t; t.func(); func();", mod3);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		engine->Release();
	}

	// Test shared classes as members of other classes
	// http://www.gamedev.net/topic/617717-shared-template-factory-stub/
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		const char *code = 
			"shared class S { int a; } \n"
			"class A { S s; } \n";

		asIScriptModule *mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("code", code);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("code", code);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test reloading scripts with shared code
	// http://www.gamedev.net/topic/618417-problem-with-shared-keyword/
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		const char *code = 
			"shared class B {} \n"
			"shared class iMyInterface { \n"
			"  void MyFunc(const B &in) {} \n"
			"} \n"
			"class cMyClass : iMyInterface { \n"
			"  void MyFunc(const B &in) { \n"
			"  } \n"
			"} \n";

		asIScriptModule *mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("code", code);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		int id = mod->GetTypeIdByDecl("cMyClass");
		asITypeInfo *type = engine->GetTypeInfoById(id);
		asIScriptFunction *func = type->GetMethodByDecl("void MyFunc(const B &in)");
		if( func == 0 )
			TEST_FAILED;

		asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(type);
		asIScriptContext *ctx = engine->CreateContext();

		ctx->Prepare(func);
		ctx->SetObject(obj);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		ctx->Release();
		obj->Release();

		// This will orphan the shared types, but won't destroy them yet
		// as they are still kept alive by the garbage collector
		engine->DiscardModule("1");

		// Build the module again. This will re-use the orphaned 
		// shared classes, the code should still work normally
		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("code", code);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		id = mod->GetTypeIdByDecl("cMyClass");
		type = engine->GetTypeInfoById(id);
		func = type->GetMethodByDecl("void MyFunc(const B &in)");
		if( func == 0 )
			TEST_FAILED;

		obj = (asIScriptObject*)engine->CreateScriptObject(type);
		ctx = engine->CreateContext();

		ctx->Prepare(func);
		ctx->SetObject(obj);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		ctx->Release();
		obj->Release();

		engine->Release();
	}

	// Test problem reported by TheAtom
	// http://www.gamedev.net/topic/622841-shared-issues/
	{
 		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		const char *code = 
			"shared int f() \n"
			"{ \n"
			"  return 0; \n"
			"} \n";

		asIScriptModule *mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("code", code);
		bout.buffer = "";
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}		

		engine->Release();
	}

	// Success
	return fail;
}



} // namespace

