

#if !defined(STEPUTIL__INCLUDED_)
#define STEPUTIL__INCLUDED_

#pragma warning( disable : 4018 )
#pragma warning( disable : 4800 )

#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>
#include <STEPcomplex.h>
#include <SdaiHeaderSchema.h>
#include <schema.h>
#include "SdaiCONFIG_CONTROL_DESIGN.h"
#include <string>

#include "APIDefines.h"

class VspSurf;

class STEPutil
{

public:
    STEPutil( const int & len, const double & tol );
    virtual ~STEPutil();

    void  WriteFile( string fname );
    SdaiCartesian_point * MakePoint( const double & x, const double & y, const double & z );
    void AddSurf( VspSurf *s, bool splitsurf, bool mergepts, bool tocubic, double tol, bool trimte, const vector < double > &USplit, const vector < double > &WSplit, string name = "''" );

    Registry * registry;
    InstMgr * instance_list;

protected:

    STEPfile * sfile;
    InstMgr * header_instances;

    STEPcomplex * context;
    SdaiShape_representation * shape_rep;

    STEPcomplex * Geometric_Context( const vsp::LEN_UNITS & len, const vsp::ANG_UNITS & angle, const char * tolstr );


    SdaiDirection * MakeDirection( const double & x, const double & y, const double & z );

    SdaiAxis2_placement_3d * DefaultAxis( );

    SdaiDate_and_time * DateTime( );

    SdaiSecurity_classification * Classification( SdaiPerson_and_organization * per_org, SdaiDate_and_time * date_time, SdaiProduct_definition_formation_with_specified_source * prod_def_form );

    void  STEPBoilerplate( const vsp::LEN_UNITS & len, const char * tolstr );


};

class IGESutil
{

public:
    IGESutil( const int& len );
    virtual ~IGESutil();

protected:


};



#endif // !defined(STEPUTIL__INCLUDED_)
