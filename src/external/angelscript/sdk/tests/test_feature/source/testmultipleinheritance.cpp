//
// This test was designed to test the functionality of methods
// from classes with multiple inheritance
//
// Author: Andreas Jönsson
//

#include "utils.h"

static const char * const TESTNAME = "TestMultipleInheritance";

static std::string output2;

class CBase1
{
public:
	CBase1() {me1 = "CBase1";}
	virtual void CallMe1()
	{
		output2 += me1;
		output2 += ": ";
		output2 += "CBase1::CallMe1()\n";
	}
	const char *me1;
};

class CBase2
{
public:
	CBase2() {me2 = "CBase2";}
	virtual void CallMe2()
	{
		output2 += me2;
		output2 += ": ";
		output2 += "CBase2::CallMe2()\n";
	}
	const char *me2;
};

class CDerivedMultiple : public CBase1, public CBase2
{
public:
	CDerivedMultiple() : CBase1(), CBase2() {}
};


static CDerivedMultiple d;

bool TestMultipleInheritance2();
bool TestMultipleInheritance3();

bool TestMultipleInheritance()
{
	RET_ON_MAX_PORT

	bool fail = TestMultipleInheritance2();
	fail |= TestMultipleInheritance3();
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	// Register the CDerived class
	r = engine->RegisterObjectType("class1", 0, asOBJ_REF | asOBJ_NOHANDLE);
	r = engine->RegisterObjectMethod("class1", "void CallMe1()", asMETHOD(CDerivedMultiple, CallMe1), asCALL_THISCALL);
	r = engine->RegisterObjectMethod("class1", "void CallMe2()", asMETHOD(CDerivedMultiple, CallMe2), asCALL_THISCALL);

	// Register the global CDerived object
	r = engine->RegisterGlobalProperty("class1 d", &d);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	ExecuteString(engine, "d.CallMe1(); d.CallMe2();");

	if( output2 != "CBase1: CBase1::CallMe1()\nCBase2: CBase2::CallMe2()\n" )
	{
		PRINTF("%s: Method calls failed.\n%s", TESTNAME, output2.c_str());
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

//-------------------------------------------------------------------------

// http://www.gamedev.net/community/forums/topic.asp?topic_id=521199
// http://www.gamedev.net/community/forums/topic.asp?topic_id=536299


bool addOverlayCalled = false;

class Drawable {
private:
    int somestuff;
protected:
	int somethingElse;

public:
    Drawable(  ) {}
    virtual ~Drawable() {}

    virtual void addOverlay(  ) { addOverlayCalled = true; }
};

class Creep {
private:
	int somestuffOfMine;
protected:
	int duh;
	int h;
public:
	Creep() { h = 0; }
	virtual ~Creep() {}

	virtual short health() const
	{
//		PRINTF("Creep::health()\n");
		return short(h);
	}
	virtual void health(short /*h*/)
	{
//		PRINTF("Creep::health(%d)\n", h);
	}
};

class CreepClient : public Creep, public Drawable {
private:
protected:
public:
	CreepClient( ) : Creep(), Drawable() { h = 1; }
    virtual ~CreepClient() {}

	short health() const
	{
//		PRINTF("CreepClient::health()\n");
		return short(h);
	}
	void health(short /*h*/)
	{
//		PRINTF("CreepClient::health(%d)\n", h);
	}
};

void Dummy() {}


bool Exec(asIScriptEngine *engine, Creep &c)
{
	bool fail = false;

	CreepClient &cc = dynamic_cast<CreepClient&>(c);

	asIScriptModule *mod = engine->GetModule("mod");
	asIScriptFunction *func = mod->GetFunctionByIndex(0);
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(func);
	ctx->SetArgObject(0, &cc);
	int r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	ctx->Release();

	return fail;

}

bool TestMultipleInheritance2()
{
	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

    r = engine->RegisterObjectType("Creep", sizeof(Creep), asOBJ_REF ); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Creep", asBEHAVE_ADDREF, "void f()", asFUNCTION(Dummy), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Creep", asBEHAVE_RELEASE, "void f()", asFUNCTION(Dummy), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("Creep", "int16 health() const", asMETHODPR(Creep, health, () const, short), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Creep", "void health(int16)", asMETHODPR(Creep, health, (short), void), asCALL_THISCALL); assert( r >= 0 );

    r = engine->RegisterObjectType("CreepClient", sizeof(CreepClient), asOBJ_REF ); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("CreepClient", asBEHAVE_ADDREF, "void f()", asFUNCTION(Dummy), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("CreepClient", asBEHAVE_RELEASE, "void f()", asFUNCTION(Dummy), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("CreepClient", "int16 health() const", asMETHODPR(CreepClient, health, () const, short), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("CreepClient", "void health(int16)", asMETHODPR(CreepClient, health, (short), void), asCALL_THISCALL); assert( r >= 0 );

    r = engine->RegisterObjectMethod("CreepClient", "void addOverlay(  )", asMETHOD(CreepClient, addOverlay), asCALL_THISCALL); assert( r >= 0 );

	const char *script =
		"void fireEffects( CreepClient@ c ) { \n"
		"  if( c.health() != 0 ) \n"
		"  { \n"
		"		c.addOverlay(  ); // This one doesn't ever fire off  \n"
		"  } \n"
		"} \n";

	asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

    CreepClient cc;

	fail = Exec(engine, cc) || fail;

	if( addOverlayCalled == false )
		TEST_FAILED;

	engine->Release();

	return fail;
}

//------------------------------------------------------------

// Problem with registering classes using templated registration code
// Reported by Patrick Jeeves

class iCountable
{
	mutable int refCount;

public:
	iCountable() : refCount(1) { }
	virtual ~iCountable() {}
	iCountable(const iCountable & obj) = delete;
	iCountable(iCountable && obj) = delete;

	iCountable & operator=(const iCountable & /*obj*/) { return *this; };
	iCountable & operator=(iCountable && /*obj*/) { return *this; };

	bool operator==(const iCountable && /*obj*/) { return true; };

	void AddRef() const { ++refCount; }
	void Release() { if (--refCount == 0) delete this; }

	template<class kType>
	static void asRegister(asIScriptEngine * engine, const char * kClass)
	{
		int r;
		r = engine->RegisterObjectBehaviour(kClass, asBEHAVE_ADDREF, "void f()", asMETHODPR(kType, AddRef, () const, void), asCALL_THISCALL); assert( r >= 0);
		r = engine->RegisterObjectBehaviour(kClass, asBEHAVE_RELEASE, "void f()", asMETHODPR(kType, Release, (), void), asCALL_THISCALL); assert( r >= 0);
	}
};

class Bar
{
	int bar;

public:
	Bar() : bar(0) {}
	int get_bar() const { return bar; }
	void set_bar(int v) { bar = v; }

	template<class kType>
	static void asRegister(asIScriptEngine * engine, const char * kClass)
	{
		int r;
		r = engine->RegisterObjectMethod(kClass, "int  get_bar() const property", asMETHODPR(kType, get_bar, () const, int), asCALL_THISCALL); assert( r >= 0);
		r = engine->RegisterObjectMethod(kClass, "void set_bar(int) property", asMETHODPR(kType, set_bar, (int), void), asCALL_THISCALL); assert( r >= 0);
	}
};

class BarObject : public Bar, public iCountable
{
public:
	static BarObject * factory() { return new BarObject(); }

	static void asRegister(asIScriptEngine * engine, const char * kClass)
	{
		iCountable::asRegister<BarObject>(engine, kClass);
		Bar::asRegister<BarObject>(engine, kClass);
	}

	static void asRegisterObject(asIScriptEngine * engine)
	{
		int r;
		r = engine->RegisterObjectType("BarObject", sizeof(BarObject), asOBJ_REF); assert( r >= 0);
		r = engine->RegisterObjectBehaviour("BarObject", asBEHAVE_FACTORY, "BarObject@ f()", asFUNCTION(BarObject::factory), asCALL_CDECL); assert( r >= 0);
		asRegister(engine, "BarObject");
	}
};

bool TestMultipleInheritance3()
{
	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	BarObject::asRegisterObject(engine);

	r = ExecuteString(engine, "BarObject b; b.bar = 42; assert( b.bar == 42 );");
	if (r != asEXECUTION_FINISHED)
		TEST_FAILED;

	engine->Release();

	return fail;
}