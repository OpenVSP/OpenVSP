#include "utils.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace TestDump
{

void DumpModule(asIScriptModule *mod);

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;

	const char *script = 
		"void Test() {} \n"
		"class A : I { void i(float) {} void a(int) {} float f; } \n"
		"class B : A { B(int) {} } \n"
		"interface I { void i(float); } \n"
		"float a; \n"
		"const float aConst = 3.141592f; \n"
		"I@ i; \n"
		"enum E { eval = 0, eval2 = 2 } \n"
		"E e; \n"
		"typedef float real; \n"
		"real pi = 3.14f; \n"
		"import void ImpFunc() from \"mod\"; \n";

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	RegisterScriptArray(engine, true);
	RegisterStdString(engine);

	float f;
	engine->RegisterTypedef("myFloat", "float");
	engine->RegisterGlobalProperty("myFloat f", &f);
	engine->RegisterGlobalProperty("const float myConst", &f);
	engine->RegisterGlobalFunction("void func(int &in)", asFUNCTION(0), asCALL_GENERIC);

	engine->BeginConfigGroup("test");
	engine->RegisterGlobalFunction("void func2(const string &in = \"\")", asFUNCTION(0), asCALL_GENERIC);
	engine->EndConfigGroup();

	engine->RegisterEnum("myEnum");
	engine->RegisterEnumValue("myEnum", "value1", 1);
	engine->RegisterEnumValue("myEnum", "value2", 2);

	engine->RegisterFuncdef("void Callback(int a, int b)");

	engine->RegisterInterface("MyIntf");
	engine->RegisterInterfaceMethod("MyIntf", "void func() const");

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	WriteConfigToFile(engine, "AS_DEBUG/config.txt");

	DumpModule(mod);

	// Save/Restore the bytecode and then test again for the loaded bytecode
	CBytecodeStream stream(__FILE__"1");
	mod->SaveByteCode(&stream);

	mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
	mod->LoadByteCode(&stream);

	DumpModule(mod);

	engine->Release();

	return fail;
}

void DumpObjectType(stringstream &s, asITypeInfo *objType)
{
	if( objType->GetFlags() & asOBJ_SCRIPT_OBJECT )
	{
		if( objType->GetSize() ) 
		{
			string inheritance;
			if( objType->GetBaseType() )
				inheritance += objType->GetBaseType()->GetName();

			for( asUINT i = 0; i < objType->GetInterfaceCount(); i++ )
			{
				if( inheritance.length() )
					inheritance += ", ";
				inheritance += objType->GetInterface(i)->GetName();
			}

			s << "type: class " << objType->GetName() << " : " << inheritance << endl;
		}
		else
		{
			s << "type: interface " << objType->GetName() << endl;
		}
	}
	else
	{
		s << "reg type: ";
		if( objType->GetFlags() & asOBJ_REF )
			s << "ref ";
		else
			s << "val ";

		s << objType->GetName();

		const char *group = objType->GetConfigGroup();
		s << " group: " << (group ? group : "<null>") << endl;
	}

	// Show factory functions
	for( asUINT f = 0; f < objType->GetFactoryCount(); f++ )
	{
		s << " " << objType->GetFactoryByIndex(f)->GetDeclaration() << endl;
	}

	if( !( objType->GetFlags() & asOBJ_SCRIPT_OBJECT ) )
	{
		// Show behaviours
		for( asUINT b = 0; b < objType->GetBehaviourCount(); b++ )
		{
			asEBehaviours beh;
			asIScriptFunction *bid = objType->GetBehaviourByIndex(b, &beh);
			s << " beh(" << beh << ") " << bid->GetDeclaration(false) << endl;
		}
	}

	// Show methods
	for( asUINT m = 0; m < objType->GetMethodCount(); m++ )
	{
		s << " " << objType->GetMethodByIndex(m)->GetDeclaration(false) << endl;
	}

	// Show properties
	for( asUINT p = 0; p < objType->GetPropertyCount(); p++ )
	{
		s << " " << objType->GetPropertyDeclaration(p) << endl;
	}
}

void DumpModule(asIScriptModule *mod)
{
	int c, n;
	asIScriptEngine *engine = mod->GetEngine();
	stringstream s;

	// Enumerate global functions
	c = mod->GetFunctionCount();
	for( n = 0; n < c; n++ )
	{
		asIScriptFunction *func = mod->GetFunctionByIndex(n);
		s << "func: " << func->GetDeclaration() << endl;
	}

	// Enumerate object types
	c = mod->GetObjectTypeCount();
	for( n = 0; n < c; n++ )
	{
		DumpObjectType(s, mod->GetObjectTypeByIndex(n));
	}

	// Enumerate global variables
	c = mod->GetGlobalVarCount();
	for( n = 0; n < c; n++ )
	{
		s << "global: " << mod->GetGlobalVarDeclaration(n) << endl;
	}

	// Enumerate enums
	c = mod->GetEnumCount();
	for( n = 0; n < c; n++ )
	{
		asITypeInfo *ti = mod->GetEnumByIndex(n);

		s << "enum: " << ti->GetName() << endl;

		// List enum values
		for( asUINT e = 0; e < ti->GetEnumValueCount(); e++ )
		{
			int value;
			const char *name = ti->GetEnumValueByIndex(e, &value);
			s << " " << name << " = " << value << endl;
		}
	}

	// Enumerate type defs
	c = mod->GetTypedefCount();
	for( n = 0; n < c; n++ )
	{
		asITypeInfo *ti = mod->GetTypedefByIndex(n);

		s << "typedef: " << ti->GetName() << " => " << engine->GetTypeDeclaration(ti->GetTypedefTypeId(), true) << endl;
	}

	// Enumerate imported functions
	c = mod->GetImportedFunctionCount();
	for( n = 0; n < c; n++ )
	{
		s << "import: " << mod->GetImportedFunctionDeclaration(n) << " from \"" << mod->GetImportedFunctionSourceModule(n) << "\"" << endl;
	}

	s << "-------" << endl;

	// Enumerate registered global properties
	c = engine->GetGlobalPropertyCount();
	for( n = 0; n < c; n++ )
	{
		const char *name, *nameSpace;
		int typeId;
		bool isConst;
		const char *group;
		engine->GetGlobalPropertyByIndex(n, &name, &nameSpace, &typeId, &isConst, &group);
		s << "reg prop: ";
		if( isConst ) 
			s << "const ";
		s << engine->GetTypeDeclaration(typeId, true) << " " << name;
		s << " group: " << (group ? group : "<null>") << endl;
	}

	// Enumerate registered typedefs
	c = engine->GetTypedefCount();
	for( n = 0; n < c; n++ )
	{
		asITypeInfo *ti = engine->GetTypedefByIndex(n);
		s << "reg typedef: " << ti->GetName() << " => " << engine->GetTypeDeclaration(ti->GetTypedefTypeId(), true) << endl;
	}

	// Enumerate registered global functions
	c = engine->GetGlobalFunctionCount();
	for( n = 0; n < c; n++ )
	{
		asIScriptFunction *func = engine->GetGlobalFunctionByIndex(n);
		const char *group = func->GetConfigGroup();
		s << "reg func: " << func->GetDeclaration() << 
			" group: " << (group ? group : "<null>") << endl;
	}

	// Enumerate registered enums
	c = engine->GetEnumCount();
	for( n = 0; n < c; n++ )
	{
		asITypeInfo *ti = engine->GetEnumByIndex(n);

		s << "reg enum: " << ti->GetName() << endl;

		// List enum values
		for( asUINT e = 0; e < ti->GetEnumValueCount(); e++ )
		{
			int value;
			const char *name = ti->GetEnumValueByIndex(e, &value);
			s << " " << name << " = " << value << endl;
		}
	}

	// Enumerate registered func defs
	c = engine->GetFuncdefCount();
	for( n = 0; n < c; n++ )
	{
		asITypeInfo *funcdef = engine->GetFuncdefByIndex(n);

		s << "reg funcdef: " << funcdef->GetFuncdefSignature()->GetDeclaration() << endl;
	}

	// Get the string factory return type
	int typeId = engine->GetStringFactoryReturnTypeId();
	s << "string factory: " << engine->GetTypeDeclaration(typeId) << endl;

	// Enumerate registered types
	c = engine->GetObjectTypeCount();
	for( n = 0; n < c; n++ )
	{
		DumpObjectType(s, engine->GetObjectTypeByIndex(n));
	}

	//--------------------------------
	// Validate the dump
	if( s.str() != 
		"func: void Test()\n"
		"type: class A : I\n"
		" A@ A()\n"
		" void i(float)\n"
		" void a(int)\n"
		" float f\n"
		"type: class B : A, I\n"
		" B@ B(int)\n"
		" void i(float)\n"
		" void a(int)\n"
		" float f\n"
		"type: interface I\n"
		" void i(float)\n"
		"global: float a\n"
		"global: const float aConst\n"
		"global: E e\n"
		"global: float pi\n"
		"global: I@ i\n"
		"enum: E\n"
		" eval = 0\n"
		" eval2 = 2\n"
		"typedef: real => float\n"
		"import: void ImpFunc() from \"mod\"\n"
		"-------\n"
		"reg prop: float f group: <null>\n"
		"reg prop: const float myConst group: <null>\n"
		"reg typedef: myFloat => float\n"
		"reg func: string formatInt(int64, const string&in = \"\", uint = 0) group: <null>\n"
		"reg func: string formatUInt(uint64, const string&in = \"\", uint = 0) group: <null>\n"
		"reg func: string formatFloat(double, const string&in = \"\", uint = 0, uint = 0) group: <null>\n"
		"reg func: int64 parseInt(const string&in, uint = 10, uint&out = 0) group: <null>\n"
		"reg func: uint64 parseUInt(const string&in, uint = 10, uint&out = 0) group: <null>\n"
		"reg func: double parseFloat(const string&in, uint&out = 0) group: <null>\n"
		"reg func: void func(int&in) group: <null>\n"
		"reg func: void func2(const string&in = \"\") group: test\n"
		"reg enum: myEnum\n"
		" value1 = 1\n"
		" value2 = 2\n"
		"reg funcdef: bool array::less(const T&in, const T&in)\n"
		"reg funcdef: void Callback(int, int)\n"
		"string factory: string\n"
		"reg type: ref array group: <null>\n"
		" T[]@ array(int&in)\n"
		" T[]@ array(int&in, uint)\n"
		" T[]@ array(int&in, uint, const T&in)\n"
		" beh(5) void $beh5()\n"
		" beh(6) void $beh6()\n"
		" beh(9) int $beh9()\n"
		" beh(10) void $beh10()\n"
		" beh(11) bool $beh11()\n"
		" beh(12) void $beh12(int&in)\n"
		" beh(13) void $beh13(int&in)\n"
		" beh(8) bool $beh8(int&in, bool&out)\n"
		" beh(4) T[]@ $list(int&in, int&in) { repeat T }\n"
		" T& opIndex(uint)\n"
		" const T& opIndex(uint) const\n"
		" T[]& opAssign(const T[]&in)\n"
		" void insertAt(uint, const T&in)\n"
		" void insertAt(uint, const T[]&inout)\n"
		" void insertLast(const T&in)\n"
		" void removeAt(uint)\n"
		" void removeLast()\n"
		" void removeRange(uint, uint)\n"
		" uint length() const\n"
		" void reserve(uint)\n"
		" void resize(uint)\n"
		" void sortAsc()\n"
		" void sortAsc(uint, uint)\n"
		" void sortDesc()\n"
		" void sortDesc(uint, uint)\n"
		" void reverse()\n"
		" int find(const T&in) const\n"
		" int find(uint, const T&in) const\n"
		" int findByRef(const T&in) const\n"
		" int findByRef(uint, const T&in) const\n"
		" bool opEquals(const T[]&in) const\n"
		" bool isEmpty() const\n"
		" void sort(array::less&in, uint = 0, uint = uint ( - 1 ))\n"
		"reg type: val string group: <null>\n"
		" beh(2) ~string()\n"
		" beh(0) string()\n"
		" beh(0) string(const string&in)\n"
		" string& opAssign(const string&in)\n"
		" string& opAddAssign(const string&in)\n"
		" bool opEquals(const string&in) const\n"
		" int opCmp(const string&in) const\n"
		" string opAdd(const string&in) const\n"
		" uint length() const\n"
		" void resize(uint)\n"
		" bool isEmpty() const\n"
		" uint8& opIndex(uint)\n"
		" const uint8& opIndex(uint) const\n"
		" string& opAssign(double)\n"
		" string& opAddAssign(double)\n"
		" string opAdd(double) const\n"
		" string opAdd_r(double) const\n"
		" string& opAssign(float)\n"
		" string& opAddAssign(float)\n"
		" string opAdd(float) const\n"
		" string opAdd_r(float) const\n"
		" string& opAssign(int64)\n"
		" string& opAddAssign(int64)\n"
		" string opAdd(int64) const\n"
		" string opAdd_r(int64) const\n"
		" string& opAssign(uint64)\n"
		" string& opAddAssign(uint64)\n"
		" string opAdd(uint64) const\n"
		" string opAdd_r(uint64) const\n" 
		" string& opAssign(bool)\n"
		" string& opAddAssign(bool)\n"
		" string opAdd(bool) const\n"
		" string opAdd_r(bool) const\n"
		" string substr(uint = 0, int = - 1) const\n"
		" int findFirst(const string&in, uint = 0) const\n"
		" int findFirstOf(const string&in, uint = 0) const\n"
		" int findFirstNotOf(const string&in, uint = 0) const\n"
		" int findLast(const string&in, int = - 1) const\n"
		" int findLastOf(const string&in, int = - 1) const\n"
		" int findLastNotOf(const string&in, int = - 1) const\n"
		" void insert(uint, const string&in)\n"
		" void erase(uint, int = - 1)\n"
		"type: interface MyIntf\n"
		" void func() const\n" )
	{
		cout << s.str() << endl;
		cout << "Failed to get the expected result when dumping the module" << endl << endl;
	}
}


} // namespace

