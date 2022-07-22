#include "utils.h"
using namespace std;

static const char * const TESTNAME = "TestOptimize";


static const char *script1 =
"void TestOptimizeAdd()        \n"
"{                             \n"
"  int a = 43, b = 13;         \n"
"  g_i0 = a + b;               \n"
"  g_i1 = a + 13;              \n"
"  g_i2 = 43 + b;              \n"
"  g_i3 = 43 + 13;             \n"
"  g_i4 = a;                   \n"
"  g_i4 += b;                  \n"
"  g_i5 = a;                   \n"
"  g_i5 += 13;                 \n"
"}                             \n"
"void TestOptimizeSub()        \n"
"{                             \n"
"  int a = 43, b = 13;         \n"
"  g_i0 = a - b;               \n"
"  g_i1 = a - 13;              \n"
"  g_i2 = 43 - b;              \n"
"  g_i3 = 43 - 13;             \n"
"  g_i4 = a;                   \n"
"  g_i4 -= b;                  \n"
"  g_i5 = a;                   \n"
"  g_i5 -= 13;                 \n"
"}                             \n"
"void TestOptimizeMul()        \n"
"{                             \n"
"  int a = 43, b = 13;         \n"
"  g_i0 = a * b;               \n"
"  g_i1 = a * 13;              \n"
"  g_i2 = 43 * b;              \n"
"  g_i3 = 43 * 13;             \n"
"  g_i4 = a;                   \n"
"  g_i4 *= b;                  \n"
"  g_i5 = a;                   \n"
"  g_i5 *= 13;                 \n"
"}                             \n"
"void TestOptimizeDiv()        \n"
"{                             \n"
"  int a = 43, b = 13;         \n"
"  g_i0 = a / b;               \n"
"  g_i1 = a / 13;              \n"
"  g_i2 = 43 / b;              \n"
"  g_i3 = 43 / 13;             \n"
"  g_i4 = a;                   \n"
"  g_i4 /= b;                  \n"
"  g_i5 = a;                   \n"
"  g_i5 /= 13;                 \n"
"}                             \n"
"void TestOptimizeMod()        \n"
"{                             \n"
"  int a = 43, b = 13;         \n"
"  g_i0 = a % b;               \n"
"  g_i1 = a % 13;              \n"
"  g_i2 = 43 % b;              \n"
"  g_i3 = 43 % 13;             \n"
"  g_i4 = a;                   \n"
"  g_i4 %= b;                  \n"
"  g_i5 = a;                   \n"
"  g_i5 %= 13;                 \n"
"}                             \n"
"void TestOptimizeAdd64()      \n"
"{                             \n"
"  int64 a = 43, b = 13;       \n"
"  g_i64_0 = a + b;            \n"
"  g_i64_1 = a + 13;           \n"
"  g_i64_2 = 43 + b;           \n"
"  g_i64_3 = 43 + 13;          \n"
"  g_i64_4 = a;                \n"
"  g_i64_4 += b;               \n"
"  g_i64_5 = a;                \n"
"  g_i64_5 += 13;              \n"
"}                             \n"
"void TestOptimizeSub64()      \n"
"{                             \n"
"  int64 a = 43, b = 13;       \n"
"  g_i64_0 = a - b;            \n"
"  g_i64_1 = a - 13;           \n"
"  g_i64_2 = 43 - b;           \n"
"  g_i64_3 = 43 - 13;          \n"
"  g_i64_4 = a;                \n"
"  g_i64_4 -= b;               \n"
"  g_i64_5 = a;                \n"
"  g_i64_5 -= 13;              \n"
"}                             \n"
"void TestOptimizeMul64()      \n"
"{                             \n"
"  int64 a = 43, b = 13;       \n"
"  g_i64_0 = a * b;            \n"
"  g_i64_1 = a * 13;           \n"
"  g_i64_2 = 43 * b;           \n"
"  g_i64_3 = 43 * 13;          \n"
"  g_i64_4 = a;                \n"
"  g_i64_4 *= b;               \n"
"  g_i64_5 = a;                \n"
"  g_i64_5 *= 13;              \n"
"}                             \n"
"void TestOptimizeDiv64()      \n"
"{                             \n"
"  int64 a = 43, b = 13;       \n"
"  g_i64_0 = a / b;            \n"
"  g_i64_1 = a / 13;           \n"
"  g_i64_2 = 43 / b;           \n"
"  g_i64_3 = 43 / 13;          \n"
"  g_i64_4 = a;                \n"
"  g_i64_4 /= b;               \n"
"  g_i64_5 = a;                \n"
"  g_i64_5 /= 13;              \n"
"}                             \n"
"void TestOptimizeMod64()      \n"
"{                             \n"
"  int64 a = 43, b = 13;       \n"
"  g_i64_0 = a % b;            \n"
"  g_i64_1 = a % 13;           \n"
"  g_i64_2 = 43 % b;           \n"
"  g_i64_3 = 43 % 13;          \n"
"  g_i64_4 = a;                \n"
"  g_i64_4 %= b;               \n"
"  g_i64_5 = a;                \n"
"  g_i64_5 %= 13;              \n"
"}                             \n"
"void TestOptimizeAddf()       \n"
"{                             \n"
"  float a = 43, b = 13;       \n"
"  g_f0 = a + b;               \n"
"  g_f1 = a + 13;              \n"
"  g_f2 = 43 + b;              \n"
"  g_f3 = 43.0f + 13;          \n"
"  g_f4 = a;                   \n"
"  g_f4 += b;                  \n"
"  g_f5 = a;                   \n"
"  g_f5 += 13;                 \n"
"}                             \n"
"void TestOptimizeSubf()       \n"
"{                             \n"
"  float a = 43, b = 13;       \n"
"  g_f0 = a - b;               \n"
"  g_f1 = a - 13;              \n"
"  g_f2 = 43 - b;              \n"
"  g_f3 = 43.0f - 13;          \n"
"  g_f4 = a;                   \n"
"  g_f4 -= b;                  \n"
"  g_f5 = a;                   \n"
"  g_f5 -= 13;                 \n"
"}                             \n"
"void TestOptimizeMulf()       \n"
"{                             \n"
"  float a = 43, b = 13;       \n"
"  g_f0 = a * b;               \n"
"  g_f1 = a * 13;              \n"
"  g_f2 = 43 * b;              \n"
"  g_f3 = 43.0f * 13;          \n"
"  g_f4 = a;                   \n"
"  g_f4 *= b;                  \n"
"  g_f5 = a;                   \n"
"  g_f5 *= 13;                 \n"
"}                             \n"
"void TestOptimizeDivf()       \n"
"{                             \n"
"  float a = 43, b = 13;       \n"
"  g_f0 = a / b;               \n"
"  g_f1 = a / 13;              \n"
"  g_f2 = 43 / b;              \n"
"  g_f3 = 43.0f / 13.0f;       \n"
"  g_f4 = a;                   \n"
"  g_f4 /= b;                  \n"
"  g_f5 = a;                   \n"
"  g_f5 /= 13;                 \n"
"}                             \n"
"void TestOptimizeModf()       \n"
"{                             \n"
"  float a = 43, b = 13;       \n"
"  g_f0 = a % b;               \n"
"  g_f1 = a % 13;              \n"
"  g_f2 = 43 % b;              \n"
"  g_f3 = 43.0f % 13;          \n"
"  g_f4 = a;                   \n"
"  g_f4 %= b;                  \n"
"  g_f5 = a;                   \n"
"  g_f5 %= 13;                 \n"
"}                             \n"
"void TestOptimizeAddd()       \n"
"{                             \n"
"  double a = 43.0, b = 13.0;  \n"
"  g_d0 = a + b;               \n"
"  g_d1 = a + 13.0;            \n"
"  g_d2 = 43.0 + b;            \n"
"  g_d3 = 43.0 + 13.0;         \n"
"  g_d4 = a;                   \n"
"  g_d4 += b;                  \n"
"  g_d5 = a;                   \n"
"  g_d5 += 13.0;               \n"
"}                             \n"
"void TestOptimizeSubd()       \n"
"{                             \n"
"  double a = 43.0, b = 13.0;  \n"
"  g_d0 = a - b;               \n"
"  g_d1 = a - 13.0;            \n"
"  g_d2 = 43.0 - b;            \n"
"  g_d3 = 43.0 - 13.0;         \n"
"  g_d4 = a;                   \n"
"  g_d4 -= b;                  \n"
"  g_d5 = a;                   \n"
"  g_d5 -= 13.0;               \n"
"}                             \n"
"void TestOptimizeMuld()       \n"
"{                             \n"
"  double a = 43.0, b = 13.0;  \n"
"  g_d0 = a * b;               \n"
"  g_d1 = a * 13.0;            \n"
"  g_d2 = 43.0 * b;            \n"
"  g_d3 = 43.0 * 13.0;         \n"
"  g_d4 = a;                   \n"
"  g_d4 *= b;                  \n"
"  g_d5 = a;                   \n"
"  g_d5 *= 13.0;               \n"
"}                             \n"
"void TestOptimizeDivd()       \n"
"{                             \n"
"  double a = 43.0, b = 13.0;  \n"
"  g_d0 = a / b;               \n"
"  g_d1 = a / 13.0;            \n"
"  g_d2 = 43.0 / b;            \n"
"  g_d3 = 43.0 / 13.0;         \n"
"  g_d4 = a;                   \n"
"  g_d4 /= b;                  \n"
"  g_d5 = a;                   \n"
"  g_d5 /= 13.0;               \n"
"}                             \n"
"void TestOptimizeModd()       \n"
"{                             \n"
"  double a = 43.0, b = 13.0;  \n"
"  g_d0 = a % b;               \n"
"  g_d1 = a % 13.0;            \n"
"  g_d2 = 43.0 % b;            \n"
"  g_d3 = 43.0 % 13.0;         \n"
"  g_d4 = a;                   \n"
"  g_d4 %= b;                  \n"
"  g_d5 = a;                   \n"
"  g_d5 %= 13.0;               \n"
"}                             \n"
"void TestOptimizeAnd()        \n"
"{                             \n"
"  uint a = 0xF3, b = 0x17;    \n"
"  g_b0 = a & b;               \n"
"  g_b1 = a & 0x17;            \n"
"  g_b2 = 0xF3 & b;            \n"
"  g_b3 = 0xF3 & 0x17;         \n"
"  g_b4 = a;                   \n"
"  g_b4 &= b;                  \n"
"  g_b5 = a;                   \n"
"  g_b5 &= 0x17;               \n"
"}                             \n"
"void TestOptimizeOr()         \n"
"{                             \n"
"  uint a = 0xF3, b = 0x17;    \n"
"  g_b0 = a | b;               \n"
"  g_b1 = a | 0x17;            \n"
"  g_b2 = 0xF3 | b;            \n"
"  g_b3 = 0xF3 | 0x17;         \n"
"  g_b4 = a;                   \n"
"  g_b4 |= b;                  \n"
"  g_b5 = a;                   \n"
"  g_b5 |= 0x17;               \n"
"}                             \n"
"void TestOptimizeXor()        \n"
"{                             \n"
"  uint a = 0xF3, b = 0x17;    \n"
"  g_b0 = a ^ b;               \n"
"  g_b1 = a ^ 0x17;            \n"
"  g_b2 = 0xF3 ^ b;            \n"
"  g_b3 = 0xF3 ^ 0x17;         \n"
"  g_b4 = a;                   \n"
"  g_b4 ^= b;                  \n"
"  g_b5 = a;                   \n"
"  g_b5 ^= 0x17;               \n"
"}                             \n"
"void TestOptimizeSLL()        \n"
"{                             \n"
"  uint a = 0xF3;              \n"
"  uint b = 3;                 \n"
"  g_b0 = a << b;              \n"
"  g_b1 = a << 3;              \n"
"  g_b2 = 0xF3 << b;           \n"
"  g_b3 = 0xF3 << 3;           \n"
"  g_b4 = a;                   \n"
"  g_b4 <<= b;                 \n"
"  g_b5 = a;                   \n"
"  g_b5 <<= 3;                 \n"
"}                             \n"
"void TestOptimizeSRA()        \n"
"{                             \n"
"  uint a = 0xF3;              \n"
"  uint b = 3;                 \n"
"  g_b0 = a >>> b;             \n"
"  g_b1 = a >>> 3;             \n"
"  g_b2 = 0xF3 >>> b;          \n"
"  g_b3 = 0xF3 >>> 3;          \n"
"  g_b4 = a;                   \n"
"  g_b4 >>>= b;                \n"
"  g_b5 = a;                   \n"
"  g_b5 >>>= 3;                \n"
"}                             \n"
"void TestOptimizeSRL()        \n"
"{                             \n"
"  uint a = 0xF3;              \n"
"  uint b = 3;                 \n"
"  g_b0 = a >> b;              \n"
"  g_b1 = a >> 3;              \n"
"  g_b2 = 0xF3 >> b;           \n"
"  g_b3 = 0xF3 >> 3;           \n"
"  g_b4 = a;                   \n"
"  g_b4 >>= b;                 \n"
"  g_b5 = a;                   \n"
"  g_b5 >>= 3;                 \n"
"}                             \n"
"void TestOptimizeAnd64()      \n"
"{                             \n"
"  uint64 a = 0xF3, b = 0x17;  \n"
"  g_b64_0 = a & b;            \n"
"  g_b64_1 = a & 0x17;         \n"
"  g_b64_2 = 0xF3 & b;         \n"
"  g_b64_3 = 0xF3 & 0x17;      \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 &= b;               \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 &= 0x17;            \n"
"}                             \n"
"void TestOptimizeOr64()       \n"
"{                             \n"
"  uint64 a = 0xF3, b = 0x17;  \n"
"  g_b64_0 = a | b;            \n"
"  g_b64_1 = a | 0x17;         \n"
"  g_b64_2 = 0xF3 | b;         \n"
"  g_b64_3 = 0xF3 | 0x17;      \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 |= b;               \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 |= 0x17;            \n"
"}                             \n"
"void TestOptimizeXor64()      \n"
"{                             \n"
"  uint64 a = 0xF3, b = 0x17;  \n"
"  g_b64_0 = a ^ b;            \n"
"  g_b64_1 = a ^ 0x17;         \n"
"  g_b64_2 = 0xF3 ^ b;         \n"
"  g_b64_3 = 0xF3 ^ 0x17;      \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 ^= b;               \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 ^= 0x17;            \n"
"}                             \n"
"void TestOptimizeSLL64()      \n"
"{                             \n"
"  uint64 a = 0xF3;            \n"
"  uint64 b = 3;               \n"
"  g_b64_0 = a << b;           \n"
"  g_b64_1 = a << 3;           \n"
"  g_b64_2 = 0xF3 << b;        \n"
"  g_b64_3 = 0xF3 << 3;        \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 <<= b;              \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 <<= 3;              \n"
"}                             \n"
"void TestOptimizeSRA64()      \n"
"{                             \n"
"  uint64 a = 0xF3;            \n"
"  uint64 b = 3;               \n"
"  g_b64_0 = a >>> b;          \n"
"  g_b64_1 = a >>> 3;          \n"
"  g_b64_2 = 0xF3 >>> b;       \n"
"  g_b64_3 = 0xF3 >>> 3;       \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 >>>= b;             \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 >>>= 3;             \n"
"}                             \n"
"void TestOptimizeSRL64()      \n"
"{                             \n"
"  uint64 a = 0xF3;            \n"
"  uint64 b = 3;               \n"
"  g_b64_0 = a >> b;           \n"
"  g_b64_1 = a >> 3;           \n"
"  g_b64_2 = 0xF3 >> b;        \n"
"  g_b64_3 = 0xF3 >> 3;        \n"
"  g_b64_4 = a;                \n"
"  g_b64_4 >>= b;              \n"
"  g_b64_5 = a;                \n"
"  g_b64_5 >>= 3;              \n"
"}                             \n";


static asINT64 g_i64[6];
static int g_i[6];
static float g_f[6];
static double g_d[6];
static asUINT g_b[6];
static asQWORD g_b64[6];

bool TestOptimize()
{
	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterGlobalProperty("int g_i0", &g_i[0]);
	engine->RegisterGlobalProperty("int g_i1", &g_i[1]);
	engine->RegisterGlobalProperty("int g_i2", &g_i[2]);
	engine->RegisterGlobalProperty("int g_i3", &g_i[3]);
	engine->RegisterGlobalProperty("int g_i4", &g_i[4]);
	engine->RegisterGlobalProperty("int g_i5", &g_i[5]);
	engine->RegisterGlobalProperty("int64 g_i64_0", &g_i64[0]);
	engine->RegisterGlobalProperty("int64 g_i64_1", &g_i64[1]);
	engine->RegisterGlobalProperty("int64 g_i64_2", &g_i64[2]);
	engine->RegisterGlobalProperty("int64 g_i64_3", &g_i64[3]);
	engine->RegisterGlobalProperty("int64 g_i64_4", &g_i64[4]);
	engine->RegisterGlobalProperty("int64 g_i64_5", &g_i64[5]);
	engine->RegisterGlobalProperty("float g_f0", &g_f[0]);
	engine->RegisterGlobalProperty("float g_f1", &g_f[1]);
	engine->RegisterGlobalProperty("float g_f2", &g_f[2]);
	engine->RegisterGlobalProperty("float g_f3", &g_f[3]);
	engine->RegisterGlobalProperty("float g_f4", &g_f[4]);
	engine->RegisterGlobalProperty("float g_f5", &g_f[5]);
	engine->RegisterGlobalProperty("double g_d0", &g_d[0]);
	engine->RegisterGlobalProperty("double g_d1", &g_d[1]);
	engine->RegisterGlobalProperty("double g_d2", &g_d[2]);
	engine->RegisterGlobalProperty("double g_d3", &g_d[3]);
	engine->RegisterGlobalProperty("double g_d4", &g_d[4]);
	engine->RegisterGlobalProperty("double g_d5", &g_d[5]);
	engine->RegisterGlobalProperty("uint g_b0", &g_b[0]);
	engine->RegisterGlobalProperty("uint g_b1", &g_b[1]);
	engine->RegisterGlobalProperty("uint g_b2", &g_b[2]);
	engine->RegisterGlobalProperty("uint g_b3", &g_b[3]);
	engine->RegisterGlobalProperty("uint g_b4", &g_b[4]);
	engine->RegisterGlobalProperty("uint g_b5", &g_b[5]);
	engine->RegisterGlobalProperty("uint64 g_b64_0", &g_b64[0]);
	engine->RegisterGlobalProperty("uint64 g_b64_1", &g_b64[1]);
	engine->RegisterGlobalProperty("uint64 g_b64_2", &g_b64[2]);
	engine->RegisterGlobalProperty("uint64 g_b64_3", &g_b64[3]);
	engine->RegisterGlobalProperty("uint64 g_b64_4", &g_b64[4]);
	engine->RegisterGlobalProperty("uint64 g_b64_5", &g_b64[5]);


	COutStream out;

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	mod->Build();

	int n;
	ExecuteString(engine, "TestOptimizeAdd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i[n] != 56 )
		{
			PRINTF("%s: Optimized add failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSub()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i[n] != 30 )
		{
			PRINTF("%s: Optimized sub failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMul()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i[n] != 559 )
		{
			PRINTF("%s: Optimized mul failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeDiv()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i[n] != 3 )
		{
			PRINTF("%s: Optimized div failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMod()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i[n] != 4 )
		{
			PRINTF("%s: Optimized mod failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "g_i0 = 13; g_i1 = -g_i0; g_i2 = -13;"); if( g_i[1] != -13 || g_i[2] != -13 ) { PRINTF("%s: negi failed\n", TESTNAME); }
	ExecuteString(engine, "g_i0 = 0; g_i1 = g_i0++; g_i2 = ++g_i0;"); if( g_i[0] != 2 || g_i[1] != 0 || g_i[2] != 2 ) { PRINTF("%s: inci failed\n", TESTNAME); }
	ExecuteString(engine, "g_i0 = 0; g_i1 = g_i0--; g_i2 = --g_i0;"); if( g_i[0] != -2 || g_i[1] != 0 || g_i[2] != -2 ) { PRINTF("%s: deci failed\n", TESTNAME); }


	ExecuteString(engine, "TestOptimizeAdd64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i64[n] != 56 )
		{
			PRINTF("%s: Optimized add64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSub64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i64[n] != 30 )
		{
			PRINTF("%s: Optimized sub64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMul64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i64[n] != 559 )
		{
			PRINTF("%s: Optimized mul64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeDiv64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i64[n] != 3 )
		{
			PRINTF("%s: Optimized div64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMod64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_i64[n] != 4 )
		{
			PRINTF("%s: Optimized mod64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "g_i64_0 = 13; g_i64_1 = -g_i64_0; g_i64_2 = -13;"); if( g_i64[1] != -13 || g_i64[2] != -13 ) { PRINTF("%s: negi64 failed\n", TESTNAME); }
	ExecuteString(engine, "g_i64_0 = 0; g_i64_1 = g_i64_0++; g_i64_2 = ++g_i64_0;"); if( g_i64[0] != 2 || g_i64[1] != 0 || g_i64[2] != 2 ) { PRINTF("%s: inci64 failed\n", TESTNAME); }
	ExecuteString(engine, "g_i64_0 = 0; g_i64_1 = g_i64_0--; g_i64_2 = --g_i64_0;"); if( g_i64[0] != -2 || g_i64[1] != 0 || g_i64[2] != -2 ) { PRINTF("%s: deci64 failed\n", TESTNAME); }



	ExecuteString(engine, "TestOptimizeAddf()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_f[n] != 56 )
		{
			PRINTF("%s: Optimized addf failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSubf()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_f[n] != 30 )
		{
			PRINTF("%s: Optimized subf failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMulf()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_f[n] != 559 )
		{
			PRINTF("%s: Optimized mulf failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeDivf()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_f[n] != 3.30769230769230748f )
		{
			PRINTF("%s: Optimized divf failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeModf()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_f[n] != 4 )
		{
			PRINTF("%s: Optimized modf failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "g_f0 = 13; g_f1 = -g_f0; g_f2 = -13;"); if( g_f[1] != -13 || g_f[2] != -13 ) { PRINTF("%s: negf failed\n", TESTNAME); }
	ExecuteString(engine, "g_f0 = 0; g_f1 = g_f0++; g_f2 = ++g_f0;"); if( g_f[0] != 2 || g_f[1] != 0 || g_f[2] != 2 ) { PRINTF("%s: incf failed\n", TESTNAME); }
	ExecuteString(engine, "g_f0 = 0; g_f1 = g_f0--; g_f2 = --g_f0;"); if( g_f[0] != -2 || g_f[1] != 0 || g_f[2] != -2 ) { PRINTF("%s: decf failed\n", TESTNAME); }

	ExecuteString(engine, "TestOptimizeAddd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_d[n] != 56 )
		{
			PRINTF("%s: Optimized addd failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSubd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_d[n] != 30 )
		{
			PRINTF("%s: Optimized subd failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeMuld()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_d[n] != 559 )
		{
			PRINTF("%s: Optimized muld failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeDivd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_d[n] != 3.30769230769230748 )
		{
			PRINTF("%s: Optimized divd failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeModd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_d[n] != 4 )
		{
			PRINTF("%s: Optimized modd failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "g_d0 = 13; g_d1 = -g_d0; g_d2 = -13;"); if( g_d[1] != -13 || g_d[2] != -13 ) { PRINTF("%s: negd failed\n", TESTNAME); }
	ExecuteString(engine, "g_d0 = 0; g_d1 = g_d0++; g_d2 = ++g_d0;"); if( g_d[0] != 2 || g_d[1] != 0 || g_d[2] != 2 ) { PRINTF("%s: incd failed\n", TESTNAME); }
	ExecuteString(engine, "g_d0 = 0; g_d1 = g_d0--; g_d2 = --g_d0;"); if( g_d[0] != -2 || g_d[1] != 0 || g_d[2] != -2 ) { PRINTF("%s: decd failed\n", TESTNAME); }

	ExecuteString(engine, "TestOptimizeAnd()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3 & 0x17) )
		{
			PRINTF("%s: Optimized and failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeOr()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3 | 0x17) )
		{
			PRINTF("%s: Optimized or failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeXor()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3 ^ 0x17) )
		{
			PRINTF("%s: Optimized xor failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSLL()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3 << 3) )
		{
			PRINTF("%s: Optimized sll failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSRL()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3u >> 3) )
		{
			PRINTF("%s: Optimized srl failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSRA()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b[n] != (0xF3 >> 3) )
		{
			PRINTF("%s: Optimized sra failed\n", TESTNAME);
			break;
		}
	}
	
	ExecuteString(engine, "g_b0 = 0xF3; g_b1 = ~g_b0; g_b2 = ~0xF3;"); if( g_b[1] != ~0xF3 || g_b[2] != ~0xF3 ) { PRINTF("%s: bnot failed\n", TESTNAME); }


	ExecuteString(engine, "TestOptimizeAnd64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3 & 0x17) )
		{
			PRINTF("%s: Optimized and64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeOr64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3 | 0x17) )
		{
			PRINTF("%s: Optimized or64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeXor64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3 ^ 0x17) )
		{
			PRINTF("%s: Optimized xor64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSLL64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3 << 3) )
		{
			PRINTF("%s: Optimized sll64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSRL64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3u >> 3) )
		{
			PRINTF("%s: Optimized srl64 failed\n", TESTNAME);
			break;
		}
	}

	ExecuteString(engine, "TestOptimizeSRA64()", mod);
	for( n = 0; n < 6; ++ n )
	{
		if( g_b64[n] != (0xF3 >> 3) )
		{
			PRINTF("%s: Optimized sra64 failed\n", TESTNAME);
			break;
		}
	}
	
	ExecuteString(engine, "g_b64_0 = 0xF3; g_b64_1 = ~g_b64_0; g_b64_2 = ~uint64(0xF3);"); if( g_b64[1] != ~I64(0xF3) || g_b64[2] != ~I64(0xF3) ) { PRINTF("%s: bnot64 failed\n", TESTNAME); }



	int r;
	r = ExecuteString(engine, "bool b = false; if( !b ) {}");
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: !b failed\n", TESTNAME);
		TEST_FAILED;
	}
		
	r = ExecuteString(engine, "bool b = false; b = not b;");
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: !b failed\n", TESTNAME);
		TEST_FAILED;
	}

	r = ExecuteString(engine, "uint tmp = 50; uint x = tmp + 0x50;");
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: uint failed\n", TESTNAME);
		TEST_FAILED;
	}

	bool boolValue;
	engine->RegisterGlobalProperty("bool boolValue", &boolValue);
	r = ExecuteString(engine, "bool a = false, b = false; boolValue = a xor b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = false; boolValue = a xor b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = true; boolValue = a xor b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = false, b = true; boolValue = a xor b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;

	r = ExecuteString(engine, "bool a = false, b = false; boolValue = a and b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = false; boolValue = a and b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = true; boolValue = a and b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = false, b = true; boolValue = a and b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;

	r = ExecuteString(engine, "bool a = false, b = false; boolValue = a or b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = false; boolValue = a or b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = true; boolValue = a or b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = false, b = true; boolValue = a or b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;

	r = ExecuteString(engine, "bool a = false, b = false; boolValue = a == b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = false; boolValue = a == b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = true; boolValue = a == b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = false, b = true; boolValue = a == b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;

	r = ExecuteString(engine, "bool a = false, b = false; boolValue = a != b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = false; boolValue = a != b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = true, b = true; boolValue = a != b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != false ) TEST_FAILED;
	r = ExecuteString(engine, "bool a = false, b = true; boolValue = a != b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;

	r = ExecuteString(engine, "bool a = false; bool b = a == false; boolValue = b;"); assert( r == asEXECUTION_FINISHED );
	if( boolValue != true ) TEST_FAILED;

	r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "assert(float(5-10) == float(-5));");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	// Test integer division when most significant bit is set
	r = ExecuteString(engine, "uint32 a = 0x80000000; \n uint result = a/4; \n assert( result == 0x20000000 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int32 a = int32(0x80000000); \n int result = a/4; \n assert( result == int32(0xE0000000) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint64 a = 0x8000000000000000; \n uint64 result = a/4; \n assert( result == 0x2000000000000000 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int64 a = int64(0x8000000000000000); \n int64 result = a/4; \n assert( result == int64(0xE000000000000000) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint result = 0x80000000/4; \n assert( result == 0x20000000 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int result = int32(0x80000000)/4; \n assert( result == int32(0xE0000000) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint64 result = 0x8000000000000000/4; \n assert( result == 0x2000000000000000 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int64 result = int64(0x8000000000000000)/4; \n assert( result == int64(0xE000000000000000) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	// Test integer modulo when most significant bit is set
	r = ExecuteString(engine, "uint32 a = 0x80000000; \n uint result = a%5; \n assert( result == 0x00000003 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int32 a = int32(0x80000000); \n int result = a%5; \n assert( result == int32(0xFFFFFFFD) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint64 a = 0x8000000000000000; \n uint64 result = a%5; \n assert( result == 0x0000000000000003 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int64 a = int64(0x8000000000000000); \n int64 result = a%5; \n assert( result == int64(0xFFFFFFFFFFFFFFFD) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint result = 0x80000000%5; \n assert( result == 0x00000003 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int result = int32(0x80000000)%5; \n assert( result == int32(0xFFFFFFFD) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "uint64 result = 0x8000000000000000%5; \n assert( result == 0x0000000000000003 );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	r = ExecuteString(engine, "int64 result = int64(0x8000000000000000)%5; \n assert( result == int64(0xFFFFFFFFFFFFFFFD) );");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	// TODO: optimize: Converting an integer variable to double shouldn't require an intermediate copy of the integer variable

	// Validate the bytecode sequence for a for-loop and switch case
	{
		const char *script = 
			"void add(int) {}                \n"
			"void func()                     \n" 
			"{                               \n" 
			"  for( int n = -1; n < 2; ++n ) \n" 
			"    switch( n )                 \n" 
			"    {                           \n" 
			"    case 0:                     \n" 
			"      add(0);                   \n" 
			"      break;                    \n" 
			"    case -1:                    \n" 
			"      add(-1);                  \n" 
			"      break;                    \n" 
			"    default:                    \n" 
			"      add(255);                 \n" 
			"      break;                    \n" 
			"    }                           \n" 
			"}                               \n";
		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_SetV4,asBC_JMP,asBC_SUSPEND,
				asBC_SUSPEND,asBC_CMPIi,asBC_JP,asBC_CMPIi,asBC_JZ,asBC_CMPIi,asBC_JZ,asBC_JMP,
				asBC_SUSPEND,asBC_PshC4,asBC_CALL,
				asBC_SUSPEND,asBC_JMP,
				asBC_SUSPEND,asBC_PshC4,asBC_CALL,
				asBC_SUSPEND,asBC_JMP,
				asBC_SUSPEND,asBC_PshC4,asBC_CALL,
				asBC_SUSPEND,asBC_SUSPEND,asBC_IncVi,asBC_SUSPEND,asBC_CMPIi,asBC_JS,
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	// Validate bytecode sequence for a class constructor that sets a member
	{
		const char *script = 
			"class C {int val; C() {val = 0;}} \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asITypeInfo *type = mod->GetTypeInfoByName("C");
		asEBehaviours beh;
		asIScriptFunction *func = type->GetBehaviourByIndex(8, &beh);
		if( beh != asBEHAVE_CONSTRUCT )
			TEST_FAILED;
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_SetV4,asBC_LoadThisR,asBC_WRTV4,asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	// Validate bytecode sequence for a function returning a handle by reference
	{
		const char *script = 
			"class C {} \n"
			"C @c; \n"
			"C @&func_inner() { return c; } \n"
			"C @func() { C @l = func_inner(); return l; } \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_CALL,asBC_PshRPtr,asBC_RDSPtr,asBC_RefCpyV,asBC_PopPtr,
				asBC_SUSPEND,asBC_LOADOBJ,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	// Validate bytecode sequence for a function returning a handle by value
	{
		const char *script = 
			"class C {} \n"
			"C @func_inner() { return C(); } \n"
			"void func() { C @l = func_inner(); } \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
//		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_CALL,asBC_FREE,asBC_STOREOBJ,
				asBC_SUSPEND,asBC_FREE,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	// Validate bytecode sequence for a function returning a handle
	{
		const char *script = 
			"class C {} \n"
			"void func() { C c; C @h = c; @h = c; } \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_CALL,asBC_STOREOBJ,
				asBC_SUSPEND,asBC_PshVPtr,asBC_RefCpyV,asBC_PopPtr,
				asBC_SUSPEND,asBC_PshVPtr,asBC_RefCpyV,asBC_PopPtr,
				asBC_SUSPEND,asBC_FREE,asBC_FREE,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	// Validate byte sequence for multiply with constant
	// TODO: optimize: currently divisions are not optimized as there is no DIVIi instruction
	{
		const char *script = 
			"void func() { int a = 42, b = a*2; } \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_SetV4,asBC_MULIi,
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;
	}

	engine->Release();

	// Validate sequence for passing object to const ref arg
	// Should use the copy constructor when creating the temporary value
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);

		const char *script =
			"string text = 'hello'; \n"
			"void main() { \n"
			"  func(text); \n"
			"} \n"
			"void func(const string &in) {} \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] = 
			{	
				asBC_SUSPEND,asBC_PshGPtr,asBC_CHKREF,asBC_PSF,asBC_CALLSYS,asBC_PSF,asBC_CALL,asBC_PSF,asBC_CALLSYS,
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		engine->Release();
	}

	// Passing a reference type to a function taking const type &in
	// shouldn't copy the object, only hold a handle for safe keeping
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script =
			"void main() { \n"
			"  func(glob); \n"
			"} \n"
			"C glob; \n"
			"class C {} \n"
			"void func(const C &in arg) { assert( arg is glob ); } \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] = 
			{
				// TODO: optimize: ChkNullS shouldn't be necessary
				asBC_SUSPEND,asBC_PshGPtr,asBC_CHKREF,asBC_RefCpyV,asBC_PopPtr,asBC_PshVPtr,asBC_ChkNullS,asBC_CALL,asBC_FREE,
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test bytecode produced for common string operations
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);

		const char *script =
			"void main() { \n"
			"  string val = 'test'; \n" // initialization with a string constant
			"  assert( val == 'test' ); \n" // comparison with a string constant
			"} \n";

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] = 
			{
				asBC_SUSPEND, asBC_PGA, asBC_PSF, asBC_CALLSYS,
				asBC_SUSPEND, asBC_PGA, asBC_PSF, asBC_CALLSYS, asBC_CpyRtoV4, asBC_PshV4, asBC_CALLSYS,
				asBC_SUSPEND, asBC_PSF, asBC_CALLSYS, asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Avoid unnecessary copies of objects
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		engine->RegisterGlobalFunction("bool GetInputDown(int, const string &in)", asFUNCTION(0), asCALL_GENERIC);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void func() { \n"
			"	GetInputDown(1, 'ctrl'); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] = 
			{
				asBC_SUSPEND,
				// Push the pointer to the string constant on the stack
				asBC_PGA,
				// Push the constant 1 on the stack
				asBC_PshC4,
				// Call the GetInputDown 
				asBC_CALLSYS,
				// Finish
				asBC_SUSPEND,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		engine->Release();
	}

	// Local handle variable initialized with null shall not generate any bytecode
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class T {} \n"
			"void func() { \n"
			"	T @t = null; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("func");
		asBYTE expect[] =
		{
			asBC_SUSPEND,
			asBC_FREE,    // TODO: this can be removed as well if the compiler can determine that the variable is never used
			asBC_RET
		};
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Success
	return fail;
}
