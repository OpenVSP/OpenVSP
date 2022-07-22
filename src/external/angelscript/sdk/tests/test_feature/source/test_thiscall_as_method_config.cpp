//
// Tests of asCALL_THISCALL_OBJLAST and asCALL_THISCALL_OBJFIRST.
//
// Check if returned value when register with this calling conventions are the expected.
//
// Test author: Jordi Oliveras Rovira
//

#include "utils.h"

namespace TestThisCallMethod_ConfigErrors
{
	static const char * const TESTNAME = "TestThisCallMethod_ConfigErrors";

	class TestType
	{
	};

	// Class to use this methods with new calling convention
	class MethodsClass
	{
		public:
			void TestMethod(void* /*thisPtr*/) {  }
	} test;

	// Value type behaviour auxiliar funtions
	namespace Value
	{
		void Constructor(void *) { }

		void Destructor(void *) { }

		void CopyConstructor(void *, void *) { }

		void ListConstructor(void *, void *) { }
	}

	// Reference type behaviour auxiliar funtions
	namespace Refer
	{
		TestType* Factory() { return NULL; }

		void AddRef(void *) { }

		void Release(void *) { }

		TestType* ListFactory(void *, void *) { return NULL; }
	}

	// Check if the returned value is the expected error value
#define CHECK(expected, call) if (expected != (call)) TEST_FAILED;

	// Check if no error
#define CHECK_OK(call) if ((call) < 0) TEST_FAILED;

	bool Test()
	{
		RET_ON_MAX_PORT

		bool fail = false;
		// COutStream out;

		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		// engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		// Value type behaviours checks
		{
			engine->RegisterObjectType("Val", sizeof(TestType), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CD);
			engine->RegisterObjectType("Val2", sizeof(TestType), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CD);

			// Constructor behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Val", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Val2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// Destructor behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Val", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Val2", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// Copy constructor behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Val", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Val2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// List constructor behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Val", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) { float, float }", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Val2", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) { float, float }", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));
		}

		{
			engine->RegisterObjectType("Ref", 0, asOBJ_REF);
			engine->RegisterObjectType("Ref2", 0, asOBJ_REF);

			// Factory behaviour
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectBehaviour("Ref", asBEHAVE_FACTORY, "Ref@ f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectBehaviour("Ref2", asBEHAVE_FACTORY, "Ref2@ f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// Add ref behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Ref2", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// Release behaviour
			CHECK_OK(engine->RegisterObjectBehaviour("Ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK_OK(engine->RegisterObjectBehaviour("Ref2", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));

			// List factory behaviour
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectBehaviour("Ref", asBEHAVE_LIST_FACTORY, "Ref@ f(int &in) { float, float }",
				asFUNCTION(0), asCALL_THISCALL_OBJFIRST, (void*)1));
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectBehaviour("Ref2", asBEHAVE_LIST_FACTORY, "Ref2@ f(int &in) { float, float }",
				asFUNCTION(0), asCALL_THISCALL_OBJLAST, (void*)1));
		}

		if (strstr(asGetLibraryOptions(), "THISCALL_METHOD_NO_IMPLEMENTED"))
		{
			// Not suport the new calling conventions
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectMethod("Val", "void Method1()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, &test));
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectMethod("Val", "void Method2()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, &test));

			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Val", "void Method3()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, NULL));
			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Val", "void Method4()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, NULL));

			CHECK(asNOT_SUPPORTED, engine->RegisterObjectMethod("Ref", "void Method1()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, &test));
			CHECK(asNOT_SUPPORTED, engine->RegisterObjectMethod("Ref", "void Method2()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, &test));

			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Ref", "void Method3()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, NULL));
			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Ref", "void Method4()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, NULL));
		}
		else
		{
			CHECK_OK(engine->RegisterObjectMethod("Val", "void Method1()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, &test));
			CHECK_OK(engine->RegisterObjectMethod("Val", "void Method2()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, &test));

			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Val", "void Method3()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, NULL));
			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Val", "void Method4()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, NULL));

			CHECK_OK(engine->RegisterObjectMethod("Ref", "void Method1()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, &test));
			CHECK_OK(engine->RegisterObjectMethod("Ref", "void Method2()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, &test));

			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Ref", "void Method3()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJFIRST, NULL));
			CHECK(asINVALID_ARG, engine->RegisterObjectMethod("Ref", "void Method4()", asMETHOD(MethodsClass, TestMethod), asCALL_THISCALL_OBJLAST, NULL));
		}

		engine->Release();

		return fail;
	}
}
