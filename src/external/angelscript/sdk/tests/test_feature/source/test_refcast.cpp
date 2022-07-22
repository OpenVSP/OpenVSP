#include "utils.h"


namespace TestRefCast
{


class typeA
{
public:
	int iRef;

	typeA()
	{
		iRef = 1;
	}

	virtual ~typeA()
	{
	}

	void AddRef()
	{
		iRef++;
	}

	void Release()
	{
		if (--iRef == 0)
		{
			delete this;
		}
	}

};


class typeB : public typeA
{
public:
	int a;
	typeB()
	{
		a = 3;
	}
};


typeA *typeA_Factory()
{
    return new typeA();
}

typeB *typeB_Factory()
{
    return new typeB();
}



typeA* B_to_A(typeB* obj)
{
	if( !obj ) return 0;
	typeA* o = dynamic_cast<typeA*>(obj);
	o->AddRef();
	return o;
}


typeB* A_to_B(typeA* obj)
{
	if( !obj ) return 0;
	typeB* o = dynamic_cast<typeB*>(obj);
	o->AddRef();
	return o;
}



void RegisterA(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("typeA", sizeof(typeA), asOBJ_REF);

#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_FACTORY, "typeA@ f()", asFUNCTION(typeA_Factory), asCALL_CDECL);
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_ADDREF, "void f()", asMETHOD(typeA, AddRef), asCALL_THISCALL);
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_RELEASE, "void f()", asMETHOD(typeA, Release), asCALL_THISCALL);
#else
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_FACTORY, "typeA@ f()", WRAP_FN(typeA_Factory), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_ADDREF, "void f()", WRAP_MFN(typeA, AddRef), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("typeA", asBEHAVE_RELEASE, "void f()", WRAP_MFN(typeA, Release), asCALL_GENERIC);
#endif
}


void RegisterB(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("typeB", sizeof(typeB), asOBJ_REF);

#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_FACTORY, "typeB@ f()", asFUNCTION(typeB_Factory), asCALL_CDECL);
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_ADDREF, "void f()", asMETHOD(typeB, AddRef), asCALL_THISCALL);
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_RELEASE, "void f()", asMETHOD(typeB, Release), asCALL_THISCALL);

	r = engine->RegisterObjectMethod("typeB", "typeA@ opCast()", asFUNCTION(B_to_A), asCALL_CDECL_OBJLAST);
	r = engine->RegisterObjectMethod("typeA", "typeB@ opCast()", asFUNCTION(A_to_B), asCALL_CDECL_OBJLAST);
#else
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_FACTORY, "typeB@ f()", WRAP_FN(typeB_Factory), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_ADDREF, "void f()", WRAP_MFN(typeB, AddRef), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("typeB", asBEHAVE_RELEASE, "void f()", WRAP_MFN(typeB, Release), asCALL_GENERIC);

	r = engine->RegisterObjectMethod("typeB", "typeA@ opCast()", WRAP_OBJ_LAST(B_to_A), asCALL_GENERIC);
	r = engine->RegisterObjectMethod("typeA", "typeB@ opCast()", WRAP_OBJ_LAST(A_to_B), asCALL_GENERIC);
#endif
}





static const char* script =
"class CTest\n"
"{\n"
"	typeA@ m_a;\n"
"	typeB@ m_b;\n"

"	CTest()\n"
"	{\n"
"		@m_a = null;\n"
"		@m_b = null;\n"
"	}\n"

"	void dont_work(typeA@ arg)\n"
"	{\n"
"		@m_a = @arg;\n"
"		@m_b = cast<typeB@>(m_a);\n"
"	}\n"

"	void work(typeA@ arg)\n"
"	{\n"
"		typeA@ a = @arg;\n"
"		@m_b = cast<typeB@>(a);\n"
"	}\n"
"};\n";



template<class A, class B>
B* scriptRefCast(A* a)
{
	// If the handle already is a null handle, then just return the null handle
	if (!a) return 0;
	// Now try to dynamically cast the pointer to the wanted type
	B* b = dynamic_cast<B*>(a);
	if (b != 0)
	{
		// Since the cast was made, we need to increase the ref counter for the returned handle
		b->addRef();
	}
	return b;
}

bool Test()
{
	bool fail = false;
	int r = 0;
	COutStream out;
	CBufferedOutStream bout;

	// Test simple ref cast
	{
		asIScriptEngine* engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);


		RegisterA(engine);
		RegisterB(engine);


		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection("test", script, strlen(script));
		r = mod->Build();

		asITypeInfo *objType = engine->GetModule("test")->GetTypeInfoByName("CTest");
		asIScriptObject* testClassObj = (asIScriptObject*)engine->CreateScriptObject(objType);

		typeA* a = new typeB();

		if (testClassObj)
		{
			asIScriptFunction *method = testClassObj->GetObjectType()->GetMethodByName("dont_work");
			asIScriptContext* ctx = engine->CreateContext();

			r = ctx->Prepare(method);
			r = ctx->SetObject(testClassObj);
			r = ctx->SetArgObject(0, a);
			r = ctx->Execute();

			ctx->Release();
			testClassObj->Release();
		}

		typeB* b = 0;
		r = engine->RefCastObject(a, engine->GetTypeInfoByName("typeA"), engine->GetTypeInfoByName("typeB"), (void**)&b);
		if (r != asSUCCESS)
			TEST_FAILED;
		if (b == 0)
			TEST_FAILED;
		if (b->iRef != 2)
			TEST_FAILED;
		b->Release();

		a->Release();

		engine->Release();
	}

	return fail;
}



}
