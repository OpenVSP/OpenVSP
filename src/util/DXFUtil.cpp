#include "DXFUtil.h"

void WriteDXFHeader( FILE* dxf_file, int LenUnitChoice )
{
    if ( dxf_file )
    {
        fprintf( dxf_file, "%d\n", 999 );
        fprintf( dxf_file, "%s\n", "DXF file written from OpenVSP" );
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "SECTION" );
        fprintf( dxf_file, "  %d\n", 2 );
        fprintf( dxf_file, "%s\n", "HEADER" );
        fprintf( dxf_file, "  %d\n", 9 );
        //AutoCAD drawing database version number: AC1006 = R10, AC1009 = R11 and R12, AC1012 = R13, AC1014 = R14
        fprintf( dxf_file, "%s\n", "$ACADVER" );
        fprintf( dxf_file, "  %d\n", 1 );
        fprintf( dxf_file, "%s\n", "AC1006" );
        fprintf( dxf_file, "  %d\n", 9 );
        // Insertion base:
        fprintf( dxf_file, "%s\n", "$INSBASE" );
        fprintf( dxf_file, "  %d\n", 10 );
        fprintf( dxf_file, "%f\n", 0.0 );
        fprintf( dxf_file, "  %d\n", 20 );
        fprintf( dxf_file, "%f\n", 0.0 );
        fprintf( dxf_file, "  %d\n", 30 );
        fprintf( dxf_file, "%f\n", 0.0 );

        // Units:
        if ( LenUnitChoice == vsp::LEN_UNITS::LEN_UNITLESS )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 0 );
        }
        else if ( LenUnitChoice == vsp::LEN_UNITS::LEN_MM )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 4 );
        }
        else if ( LenUnitChoice == vsp::LEN_UNITS::LEN_CM )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 5 );
        }
        else if (  LenUnitChoice == vsp::LEN_UNITS::LEN_M )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 6 );
        }
        else if ( LenUnitChoice == vsp::LEN_UNITS::LEN_IN )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 1 );
        }
        else if (  LenUnitChoice == vsp::LEN_UNITS::LEN_FT )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 2 );
        }
        else if ( LenUnitChoice == vsp::LEN_UNITS::LEN_YD )
        {
            fprintf( dxf_file, "  %d\n", 9 );
            fprintf( dxf_file, "%s\n", "$INSUNITS" );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "  %d\n", 10 );
        }

        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "ENDSEC" );

        //Blocks
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "SECTION" );
        fprintf( dxf_file, "  %d\n", 2 );
        fprintf( dxf_file, "%s\n", "BLOCKS" );
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "ENDSEC" );

        //Entities
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "SECTION" );
        fprintf( dxf_file, "  %d\n", 2 );
        fprintf( dxf_file, "%s\n", "ENTITIES" );
    }
}

void DXFManipulate( vector < vector < vec3d > > &allflines, const BndBox &dxfbox, int view, int ang )
{
    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            // Left: +x,+z; swap y & z
            if ( view == vsp::VIEW_TYPE::VIEW_LEFT )
            {
                allflines[l][j] = allflines[l][j].swap_yz();
            }
            // Right: -x,+z; reflect x, swap y & z
            else if ( view == vsp::VIEW_TYPE::VIEW_RIGHT )
            {
                allflines[l][j] = allflines[l][j].reflect_yz();
                allflines[l][j].set_x( allflines[l][j].x() + ( dxfbox.GetMax().x() + dxfbox.GetMin().x() ) );
                allflines[l][j] = allflines[l][j].swap_yz();
            }
            // Top: +x,+y; Do nothing
            else if ( view == vsp::VIEW_TYPE::VIEW_TOP )
            {
            }
            // Bottom: +x,-y; reflect y
            else if ( view == vsp::VIEW_TYPE::VIEW_BOTTOM )
            {
                allflines[l][j] = allflines[l][j].reflect_xz();
                allflines[l][j].set_y( allflines[l][j].y() + ( dxfbox.GetMax().y() + dxfbox.GetMin().y() ) );
            }
            // Front: -y,+z; swap x & y, swap y & z, reflect y
            else if ( view == vsp::VIEW_TYPE::VIEW_FRONT )
            {
                allflines[l][j] = allflines[l][j].swap_xy();
                allflines[l][j] = allflines[l][j].reflect_yz();
                allflines[l][j].set_x( allflines[l][j].x() + ( dxfbox.GetMax().y() + dxfbox.GetMin().y() ) );
                allflines[l][j] = allflines[l][j].swap_yz();
            }
            // Rear: +y,+z; swap x & y, swap y & z
            else if ( view == vsp::VIEW_TYPE::VIEW_REAR )
            {
                allflines[l][j] = allflines[l][j].swap_xy();
                allflines[l][j] = allflines[l][j].swap_yz();
            }
        }
    }
    if ( view == vsp::VIEW_TYPE::VIEW_NONE )
    {
        allflines.clear();
    }

    if ( ang == vsp::VIEW_ROT::ROT_0 )
    {
        ang = 0;
    }
    else if ( ang == vsp::VIEW_ROT::ROT_90 )
    {
        ang = 90;
    }
    else if ( ang == vsp::VIEW_ROT::ROT_180 )
    {
        ang = 180;
    }
    else if ( ang == vsp::VIEW_ROT::ROT_270 )
    {
        ang = 270;
    }

    double rad = ang * 3.14159265 / 180;

    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            double old_x = allflines[l][j].x();
            allflines[l][j].set_x( ( cos( rad ) * allflines[l][j].x() + sin( rad ) * allflines[l][j].y() ) );
            allflines[l][j].set_y( ( -sin( rad ) * old_x + cos( rad ) * allflines[l][j].y() ) );
        }
    }
}

void DXFShift( vector < vector < vec3d > > &allflines, vec3d shiftvec, int shift, int ang1, int ang2 )
{
    if ( ang1 == vsp::VIEW_ROT::ROT_90 )
    {
        shiftvec = shiftvec.swap_xy();
    }
    else if ( ang1 == vsp::VIEW_ROT::ROT_270 )
    {
        shiftvec = shiftvec.swap_xy();
    }
    else if ( ang2 == vsp::VIEW_ROT::ROT_90 )
    {
        shiftvec = shiftvec.swap_xy();
    }
    else if ( ang2 == vsp::VIEW_ROT::ROT_270 )
    {
        shiftvec = shiftvec.swap_xy();
    }

    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            if ( shift == vsp::VIEW_SHIFT::LEFT )
            {
                allflines[l][j].offset_x( -abs( shiftvec.x() ) * 0.75 );
            }
            else if ( shift == vsp::VIEW_SHIFT::RIGHT )
            {
                allflines[l][j].offset_x( abs( shiftvec.x() ) * 0.75 );
            }
            else if ( shift == vsp::VIEW_SHIFT::UP )
            {
                allflines[l][j].offset_y( abs( shiftvec.y() ) );
            }
            else if ( shift == vsp::VIEW_SHIFT::DOWN )
            {
                allflines[l][j].offset_y( -abs( shiftvec.y() ) );
            }
        }
    }
}

void WriteDXFPolylines3D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer )
{
    if ( dxf_file )
    {
        for ( unsigned int l = 0; l < allflines.size(); l++ )
        {
            fprintf( dxf_file, "  %d\n", 0 );
            fprintf( dxf_file, "%s\n", "POLYLINE" );
            fprintf( dxf_file, "  %d\n", 100 );
            fprintf( dxf_file, "%s\n", "AcDbEntity" );
            fprintf( dxf_file, "  %d\n", 8 );
            fprintf( dxf_file, "%s\n", layer.c_str() );
            fprintf( dxf_file, "  %d\n", 100 );
            fprintf( dxf_file, "%s\n", "AcDb3dPolyline" );
            fprintf( dxf_file, "  %d\n", 66 );
            fprintf( dxf_file, "     %d\n", 1 );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "     %d\n", 8 );
            fprintf( dxf_file, "  %d\n", 10 );
            fprintf( dxf_file, "%f\n", 0.0 ); //x
            fprintf( dxf_file, "  %d\n", 20 );
            fprintf( dxf_file, "%f\n", 0.0 ); //y
            fprintf( dxf_file, "  %d\n", 30 );
            fprintf( dxf_file, "%f\n", 0.0 ); //z
            //Same Start/End Width:
            fprintf( dxf_file, "  %d\n", 40 );
            fprintf( dxf_file, "%f\n", 0.0 );
            fprintf( dxf_file, "  %d\n", 41 );
            fprintf( dxf_file, "%f\n", 0.0 );

            for ( unsigned int j = 0; j < allflines[l].size(); j++ )
            {
                fprintf( dxf_file, "  %d\n", 0 );
                fprintf( dxf_file, "%s\n", "VERTEX" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbEntity" );
                fprintf( dxf_file, "  %d\n", 8 );
                fprintf( dxf_file, "%s\n", layer.c_str() );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbVertex" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDb3dPolylineVertex" );
                fprintf( dxf_file, "  %d\n", 10 );
                fprintf( dxf_file, "%f\n", allflines[l][j].x() ); //x
                fprintf( dxf_file, "  %d\n", 20 );
                fprintf( dxf_file, "%f\n", allflines[l][j].y() ); //y
                fprintf( dxf_file, "  %d\n", 30 );
                fprintf( dxf_file, "%f\n", allflines[l][j].z() ); //z
                fprintf( dxf_file, "  %d\n", 70 );
                fprintf( dxf_file, "    %d\n", 32 );
            }
            fprintf( dxf_file, "  %d\n", 0 );
            fprintf( dxf_file, "%s\n", "SEQEND" );
            fprintf( dxf_file, "  %d\n", 8 );
            fprintf( dxf_file, "%s\n", layer.c_str() );
        }
    }
}

void WriteDXFPolylines2D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer )
{
    if ( dxf_file )
    {
        for ( unsigned int l = 0; l < allflines.size(); l++ )
        {
            fprintf( dxf_file, "  %d\n", 0 );
            fprintf( dxf_file, "%s\n", "POLYLINE" );
            fprintf( dxf_file, "  %d\n", 100 );
            fprintf( dxf_file, "%s\n", "AcDbEntity" );
            fprintf( dxf_file, "  %d\n", 8 );
            fprintf( dxf_file, "%s\n", layer.c_str() );
            fprintf( dxf_file, "  %d\n", 100 );
            fprintf( dxf_file, "%s\n", "AcDb3dPolyline" );
            fprintf( dxf_file, "  %d\n", 66 );
            fprintf( dxf_file, "     %d\n", 1 );
            fprintf( dxf_file, "  %d\n", 70 );
            fprintf( dxf_file, "     %d\n", 8 );
            fprintf( dxf_file, "  %d\n", 10 );
            fprintf( dxf_file, "%f\n", 0.0 ); //x
            fprintf( dxf_file, "  %d\n", 20 );
            fprintf( dxf_file, "%f\n", 0.0 ); //y
            //Same Start/End Width:
            fprintf( dxf_file, "  %d\n", 40 );
            fprintf( dxf_file, "%f\n", 0.0 );
            fprintf( dxf_file, "  %d\n", 41 );
            fprintf( dxf_file, "%f\n", 0.0 );

            for ( unsigned int j = 0; j < allflines[l].size(); j++ )
            {
                fprintf( dxf_file, "  %d\n", 0 );
                fprintf( dxf_file, "%s\n", "VERTEX" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbEntity" );
                fprintf( dxf_file, "  %d\n", 8 );
                fprintf( dxf_file, "%s\n", layer.c_str() );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbVertex" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDb3dPolylineVertex" );
                fprintf( dxf_file, "  %d\n", 10 );
                fprintf( dxf_file, "%f\n", allflines[l][j].x() ); //x
                fprintf( dxf_file, "  %d\n", 20 );
                fprintf( dxf_file, "%f\n", allflines[l][j].y() ); //y
                fprintf( dxf_file, "  %d\n", 70 );
                fprintf( dxf_file, "    %d\n", 32 );
            }
            fprintf( dxf_file, "  %d\n", 0 );
            fprintf( dxf_file, "%s\n", "SEQEND" );
            fprintf( dxf_file, "  %d\n", 8 );
            fprintf( dxf_file, "%s\n", layer.c_str() );
        }
    }
}

void WriteDXFClose( FILE* dxf_file )
{
    if ( dxf_file )
    {
        //End of File
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "ENDSEC" );
        fprintf( dxf_file, "  %d\n", 0 );
        fprintf( dxf_file, "%s\n", "EOF" );
    }
}
