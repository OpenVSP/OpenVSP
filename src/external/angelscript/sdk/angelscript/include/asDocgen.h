//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// asDocgen.h
//
// AngelScript documentation generator
// Released into the public domain by the author on this website:
// http://www.gamedev.net/topic/629517-document-generation-for-registered-interface/
//
// Modified by Rob McDonald & Justin Gravett (ESAero)
//

#ifndef asdocgen_h
#define asdocgen_h

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace asDocgen
{
    using std::string;
    using std::cout;
    using std::endl;
    using std::ios_base;

    class ScriptTypeComment
    {
    public:

        string comment;

        std::map<string, string> members;
    };

    std::map<string, ScriptTypeComment> typeComments;
    std::map<string, string> globalComments;
    std::map<string, std::map<string, string>> enumeratorComments;
    std::map<string, string> enumerationComment;
    std::map<string, string> globalPropertyComments;

    std::map<string, string> skippedComments;
    std::map<string, string> groupTitles;
    std::map<string, string> groupDescriptions;

    std::map<string, ScriptTypeComment> typeGroups;
    std::map<string, string> globalGroups;
    std::map<string, std::map<string, string>> enumeratorGroups;
    std::map<string, string> enumerationGroups;
    std::map<string, string> globalPropertyGroups;

    void AddTypeComment( const string& name, const string& comment )
    {
        if ( typeComments.count( name ) )
        {
            typeComments[name].comment = comment;
        }
        else
        {
            typeComments[name] = ScriptTypeComment();
            typeComments[name].comment = comment;
        }
    }

    void AddTypeMemberComment( const string& name, const string& member, const string& comment )
    {
        if ( typeComments.count( name ) )
        {
            typeComments[name].members[member] = comment;
        }
        else
        {
            typeComments[name].comment = "// TODO, add comment";
            typeComments[name] = ScriptTypeComment();
            typeComments[name].members[member] = comment;
        }
    }

    void AddGlobalFunctionComment( const string& decl, const string& comment )
    {
        globalComments[decl] = comment;
    }

    void AddGlobalPropertyComment( const string& decl, const string& comment )
    {
        globalPropertyComments[decl] = comment;
    }

    void AddEnumerationComment( const string& decl, const string& comment )
    {
        enumerationComment[decl] = comment;
    }

    void AddEnumeratorComment( const string& enumname, const string& enummember, const string& comment )
    {
        if ( enumeratorComments.count( enumname ) )
        {
            enumeratorComments[enumname][enummember] = comment;
        }
        else
        {
            enumeratorComments[enumname] = std::map<string, string>();
            enumeratorComments[enumname][enummember] = comment;
        }
    }

    void AddSkippedComment( const string& decl, const string& comment )
    {
        skippedComments[decl] = comment;
    }

    void AddGroup( const string& group, const string& title, const string& description )
    {
        groupTitles[group] = title;
        groupDescriptions[group] = description;
    }

    void AddTypeGroup( const string& name, const string& group )
    {
        if ( typeGroups.count( name ) )
        {
            typeGroups[name].comment = group;
        }
        else
        {
            typeGroups[name] = ScriptTypeComment();
            typeGroups[name].comment = group;
        }
    }

    void AddTypeMemberGroup( const string& name, const string& member, const string& group )
    {
        if ( typeGroups.count( name ) )
        {
            typeGroups[name].members[member] = group;
        }
        else
        {
            typeGroups[name].comment = "// TODO, add comment";
            typeGroups[name] = ScriptTypeComment();
            typeGroups[name].members[member] = group;
        }
    }

    void AddGlobalFunctionGroup( const string& decl, const string& group )
    {
        globalGroups[decl] = group;
    }

    void AddGlobalPropertyGroup( const string& decl, const string& group )
    {
        globalPropertyGroups[decl] = group;
    }

    void AddEnumerationGroup( const string& decl, const string& group )
    {
        enumerationGroups[decl] = group;
    }

    void AddEnumeratorGroup( const string& enumname, const string& enummember, const string& group )
    {
        if ( enumeratorGroups.count( enumname ) )
        {
            enumeratorGroups[enumname][enummember] = group;
        }
        else
        {
            enumeratorGroups[enumname] = std::map<string, string>();
            enumeratorGroups[enumname][enummember] = group;
        }
    }

    void ClearComments()
    {
        typeComments.clear();
        globalComments.clear();
        enumeratorComments.clear();
        enumerationComment.clear();
        globalPropertyComments.clear();

        skippedComments.clear();
    }

    void ClearGroups()
    {
        typeGroups.clear();
        globalGroups.clear();
        enumeratorGroups.clear();
        enumerationGroups.clear();
        globalPropertyGroups.clear();
    }

    void replaceSubStr( std::string& str, const std::string& from, const std::string& to )
    {
        size_t start_pos = 0;
        while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos )
        {
            str.replace( start_pos, from.length(), to );
            start_pos += to.length();
        }
    }

    void ProcessFunctionDecleration( std::string& str )
    {
        replaceSubStr( str, "opIndex", "operator[]" );
        replaceSubStr( str, "opAddAssign", "operator+=" );
        replaceSubStr( str, "opSubAssign", "operator-=" );
        replaceSubStr( str, "opMulAssign", "operator*=" );
        replaceSubStr( str, "opDivAssign", "operator/=" );
        replaceSubStr( str, "opModAssign", "operator%=" );
        replaceSubStr( str, "opAndAssign", "operator&=" );
        replaceSubStr( str, "opOrAssign", "operator|=" );
        replaceSubStr( str, "opXorAssign", "operator^=" );
        replaceSubStr( str, "opShlAssign", "operator<<=" );
        replaceSubStr( str, "opShrAssign", "operator>>=" );
        replaceSubStr( str, "opUShrAssign", "operator>>>=" );
        replaceSubStr( str, "opAssign", "operator=" );
        replaceSubStr( str, "opAdd_r", "operator+" );
        replaceSubStr( str, "opAdd", "operator+" );
        replaceSubStr( str, "opSub_r", "operator-" );
        replaceSubStr( str, "opSub", "operator-" );
        replaceSubStr( str, "opMul_r", "operator*" );
        replaceSubStr( str, "opMul", "operator*" );
        replaceSubStr( str, "opDiv_r", "operator/" );
        replaceSubStr( str, "opDiv", "operator/" );
        replaceSubStr( str, "opMod_r", "operator%" );
        replaceSubStr( str, "opMod", "operator%" );
        replaceSubStr( str, "opAnd_r", "operator&" );
        replaceSubStr( str, "opAnd", "operator&" );
        replaceSubStr( str, "opOr_r", "operator|" );
        replaceSubStr( str, "opOr", "operator|" );
        replaceSubStr( str, "opXor_r", "operator^" );
        replaceSubStr( str, "opXor", "operator^" );
        replaceSubStr( str, "opShl_r", "operator<<" );
        replaceSubStr( str, "opShl", "operator<<" );
        replaceSubStr( str, "opShr_r", "operator>>" );
        replaceSubStr( str, "opShr", "operator>>" );
        replaceSubStr( str, "opUShr_r", "operator>>>" );
        replaceSubStr( str, "opUShr", "operator>>>" );
        replaceSubStr( str, "opCmp", "operator>" ); // TODO, maybe add all 4 operators
        replaceSubStr( str, "opEquals", "operator==" );
    }

};


#endif
