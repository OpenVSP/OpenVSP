//
// This test was designed to test the functionality of virtual methods
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestVirtualMethod";

static std::string output1;

class CBase
{
public:
	CBase() {me = "CBase";}
	virtual void CallMe() 
	{
		output1 += me; 
		output1 += ": "; 
		output1 += "CBase::CallMe()\n";
	}
	const char *me;
};

class CDerived : public CBase
{
public:
	CDerived() : CBase() {me = "CDerived";}
	void CallMe() 
	{
		output1 += me; 
		output1 += ": "; 
		output1 += "CDerived::CallMe()\n";
	}
};

static CBase b;
static CDerived d;

class CRefBase
{
public:
	CRefBase() {refCount = 1;}
	virtual ~CRefBase() {}

	virtual void AddRef() { refCount++; };
	virtual void Release() { if( --refCount == 0 ) delete this; };

	int refCount;
};

class CRefDerived1 : public CBase, public CRefBase
{
public:
	CRefDerived1() : CBase(), CRefBase() {}

	static CRefDerived1* Factory() { return new CRefDerived1(); }
};

bool TestVirtualMethod()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("class1", 0, asOBJ_REF | asOBJ_NOHANDLE);
		r = engine->RegisterObjectMethod("class1", "void CallMe()", asMETHOD(CBase, CallMe), asCALL_THISCALL);
		
		// We must register the property as a pointer to the base class since
		// all registered methods are taken from the base class. This is 
		// especially important when there is multiple or virtual inheritance
		r = engine->RegisterGlobalProperty("class1 b", &b);
		r = engine->RegisterGlobalProperty("class1 d", (CBase*)&d);

		COutStream out;
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		ExecuteString(engine, "b.CallMe(); d.CallMe();");
		
		if( output1 != "CBase: CBase::CallMe()\nCDerived: CDerived::CallMe()\n" )
		{
			PRINTF("%s: Virtual method calls failed.\n%s", TESTNAME, output1.c_str());
			TEST_FAILED;
		}
	
		engine->Release();
	}

	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("ref1", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("ref1", asBEHAVE_FACTORY, "ref1 @f()", asFUNCTION(CRefDerived1::Factory), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("ref1", asBEHAVE_ADDREF, "void f()", asMETHOD(CRefDerived1, AddRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("ref1", asBEHAVE_RELEASE, "void f()", asMETHOD(CRefDerived1, Release), asCALL_THISCALL); assert( r >= 0 );

		r = ExecuteString(engine, "ref1 r;");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	return fail;
}
