#include <stdio.h>
#if defined(WIN32) || defined(_WIN64)
#include <conio.h>
#endif
#if defined(_MSC_VER)
#include <crtdbg.h>
#endif

#ifdef __dreamcast__
#include <kos.h>

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

#endif

bool TestCreateEngine();
bool TestExecute();
bool TestExecute1Arg();
bool TestExecute2Args();
bool TestExecute4Args();
bool TestExecute4Argsf();
bool TestExecute32Args();
bool TestExecuteMixedArgs();
bool TestExecute32MixedArgs();
bool TestExecuteThis32MixedArgs();
bool TestTempVar();
bool TestExecuteScript();
bool Test2Modules();
bool TestStdcall4Args();
bool TestInt64();
bool TestModuleRef();
bool TestEnumGlobVar();
bool TestGlobalVar();
bool TestBStr();
bool TestSwitch();
bool TestNegateOperator();
bool TestCDecl_Class();
bool TestCDecl_ClassA();
bool TestCDecl_ClassC();
bool TestCDecl_ClassD();
bool TestCDecl_ClassK();
bool TestThiscallClass();
bool TestNotComplexThisCall();
bool TestNotComplexStdcall();
bool TestReturnWithCDeclObjFirst();
bool TestStdString();
bool TestLongToken();
bool TestVirtualMethod();
bool TestMultipleInheritance();
bool TestVirtualInheritance();
bool TestStack();
bool TestExecuteString();
bool TestCondition();
bool TestFuncOverload();
bool TestNeverVisited();
bool TestNested();
bool TestConstructor();
bool TestOptimize();
bool TestNotInitialized();
bool TestVector3();

namespace TestException         { bool Test(); }
namespace TestCDeclReturn       { bool Test(); }
namespace TestCustomMem         { bool Test(); }
namespace TestGeneric           { bool Test(); }
namespace TestDebug             { bool Test(); }
namespace TestSuspend           { bool Test(); }
namespace TestConstProperty     { bool Test(); }
namespace TestConstObject       { bool Test(); }
namespace TestOutput            { bool Test(); }
namespace TestImport            { bool Test(); }
namespace TestImport2           { bool Test(); }
namespace Test2Func             { bool Test(); }
namespace TestDiscard           { bool Test(); }
namespace TestCircularImport    { bool Test(); }
namespace TestMultiAssign       { bool Test(); }
namespace TestSaveLoad          { bool Test(); }
namespace TestConstructor2      { bool Test(); }
namespace TestScriptCall        { bool Test(); }
namespace TestArray             { bool Test(); }
namespace TestArrayHandle       { bool Test(); }
namespace TestStdVector         { bool Test(); }
namespace TestArrayObject       { bool Test(); }
namespace TestPointer           { bool Test(); }
namespace TestConversion        { bool Test(); }
namespace TestObject            { bool Test(); }
namespace TestObject2           { bool Test(); }
namespace TestObject3           { bool Test(); }
namespace TestExceptionMemory   { bool Test(); }
namespace TestArgRef            { bool Test(); }
namespace TestObjHandle         { bool Test(); }
namespace TestObjHandle2        { bool Test(); }
namespace TestObjZeroSize       { bool Test(); }
namespace TestRefArgument       { bool Test(); }
namespace TestStack2            { bool Test(); }
namespace TestScriptString      { bool Test(); }
namespace TestScriptStruct      { bool Test(); }
namespace TestStructIntf        { bool Test(); }
namespace TestAutoHandle        { bool Test(); }
namespace TestAny               { bool Test(); }
namespace TestArrayIntf         { bool Test(); }
namespace TestDynamicConfig     { bool Test(); }
namespace TestStream            { bool Test(); }
namespace TestConfig            { bool Test(); }
namespace TestConfigAccess      { bool Test(); }
namespace TestFloat             { bool Test(); }
namespace TestVector3_2         { bool Test(); }
namespace TestDict              { bool Test(); }
namespace TestUnsafeRef         { bool Test(); }
namespace TestReturnString      { bool Test(); }
namespace TestScriptClassMethod { bool Test(); }
namespace TestPostProcess       { bool Test(); }
namespace TestShark             { bool Test(); }
namespace TestParser            { bool Test(); }
namespace TestInterface         { bool Test(); }
namespace TestCompiler          { bool Test(); }
namespace TestSingleton         { bool Test(); }
namespace TestCastOp            { bool Test(); }
namespace TestFor               { bool Test(); }
namespace TestBits              { bool Test(); }
namespace TestGetArgPtr         { bool Test(); }
namespace TestCString           { bool Test(); }
namespace TestBool              { bool Test(); }
namespace TestInt8              { bool Test(); }
namespace TestScriptMath        { bool Test(); }
namespace TestVarType           { bool Test(); }
namespace TestDestructor        { bool Test(); }
namespace TestRegisterType      { bool Test(); }
namespace TestFactory           { bool Test(); }
namespace TestRZ                { bool Test(); }
namespace TestImplicitCast      { bool Test(); }
namespace TestAssign            { bool Test(); }
namespace TestTypedef           { bool Test(); }
namespace TestEnum              { bool Test(); }
namespace TestRefCast           { bool Test(); }
namespace TestImplicitHandle    { bool Test(); }
namespace TestAuto              { bool Test(); }
namespace TestInheritance       { bool Test(); }
namespace TestDump              { bool Test(); }
namespace TestTemplate          { bool Test(); }
namespace TestOperator          { bool Test(); }
namespace TestGetSet            { bool Test(); }
namespace TestModule            { bool Test(); }
namespace TestFunctionPtr       { bool Test(); }
namespace TestGarbageCollect    { bool Test(); }
namespace TestScriptRetRef      { bool Test(); }
namespace TestDefaultArg        { bool Test(); }
namespace TestNamedArgs         { bool Test(); }
namespace TestShared            { bool Test(); }
namespace TestNamespace         { bool Test(); }
namespace TestCDeclObjLast      { bool Test(); }
namespace TestMixin             { bool Test(); }
namespace TestThiscallAsGlobal  { bool Test(); }
namespace TestPow               { bool Test(); }
namespace TestThisCallMethod    { bool Test(); }
namespace TestThisCallMethod_ConfigErrors { bool Test(); }
namespace TestPropIntegerDivision { bool Test(); }
namespace TestComposition       { bool Test(); }
namespace Test_Native_DefaultFunc { bool Test(); }

namespace Test_Addon_ScriptArray   { bool Test(); }
namespace Test_Addon_ScriptHandle  { bool Test(); }
namespace Test_Addon_Serializer    { bool Test(); }
namespace Test_Addon_ScriptMath    { bool Test(); }
namespace Test_Addon_ScriptBuilder { bool Test(); }
namespace Test_Addon_Dictionary    { bool Test(); }
namespace Test_Addon_Debugger      { bool Test(); }
namespace Test_Addon_WeakRef       { bool Test(); }
namespace Test_Addon_ScriptGrid    { bool Test(); }
namespace Test_Addon_ContextMgr    { bool Test(); }
namespace Test_Addon_ScriptFile    { bool Test(); }
namespace Test_Addon_DateTime      { bool Test(); }
namespace Test_Addon_StdString     { bool Test(); }

#include "utils.h"

void DetectMemoryLeaks()
{
#if defined(_MSC_VER)
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);

	// Use _CrtSetBreakAlloc(n) to find a specific memory leak
	// Remember to "Enable Windows Debug Heap Allocator" in the debug options on MSVC2015. Without it
	// enabled the memory allocation numbers shifts randomly from one execution to another making it
	// impossible to predict the correct number for a specific allocation.
	//_CrtSetBreakAlloc(924);

#endif
}

// This class is just to verify that releasing the engine as part of the cleanup
// of global variables doesn't cause crashes due to out-of-order cleanup with
// internally shared objects in the engine.
class EngineDestroyer
{
public:
	EngineDestroyer() { en = 0; }
	~EngineDestroyer() { if( en ) en->Release(); }
	asIScriptEngine *en;
} g_engine;

//----------------------------------
// Test with these flags as well
//
// + AS_MAX_PORTABILITY
//----------------------------------

int allTests()
{
	DetectMemoryLeaks();

	PRINTF("AngelScript version: %s\n", asGetLibraryVersion());
	PRINTF("AngelScript options: %s\n", asGetLibraryOptions());

#ifndef AS_MAX_PORTABILITY
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		PRINTF("Compile the tests with AS_MAX_PORTABILITY defined\n");
		goto failed;
	}
#endif

#ifdef __dreamcast__
	fs_chdir(asTestDir);
#endif

	InstallMemoryManager();

	if( Test_Addon_ScriptFile::Test()    ) goto failed; else PRINTF("-- Test_Addon_ScriptFile passed\n");
	if( Test_Addon_ContextMgr::Test()    ) goto failed; else PRINTF("-- Test_Addon_ContextMgr passed\n");
	if( Test_Addon_ScriptGrid::Test()    ) goto failed; else PRINTF("-- Test_Addon_ScriptGrid passed\n");
	if( Test_Addon_WeakRef::Test()       ) goto failed; else PRINTF("-- Test_Addon_WeakRef passed\n");
	if( Test_Addon_Debugger::Test()      ) goto failed; else PRINTF("-- Test_Addon_Debugger passed\n");
	if( Test_Addon_ScriptBuilder::Test() ) goto failed; else PRINTF("-- Test_Addon_ScriptBuilder passed\n");
	if( Test_Addon_ScriptMath::Test()    ) goto failed; else PRINTF("-- Test_Addon_ScriptMath passed\n");
	if( Test_Addon_Serializer::Test()    ) goto failed; else PRINTF("-- Test_Addon_Serializer passed\n");
	if( Test_Addon_ScriptHandle::Test()  ) goto failed; else PRINTF("-- Test_Addon_ScriptHandle passed\n");
	if( Test_Addon_ScriptArray::Test()   ) goto failed; else PRINTF("-- Test_Addon_ScriptArray passed\n");
	if( Test_Addon_Dictionary::Test()    ) goto failed; else PRINTF("-- Test_Addon_Dictionary passed\n");
	if( Test_Addon_DateTime::Test()      ) goto failed; else PRINTF("-- Test_Addon_DateTime passed\n");
	if( Test_Addon_StdString::Test()     ) goto failed; else PRINTF("-- Test_Addon_StdString passed\n");

	if( TestComposition::Test()         ) goto failed; else PRINTF("-- TestComposition passed\n");
	if( TestPropIntegerDivision::Test() ) goto failed; else PRINTF("-- TestPropIntegerDivision passed\n");
	if( TestThisCallMethod_ConfigErrors::Test() ) goto failed; else PRINTF("-- TestThisCallMethod_ConfigErrors passed\n");
	if( TestAuto::Test()                ) goto failed; else PRINTF("-- TestAuto passed\n");
	if( TestPow::Test()                 ) goto failed; else PRINTF("-- TestPow passed\n");
	if( TestMixin::Test()               ) goto failed; else PRINTF("-- TestMixin passed\n");
	if( TestNamespace::Test()           ) goto failed; else PRINTF("-- TestNamespace passed\n");
	if( TestShared::Test()              ) goto failed; else PRINTF("-- TestShared passed\n");
	if( TestDefaultArg::Test()          ) goto failed; else PRINTF("-- TestDefaultArg passed\n");
	if( TestNamedArgs::Test()           ) goto failed; else PRINTF("-- TestNamedArgs passed\n");
	if( TestScriptRetRef::Test()        ) goto failed; else PRINTF("-- TestScriptRetRef passed\n");
	if( TestGarbageCollect::Test()      ) goto failed; else PRINTF("-- TestGarbageCollect passed\n");
	if( TestFunctionPtr::Test()         ) goto failed; else PRINTF("-- TestFunctionPtr passed\n");
	if( TestModule::Test()              ) goto failed; else PRINTF("-- TestModule passed\n");
	if( TestGetSet::Test()              ) goto failed; else PRINTF("-- TestGetSet passed\n");
	if( TestOperator::Test()            ) goto failed; else PRINTF("-- TestOperator passed\n");
	if( TestTemplate::Test()            ) goto failed; else PRINTF("-- TestTemplate passed\n");
	if( TestDump::Test()                ) goto failed; else PRINTF("-- TestDump passed\n");
	if( TestInheritance::Test()         ) goto failed; else PRINTF("-- TestInheritance passed\n");
	if( TestScriptClassMethod::Test()   ) goto failed; else PRINTF("-- TestScriptClassMethod passed\n");
	if( TestScriptString::Test()        ) goto failed; else PRINTF("-- TestScriptString passed\n");
	if( TestSaveLoad::Test()            ) goto failed; else PRINTF("-- TestSaveLoad passed\n");
	if( TestInterface::Test()           ) goto failed; else PRINTF("-- TestInterface passed\n");
	if( TestCastOp::Test()              ) goto failed; else PRINTF("-- TestCastOp passed\n");
	if( Test2Modules()                  ) goto failed; else PRINTF("-- Test2Modules passed\n");
	if( TestArrayObject::Test()         ) goto failed; else PRINTF("-- TestArrayObject passed\n");
	if( TestCompiler::Test()            ) goto failed; else PRINTF("-- TestCompiler passed\n");
	if( TestOptimize()                  ) goto failed; else PRINTF("-- TestOptimize passed\n");
	if( TestConversion::Test()          ) goto failed; else PRINTF("-- TestConversion passed\n");
	if( TestRegisterType::Test()        ) goto failed; else PRINTF("-- TestRegisterType passed\n");
	if( TestRefArgument::Test()         ) goto failed; else PRINTF("-- TestRefArgument passed\n");
	if( TestStream::Test()              ) goto failed; else PRINTF("-- TestStream passed\n");
	if( TestEnum::Test()                ) goto failed; else PRINTF("-- TestEnum passed\n");
	if( TestDynamicConfig::Test()       ) goto failed; else PRINTF("-- TestDynamicConfig passed\n");
	if( TestObjHandle::Test()           ) goto failed; else PRINTF("-- TestObjHandle passed\n");
	if( TestGlobalVar()                 ) goto failed; else PRINTF("-- TestGlobalVar passed\n");
	if( TestScriptStruct::Test()        ) goto failed; else PRINTF("-- TestScriptStruct passed\n");
	if( TestRZ::Test()                  ) goto failed; else PRINTF("-- TestRZ passed\n");
	if( TestArray::Test()               ) goto failed; else PRINTF("-- TestArray passed\n");
	if( TestAny::Test()                 ) goto failed; else PRINTF("-- TestAny passed\n");
	if( TestObjHandle2::Test()          ) goto failed; else PRINTF("-- TestObjHandle2 passed\n");
	if( TestVector3()                   ) goto failed; else PRINTF("-- TestVector3 passed\n");
	if( TestConstObject::Test()         ) goto failed; else PRINTF("-- TestConstObject passed\n");
	if( TestImplicitCast::Test()        ) goto failed; else PRINTF("-- TestImplicitCast passed\n");
	if( TestImplicitHandle::Test()      ) goto failed; else PRINTF("-- TestImplicitHandle passed\n");
	if( TestFor::Test()                 ) goto failed; else PRINTF("-- TestFor passed\n");
	if( TestRefCast::Test()             ) goto failed; else PRINTF("-- TestRefCast passed\n");
	if( TestStdString()                 ) goto failed; else PRINTF("-- TestStdString passed\n");
	if( TestStack2::Test()              ) goto failed; else PRINTF("-- TestStack2 passed\n");
	if( TestStdVector::Test()           ) goto failed; else PRINTF("-- TestStdVector passed\n");
	if( TestArrayHandle::Test()         ) goto failed; else PRINTF("-- TestArrayHandle passed\n");
	if( TestDict::Test()                ) goto failed; else PRINTF("-- TestDict passed\n");
	if( TestMultiAssign::Test()         ) goto failed; else PRINTF("-- TestMultiAssign passed\n");
	if( TestException::Test()           ) goto failed; else PRINTF("-- TestException passed\n");
	if( TestInt8::Test()                ) goto failed; else PRINTF("-- TestInt8 passed\n");
	if( TestGeneric::Test()             ) goto failed; else PRINTF("-- TestGeneric passed\n");
	if( TestBStr()                      ) goto failed; else PRINTF("-- TestBStr passed\n");
	if( TestTypedef::Test()             ) goto failed; else PRINTF("-- TestTypedef passed\n");
	if( TestImport::Test()              ) goto failed; else PRINTF("-- TestImport passed\n");
	if( TestAssign::Test()              ) goto failed; else PRINTF("-- TestAssign passed\n");
	if( TestSwitch()                    ) goto failed; else PRINTF("-- TestSwitch passed\n");
	if( TestExceptionMemory::Test()     ) goto failed; else PRINTF("-- TestExceptionMemory passed\n");
	if( TestObject::Test()              ) goto failed; else PRINTF("-- TestObject passed\n");
	if( TestFactory::Test()             ) goto failed; else PRINTF("-- TestFactory passed\n");
	if( TestFuncOverload()              ) goto failed; else PRINTF("-- TestFuncOverload passed\n");
	if( TestObjZeroSize::Test()         ) goto failed; else PRINTF("-- TestObjZeroSize passed\n");
	if( TestSingleton::Test()           ) goto failed; else PRINTF("-- TestSingleton passed\n");
	if( TestCondition()                 ) goto failed; else PRINTF("-- TestCondition passed\n");
	if( TestObject2::Test()             ) goto failed; else PRINTF("-- TestObject2 passed\n");
	if( TestShark::Test()               ) goto failed; else PRINTF("-- TestShark passed\n");
	if( TestBool::Test()                ) goto failed; else PRINTF("-- TestBool passed\n");
	if( TestBits::Test()                ) goto failed; else PRINTF("-- TestBits passed\n");
	if( TestDestructor::Test()          ) goto failed; else PRINTF("-- TestDestructor passed\n");
	if( TestConstructor2::Test()        ) goto failed; else PRINTF("-- TestConstructor2 passed\n");
	if( TestUnsafeRef::Test()           ) goto failed; else PRINTF("-- TestUnsafeRef passed\n");
	if( TestVarType::Test()             ) goto failed; else PRINTF("-- TestVarType passed\n");
	if( TestScriptMath::Test()          ) goto failed; else PRINTF("-- TestScriptMath passed\n");
	if( TestDebug::Test()               ) goto failed; else PRINTF("-- TestDebug passed\n");
	if( TestGetArgPtr::Test()           ) goto failed; else PRINTF("-- TestGetArgPtr passed\n");
	if( TestAutoHandle::Test()          ) goto failed; else PRINTF("-- TestAutoHandle passed\n");
	if( TestObject3::Test()             ) goto failed; else PRINTF("-- TestObject3 passed\n");
	if( TestArrayIntf::Test()           ) goto failed; else PRINTF("-- TestArrayIntf passed\n");
	if( TestConstProperty::Test()       ) goto failed; else PRINTF("-- TestConstProperty passed\n");
	if( TestSuspend::Test()             ) goto failed; else PRINTF("-- TestSuspend passed\n");
	if( TestVector3_2::Test()           ) goto failed; else PRINTF("-- TestVector3_2 passed\n");
	if( TestNested()                    ) goto failed; else PRINTF("-- TestNested passed\n");
	if( TestConstructor()               ) goto failed; else PRINTF("-- TestConstructor passed\n");
	if( TestExecuteScript()             ) goto failed; else PRINTF("-- TestExecuteScript passed\n");
	if( TestCustomMem::Test()           ) goto failed; else PRINTF("-- TestCustomMem passed\n");
	if( TestPostProcess::Test()         ) goto failed; else PRINTF("-- TestPostProcess passed\n");
	if( TestArgRef::Test()              ) goto failed; else PRINTF("-- TestArgRef passed\n");
	if( TestNotInitialized()            ) goto failed; else PRINTF("-- TestNotInitialized passed\n");
	if( TestConfig::Test()              ) goto failed; else PRINTF("-- TestConfig passed\n");
	if( TestInt64()                     ) goto failed; else PRINTF("-- TestInt64 passed\n");
	if( TestImport2::Test()             ) goto failed; else PRINTF("-- TestImport2 passed\n");
	if( TestEnumGlobVar()               ) goto failed; else PRINTF("-- TestEnumGlobVar passed\n");
	if( TestConfigAccess::Test()        ) goto failed; else PRINTF("-- TestConfigAccess passed\n");
	if( TestDiscard::Test()             ) goto failed; else PRINTF("-- TestDiscard passed\n");
	if( TestParser::Test()              ) goto failed; else PRINTF("-- TestParser passed\n");
	if( TestFloat::Test()               ) goto failed; else PRINTF("-- TestFloat passed\n");
	if( TestTempVar()                   ) goto failed; else PRINTF("-- TestTempVar passed\n");
	if( TestModuleRef()                 ) goto failed; else PRINTF("-- TestModuleRef passed\n");
	if( TestExecuteString()             ) goto failed; else PRINTF("-- TestExecuteString passed\n");
	if( TestStack()                     ) goto failed; else PRINTF("-- TestStack passed\n");
	if( TestCreateEngine()              ) goto failed; else PRINTF("-- TestCreateEngine passed\n");
	if( TestLongToken()                 ) goto failed; else PRINTF("-- TestLongToken passed\n");
	if( TestOutput::Test()              ) goto failed; else PRINTF("-- TestOutput passed\n");
	if( Test2Func::Test()               ) goto failed; else PRINTF("-- Test2Func passed\n");
	if( TestCircularImport::Test()      ) goto failed; else PRINTF("-- TestCircularImport passed\n");
	if( TestNeverVisited()              ) goto failed; else PRINTF("-- TestNeverVisited passed\n");
	if( TestReturnString::Test()        ) goto failed; else PRINTF("-- TestReturnString passed\n");
	if( TestNegateOperator()            ) goto failed; else PRINTF("-- TestNegateOperator passed\n");

	// The following tests are designed specifically to test the native calling conventions.
	// These are grouped by calling convention and ordered in increasing complexity.
	{
		// cdecl
		if( TestExecute()                 ) goto failed; else PRINTF("-- TestExecute passed\n");
		if( TestCDeclReturn::Test()       ) goto failed; else PRINTF("-- TestCDeclReturn passed\n");
		if( TestExecute1Arg()             ) goto failed; else PRINTF("-- TestExecute1Arg passed\n");
		if( TestExecute2Args()            ) goto failed; else PRINTF("-- TestExecute2Args passed\n");
		if( TestExecute4Args()            ) goto failed; else PRINTF("-- TestExecute4Args passed\n");
		if( TestExecute4Argsf()           ) goto failed; else PRINTF("-- TestExecute4Argsf passed\n");
		if( TestExecuteMixedArgs()        ) goto failed; else PRINTF("-- TestExecuteMixedArgs passed\n");
		if( TestExecute32Args()           ) goto failed; else PRINTF("-- TestExecute32Args passed\n");
		if( TestExecute32MixedArgs()      ) goto failed; else PRINTF("-- TestExecute32MixedArgs passed\n");
		if( TestCDecl_Class()             ) goto failed; else PRINTF("-- TestCDecl_Class passed\n");
		if( TestCDecl_ClassA()            ) goto failed; else PRINTF("-- TestCDecl_ClassA passed\n");
		if( TestCDecl_ClassC()            ) goto failed; else PRINTF("-- TestCDecl_ClassC passed\n");
		if( TestCDecl_ClassD()            ) goto failed; else PRINTF("-- TestCDecl_ClassD passed\n");
		if( TestCDecl_ClassK()            ) goto failed; else PRINTF("-- TestCDecl_ClassK passed\n");

		// cdecl_objlast and cdecl_objfirst
		if( TestCDeclObjLast::Test()      ) goto failed; else PRINTF("-- TestCDeclObjLast passed\n");
		if( TestReturnWithCDeclObjFirst() ) goto failed; else PRINTF("-- TestReturnWithCDeclObjFirst passed\n");

		// thiscall
		if( TestExecuteThis32MixedArgs()  ) goto failed; else PRINTF("-- TestExecuteThis32MixedArgs passed\n");
		if( TestThiscallClass()           ) goto failed; else PRINTF("-- TestThiscallClass passed\n");
		if( TestNotComplexThisCall()      ) goto failed; else PRINTF("-- TestNotComplexThisCall passed\n");
		if( TestVirtualMethod()           ) goto failed; else PRINTF("-- TestVirtualMethod passed\n");
		if( TestMultipleInheritance()     ) goto failed; else PRINTF("-- TestMultipleInheritance passed\n");
		if( TestVirtualInheritance()      ) goto failed; else PRINTF("-- TestVirtualInheritance passed\n");

		// thiscall_asglobal
		if( TestThiscallAsGlobal::Test()  ) goto failed; else PRINTF("-- TestThiscallAsGlobal passed\n");

		// thiscall_objlast & objfirst
		if( TestThisCallMethod::Test()    ) goto failed; else PRINTF("-- TestThisCallMethod passed\n");

		// stdcall
		if( TestStdcall4Args()            ) goto failed; else PRINTF("-- TestStdcall4Args passed\n");
		if( TestNotComplexStdcall()       ) goto failed; else PRINTF("-- TestNotComplexStdcall passed\n");

		// Default functions
		if( Test_Native_DefaultFunc::Test() ) goto failed; else PRINTF("-- Test_Native_DefaultFunc passed\n");
	}

	// This test uses ATL::CString thus it is turned off by default
//	if( TestCString::Test()           ) goto failed; else PRINTF("-- TestCString passed\n");
	// Pointers are not supported by AngelScript at the moment, but they may be in the future
//	if( TestPointer::Test()           ) goto failed; else PRINTF("-- TestPointer passed\n");

	RemoveMemoryManager();

	// Populate the global engine destroyer after the memory manager has
	// been removed so we don't get false positives for memory leaks.
	g_engine.en = asCreateScriptEngine(ANGELSCRIPT_VERSION);

//succeed:
	PRINTF("--------------------------------------------\n");
	PRINTF("All of the tests passed with success.\n\n");
#if !defined(DONT_WAIT) && !defined(_M_ARM) && (defined(WIN32) || defined(_WIN64))
	PRINTF("Press any key to quit.\n");
	while(!_getch());
#endif
	return 0;

failed:
	PRINTF("--------------------------------------------\n");
	PRINTF("One of the tests failed, see details above.\n\n");
#if !defined(DONT_WAIT) && !defined(_M_ARM) && (defined(WIN32) || defined(_WIN64))
	PRINTF("Press any key to quit.\n");
	while(!_getch());
#endif
	return -1;
}

#if !defined(IS_IOS) && !defined(IS_ANDROID)
// ios and android has their entry points.
// For iOS call allTests() from application entry point.
int main(int /* argc */, char ** /* argv */)
{
	return allTests();
}
#endif

#ifdef IS_ANDROID
// Android entry function (called from java of android test project)
#include <jni.h>

extern "C" {
	void Java_com_angelcode_android_AndroidTests_runTests( JNIEnv* env, jobject thiz )
	{
		allTests();
	}
}
#endif

