

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

class VspSurf;

class STEPutil
{

public:
    STEPutil();
    virtual ~STEPutil();

    void  WriteFile( string fname );
    SdaiCartesian_point * MakePoint( const double & x, const double & y, const double & z );
    void AddSurf( VspSurf *s );

    enum LenEnum { u_MM, u_CM, u_M, u_IN, u_FT, u_YD };
    enum AngEnum { u_RAD, u_DEG };

    Registry * registry;
    InstMgr * instance_list;

protected:

    STEPfile * sfile;
    InstMgr * header_instances;

    STEPcomplex * context;
    SdaiShape_representation * shape_rep;

    STEPcomplex * Geometric_Context( const LenEnum & len, const AngEnum & angle, const char * tolstr );


    SdaiDirection * MakeDirection( const double & x, const double & y, const double & z );

    SdaiAxis2_placement_3d * DefaultAxis( );

    SdaiDate_and_time * DateTime( );

    SdaiSecurity_classification * Classification( SdaiPerson_and_organization * per_org, SdaiDate_and_time * date_time, SdaiProduct_definition_formation_with_specified_source * prod_def_form );

    void  STEPBoilerplate( );


};




#endif // !defined(STEPUTIL__INCLUDED_)
