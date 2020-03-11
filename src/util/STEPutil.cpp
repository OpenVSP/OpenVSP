
#include "STEPutil.h"
#include "VspSurf.h"
#include "main.h"

//===================================================================//
//=================        STEP Functions         ===================//
//===================================================================//

STEPutil::STEPutil( const int & len, const double & tol )
{
    context = NULL;
    shape_rep = NULL;

    // The registry contains information about types present in the current schema; SchemaInit is a function in the schema-specific SDAI library
    registry = new Registry( SchemaInit );

    // The InstMgr holds instances that have been created or that have been loaded from a file
    instance_list = new InstMgr();

    // STEPfile takes care of reading and writing Part 21 files
    sfile = new STEPfile( *registry, *instance_list, "", false );

    // Build file header
    header_instances = sfile->HeaderInstances();

    string tolstr = std::to_string( tol );

    STEPBoilerplate( (vsp::LEN_UNITS) len, tolstr.c_str() );
}

STEPutil::~STEPutil()
{
    header_instances->DeleteInstances();
    instance_list->DeleteInstances();
    delete registry;
    delete instance_list;
    delete sfile;
}


STEPcomplex * STEPutil::Geometric_Context( const vsp::LEN_UNITS & len, const vsp::ANG_UNITS & angle, const char * tolstr )
{
    int instance_cnt = 0;
    STEPattribute * attr;
    STEPcomplex * stepcomplex;

    SdaiDimensional_exponents * dimensional_exp = new SdaiDimensional_exponents();
    dimensional_exp->length_exponent_( 0.0 );
    dimensional_exp->mass_exponent_( 0.0 );
    dimensional_exp->time_exponent_( 0.0 );
    dimensional_exp->electric_current_exponent_( 0.0 );
    dimensional_exp->thermodynamic_temperature_exponent_( 0.0 );
    dimensional_exp->amount_of_substance_exponent_( 0.0 );
    dimensional_exp->luminous_intensity_exponent_( 0.0 );
    instance_list->Append( ( SDAI_Application_instance * ) dimensional_exp, completeSE );
    instance_cnt++;

    STEPcomplex * ua_length;
    // First set up metric units if appropriate.  Default to mm.
    // If imperial units, set up mm to be used as base to define imperial units.
    Si_prefix pfx = Si_prefix__milli;
    switch( len )
    {
    case vsp::LEN_CM:
        pfx = Si_prefix__centi;
        break;
    case vsp::LEN_M:
        pfx = Si_prefix_unset;
        break;
    case vsp::LEN_MM:
    case vsp::LEN_IN:
    case vsp::LEN_FT:
    case vsp::LEN_YD:
    case vsp::LEN_UNITLESS:
        break;
    }

    const char * ua_length_types[4] = { "length_unit", "named_unit", "si_unit", "*" };
    ua_length = new STEPcomplex( registry, ( const char ** ) ua_length_types, instance_cnt );
    stepcomplex = ua_length->head;
    while( stepcomplex )
    {
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "prefix" ) )
                {
                    attr->Raw()->e = new SdaiSi_prefix_var( pfx );
                }
                if( !strcmp( attr->Name(), "name" ) )
                {
                    attr->Raw()->e = new SdaiSi_unit_name_var( Si_unit_name__metre );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_length, completeSE );
    instance_cnt++;

    // If imperial, create conversion based unit.
    if( len >= vsp::LEN_IN )
    {
        STEPcomplex * len_mm = ua_length;

        char lenname[10];
        lenname[0] = '\0';
        double lenconv = 1.0;

        switch( len )
        {
        case vsp::LEN_IN:
            strcat( lenname, "'INCH'\0" );
            lenconv = 25.4;
            break;
        case vsp::LEN_FT:
            strcat( lenname, "'FOOT'\0" );
            lenconv = 25.4 * 12.0;
            break;
        case vsp::LEN_YD:
            strcat( lenname, "'YARD'\0" );
            lenconv = 25.4 * 36.0;
            break;
        case vsp::LEN_MM:
        case vsp::LEN_CM:
        case vsp::LEN_M:
        case vsp::LEN_UNITLESS:
            break;
        }

        SdaiUnit * len_unit = new SdaiUnit( ( SdaiNamed_unit * ) len_mm );

        SdaiMeasure_value * len_measure_value = new SdaiMeasure_value( lenconv, config_control_design::t_measure_value );
        len_measure_value->SetUnderlyingType( config_control_design::t_length_measure );

        SdaiLength_measure_with_unit * len_measure_with_unit = new SdaiLength_measure_with_unit();
        len_measure_with_unit->value_component_( len_measure_value );
        len_measure_with_unit->unit_component_( len_unit );
        instance_list->Append( ( SDAI_Application_instance * ) len_measure_with_unit, completeSE );
        instance_cnt++;

        SdaiDimensional_exponents * dimensional_exp_len = new SdaiDimensional_exponents();
        dimensional_exp_len->length_exponent_( 1.0 );
        dimensional_exp_len->mass_exponent_( 0.0 );
        dimensional_exp_len->time_exponent_( 0.0 );
        dimensional_exp_len->electric_current_exponent_( 0.0 );
        dimensional_exp_len->thermodynamic_temperature_exponent_( 0.0 );
        dimensional_exp_len->amount_of_substance_exponent_( 0.0 );
        dimensional_exp_len->luminous_intensity_exponent_( 0.0 );
        instance_list->Append( ( SDAI_Application_instance * ) dimensional_exp_len, completeSE );
        instance_cnt++;

        const char * ua_conv_len_types[4] = { "conversion_based_unit", "named_unit", "length_unit", "*" };
        ua_length = new STEPcomplex( registry, ( const char ** ) ua_conv_len_types, instance_cnt );
        stepcomplex = ua_length->head;
        while( stepcomplex )
        {
            if( !strcmp( stepcomplex->EntityName(), "Conversion_Based_Unit" ) )
            {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL )
                {
                    if( !strcmp( attr->Name(), "name" ) )
                    {
                        attr->StrToVal( lenname );
                    }
                    if( !strcmp( attr->Name(), "conversion_factor" ) )
                    {
                        attr->Raw()->c = new( STEPentity * );
                        *( attr->Raw()->c ) = ( STEPentity * )( len_measure_with_unit );
                    }
                }
            }
            if( !strcmp( stepcomplex->EntityName(), "Named_Unit" ) )
            {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL )
                {
                    if( !strcmp( attr->Name(), "dimensions" ) )
                    {
                        attr->Raw()->c = new( STEPentity * );
                        *( attr->Raw()->c ) = ( STEPentity * )( dimensional_exp_len );
                    }
                }
            }
            stepcomplex = stepcomplex->sc;
        }

        instance_list->Append( ( SDAI_Application_instance * ) ua_length, completeSE );
        instance_cnt++;
    }

    SdaiUncertainty_measure_with_unit * uncertainty = ( SdaiUncertainty_measure_with_unit * )registry->ObjCreate( "UNCERTAINTY_MEASURE_WITH_UNIT" );
    uncertainty->name_( "'DISTANCE_ACCURACY_VALUE'" );
    uncertainty->description_( "'Threshold below which geometry imperfections (such as overlaps) are not considered errors.'" );
    SdaiUnit * tol_unit = new SdaiUnit( ( SdaiNamed_unit * ) ua_length );
    uncertainty->ResetAttributes();
    {
        while( ( attr = uncertainty->NextAttribute() ) != NULL )
        {
            if( !strcmp( attr->Name(), "unit_component" ) )
            {
                attr->Raw()->sh = tol_unit;
            }
            if( !strcmp( attr->Name(), "value_component" ) )
            {
                attr->StrToVal( tolstr );
            }
            if( !strcmp( attr->Name(), "name" ) )
            {
                attr->StrToVal( "'closure'" );
            }
        }
    }
    instance_list->Append( ( SDAI_Application_instance * ) uncertainty, completeSE );
    instance_cnt++;

    // First set up radians as base angle unit.
    const char * ua_plane_angle_types[4] = { "named_unit", "plane_angle_unit", "si_unit", "*" };
    STEPcomplex * ua_plane_angle = new STEPcomplex( registry, ( const char ** ) ua_plane_angle_types, instance_cnt );
    stepcomplex = ua_plane_angle->head;
    while( stepcomplex )
    {
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "name" ) )
                {
                    attr->Raw()->e = new SdaiSi_unit_name_var( Si_unit_name__radian );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_plane_angle, completeSE );
    instance_cnt++;

    // If degrees, create conversion based unit.
    if( angle == vsp::ANG_DEG )
    {
        STEPcomplex * ang_rad = ua_plane_angle;

        const double angconv = ( 3.14159265358979323846264338327950 / 180.0 );

        SdaiUnit * p_ang_unit = new SdaiUnit( ( SdaiNamed_unit * ) ang_rad );

        SdaiMeasure_value * p_ang_measure_value = new SdaiMeasure_value( angconv, config_control_design::t_measure_value );
        p_ang_measure_value->SetUnderlyingType( config_control_design::t_plane_angle_measure );

        SdaiPlane_angle_measure_with_unit * p_ang_measure_with_unit = new SdaiPlane_angle_measure_with_unit();
        p_ang_measure_with_unit->value_component_( p_ang_measure_value );
        p_ang_measure_with_unit->unit_component_( p_ang_unit );
        instance_list->Append( ( SDAI_Application_instance * ) p_ang_measure_with_unit, completeSE );
        instance_cnt++;

        const char * ua_conv_angle_types[4] = { "conversion_based_unit", "named_unit", "plane_angle_unit", "*" };
        ua_plane_angle = new STEPcomplex( registry, ( const char ** ) ua_conv_angle_types, instance_cnt );
        stepcomplex = ua_plane_angle->head;
        while( stepcomplex )
        {
            if( !strcmp( stepcomplex->EntityName(), "Conversion_Based_Unit" ) )
            {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL )
                {
                    if( !strcmp( attr->Name(), "name" ) )
                    {
                        attr->StrToVal( "'DEGREES'" );
                    }
                    if( !strcmp( attr->Name(), "conversion_factor" ) )
                    {
                        attr->Raw()->c = new( STEPentity * );
                        *( attr->Raw()->c ) = ( STEPentity * )( p_ang_measure_with_unit );
                    }
                }
            }
            if( !strcmp( stepcomplex->EntityName(), "Named_Unit" ) )
            {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL )
                {
                    if( !strcmp( attr->Name(), "dimensions" ) )
                    {
                        attr->Raw()->c = new( STEPentity * );
                        *( attr->Raw()->c ) = ( STEPentity * )( dimensional_exp );
                    }
                }
            }
            stepcomplex = stepcomplex->sc;
        }
        instance_list->Append( ( SDAI_Application_instance * ) ua_plane_angle, completeSE );
        instance_cnt++;
    }

    const char * ua_solid_angle_types[4] = { "named_unit", "si_unit", "solid_angle_unit", "*" };
    STEPcomplex * ua_solid_angle = new STEPcomplex( registry, ( const char ** ) ua_solid_angle_types, instance_cnt );
    stepcomplex = ua_solid_angle->head;
    while( stepcomplex )
    {
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "name" ) )
                {
                    attr->Raw()->e = new SdaiSi_unit_name_var( Si_unit_name__steradian );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_solid_angle, completeSE );
    instance_cnt++;

    // All units set up, stored in: ua_length, ua_plane_angle, ua_solid_angle
    const char * entNmArr[5] = { "geometric_representation_context", "global_uncertainty_assigned_context", "global_unit_assigned_context", "representation_context", "*" };
    STEPcomplex * complex_entity = new STEPcomplex( registry, ( const char ** ) entNmArr, instance_cnt );
    stepcomplex = complex_entity->head;

    while( stepcomplex )
    {

        if( !strcmp( stepcomplex->EntityName(), "Geometric_Representation_Context" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "coordinate_space_dimension" ) )
                {
                    attr->StrToVal( "3" );
                }
            }
        }

        if( !strcmp( stepcomplex->EntityName(), "Global_Uncertainty_Assigned_Context" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "uncertainty" ) )
                {
                    EntityAggregate * unc_agg = new EntityAggregate();
                    unc_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) uncertainty ) );
                    attr->Raw()->a = unc_agg;
                }
            }

        }

        if( !strcmp( stepcomplex->EntityName(), "Global_Unit_Assigned_Context" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                std::string attrval;
                if( !strcmp( attr->Name(), "units" ) )
                {
                    EntityAggregate * unit_assigned_agg = new EntityAggregate();
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_length ) );
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_plane_angle ) );
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_solid_angle ) );
                    attr->Raw()->a = unit_assigned_agg;
                }
            }
        }

        if( !strcmp( stepcomplex->EntityName(), "Representation_Context" ) )
        {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL )
            {
                if( !strcmp( attr->Name(), "context_identifier" ) )
                {
                    attr->StrToVal( "'STANDARD'" );
                }
                if( !strcmp( attr->Name(), "context_type" ) )
                {
                    attr->StrToVal( "'3D'" );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) complex_entity, completeSE );
    instance_cnt++;

    return complex_entity;
}

SdaiCartesian_point * STEPutil::MakePoint( const double & x, const double & y, const double & z )
{
    SdaiCartesian_point * pnt = ( SdaiCartesian_point * ) registry->ObjCreate( "CARTESIAN_POINT" );
    pnt->name_( "''" );

    RealAggregate * coords = pnt->coordinates_();

    RealNode * xnode = new RealNode();
    xnode->value = x;
    coords->AddNode( xnode );

    RealNode * ynode = new RealNode();
    ynode->value = y;
    coords->AddNode( ynode );

    RealNode * znode = new RealNode();
    znode->value = z;
    coords->AddNode( znode );

    instance_list->Append( ( SDAI_Application_instance * ) pnt, completeSE );

    return pnt;
}

SdaiDirection * STEPutil::MakeDirection( const double & x, const double & y, const double & z )
{
    SdaiDirection * dir = ( SdaiDirection * ) registry->ObjCreate( "DIRECTION" );
    dir->name_( "''" );

    RealAggregate * components = dir->direction_ratios_();

    RealNode * xnode = new RealNode();
    xnode->value = x;
    components->AddNode( xnode );

    RealNode * ynode = new RealNode();
    ynode->value = y;
    components->AddNode( ynode );

    RealNode * znode = new RealNode();
    znode->value = z;
    components->AddNode( znode );

    instance_list->Append( ( SDAI_Application_instance * ) dir, completeSE );

    return dir;
}

SdaiAxis2_placement_3d * STEPutil::DefaultAxis( )
{
    SdaiCartesian_point * pnt = MakePoint( 0.0, 0.0, 0.0 );
    SdaiDirection * axis = MakeDirection( 0.0, 0.0, 1.0 );
    SdaiDirection * refd = MakeDirection( 1.0, 0.0, 0.0 );

    SdaiAxis2_placement_3d * placement = ( SdaiAxis2_placement_3d * ) registry->ObjCreate( "AXIS2_PLACEMENT_3D" );
    placement->name_( "''" );
    placement->location_( pnt );
    placement->axis_( axis );
    placement->ref_direction_( refd );

    instance_list->Append( ( SDAI_Application_instance * ) placement, completeSE );

    return placement;
}

SdaiDate_and_time * STEPutil::DateTime( )
{
    SdaiCalendar_date * caldate = ( SdaiCalendar_date * ) registry->ObjCreate( "CALENDAR_DATE" );
    instance_list->Append( ( SDAI_Application_instance * ) caldate, completeSE );
    caldate->year_component_( 2000 );
    caldate->month_component_( 1 );
    caldate->day_component_( 1 );

    SdaiCoordinated_universal_time_offset * tzone = ( SdaiCoordinated_universal_time_offset * ) registry->ObjCreate( "COORDINATED_UNIVERSAL_TIME_OFFSET" );
    instance_list->Append( ( SDAI_Application_instance * ) tzone, completeSE );
    tzone->hour_offset_( 0 );
    tzone->minute_offset_( 0 );
    tzone->sense_( Ahead_or_behind__behind );

    SdaiLocal_time * loctime = ( SdaiLocal_time * ) registry->ObjCreate( "LOCAL_TIME" );
    instance_list->Append( ( SDAI_Application_instance * ) loctime, completeSE );
    loctime->hour_component_( 12 );
    loctime->minute_component_( 0 );
    loctime->second_component_( 0 );
    loctime->zone_( tzone );

    SdaiDate_and_time * date_time = ( SdaiDate_and_time * ) registry->ObjCreate( "DATE_AND_TIME" );
    instance_list->Append( ( SDAI_Application_instance * )date_time, completeSE );
    date_time->date_component_( caldate );
    date_time->time_component_( loctime );

    return date_time;
}

SdaiSecurity_classification * STEPutil::Classification( SdaiPerson_and_organization * per_org, SdaiDate_and_time * date_time, SdaiProduct_definition_formation_with_specified_source * prod_def_form )
{
    SdaiSecurity_classification_level * level = ( SdaiSecurity_classification_level * ) registry->ObjCreate( "SECURITY_CLASSIFICATION_LEVEL" );
    instance_list->Append( ( SDAI_Application_instance * ) level, completeSE );
    level->name_( "'unclassified'" );

    SdaiSecurity_classification * clas = ( SdaiSecurity_classification * ) registry->ObjCreate( "SECURITY_CLASSIFICATION" );
    instance_list->Append( ( SDAI_Application_instance * ) clas, completeSE );
    clas->name_( "''" );
    clas->purpose_( "''" );
    clas->security_level_( level );

    SdaiCc_design_security_classification * des_class = ( SdaiCc_design_security_classification * ) registry->ObjCreate( "CC_DESIGN_SECURITY_CLASSIFICATION" );
    instance_list->Append( ( SDAI_Application_instance * ) des_class, completeSE );
    des_class->assigned_security_classification_( clas );
    des_class->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def_form ) );

    SdaiPerson_and_organization_role * class_role = ( SdaiPerson_and_organization_role * ) registry->ObjCreate( "PERSON_AND_ORGANIZATION_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) class_role, completeSE );
    class_role->name_( "'classification_officer'" );

    SdaiCc_design_person_and_organization_assignment * class_personorg = ( SdaiCc_design_person_and_organization_assignment * ) registry->ObjCreate( "CC_DESIGN_PERSON_AND_ORGANIZATION_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) class_personorg, completeSE );
    class_personorg->assigned_person_and_organization_( per_org );
    class_personorg->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) clas ) );
    class_personorg->role_( class_role );

    SdaiDate_time_role * class_datetime = ( SdaiDate_time_role * ) registry->ObjCreate( "DATE_TIME_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) class_datetime, completeSE );
    class_datetime->name_( "'classification_date'" );

    SdaiCc_design_date_and_time_assignment * assign = ( SdaiCc_design_date_and_time_assignment * ) registry->ObjCreate( "CC_DESIGN_DATE_AND_TIME_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) assign, completeSE );
    assign->assigned_date_and_time_( date_time );
    assign->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) clas ) );
    assign->role_( class_datetime );

    return clas;
}


void  STEPutil::STEPBoilerplate( const vsp::LEN_UNITS & len, const char * tolstr  )
{

    // Increment FileId so entities start at #1 instead of #0.
    instance_list->NextFileId();

    registry->ResetSchemas();
    registry->ResetEntities();

    SdaiFile_name * fn = ( SdaiFile_name * ) sfile->HeaderDefaultFileName();
    header_instances->Append( ( SDAI_Application_instance * ) fn, completeSE );
    fn->name_( "'outfile.stp'" );
    fn->time_stamp_( "''" );
    fn->author_()->AddNode( new StringNode( "''" ) );
    fn->organization_()->AddNode( new StringNode( "''" ) );
    fn->preprocessor_version_( "''" );
    fn->originating_system_( "''" );
    fn->authorization_( "''" );

    SdaiFile_description * fd = ( SdaiFile_description * ) sfile->HeaderDefaultFileDescription();
    header_instances->Append( ( SDAI_Application_instance * ) fd, completeSE );
    fd->description_()->AddNode( new StringNode( "''" ) );
    fd->implementation_level_( "'1'" );

    SdaiFile_schema * fs = ( SdaiFile_schema * ) sfile->HeaderDefaultFileSchema();
    header_instances->Append( ( SDAI_Application_instance * ) fs, completeSE );
    fs->schema_identifiers_()->AddNode( new StringNode( "'CONFIG_CONTROL_DESIGN'" ) );


    // Build file data.  The entities have been created and added in order such that no entity
    // references a later entity.  This is not required, but has been done to give a logical
    // flow to the source and the resulting STEP file.

    // Stand-in date and time.
    SdaiDate_and_time * date_time = DateTime( );

    // Global units and tolerance.
    context = Geometric_Context( len, vsp::ANG_DEG, tolstr );

    // Primary coordinate system.
    SdaiAxis2_placement_3d * orig_transform = DefaultAxis( );

    // Basic context through product and shape representation
    SdaiApplication_context * app_context = ( SdaiApplication_context * ) registry->ObjCreate( "APPLICATION_CONTEXT" );
    instance_list->Append( ( SDAI_Application_instance * ) app_context, completeSE );
    app_context->application_( "'configuration controlled 3d designs of mechanical parts and assemblies'" );

    SdaiMechanical_context * mech_context = ( SdaiMechanical_context * ) registry->ObjCreate( "MECHANICAL_CONTEXT" );
    instance_list->Append( ( SDAI_Application_instance * ) mech_context, completeSE );
    mech_context->name_( "''" );
    mech_context->discipline_type_( "'mechanical'" );
    mech_context->frame_of_reference_( app_context );

    SdaiApplication_protocol_definition * app_protocol = ( SdaiApplication_protocol_definition * ) registry->ObjCreate( "APPLICATION_PROTOCOL_DEFINITION" );
    instance_list->Append( ( SDAI_Application_instance * ) app_protocol, completeSE );
    app_protocol->status_( "'international standard'" );
    app_protocol->application_protocol_year_( 1994 );
    app_protocol->application_interpreted_model_schema_name_( "'config_control_design'" );
    app_protocol->application_( app_context );

    SdaiDesign_context * design_context = ( SdaiDesign_context * ) registry->ObjCreate( "DESIGN_CONTEXT" );
    instance_list->Append( ( SDAI_Application_instance * ) design_context, completeSE );
    design_context->name_( "''" );
    design_context->life_cycle_stage_( "'design'" );
    design_context->frame_of_reference_( app_context );

    SdaiProduct * prod = ( SdaiProduct * ) registry->ObjCreate( "PRODUCT" );
    instance_list->Append( ( SDAI_Application_instance * ) prod, completeSE );
    prod->id_( "''" );
    prod->name_( "'prodname'" );
    prod->description_( "''" );
    prod->frame_of_reference_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) mech_context ) );

    SdaiProduct_related_product_category * prodcat = ( SdaiProduct_related_product_category * ) registry->ObjCreate( "PRODUCT_RELATED_PRODUCT_CATEGORY" );
    instance_list->Append( ( SDAI_Application_instance * ) prodcat, completeSE );
    prodcat->name_( "'assembly'" );
    prodcat->description_( "''" );
    prodcat->products_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod ) );

    SdaiProduct_definition_formation_with_specified_source * prod_def_form = ( SdaiProduct_definition_formation_with_specified_source * ) registry->ObjCreate( "PRODUCT_DEFINITION_FORMATION_WITH_SPECIFIED_SOURCE" );
    instance_list->Append( ( SDAI_Application_instance * ) prod_def_form, completeSE );
    prod_def_form->id_( "''" );
    prod_def_form->description_( "''" );
    prod_def_form->of_product_( prod );
    prod_def_form->make_or_buy_( Source__made );

    SdaiProduct_definition * prod_def = ( SdaiProduct_definition * ) registry->ObjCreate( "PRODUCT_DEFINITION" );
    instance_list->Append( ( SDAI_Application_instance * ) prod_def, completeSE );
    prod_def->id_( "''" );
    prod_def->description_( "''" );
    prod_def->frame_of_reference_( design_context );
    prod_def->formation_( prod_def_form );

    SdaiProduct_definition_shape * pshape = ( SdaiProduct_definition_shape * ) registry->ObjCreate( "PRODUCT_DEFINITION_SHAPE" );
    instance_list->Append( ( SDAI_Application_instance * ) pshape, completeSE );
    pshape->name_( "''" );
    pshape->description_( "'ProductShapeDescription'" );
    pshape->definition_( new SdaiCharacterized_definition( new SdaiCharacterized_product_definition( prod_def ) ) );

    shape_rep = ( SdaiShape_representation * ) registry->ObjCreate( "SHAPE_REPRESENTATION" );
    instance_list->Append( ( SDAI_Application_instance * ) shape_rep, completeSE );
    shape_rep->name_( "''" ); // Document?
    shape_rep->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) orig_transform ) );
    shape_rep->context_of_items_( ( SdaiRepresentation_context * ) context );

    SdaiShape_definition_representation * shape_def_rep = ( SdaiShape_definition_representation * ) registry->ObjCreate( "SHAPE_DEFINITION_REPRESENTATION" );
    instance_list->Append( ( SDAI_Application_instance * ) shape_def_rep, completeSE );
    shape_def_rep->definition_( pshape );
    shape_def_rep->used_representation_( shape_rep );


    // Stand-in person and org.
    SdaiPerson * person = ( SdaiPerson * ) registry->ObjCreate( "PERSON" );
    instance_list->Append( ( SDAI_Application_instance * ) person, completeSE );
    person->id_( "''" );
    person->last_name_( "'Doe'" );
    person->first_name_( "'John'" );

    SdaiOrganization * org = ( SdaiOrganization * ) registry->ObjCreate( "ORGANIZATION" );
    instance_list->Append( ( SDAI_Application_instance * ) org, completeSE );
    org->id_( "''" );
    org->name_( "''" );
    org->description_( "''" );

    SdaiPerson_and_organization * per_org = ( SdaiPerson_and_organization * ) registry->ObjCreate( "PERSON_AND_ORGANIZATION" );
    instance_list->Append( ( SDAI_Application_instance * ) per_org, completeSE );
    per_org->the_person_( person );
    per_org->the_organization_( org );


    // Required roles.
    SdaiPerson_and_organization_role * creator_role = ( SdaiPerson_and_organization_role * ) registry->ObjCreate( "PERSON_AND_ORGANIZATION_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) creator_role, completeSE );
    creator_role->name_( "'creator'" );

    SdaiPerson_and_organization_role * owner_role = ( SdaiPerson_and_organization_role * ) registry->ObjCreate( "PERSON_AND_ORGANIZATION_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) owner_role, completeSE );
    owner_role->name_( "'design_owner'" );

    SdaiPerson_and_organization_role * supplier_role = ( SdaiPerson_and_organization_role * ) registry->ObjCreate( "PERSON_AND_ORGANIZATION_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) supplier_role, completeSE );
    supplier_role->name_( "'design_supplier'" );


    // Basic approval.
    SdaiApproval_status * approval_status = ( SdaiApproval_status * ) registry->ObjCreate( "APPROVAL_STATUS" );
    instance_list->Append( ( SDAI_Application_instance * ) approval_status, completeSE );
    approval_status->name_( "'approved'" );

    SdaiApproval * approval = ( SdaiApproval * ) registry->ObjCreate( "APPROVAL" );
    instance_list->Append( ( SDAI_Application_instance * ) approval, completeSE );
    approval->status_( approval_status );
    approval->level_( "''" );

    SdaiApproval_date_time * app_date_time = ( SdaiApproval_date_time * ) registry->ObjCreate( "APPROVAL_DATE_TIME" );
    instance_list->Append( ( SDAI_Application_instance * ) app_date_time, completeSE );
    app_date_time->date_time_( new SdaiDate_time_select( date_time ) );
    app_date_time->dated_approval_( approval );

    SdaiApproval_role * app_role = ( SdaiApproval_role * ) registry->ObjCreate( "APPROVAL_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) app_role, completeSE );
    app_role->role_( "'approver'" );

    SdaiApproval_person_organization * app_per_org = ( SdaiApproval_person_organization * ) registry->ObjCreate( "APPROVAL_PERSON_ORGANIZATION" );
    instance_list->Append( ( SDAI_Application_instance * ) app_per_org, completeSE );
    app_per_org->person_organization_( new SdaiPerson_organization_select( per_org ) );
    app_per_org->authorized_approval_( approval );
    app_per_org->role_( app_role );


    // Basic Classification.
    SdaiSecurity_classification * clas = Classification( per_org, date_time, prod_def_form );


    // Basic CC approval.
    SdaiCc_design_approval * desapproval = ( SdaiCc_design_approval * ) registry->ObjCreate( "CC_DESIGN_APPROVAL" );
    instance_list->Append( ( SDAI_Application_instance * ) desapproval, completeSE );
    desapproval->assigned_approval_( approval );
    desapproval->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def ) );
    desapproval->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def_form ) );
    desapproval->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) clas ) );

    SdaiCc_design_person_and_organization_assignment * creatorpersonorg = ( SdaiCc_design_person_and_organization_assignment * ) registry->ObjCreate( "CC_DESIGN_PERSON_AND_ORGANIZATION_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) creatorpersonorg, completeSE );
    creatorpersonorg->assigned_person_and_organization_( per_org );
    creatorpersonorg->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def ) );
    creatorpersonorg->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def_form ) );
    creatorpersonorg->role_( creator_role );

    SdaiCc_design_person_and_organization_assignment * supplierpersonorg = ( SdaiCc_design_person_and_organization_assignment * ) registry->ObjCreate( "CC_DESIGN_PERSON_AND_ORGANIZATION_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) supplierpersonorg, completeSE );
    supplierpersonorg->assigned_person_and_organization_( per_org );
    supplierpersonorg->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def_form ) );
    supplierpersonorg->role_( supplier_role );

    SdaiDate_time_role * datetimerole = ( SdaiDate_time_role * ) registry->ObjCreate( "DATE_TIME_ROLE" );
    instance_list->Append( ( SDAI_Application_instance * ) datetimerole, completeSE );
    datetimerole->name_( "'creation_date'" );

    SdaiCc_design_date_and_time_assignment * assign = ( SdaiCc_design_date_and_time_assignment * ) registry->ObjCreate( "CC_DESIGN_DATE_AND_TIME_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) assign, completeSE );
    assign->assigned_date_and_time_( date_time );
    assign->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_def ) );
    assign->role_( datetimerole );

    SdaiCc_design_person_and_organization_assignment * ownerpersonorg = ( SdaiCc_design_person_and_organization_assignment * ) registry->ObjCreate( "CC_DESIGN_PERSON_AND_ORGANIZATION_ASSIGNMENT" );
    instance_list->Append( ( SDAI_Application_instance * ) ownerpersonorg, completeSE );
    ownerpersonorg->assigned_person_and_organization_( per_org );
    ownerpersonorg->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod ) );
    ownerpersonorg->role_( owner_role );
}

void STEPutil::WriteFile( string fname )
{
    sfile->WriteExchangeFile( fname.c_str() );
    if( sfile->Error().severity() < SEVERITY_USERMSG )
    {
        sfile->Error().PrintContents( cout );
    }
}

void STEPutil::AddSurf( VspSurf *s, bool splitsurf, bool mergepts, bool tocubic, double tol, bool trimte, const vector < double > &USplit, const vector < double > &WSplit, string name )
{
//  vector<SdaiBezier_surface *> surfs;
//  s->ToSTEP_Bez_Patches( this, surfs );

    vector<SdaiB_spline_surface_with_knots *> surfs;
    s->ToSTEP_BSpline_Quilt( this, surfs, splitsurf, mergepts, tocubic, tol, trimte, USplit, WSplit );

    SdaiGeometric_set *gset = ( SdaiGeometric_set* ) registry->ObjCreate( "GEOMETRIC_SET" );
    instance_list->Append( ( SDAI_Application_instance * ) gset, completeSE );
    gset->name_( "'" + name + "'" );

    for( int i = 0; i < surfs.size(); ++i )
    {
        gset->elements_()->AddNode( new EntityNode( ( SDAI_Application_instance* ) surfs[i] ) );
    }

    SdaiGeometrically_bounded_surface_shape_representation * gbshape = ( SdaiGeometrically_bounded_surface_shape_representation* ) registry->ObjCreate( "GEOMETRICALLY_BOUNDED_SURFACE_SHAPE_REPRESENTATION" );
    instance_list->Append( ( SDAI_Application_instance * ) gbshape, completeSE );
    gbshape->name_( "''" );
    gbshape->context_of_items_( ( SdaiRepresentation_context* ) context );
    gbshape->items_()->AddNode( new EntityNode( ( SDAI_Application_instance* ) gset ) );

    SdaiShape_representation_relationship *shapereprelay = ( SdaiShape_representation_relationship* ) registry->ObjCreate( "SHAPE_REPRESENTATION_RELATIONSHIP" );
    instance_list->Append( ( SDAI_Application_instance * ) shapereprelay, completeSE );
    shapereprelay->name_( "''" );
    shapereprelay->description_( "''" );
    shapereprelay->rep_1_( shape_rep );
    shapereprelay->rep_2_( gbshape );

}



//===================================================================//
//=================        IGES Functions         ===================//
//===================================================================//

IGESutil::IGESutil( const int& len_unit )
{
    switch ( len_unit )
    {
        case vsp::LEN_CM:
            model.SetUnitsFlag( UNIT_CENTIMETER );
            break;
        case vsp::LEN_M:
            model.SetUnitsFlag( UNIT_METER );
            break;
        case vsp::LEN_MM:
            model.SetUnitsFlag( UNIT_MM );
            break;
        case vsp::LEN_IN:
            model.SetUnitsFlag( UNIT_IN );
            break;
        case vsp::LEN_FT:
            model.SetUnitsFlag( UNIT_FOOT );
            break;
    }

    model.SetNativeSystemID( VSPVERSION4 );
}

IGESutil::~IGESutil()
{
}

void IGESutil::WriteFile( string fname, bool overwrite )
{
    model.Write( fname.c_str(), overwrite );
}

DLL_IGES_ENTITY_128 IGESutil::MakeSurf( piecewise_surface_type& s, const string& label )
{
    vector< vector< int > > ptindxs;
    vector< vec3d > allPntVec;

    piecewise_surface_type::index_type nupatch, nvpatch;
    piecewise_surface_type::index_type maxu, maxv;
    piecewise_surface_type::index_type nupts, nvpts;

    ExtractCPts( s, ptindxs, allPntVec, maxu, maxv, nupatch, nvpatch, nupts, nvpts );

    DLL_IGES_ENTITY_128 isurf( model, true );

    if ( label.size() > 0 )
    {
        DLL_IGES_ENTITY_406 e406( model, true );
        e406.SetProperty_Name( label.c_str() );
        isurf.AddOptionalEntity( e406.GetRawPtr() );
        e406.Detach();
    }

    // Identify coefficient and knot vectors
    vector < double > coeff( nupts * nvpts * 3 );

    int icoeff = 0;
    for ( int v = 0; v < nvpts; v++ )
    {
        for ( int n = 0; n < nupts; ++n )
        {
            int pindx = ptindxs[n][v];
            vec3d pt = allPntVec[pindx];

            for ( int k = 0; k < 3; k++ )
            {
                coeff[icoeff] = pt.v[k];
                icoeff++;
            }
        }
    }

    vector< double > knotu, knotv;

    IGESKnots( maxu, nupatch, knotu );
    IGESKnots( maxv, nvpatch, knotv );

    if ( !isurf.SetNURBSData( nupts, nvpts, maxu + 1, maxv + 1,
                              knotu.data(), knotv.data(), coeff.data(),
                              false, false, false,
                              knotu[0], knotu.back(), knotv[0], knotv.back() ) )
    {
        model.DelEntity( &isurf );
    }

    return isurf;
}


void IGESutil::IGESKnots( int deg, int npatch, vector< double >& knot )
{
    int i, j;

    knot.clear();

    for ( i = 0; i <= deg; i++ )
    {
        knot.push_back( 0.0 );
    }
    for ( i = 1; i <= npatch; ++i )
    {
        for ( j = 0; j < deg; j++ )
        {
            knot.push_back( 1.0 * i );
        }
    }
    for ( i = 0; i <= deg; i++ )
    {
        knot.push_back( 1.0 * npatch );
    }
}

//===================================================================//
//==============       General CAD Functions         ================//
//===================================================================//

void ExtractCPts( piecewise_surface_type& s, vector< vector< int > >& ptindxs, vector< vec3d >& allPntVec,
                  piecewise_surface_type::index_type& maxu, piecewise_surface_type::index_type& maxv,
                  piecewise_surface_type::index_type& nupatch, piecewise_surface_type::index_type& nvpatch,
                  piecewise_surface_type::index_type& nupts, piecewise_surface_type::index_type& nvpts )
{
    piecewise_surface_type::index_type minu, minv;
    piecewise_surface_type::index_type ip, jp;

    nupatch = s.number_u_patches();
    nvpatch = s.number_v_patches();

    s.degree_u( minu, maxu );
    s.degree_v( minv, maxv );

    nupts = nupatch * maxu + 1;
    nvpts = nvpatch * maxv + 1;

    ptindxs.resize( nupts );
    for ( int i = 0; i < nupts; ++i )
    {
        ptindxs[i].resize( nvpts );
    }

    allPntVec.clear();
    allPntVec.reserve( nupts * nvpts );

    for ( ip = 0; ip < nupatch; ++ip )
    {
        for ( jp = 0; jp < nvpatch; ++jp )
        {
            surface_patch_type::index_type icp, jcp;
            surface_patch_type::index_type ilim = maxu;
            surface_patch_type::index_type jlim = maxv;

            surface_patch_type* patch = s.get_patch( ip, jp );

            patch->promote_u_to( maxu );
            patch->promote_v_to( maxv );

            if ( ip == nupatch - 1 ) // Last u patch
            {
                ilim = maxu + 1;
            }

            if ( jp == nvpatch - 1 ) // Last u patch
            {
                jlim = maxv + 1;
            }

            for ( icp = 0; icp < ilim; ++icp )
            {
                for ( jcp = 0; jcp < jlim; ++jcp )
                {
                    surface_patch_type::point_type p = patch->get_control_point( icp, jcp );
                    ptindxs[ip * maxu + icp][jp * maxv + jcp] = allPntVec.size();
                    allPntVec.emplace_back( vec3d( p[0], p[1], p[2] ) );
                }
            }
        }
    }
}
