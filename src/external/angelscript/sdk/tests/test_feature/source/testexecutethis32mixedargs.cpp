//
// Tests calling of an object-function from a script
// with 32 parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "this->TestExecute32Args (mixed arguments)";


class TestClass {
public:
	TestClass() : testVal(false), called(false) {}
	bool testVal;
	bool called;

	int ivalues[16];
	float fvalues[16];

	void test() {
	}
	void cfunction(	int f1 , int f2 , int f3 , int f4 ,
			float f5 , float f6 , float f7 , float f8 ,
			int f9 , int f10, int f11, int f12,
			float f13, float f14, float f15, float f16,
			int f17, int f18, int f19, int f20,
			float f21, float f22, float f23, float f24,
			int f25, int f26, int f27, int f28,
			float f29, float f30, float f31, float f32
	) {
		called = true;
		ivalues[ 0] = f1;
		ivalues[ 1] = f2;
		ivalues[ 2] = f3;
		ivalues[ 3] = f4;
		fvalues[ 0] = f5;
		fvalues[ 1] = f6;
		fvalues[ 2] = f7;
		fvalues[ 3] = f8;
		ivalues[ 4] = f9;
		ivalues[ 5] = f10;
		ivalues[ 6] = f11;
		ivalues[ 7] = f12;
		fvalues[ 4] = f13;
		fvalues[ 5] = f14;
		fvalues[ 6] = f15;
		fvalues[ 7] = f16;
		ivalues[ 8] = f17;
		ivalues[ 9] = f18;
		ivalues[10] = f19;
		ivalues[11] = f20;
		fvalues[ 8] = f21;
		fvalues[ 9] = f22;
		fvalues[10] = f23;
		fvalues[11] = f24;
		ivalues[12] = f25;
		ivalues[13] = f26;
		ivalues[14] = f27;
		ivalues[15] = f28;
		fvalues[12] = f29;
		fvalues[13] = f30;
		fvalues[14] = f31;
		fvalues[15] = f32;


		testVal =	(f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
				(f5  ==  5.0f) && (f6  ==  6.0f) && (f7  ==  7.0f) && (f8  ==  8.0f) &&
				(f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
				(f13 == 13.0f) && (f14 == 14.0f) && (f15 == 15.0f) && (f16 == 16.0f) &&
				(f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
				(f21 == 21.0f) && (f22 == 22.0f) && (f23 == 23.0f) && (f24 == 24.0f) &&
				(f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
				(f29 == 29.0f) && (f30 == 30.0f) && (f31 == 31.0f) && (f32 == 32.0f);
	}
	static void cfunction_gen(asIScriptGeneric *gen)
	{
		TestClass *self = (TestClass*)gen->GetObject();
		self->called = true;
		int *ivalues = self->ivalues;
		float *fvalues = self->fvalues;
		ivalues[ 0] = gen->GetArgDWord(0);
		ivalues[ 1] = gen->GetArgDWord(1);
		ivalues[ 2] = gen->GetArgDWord(2);
		ivalues[ 3] = gen->GetArgDWord(3);
		fvalues[ 0] = gen->GetArgFloat(4);
		fvalues[ 1] = gen->GetArgFloat(5);
		fvalues[ 2] = gen->GetArgFloat(6);
		fvalues[ 3] = gen->GetArgFloat(7);
		ivalues[ 4] = gen->GetArgDWord(8);
		ivalues[ 5] = gen->GetArgDWord(9);
		ivalues[ 6] = gen->GetArgDWord(10);
		ivalues[ 7] = gen->GetArgDWord(11);
		fvalues[ 4] = gen->GetArgFloat(12);
		fvalues[ 5] = gen->GetArgFloat(13);
		fvalues[ 6] = gen->GetArgFloat(14);
		fvalues[ 7] = gen->GetArgFloat(15);
		ivalues[ 8] = gen->GetArgDWord(16);
		ivalues[ 9] = gen->GetArgDWord(17);
		ivalues[10] = gen->GetArgDWord(18);
		ivalues[11] = gen->GetArgDWord(19);
		fvalues[ 8] = gen->GetArgFloat(20);
		fvalues[ 9] = gen->GetArgFloat(21);
		fvalues[10] = gen->GetArgFloat(22);
		fvalues[11] = gen->GetArgFloat(23);
		ivalues[12] = gen->GetArgDWord(24);
		ivalues[13] = gen->GetArgDWord(25);
		ivalues[14] = gen->GetArgDWord(26);
		ivalues[15] = gen->GetArgDWord(27);
		fvalues[12] = gen->GetArgFloat(28);
		fvalues[13] = gen->GetArgFloat(29);
		fvalues[14] = gen->GetArgFloat(30);
		fvalues[15] = gen->GetArgFloat(31);


		self->testVal =	(ivalues[0]  ==  1) && (ivalues[1]  ==  2) && (ivalues[2]  ==  3) && (ivalues[3]  ==  4) &&
				(fvalues[0]  ==  5.0f) && (fvalues[1]  ==  6.0f) && (fvalues[2]  ==  7.0f) && (fvalues[3]  ==  8.0f) &&
				(ivalues[4]  ==  9) && (ivalues[5] == 10) && (ivalues[6] == 11) && (ivalues[7] == 12) &&
				(fvalues[4] == 13.0f) && (fvalues[5] == 14.0f) && (fvalues[6] == 15.0f) && (fvalues[7] == 16.0f) &&
				(ivalues[8] == 17) && (ivalues[9] == 18) && (ivalues[10] == 19) && (ivalues[11] == 20) &&
				(fvalues[8] == 21.0f) && (fvalues[9] == 22.0f) && (fvalues[10] == 23.0f) && (fvalues[11] == 24.0f) &&
				(ivalues[12] == 25) && (ivalues[13] == 26) && (ivalues[14] == 27) && (ivalues[15] == 28) &&
				(fvalues[12] == 29.0f) && (fvalues[13] == 30.0f) && (fvalues[14] == 31.0f) && (fvalues[15] == 32.0f);
	}
};

static TestClass test;

static bool testVal = false;
static bool called  = false;

static float  t1 = 0;
static float  t2 = 0;
static double t3 = 0;
static float  t4 = 0;

// This class is implemented to guarantee that it is treated as a complex type on all platforms
class CComplex
{
public:
	// The existance of constructor, copy constructor, destructor, and assignment operator should be enough
	CComplex() { buf = new char[100]; }
	~CComplex() { if( buf ) delete[] buf; }
	CComplex(const CComplex &o) { buf = new char[100]; memcpy(buf, o.buf, 100); }
	CComplex &operator=(const CComplex &o) { if( buf ) memcpy(buf, o.buf, 100); return *this; }

	static void Construct(void *p) {new(p) CComplex();}
	static void CopyConstruct(const CComplex &o, void *p) {new(p) CComplex(o);}
	static void Destruct(CComplex *p) {p->~CComplex();}

	static void Register(asIScriptEngine *engine)
	{
		engine->RegisterObjectType("CComplex", sizeof(CComplex), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
		engine->RegisterObjectBehaviour("CComplex", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Construct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CComplex", asBEHAVE_CONSTRUCT, "void f(const CComplex &in)", asFUNCTION(CopyConstruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CComplex", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("CComplex", "CComplex &opAssign(const CComplex &in)", asMETHOD(CComplex, operator=), asCALL_THISCALL);
	}

private:
	// But if it is not, the size of class should do it
	char *buf;
	double a, b, c, d;
};

class Class1
{
public:
	int a;

	void cfunc(float f1, float f2, double f3, float f4)
	{
		assert(a == 0xDEADC0DE);
		called = true;
		t1 = f1;
		t2 = f2;
		t3 = f3;
		t4 = f4;
		testVal = (f1 == 9.2f) && (f2 == 13.3f) && (f3 == 18.8) && (f4 == 3.1415f);
	}
	void cfunc2(double f1, double f2, double f3, double f4)
	{
		assert(a == 0xDEADC0DE);
		called = true;
		assert(f1 == 1337.0);
		assert(f2 == 1338.0);
		assert(f3 == 1339.0);
		assert(f4 == 1340.0);
	}

	CComplex cfunction5(int f1, double f3, float f2, int f4)
	{
		called = true;

		testVal = (f1 == 10) && (f2 == 1.92f) && (f3 == 3.88) && (f4 == 97);

		return CComplex();
	}
};

static Class1 c1;

bool TestExecuteThis32MixedArgs()
{
	bool fail = false;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	int r;

    r = engine->RegisterObjectType("class1", sizeof(Class1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS | asOBJ_APP_CLASS_ALLINTS); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectMethod("class1", "void cfunction(float, float, double, float)", asMETHOD(Class1, cfunc), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("class1", "void cfunction2(double, double, double, double)", asMETHOD(Class1, cfunc2), asCALL_THISCALL); assert( r >= 0 );
#else
	r = engine->RegisterObjectMethod("class1", "void cfunction(float, float, double, float)", WRAP_MFN(Class1, cfunc), asCALL_GENERIC); assert( r >= 0 );
    r = engine->RegisterObjectMethod("class1", "void cfunction2(double, double, double, double)", WRAP_MFN(Class1, cfunc2), asCALL_GENERIC); assert( r >= 0 );
#endif
    r = engine->RegisterGlobalProperty("class1 c1", &c1); assert( r >= 0 );

#ifndef AS_MAX_PORTABILITY
	CComplex::Register(engine);
	r = engine->RegisterObjectMethod("class1", "CComplex cfunction5(int, double, float, int)", asMETHOD(Class1, cfunction5), asCALL_THISCALL); assert( r >= 0 );
#endif

	c1.a = 0xDEADC0DE;

    called = false;
    r = ExecuteString(engine, "c1.cfunction(9.2f, 13.3f, 18.8, 3.1415f)");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
    if( !called )
    {
        // failure
        PRINTF("\n%s: c1.cfunction1 not called from script\n\n", TESTNAME);
        TEST_FAILED;
    }
    else if( !testVal )
    {
        // failure
        PRINTF("\n%s: testVal is not of expected value. Got (%f, %f, %f, %f), expected (%f, %f, %f, %f)\n\n", TESTNAME, t1, t2, t3, t4, 9.2f, 13.3f, 18.8, 3.1415f);
        TEST_FAILED;
    }
    called = false;
    ExecuteString(engine, "c1.cfunction2(1337.0, 1338.0, 1339.0, 1340.0)");
    if( !called )
    {
        // failure
        PRINTF("\n%s: c1.cfunction2 not called from script\n\n", TESTNAME);
        TEST_FAILED;
    }

#ifndef AS_MAX_PORTABILITY
    called = false;
	testVal = false;
	ExecuteString(engine, "c1.cfunction5(10, 3.88, 1.92f, 97)");
	if( !called )
		TEST_FAILED;
	if( !testVal ) 
		TEST_FAILED;
#endif

	r = engine->RegisterObjectType("TestClass", 0/*sizeof(TestClass)*/, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		r = engine->RegisterObjectMethod(
			"TestClass",
			"void cfunction("
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float"
			")",

			asFUNCTION(TestClass::cfunction_gen)
			, asCALL_GENERIC); assert(r >= 0);
	}
	else
	{
		r = engine->RegisterObjectMethod(
			"TestClass",
			"void cfunction("
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float"
			")",

			asMETHOD(TestClass,cfunction)
			, asCALL_THISCALL); assert(r >= 0);
	}
	r = engine->RegisterGlobalProperty("TestClass test", &test);

	ExecuteString(engine,
		"test.cfunction("
			" 1,  2,  3,  4,"
			" 5.0f,  6.0f,  7.0f,  8.0f,"
			" 9, 10, 11, 12,"
			"13.0f, 14.0f, 15.0f, 16.0f,"
			"17, 18, 19, 20,"
			"21.0f, 22.0f, 23.0f, 24.0f,"
			"25, 26, 27, 28,"
			"29.0f, 30.0f, 31.0f, 32.0f"
		")");

	if (!test.called) {
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} else if (!test.testVal) {
		PRINTF("\n%s: testVal is not of expected value. Got:\n\n", TESTNAME);
		int pos = 0;
		for (int i = 0; i < 4; i++) {
			int j;
			for (j = 0; j < 4; j++) {
				PRINTF("ivalue[%d]: %d\n", pos+j, test.ivalues[pos+j]);
			}
			for (j = 0; j < 4; j++) {
				PRINTF("fvalue[%d]: %f\n", pos+j, test.fvalues[pos+j]);
			}
			pos += 4;
		}
		TEST_FAILED;
	}

	engine->Release();
	engine = NULL;

	return fail;
}
