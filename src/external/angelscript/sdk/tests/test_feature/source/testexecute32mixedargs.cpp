//
// Tests calling of a c-function from a script with 32 parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute32Args (mixed arguments)";

static bool testVal = false;
static bool called  = false;

static int ivalues[16];
static float fvalues[16];

static void cfunction(int f1 , int f2 , int f3 , int f4 ,
		float f5 , float f6 , float f7 , float f8 ,
		int f9 , int f10, int f11, int f12,
		float f13, float f14, float f15, float f16,
		int f17, int f18, int f19, int f20,
		float f21, float f22, float f23, float f24,
		int f25, int f26, int f27, int f28,
		float f29, float f30, float f31, float f32)
{
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
	called = true;
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
	

	testVal =	(ivalues[0]  ==  1) && (ivalues[1]  ==  2) && (ivalues[2]  ==  3) && (ivalues[3]  ==  4) &&
			(fvalues[0]  ==  5.0f) && (fvalues[1]  ==  6.0f) && (fvalues[2]  ==  7.0f) && (fvalues[3]  ==  8.0f) &&
			(ivalues[4]  ==  9) && (ivalues[5] == 10) && (ivalues[6] == 11) && (ivalues[7] == 12) &&
			(fvalues[4] == 13.0f) && (fvalues[5] == 14.0f) && (fvalues[6] == 15.0f) && (fvalues[7] == 16.0f) &&
			(ivalues[8] == 17) && (ivalues[9] == 18) && (ivalues[10] == 19) && (ivalues[11] == 20) &&
			(fvalues[8] == 21.0f) && (fvalues[9] == 22.0f) && (fvalues[10] == 23.0f) && (fvalues[11] == 24.0f) &&
			(ivalues[12] == 25) && (ivalues[13] == 26) && (ivalues[14] == 27) && (ivalues[15] == 28) &&
			(fvalues[12] == 29.0f) && (fvalues[13] == 30.0f) && (fvalues[14] == 31.0f) && (fvalues[15] == 32.0f);
}

bool TestExecute32MixedArgs()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		engine->RegisterGlobalFunction(		"void cfunction("
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float"
			")", asFUNCTION(cfunction_gen), asCALL_GENERIC);
	}
	else
	{
		engine->RegisterGlobalFunction(		"void cfunction("
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float,"
				"int, int, int, int,"
				"float, float, float, float"
			")", asFUNCTION(cfunction), asCALL_CDECL);
	}

	ExecuteString(engine, 
		"cfunction("
			" 1,  2,  3,  4,"
			" 5.0f,  6.0f,  7.0f,  8.0f,"
			" 9, 10, 11, 12,"
			"13.0f, 14.0f, 15.0f, 16.0f,"
			"17, 18, 19, 20,"
			"21.0f, 22.0f, 23.0f, 24.0f,"
			"25, 26, 27, 28,"
			"29.0f, 30.0f, 31.0f, 32.0f"
		")");

	if( !called ) 
	{
		// failure
		PRINTF("\n%s: cfunction not called from script\n\n", TESTNAME);
		TEST_FAILED;
	} 
	else if( !testVal ) 
	{
		// failure
		PRINTF("\n%s: testVal is not of expected value. Got:\n\n", TESTNAME);
		int pos = 0;
		for( int i = 0; i < 4; i++ ) 
		{
			int j;
			for( j = 0; j < 4; j++ ) 
				PRINTF("ivalue[%d]: %d\n", pos+j, ivalues[pos+j]);
			for( j = 0; j < 4; j++ ) 
				PRINTF("fvalue[%d]: %f\n", pos+j, fvalues[pos+j]);
			pos += 4;
		}
		TEST_FAILED;
	}

	engine->Release();
	
	// Success
	return fail;
}
