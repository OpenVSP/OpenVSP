//
// Tests calling of a c-function from a script with 32 parameters
//
// Test author: Fredrik Ehnbom
//

#include "utils.h"

static const char * const TESTNAME = "TestExecute32Args";

static bool testVal = false;
static bool called  = false;

static int values[32];
static float fvalues[32];
static double dvalues[32];

static void cfunction(int f1 , int f2 , int f3 , int f4 ,
                      int f5 , int f6 , int f7 , int f8 ,
                      int f9 , int f10, int f11, int f12,
                      int f13, int f14, int f15, int f16,
                      int f17, int f18, int f19, int f20,
                      int f21, int f22, int f23, int f24,
                      int f25, int f26, int f27, int f28,
                      int f29, int f30, int f31, int f32) 
{
	called = true;
	values[ 0] = f1;
	values[ 1] = f2;
	values[ 2] = f3;
	values[ 3] = f4;
	values[ 4] = f5;
	values[ 5] = f6;
	values[ 6] = f7;
	values[ 7] = f8;
	values[ 8] = f9;
	values[ 9] = f10;
	values[10] = f11;
	values[11] = f12;
	values[12] = f13;
	values[13] = f14;
	values[14] = f15;
	values[15] = f16;
	values[16] = f17;
	values[17] = f18;
	values[18] = f19;
	values[19] = f20;
	values[20] = f21;
	values[21] = f22;
	values[22] = f23;
	values[23] = f24;
	values[24] = f25;
	values[25] = f26;
	values[26] = f27;
	values[27] = f28;
	values[28] = f29;
	values[29] = f30;
	values[30] = f31;
	values[31] = f32;
	

	testVal = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
	          (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
	          (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
	          (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
	          (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
	          (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
	          (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
	          (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}

static void cfunctionf(float f1 , float f2 , float f3 , float f4 ,
                      float f5 , float f6 , float f7 , float f8 ,
                      float f9 , float f10, float f11, float f12,
                      float f13, float f14, float f15, float f16,
                      float f17, float f18, float f19, float f20,
                      float f21, float f22, float f23, float f24,
                      float f25, float f26, float f27, float f28,
                      float f29, float f30, float f31, float f32) 
{
	called = true;
	fvalues[ 0] = f1;
	fvalues[ 1] = f2;
	fvalues[ 2] = f3;
	fvalues[ 3] = f4;
	fvalues[ 4] = f5;
	fvalues[ 5] = f6;
	fvalues[ 6] = f7;
	fvalues[ 7] = f8;
	fvalues[ 8] = f9;
	fvalues[ 9] = f10;
	fvalues[10] = f11;
	fvalues[11] = f12;
	fvalues[12] = f13;
	fvalues[13] = f14;
	fvalues[14] = f15;
	fvalues[15] = f16;
	fvalues[16] = f17;
	fvalues[17] = f18;
	fvalues[18] = f19;
	fvalues[19] = f20;
	fvalues[20] = f21;
	fvalues[21] = f22;
	fvalues[22] = f23;
	fvalues[23] = f24;
	fvalues[24] = f25;
	fvalues[25] = f26;
	fvalues[26] = f27;
	fvalues[27] = f28;
	fvalues[28] = f29;
	fvalues[29] = f30;
	fvalues[30] = f31;
	fvalues[31] = f32;
	

	testVal = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
	          (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
	          (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
	          (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
	          (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
	          (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
	          (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
	          (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}

static void cfunctiond(double f1 , double f2 , double f3 , double f4 ,
                      double f5 , double f6 , double f7 , double f8 ,
                      double f9 , double f10, double f11, double f12,
                      double f13, double f14, double f15, double f16,
                      double f17, double f18, double f19, double f20,
                      double f21, double f22, double f23, double f24,
                      double f25, double f26, double f27, double f28,
                      double f29, double f30, double f31, double f32) 
{
	called = true;
	dvalues[ 0] = f1;
	dvalues[ 1] = f2;
	dvalues[ 2] = f3;
	dvalues[ 3] = f4;
	dvalues[ 4] = f5;
	dvalues[ 5] = f6;
	dvalues[ 6] = f7;
	dvalues[ 7] = f8;
	dvalues[ 8] = f9;
	dvalues[ 9] = f10;
	dvalues[10] = f11;
	dvalues[11] = f12;
	dvalues[12] = f13;
	dvalues[13] = f14;
	dvalues[14] = f15;
	dvalues[15] = f16;
	dvalues[16] = f17;
	dvalues[17] = f18;
	dvalues[18] = f19;
	dvalues[19] = f20;
	dvalues[20] = f21;
	dvalues[21] = f22;
	dvalues[22] = f23;
	dvalues[23] = f24;
	dvalues[24] = f25;
	dvalues[25] = f26;
	dvalues[26] = f27;
	dvalues[27] = f28;
	dvalues[28] = f29;
	dvalues[29] = f30;
	dvalues[30] = f31;
	dvalues[31] = f32;
	

	testVal = (f1  ==  1) && (f2  ==  2) && (f3  ==  3) && (f4  ==  4) &&
	          (f5  ==  5) && (f6  ==  6) && (f7  ==  7) && (f8  ==  8) &&
	          (f9  ==  9) && (f10 == 10) && (f11 == 11) && (f12 == 12) &&
	          (f13 == 13) && (f14 == 14) && (f15 == 15) && (f16 == 16) &&
	          (f17 == 17) && (f18 == 18) && (f19 == 19) && (f20 == 20) &&
	          (f21 == 21) && (f22 == 22) && (f23 == 23) && (f24 == 24) &&
	          (f25 == 25) && (f26 == 26) && (f27 == 27) && (f28 == 28) &&
	          (f29 == 29) && (f30 == 30) && (f31 == 31) && (f32 == 32);
}


static void cfunction_gen(asIScriptGeneric *gen) 
{
	called = true;
	testVal = true;
	for( int n = 0; n < 32; n++ )
	{
		values[n] = gen->GetArgDWord(n);
		if( values[n] != n+1 )
			testVal = false;
	}
}

bool TestExecute32Args()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		engine->RegisterGlobalFunction("void cfunction(int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int)", 
									   asFUNCTION(cfunction_gen), asCALL_GENERIC);
	else		
	{
		engine->RegisterGlobalFunction("void cfunction(int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int,"
													  "int, int, int, int)", 
									   asFUNCTION(cfunction), asCALL_CDECL);

		engine->RegisterGlobalFunction("void cfunctionf(float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float,"
													  "float, float, float, float)", 
									   asFUNCTION(cfunctionf), asCALL_CDECL);

		engine->RegisterGlobalFunction("void cfunctiond(double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double,"
													  "double, double, double, double)", 
									   asFUNCTION(cfunctiond), asCALL_CDECL);
	}

	ExecuteString(engine, "cfunction( 1,  2,  3,  4,"
	                                " 5,  6,  7,  8,"
	                                " 9, 10, 11, 12,"
	                                "13, 14, 15, 16,"
	                                "17, 18, 19, 20,"
	                                "21, 22, 23, 24,"
	                                "25, 26, 27, 28,"
	                                "29, 30, 31, 32)");

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
		for (int i = 0; i < 32; i++) 
			PRINTF("value %d: %d\n", i, values[i]);
		
		TEST_FAILED;
	}
	
	SKIP_ON_MAX_PORT
	{
		called = false;
		testVal = false;
		
		ExecuteString(engine, "cfunctionf( 1,  2,  3,  4,"
	                                " 5,  6,  7,  8,"
	                                " 9, 10, 11, 12,"
	                                "13, 14, 15, 16,"
	                                "17, 18, 19, 20,"
	                                "21, 22, 23, 24,"
	                                "25, 26, 27, 28,"
	                                "29, 30, 31, 32)");
		
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
			for (int i = 0; i < 32; i++) 
				PRINTF("value %d: %f\n", i, fvalues[i]);
		
			TEST_FAILED;
		}
		
		called = false;
		testVal = false;
		
		ExecuteString(engine, "cfunctiond( 1,  2,  3,  4,"
	                                " 5,  6,  7,  8,"
	                                " 9, 10, 11, 12,"
	                                "13, 14, 15, 16,"
	                                "17, 18, 19, 20,"
	                                "21, 22, 23, 24,"
	                                "25, 26, 27, 28,"
	                                "29, 30, 31, 32)");
		
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
			for (int i = 0; i < 32; i++) 
				PRINTF("value %d: %f\n", i, dvalues[i]);
		
			TEST_FAILED;
		}
	}

	engine->Release();
	
	// Success
	return fail;
}
