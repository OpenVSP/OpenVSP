//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DXFUtil.cpp
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#include "DXFUtil.h"
#include "main.h"

void WriteDXFHeader( FILE* dxf_file, int LenUnitChoice )
{
    if ( dxf_file )
    {
        fprintf( dxf_file, "999\n" );
        fprintf( dxf_file, "DXF file written from %s\n", VSPVERSION4 );
        fprintf( dxf_file, "  0\n" );
        fprintf( dxf_file, "SECTION\n" );
        fprintf( dxf_file, "  2\n" );
        fprintf( dxf_file, "HEADER\n" );
        fprintf( dxf_file, "  9\n" );
        //AutoCAD drawing database version number: AC1006 = R10, AC1009 = R11 and R12, AC1012 = R13, AC1014 = R14
        fprintf( dxf_file, "$ACADVER\n" );
        fprintf( dxf_file, "  1\n" );
        fprintf( dxf_file, "AC1006s\n" );
        fprintf( dxf_file, "  9\n" );
        // Insertion base:
        fprintf( dxf_file, "$INSBASE\n" );
        fprintf( dxf_file, "  10\n" );
        fprintf( dxf_file, "0.0\n" );
        fprintf( dxf_file, "  20\n" );
        fprintf( dxf_file, "0.0\n" );
        fprintf( dxf_file, "  30\n" );
        fprintf( dxf_file, "0.0\n" );

        // Units:
        fprintf( dxf_file, "  9\n" );
        fprintf( dxf_file, "$INSUNITS\n" );
        fprintf( dxf_file, "  70\n" );

        switch ( LenUnitChoice )
        {
        case vsp::LEN_UNITS::LEN_UNITLESS:
            fprintf( dxf_file, "  0\n" );
            break;
        case vsp::LEN_UNITS::LEN_MM:
            fprintf( dxf_file, "  4\n" );
            break;
        case vsp::LEN_UNITS::LEN_CM:
            fprintf( dxf_file, "  5\n" );
            break;
        case vsp::LEN_UNITS::LEN_M:
            fprintf( dxf_file, "  6\n" );
            break;
        case vsp::LEN_UNITS::LEN_IN:
            fprintf( dxf_file, "  1\n" );
            break;
        case vsp::LEN_UNITS::LEN_FT:
            fprintf( dxf_file, "  2\n" );
            break;
        case vsp::LEN_UNITS::LEN_YD:
            fprintf( dxf_file, "  10\n" );
            break;
        }

        fprintf( dxf_file, "  0\n" );
        fprintf( dxf_file, "ENDSEC\n" );

        //Blocks
        fprintf( dxf_file, "  0\n" );
        fprintf( dxf_file, "SECTION\n" );
        fprintf( dxf_file, "  2\n" );
        fprintf( dxf_file, "BLOCKS\n" );
        fprintf( dxf_file, "  0\n" );
        fprintf( dxf_file, "ENDSEC\n" );

        //Entities
        fprintf( dxf_file, "  0\n" );
        fprintf( dxf_file, "SECTION\n" );
        fprintf( dxf_file, "  2\n" );
        fprintf( dxf_file, "ENTITIES\n" );
    }
}

void FeatureLinesManipulate( vector < vector < vec3d > > &allflines, int view, int ang, vec3d shiftvec )
{
    if ( view == vsp::VIEW_TYPE::VIEW_NONE )
    {
        allflines.clear();
        return;
    }

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
                allflines[l][j].offset_x( shiftvec.x() ); 
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
                allflines[l][j].offset_y( shiftvec.y() );
            }
            // Front: -y,+z; swap x & y, swap y & z, reflect y
            else if ( view == vsp::VIEW_TYPE::VIEW_FRONT )
            {
                allflines[l][j] = allflines[l][j].swap_xy();
                allflines[l][j] = allflines[l][j].reflect_yz();
                allflines[l][j].offset_x( shiftvec.y() );
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

    vec3d NewShiftVec;

    if ( view == vsp::VIEW_TYPE::VIEW_LEFT || view == vsp::VIEW_TYPE::VIEW_RIGHT )
    {
        NewShiftVec = shiftvec.swap_yz();
    }
    else if ( view == vsp::VIEW_TYPE::VIEW_FRONT || view == vsp::VIEW_TYPE::VIEW_REAR )
    {
        NewShiftVec = shiftvec.swap_xy();
        NewShiftVec = NewShiftVec.swap_yz();
    }
    else
    {
        NewShiftVec = shiftvec;
    }

    vec3d RotShiftVec;

    if ( ang == vsp::VIEW_ROT::ROT_0 )
    {
        ang = 0;
    }
    else if ( ang == vsp::VIEW_ROT::ROT_90 )
    {
        ang = 90;
        RotShiftVec.set_x( NewShiftVec.y() );
    }
    else if ( ang == vsp::VIEW_ROT::ROT_180 )
    {
        ang = 180;
        RotShiftVec.set_x( NewShiftVec.x() );
        RotShiftVec.set_y( NewShiftVec.y() );
    }
    else if ( ang == vsp::VIEW_ROT::ROT_270 )
    {
        ang = 270;
        RotShiftVec.set_y( NewShiftVec.x() );
    }

    double rad = ang * DEG_2_RAD;

    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            double old_x = allflines[l][j].x();
            double old_y = allflines[l][j].y();

            allflines[l][j].set_x( ( cos( rad ) * old_x - sin( rad ) * old_y ) + RotShiftVec.x() );
            allflines[l][j].set_y( ( sin( rad ) * old_x + cos( rad ) * old_y ) + RotShiftVec.y() );
        }
    }
}

void FeatureLinesShift( vector < vector < vec3d > > &allflines, vec3d shiftvec, int shift, int ang1, int ang2 )
{
    vec3d shiftvec_temp = shiftvec;

    if ( ang1 == vsp::VIEW_ROT::ROT_90 || ang1 == vsp::VIEW_ROT::ROT_270 )
    {
        shiftvec_temp = shiftvec.swap_xy();
    }
    else if ( ang2 == vsp::VIEW_ROT::ROT_90 || ang2 == vsp::VIEW_ROT::ROT_270 )
    {
        shiftvec_temp = shiftvec.swap_xy();
    }

    for ( unsigned int l = 0; l < allflines.size(); l++ )
    {
        for ( unsigned int j = 0; j < allflines[l].size(); j++ )
        {
            if ( shift == vsp::VIEW_SHIFT::LEFT )
            {
                allflines[l][j].offset_x( -std::abs( shiftvec_temp.x() ) );
            }
            else if ( shift == vsp::VIEW_SHIFT::RIGHT )
            {
                allflines[l][j].offset_x( std::abs( shiftvec_temp.x() ) );
            }
            else if ( shift == vsp::VIEW_SHIFT::UP )
            {
                allflines[l][j].offset_y( std::abs( shiftvec_temp.y() ) );
            }
            else if ( shift == vsp::VIEW_SHIFT::DOWN )
            {
                allflines[l][j].offset_y( -std::abs( shiftvec_temp.y() ) );
            }
        }
    }
}

void WriteDXFPolylines3D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer, bool colorflag, int color_count )
{
    if ( dxf_file )
    {
        int color;

        if ( colorflag )
        {
            color = DXFColorWheel( color_count );
        }
        else
        {
            color = 0; // Black
        }

        for ( unsigned int l = 0; l < allflines.size(); l++ )
        {
            fprintf( dxf_file, "  0\n" );
            fprintf( dxf_file, "POLYLINE\n" );
            fprintf( dxf_file, "  100\n" );
            fprintf( dxf_file, "AcDbEntity\n" );
            fprintf( dxf_file, "  8\n" );
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
            fprintf( dxf_file, "  62\n" );
            fprintf( dxf_file, "  %d\n", color );
            //Same Start/End Width:
            fprintf( dxf_file, "  40\n" );
            fprintf( dxf_file, "0.0\n" );
            fprintf( dxf_file, "  41\n" );
            fprintf( dxf_file, "0.0\n" );

            for ( unsigned int j = 0; j < allflines[l].size(); j++ )
            {
                fprintf( dxf_file, "  0\n" );
                fprintf( dxf_file, "VERTEX\n" );
                fprintf( dxf_file, "  100\n" );
                fprintf( dxf_file, "AcDbEntity\n" );
                fprintf( dxf_file, "  8\n" );
                fprintf( dxf_file, "%s\n", layer.c_str() );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbVertex" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDb3dPolylineVertex" );
                fprintf( dxf_file, "  %d\n", 10 );
                fprintf( dxf_file, "  62\n" );
                fprintf( dxf_file, "  %d\n", color );
                fprintf( dxf_file, "%f\n", allflines[l][j].x() ); //x
                fprintf( dxf_file, "  20\n" );
                fprintf( dxf_file, "%f\n", allflines[l][j].y() ); //y
                fprintf( dxf_file, "  30\n" );
                fprintf( dxf_file, "%f\n", allflines[l][j].z() ); //z
                fprintf( dxf_file, "  70\n" );
                fprintf( dxf_file, "    32\n" );
            }
            fprintf( dxf_file, "  0\n" );
            fprintf( dxf_file, "SEQEND\n" );
            fprintf( dxf_file, "  8\n" );
            fprintf( dxf_file, "%s\n", layer.c_str() );
        }
    }
}

void WriteDXFPolylines2D( FILE* dxf_file, const vector < vector < vec3d > > &allflines, string layer, bool colorflag, int color_count )
{
    if ( dxf_file )
    {
        int color;

        if ( colorflag )
        {
            color = DXFColorWheel( color_count );
        }
        else
        {
            color = 0; // Black
        }

        for ( unsigned int l = 0; l < allflines.size(); l++ )
        {
            fprintf( dxf_file, "  0\n" );
            fprintf( dxf_file, "POLYLINE\n" );
            fprintf( dxf_file, "  100\n" );
            fprintf( dxf_file, "AcDbEntity\n" );
            fprintf( dxf_file, "  8\n" );
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
            fprintf( dxf_file, "  62\n" );
            fprintf( dxf_file, "  %d\n", color );
            //Same Start/End Width:
            fprintf( dxf_file, "  40\n" );
            fprintf( dxf_file, "0.0\n" );
            fprintf( dxf_file, "  41\n" );
            fprintf( dxf_file, "0.0\n" );

            for ( unsigned int j = 0; j < allflines[l].size(); j++ )
            {
                fprintf( dxf_file, "  0\n" );
                fprintf( dxf_file, "VERTEX\n" );
                fprintf( dxf_file, "  100\n" );
                fprintf( dxf_file, "AcDbEntity\n" );
                fprintf( dxf_file, "  8\n" );
                fprintf( dxf_file, "%s\n", layer.c_str() );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDbVertex" );
                fprintf( dxf_file, "  %d\n", 100 );
                fprintf( dxf_file, "%s\n", "AcDb3dPolylineVertex" );
                fprintf( dxf_file, "  %d\n", 10 );
                fprintf( dxf_file, "  62\n" );
                fprintf( dxf_file, "  %d\n", color );
                fprintf( dxf_file, "%f\n", allflines[l][j].x() ); //x
                fprintf( dxf_file, "  20\n" );
                fprintf( dxf_file, "%f\n", allflines[l][j].y() ); //y
                fprintf( dxf_file, "  70\n" );
                fprintf( dxf_file, "    32\n" );
            }
            fprintf( dxf_file, "  0\n" );
            fprintf( dxf_file, "SEQEND\n" );
            fprintf( dxf_file, "  8\n" );
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

vec3d GetVecToOrgin( const BndBox &bndbox )
{
    vec3d to_orgin;

    int max_majcomp = bndbox.GetMax().major_comp();
    double max_maxval = bndbox.GetMax()[max_majcomp];

    int min_mincomp = bndbox.GetMin().minor_comp();
    double min_minval = bndbox.GetMin()[min_mincomp];

    if ( ( max_maxval > 0 && min_minval >= 0 ) || ( max_maxval > 0 && min_minval < 0 ) )
    {
        to_orgin = bndbox.GetMin();
    }
    else if ( ( max_maxval < 0 && min_minval <= 0 ) || ( max_maxval < 0 && min_minval > 0 ) )
    {
        to_orgin = bndbox.GetMax();
    }
    else
    {
        to_orgin.set_x( 0 );
        to_orgin.set_y( 0 );
        to_orgin.set_z( 0 );
    }

    return to_orgin;
}

int DXFColorWheel( int count )
{
    // Documentation: http://sub-atomic.com/~moses/acadcolors.html

    int intense_group = ( count / 24 ) % 3; // Last digit in color_index will be 0, 1, or 3, corresponding to an intensity

    count = count % 24; // Bound count to range 0-24

    // Increment in steps of 50, beginning at 10 for count = 0.
    int color_index = ( count % 5 ) * 50 + 10 * ( ( ( count / 5 ) % 24 ) + 1 ) + intense_group;

    return color_index;
}